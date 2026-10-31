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
# Linked firmware licenses also accompany the downloadable binary distribution.
idf=Path(project['idf_path'])
for label,path in [('ESP-IDF',idf/'LICENSE'),('mbedTLS',idf/'components/mbedtls/mbedtls/LICENSE'),('esp32-wifi-lib',idf/'components/esp_wifi/lib/LICENSE'),('esp-phy-lib',idf/'components/esp_phy/lib/LICENSE')]:
 if not path.is_file():raise SystemExit(f'Missing linked firmware license: {label}')
 shutil.copyfile(path,licenses/f'{label}-LICENSE.txt')
print(f'Installer: firmware {version}, 3 separate regions, SHA256, known partitions, bundled dependencies/licenses; no user data')
gallery=ROOT/'firmware/tests/out/ui-build/appearance-preview.png'
if gallery.is_file():
 shutil.copyfile(gallery,out/'assets/personalizzazione.png')
 (out/'assets/personalizzazione.css').write_text('body{margin:0;background:#07151e;color:#ffefd2;font:17px system-ui}main{max-width:1500px;margin:auto;padding:24px}a{color:#8ce6c2}img{display:block;width:100%;height:auto;margin-top:24px}p{max-width:800px;line-height:1.6}')
 (out/'personalizzazione.html').write_text(f'''<!doctype html><html lang="it"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>PocketD-D {version} · Personalizzazione</title><link rel="stylesheet" href="assets/personalizzazione.css"><main><a href="./">← PocketD-D</a><h1>Il tuo eroe, il tuo ambiente</h1><p>Pelle, capelli, taglio e mantello configurabili sul dispositivo. Accampamento, rovine lunari e porto all’alba, con anteprima e conferma.</p><p>Queste sono catture del software del firmware {version} eseguito su PC con un personaggio di prova. La verifica sul dispositivo resta da fare.</p><a href="assets/personalizzazione.png">Apri l’immagine alla risoluzione originale</a><img src="assets/personalizzazione.png" width="1470" height="980" alt="Tre ambienti della home PocketD-D e le schermate per personalizzare e confermare l’aspetto del personaggio"></main></html>''')

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
page.write_text(page.read_text().replace('</main>', '<h2>21 specie · 42 silhouette</h2><p>Ora anche Goblin, Hobgoblin, Bugbear, Kobold e Lizardfolk, con anatomie dedicate. Maschile e femminile sono indipendenti da colori e tagli. Beyond propone la famiglia visiva riconosciuta; puoi cambiarla nell’anteprima o sul dispositivo. Le specie sconosciute richiedono una scelta manuale.</p><section class="species-grid">'+''.join(cards)+'</section></main>'))
css=out/'assets/personalizzazione.css';css.write_text(css.read_text()+'.species-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(min(100%,300px),1fr));gap:20px}figure{margin:0;padding:12px;background:#12252e;border:1px solid #40645e;border-radius:14px}figure img{margin:0;width:100%;image-rendering:pixelated}figcaption{padding:10px;color:#eac884;text-align:center}')

# Actual class looks and native renderer captures; SD assets stay external to flash.
sd=ROOT/'public/assets/PocketDD-SD.zip';assert sd.is_file()
(out/'downloads').mkdir(exist_ok=True);shutil.copyfile(sd,out/'downloads/PocketDD-SD.zip')
for body in range(2):shutil.copyfile(ROOT/f'docs/design/classes/catalog-{body}.png',out/f'assets/classi-catalog-{body}.png')
class_cards=[]
for outfit in range(1,13):
 name=json.loads((ROOT/'src/avatar-classes.json').read_text())[outfit]['name']
 for body,label in enumerate(['Maschile','Femminile']):
  filename=f'class-home-00-{body}-{outfit:02d}.png';source=ROOT/'firmware/tests/out/ui-build'/filename
  assert source.is_file(),filename;shutil.copyfile(source,out/'assets'/filename)
  class_cards.append(f'<figure><img loading="lazy" src="assets/{filename}" width="466" height="466" alt="{name}, {label.lower()}, nella home del firmware"><figcaption>{name} · {label}</figcaption></figure>')
