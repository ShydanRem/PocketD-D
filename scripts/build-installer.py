#!/usr/bin/env python3
"""Stage only validated, built PocketD-D firmware into the static USB installer."""
import hashlib,json,shutil,struct,subprocess
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
build=ROOT/'firmware/build';out=ROOT/'dist/install'
version=json.loads((ROOT/'package.json').read_text())['version']
if not (out/'index.html').is_file():raise SystemExit('Build installer UI first: npm run installer:build')
project=json.loads((build/'project_description.json').read_text())
if project['project_version']!=version or project['target']!='esp32s3':raise SystemExit('Firmware version/target mismatch: rebuild firmware first')
args=json.loads((build/'flasher_args.json').read_text())
expected={'0x0':'bootloader/bootloader.bin','0x8000':'partition_table/partition-table.bin','0x10000':'pocketdd.bin'}
if args['flash_files']!=expected or args['flash_settings']!={'flash_mode':'dio','flash_size':'16MB','flash_freq':'80m'}:raise SystemExit('Unexpected flash layout or settings')
parts=[];data_by_id={}
for ident,offset,source,limit in [('bootloader',0,'bootloader/bootloader.bin',0x8000),('partitions',0x8000,'partition_table/partition-table.bin',0x9000),('app',0x10000,'pocketdd.bin',0x410000)]:
 data=(build/source).read_bytes();data_by_id[ident]=data
 if not data or offset+((len(data)+4095)//4096)*4096>limit:raise SystemExit('Firmware extends into protected sectors')
 if ident!='partitions' and data[0]!=0xe9:raise SystemExit('Invalid ESP image')
 parts.append(dict(id=ident,path=f'firmware/{ident}.bin',offset=offset,size=len(data),sha256=hashlib.sha256(data).hexdigest()))
# Require the known PocketD-D layout, including both separately protected NVS areas.
table=data_by_id['partitions'];entries=[]
if len(table)!=0xc00:raise SystemExit('Unexpected partition table size')
for at in range(0,len(table),32):
 entry=table[at:at+32]
 if entry[:2]!=b'\xaaP':break
 _,kind,subtype,offset,size,label,flags=struct.unpack('<HBBII16sI',entry)
 entries.append((label.rstrip(b'\0').decode(),kind,subtype,offset,size,flags))
if entries!=[('pdd_nvs',1,2,0x9000,0x6000,0),('phy_init',1,1,0xf000,0x1000,0),('factory',0,0,0x10000,0x400000,0),('pdd_state',1,2,0x410000,0x40000,0)]:raise SystemExit('Unknown PocketD-D partitions')
firmware=out/'firmware';firmware.mkdir(exist_ok=True)
for part in parts:(out/part['path']).write_bytes(data_by_id[part['id']])
manifest=dict(schema=1,name='PocketD-D',version=version,board='Waveshare ESP32-S3-Touch-AMOLED-1.75',chip='ESP32-S3',flash_size=0x1000000,hardware_tested=False,commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip(),parts=parts)
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
licenses=out/'licenses';licenses.mkdir(exist_ok=True)
for package in ('esptool-js','pako','tslib','atob-lite','crypto-js'):
 directory=ROOT/'node_modules'/package;found=False
 for path in directory.iterdir():
  if path.is_file() and path.name.lower().startswith(('license','licence','copying','copyright','notice')):
   shutil.copyfile(path,licenses/f'{package}-{path.name}.txt');found=True
 if not found:raise SystemExit(f'Missing third-party license: {package}')
print(f'Installer: firmware {version}, 3 separate regions, SHA256, known partitions, bundled dependencies/licenses; no user data')
gallery=ROOT/'firmware/tests/out/ui-build/appearance-preview.png'
if gallery.is_file():
 shutil.copyfile(gallery,out/'assets/personalizzazione.png')
 (out/'assets/personalizzazione.css').write_text('body{margin:0;background:#07151e;color:#ffefd2;font:17px system-ui}main{max-width:1500px;margin:auto;padding:24px}a{color:#8ce6c2}img{display:block;width:100%;height:auto;margin-top:24px}p{max-width:800px;line-height:1.6}')
 (out/'personalizzazione.html').write_text(f'''<!doctype html><html lang="it"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>PocketD-D {version} · Personalizzazione</title><link rel="stylesheet" href="assets/personalizzazione.css"><main><a href="/">← PocketD-D</a><h1>Il tuo eroe, il tuo ambiente</h1><p>Pelle, capelli, taglio e mantello configurabili sul dispositivo. Accampamento, rovine lunari e porto all’alba, con anteprima e conferma.</p><p>Queste sono catture del software del firmware {version} eseguito su PC con un personaggio di prova. La verifica sul dispositivo resta da fare.</p><a href="assets/personalizzazione.png">Apri l’immagine alla risoluzione originale</a><img src="assets/personalizzazione.png" width="1470" height="980" alt="Tre ambienti della home PocketD-D e le schermate per personalizzare e confermare l’aspetto del personaggio"></main></html>''')

# Native resolution screenshots from the production renderer, never concept replacements.
species=json.loads((ROOT/'src/avatar-species.json').read_text())
cards=[]
for entry in species:
 for body,label in enumerate(['Maschile','Femminile']):
  filename=f'species-home-{entry["id"]:02d}-{body}.png';source=ROOT/'firmware/tests/out/ui-build'/filename
  if not source.is_file():raise SystemExit(f'Missing species production capture: {filename}')
  shutil.copyfile(source,out/'assets'/filename)
  cards.append(f'<figure><img loading="lazy" src="assets/{filename}" width="466" height="466" alt="{entry["name"]}, silhouette {label.lower()}, nella home del firmware"><figcaption>{entry["name"]} · {label}</figcaption></figure>')
page=out/'personalizzazione.html'
page.write_text(page.read_text().replace('</main>', '<h2>16 specie · 32 silhouette</h2><p>Ora anche Tabaxi, Kenku, Aarakocra e Tortle, con anatomie dedicate. Maschile e femminile sono indipendenti da colori e tagli. Beyond propone la famiglia visiva riconosciuta; puoi cambiarla nell’anteprima o sul dispositivo. Le specie sconosciute richiedono una scelta manuale.</p><section class="species-grid">'+''.join(cards)+'</section></main>'))
css=out/'assets/personalizzazione.css';css.write_text(css.read_text()+'.species-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,300px),1fr));gap:20px}figure{margin:0;padding:12px;background:#12252e;border:1px solid #40645e;border-radius:14px}figure img{margin:0;width:100%;image-rendering:pixelated}figcaption{padding:10px;color:#eac884;text-align:center}')
