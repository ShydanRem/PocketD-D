#!/usr/bin/env python3
"""Synthetic production-browser checks for ability caps/floors. No remote character or USB hardware."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {convertBeyond} from './src/beyond-conversion.js';
import {beyondCoreFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
 const p=beyondCoreFixture(),c=p.data;c.id=Number(id);c.modifiers.race=[];c.stats[1].value=18;c.stats[2].value=18;c.bonusStats[1].value=2;
 c.modifiers.feat.push({type:'bonus',subType:'dexterity-score',value:4},{type:'bonus',subType:'constitution-score',value:4},{id:'dex-cap',type:'bonus',subType:'ability-score-maximum',value:2,statId:2},{id:'con-cap',type:'bonus',subType:'ability-score-maximum',value:4,statId:3},{type:'set',subType:'constitution-score',value:25});
 const m={id:'floor-item',type:'set',subType:'strength-score',value:19,restriction:'if not already higher',isGranted:true};
 c.modifiers.item.push(m);c.inventory.push({quantity:1,equipped:id!=='555555',isAttuned:true,definition:{name:'Sonda fissapunteggio',filterType:'Other Gear',canAttune:true,grantedModifiers:[structuredClone(m)]}});
 if(id==='222222')c.modifiers.feat[2].statId=7;
 if(id==='333333')c.inventory.at(-1).definition.grantedModifiers[0].duration=1;
 if(id==='444444')c.overrideStats[2].value=8;
 return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};
}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.goto(address);page.locator('[data-action=beyond-import]').wait_for();key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  before=raw();page.locator('[data-action=beyond-import]').click()
  def read(id):
   page.locator('[name=character]').fill(id);page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible()
  for id in ['222222','333333']:
   read(id);expect(page.locator('[data-accept]')).to_be_disabled();expect(page.locator('[data-blocker-title]')).to_be_visible();assert raw()==before
  read('444444');expect(page.locator('[data-values]')).to_contain_text('13/17');expect(page.locator('[data-values]')).to_contain_text('17');expect(page.locator('[data-evidence]')).to_contain_text('override finale 8');assert raw()==before
  read('555555');expect(page.locator('[data-evidence]')).to_contain_text('FOR: base 13 + modificatori 0');assert raw()==before
  read('123456');expect(page.locator('[data-values]')).to_contain_text('37/41');expect(page.locator('[data-evidence]')).to_contain_text('massimo 22; bonus personalizzato successivo 2 = 24');expect(page.locator('[data-evidence]')).to_contain_text('minimo fissato 25');assert raw()==before
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.dismiss());page.locator('[data-create]').click();assert raw()==before
  page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  library=json.loads(raw());assert library['characters'][:-1]==json.loads(before)['characters'];s=library['characters'][-1]['state'];assert s['abilities']=={'FOR':19,'DES':24,'COS':25,'INT':10,'SAG':11,'CAR':8};assert (s['hp'],s['maxHp'],s['armorClass'])==(37,41,24);assert s['version']==11
  saved=raw();page.reload();page.locator('[data-action=device-export]').click()
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['abilities']==s['abilities'];assert (packet['hp'],packet['maxHp'],packet['armorClass'])==(37,41,24);assert packet['version']==9;assert raw()==saved
  assert not errors,errors;browser.close()
 print('PASS: synthetic capped/fixed abilities, post-cap bonus, final override, blocked invalid/hidden requirements, read-only preview, cancel/confirm/new PG, old collection, reload/USB projection and mobile layout. No real Beyond or USB evidence.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
