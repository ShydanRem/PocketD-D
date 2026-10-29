#!/usr/bin/env python3
"""Compiled UI, synthetic collection and local corrections; no Beyond/USB IO."""
from pathlib import Path
import json,subprocess
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
fixture=subprocess.check_output(['node','--input-type=module','-e',"""
import {initialState} from './src/model.js';import {emptyExtended} from './src/extended.js';
import {createLibrary,addCharacter,serializeLibrary} from './src/library.js';
const old=initialState();old.name='Altra scheda sintetica';const lib=createLibrary(old,'old');
const s=initialState();s.name='Controllo sintetico';s.extended=emptyExtended();s.extended.blockedChecks=['skill:arcana','save:COS'];
s.importInfo={provider:'dndbeyond',characterId:'123456',scope:'extended-v1',limitations:['Regola sintetica non automatizzata.']};
console.log(serializeLibrary(addCharacter(lib,s,'review')));
"""],cwd=ROOT,text=True).strip()
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
 page=browser.new_page(viewport={'width':1280,'height':1000});errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 page.add_init_script('localStorage.setItem("pocketdd-library-v1",'+json.dumps(fixture)+');')
 page.route('**/*',lambda r:r.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if r.request.is_navigation_request() else r.abort())
 page.goto('http://pocket.test');page.locator('[data-action=review-import]').click();review=page.locator('.import-review')
 expect(review).to_contain_text('Arcano');expect(review).to_contain_text('Salvezza COS');expect(review).to_contain_text('0/13 tipi configurati');expect(review).to_contain_text('Copia generabile')
 assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==fixture
 # Cancel does not change either character; a quota failure keeps the draft open.
 review.locator('[data-review-action=values]').first.click();editor=page.locator('#hero-editor');editor.locator('[name=armorClass]').fill('18');page.get_by_role('button',name='Chiudi editor').click()
 assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==fixture
 review.locator('[data-review-action=values]').first.click();editor.locator('[name=armorClass]').fill('18')
 page.evaluate("()=>{window.reviewSet=Storage.prototype.setItem;Storage.prototype.setItem=function(k,v){if(k==='pocketdd-library-v1')throw new DOMException('synthetic quota','QuotaExceededError');return window.reviewSet.call(this,k,v)}}")
 editor.locator('[type=submit]').click();expect(page.locator('.editor-error')).not_to_be_empty();expect(editor).to_be_visible()
 assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==fixture
 page.evaluate('()=>{Storage.prototype.setItem=window.reviewSet}');editor.locator('[type=submit]').click();expect(editor).to_have_count(0);expect(review).to_contain_text('CA 18')
 # Mechanical blocks disappear only with the existing explicit manual verification.
 review.locator('[data-review-action=state]').first.click();child=page.locator('dialog').last;child.locator('[name=verify-checks]').check();child.locator('form [type=submit]').click()
 expect(review).to_contain_text('Nessuno dei 30 tiri');expect(review).to_contain_text('Regola sintetica non automatizzata.')
 review.locator('[data-review-action=defences]').click();child=page.locator('dialog').last;child.locator('[name=defences-enabled]').check();child.locator('[data-defence-review]').click();child.locator('[data-defence-confirm]').click();child.locator('form [type=submit]').click()
 expect(review).to_contain_text('13/13 tipi configurati');expect(review).to_contain_text('resistenze condizionali')
 current=page.evaluate('localStorage.getItem("pocketdd-library-v1")');library=json.loads(current);original=json.loads(fixture)
 assert len(library['characters'])==2 and library['characters'][0]==original['characters'][0]
 for width in [1280,390,320]:
  page.set_viewport_size({'width':width,'height':1000});assert review.evaluate('(e)=>e.scrollWidth<=e.clientWidth')
 page.set_viewport_size({'width':1280,'height':1000});review.screenshot(path='/tmp/pdd82-import-review.png')
 review.locator('[data-review-action=send]').click();copy=page.locator('dialog').last;expect(copy).to_contain_text('CA 18');expect(copy.locator('[data-device-defences]')).to_contain_text('13')
 assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==current
 copy.locator('[data-device-close]').click();review.locator('[data-review-close]').click();assert not errors,errors
 browser.close()
print('PASS current-sheet review: targeted editors/cancel/quota retry, explicit unblock and defence verification, preserved source limitations/other character, live USB preview and mobile layout; synthetic only, no Beyond/USB IO')
