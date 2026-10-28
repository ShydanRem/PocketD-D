#!/usr/bin/env python3
"""Manual source counters in isolated PC browser; synthetic source, no USB hardware."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyond} from './src/beyond-conversion.js';import {beyondManualFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{const p=beyondManualFixture(id==='999999'?9:5);p.data.id=Number(id);return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(address);page.locator('[data-action=beyond-import]').wait_for()
  key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():return json.loads(raw())['characters'][-1]['state']
  before=raw();page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill('999999');page.locator('[data-read] button').click();expect(page.locator('[data-status]')).to_contain_text('8 contatori');assert raw()==before
  page.locator('[name=character]').fill('123456');page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-manual-resources] li')).to_have_count(5);expect(page.locator('[data-manual-resources]')).to_contain_text('35/50');expect(page.locator('[data-powers]')).to_contain_text('Riserva manuale 3: nota/effetto manuale');assert raw()==before
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0);saved=raw();original=active();assert original['version']==18 and len(original['extended']['manualResources'])==5;assert json.loads(saved)['characters'][:-1]==json.loads(before)['characters']
  def editor():page.locator('[data-action=extended-state]').click()
  editor();expect(page.locator('[data-manual-resources] fieldset')).to_have_count(5);page.locator('[name=manual-2-current]').fill('34');page.locator('dialog [data-close]').click();assert raw()==saved
  editor();page.locator('[name=manual-2-current]').fill('34');page.evaluate("()=>{Storage.prototype.setItem=function(){throw new DOMException('quota','QuotaExceededError')}}");page.locator('dialog button[type=submit]').click();expect(page.locator('dialog [data-error]')).not_to_be_empty();assert raw()==saved;page.locator('dialog [data-close]').click();editor();expect(page.locator('[name=manual-2-current]')).to_have_value('35');page.locator('dialog [data-close]').click();page.reload()
  editor();page.locator('[name=manual-2-max]').fill('30');page.locator('dialog button[type=submit]').click();expect(page.locator('dialog [data-error]')).not_to_be_empty();assert raw()==saved;page.locator('[name=manual-2-max]').fill('50');page.locator('[name=manual-2-current]').fill('34')
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth');assert page.locator('dialog').evaluate('(e)=>e.scrollWidth<=e.clientWidth+1')
  page.locator('dialog button[type=submit]').click();expect(page.locator('dialog')).to_have_count(0);assert active()['extended']['manualResources'][2]['current']==34
  assert json.loads(raw())['characters'][:-1]==json.loads(before)['characters'];page.reload();assert active()['extended']['manualResources'][2]['current']==34
  page.locator('[data-action=device-export]').click();expect(page.locator('dialog')).to_contain_text('Contatori: Riserva manuale 1 4/4')
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==9 and len(packet['resources'])==5 and packet['resources'][2]['current']==34
  note=next(v for v in packet['magic']['powers'] if v['name']=='Riserva manuale 3');assert note['flags']&16 and note['maximum']==0
  assert next(v for v in packet['magic']['powers'] if v['name']=='Rage')['remaining']==2;assert not errors,errors;browser.close()
 print('PASS: synthetic manual counters preview/new PG, ninth blocks, ordinary uses separate, PC editor cancel/quota rollback/invalid max/save/reload, preserved old sheets, exact USB9 export/passive notes and mobile widths; no real Beyond or physical USB.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
