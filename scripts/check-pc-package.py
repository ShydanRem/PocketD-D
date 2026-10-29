#!/usr/bin/env python3
"""Verify the distributed PC ZIP independently, without a developer checkout or node_modules."""
import json,os,re,subprocess,tempfile,time,urllib.request,urllib.error,zipfile,hashlib,struct
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];archive=ROOT/'dist/install/downloads/PocketDD-PC.zip'
with zipfile.ZipFile(archive) as z:
 assert z.testzip() is None
 names=z.namelist();assert len(names)==len(set(names));assert 'PocketDD-PC/docs/STATO-SOFTWARE.md' in names;assert z.read('PocketDD-PC/docs/STATO-SOFTWARE.md')==(ROOT/'docs/STATO-SOFTWARE.md').read_bytes();assert all(n.startswith('PocketDD-PC/') and '..' not in Path(n).parts for n in names)
 assert not any('/node_modules/' in n or '/device-backups/' in n or '/.git/' in n for n in names)
 with tempfile.TemporaryDirectory(prefix='pocketdd-pc-') as tmp:
  z.extractall(tmp);app=Path(tmp)/'PocketDD-PC';assert (app/'Avvia-PocketDD.cmd').exists();assert not (app/'node_modules').exists()
  for name in ['Avvia-Ponte.cmd','docs/PONTE-BEYOND.md','extension/manifest.json','extension/background.js','extension/protocol.js','tools/bridge/index.html']:
   assert (app/name).read_bytes()==(ROOT/name).read_bytes()
  assert (app/'extension/protocol.js').read_bytes()==(app/'src/sync-protocol.js').read_bytes()
  pin=json.loads((ROOT/'scripts/windows-runtime.json').read_text());node=(app/'runtime/node.exe').read_bytes()
  assert hashlib.sha256(node).hexdigest()==pin['node_sha256'] and node[:2]==b'MZ'
  assert hashlib.sha256((app/'runtime/LICENSE').read_bytes()).hexdigest()==pin['license_sha256']
  assert json.loads((app/'package-info.json').read_text())['firmware_included'] is True
  assert (app/'dist/install/firmware/app.bin').read_bytes()==(ROOT/'firmware/build/pocketdd.bin').read_bytes()
  assert (app/'docs/INIZIA-QUI.md').exists()
  assert not (app/'dist/install/downloads/PocketDD-PC.zip').exists()
  # Exercise the extracted bridge without checkout dependencies, user profile or a remote service.
  bridge=subprocess.Popen(['node','--input-type=module','-e',"import {createBridge} from './scripts/bridge-server.js';const b=createBridge({directory:process.argv[1],ip:'127.0.0.1',test:true,devicePort:0,adminPort:0});await b.listen();console.log('http://127.0.0.1:'+b.admin.address().port);process.on('SIGTERM',async()=>{await b.close();process.exit(0)});",str(Path(tmp)/'private-bridge')],cwd=app,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
  try:
   import select
   assert select.select([bridge.stdout],[],[],10)[0]
   bridge_url=bridge.stdout.readline().strip();assert bridge_url.startswith('http://127.0.0.1:')
   bridge_html=urllib.request.urlopen(bridge_url).read().decode();assert 'CSRF_PLACEHOLDER' not in bridge_html and '<title>Ponte PocketD-D</title>' in bridge_html
   for asset in ['/bridge.js','/bridge.css']:assert urllib.request.urlopen(bridge_url+asset).status==200
  finally:bridge.terminate();bridge.wait(timeout=5)
  server=subprocess.Popen(['node','--use-env-proxy','scripts/start-pc.js'],cwd=app,env={**os.environ,'POCKETDD_APP_PORT':'8797','POCKETDD_NO_OPEN':'1'},stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
  try:
   base='http://127.0.0.1:8797'
   for _ in range(60):
    try:html=urllib.request.urlopen(base,timeout=.5).read().decode();break
    except Exception:
     if server.poll() is not None:raise RuntimeError('Packaged app did not start')
     time.sleep(.1)
   else:raise RuntimeError('Packaged app not ready')
   for asset in re.findall(r'(?:src|href)="(/assets/[^\"]+)"',html):assert urllib.request.urlopen(base+asset).status==200
   setup=json.loads(urllib.request.urlopen(base+'/api/setup').read());assert setup['kind']=='pocketdd-setup' and setup['installer']
   assert urllib.request.urlopen(base+'/?setup=1').status==200
   assert urllib.request.urlopen(base+'/install/?guided=1').status==200
   assert urllib.request.urlopen(base+'/install/firmware/app.bin').read()==(ROOT/'firmware/build/pocketdd.bin').read_bytes()
   second=subprocess.run(['node','--use-env-proxy','scripts/start-pc.js'],cwd=app,env={**os.environ,'POCKETDD_APP_PORT':'8797','POCKETDD_NO_OPEN':'1'},capture_output=True,text=True,timeout=10);assert second.returncode==0 and 'già aperto' in second.stdout
   assert urllib.request.urlopen(base+'/assets/classes.pdc').read()==(ROOT/'public/assets/classes.pdc').read_bytes()
   assert urllib.request.urlopen(base+'/assets/PocketDD-SD.zip').read()==(ROOT/'public/assets/PocketDD-SD.zip').read_bytes()
   req=urllib.request.Request(base+'/api/probe',data=json.dumps({'character':'not-a-character'}).encode(),headers={'Content-Type':'application/json'})
   try:urllib.request.urlopen(req);raise AssertionError('Invalid Beyond URL accepted')
   except urllib.error.HTTPError as e:assert e.code==400
  finally:server.terminate();server.wait(timeout=5)
print('PC ZIP: CRC, safe paths, Windows launchers, exact extension/protocol/guide, extracted HTTPS bridge UI without node_modules, built app/assets and local Beyond connector validation passed; Windows shell, loaded MV3, RF and authenticated Beyond not run')
