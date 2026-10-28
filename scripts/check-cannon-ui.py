#!/usr/bin/env python3
"""Synthetic import, confirmed slot payment, atomic PC failure/cancel and wire export."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyond} from './src/beyond-conversion.js';import {beyondCannonFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{const p=beyondCannonFixture(id==='654321'?1:0);p.data.id=Number(id);return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(address);page.locator('[data-action=beyond-import]').wait_for();key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():return json.loads(raw())['characters'][-1]['state']
  def import_pg(id):
   before=raw();page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill(id);page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-powers]')).to_contain_text('poi slot a scelta');assert raw()==before;page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0);assert json.loads(raw())['characters'][:-1]==json.loads(before)['characters']
  def powers():page.locator('[data-go-page=actions]').click();page.locator('[data-action-tab=powers]').click()
  def card():return page.locator('.power-card').filter(has=page.locator('strong',has_text='Create Eldritch Cannon'))
  original=raw();import_pg('123456');assert active()['version']==19;powers();expect(card().locator('button')).to_be_enabled();expect(card()).not_to_contain_text('CD ');card().locator('select').select_option('pact:2');before=raw();page.once('dialog',lambda d:d.dismiss());card().locator('button').click();assert raw()==before
  page.evaluate("()=>{Storage.prototype.setItem=function(){throw new DOMException('quota','QuotaExceededError')}}");page.once('dialog',lambda d:d.accept());card().locator('button').click();expect(page.locator('.power-message')).not_to_be_empty();assert raw()==before;page.reload();powers();card().locator('select').select_option('pact:2');source=active();page.once('dialog',lambda d:d.accept());card().locator('button').click();after=active();assert after['extended']['pactSlots'][1]['current']==0 and after['slots']==source['slots'];assert after['extended']['manualResources']==source['extended']['manualResources'];assert next(v for v in after['powers'] if v.get('effect')=='eldritch-cannon')['remaining']==0
  page.locator('[data-action=undo]').click();assert active()==source
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.locator('[data-action=device-export]').click();expect(page.locator('dialog')).to_contain_text('USB16')
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==16;effect=next(v for v in packet['magic']['powers'] if v['effect']==3);assert effect['kind']==1 and effect['remaining']==0 and effect['level']==0
  import_pg('654321');powers();expect(card().locator('select')).to_have_count(0);source=active();page.once('dialog',lambda d:d.accept());card().locator('button').click();after=active();assert next(v for v in after['powers'] if v.get('effect')=='eldritch-cannon')['remaining']==0 and after['slots']==source['slots'] and after['extended']==source['extended'];expect(card().locator('select')).to_have_count(1)
  assert json.loads(raw())['characters'][0]==json.loads(original)['characters'][0];assert not errors,errors;browser.close()
 print('PASS: pinned synthetic source preview, new v19 PG, feature stays distinct from spells, explicit free/ordinary/pact payment, cancel, atomic quota failure, exact charge/undo, preserved manual balances/other sheets, USB16 and mobile. No live Beyond or physical USB.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
