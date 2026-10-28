#!/usr/bin/env python3
"""Actual PC app + full converter, synthetic source, isolated browser storage only."""
import json,select,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';import {convertBeyondFull} from './src/beyond-conversion.js';import {beyondProfileFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>({...convertBeyondFull(beyondProfileFixture(),id),testFixture:true,transport:{status:200,bytes:1000,elapsedMs:1,authenticated:false}})});server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);ctx=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
  ctx.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort());page=ctx.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(address);page.locator('[data-action=beyond-import]').wait_for();key='pocketdd-library-v1'
  def raw():return page.evaluate('(key)=>localStorage.getItem(key)',key)
  before=raw();page.locator('[data-action=beyond-import]').click();page.locator('[name=character]').fill('123456');page.locator('[data-read] button').click();expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-reference-rows]')).to_contain_text('Passo: 30 ft');expect(page.locator('[data-reference-rows]')).to_contain_text('Bonus +1');expect(page.locator('[data-reference-rows]')).to_contain_text('5 lb/unita');expect(page.locator('[data-checks]')).to_contain_text('DATI SINTETICI');assert raw()==before
  page.locator('[data-reference] summary').click()
  for width in [320,390,1440]:
   page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
  lib=json.loads(raw());assert lib['characters'][:-1]==json.loads(before)['characters'];s=lib['characters'][-1]['state'];assert s['version']==20 and sum('reference' in v for v in s['powers'])>=7
  page.locator('[data-action=extended-state]').click();expect(page.locator('dialog')).to_contain_text('Profilo e dotazioni importati');expect(page.locator('dialog')).to_contain_text('Corda sonda x2');page.locator('dialog [data-close]').click()
  page.locator('[data-go-page=actions]').click();page.locator('[data-action-tab=powers]').click();assert page.locator('.power-card').filter(has_text='Fotografia all importazione').count()==0
  page.locator('[data-action=device-export]').click();expect(page.locator('dialog')).to_contain_text('USB17');expect(page.locator('dialog')).to_contain_text('Dato del profilo / fotografia')
  with page.expect_download() as dl:page.locator('[data-device-download]').click()
  packet=json.loads(Path(dl.value.path()).read_text());assert packet['version']==17;assert sum(v['effect']>=4 for v in packet['magic']['powers'])>=7;assert all(v['flags']&16 and not v['flags']&4 for v in packet['magic']['powers'] if v['effect']>=4);assert packet['inventory']['items'][0]['quantity']==2
  page.reload();lib2=json.loads(raw());assert lib2==lib;assert not errors,errors;browser.close()
 print('PASS full v1: synthetic preview/reference/readiness, unchanged old sheets, confirmed new v20 PG, profile consultation separate from operational powers, reload, exact USB17 download and mobile widths; no live Beyond/physical USB/personal collection.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
