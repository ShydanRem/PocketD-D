#!/usr/bin/env python3
"""Synthetic tests only. No board, serial port, personal collection or Beyond access."""
import importlib.util
import json
from pathlib import Path
import tempfile
import unittest

spec = importlib.util.spec_from_file_location('upload', Path(__file__).with_name('device-upload.py'))
upload = importlib.util.module_from_spec(spec)
spec.loader.exec_module(upload)

class Port:
    def __init__(self, lines):
        self.lines = iter(lines)
        self.sent = []
    def write(self, data):
        self.sent.append(data)
    def flush(self):
        pass
    def readline(self, size=256):
        return next(self.lines, b'PDD ERROR no-response\n')

class USBTests(unittest.TestCase):
    def test_success_requires_device_ack_after_confirmation(self):
        for version, ready in [(v,r) for r in range(1,6) for v in range(1,r+1)]:
            data=json.dumps({'version':version}).encode()+b'\n'
            port = Port([f'PDD READY {ready}\n'.encode(), b'PDD WAIT_CONFIRM\n', b'PDD IMPORTED\n'])
            upload.upload(port, data, lambda _: None)
            self.assertEqual(port.sent, [b'\nPDD HELLO\n', data])
    def test_fragmented_responses_and_interrupted_frame(self):
        port = Port([b'PDD ERROR packet\n', b'PDD RE', b'ADY 2\n', b'PDD WAIT_', b'CONFIRM\n', b'PDD IMP', b'ORTED\n'])
        upload.upload(port, b'{"version":2}\n', lambda _: None)
    def test_refusal_and_storage_error_never_report_success(self):
        for response in [b'PDD CANCELLED\n', b'PDD ERROR storage\n']:
            port = Port([b'PDD READY 2\n', b'PDD WAIT_CONFIRM\n', response])
            with self.assertRaises(RuntimeError):
                upload.upload(port, b'{"version":2}\n', lambda _: None)
    def test_old_or_unknown_firmware_receives_no_v5_payload(self):
        for ready in [b'PDD READY 1\n',b'PDD READY 2\n',b'PDD READY 3\n',b'PDD READY 4\n',b'PDD READY 6\n']:
            port=Port([ready])
            with self.assertRaises(RuntimeError):upload.upload(port,b'{"version":5}\n',lambda _:None)
            self.assertEqual(port.sent,[b'\nPDD HELLO\n'])
    def test_backup_wrong_schema_and_duplicate_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            path = Path(directory) / 'input.json'
            for raw in ['{"version":10}', '{"format":"pocketdd-device","version":1,"version":1}', '[]', '{"format":"pocketdd-device","version":6}', '{"format":"pocketdd-device","version":true}']:
                path.write_text(raw)
                with self.assertRaises(ValueError):
                    upload.payload(path)
            path.write_text('{"format":"pocketdd-device","version":1}')
            self.assertTrue(upload.payload(path).endswith(b'\n'))
            path.write_text('{"format":"pocketdd-device","version":2}')
            self.assertTrue(upload.payload(path).endswith(b'\n'))

if __name__ == '__main__':
    unittest.main()
