#!/usr/bin/env python3
"""Ready-to-run PC support app: compiled UI and local Beyond connector, no user data."""
import argparse,hashlib,json,subprocess,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
p=argparse.ArgumentParser();p.add_argument('output',type=Path,nargs='?',default=ROOT/'dist/install/downloads/PocketDD-PC.zip');args=p.parse_args()
if not (ROOT/'dist/index.html').is_file():raise SystemExit('Run npm run build first')
subprocess.run(['python3',str(ROOT/'scripts/fetch-windows-runtime.py'),'--verify'],check=True)
runtime_info=json.loads((ROOT/'scripts/windows-runtime.json').read_text());runtime=ROOT/'.pocketdd-runtime'/f'node-{runtime_info["version"]}-{runtime_info["platform"]}'
files=[ROOT/'docs/OGGETTI-SINTONIA.md',ROOT/'docs/previews/v086-attunement.png',ROOT/'docs/EQUIPAGGIAMENTO-MAGIE.md',ROOT/'docs/previews/v085-device-session.png',ROOT/'docs/GITHUB-PAGES.md',ROOT/'docs/previews/v084-wifi-direct.png',ROOT/'docs/previews/v083-sd-install.png',ROOT/'docs/SD-WEB-USB.md',ROOT/'package.json',ROOT/'Avvia-PocketDD.cmd',ROOT/'Avvia-Ponte.cmd',ROOT/'docs/PONTE-BEYOND.md',ROOT/'docs/PONTE-BEYOND-PROVE.md',ROOT/'docs/STATO-SOFTWARE.md',ROOT/'docs/USO-PRATICO.md',ROOT/'docs/INIZIA-QUI.md',ROOT/'docs/SESSIONE-V082.md',ROOT/'docs/BEYOND-CONSENSO-V0821.md',ROOT/'docs/previews/v082-key-picker.png',ROOT/'docs/previews/v081-setup-flow.png',ROOT/'docs/previews/v080-practical-device.png',ROOT/'docs/BEYOND-PERSONALIZZAZIONI-TIRI.md',ROOT/'docs/ABILITA-CARATTERISTICHE.md',ROOT/'docs/DIFESE-VERIFICA-RAPIDA.md',ROOT/'docs/BEYOND-CONTATORI-MANUALI.md',ROOT/'docs/BEYOND-CANNONE.md',ROOT/'docs/BEYOND-V1.md',ROOT/'docs/WIFI.md',ROOT/'docs/BEYOND-TEMPO-REALE.md',ROOT/'docs/previews/v078-reference-wifi.png',ROOT/'docs/previews/v079-bridge.png']
for directory in ('src','scripts'):
 files += [f for f in (ROOT/directory).glob('*') if f.is_file() and f.suffix in ('.js','.json') and not f.name.endswith('.test.js')]
