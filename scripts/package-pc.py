#!/usr/bin/env python3
"""Ready-to-run PC support app: compiled UI and local Beyond connector, no user data."""
import argparse,hashlib,json,subprocess,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('output',type=Path,nargs='?',default=ROOT/'dist/install/downloads/PocketDD-PC.zip');args=p.parse_args()
if not (ROOT/'dist/index.html').is_file():raise SystemExit('Run npm run build first')
files=[ROOT/'package.json',ROOT/'Avvia-PocketDD.cmd']
for directory in ('src','scripts'):
 files += [f for f in (ROOT/directory).glob('*') if f.is_file() and f.suffix in ('.js','.json') and not f.name.endswith('.test.js')]
files += [ROOT/'dist/index.html']+[f for f in (ROOT/'dist/assets').rglob('*') if f.is_file()]
# Include newly added launcher files before their first commit; no scans of runtime/private directories.
version=json.loads((ROOT/'package.json').read_text())['version'];commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
args.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as z:
 for f in sorted(files):z.write(f,'PocketDD-PC/'+str(f.relative_to(ROOT)))
 z.writestr('PocketDD-PC/LEGGIMI.txt',f'''PocketD-D PC {version}

1. Estrai tutta la cartella PocketDD-PC.
2. Installa Node.js 24 attuale da https://nodejs.org/ (una sola volta).
3. Su Windows fai doppio clic su Avvia-PocketDD.cmd.
4. Si apre http://127.0.0.1:5173/. Lascia aperta la finestra del programma.
5. Usa Chrome o Edge. Importa da D&D Beyond -> link del PG pubblico -> Leggi e verifica.
6. Controlla caratteristiche, PF, CA, inventario e omissioni; crea il nuovo personaggio solo dopo la verifica.
7. Prepara per dispositivo (USB) -> Invia al dispositivo via USB -> conferma sul display.

L'app e gia compilata: non servono npm install o una compilazione del firmware.
La conversione Beyond resta parziale; personaggi privati non sono supportati.
L'importazione crea una nuova scheda PC; il trasferimento sostituisce la copia sul dispositivo.
La raccolta resta nel browser: usa sempre lo stesso indirizzo e conserva un backup.
macOS/Linux: node --use-env-proxy scripts/start-pc.js
Chiusura: Ctrl+C nella finestra del programma.
''')
 z.writestr('PocketDD-PC/package-info.json',json.dumps(dict(version=version,source_commit=commit,personal_data=False,firmware_included=False),indent=2)+'\n')
digest=hashlib.sha256(args.output.read_bytes()).hexdigest();args.output.with_suffix('.zip.sha256').write_text(f'{digest}  {args.output.name}\n');print(f'PC package: {args.output.stat().st_size} bytes, SHA256 {digest}')
