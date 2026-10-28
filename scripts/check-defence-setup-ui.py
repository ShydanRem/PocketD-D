#!/usr/bin/env python3
"""Explicit defence setup in isolated synthetic PC collection, no real Beyond or USB."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyond} from './src/beyond-conversion.js';import {beyondRichFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{const p=beyondRichFixture();p.data.id=Number(id);p.data.modifiers.race.push({id:'test-fire',type:'resistance',subType:'fire',isGranted:true,restriction:'',duration:null},{id:'test-cold',type:'immunity',subType:'cold',isGranted:true,restriction:'',duration:null});return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(address);page.locator('[data-action=beyond-import]').wait_for()
  key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():return json.loads(raw())['characters'][-1]['state']
  before=raw();page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill('123456');page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible();assert raw()==before
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  imported=raw();original=active();assert original['extended']['defences']=={'configured':48,'resistance':32,'vulnerability':0,'immunity':16}
  def open_setup():
   page.locator('[data-action=device-export]').click();page.locator('[data-device-defences] summary').click();expect(page.locator('[data-device-defences]')).to_contain_text('2/13');page.locator('[data-device-configure-defences]').click();expect(page.locator('[data-defence-editor]')).to_have_attribute('open','')
  open_setup();count=page.locator('[data-defence-count]');expect(count).to_contain_text('2/13');page.locator('[data-defence-review]').click();review=page.locator('[data-defence-completion]');expect(review).to_be_visible();expect(page.locator('[data-defence-missing] li')).to_have_count(11);assert 'Fuoco' not in page.locator('[data-defence-missing]').inner_text() and 'Freddo' not in page.locator('[data-defence-missing]').inner_text();assert raw()==imported
  page.locator('[data-defence-cancel]').click();expect(review).to_be_hidden();page.locator('[data-defence-review]').click();page.locator('[name=defence-0]').select_option('3');expect(review).to_be_hidden();page.locator('[data-defence-review]').click();expect(page.locator('[data-defence-missing] li')).to_have_count(10);page.locator('[data-defence-confirm]').click();expect(count).to_contain_text('13/13');expect(page.locator('[name=defence-0]')).to_have_value('3');expect(page.locator('[name=defence-5]')).to_have_value('1');expect(page.locator('[name=defence-4]')).to_have_value('4');assert raw()==imported
  page.locator('dialog [data-close]').click();assert raw()==imported
  open_setup();page.locator('[name=defences-enabled]').uncheck();expect(page.locator('[data-defence-review]')).to_be_disabled();page.locator('[name=defences-enabled]').check();page.locator('[data-defence-review]').click();page.locator('[data-defence-confirm]').click()
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.evaluate("()=>{Storage.prototype.setItem=function(){throw new DOMException('quota','QuotaExceededError')}}");page.locator('dialog button[type=submit]').click();expect(page.locator('dialog [data-error]')).not_to_be_empty();assert raw()==imported;page.locator('dialog [data-close]').click();open_setup();expect(count).to_contain_text('2/13');page.locator('dialog [data-close]').click();page.reload()
  open_setup();page.locator('[data-defence-review]').click();page.locator('[data-defence-confirm]').click();page.locator('dialog button[type=submit]').click();expect(page.locator('dialog')).to_have_count(0)
  complete=active();assert complete['extended']['defences']=={'configured':8191,'resistance':32,'vulnerability':0,'immunity':16};assert {**original,'version':16,'extended':{**original['extended'],'defences':complete['extended']['defences']}}==complete
  assert json.loads(raw())['characters'][:-1]==json.loads(before)['characters'];saved=raw();page.reload();assert raw()==saved
  page.locator('[data-action=device-export]').click();page.locator('[data-device-defences] summary').click();expect(page.locator('[data-device-defences]')).to_contain_text('13/13')
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==14 and packet['defences']==complete['extended']['defences']
  page.locator('[data-action=extended-state]').click();page.locator('[data-defence-editor] summary').click();expect(page.locator('[data-defence-review]')).to_be_disabled();page.locator('[name=defence-12]').select_option('-1');page.locator('dialog button[type=submit]').click();assert active()['extended']['defences']['configured']==4095
  assert not errors,errors;browser.close()
 print('PASS: synthetic import keeps unknowns, export shortcut, exact missing list, stale review invalidated, staged/cancel, existing flags, disabled profile, 13/13, atomic quota rollback in memory/storage, save/reload, preserved other sheets, USB14 download and mobile widths. No real Beyond or physical USB.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
