"""Synthetic extended import + play verification. No remote request or personal data."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
OUT=Path('/tmp/pocketdd-rich-qa');OUT.mkdir(exist_ok=True)
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {convertBeyond} from './src/beyond-conversion.js';
import {beyondRichFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
 const p=beyondRichFixture();p.data.id=Number(id);
 p.data.classSpells[0].spells[0].definition.description='<p>&lt;img src=x onerror=alert(1)&gt;</p>';
 return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};
}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0]
 address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
  context=browser.new_context(viewport={'width':1440,'height':1050},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort())
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.on('dialog',lambda d:d.accept())
  page.goto(address);key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():
   collection=json.loads(raw());return next(e['state'] for e in collection['characters'] if e['id']==collection['activeId'])
  before=json.loads(raw())
  page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill('123456');page.locator('[data-read] button').click()
  expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-checks]')).to_contain_text('DATI SINTETICI')
  expect(page.locator('[data-training]')).to_contain_text('Percezione: +2');expect(page.locator('[data-attacks]')).to_contain_text('Ascia sonda: attacco +4')
  expect(page.locator('[data-powers]')).to_contain_text('Scintilla innata');page.locator('[data-weapon]').select_option('beyond-item-2')
  assert json.loads(raw())==before
  for width in [1440,390,320]:
   page.set_viewport_size({'width':width,'height':1000 if width==1440 else 844})
   assert page.locator('.beyond-import').evaluate('(e)=>e.scrollWidth<=e.clientWidth')
   page.locator('.beyond-import').evaluate('(e)=>e.scrollTop=0');page.screenshot(path=str(OUT/f'preview-{width}.png'))
  page.set_viewport_size({'width':1440,'height':1050});page.locator('[data-accept]').check();page.locator('[data-create]').click()
  expect(page.locator('.beyond-import')).to_have_count(0)
  assert json.loads(raw())['characters'][:-1]==before['characters'];assert active()['version']==11
  assert active()['equippedWeaponId']=='beyond-item-2';assert active()['extended']['tempHp']==4
  page.locator('[data-go-page=actions]').click();expect(page.locator('.equipment-status')).to_contain_text('Attacco +4')
  page.locator('[data-action-tab=powers]').click()
  def card(name):return page.locator('.power-card').filter(has=page.locator('strong',has_text=name))
  expect(card('Luce di prova')).to_contain_text('<img src=x onerror=alert(1)>');assert page.locator('.power-card img').count()==0
  expect(card('Luce di prova')).to_contain_text('INT · attacco +2 · CD 10')
  expect(card('Scintilla innata')).to_contain_text('CAR · attacco +1 · CD 9');expect(card('Eco di prova').locator('button')).to_be_disabled()
  card('Rage').locator('button').click();assert active()['extended']['combat']['raging'];assert next(p for p in active()['powers'] if p['name']=='Rage')['remaining']==1
  expect(card('Luce di prova').locator('button')).to_be_disabled()
  page.locator('[data-action-tab=weapon]').click();expect(page.locator('.equipment-status')).to_contain_text('1d12+4')
  page.locator('[data-combat-option=greatWeaponMaster]').click();expect(page.locator('.equipment-status')).to_contain_text('Attacco -1');expect(page.locator('.equipment-status')).to_contain_text('1d12+14')
  page.evaluate('()=>{crypto.getRandomValues=(a)=>{a.fill(19);return a}}')
  page.locator('[data-action=attack]').click();expect(page.locator('[data-action=damage]')).to_contain_text('3d12+14');page.locator('[data-action=damage]').click()
  assert len(active()['history'][0]['dice'])==3 and active()['history'][0]['total']==38
  page.locator('[data-combat-option=raging]').click();page.locator('[data-combat-option=greatWeaponMaster]').click()
  page.locator('[data-action-tab=powers]').click()
  card('Luce di prova').locator('select').select_option('pact:2');card('Luce di prova').locator('button').click()
  assert active()['extended']['pactSlots'][1]['current']==0 and active()['slots'][0]['current']==2
  card('Scintilla innata').locator('button').click();expect(card('Scintilla innata').locator('button')).to_be_disabled();assert active()['slots'][0]['current']==2
  page.locator('[data-go-page=pack]').click();page.locator('[data-action=rest-short]').click()
  assert active()['extended']['pactSlots'][1]['current']==2 and active()['slots'][0]['current']==2
  page.locator('[data-action=extended-state]').click();page.locator('[name=tempHp]').fill('7');page.locator('[name=coin-gp]').fill('17');page.locator('[name=hd-0]').fill('2')
  for width in [390,320]:
   page.set_viewport_size({'width':width,'height':844});assert page.locator('.beyond-import').evaluate('(e)=>e.scrollWidth<=e.clientWidth')
  page.locator('.beyond-import button[type=submit]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  assert active()['extended']['tempHp']==7 and active()['extended']['currencies']['gp']==17 and active()['extended']['hitDice'][0]['used']==2
  page.locator('[data-go-page=hero]').click();hp=active()['hp'];page.locator('[data-hp="-1"]').click();assert active()['hp']==hp and active()['extended']['tempHp']==6
  # Profile and inventory edits must preserve schema v10 and imported mechanics.
  page.locator('[data-action=edit]').click();page.locator('[name=title]').fill('Titolo modificato');page.locator('dialog button[type=submit]').click()
  assert active()['version']==11 and active()['extended']['tempHp']==6
  page.locator('[data-action=manage-inventory]').first.click();page.locator('[data-edit-item="beyond-item-2"]').click();page.locator('[name=item-name]').fill('Ascia rinominata');page.locator('#item-editor button[type=submit]').click();page.locator('[data-inventory-close]').click()
  assert active()['inventory'][1]['profile']['heavy'] is True
  page.locator('[data-action=manage-powers]').first.click();spellid=next(p['id'] for p in active()['powers'] if p['name']=='Scintilla innata')
  page.locator(f'[data-edit-power="{spellid}"]').click();page.locator('[name=spell-remaining]').fill('1');page.locator('#power-editor button[type=submit]').click();page.locator('[data-manager-close]').click()
  assert next(p for p in active()['powers'] if p['id']==spellid)['limited']['remaining']==1
  saved=raw();page.reload();assert raw()==saved;assert json.loads(raw())['characters'][:-1]==before['characters']
  page.locator('[data-action=export]').click()
  page.locator('[data-action=manage-characters]').click()
  with page.expect_download() as download:page.locator('[data-action=export-library]').click()
  exported=json.loads(Path(download.value.path()).read_text());assert exported==json.loads(saved)
  assert not errors,errors
  (OUT/'result.json').write_text(json.dumps({'passed':True,'evidence':'synthetic extended source, no live Beyond access','checks':['preview and weapon selection without writes','v10 create and old collection unchanged','skills and saves','configured attacks','rage and GWM choices','extra melee critical die','caster-specific spell values','innate resources','prepared state','ordinary and pact slots','short rest','state editing','temp HP absorption','profile/item/power edits preserve imported data','reload and backup','mobile 390/320','HTML treated as text'],'jsErrors':errors},indent=2))
  print('PASS: synthetic extended import and play, spellcasters/innate/pact resources, attacks/effects, state/editing/backup, old sheets preserved, mobile; no JS errors.')
  browser.close()
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