files += [f for folder in ('extension','tools/bridge') for f in (ROOT/folder).rglob('*') if f.is_file()]
files += [ROOT/'dist/index.html']+[f for f in (ROOT/'dist/assets').rglob('*') if f.is_file()]
# Include the embedded installer without its downloads (avoids a recursive PC ZIP).
files += [f for name in ('index.html','manifest.json') for f in [ROOT/'dist/install'/name]]
files += [f for folder in ('assets','firmware','licenses') for f in (ROOT/'dist/install'/folder).rglob('*') if f.is_file()]
# Include newly added launcher files before their first commit; no scans of runtime/private directories.
version=json.loads((ROOT/'package.json').read_text())['version'];commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip()
args.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as z:
 for f in sorted(set(files)):z.write(f,'PocketDD-PC/'+str(f.relative_to(ROOT)))
 for name in ['node.exe','LICENSE','runtime-info.json']:z.write(runtime/name,'PocketDD-PC/runtime/'+name)
 z.writestr('PocketDD-PC/LEGGIMI.txt',f'''PocketD-D PC {version}

1. Estrai tutta la cartella PocketDD-PC (Windows x64).
2. Apri Avvia-PocketDD.cmd. Node.js è già incluso: non installare altri strumenti.
3. In Chrome o Edge segui la guida: Pocket -> Personaggio -> Al tavolo.

Installazione, importazione Beyond, aspetto e invio USB sono nella stessa app.
Se il Pocket è già pronto puoi saltare l'installazione. Puoi anche preparare il PG ora e inviarlo dopo.
L'invio termina solo quando confermi sul display e il Pocket risponde che ha salvato.
Le altre schede PC restano conservate. Guida completa: docs/INIZIA-QUI.md.
L'installer incluso è per Waveshare ESP32-S3-Touch-AMOLED-1.75 standard, non C.
Il runtime Windows ufficiale non modificato e la sua licenza sono in runtime/.
Lascia aperta la finestra di avvio durante la preparazione; per giocare offline puoi chiuderla.

Look sulla SD senza lettore: Installa look sulla SD via USB (firmware 0.83), docs/SD-WEB-USB.md.
Uso pratico sul dispositivo, filtri e icone: docs/USO-PRATICO.md.
Profilo PG, iniziativa e dotazioni: nuova importazione v20, firmware 0.78 / USB17.
Wi-Fi: Menu > ingranaggio > Wi-Fi > Cerca reti; password sul display; vedi docs/WIFI.md.
Ponte Beyond sperimentale: Avvia-Ponte.cmd e cartella extension. Guida docs/PONTE-BEYOND.md.
USB/installer/SD riferiti risolti dall utente; le nuove funzioni e il ponte richiedono collaudo separato.
Sintonia e bonus permanenti: docs/OGGETTI-SINTONIA.md. Firmware 0.86 / USB19, PC22, PDDc38; legge i precedenti.
Magie ed equipaggiamento operativi: docs/EQUIPAGGIAMENTO-MAGIE.md. Firmware 0.85 / USB18, PC21, backup PDDb37; legge i precedenti.
L'app e gia compilata: non servono npm install o una compilazione del firmware.
Caratteristica personalizzata delle abilita: da Beyond tipo 27 o Personalizza eroe.
Per trasferirla serve firmware 0.74 / USB15; nuovi backup nativi PDD34 letti da questa app.
Creazione del cannone riconosciuta: gratuito per riposo lungo, poi slot scelto e confermato.
Richiede firmware 0.77 / USB16; backup PC v19 e nativi PDD35. Altre varianti manuali.
Contatori manuali Beyond separati dagli utilizzi; saldi in Stato, risorse e note e Usi sul dispositivo.
Backup PC v18 solo con questi contatori, protocolli USB esistenti. Nessun recupero automatico.
Verifica rapida difese: Danni > Difese per tipo > Completa sul dispositivo (0.75).
Sul PC: Stato, risorse e note > Difese ai danni. Scelta esplicita, non automatica.
Report aggiornato e limiti: docs/STATO-SOFTWARE.md.
Personalizzazioni standard di abilita/salvezze riconciliate; altre eccezioni restano da verificare.
Punteggi fissati e massimi delle caratteristiche riconciliati; bonus personalizzato dopo il limite.
Cariche, consumabili e cure esplicite vengono riconciliati; recuperi ambigui restano manuali.
Un profilo difese richiede firmware 0.71 / USB14: aggiorna PocketD-D conservando i dati.
La conversione Beyond resta parziale; personaggi privati non sono supportati.
L'importazione crea una nuova scheda PC; il trasferimento sostituisce la copia sul dispositivo.
La raccolta resta nel browser: usa sempre lo stesso indirizzo e conserva un backup.
macOS/Linux: node --use-env-proxy scripts/start-pc.js
Chiusura: Ctrl+C nella finestra del programma.
''')
 z.writestr('PocketDD-PC/package-info.json',json.dumps(dict(version=version,source_commit=commit,personal_data=False,firmware_included=True,windows_runtime=runtime_info['version']),indent=2)+'\n')
digest=hashlib.sha256(args.output.read_bytes()).hexdigest();args.output.with_suffix('.zip.sha256').write_text(f'{digest}  {args.output.name}\n');print(f'PC package: {args.output.stat().st_size} bytes, SHA256 {digest}')

# Keep hosting files below per-object limits; browser returns one byte-identical ZIP.
data=args.output.read_bytes();parts=[]
for old in args.output.parent.glob('PocketDD-PC.part*.bin'):old.unlink()
for i,at in enumerate(range(0,len(data),16*1024*1024)):
 block=data[at:at+16*1024*1024];name=f'PocketDD-PC.part{i}.bin';(args.output.parent/name).write_bytes(block);parts.append(dict(path=name,size=len(block),sha256=hashlib.sha256(block).hexdigest()))
(args.output.parent/'pc-package.json').write_text(json.dumps(dict(version=1,filename='PocketDD-PC.zip',size=len(data),sha256=digest,parts=parts),indent=2)+'\n')