page=out/'personalizzazione.html'
page.write_text(page.read_text().replace('</main>', '<section id="classi" data-class-runtime><h2>Preset delle classi · disponibili nella v0.70</h2><p>504 look: dodici classi per tutte le 21 specie e per entrambe le silhouette. Cappucci e capelli sono disegnati insieme. Solo aspetto: nessun oggetto equipaggiato e nessun valore di gioco cambia.</p><p><a href="downloads/PocketDD-SD.zip" download>Scarica il pacchetto SD</a>. Copia la cartella pocketdd nella radice della SD FAT32. Sul dispositivo: Aspetto → Modifica → Classe, Anteprima, Conferma. Senza asset resta lo stile originale. Preset con colori e taglio propri; scelte originali conservate. Un fotogramma neutro per look; movimento e feedback della scena restano attivi.</p><p>Catture del renderer del firmware su PC, con file SD letto dal disco host: non sono fotografie né prove del lettore SD fisico.</p><div class="species-grid">'+''.join(class_cards)+'</div><h3>Tutte le combinazioni sulla griglia degli sprite</h3><p>Cataloghi normalizzati: non sono catture del dispositivo.</p><a href="assets/classi-catalog-0.png">Catalogo maschile</a> · <a href="assets/classi-catalog-1.png">Catalogo femminile</a></section></main>'))

# Import workflow captures from the native UI; synthetic data, no USB hardware claim.
for name in ('beyond-defences-import.png','beyond-defences-device.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing Beyond capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Inventario e difese · v0.71</h2><p>Cariche separate dalle quantità, cure con formula esplicita e difese incondizionate riconciliate da Beyond. Gli altri tipi restano da verificare. Il profilo difese richiede firmware 0.71 / USB14; la conversione resta parziale.</p><p>Catture del firmware eseguito su PC con dati sintetici: anteprima prima della conferma e difese ricevute. Non attestano il trasferimento USB fisico.</p><div class="import-captures"><img src="assets/beyond-defences-import.png" width="466" height="466" alt="Anteprima importazione con due tipi verificati"><img src="assets/beyond-defences-device.png" width="466" height="466" alt="Difese ricevute nella scheda del dispositivo"></div></section></main>'))

shutil.copyfile(ROOT/'docs/STATO-SOFTWARE.md',out/'downloads/PocketDD-Stato.md')

# Actual production renderer, synthetic ability association; no physical USB claim.
for name in ('skill-ability-list.png','skill-ability-detail.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing skill capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>La caratteristica giusta per il tuo tiro · v0.74</h2><p>Abilità con caratteristica personalizzata da Beyond o dall’editor PC fino al dispositivo. Per trasferire questa scelta occorre firmware 0.74. Competenza e bonus restano separati.</p><p>Intimidire con FOR: catture del renderer del firmware eseguito su PC con dati sintetici, non fotografie o prove USB fisiche.</p><div class="import-captures"><img src="assets/skill-ability-list.png" width="466" height="466" alt="Elenco abilità con il bonus personalizzato"><img src="assets/skill-ability-detail.png" width="466" height="466" alt="Intimidire con FOR più quattro, competenza più due e vantaggio FOR"></div></section></main>'))

# Explicit defence setup; actual native renderer, synthetic character only.
for name in ('defence-setup-preview.png','defence-setup-complete.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing defence setup capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Difese pronte per il tavolo · v0.75</h2><p>Danni → Difese per tipo → Completa: controlla i tipi mancanti e confermali Normali di base solo dopo verifica. Difese note, regole ed extra restano conservati. Configurazione rapida disponibile anche sul PC; nessuna deduzione automatica da Beyond.</p><p>Catture del renderer del firmware su PC con dati sintetici. Il trasferimento USB sul dispositivo resta da verificare.</p><div class="import-captures"><img src="assets/defence-setup-preview.png" width="466" height="466" alt="Anteprima dei soli tipi da verificare"><img src="assets/defence-setup-complete.png" width="466" height="466" alt="Profilo completato con difese precedenti conservate"></div></section></main>'))

