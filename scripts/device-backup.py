#!/usr/bin/env python3
"""Acquire/restore a native PocketD-D backup over USB. Requires pyserial 3.5.
Never flashes, resets, erases, modifies the PC character collection or contacts Beyond.
"""
import argparse
import base64
import json
import os
from pathlib import Path
import re
import tempfile
import time
import zlib

MAX_BYTES = 110000


def validate(raw):
    """Envelope/CRC/epoch integrity only; firmware validates all native semantics."""
    if not raw or len(raw) > MAX_BYTES or b'\\' in raw:
        raise ValueError('Backup non valido o oltre 110000 byte')
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError('Chiave duplicata nel backup')
            result[key] = value
        return result
    obj = json.loads(raw.decode('ascii'), object_pairs_hook=unique)
    if (not isinstance(obj, dict) or set(obj) != {'format', 'version', 'state', 'history'}
            or obj['format'] != 'pocketdd-device-backup' or type(obj['version']) is not int or obj['version'] != 1):
        raise ValueError('Usa un backup nativo del dispositivo v1')
    records = []
    for key, magic, maximum in [('state', (b'PDDP', b'PDDQ', b'PDDR'), 75182), ('history', (b'PDH1',), 2481)]:
        text = obj[key]
        if not isinstance(text, str) or not text or len(text) > 4*((maximum+2)//3):
            raise ValueError('Record nativo fuori limite')
        data = base64.b64decode(text, validate=True)
        if (base64.b64encode(data).decode('ascii') != text or len(data) < 8 or len(data) > maximum
                or data[:4] not in magic or zlib.crc32(data[:-4]) != int.from_bytes(data[-4:], 'little')):
            raise ValueError('Record nativo corrotto o non supportato')
        records.append(data)
    state, history = records
    trailer = 63 if state[:4] == b'PDDR' else 0
    if (len(state) < 8879+trailer or state[5123] > 120 or len(state) != 8879+trailer+552*state[5123]
            or len(history) < 21 or history[16] > 20 or len(history) != 21+123*history[16]
            or state[len(state)-8-trailer:len(state)-4-trailer] != history[8:12]):
        raise ValueError('Scheda e storico non coerenti')
    name = state[8:57].split(b'\0', 1)[0]
    if not name or len(name) > 48 or any(c < 32 or c > 126 for c in name):
        raise ValueError('Nome nel backup non valido')
    return obj, {'name': name.decode('ascii'), 'rolls': history[16]}


def _write(port, data):
    at = 0
    while at < len(data):
        count = port.write(data[at:])
        if not isinstance(count, int) or count <= 0 or count > len(data)-at:
            raise OSError('Scrittura USB incompleta')
        at += count
    port.flush()


def _line(port, deadline):
    data = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(1)
        if not chunk:
            continue
        if chunk == b'\n':
            return data.decode('ascii').rstrip('\r')
        data.extend(chunk)
        if len(data) > 256:
            raise RuntimeError('Risposta USB fuori limite')
    raise TimeoutError('Tempo USB esaurito: esito non confermato')


def _wait(port, expected, timeout, interrupted=False):
    deadline = time.monotonic()+timeout
    while time.monotonic() < deadline:
        line = _line(port, deadline)
        if not line:
            continue
        if interrupted and line in ('PDD ERROR packet', 'PDD ERROR size'):
            continue
        if line.startswith(('PDD ERROR', 'PDD CANCELLED')):
            raise RuntimeError(line)
        if line == expected or (expected == 'DATA' and line.startswith('PDD BACKUP_DATA ')):
            return line
        raise RuntimeError('Protocollo backup non supportato o risposta inattesa')
    raise TimeoutError('Nessuna conferma USB')


def negotiate(port):
    _write(port, b'\nPDD BACKUP HELLO\n')
    try:
        _wait(port, 'PDD BACKUP_READY 1', 10, interrupted=True)
    except (RuntimeError, TimeoutError) as error:
        raise RuntimeError('Negoziazione backup non riuscita: nessuna copia inviata. Usa firmware e strumento v0.53 o successivi.') from error


def acquire(port, log=print):
    negotiate(port)
    _write(port, b'PDD BACKUP GET 1\n')
    _wait(port, 'PDD WAIT_BACKUP', 10)
    log('Conferma l’invio sul display entro 120 secondi.')
    header = _wait(port, 'DATA', 125)
    match = re.fullmatch(r'PDD BACKUP_DATA ([1-9][0-9]{0,5}) ([0-9a-f]{8})', header)
    if not match or int(match[1]) > MAX_BYTES:
        raise ValueError('Dimensione o intestazione backup non valida')
    size = int(match[1]);data = bytearray();deadline = time.monotonic()+30
    while len(data) < size and time.monotonic() < deadline:
        chunk = port.read(min(4096, size-len(data)))
        if len(chunk) > size-len(data):
            raise RuntimeError('Dati USB eccedenti')
        data.extend(chunk)
    if len(data) != size or zlib.crc32(data) != int(match[2], 16):
        raise ValueError('Backup incompleto o corrotto: nessun file salvato')
    _wait(port, 'PDD BACKUP_DONE', 10)
    validate(data)
    return bytes(data)


def save_new(path, raw):
    """Publish a private complete file atomically, without overwriting any path."""
    validate(raw);path = Path(path)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.pocketdd-', delete=False) as f:
            temporary = Path(f.name);f.write(raw);f.flush();os.fsync(f.fileno())
        os.link(temporary, path)  # Atomic create; an existing file/symlink always fails.
        if hasattr(os, 'O_DIRECTORY'):
            fd = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(fd)
            finally:
                os.close(fd)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def restore(port, raw, log=print):
    obj, _ = validate(raw)
    encoded = json.dumps(obj, separators=(',', ':'), ensure_ascii=True).encode('ascii')+b'\n'
    negotiate(port)
    _write(port, encoded)
    _wait(port, 'PDD WAIT_RESTORE', 10)
    log('Controlla la copia e conferma sul dispositivo entro 120 secondi. Ripristina scheda, storico e preferenze.')
    _wait(port, 'PDD RESTORED', 125)
    log('Ripristino confermato dal dispositivo. Raccolta PC conservata.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    read = sub.add_parser('read');read.add_argument('--output', required=True);read.add_argument('--port', required=True)
    write = sub.add_parser('restore');write.add_argument('file');write.add_argument('--port', required=True)
    args = parser.parse_args()
    try:
        if args.command == 'read' and os.path.lexists(args.output):
            raise FileExistsError('Il file di destinazione esiste già: scegli un nuovo nome')
        raw = None
        if args.command == 'restore':
            with Path(args.file).open('rb') as source:
                raw = source.read(MAX_BYTES+1)
        if raw is not None:
            validate(raw)
        import serial
        port = serial.Serial(port=None, baudrate=115200, timeout=0.25, write_timeout=10)
        port.dtr = False;port.rts = False;port.port = args.port;port.open()
        try:
            if args.command == 'read':
                raw = acquire(port);save_new(args.output, raw)
                print('Backup integro salvato nel nuovo file. Scheda sul dispositivo invariata.')
            else:
                restore(port, raw)
        finally:
            port.close()
    except (ValueError, OSError, RuntimeError, TimeoutError, ImportError) as error:
        raise SystemExit(str(error))


if __name__ == '__main__':
    main()
