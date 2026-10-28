#!/usr/bin/env python3
"""Synthetic standard custom skills/saves in the production app. No live Beyond or hardware."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {convertBeyond} from './src/beyond-conversion.js';
import {beyondRichFixture} from './src/beyond-fixtures.js';
const cv=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
 const p=beyondRichFixture(),c=p.data;c.id=Number(id);
 c.characterValues=[cv(26,14,1),cv(24,14,3),cv(25,14,-1),cv(26,3,2),cv(24,6,2),cv(24,'6','2'),cv(26,5,4),cv(41,3,1),cv(39,3,-1),cv(40,3,0),cv(41,2,3),cv(39,2,2),cv(40,2,-1)];
 if(id==='222222')c.characterValues.push(cv(24,14,5));
 if(id==='333333')c.characterValues[7].value=2;
 if(id==='444444')c.characterValues[2].value=30;
 if(id==='555555'){c.characterValues[0].value=0;c.characterValues[7].value=null;}
 if(id==='666666')c.modifiers.feat.push({type:'bonus',subType:'perception',value:null,statId:4});
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
  for id in ['222222','333333','444444']:
   read(id);expect(page.locator('[data-accept]')).to_be_disabled();expect(page.locator('[data-blocker-title]')).to_be_visible();assert raw()==before
  read('666666');expect(page.locator('[data-training]')).to_contain_text('tiro da verificare');assert raw()==before
  read('555555');expect(page.locator('[data-training]')).to_contain_text('Percezione: +4');expect(page.locator('[data-training]')).to_contain_text('Salvezza COS: +4');assert raw()==before
  read('123456');expect(page.locator('[data-training]')).to_contain_text('Percezione: +2 · personalizzato');expect(page.locator('[data-training]')).to_contain_text('Acrobazia: +2 · mezza competenza');expect(page.locator('[data-training]')).to_contain_text('Arcano: +2 · personalizzato');expect(page.locator('[data-training]')).to_contain_text('Salvezza COS: +2 · personalizzato');expect(page.locator('[data-training]')).to_contain_text('Salvezza DES: +4 · personalizzato');assert raw()==before
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.dismiss());page.locator('[data-create]').click();assert raw()==before
  page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  library=json.loads(raw());assert library['characters'][:-1]==json.loads(before)['characters'];s=library['characters'][-1]['state'];assert s['skillRanks']['perception']==0 and s['skillRanks']['stealth']==2 and 'acrobatics' in s['extended']['halfSkills'];assert s['extended']['skillBonuses']['arcana']==2;assert s['saveProficiencies']['COS'] is False and s['saveProficiencies']['DES'] is True;assert s['version']==11
  saved=raw();page.reload();page.locator('[data-action=device-export]').click()
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());t=packet['training'];assert packet['version']==9 and t['skillRanks']['perception']==0 and t['skillRanks']['stealth']==2;assert 'acrobatics' in t['halfSkills'];assert t['skillBonuses']['perception']==2 and t['saveBonuses']['COS']==-1;assert t['saveProficiencies']['COS'] is False;assert raw()==saved
  assert not errors,errors;browser.close()
 print('PASS: synthetic custom training preview, explicit none/half/extra visibility, unset preservation, conflicts/unsupported/overflow blocks, unresolved-roll guards, read-only cancel/new PG, old collection/reload/USB9 and mobile layout. No live Beyond or physical USB.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