# Manual counters are rendered by the existing native resources view; synthetic source.
for name in ('manual-counter-first.png','manual-counter-note.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing manual counter capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Contatori e capacità distinti · v0.76</h2><p>I saldi manuali Beyond arrivano nei contatori del dispositivo, separati dagli utilizzi delle capacità. Correzioni esplicite, nessun consumo o recupero automatico. Testi negli estratti attuali.</p><p>Catture del firmware su PC con dati sintetici. Nuova lettura reale verificata separatamente nel report; trasferimento USB fisico ancora da collaudare.</p><div class="import-captures"><img src="assets/manual-counter-first.png" width="466" height="466" alt="Saldo manuale separato dalle capacità"><img src="assets/manual-counter-note.png" width="466" height="466" alt="Capacità informativa senza utilizzi inventati"></div></section></main>'))

# Real native renderer captures with synthetic data; source verification is documented separately.
for name in ('cannon-free.png','cannon-paid-confirm.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing cannon capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Creazione del cannone · v0.77</h2><p>Per l’Artillerist riconosciuto di Tasha’s: una creazione gratuita per riposo lungo, poi uno slot ordinario o del patto scelto e confermato. Contatori manuali e concentrazione conservati. Aggiorna app e firmware a 0.77; una nuova importazione aggiunge il nuovo comportamento conservando le schede precedenti.</p><p>Catture del renderer del firmware su PC con dati sintetici. La nuova lettura reale e i limiti sono distinti nel report; USB fisico ancora da verificare.</p><div class="import-captures"><img src="assets/cannon-free.png" width="466" height="466" alt="Creazione gratuita del cannone"><img src="assets/cannon-paid-confirm.png" width="466" height="466" alt="Conferma della creazione pagata con uno slot del patto"></div></section></main>'))

# Native renderer, synthetic SSID/password only. Never copy runtime Wi-Fi credentials.
for name in ('reference-list.png','reference-initiative.png','wifi-provisioning.png','wifi-connected.png'):
 source=ROOT/'firmware/tests/out/ui-build'/name
 if not source.is_file():raise SystemExit(f'Missing v0.78 capture: {name}')
 shutil.copyfile(source,out/'assets'/name)
