#!/usr/bin/env python3
"""Send a bounded PocketD-D device projection over physical USB serial.
Requires pyserial 3.5. Never flashes, resets, erases or writes to Beyond.
"""
import argparse
import json
import time
from pathlib import Path


def payload(path):
    raw = Path(path).read_bytes()
    if len(raw) > 262144:
        raise ValueError('File oltre 262144 byte')
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError('Chiave duplicata')
            result[key] = value
        return result
    obj = json.loads(raw, object_pairs_hook=unique)
    if not isinstance(obj, dict) or obj.get('format') != 'pocketdd-device' or type(obj.get('version')) is not int or obj['version'] not in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11):
        raise ValueError('Usa la copia da “Prepara per dispositivo”, non il backup completo')
    encoded = json.dumps(obj, ensure_ascii=False, separators=(',', ':')).encode('ascii')
    if len(encoded) > 262144 or b'\n' in encoded:
        raise ValueError('Copia non compatibile')
    return encoded + b'\n'


def wait_for(port, expected, timeout, log=print):
    accepted = (expected,) if isinstance(expected, str) else expected
    deadline = time.monotonic() + timeout
    buffer = bytearray()
    while time.monotonic() < deadline:
        buffer.extend(port.readline(256))
        if len(buffer) > 512:
            raise RuntimeError('Risposta USB troppo lunga')
        if not buffer.endswith(b'\n'):
            continue
        line = buffer.decode('ascii', errors='replace').strip()
        buffer.clear()
        if not line:
            continue
        if 'PDD READY 1' in accepted and line in ['PDD ERROR packet', 'PDD ERROR size']:
            continue  # Discard an interrupted frame before the leading newline.
        if line.startswith('PDD ERROR') or line.startswith('PDD CANCELLED'):
            raise RuntimeError(line)
        if 'PDD READY 1' in accepted and line.startswith('PDD READY') and line not in accepted:
            raise RuntimeError('Versione protocollo del firmware non supportata')
        if line in accepted:
            log(line)
            return line
    raise TimeoutError(f'Nessuna risposta {expected}; esito non confermato. Controlla il display prima di riprovare.')


def upload(port, data, log=print):
    version = json.loads(data).get('version')
    if type(version) is not int or version not in (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11):
        raise ValueError('Versione della copia non supportata')
    port.write(b'\nPDD HELLO\n'); port.flush()
    ready = wait_for(port, ('PDD READY 1', 'PDD READY 2', 'PDD READY 3', 'PDD READY 4', 'PDD READY 5', 'PDD READY 6', 'PDD READY 7', 'PDD READY 8', 'PDD READY 9', 'PDD READY 10', 'PDD READY 11'), 10, log)
    if version > int(ready.rsplit(' ', 1)[1]):
        raise RuntimeError(f'Aggiorna il firmware PocketD-D: questa copia richiede la versione USB {version}. Nessuna copia inviata.')
    port.write(data); port.flush()
    wait_for(port, 'PDD WAIT_CONFIRM', 10, log)
    log('Conferma sul display entro 120 secondi. Sostituisce solo la copia sul dispositivo.')
    wait_for(port, 'PDD IMPORTED', 125, log)
    log('Copia confermata e salvata dal firmware. Raccolta PC conservata.')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('file')
    parser.add_argument('--port', required=True, help='es. /dev/ttyACM0, COM5, /dev/cu.usbmodem...')
    args = parser.parse_args()
    try:
        data = payload(args.file)
        import serial
        # Native USB Serial/JTAG; do not toggle DTR/RTS and reset the board.
        port = serial.Serial(port=None, baudrate=115200, timeout=0.25, write_timeout=10)
        port.dtr = False; port.rts = False; port.port = args.port
        port.open()
        try:
            upload(port, data)
        finally:
            port.close()
    except (ValueError, OSError, RuntimeError, TimeoutError, ImportError) as error:
        raise SystemExit(str(error))
