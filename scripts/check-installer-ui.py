#!/usr/bin/env python3
"""Browser checks of the compiled installer. USB chooser is simulated; no hardware writes."""
import os,subprocess,time,urllib.request
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1];env={**os.environ,'POCKETDD_INSTALLER_PORT':'8791'}
server=subprocess.Popen(['node','scripts/installer-server.js'],cwd=ROOT,env=env,stdout=subprocess.DEVNULL)
url='http://127.0.0.1:8791/'
try:
 for _ in range(60):
  try:urllib.request.urlopen(url,timeout=.5);break
  except Exception:time.sleep(.1)
 else:raise RuntimeError('Installer server did not start')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
  context=browser.new_context(viewport={'width':1100,'height':1000});errors=[]
  context.add_init_script("Object.defineProperty(navigator, 'serial', {value: {requestPort: async () => {window.chooserCalls=(window.chooserCalls||0)+1;throw new DOMException('No port selected','NotFoundError')}}});")
  page=context.new_page();page.on('pageerror',lambda e:errors.append(str(e)));requests=[];page.on('request',lambda req:requests.append(req.url));page.goto(url)
  expect(page.locator('#status')).to_contain_text('Firmware pronto');expect(page.locator('#install')).to_be_disabled();page.locator('#board').check();expect(page.locator('#install')).to_be_enabled()
  with page.expect_download() as download:
   page.get_by_role('link',name='Scarica app PC per Windows').click()
  artifact=download.value;assert artifact.suggested_filename=='PocketDD-PC.zip'
  assert Path(artifact.path()).read_bytes()==(ROOT/'dist/install/downloads/PocketDD-PC.zip').read_bytes()
  page.screenshot(path='/workspace/PocketD-D-installer-preview.png',full_page=True)
  page.locator('input[value=update]').check();page.locator('#install').click();expect(page.locator('#status')).to_contain_text('annullata');expect(page.locator('#install')).to_be_enabled();assert page.evaluate('window.chooserCalls')==1
  assert not errors,errors;assert all(u.startswith(url) for u in requests),requests
  page.set_viewport_size({'width':390,'height':844});page.screenshot(path='/workspace/PocketD-D-installer-mobile.png',full_page=True);assert page.evaluate('document.documentElement.scrollWidth <= innerWidth')
  # Corrupt one download: no install is possible, even after board confirmation.
  page.route('**/firmware/app.bin',lambda route:route.fulfill(body=b'corrupt',content_type='application/octet-stream'));page.reload();expect(page.locator('#status')).to_contain_text('incompleto o alterato');page.locator('#board').check();expect(page.locator('#install')).to_be_disabled();expect(page.locator('#retry')).to_be_visible()
  page.unroute('**/firmware/app.bin');page.locator('#retry').click();expect(page.locator('#status')).to_contain_text('Firmware pronto');expect(page.locator('#install')).to_be_enabled()
  # An unsupported browser never offers an actionable install button.
  unsupported=browser.new_context();unsupported.add_init_script('delete Navigator.prototype.serial');other=unsupported.new_page();other.goto(url);expect(other.locator('#status')).to_contain_text('Chrome o Edge');other.locator('#board').check();expect(other.locator('#install')).to_be_disabled()
  preview=context.new_page();preview.goto(url+'personalizzazione.html');expect(preview.locator('h1')).to_have_text('Il tuo eroe, il tuo ambiente');assert preview.locator('img').first.evaluate('(e)=>e.complete&&e.naturalWidth===1470');assert preview.evaluate('document.documentElement.scrollWidth<=innerWidth');assert preview.locator('body').evaluate('(e)=>getComputedStyle(e).backgroundColor')=='rgb(7, 21, 30)'
  expect(preview.locator('.species-grid figure')).to_have_count(42)
  for width in [1100,390,320]:
   preview.set_viewport_size({'width':width,'height':844});assert preview.evaluate('document.documentElement.scrollWidth<=innerWidth')
  expect(preview.locator('[data-class-concept]')).to_contain_text('non sono ancora attivabili');assert preview.locator('[data-class-concept] img').evaluate('(e)=>e.complete&&e.naturalWidth>1000')
  # The real production library is bundled. This deliberately does not test USB flashing.
  browser.close()
 print('Installer browser: exact PC ZIP download, local assets/no CDN, SHA256-ready state, model gate, chooser cancel/retry, corrupt download/retry, unsupported browser and mobile layout passed; USB/hardware not tested')
finally:server.terminate();server.wait(timeout=5)