for name in ('BEYOND-V1','WIFI','BEYOND-TEMPO-REALE','PONTE-BEYOND','PONTE-BEYOND-PROVE'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
(out/'downloads/previews').mkdir(exist_ok=True)
shutil.copyfile(ROOT/'docs/previews/v078-reference-wifi.png',out/'downloads/previews/v078-reference-wifi.png')
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Profilo e Wi-Fi · v0.78</h2><p>Scheda → Profilo PG: movimento, sensi/passivi, lingue, strumenti, dotazioni e iniziativa riconciliata. Le fotografie e i valori da verificare sono distinti dai comandi operativi. Reimporta da PC con app e firmware aggiornati; le altre schede restano conservate.</p><p>Firmware 0.84: Menu → ingranaggio → Wi-Fi → Cerca reti, scegli la rete e inserisci la password sul display. Configura da telefono / PC mantiene il portale temporaneo come alternativa e per abbinare il ponte. Le credenziali restano fuori dai backup del PG. La v0.79 aggiunge il ponte sperimentale descritto sotto.</p><p><a href="downloads/PocketDD-BEYOND-V1.md">Ambito Beyond v1</a> · <a href="downloads/PocketDD-WIFI.md">Guida Wi-Fi</a> · <a href="downloads/PocketDD-BEYOND-TEMPO-REALE.md">Progetto tempo reale</a></p><p>Queste catture provengono dal firmware eseguito su PC con dati di prova e password fittizia. Non attestano USB o Wi-Fi fisici, ancora da collaudare.</p><div class="import-captures"><img src="assets/reference-list.png" width="466" height="466" alt="Profilo del PG sul dispositivo"><img src="assets/reference-initiative.png" width="466" height="466" alt="Iniziativa con bonus riconciliato"><img src="assets/wifi-provisioning.png" width="466" height="466" alt="Configurazione Wi-Fi con dati esclusivamente sintetici"><img src="assets/wifi-connected.png" width="466" height="466" alt="Stato connesso simulato"></div></section></main>'))

# Experimental bridge: software evidence only, no authenticated account/hardware claim.
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Ponte Beyond sperimentale · v0.79</h2><p>Pocket → Wi-Fi → HTTPS sul PC → estensione Chrome/Edge → pagina Beyond. Stesse facce e totale dei tiri salvati; PF attuali e temporanei con verifica, conflitti visibili e nessun reinvio automatico dopo un esito incerto. Nessun requisito Foundry.</p><p>Nel pacchetto PC trovi Avvia-Ponte.cmd e la cartella extension. <a href="downloads/PocketDD-PONTE-BEYOND.md">Leggi la guida di abbinamento e i limiti</a>. I controlli software usano dati e pagine sintetiche: account Beyond, persistenza remota e radio ESP32 restano da collaudare. L utente riferisce risolti USB, installer e SD; le nuove funzioni e il ponte richiedono collaudo separato.</p></section></main>'))

shutil.copyfile(ROOT/'docs/previews/v079-bridge.png',out/'downloads/previews/v079-bridge.png')

# Actual native renderer with synthetic data. No USB/touch/radio hardware claim.
shutil.copyfile(ROOT/'docs/USO-PRATICO.md',out/'downloads/PocketDD-USO-PRATICO.md')
shutil.copyfile(ROOT/'docs/previews/v080-practical-device.png',out/'downloads/previews/v080-practical-device.png')
for name in ('ux080-home','ux080-vitals','ux080-magic','ux080-ready'):
 source=ROOT/'firmware/tests/out/ui-build'/f'{name}.png'
 if not source.is_file():raise SystemExit(f'Missing v0.80 native capture: {name}')
 shutil.copyfile(source,out/'assets'/f'{name}.png')
page=out/'index.html'
page.write_text(page.read_text().replace('</main>', '<section class="after"><h2>Più gioco, meno passaggi · v0.80</h2><p>Arma e magie direttamente dalla home; Danni e Cura in prima riga. Icone coerenti con aiuto su pressione lunga, filtri Consumabili e Pronte, swipe nelle liste. I tasti configurabili e le conferme restano disponibili. Il PC segue tre passaggi: importa, verifica e trasferisci, conserva le copie.</p><p><a href="downloads/PocketDD-USO-PRATICO.md">Guida rapida al dispositivo</a>. Catture del firmware su PC con dati sintetici: non fotografie o prove fisiche. Il collaudo del ponte reale resta separato; USB, installer e SD sono riferiti risolti dall utente.</p><div class="import-captures"><img src="assets/ux080-home.png" width="466" height="466" alt="Home con arma e magie dirette"><img src="assets/ux080-vitals.png" width="466" height="466" alt="Danni e cura prima delle correzioni manuali"><img src="assets/ux080-magic.png" width="466" height="466" alt="Lista magie con ricerca e filtri"><img src="assets/ux080-ready.png" width="466" height="466" alt="Filtro delle magie e capacità con risorse disponibili"></div></section></main>'))

shutil.copyfile(ROOT/'docs/INIZIA-QUI.md',out/'downloads/PocketDD-INIZIA-QUI.md')

shutil.copyfile(ROOT/'docs/previews/v081-setup-flow.png',out/'downloads/previews/v081-setup-flow.png')

shutil.copyfile(ROOT/'docs/SESSIONE-V082.md',out/'downloads/SESSIONE-V082.md')
shutil.copyfile(ROOT/'docs/previews/v082-key-picker.png',out/'downloads/previews/v082-key-picker.png')
shutil.copyfile(ROOT/'docs/BEYOND-CONSENSO-V0821.md',out/'downloads/BEYOND-CONSENSO-V0821.md')
shutil.copyfile(ROOT/'public/assets/classes.pdc',out/'assets/classes.pdc')

shutil.copyfile(ROOT/'docs/SD-WEB-USB.md',out/'downloads/PocketDD-SD-WEB-USB.md')
shutil.copyfile(ROOT/'docs/previews/v083-sd-install.png',out/'downloads/previews/v083-sd-install.png')

shutil.copyfile(ROOT/'docs/previews/v084-wifi-direct.png',out/'downloads/previews/v084-wifi-direct.png')

shutil.copyfile(ROOT/'docs/GITHUB-PAGES.md',out/'downloads/PocketDD-GITHUB-PAGES.md')

# New device session guide and screenshots: actual LVGL renderer, synthetic input.
for name in ('EQUIPAGGIAMENTO-MAGIE','STATO-SOFTWARE'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
shutil.copyfile(ROOT/'docs/previews/v085-device-session.png',out/'downloads/previews/v085-device-session.png')
for name in ('ux085-true-strike','ux085-true-strike-damage','ux085-armor-off'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>Magie e oggetti pronti sul Pocket · v0.85</h2><p>Prepara una magia direttamente dalla scheda. Equipaggia o riponi armature e oggetti con anteprima di CA, caratteristiche, PF massimi e tiri collegati. Configura le armi già importate dal dispositivo.</p><p>True Strike 2024 usa la caratteristica da incantatore e tira arma più dadi radianti, anche su critico. Icone riconoscibili condivise e disponibilità visibile nelle liste. Gli effetti speciali non rappresentati restano manuali.</p><p><a href="downloads/PocketDD-EQUIPAGGIAMENTO-MAGIE.md">Guida equipaggiamento e True Strike</a>. Aggiorna il firmware conservando il PG; per le nuove importazioni USB18 aggiorna anche l’app PC. Le schede esistenti si possono configurare sul Pocket senza reimportazione.</p><p>Catture del firmware su PC con dati sintetici. Lettura Beyond reale verificata separatamente; nuove interazioni da confermare sul dispositivo fisico.</p><div class="import-captures"><img src="assets/ux085-true-strike.png" width="466" height="466" alt="Attacco con True Strike e CAR"><img src="assets/ux085-true-strike-damage.png" width="466" height="466" alt="Danni arma ed extra radiante con totale comune"><img src="assets/ux085-armor-off.png" width="466" height="466" alt="Anteprima CA quando si ripone l armatura"></div></section>'
page=out/'index.html'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))

# Read-only account feasibility helper: explicit archive contents, no profile/session files.
# This is separate from firmware and does not require a device update.
import zipfile
extension_names=('manifest.json','background.js','content.js','protocol.js','page-adapter.js',
                 'popup.html','popup.css','popup.js','account.html','account.css','account.js','account-check.js')
with zipfile.ZipFile(out/'downloads/PocketDD-Beyond-Check.zip','w',zipfile.ZIP_DEFLATED) as archive:
 for name in extension_names:archive.write(ROOT/'extension'/name,'extension/'+name)
 archive.write(ROOT/'docs/POCKET-AUTONOMO.md','LEGGIMI.md')
for name in ('POCKET-AUTONOMO',):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
account_section='<details class="after"><summary>Verso il Pocket autonomo · verifica account Beyond</summary><p>Controllo sperimentale nell’estensione: login ed elenco dei PG, senza esportare credenziali o modificare schede. Non è ancora importazione Wi-Fi dal dispositivo; non serve aggiornare il firmware per questa prova.</p><p><a href="downloads/PocketDD-Beyond-Check.zip" download>Scarica solo l’estensione di verifica</a> · <a href="downloads/PocketDD-POCKET-AUTONOMO.md">Istruzioni e stato dei lavori</a>. Confronta l’elenco con quello del tuo account; un HTTP riuscito non prova la conversione.</p></details>'
page.write_text(page.read_text().replace('<section class="after">',account_section+'<section class="after">',1))

# Generic equipment/attunement UI, not a particular character build.
for name in ('OGGETTI-SINTONIA','EQUIPAGGIAMENTO-MAGIE'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
shutil.copyfile(ROOT/'docs/previews/v086-attunement.png',out/'downloads/previews/v086-attunement.png')
for name in ('ux086-attunement-preview','ux086-attuned','ux086-equipment-icons'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>Oggetti e sintonia per tutti i PG · v0.86</h2><p>Equipaggiato e sintonizzato sono stati distinti. Anteprima dei bonus a CA, caratteristiche, PF, armi, magie e tiri; modifiche salvate insieme, senza duplicare gli effetti importati. Profili permanenti riconciliati anche sui personaggi senza armatura.</p><p>Icone dedicate per anelli, mantelli, bastoni, stivali e amuleti. Requisiti di sintonia, effetti condizionali, resistenze e altre proprietà non rappresentate restano manuali.</p><p><a href="downloads/PocketDD-OGGETTI-SINTONIA.md">Guida agli oggetti e alla sintonia</a>. Aggiorna il firmware conservando il PG; per importare i nuovi profili di sintonia aggiorna anche l app PC. Le schede già presenti si possono configurare sul Pocket.</p><p>Catture del firmware su PC con oggetti sintetici. Prove su tutte le caratteristiche e livelli 1–20; nessun nuovo collaudo USB fisico implicito.</p><div class="import-captures"><img src="assets/ux086-attunement-preview.png" width="466" height="466" alt="Anteprima degli effetti della sintonia"><img src="assets/ux086-attuned.png" width="466" height="466" alt="Stato equipaggiato e sintonizzato"><img src="assets/ux086-equipment-icons.png" width="466" height="466" alt="Icone riconoscibili di mantello, anello e stivali"></div></section>'
page=out/'index.html'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))

# Direct attack variants: firmware UI, synthetic characters, no source JSON.
shutil.copyfile(ROOT/'docs/ATTACCHI-CONTESTUALI.md',out/'downloads/PocketDD-ATTACCHI-CONTESTUALI.md')
shutil.copyfile(ROOT/'docs/ATTACCHI-CONTESTUALI.md',out/'downloads/ATTACCHI-CONTESTUALI.md')
shutil.copyfile(ROOT/'docs/previews/v087-contextual-attacks.png',out/'downloads/previews/v087-contextual-attacks.png')
for name in ('ux087-attack-choices','ux087-weapon-spell','ux087-raging'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>Attacco normale o magia, subito · v0.87</h2><p>Attacca apre la scelta fra attacco normale e varianti con arma: icone, caratteristica, bonus e danni visibili. Se non ci sono varianti, si va direttamente al tiro. Preparazione nel flusso e motivi dei blocchi visibili; si cambia modalità anche dal pulsante Varianti.</p><p>True Strike 2024 usa la caratteristica della sua origine, per tutti i PG. Le altre magie e proprietà non rappresentate restano manuali: nessun effetto assegnato dal solo nome. Il menu non tira dadi né consuma slot; cambiare modalità richiede un nuovo tiro.</p><p><a href="downloads/PocketDD-ATTACCHI-CONTESTUALI.md">Guida agli attacchi contestuali</a>. Basta aggiornare il firmware: scheda, tasti e SD conservati, nessuna reimportazione. Prove software e catture native con dati sintetici; nuove interazioni da provare sul Pocket fisico.</p><div class="import-captures"><img src="assets/ux087-attack-choices.png" width="466" height="466" alt="Scelta diretta fra attacco normale e magia con valori"><img src="assets/ux087-weapon-spell.png" width="466" height="466" alt="Attacco con arma e caratteristica da incantatore"><img src="assets/ux087-raging.png" width="466" height="466" alt="Motivo visibile per le varianti non disponibili"></div></section>'
page=out/'index.html'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))

