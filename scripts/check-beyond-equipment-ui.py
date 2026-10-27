#!/usr/bin/env python3
"""Synthetic Beyond inventory/defense import, PC configuration and USB export. No real profile or remote access."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyond} from './src/beyond-conversion.js';import {beyondCoreFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
 const p=beyondCoreFixture();p.data.id=Number(id);p.data.name='Prova inventario';
 p.data.modifiers.race.push({id:'fire-proof',type:'resistance',subType:'fire',isGranted:true},{id:'cold-condition',type:'immunity',subType:'cold',restriction:'while raging'});
 p.data.inventory.push({id:991,quantity:1,equipped:false,chargesUsed:2,limitedUse:{maxUses:7,numberUsed:2,resetType:'Dawn'},definition:{name:'Bastone sonda',filterType:'Wondrous item',grantedModifiers:[]}}, {id:992,quantity:4,equipped:false,chargesUsed:0,limitedUse:{maxUses:1,numberUsed:0,resetType:'Consumable'},definition:{name:'Fiala sonda',filterType:'Potion',isConsumable:true,grantedModifiers:[{type:'bonus',subType:'hit-points',fixedValue:2,dice:{diceCount:2,diceValue:4,fixedValue:2,diceString:'2d4 + 2'}}]}});
 return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};
}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];url=server.stdout.readline().strip();assert url.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1050},accept_downloads=True);context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(url+'/') else r.abort())
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.on('dialog',lambda d:d.accept());page.goto(url);page.locator('[data-action=beyond-import]').wait_for();key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():
   c=json.loads(raw());return next(e['state'] for e in c['characters'] if e['id']==c['activeId'])
  before=raw();page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill('123456');page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-checks]')).to_contain_text('DATI SINTETICI')
  expect(page.locator('[data-items]')).to_contain_text('cura 2d4+2');expect(page.locator('[data-items]')).to_contain_text('cariche 5/7, recupero manuale');page.locator('[data-defence-preview] summary').click();expect(page.locator('[data-defences]')).to_contain_text('Fuoco: Resistenza');expect(page.locator('[data-defences]')).to_contain_text('Freddo: Da verificare');assert raw()==before
  for width in [1440,390,320]:page.set_viewport_size({'width':width,'height':1050});assert page.locator('.beyond-import').evaluate('(e)=>e.scrollWidth<=e.clientWidth+2')
  page.set_viewport_size({'width':1440,'height':1050});page.screenshot(path='/workspace/PocketD-D-beyond-equipment-preview.png',full_page=True);page.locator('[data-accept]').check();page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  saved=json.loads(raw());assert saved['characters'][:-1]==json.loads(before)['characters'];assert active()['version']==16 and active()['extended']['defences']=={'configured':32,'resistance':32,'vulnerability':0,'immunity':0};assert active()['inventory'][-1]['quantity']==4 and active()['inventory'][-1]['type']=='healing';assert active()['inventory'][-2]['charges']=={'max':7,'current':5,'recovery':'manual'}
  page.locator('[data-action=extended-state]').click();dialog=page.locator('dialog');dialog.locator('[data-defence-editor] summary').click();expect(dialog.locator('[name=defence-5]')).to_have_value('1');expect(dialog.locator('[name=defence-0]')).to_have_value('-1');draft_before=raw();dialog.locator('[name=defence-0]').select_option('0');dialog.locator('[data-close]').click();assert raw()==draft_before
  page.locator('[data-action=extended-state]').click();dialog.locator('[data-defence-editor] summary').click();dialog.locator('[name=defence-0]').select_option('0');dialog.get_by_role('button',name='Salva stato e risorse',exact=True).click();expect(dialog).to_have_count(0);assert active()['extended']['defences']['configured']==33;assert json.loads(raw())['characters'][:-1]==json.loads(before)['characters']
  page.reload();page.locator('[data-action=device-export]').click();dialog=page.locator('dialog');expect(dialog).to_contain_text('USB14');dialog.locator('[data-device-defences] summary').click();expect(dialog.locator('[data-device-defences]')).to_contain_text('Contundente: Normale');expect(dialog.locator('[data-device-defences]')).to_contain_text('Fuoco: Resistenza')
  with page.expect_download() as dl:dialog.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==14 and packet['defences']['configured']==33 and packet['defences']['resistance']==32;assert packet['inventory']['items'][-1]['kind']==2 and packet['inventory']['items'][-1]['count']==2 and packet['inventory']['items'][-1]['quantity']==4
  # Failed persistence must leave both stored data and the active in-memory profile unchanged.
  quota_before=raw();page.locator('[data-action=extended-state]').click();dialog.locator('[data-defence-editor] summary').click();dialog.locator('[name=defence-5]').select_option('4');page.evaluate("()=>{Storage.prototype.setItem=function(){throw new DOMException('quota','QuotaExceededError')}}")
  dialog.get_by_role('button',name='Salva stato e risorse',exact=True).click();expect(dialog.locator('[data-error]')).not_to_be_empty();assert raw()==quota_before;dialog.locator('[data-close]').click();page.locator('[data-action=extended-state]').click();dialog.locator('[data-defence-editor] summary').click();expect(dialog.locator('[name=defence-5]')).to_have_value('1');dialog.locator('[data-close]').click();page.reload()
  # Removing a PC defense profile is explicit; it must not silently persist under an old format.
  page.locator('[data-action=extended-state]').click();dialog.locator('[data-defence-editor] summary').click();dialog.locator('[name=defences-enabled]').uncheck();expect(dialog.locator('[name=defence-5]')).to_be_disabled();dialog.get_by_role('button',name='Salva stato e risorse',exact=True).click();assert 'defences' not in active()['extended'];assert active()['version']==11
  assert not errors,errors;browser.close()
 print('PASS: synthetic Beyond charges/potions/defenses preview, isolated collection preservation, new PG, manual normal versus unknown, cancel/save/reload, USB14 export, quota rollback and explicit profile removal; mobile widths checked. No live Beyond or hardware.')
finally:server.terminate();server.wait(timeout=5)
