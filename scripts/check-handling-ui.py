#!/usr/bin/env python3
"""Real inventory editor with synthetic local state only; no serial or Beyond."""
import http.server,threading
from functools import partial
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
HTML='''<!doctype html><meta name="viewport" content="width=device-width,initial-scale=1"><link rel="stylesheet" href="/src/style.css"><script type="module">
import {openInventoryManager} from '/src/inventory-ui.js';import {initialState,serialize} from '/src/model.js';
window.state=initialState();window.writes=0;window.before=JSON.stringify(state);
const escapeHtml=s=>String(s??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
window.backup=()=>serialize(state);
openInventoryManager({getState:()=>state,commit:s=>{state=s;writes++},newId:()=> 'synthetic-new',escapeHtml,abilityKeys:['FOR','DES','COS','INT','SAG','CAR']});
</script>'''
class Handler(http.server.SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
 def do_GET(self):
  if self.path=='/handling-check':
   data=HTML.encode();self.send_response(200);self.send_header('Content-Type','text/html; charset=utf-8');self.send_header('Content-Length',str(len(data)));self.end_headers();self.wfile.write(data)
  else:super().do_GET()
server=http.server.ThreadingHTTPServer(('127.0.0.1',0),partial(Handler,directory=str(ROOT)));threading.Thread(target=server.serve_forever,daemon=True).start()
try:
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);page=browser.new_page(viewport={'width':1000,'height':1000});errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.goto(f'http://127.0.0.1:{server.server_port}/handling-check');page.locator('[data-edit-item]').first.click()
  page.locator('[name=handling-enabled]').check();page.locator('[name=handling-type]').select_option('2');page.locator('[name=handling-sides]').select_option('10');page.locator('[name=handling-grip]').check();page.locator('[name=handling-finesse]').check()
  ammo=page.evaluate('state.inventory[2].id');page.locator('[name=handling-ammo]').select_option(ammo)
  page.locator('[data-cancel-item]').click();assert page.evaluate('writes===0&&JSON.stringify(state)===before')
  page.locator('[data-edit-item]').first.click();page.locator('[name=handling-enabled]').check();page.locator('[name=handling-type]').select_option('2');page.locator('[name=handling-sides]').select_option('10');page.locator('[name=handling-grip]').check();page.locator('[name=handling-finesse]').check();page.locator('[name=handling-ammo]').select_option(ammo)
  page.locator('#item-editor [type=submit]').click();expect(page.locator('.manager-status')).to_have_text('Oggetto salvato');assert page.evaluate('writes')==1
  assert page.evaluate('state.inventory[0].handling')=={'flags':6,'grip':1,'versatileSides':10,'ammoId':ammo};assert page.evaluate('JSON.parse(backup()).version')==26
  assert page.evaluate('state.hp===24&&state.inventory.every((i,n)=>i.quantity===JSON.parse(before).inventory[n].quantity)')
  page.evaluate('window.saved=JSON.stringify(state)');page.locator('[data-edit-item]').first.click();page.locator('[name=dieSides]').select_option('12');page.locator('#item-editor [type=submit]').click();expect(page.locator('.editor-error')).not_to_be_empty();assert page.evaluate('writes===1&&JSON.stringify(state)===saved')
  page.locator('[name=dieSides]').select_option('8')
  for width in [1000,390,320]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth');assert page.locator('[name=handling-type]').bounding_box()['width']>0
  page.set_viewport_size({'width':1000,'height':1000});page.locator('[data-handling-fields]').screenshot(path='/workspace/PocketDD-handling-PC-preview.png')
  page.locator('[data-cancel-item]').click();page.evaluate("state.inventory[0].handling={flags:12,grip:0,versatileSides:0,ammoId:null};state.inventory.push({...state.inventory[0],id:'second',name:'Secondaria sintetica'})")
  page.evaluate('window.dualBefore=JSON.stringify(state)');page.locator('[data-dual-config]').click();page.locator('dialog').last.locator('[name=weapon]').select_option('second');page.locator('dialog').last.locator('[data-back]').click();assert page.evaluate('writes===1&&JSON.stringify(state)===dualBefore')
  page.locator('[data-dual-config]').click();modal=page.locator('dialog').last;modal.locator('[name=weapon]').select_option('second');modal.locator('[name=edition]').select_option('2');modal.locator('[name=style]').check()
  for width in [1000,390,320]:page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.set_viewport_size({'width':1000,'height':1000});modal.screenshot(path='/workspace/PocketDD-dual-PC-preview.png');modal.locator('[type=submit]').click();assert page.evaluate("writes===2&&state.dualWield.weaponId==='second'&&state.dualWield.edition===2&&state.dualWield.style&&!state.dualWield.bonusSpent&&JSON.parse(backup()).version===27")
  assert not errors,errors;browser.close()

 print('PASS handling editor: cancel unchanged, one explicit saved v26 profile, no HP/quantity cost, invalid dice rejected unchanged, real form at 1000/390/320 px; dual dialog cancel/save/edition/style and v27 checked; synthetic character only.')
finally:server.shutdown();server.server_close()