# On-hit concentration: native production UI, synthetic characters only.
for name in ('EFFETTI-SUL-COLPO','ATTACCHI-CONTESTUALI'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
shutil.copyfile(ROOT/'docs/previews/v088-on-hit-effects.png',out/'downloads/previews/v088-on-hit-effects.png')
for name in ('ux088-context-spells','ux088-target','ux088-necrotic'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>Hex e Marchio sul colpo · v0.88</h2><p>Lancia gli effetti 2024 dalla scelta dell attacco, con slot ordinari o patto e concentrazione. Dopo un colpo riuscito confermi se hai colpito il loro bersaglio: extra necrotico o forza, anche su critico e insieme a True Strike.</p><p>Totale comune e tre parti distinte nello storico, con riprova senza rilanciare. Icone condivise e profili espliciti per le schede già presenti. Durata, azioni, bersagli, potenziamenti e altre magie restano da gestire al tavolo.</p><p><a href="downloads/PocketDD-EFFETTI-SUL-COLPO.md">Guida agli effetti sul colpo</a>. Per import e backup USB aggiorna anche l’app PC alla 0.88. Aggiorna il firmware conservando PG e SD; sui PG già presenti puoi configurare il profilo dal Pocket. Nuove regole provate con dati sintetici, nuove interazioni da collaudare sul dispositivo fisico.</p><div class="import-captures"><img src="assets/ux088-context-spells.png" width="466" height="466" alt="Magie pertinenti dalla scelta dell attacco"><img src="assets/ux088-target.png" width="466" height="466" alt="Conferma esplicita del bersaglio prima dei danni"><img src="assets/ux088-necrotic.png" width="466" height="466" alt="Extra necrotico con risultato congelato e totale comune"></div></section>'
page=out/'index.html'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))

