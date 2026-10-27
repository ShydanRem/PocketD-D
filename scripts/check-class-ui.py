#!/usr/bin/env python3
"""Synthetic PC class editor/backup/export QA. Never opens the user's profile or Beyond."""
import json,os,subprocess,time,urllib.request
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
seed=subprocess.check_output(['node','--input-type=module','-e',"import{initialState}from'./src/model.js';import{createLibrary,addCharacter,serializeLibrary}from'./src/library.js';const first={...initialState(),name:'Scheda conservata',hp:7},second={...initialState(),name:'Prova classi',hp:11,avatar:{skin:2,hair:1,cloak:2,style:1,lineage:12,body:1}};process.stdout.write(serializeLibrary(addCharacter(createLibrary(first,'first'),second,'second')));"],cwd=ROOT,text=True)
server=subprocess.Popen(['node','scripts/app-server.js'],cwd=ROOT,env={**os.environ,'POCKETDD_APP_PORT':'8795'},stdout=subprocess.DEVNULL)
url='http://127.0.0.1:8795/'
try:
 for _ in range(60):
  try:urllib.request.urlopen(url,timeout=.5);break
  except Exception:time.sleep(.1)
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1050},accept_downloads=True)
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(url);page.locator('[data-action=edit]').wait_for()
  page.evaluate('(s)=>localStorage.setItem("pocketdd-library-v1",s)',seed);page.reload();page.locator('[data-action=edit]').click();dialog=page.locator('.hero-editor-dialog');expect(dialog.locator('[name=outfit]')).to_have_value('0')
  original=dialog.locator('.editor-preview svg').get_attribute('viewBox');dialog.locator('[name=outfit]').select_option('9');expect(dialog.locator('.editor-preview svg')).to_have_attribute('viewBox','0 0 64 64');expect(dialog.locator('[data-class-status]')).to_contain_text('colori e taglio propri')
  for field in ['skin','hair','cloak','style']:expect(dialog.locator(f'[name={field}]').first).to_be_disabled()
  assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==seed
  for width in [1440,390,320]:page.set_viewport_size({'width':width,'height':1050});assert dialog.evaluate('(e)=>e.scrollWidth<=e.clientWidth+2')
  page.set_viewport_size({'width':1440,'height':1050});page.screenshot(path='/workspace/PocketD-D-class-editor.png',full_page=True)
  with page.expect_download() as sd:dialog.get_by_role('link',name='Scarica il pacchetto SD').click()
  assert Path(sd.value.path()).read_bytes()==(ROOT/'public/assets/PocketDD-SD.zip').read_bytes()
  dialog.get_by_role('button',name='Annulla',exact=True).click();assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==seed
  page.locator('[data-action=edit]').click();dialog.locator('[name=outfit]').select_option('8');dialog.get_by_role('button',name='SALVA PERSONAGGIO',exact=True).click();expect(dialog).to_have_count(0)
  saved=json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'));before=json.loads(seed);assert saved['characters'][0]==before['characters'][0]
  active=saved['characters'][1]['state'];assert active['avatar']['outfit']==8;expected=before['characters'][1]['state'];actual=dict(active);actual['avatar']=expected['avatar'];actual['version']=expected['version'];assert actual==expected
  page.reload();page.locator('[data-action=device-export]').click();expect(page.locator('dialog')).to_contain_text('USB13');expect(page.locator('dialog')).to_contain_text('SD')
  with page.expect_download() as packet:page.locator('[data-device-download]').click()
  wire=json.loads(Path(packet.value.path()).read_text());assert wire['version']==13 and wire['avatar']['outfit']==8 and wire['avatar']['lineage']==12 and wire['avatar']['body']==1
  page.locator('[data-action=edit]').click();dialog.locator('[name=outfit]').select_option('0');expect(dialog.locator('[name=skin]').first).to_be_enabled();assert dialog.locator('[name=skin]:checked').input_value()=='2' and dialog.locator('[name=style]:checked').input_value()=='1';assert dialog.locator('.editor-preview svg').get_attribute('viewBox')==original
  dialog.get_by_role('button',name='SALVA PERSONAGGIO',exact=True).click();restored=json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'));assert restored['characters']==before['characters']
  # With unavailable asset download, editor keeps the chosen preset but identifies fallback.
  page.route('**/assets/classes.pdc',lambda route:route.fulfill(status=404,body='missing'));page.reload();page.locator('[data-action=edit]').click();dialog.locator('[name=outfit]').select_option('9');expect(dialog.locator('[data-class-status]')).to_contain_text('Asset classi assenti');assert dialog.locator('.editor-preview svg').get_attribute('viewBox')==original
  assert not errors,errors;browser.close()
 print('PASS: PC class draft/cancel/save/reload, unchanged mechanics/other hero, original palette recovery, missing pack fallback, SD download and USB13 export; 1440/390/320 widths. Synthetic only.')
finally:
 server.terminate();server.wait(timeout=5)
