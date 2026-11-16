#!/usr/bin/env python3
"""Test staged GitHub Pages under its project prefix; no real USB or Beyond."""
import hashlib,http.server,importlib.util,json,os,subprocess,tempfile,threading,zipfile
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1];site=ROOT.parent/'PocketDD-GitHub-Pages'
spec=importlib.util.spec_from_file_location('publisher',ROOT/'scripts/publish-pages.py');publisher=importlib.util.module_from_spec(spec);spec.loader.exec_module(publisher)
base=ROOT/'dist/install';m=json.loads((base/'manifest.json').read_text())
publisher.validate(base,m['commit'],m['version'])
# Fail before pushing on accidental private JSON or a damaged firmware region.
with tempfile.TemporaryDirectory(prefix='pdd-pages-bad-') as tmp:
 clone=Path(tmp)/'site';publisher.shutil.copytree(base,clone,copy_function=publisher.shutil.copyfile)
 secret=clone/'downloads/character.json';secret.write_text('{"synthetic":"must not publish"}')
 try:publisher.validate(clone,m['commit'],m['version']);raise AssertionError('unexpected JSON accepted')
 except ValueError as e:assert 'Unexpected file' in str(e)
 secret.unlink();part=clone/'firmware/app.bin';part.unlink();part.write_bytes(b'corrupt')
 try:publisher.validate(clone,m['commit'],m['version']);raise AssertionError('bad firmware accepted')
 except ValueError as e:assert 'integrity' in str(e)
 # The new standalone helper must not allow arbitrary files inside a whitelisted ZIP.
 publisher.shutil.copyfile(base/'firmware/app.bin',part)
 with zipfile.ZipFile(clone/'downloads/PocketDD-Beyond-Check.zip','a') as z:z.writestr('profile/cookies.json','synthetic forbidden file')
 try:publisher.validate(clone,m['commit'],m['version']);raise AssertionError('unexpected extension entry accepted')
 except ValueError as e:assert 'archive contents' in str(e)
assert not (site/'downloads/PocketDD-PC.zip').exists()
assert (site/'.nojekyll').exists()
receipt=json.loads((site/'deployment.json').read_text());assert receipt['source_commit']==m['commit']
for name,digest in receipt['files_sha256'].items():assert hashlib.sha256((site/name).read_bytes()).hexdigest()==digest
class Handler(http.server.BaseHTTPRequestHandler):
 def log_message(self,*args):pass
 def do_GET(self):
  from urllib.parse import urlsplit,unquote
  name=unquote(urlsplit(self.path).path)
  if not name.startswith('/PocketD-D/'):self.send_error(404);return
  relative=name[len('/PocketD-D/'): ] or 'index.html';path=site/relative
  if not path.resolve().is_relative_to(site.resolve()) or not path.is_file():self.send_error(404);return
  kind={'.html':'text/html','.js':'text/javascript','.css':'text/css','.png':'image/png','.json':'application/json','.md':'text/markdown','.zip':'application/zip'}.get(path.suffix,'application/octet-stream')
  data=path.read_bytes();self.send_response(200);self.send_header('Content-Type',kind);self.send_header('Content-Length',str(len(data)));self.end_headers()
  try:self.wfile.write(data)
  except (BrokenPipeError,ConnectionResetError):pass