# 2024 Smite: actual firmware captures and explicit manual/2014 boundaries.
for name in ('PUNIZIONE-DIVINA','EFFETTI-SUL-COLPO','ATTACCHI-CONTESTUALI','BEYOND-V1','USO-PRATICO'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
shutil.copyfile(ROOT/'docs/previews/v089-divine-smite.png',out/'downloads/previews/v089-divine-smite.png')
for name in ('ux089-smite-combo','ux089-save-retry','ux089-large-smite'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>Punizione Divina dopo il colpo · v0.89</h2><p>Scegli slot ordinario, patto o un uso finito realmente presente. Bersaglio, critico e combinazione con True Strike e Hex/Marchio; costo applicato una volta, risultati congelati nelle riprove, storico fino a cinque parti.</p><p>Corretta anche la riconciliazione di slot ordinari e patto da tabelle non ambigue. Lettura reale e parser/codec C concordano sui valori; prove dei tiri con personaggi sintetici, nuove interazioni da verificare sul Pocket fisico.</p><p><a href="downloads/PocketDD-PUNIZIONE-DIVINA.md">Guida Punizione 2024 e slot</a>. Aggiorna firmware e app PC per USB. Le vecchie schede e la SD sono conservate; i vecchi slot irrisolti richiedono una nuova importazione con il PC aggiornato. Azioni del turno, proprietà speciali e capacità 2014 restano manuali.</p><div class="import-captures"><img src="assets/ux089-smite-combo.png" width="466" height="466" alt="Scelta di slot, bersaglio ed extra con Punizione"><img src="assets/ux089-save-retry.png" width="466" height="466" alt="Errore di salvataggio con riprova senza nuovi dadi"><img src="assets/ux089-large-smite.png" width="466" height="466" alt="Critico con secondo gruppo di d8 e totale comune"></div></section>'
page=out/'index.html'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))

