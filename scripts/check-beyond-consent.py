#!/usr/bin/env python3
"""Compiled import controls on synthetic valid/blocked/inconsistent reports."""
from pathlib import Path
import json,subprocess
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
reports=json.loads(subprocess.check_output(['node','--input-type=module','-e',"""
import {convertBeyondFull} from './src/beyond-conversion.js';import {beyondProfileFixture} from './src/beyond-fixtures.js';
const make=p=>({...convertBeyondFull(p,'123456'),testFixture:true,transport:{status:200,bytes:1000}});
const valid=make(beyondProfileFixture()),p=beyondProfileFixture();p.data.characterValues=[{typeId:12345,value:1}];const blocked=make(p);
const incomplete={...structuredClone(valid),draft:null};console.log(JSON.stringify({valid,blocked,incomplete}));
"""],cwd=ROOT,text=True))
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
 for guided in [False,True]:
  for width,height in [(1280,700),(390,700),(320,600)]:
   page=browser.new_page(viewport={'width':width,'height':height});errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
   current=['blocked']
   def handle(route):
    if route.request.url.endswith('/api/probe'):route.fulfill(status=200,content_type='application/json',body=json.dumps({'ok':True,'report':reports[current[0]]}))
    elif route.request.is_navigation_request():route.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text())
    else:route.abort()
   page.route('**/*',handle);page.goto('http://pocket.test');before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
   if guided:
    page.locator('[data-action=setup]').click();page.locator('[data-setup-ready]').click();page.locator('[data-setup-import]').click()
   else:page.locator('[data-action=beyond-import]').click()
   imp=page.locator('.beyond-import');imp.locator('[name=character]').fill('123456')
   for mode in ['blocked','incomplete']:
    current[0]=mode;imp.locator('[data-read] button').click();expect(imp.locator('[data-status]')).to_contain_text('Importazione ferma');expect(imp.locator('[data-consent]')).not_to_be_visible();expect(imp.locator('[data-create]')).not_to_be_visible();expect(imp.locator('[data-consent-status]')).to_contain_text('conversione è bloccata');assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
   current[0]='valid';imp.locator('[data-read] button').click();expect(imp.locator('[data-accept]')).to_be_enabled();expect(imp.locator('[data-create]')).to_be_disabled()
   imp.locator('[data-consent]').click();expect(imp.locator('[data-accept]')).to_be_checked();expect(imp.locator('[data-create]')).to_be_enabled()
   # Real pointer click on the visible input, including short/phone viewports.
   imp.locator('[data-accept]').click();expect(imp.locator('[data-create]')).to_be_disabled();imp.locator('[data-consent]').click()
   page.on('dialog',lambda dialog:dialog.accept());imp.locator('[data-create]').click();expect(imp).to_have_count(0)
   old=json.loads(before);after=json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'));assert len(after['characters'])==len(old['characters'])+1 and after['characters'][:-1]==old['characters'];assert not errors,errors
   page.close()
 browser.close()
print('PASS Beyond consent: actual blocked/incomplete reports have explicit reasons and no dead checkbox; reread valid report allows label/input clicks and one create in classic/guided flows at 1280/390/320, existing synthetic sheets retained. No real Beyond/USB.')