server=http.server.ThreadingHTTPServer(('127.0.0.1',0),Handler);thread=threading.Thread(target=server.serve_forever,daemon=True);thread.start();url=f'http://127.0.0.1:{server.server_port}/PocketD-D/'
try:
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1100,'height':900},accept_downloads=True)
  context.add_init_script("Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{window.chooserCalls=(window.chooserCalls||0)+1;throw new DOMException('cancel','NotFoundError')}}});")
  context.add_init_script("window.chrome={runtime:{getManifest:()=>({version:'0.116.0'}),sendMessage:(m,reply)=>{window.helperOpens=(window.helperOpens||0)+1;window.lastHelperView=m.view||'tools';reply({ok:true})}}};")
  context.add_init_script((ROOT/'extension/site-helper.js').read_text())
  page=context.new_page();errors=[];requests=[];page.on('pageerror',lambda e:errors.append(str(e)));page.on('request',lambda r:requests.append(r.url));page.goto(url)
  expect(page.locator('[data-open-pocket]')).to_be_visible();assert not any(u.endswith(('.bin','.pdc','.zip')) for u in requests)
  page.locator('[data-open-pocket]').click();expect(page.locator('[data-helper-status]')).to_contain_text('Pocket PC aperto');assert page.evaluate('helperOpens')==1
  page.locator('[data-open-beyond]').click();expect(page.locator('[data-helper-status]')).to_contain_text('Collegamento Beyond aperto');assert page.evaluate('helperOpens')==2 and page.evaluate('lastHelperView')=='beyond'
  # A user-requested reload resumes only the saved fixed view, once; no USB.
  resumed=context.new_page();resumed.goto(url);resumed.evaluate("sessionStorage.setItem('pocketdd-continue-view','tools')");resumed.reload();expect(resumed.locator('[data-helper-status]')).to_contain_text('Pocket PC aperto');assert resumed.evaluate("lastHelperView==='tools'&&sessionStorage.getItem('pocketdd-continue-view')===null");resumed.reload();resumed.wait_for_timeout(200);assert resumed.evaluate('!window.helperOpens');resumed.close()
  page.locator('[data-show-firmware]').click()
  expect(page.locator('#status')).to_contain_text('Firmware pronto');expect(page.locator('#version')).to_have_text('Firmware '+m['version']);expect(page.locator('[value=update]')).to_be_checked()
  expect(page.locator('#install')).to_be_visible();page.locator('.sd-tools>summary').click();expect(page.locator('[data-sd-install]')).to_be_visible();expect(page.locator('[data-sd-install]')).to_be_enabled(timeout=15000)
  expect(page.get_by_role('link',name='Scarica app PC per Windows')).not_to_be_visible();assert page.locator('.firmware-tools>.card').count()==1;assert not any('pc-package' in u for u in requests)
  for width in [1100,390,320]:
   page.set_viewport_size({'width':width,'height':900});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.set_viewport_size({'width':1100,'height':900});page.locator('#board').check();page.locator('#install').click();expect(page.locator('#status')).to_contain_text('annullata');assert page.evaluate('chooserCalls')==1
  page.locator('[data-sd-install]').click();expect(page.locator('[data-sd-status]')).to_contain_text('annullata');assert page.evaluate('chooserCalls')==2
  page.locator('.pc-tools>summary').click()
  with page.expect_download() as download:page.get_by_role('link',name='Scarica app PC per Windows').click()
  assert Path(download.value.path()).read_bytes()==(base/'downloads/PocketDD-PC.zip').read_bytes()
  page.locator('.pc-tools>summary').click();page.screenshot(path='/workspace/PocketDD-GitHub-Pages-preview.png',full_page=True)
  page.locator('.release-history>summary').click()
  page.get_by_text('Verso il Pocket autonomo · verifica account Beyond',exact=True).click()
  with page.expect_download() as helper:page.get_by_role('link',name='Scarica solo l’estensione di verifica').click()
  assert Path(helper.value.path()).read_bytes()==(base/'downloads/PocketDD-Beyond-Check.zip').read_bytes()
  page.goto(url+'personalizzazione.html');page.get_by_role('link',name='← PocketD-D',exact=True).click();assert page.url==url
  assert not errors,errors;assert all(u.startswith(url) or u.startswith('blob:'+url) for u in requests),requests
  page.goto(url+'#firmware-tools');expect(page.locator('.firmware-tools')).to_have_attribute('open','');expect(page.locator('#status')).to_contain_text('Firmware pronto')
  page.reload();expect(page.locator('.firmware-tools')).to_have_attribute('open','');expect(page.locator('#status')).to_contain_text('Firmware pronto')
  missing=browser.new_context();missing.add_init_script("Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{throw new DOMException('cancel','NotFoundError')}}});")
  fallback=missing.new_page();fallback.goto(url+'#beyond')
  expect(fallback.locator('.beyond-route')).to_contain_text('da solo');fallback.locator('[data-open-beyond]').click();expect(fallback.locator('[data-helper-help]')).to_be_visible();expect(fallback.locator('[data-helper-status]')).to_contain_text('Prima volta');assert fallback.evaluate('!window.chooserCalls')
  # Once installation is detected, Continue opens the association directly.
  fallback.add_script_tag(content="window.chrome={runtime:{getManifest:()=>({version:'0.116.0'}),sendMessage:(m,reply)=>{window.lastHelperView=m.view;reply({ok:true})}}};")
  fallback.add_script_tag(content=(ROOT/'extension/site-helper.js').read_text());fallback.locator('[data-helper-reload]').click();expect(fallback.locator('[data-helper-status]')).to_contain_text('Collegamento Beyond aperto');expect(fallback.locator('[data-helper-help]')).not_to_be_visible();assert fallback.evaluate("lastHelperView==='beyond'")
  tools_fallback=missing.new_page();tools_fallback.goto(url);tools_fallback.locator('[data-open-pocket]').click();expect(tools_fallback.locator('[data-helper-help]')).to_be_visible()
  tools_fallback.add_script_tag(content="window.chrome={runtime:{getManifest:()=>({version:'0.116.0'}),sendMessage:(m,reply)=>{window.lastHelperView=m.view;reply({ok:true})}}};")
  tools_fallback.add_script_tag(content=(ROOT/'extension/site-helper.js').read_text());tools_fallback.locator('[data-helper-reload]').click();expect(tools_fallback.locator('[data-helper-status]')).to_contain_text('Pocket PC aperto');assert tools_fallback.evaluate("lastHelperView===undefined");missing.close()
  legacy=browser.new_context();legacy.add_init_script("Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{throw Error('must not open serial')}}});window.legacyOpens=0;window.addEventListener('message',e=>{if(e.data?.type!=='pocketdd-helper-request')return;const d=e.data;if(d.action==='open')window.legacyOpens++;window.postMessage({type:'pocketdd-helper-response',id:d.id,...(d.action==='ping'?{installed:true,version:'0.93.0'}:{opened:true})},location.origin);});")
  old=legacy.new_page();old.goto(url);old.locator('[data-open-beyond]').click();expect(old.locator('[data-helper-status]')).to_contain_text('Aggiorna il componente');expect(old.locator('[data-helper-help]')).to_be_visible();assert old.evaluate('legacyOpens')==0;legacy.close()
  stale=browser.new_context();stale.add_init_script("Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{throw Error('must not open serial')}}});window.chrome={runtime:{getManifest:()=>({version:'0.115.0'}),sendMessage:()=>{throw Error('must update first')}}};");stale.add_init_script((ROOT/'extension/site-helper.js').read_text());old=stale.new_page();old.goto(url);old.locator('[data-open-pocket]').click();expect(old.locator('[data-helper-status]')).to_contain_text('Aggiorna il componente');old.locator('[data-open-beyond]').click();expect(old.locator('[data-helper-status]')).to_contain_text('Aggiorna il componente');stale.close()
  browser.close()
 print('PASS Pages: fixed /PocketD-D/ prefix, firmware/SD without ZIP, safe update default, optional verified PC and account-helper downloads, relative gallery back, mobile, simulated chooser, exact provenance/hashes and rejection of arbitrary JSON/corrupt firmware/extra helper ZIP entries. Does not prove GitHub activation or physical USB.')
finally:server.shutdown();server.server_close();thread.join()
