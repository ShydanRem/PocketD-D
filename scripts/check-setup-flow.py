#!/usr/bin/env python3
"""Guided app/embedded installer/Beyond/appearance/USB on synthetic local services only."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1];OUTPUT=Path('/tmp/pdd81-guide-captures');OUTPUT.mkdir(exist_ok=True)
source="""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyondFull} from './src/beyond-conversion.js';import {beyondProfileFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>({...convertBeyondFull(beyondProfileFixture(),id),testFixture:true,transport:{status:200,bytes:1000,elapsedMs:1,authenticated:false}})});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""
server=subprocess.Popen(['node','--input-type=module','-e',source],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
mock=r'''
window.setupUsb={sent:[],closed:0,action:'pending',confirm:null};
Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{const s=window.setupUsb;if(s.action==='chooser-cancel')throw new DOMException('Cancelled','NotFoundError');let controller,buffer='';const push=t=>controller.enqueue(new TextEncoder().encode(t));return {readable:new ReadableStream({start(c){controller=c;}}),writable:new WritableStream({write(bytes){buffer+=new TextDecoder().decode(bytes);let end;while((end=buffer.indexOf('\n'))>=0){const line=buffer.slice(0,end);buffer=buffer.slice(end+1);s.sent.push(line);if(line==='PDD HELLO')push('PDD READY 17\n');else if(line.startsWith('{')){push('PDD WAIT_CONFIRM\n');s.confirm=()=>push(s.action==='error'?'PDD ERROR storage\n':s.action==='cancel'?'PDD CANCELLED user\n':'PDD IMPORTED\n');}}}}),open:async()=>{},close:async()=>{s.closed++},setSignals:async()=>{},getInfo:()=>({usbVendorId:0x303a,usbProductId:0x1001})};}}});
'''
try:
 assert select.select([server.stdout],[],[],15)[0];base=server.stdout.readline().strip();assert base.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);ctx=browser.new_context(viewport={'width':1280,'height':1000});ctx.add_init_script(mock);page=ctx.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(base+'/?setup=1');guide=page.locator('.pocket-setup');expect(guide).to_be_visible();guide.screenshot(path=str(OUTPUT/'pocket.png'));before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
  # A parent-origin message with the wrong source must not pretend installation succeeded.
  page.locator('[data-setup-install]').click();frame=page.frame_locator('iframe[title="Installa PocketD-D"]');expect(frame.locator('#status')).to_contain_text('Firmware pronto');expect(frame.locator('input[value=update]')).to_be_checked()
  page.evaluate("window.postMessage({type:'pocketdd:install',phase:'verified'},location.origin)");expect(guide.locator('[data-install-observation]')).not_to_contain_text('Firmware scritto e verificato.')
  # Synthetic child messages exercise busy/receipt handling, not actual flashing.
  child=page.frames[1];child.evaluate("parent.postMessage({type:'pocketdd:install',phase:'busy',value:true},location.origin)");expect(guide.locator('[data-setup-close]')).to_be_disabled();expect(guide.locator('[data-setup-ready]')).to_be_disabled();page.keyboard.press('Escape');expect(guide).to_be_visible()
  child.evaluate("parent.postMessage({type:'pocketdd:install',phase:'verified'},location.origin);parent.postMessage({type:'pocketdd:install',phase:'busy',value:false},location.origin)");expect(guide.locator('[data-install-observation]')).to_contain_text('Firmware scritto e verificato');expect(guide.locator('li[aria-current]')).to_contain_text('Pocket');guide.locator('[data-setup-ready]').click()
  guide.locator('[data-setup-import]').click();imp=page.locator('.guided-beyond');expect(imp).to_be_visible();imp.locator('[name=character]').fill('123456');imp.locator('[data-read] button').click();expect(imp.locator('[data-preview]')).to_be_visible();expect(imp.locator('[data-checks]')).to_contain_text('DATI SINTETICI')
  assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
  # Values/appearance are visible; large evidence/inventory lists are folded.
  expect(imp.locator('[data-values]')).to_be_visible();expect(imp.locator('[data-appearance]')).to_be_visible();expect(imp.locator('[data-items]')).not_to_be_visible();expect(imp.locator('[data-create]')).to_be_disabled();imp.evaluate('(e)=>e.scrollTop=0');imp.screenshot(path=str(OUTPUT/'review.png'));imp.locator('[data-accept]').check()
  unexpected=[];page.on('dialog',lambda d:(unexpected.append(d.message),d.dismiss()));
  page.evaluate("()=>{window.originalSetItem=Storage.prototype.setItem;Storage.prototype.setItem=function(k,v){if(k==='pocketdd-library-v1')throw new DOMException('Fixture quota','QuotaExceededError');return window.originalSetItem.call(this,k,v)};}")
  imp.locator('[data-create]').click();expect(imp.locator('[data-status]')).to_contain_text('Importazione non salvata');expect(guide.locator('li[aria-current]')).to_contain_text('Personaggio');assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
  page.evaluate('()=>{Storage.prototype.setItem=window.originalSetItem;}');imp.locator('[data-create]').click();expect(imp).to_have_count(0);expect(guide.locator('li[aria-current]')).to_contain_text('Al tavolo');assert not unexpected
  library=json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'));old=json.loads(before);assert len(library['characters'])==len(old['characters'])+1 and library['characters'][:-1]==old['characters']
  # Appearance-only edit must preserve every non-cosmetic field.
  guide.locator('[data-setup-appearance]').click();editor=page.locator('.appearance-only');expect(editor.locator('[name=maxHp]')).not_to_be_visible();editor.locator('[name=body]').select_option('1');editor.locator('[type=submit]').click();expect(editor).to_have_count(0)
  updated=json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'));a=library['characters'][-1]['state'];b=updated['characters'][-1]['state'];assert b['avatar']['body']==1;assert {k:v for k,v in a.items() if k!='avatar'}=={k:v for k,v in b.items() if k!='avatar'}
  for w in [1280,390,320]:page.set_viewport_size({'width':w,'height':1000});assert guide.evaluate('(e)=>e.scrollWidth<=e.clientWidth+1')
  page.set_viewport_size({'width':1280,'height':1000});page.screenshot(path='/workspace/PocketD-D-setup-preview.png',full_page=True);guide.screenshot(path=str(OUTPUT/'transfer.png'))
  # No success until the exact import ACK. Device errors release controls and retain collection.
  guide.locator('[data-setup-send]').click();transfer=page.locator('.guided-export');transfer.locator('[data-device-send]').click();expect(transfer.locator('[data-usb-status]')).to_contain_text('conferma entro 120');expect(transfer.locator('[data-device-close]')).to_be_disabled();page.keyboard.press('Escape');expect(transfer).to_be_visible();assert 'Copia salvata sul Pocket.' not in guide.inner_text()
  page.evaluate("window.setupUsb.action='error';window.setupUsb.confirm()");expect(transfer.locator('[data-usb-status]')).to_contain_text('Salvataggio non confermato');expect(transfer.locator('[data-device-close]')).to_be_enabled();assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==json.dumps(updated,ensure_ascii=False,separators=(',',':')) or json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'))==updated
  page.evaluate("window.setupUsb.action='pending'");transfer.locator('[data-device-send]').click();expect(transfer.locator('[data-usb-status]')).to_contain_text('conferma entro 120');page.evaluate('window.setupUsb.confirm()');expect(transfer).to_have_count(0);expect(guide.locator('h3')).to_have_text('Copia salvata sul Pocket.');assert page.evaluate('window.setupUsb.closed')==2
  # Editing after ACK invalidates that receipt; cancelled editor retains it.
  guide.locator('[data-setup-appearance]').click();page.locator('.appearance-only [data-close]').first.click();expect(page.locator('.appearance-only')).to_have_count(0);expect(guide.locator('h3')).to_have_text('Copia salvata sul Pocket.')
  guide.locator('[data-setup-appearance]').click();editor=page.locator('.appearance-only');editor.locator('[name=body]').select_option('0');editor.locator('[type=submit]').click();expect(editor).to_have_count(0);expect(guide.locator('h3')).to_have_text('Ultimo passo: portalo sul Pocket.')
  guide.locator('[data-setup-back]').click();guide.locator('[data-setup-character]').select_option(old['activeId']);assert json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'))['activeId']==old['activeId'];guide.locator('[data-setup-current]').click();expect(guide.locator('h3')).to_have_text('Ultimo passo: portalo sul Pocket.')
  bad=browser.new_context();bad.add_init_script("localStorage.setItem('pocketdd-library-v1','{broken-fixture')");broken=bad.new_page();broken.goto(base+'/?setup=1');broken.locator('[data-setup-ready]').click();expect(broken.locator('[data-setup-import]')).to_be_disabled();expect(broken.locator('[data-setup-current]')).to_be_disabled();assert broken.evaluate('localStorage.getItem("pocketdd-library-v1")')=='{broken-fixture'
  assert not errors,errors;browser.close()
 print('PASS guided local flow: embedded installer identity/busy/observation, synthetic Beyond preview/create once, preserved collection, appearance-only save/cancel, mobile, pending/error/ACK USB, receipt invalidation. No Windows execution, physical USB or real Beyond access.')
finally:
 server.terminate();server.wait(timeout=5)
