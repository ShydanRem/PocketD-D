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
  page=context.new_page();errors=[];requests=[];page.on('pageerror',lambda e:errors.append(str(e)));page.on('request',lambda r:requests.append(r.url));page.goto(url)
  expect(page.locator('#status')).to_contain_text('Firmware pronto');expect(page.locator('#version')).to_have_text('Firmware '+m['version']);expect(page.locator('[value=update]')).to_be_checked()
  expect(page.locator('#install')).to_be_visible();expect(page.locator('[data-sd-install]')).to_be_visible();expect(page.locator('[data-sd-install]')).to_be_enabled(timeout=15000)
  expect(page.get_by_role('link',name='Scarica app PC per Windows')).not_to_be_visible();assert page.locator('main>.card').count()==1;assert not any('pc-package' in u for u in requests)
  for width in [1100,390,320]:
   page.set_viewport_size({'width':width,'height':900});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.set_viewport_size({'width':1100,'height':900});page.locator('#board').check();page.locator('#install').click();expect(page.locator('#status')).to_contain_text('annullata');assert page.evaluate('chooserCalls')==1
  page.locator('[data-sd-install]').click();expect(page.locator('[data-sd-status]')).to_contain_text('annullata');assert page.evaluate('chooserCalls')==2
  page.locator('.pc-tools>summary').click()
  with page.expect_download() as download:page.get_by_role('link',name='Scarica app PC per Windows').click()
  assert Path(download.value.path()).read_bytes()==(base/'downloads/PocketDD-PC.zip').read_bytes()
  page.locator('.pc-tools>summary').click();page.screenshot(path='/workspace/PocketDD-GitHub-Pages-preview.png',full_page=True)
  page.get_by_text('Verso il Pocket autonomo · verifica account Beyond',exact=True).click()
  with page.expect_download() as helper:page.get_by_role('link',name='Scarica solo l’estensione di verifica').click()
  assert Path(helper.value.path()).read_bytes()==(base/'downloads/PocketDD-Beyond-Check.zip').read_bytes()
  page.goto(url+'personalizzazione.html');page.get_by_role('link',name='← PocketD-D',exact=True).click();assert page.url==url
  assert not errors,errors;assert all(u.startswith(url) or u.startswith('blob:'+url) for u in requests),requests
  browser.close()
 print('PASS Pages: fixed /PocketD-D/ prefix, firmware/SD without ZIP, safe update default, optional verified PC and account-helper downloads, relative gallery back, mobile, simulated chooser, exact provenance/hashes and rejection of arbitrary JSON/corrupt firmware/extra helper ZIP entries. Does not prove GitHub activation or physical USB.')
finally:server.shutdown();server.server_close();thread.join()
