#!/usr/bin/env python3
"""Synthetic diagnostics transport/schema tests. Never opens a real serial port."""
import copy
import io
from contextlib import redirect_stdout
import importlib.util
import json
import os
from pathlib import Path
import tempfile
import sys
from types import SimpleNamespace
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('device_check', Path(__file__).with_name('device-check.py'))
check = importlib.util.module_from_spec(spec);spec.loader.exec_module(check)
# These are fresh outputs from the production C codec and UI snapshot, not a
# handwritten Python copy of their schema. Build/run the host suite first.
LIVE = (ROOT/'firmware/tests/out/ui-build/diagnostics-live.json').read_bytes()
CODEC = (ROOT/'firmware/tests/out/ui-build/diagnostics-codec.json').read_bytes()
DATA = check.validate(LIVE)


class Port:
    def __init__(self, raw, partial=False):
        self.raw = bytearray(raw);self.sent = bytearray();self.partial = partial
    def read(self, size):
        data = bytes(self.raw[:size]);del self.raw[:size];return data
    def write(self, data):
        n = min(len(data), 3) if self.partial else len(data)
        self.sent.extend(data[:n]);return n
    def flush(self):
        pass


class DiagnosticsTests(unittest.TestCase):
    def test_fresh_production_c_snapshot_values_and_no_character_data(self):
        self.assertEqual(DATA['firmware'], '0.54.0-host-test')
        self.assertEqual(DATA['memory']['internal']['free'], 123000)
        self.assertEqual(DATA['stack']['ui_min_bytes'], 4000)
        self.assertEqual(DATA['io']['touch_contacts'], 12)
        self.assertEqual(DATA['io']['pwr_presses'], 3)
        self.assertEqual(DATA['io']['boot_presses'], 2)
        self.assertTrue(all(DATA['storage'][key] for key in ['state_writable', 'history_available', 'history_matches', 'state_valid']))
        self.assertFalse(DATA['storage']['history_pending'])
        self.assertEqual(check.findings(DATA), [])
        self.assertNotIn(b'Elian', LIVE)
        self.assertEqual(check.validate(CODEC)['memory']['psram']['free'], 0)

    def test_exact_read_only_commands_fragmentation_and_partial_writes(self):
        port = Port(b'\r\nPDD ERROR packet\nPDD READY 9\r\n\nPDD DIAGNOSTICS '+LIVE+b'\r\n', partial=True)
        self.assertEqual(check.acquire(port), DATA)
        self.assertEqual(port.sent, b'\nPDD HELLO\nPDD DIAGNOSTICS GET 1\n')

    def test_old_firmware_unknown_protocol_and_busy_do_not_send_character_or_backup(self):
        for raw in [b'PDD READY 11\n', b'factory firmware\n', b'PDD ERROR busy\n']:
            port = Port(raw)
            with self.assertRaises(RuntimeError):
                check.acquire(port)
            self.assertEqual(port.sent, b'\nPDD HELLO\n')
        for raw in [b'PDD ERROR packet\n', b'PDD ERROR busy\n', b'PDD BACKUP_READY 1\n']:
            port = Port(b'PDD READY 9\n'+raw)
            with self.assertRaises(RuntimeError):
                check.acquire(port)
            self.assertEqual(port.sent, b'\nPDD HELLO\nPDD DIAGNOSTICS GET 1\n')

    def test_timeout_is_not_a_success(self):
        with patch.object(check.time, 'monotonic', side_effect=[0, 1, 11]):
            with self.assertRaises(TimeoutError):
                check.acquire(Port(b''))

    def test_failed_or_excessive_write_is_rejected(self):
        for result in [0, -1, None, True, 10000]:
            port = Port(b'')
            port.write = lambda data: result
            with self.assertRaises(OSError):
                check.acquire(port)

    def test_oversized_truncated_or_non_ascii_lines_reject(self):
        for raw in [b'x'*258+b'\n', b'\xff\n']:
            with self.assertRaises((ValueError, RuntimeError)):
                check.acquire(Port(raw))
        port = Port(b'PDD READY 9\nPDD DIAGNOSTICS '+b'x'*2100+b'\n')
        with self.assertRaises(ValueError):
            check.acquire(port)
        with patch.object(check.time, 'monotonic', side_effect=[0, 1, 2, 11]):
            with self.assertRaises(TimeoutError):
                check._line(Port(b'{'), 10, 2048)

    def test_duplicate_unknown_future_and_malformed_reports_reject(self):
        bad = [b'{}', b'[]', b' '*2049, LIVE[:-1], LIVE.replace(b'"version":1', b'"version":2'),
               LIVE.replace(b'"version":1', b'"version":true'), LIVE.replace(b'"version":1', b'"version":1,"version":1')]
        for raw in bad:
            with self.subTest(raw=raw[:40]), self.assertRaises(ValueError):
                check.validate(raw)
        for change in [lambda d: d.update(character='private'), lambda d: d['io'].update(extra=0),
                       lambda d: d['formats'].update(usb=10), lambda d: d.update(board_target='other'),
                       lambda d: d.update(firmware='injected\nline'), lambda d: d.update(idf='x'*49)]:
            d = copy.deepcopy(DATA);change(d)
            with self.assertRaises(ValueError):
                check.validate(json.dumps(d).encode())

    def test_counters_flags_nesting_and_nonfinite_values_reject(self):
        for group, key, value in [('io', 'touch_errors', True), ('io', 'touch_errors', -1),
                                  ('io', 'touch_errors', 2**32), ('stack', 'ui_min_bytes', 1.5),
                                  ('storage', 'state_valid', 1), ('screen', 'busy', 'false')]:
            d = copy.deepcopy(DATA);d[group][key] = value
            with self.assertRaises(ValueError):
                check.validate(json.dumps(d).encode())
        for value in [float('nan'), float('inf'), -1, 2**64, True]:
            d = copy.deepcopy(DATA);d['uptime_ms'] = value
            with self.assertRaises(ValueError):
                check.validate(json.dumps(d).encode())
        for key in ['io', 'memory', 'stack', 'storage', 'screen', 'formats']:
            d = copy.deepcopy(DATA);d[key] = []
            with self.assertRaises(ValueError):
                check.validate(json.dumps(d).encode())

    def test_faults_remain_faults_and_success_does_not_claim_physical_validation(self):
        d = copy.deepcopy(DATA)
        d['storage'].update(state_writable=False, history_available=False, history_matches=False, state_valid=False, history_pending=True)
        d['screen']['failed'] = True;d['io']['pwr_available'] = False
        self.assertEqual(len(check.findings(d)), 7)
        messages = [];self.assertEqual(check.summary(DATA, messages.append), [])
        self.assertIn('richiedono il collaudo fisico', messages[-1])
        self.assertFalse(any('superato' in message for message in messages))

    def test_complete_private_new_file_preserves_existing_paths_and_invalid_input(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'check.json';check.save_new(path, DATA)
            self.assertEqual(check.validate(path.read_bytes()), DATA)
            if os.name == 'posix':
                self.assertEqual(path.stat().st_mode & 0o777, 0o600)
            original = path.read_bytes()
            with self.assertRaises(FileExistsError):
                check.save_new(path, DATA)
            link = Path(directory)/'link.json';link.symlink_to(path)
            with self.assertRaises(FileExistsError):
                check.save_new(link, DATA)
            self.assertEqual(path.read_bytes(), original)
            bad = Path(directory)/'bad.json'
            with self.assertRaises(ValueError):
                check.save_new(bad, {})
            self.assertFalse(bad.exists())
            self.assertEqual(sorted(p.name for p in Path(directory).iterdir()), ['check.json', 'link.json'])

    def test_cli_saves_valid_report_even_when_findings_require_exit_two(self):
        for faulty in [False, True]:
            d = copy.deepcopy(DATA);d['storage']['state_writable'] = not faulty
            port = Port(b'PDD READY 9\nPDD DIAGNOSTICS '+json.dumps(d).encode()+b'\n')
            port.open = lambda: None;closed = [];port.close = lambda: closed.append(True)
            serial = SimpleNamespace(Serial=lambda **kwargs: port)
            with tempfile.TemporaryDirectory() as directory:
                path = Path(directory)/'report.json'
                with patch.dict(sys.modules, {'serial': serial}), patch.object(sys, 'argv', ['device-check.py', '--port', 'synthetic', '--output', str(path)]), redirect_stdout(io.StringIO()):
                    with self.assertRaises(SystemExit) as result:
                        check.main()
                self.assertEqual(result.exception.code, 2 if faulty else 0)
                self.assertEqual(check.validate(path.read_bytes()), d)
                self.assertFalse(port.dtr);self.assertFalse(port.rts);self.assertEqual(closed, [True])

    def test_cli_existing_report_does_not_open_serial(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory)/'report.json';path.write_bytes(b'preserve')
            with patch.object(sys, 'argv', ['device-check.py', '--port', 'synthetic', '--output', str(path)]), patch.dict(sys.modules, {'serial': None}):
                with self.assertRaises(SystemExit) as result:
                    check.main()
            self.assertIn('esiste', str(result.exception));self.assertEqual(path.read_bytes(), b'preserve')


if __name__ == '__main__':
    unittest.main()
