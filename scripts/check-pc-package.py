#!/usr/bin/env python3
"""Verify the distributed PC ZIP independently, without a developer checkout or node_modules."""
import json,os,re,subprocess,tempfile,time,urllib.request,urllib.error,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];archive=ROOT/'dist/install/downloads/PocketDD-PC.zip'
with zipfile.ZipFile(archive) as z:
 assert z.testzip() is None
 names=z.namelist();assert all(n.startswith('PocketDD-PC/') and '..' not in Path(n).parts for n in names)
 assert not any('/node_modules/' in n or '/device-backups/' in n or '/.git/' in n for n in names)
 with tempfile.TemporaryDirectory(prefix='pocketdd-pc-') as tmp:
  z.extractall(tmp);app=Path(tmp)/'PocketDD-PC';assert (app/'Avvia-PocketDD.cmd').exists();assert not (app/'node_modules').exists()
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
   req=urllib.request.Request(base+'/api/probe',data=json.dumps({'character':'not-a-character'}).encode(),headers={'Content-Type':'application/json'})
   try:urllib.request.urlopen(req);raise AssertionError('Invalid Beyond URL accepted')
   except urllib.error.HTTPError as e:assert e.code==400
  finally:server.terminate();server.wait(timeout=5)
print('PC ZIP: CRC, safe paths, Windows launcher, extracted app without node_modules, built assets and local Beyond connector validation passed; Windows shell itself not run')
