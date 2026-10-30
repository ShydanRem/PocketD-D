#!/usr/bin/env python3
"""Synthetic native backup transport/file tests. Never opens a serial port."""
import base64
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest
from unittest.mock import patch
import zlib

spec = importlib.util.spec_from_file_location('backup', Path(__file__).with_name('device-backup.py'))
backup = importlib.util.module_from_spec(spec);spec.loader.exec_module(backup)
RAW = (Path(__file__).resolve().parents[1]/'firmware/tests/fixtures/device-backup-v1.synthetic.json').read_bytes()


class Port:
    def __init__(self, data, fragments=17, partial_write=False):
        self.data = bytearray(data);self.sent = bytearray();self.fragments = fragments;self.partial_write = partial_write
    def read(self, size):
        n = min(size, self.fragments, len(self.data));value = self.data[:n];del self.data[:n];return bytes(value)
    def write(self, data):
        n = min(len(data), 13) if self.partial_write else len(data);self.sent.extend(data[:n]);return n
    def flush(self):
        pass


def frame(raw=RAW):
    return b'PDD BACKUP_READY 1\nPDD WAIT_BACKUP\n'+f'PDD BACKUP_DATA {len(raw)} {zlib.crc32(raw):08x}\n'.encode()+raw+b'\nPDD BACKUP_DONE\n'


