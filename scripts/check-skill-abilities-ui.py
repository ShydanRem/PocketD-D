#!/usr/bin/env python3
"""Synthetic source ability changes, PC profile editing, wire export and native backup preview."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyond} from './src/beyond-conversion.js';import {beyondRichFixture} from './src/beyond-fixtures.js';
const cv=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{const p=beyondRichFixture();p.data.id=Number(id);p.data.characterValues=[cv(27,17,id==='222222'?7:1),cv(27,14,4),cv(26,17,4),cv(24,17,1)];return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:1,authenticated:false}};}});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.goto(address);page.locator('[data-action=beyond-import]').wait_for();key='pocketdd-library-v1'
  def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
  def active():return json.loads(raw())['characters'][-1]['state']
  before=raw();page.locator('[data-action=beyond-import]').click()
  def read(id):
   page.locator('[name=character]').fill(id);page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible()
  read('222222');expect(page.locator('[data-accept]')).to_be_disabled();assert raw()==before
  read('123456');expect(page.locator('[data-training]')).to_contain_text('Intimidire: +7');expect(page.locator('[data-training]')).to_contain_text('FOR +2');assert raw()==before
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  assert json.loads(raw())['characters'][:-1]==json.loads(before)['characters'];assert active()['version']==17 and active()['extended']['skillAbilities']['intimidation']=='FOR'
  saved=raw();page.locator('[data-action=edit]').click();editor=page.locator('#hero-editor');expect(editor.locator('[name=skill-ability-intimidation]')).to_have_value('FOR');editor.locator('[name=FOR]').fill('20');editor.locator('[data-close]').first.click();assert raw()==saved
  page.locator('[data-action=edit]').click();editor.locator('[name=FOR]').fill('20');editor.locator('button[type=submit]').click();expect(editor).to_have_count(0);assert active()['abilities']['FOR']==20
  page.locator('[data-action=device-export]').click();expect(page.locator('dialog')).to_contain_text('USB15')
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==15 and packet['training']['skillAbilities']['intimidation']==1 and packet['training']['skillAbilities']['perception']==4 and packet['abilities']['FOR']==20
  page.locator('[data-action=edit]').click();editor.locator('[name=skill-ability-intimidation]').select_option('INT');quota_before=raw();page.evaluate("()=>{Storage.prototype.setItem=function(){throw new DOMException('quota','QuotaExceededError')}}");editor.locator('button[type=submit]').click();expect(editor.locator('.editor-error')).not_to_be_empty();assert raw()==quota_before;editor.locator('[data-close]').first.click();page.locator('[data-action=edit]').click();expect(editor.locator('[name=skill-ability-intimidation]')).to_have_value('FOR');editor.locator('[data-close]').first.click();page.reload()
  page.locator('[data-action=edit]').click()
  for width in [390,320,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  editor.locator('[name=skill-ability-intimidation]').select_option('');editor.locator('[name=skill-ability-perception]').select_option('');editor.locator('button[type=submit]').click();assert 'skillAbilities' not in active()['extended'] and active()['version']==11
  page.locator('[data-action=device-export]').click()
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  old=json.loads(Path(dl.value.path()).read_text());assert old['version']==9 and 'skillAbilities' not in old['training']
  page.locator('[data-action=device-backup]').click();page.locator('[data-native-file]').set_input_files(ROOT/'firmware/tests/out/backup-fixture.json');expect(page.locator('[data-native-skill-abilities]')).to_contain_text('Intimidire: COS')
  with page.expect_download() as dl:page.locator('[data-native-download]').click()
  assert Path(dl.value.path()).read_bytes()==(ROOT/'firmware/tests/out/backup-fixture.json').read_bytes();assert not errors,errors;browser.close()
 print('PASS: synthetic type-27 preview/new sheet, PC ability editing/cancel/atomic quota failure, dynamic scores, USB15, explicit default removal/legacy fallback, PDD34 backup preview/exact download and mobile layout. No real Beyond or physical USB.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
