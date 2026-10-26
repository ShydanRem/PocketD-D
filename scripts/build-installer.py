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
