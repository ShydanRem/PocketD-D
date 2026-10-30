#!/usr/bin/env python3
"""Read PocketD-D runtime diagnostics over USB. No flash/reset/character writes.
Requires pyserial 3.5 only when opening a real port.
"""
import argparse
import json
import os
from pathlib import Path
import re
import tempfile
import time

MAX_BYTES = 2048
BOARD = 'ESP32-S3-Touch-AMOLED-1.75'


def validate(raw):
    if not raw or len(raw) > MAX_BYTES:
        raise ValueError('Dimensione della diagnostica non valida')
    def unique(pairs):
        result = {}
        for key, value in pairs:
            if key in result:
                raise ValueError('Chiave duplicata nella diagnostica')
            result[key] = value
        return result
    def invalid_constant(_):
        raise ValueError('Numero non valido nella diagnostica')
    data = json.loads(raw.decode('ascii'), object_pairs_hook=unique, parse_constant=invalid_constant)
    def fields(obj, expected):
        if not isinstance(obj, dict) or set(obj) != set(expected.split()):
            raise ValueError('Schema diagnostico non supportato')
    def counts(obj, keys, maximum=2**32-1):
        for key in keys.split():
            if type(obj[key]) is not int or not 0 <= obj[key] <= maximum:
                raise ValueError('Contatore diagnostico non valido')
    def flags(obj, keys):
        if any(type(obj[key]) is not bool for key in keys.split()):
            raise ValueError('Stato diagnostico non valido')
    fields(data, 'format version board_target firmware idf uptime_ms reset_reason formats memory stack io storage screen')
    if data['format'] != 'pocketdd-diagnostics' or type(data['version']) is not int or data['version'] != 1 or data['board_target'] != BOARD:
        raise ValueError('Formato o destinazione firmware non supportati')
    for key in ['firmware', 'idf']:
        if not isinstance(data[key], str) or not re.fullmatch(r'[A-Za-z0-9._+\-]{1,48}', data[key]):
            raise ValueError('Versione firmware/SDK non valida')
    counts(data, 'uptime_ms', 2**64-1);counts(data, 'reset_reason')
    fields(data['formats'], 'usb state history backup');counts(data['formats'], 'usb state history backup')
    if data['formats'] not in ({'usb': u, 'state': s, 'history': 2, 'backup': 1} for u,s in [(9,25),(10,30),(11,31),(12,32),(13,33),(14,33),(15,34),(16,35),(17,36),(18,37),(19,38)]):
        raise ValueError('Formati firmware non supportati da questo strumento')
    fields(data['memory'], 'internal psram')
    fields(data['memory']['internal'], 'free minimum_free largest_block')
    counts(data['memory']['internal'], 'free minimum_free largest_block')
    fields(data['memory']['psram'], 'total free minimum_free largest_block')
    counts(data['memory']['psram'], 'total free minimum_free largest_block')
    fields(data['stack'], 'ui_min_bytes usb_min_bytes');counts(data['stack'], 'ui_min_bytes usb_min_bytes')
    io = 'lvgl_ticks lvgl_max_us flushes_submitted touch_samples touch_errors touch_contacts pwr_read_errors pwr_presses boot_presses'
    fields(data['io'], io+' pwr_available');counts(data['io'], io);flags(data['io'], 'pwr_available')
    storage = 'state_writable history_available history_pending history_matches state_valid'
    fields(data['storage'], storage);flags(data['storage'], storage)
    fields(data['screen'], 'dimmed failed busy');flags(data['screen'], 'dimmed failed busy')
    return data


def _write(port, data):
    at = 0
    while at < len(data):
        count = port.write(data[at:])
        if type(count) is not int or not 0 < count <= len(data)-at:
            raise OSError('Scrittura USB incompleta')
        at += count
    port.flush()


def _line(port, deadline, maximum):
    raw = bytearray()
    while time.monotonic() < deadline:
        chunk = port.read(1)
        if not chunk:
            continue
        if len(chunk) != 1:
            raise RuntimeError('Risposta USB eccedente')
        if chunk == b'\n':
            return raw.decode('ascii').rstrip('\r')
        raw.extend(chunk)
        if len(raw) > maximum:
            raise ValueError('Risposta USB fuori limite')
    raise TimeoutError('Nessuna diagnostica confermata: controlla porta, firmware e display')