# Device-local collection; actual LVGL captures with synthetic PGs only.
for name in ('LIBRERIA-LOCALE','POCKET-AUTONOMO','STATO-SOFTWARE'):
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'PocketDD-{name}.md')
 shutil.copyfile(ROOT/'docs'/f'{name}.md',out/'downloads'/f'{name}.md')
shutil.copyfile(ROOT/'docs/previews/v090-local-library.png',out/'downloads/previews/v090-local-library.png')
for name in ('ux090-library','ux090-open-pg','ux090-no-sd'):
 shutil.copyfile(ROOT/'firmware/tests/out/ui-build'/f'{name}.png',out/'assets'/f'{name}.png')
section='<section class="after"><h2>I miei PG sul Pocket · v0.90</h2><p>Fino a 32 copie native sulla SD, anteprima e cambio offline. PF, slot, aspetto e storico restano separati; tasti e preferenze del dispositivo sono conservati. Prima dei cambi e delle nuove importazioni USB si salva la scheda uscente.</p><p>Elenco account Beyond completo confermato dall’utente. La raccolta locale è verificata con prove software; il nuovo flusso SD fisico è ancora da collaudare. Conversione nativa e importazione Wi-Fi dal Pocket restano successive.</p><p><a href="downloads/PocketDD-LIBRERIA-LOCALE.md">Guida I miei PG e prova sul dispositivo</a>. Aggiorna conservando i dati; non serve reinstallare i look. Senza SD resta un PG attivo. I PG già sostituiti prima dell’aggiornamento restano nelle copie PC esistenti.</p><div class="import-captures"><img src="assets/ux090-library.png" width="466" height="466" alt="Raccolta locale con due personaggi sintetici"><img src="assets/ux090-open-pg.png" width="466" height="466" alt="Anteprima del PG prima di aprirlo"></div></section>'
page.write_text(page.read_text().replace('<section class="after">',section+'<section class="after">',1))
