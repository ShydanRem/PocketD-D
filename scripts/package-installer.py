#!/usr/bin/env python3
"""Package the compiled standalone installer, firmware and local launcher."""
import argparse,hashlib,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('output',type=Path);args=p.parse_args();base=ROOT/'dist/install'
if not (base/'manifest.json').is_file():raise SystemExit('Run npm run installer:build first')
with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as z:
 for path in sorted(base.rglob('*')):
  if path.is_file():z.write(path,'PocketDD-installer/'+str(path.relative_to(base)))
 z.write(ROOT/'scripts/serve-installer.py','PocketDD-installer/Avvia-PocketDD.py')
 z.writestr('PocketDD-installer/Avvia-Windows.cmd','@echo off\r\ncd /d "%~dp0"\r\nwhere py >nul 2>nul\r\nif %errorlevel% equ 0 (py -3 Avvia-PocketDD.py) else (python Avvia-PocketDD.py)\r\npause\r\n')
 z.writestr('PocketDD-installer/LEGGIMI.txt','PocketD-D — installer USB\n\nWindows: doppio clic su Avvia-Windows.cmd (richiede Python 3).\nmacOS/Linux: python3 Avvia-PocketDD.py\nApri http://127.0.0.1:8787/ in Chrome o Edge sul PC.\nNon aprire index.html direttamente. Lascia aperto il launcher.\n\nIn alternativa pubblica questa cartella su un hosting HTTPS statico: per installare da quel link non serve Python.\nFirmware, grafica e librerie sono inclusi. Nessun download esterno richiesto durante l’uso locale.\n\nSolo Waveshare ESP32-S3-Touch-AMOLED-1.75 standard, senza suffisso C.\nPrima installazione sostituisce il firmware presente. Per aggiornare PocketD-D usa Aggiorna e conserva un backup.\nLa pagina non cancella tutta la flash; controlla ESP32-S3, 16MB, SHA256 e verifica MD5 dopo la scrittura.\nChip e flash non identificano da soli il modello di scheda.\nProve hardware ancora da eseguire.\n')
digest=hashlib.sha256(args.output.read_bytes()).hexdigest();args.output.with_suffix('.zip.sha256').write_text(f'{digest}  {args.output.name}\n');print(f'{args.output}: {args.output.stat().st_size} bytes\nSHA256 {digest}')