def acquire(port):
    _write(port, b'\nPDD HELLO\n')
    deadline = time.monotonic()+10
    while True:
        line = _line(port, deadline, 256)
        if not line or line in ('PDD ERROR packet', 'PDD ERROR size'):
            continue  # An interrupted prior line is discarded by the leading newline.
        if not re.fullmatch(r'PDD READY (?:[1-9]|10|11|12|13|14|15|16|17)', line):
            raise RuntimeError('Protocollo USB non supportato: nessun dato della scheda inviato')
        break
    _write(port, b'PDD DIAGNOSTICS GET 1\n')
    deadline = time.monotonic()+10
    while True:
        line = _line(port, deadline, MAX_BYTES+32)
        if not line:
            continue
        if not line.startswith('PDD DIAGNOSTICS '):
            raise RuntimeError('Diagnostica v1 non disponibile o dispositivo occupato. Usa firmware v0.54 o successivo e riprova.')
        return validate(line[len('PDD DIAGNOSTICS '):].encode('ascii'))


def findings(data):
    result = []
    s = data['storage']
    for key, text in [('state_writable', 'Scritture della scheda bloccate'),
                      ('history_available', 'Registro tiri non disponibile'),
                      ('history_matches', 'Scheda e registro hanno identità diverse'),
                      ('state_valid', 'Scheda in memoria non valida')]:
        if not s[key]:
            result.append(text)
    if s['history_pending']:
        result.append('Un tiro attende registrazione: risolverlo dal dispositivo')
    if data['screen']['failed']:
        result.append('Comando di luminosità non riuscito')
    if not data['io']['pwr_available']:
        result.append('Ultima lettura PWR non disponibile')
    return result


def save_new(path, data):
    raw = json.dumps(data, separators=(',', ':'), ensure_ascii=True).encode('ascii')
    validate(raw);path = Path(path);temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.pocketdd-', delete=False) as f:
            temporary = Path(f.name);f.write(raw+b'\n');f.flush();os.fsync(f.fileno())
        os.link(temporary, path)
        if hasattr(os, 'O_DIRECTORY'):
            fd = os.open(path.parent, os.O_RDONLY | os.O_DIRECTORY)
            try:
                os.fsync(fd)
            finally:
                os.close(fd)
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


def summary(data, log=print):
    log(f"Firmware {data['firmware']} · SDK {data['idf']} · destinazione compilata {data['board_target']}")
    log(f"Avvio da {data['uptime_ms']} ms · motivo reset ESP-IDF {data['reset_reason']}")
    for name, memory in data['memory'].items():
        log(f"{name}: liberi {memory['free']} B · minimo {memory['minimum_free']} B · blocco maggiore {memory['largest_block']} B")
    log(f"Stack minimo libero: UI {data['stack']['ui_min_bytes']} B · USB {data['stack']['usb_min_bytes']} B")
    io = data['io']
    log(f"Touch: {io['touch_contacts']} contatti · {io['touch_errors']} errori / {io['touch_samples']} letture")
    log(f"Tasti: PWR {io['pwr_presses']} pressioni, {io['pwr_read_errors']} letture indisponibili · BOOT {io['boot_presses']} pressioni")
    log(f"LVGL: {io['lvgl_ticks']} cicli · massimo handler {io['lvgl_max_us']} us · invii display {io['flushes_submitted']}")
    issues = findings(data)
    for issue in issues:
        log('Da verificare: '+issue)
    log('Diagnostica acquisita in sola lettura. Resa, comandi e tenuta dei salvataggi richiedono il collaudo fisico.')
    return issues


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--port', required=True, help='es. COM5, /dev/ttyACM0, /dev/cu.usbmodem…')
    parser.add_argument('--output', help='Nuovo file JSON diagnostico, senza dati del personaggio')
    args = parser.parse_args()
    try:
        if args.output and os.path.lexists(args.output):
            raise FileExistsError('Il file esiste già: scegli un nuovo nome')
        import serial
        port = serial.Serial(port=None, baudrate=115200, timeout=0.25, write_timeout=10)
        port.dtr = False;port.rts = False;port.port = args.port;port.open()
        try:
            data = acquire(port)
        finally:
            port.close()
        if args.output:
            save_new(args.output, data)
        issues = summary(data)
    except (ValueError, OSError, RuntimeError, TimeoutError, ImportError) as error:
        raise SystemExit(str(error))
    raise SystemExit(2 if issues else 0)


if __name__ == '__main__':
    main()
