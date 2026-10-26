"""Synthetic browser regression for choice/custom-item reconciliation. No remote data."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {convertBeyond} from './src/beyond-conversion.js';
import {beyondRichFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
 const p=beyondRichFixture(),c=p.data;c.id=Number(id);
 c.preferences={enableOptionalOrigins:true};c.optionalOrigins=[];
 c.modifiers.race.push({id:'picked-int',type:'bonus',subType:'intelligence-score',entityId:4,value:2,isGranted:false});
 c.choices.race=[{id:'2-picked-int',type:2,subType:5,optionValue:40004,optionIds:[40004]}];
 c.choices.choiceDefinitions.push({options:[{id:40004,label:'Intelligence Score'}]});
 c.inventory[0].id=30001;
 c.characterValues=[{typeId:8,valueId:30001,valueTypeId:1439493548,value:'Lancia rinominata sonda'}];
 if(id==='222222')c.characterValues.push({typeId:1,valueId:4,value:28});
 c.customItems=[{id:70001,name:'Sasso sonda',quantity:3,description:'Testo inventato.'},{id:70002,name:'Taccuino sonda',quantity:7,description:'Solo definizione.'}];
 c.inventory.push({id:80001,quantity:3,equipped:false,definition:{id:70001,name:'Sasso sonda',isCustomItem:true,filterType:null,grantedModifiers:[],armorClass:null,armorTypeId:null,damage:null,fixedDamage:null}});
 c.deathSaves={successCount:null,failCount:null,isStabilized:false};
 c.pactMagic=[];c.classes[0].definition.canCastSpells=true;
 c.classes[0].definition.spellRules={multiClassSpellSlotDivisor:2,levelSpellSlots:Array.from({length:21},()=>[0,0,0,0,0,0,0,0,0])};
 c.classes[0].definition.spellRules.levelSpellSlots[3]=[3,1,0,0,0,0,0,0,0];
 c.spellSlots=[{level:1,available:0,used:1},{level:2,available:0,used:0}];
 return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};
}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0]
 address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
  context=browser.new_context(viewport={'width':1024,'height':900})
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort())
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.goto(address);key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  before=json.loads(raw());page.locator('[data-action=beyond-import]').click()
  def read(id):
   page.locator('[name=character]').fill(id);page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible()
  read('222222');expect(page.locator('[data-accept]')).to_be_disabled()
  expect(page.locator('[data-blocker-title]')).to_be_visible();expect(page.locator('[data-blockers]')).to_contain_text('Override personalizzati')
  assert json.loads(raw())==before
  read('123456');expect(page.locator('[data-accept]')).to_be_enabled();expect(page.locator('[data-blocker-title]')).to_be_hidden()
  expect(page.locator('[data-items]')).to_contain_text('Taccuino sonda ×7 · equipaggiamento non disponibile')
  expect(page.locator('[data-items]')).to_contain_text('Sasso sonda ×3 · non equipaggiato')
  expect(page.locator('[data-evidence]')).to_contain_text('intelligence-score')
  assert json.loads(raw())==before
  page.locator('[data-accept]').check();page.locator('[name=character]').fill('333333')
  expect(page.locator('[data-preview]')).to_be_hidden();expect(page.locator('[data-create]')).to_be_disabled()
  read('123456');page.locator('[data-weapon]').select_option('beyond-item-1')
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click()
  expect(page.locator('.beyond-import')).to_have_count(0)
  after=json.loads(raw());assert after['characters'][:-1]==before['characters']
  s=after['characters'][-1]['state'];assert s['abilities']['INT']==12 and len(s['inventory'])==5
  assert [i['quantity'] for i in s['inventory'][-2:]]==[3,7]
  assert s['inventory'][0]['name']=='Lancia rinominata sonda' and s['equippedWeaponId']=='beyond-item-1'
  assert s['extended']['deathSaves']=={'successes':0,'failures':0,'stabilized':False}
  assert s['slots'][:2]==[{'max':3,'current':2},{'max':1,'current':1}]
  assert any('Oggetti personalizzati' in v for v in s['importInfo']['limitations'])
  saved=raw();page.reload();assert raw()==saved;assert not errors,errors
  browser.close()
 print('PASS: synthetic reconciled preview/confirm/reload; chosen stat, slots, null counters, custom quantities, unknown equipment, blocked numeric override, URL invalidation and old sheets preserved. No real Beyond evidence.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