class BackupTests(unittest.TestCase):
    def test_current_loadout_record_exact_envelope(self):
        path=Path(__file__).resolve().parents[1]/'firmware/tests/out/loadout-backup.synthetic.json'
        raw=path.read_bytes();obj,preview=backup.validate(raw)
        self.assertEqual(obj,json.loads(raw));self.assertEqual(preview['rolls'],0)
        data=bytearray(base64.b64decode(obj['state']));self.assertEqual(data[:4],b'PDDd')
        data[-1085]=4;data[-4:]=zlib.crc32(data[:-4]).to_bytes(4,'little');obj['state']=base64.b64encode(data).decode('ascii')
        with self.assertRaises(ValueError):backup.validate(json.dumps(obj).encode('ascii'))

    def test_acquisition_exact_bytes_fragmented_and_partial_writes(self):
        for fragments in [1, 7, 4096]:
            port = Port(frame(), fragments, partial_write=True)
            self.assertEqual(backup.acquire(port, lambda _: None), RAW)
            self.assertEqual(port.sent, b'\nPDD BACKUP HELLO\nPDD BACKUP GET 1\n')

    def test_restore_requires_capability_then_display_confirmation_and_final_ack(self):
        port = Port(b'PDD BACKUP_READY 1\nPDD WAIT_RESTORE\nPDD RESTORED\n', partial_write=True)
        backup.restore(port, RAW, lambda _: None)
        self.assertTrue(port.sent.startswith(b'\nPDD BACKUP HELLO\n'))
        self.assertEqual(json.loads(port.sent.splitlines()[-1]), json.loads(RAW))

    def test_old_or_future_firmware_receives_no_backup_payload(self):
        for response in [b'PDD BACKUP_READY 2\n', b'PDD READY 9\n', b'PDD ERROR unsupported\n']:
            port = Port(response)
            with self.assertRaises(RuntimeError):
                backup.restore(port, RAW, lambda _: None)
            self.assertEqual(port.sent, b'\nPDD BACKUP HELLO\n')

    def test_cancel_and_nvs_errors_never_report_success(self):
        for response in [b'PDD CANCELLED backup\n', b'PDD ERROR backup-history\n', b'PDD ERROR storage\n']:
            port = Port(b'PDD BACKUP_READY 1\nPDD WAIT_RESTORE\n'+response)
            messages = []
            with self.assertRaises(RuntimeError):
                backup.restore(port, RAW, messages.append)
            self.assertFalse(any('confermato' in message for message in messages))

    def test_corrupt_payload_and_oversized_header_reject(self):
        corrupt = bytearray(frame());offset = corrupt.index(RAW);corrupt[offset+12] ^= 1
        with self.assertRaises(ValueError):
            backup.acquire(Port(corrupt), lambda _: None)
        port = Port(b'PDD BACKUP_READY 1\nPDD WAIT_BACKUP\nPDD BACKUP_DATA 110001 00000000\n')
        with self.assertRaises(ValueError):
            backup.acquire(port, lambda _: None)

    def test_timeout_is_not_a_confirmation(self):
        with patch.object(backup.time, 'monotonic', side_effect=[0, 1, 2, 100]):
            with self.assertRaises(TimeoutError):
                backup._wait(Port(b''), 'PDD RESTORED', 10)

    def test_native_envelope_duplicate_future_corrupt_and_epoch_reject(self):
        for raw in [b'{}', b'[]', RAW.replace(b'"version":1', b'"version":2'), RAW.replace(b'"version":1', b'"version":true'), RAW.replace(b'"version":1', b'"version":1.0'), RAW.replace(b'"version":1', b'"version":1,"version":1'), RAW[:-1], b' '*110001]:
            with self.assertRaises((ValueError, UnicodeError)):
                backup.validate(raw)
        p = json.loads(RAW);p['state'] = '!'+p['state'][1:]
        with self.assertRaises(ValueError):
            backup.validate(json.dumps(p).encode())

    def test_pdd26_records_are_transferred_intact_and_future_tags_reject(self):
        packet = json.loads(RAW)
        state = bytearray(base64.b64decode(packet['state']))
        state[3] = ord('Q')
        state[8375+552*state[5123]] = 9  # Explicit beginning of A, transport only.
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        encoded = json.dumps(packet).encode()
        backup.validate(encoded)
        self.assertEqual(backup.acquire(Port(frame(encoded)), lambda _: None), encoded)
        state[3] = ord('T')
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        with self.assertRaises(ValueError):
            backup.validate(json.dumps(packet).encode())

    def test_pdd27_alias_trailer_keeps_epoch_and_payload_intact(self):
        packet = json.loads(RAW)
        state = bytearray(base64.b64decode(packet['state']))
        names = b'Ally A'.ljust(21, b'\0') + b'W'*20+b'\0' + b'Mage & Co'.ljust(21, b'\0')
        state = state[:-4] + names + bytearray(4)
        state[3] = ord('R')
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        encoded = json.dumps(packet).encode()
        backup.validate(encoded)
        self.assertEqual(backup.acquire(Port(frame(encoded)), lambda _: None), encoded)
        state[-71:-67] = (1).to_bytes(4, 'little')
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        with self.assertRaises(ValueError):
            backup.validate(json.dumps(packet).encode())

    def test_pdd34_skill_associations_are_retained_and_invalid_values_reject(self):
        packet = json.loads(RAW)
        old = base64.b64decode(packet['state'])
        state = bytearray(old[:-4] + bytes(63+4+18+4))
        state[3] = ord('Y')
        for i in range(18):
            state[-22+i] = i % 7
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        encoded = json.dumps(packet).encode()
        checked, preview = backup.validate(encoded)
        self.assertEqual(checked, packet)
        port = Port(b'PDD BACKUP_READY 1\nPDD WAIT_RESTORE\nPDD RESTORED\n')
        backup.restore(port, encoded, lambda _: None)
        self.assertEqual(json.loads(port.sent.splitlines()[-1]), packet)
        state[-22] = 7
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        with self.assertRaises(ValueError):
            backup.validate(json.dumps(packet).encode())

    def test_private_complete_file_and_existing_paths_preserved(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'snapshot.json';backup.save_new(path, RAW)
            self.assertEqual(path.read_bytes(), RAW)
            with self.assertRaises(FileExistsError):
                backup.save_new(path, RAW)
            self.assertEqual(path.read_bytes(), RAW)
            link = Path(directory)/'link.json';link.symlink_to(path)
            with self.assertRaises(FileExistsError):
                backup.save_new(link, RAW)
            self.assertEqual(path.read_bytes(), RAW)
            self.assertEqual(sorted(p.name for p in Path(directory).iterdir()), ['link.json', 'snapshot.json'])

    def test_pdd28_payment_bits_are_transferred_without_changing_the_record(self):
        packet = json.loads(RAW)
        state = bytearray(base64.b64decode(packet['state']))
        state = state[:-4] + bytearray(63+4)
        state[3] = ord('S')
        for row, value in enumerate([2, 1, 3]):
            state[8333+552*state[5123]+10*row] = value
        state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
        packet['state'] = base64.b64encode(state).decode()
        encoded = json.dumps(packet).encode()
        backup.validate(encoded)  # Transport integrity; full rules are checked by firmware.
        self.assertEqual(backup.acquire(Port(frame(encoded)), lambda _: None), encoded)
        port = Port(b'PDD BACKUP_READY 1\nPDD WAIT_RESTORE\nPDD RESTORED\n')
        backup.restore(port, encoded, lambda _: None)
        self.assertEqual(json.loads(port.sent.splitlines()[-1]), packet)

    def test_invalid_file_never_creates_output_or_sends_payload(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'bad.json'
            with self.assertRaises(ValueError):
                backup.save_new(path, b'{}')
            self.assertFalse(path.exists())
        port = Port(b'')
        with self.assertRaises(ValueError):
            backup.restore(port, b'{}', lambda _: None)
        self.assertEqual(port.sent, b'')

    def test_catalog_generations_preserve_old_species_and_reject_new_ids_in_old_tag(self):
        packet = json.loads(RAW)
        original = bytearray(base64.b64decode(packet['state']))
        for tag, lineage, valid in [('U', 11, True), ('U', 12, False), ('V', 12, True), ('V', 15, True), ('V', 16, False), ('W', 20, True), ('W', 21, False), ('X', 0, False)]:
            state = original[:-4] + bytes(66) + bytes(4)
            state[3] = ord(tag); state[-7] = 2; state[-6] = lineage; state[-5] = 1
            state[-4:] = zlib.crc32(state[:-4]).to_bytes(4, 'little')
            packet['state'] = base64.b64encode(state).decode()
            if valid: backup.validate(json.dumps(packet).encode())
            else:
                with self.assertRaises(ValueError): backup.validate(json.dumps(packet).encode())


if __name__ == '__main__':
    unittest.main()
