#!/usr/bin/env python3
"""Synthetic inventory configuration in a disposable browser. No personal collection or Beyond."""
import argparse,json
from pathlib import Path
from playwright.sync_api import sync_playwright
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--url',default='http://127.0.0.1:5181')
args=parser.parse_args()
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
 context=browser.new_context(viewport={'width':1280,'height':950})
 context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(args.url+'/') else r.abort())
 page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 page.goto(args.url,wait_until='networkidle')
 page.evaluate("""async()=>{
  const {initialState}=await import('/src/model.js');
  const {createLibrary,addCharacter,serializeLibrary}=await import('/src/library.js');
  const lib=addCharacter(createLibrary(initialState(),'keep'),{...initialState(),name:'Sonda cariche'},'configure');
  localStorage.setItem('pocketdd-library-v1',serializeLibrary(lib));
 }""")
 page.reload(wait_until='networkidle')
 def raw():return page.evaluate("()=>localStorage.getItem('pocketdd-library-v1')")
 before=json.loads(raw());original=raw()
 page.locator('[data-action="manage-inventory"]').first.click()
 page.locator('[data-edit-item="starter-sword"]').click()
 assert not page.locator('[name="charges-enabled"]').is_checked()
 assert not page.locator('[name="charge-current"]').is_enabled()
 page.locator('[name="charges-enabled"]').check()
 page.locator('[name="charge-current"]').fill('3');page.locator('[name="charge-max"]').fill('7');page.locator('[name="charge-recovery"]').select_option('short')
 page.locator('[data-cancel-item]').click();assert raw()==original
 page.locator('[data-edit-item="starter-sword"]').click();page.locator('[name="charges-enabled"]').check()
 page.locator('[name="charge-current"]').fill('8');page.locator('[name="charge-max"]').fill('7')
 page.locator('#item-editor [type="submit"]').click();assert raw()==original
 assert 'Cariche oggetto non valide' in page.locator('.editor-error').inner_text()
 page.locator('[name="charge-current"]').fill('3');page.locator('[name="charge-recovery"]').select_option('short')
 page.locator('#item-editor [type="submit"]').click()
 after=json.loads(raw());assert after['characters'][0]==before['characters'][0]
 state=after['characters'][1]['state'];assert state['version']==11 and state['inventory'][0]['charges']=={'current':3,'max':7,'recovery':'short'} and state['inventory'][0]['quantity']==1
 assert 'Cariche 3/7' in page.locator('.inventory-manager-list').inner_text()
 page.locator('[data-edit-item="starter-sword"]').click()
 assert page.locator('[name="charge-recovery"]').input_value()=='short'
 out=Path('/tmp/pocketdd-charges-qa');out.mkdir(exist_ok=True);page.screenshot(path=str(out/'charges-pc.png'),full_page=True)
 page.locator('[data-inventory-close]').click();page.reload(wait_until='networkidle')
 page.locator('[data-action="manage-inventory"]').first.click();page.locator('[data-edit-item="starter-sword"]').click()
 assert page.locator('[name="charge-current"]').input_value()=='3'
 for width in [390,320]:
  page.set_viewport_size({'width':width,'height':850})
  assert page.evaluate('()=>document.documentElement.scrollWidth<=innerWidth')
 page.locator('[name="charges-enabled"]').uncheck();page.locator('#item-editor [type="submit"]').click()
 final=json.loads(raw());assert 'charges' not in final['characters'][1]['state']['inventory'][0] and final['characters'][0]==before['characters'][0]
 assert not errors,errors
 (out/'result.json').write_text(json.dumps({'passed':True,'evidence':'synthetic disposable browser only','checks':['draft cancellation','invalid balance rejected','save/reload','disable pool','other character preserved','320/390px','no JS errors']}))
 context.close();browser.close()
 print('PC charges configuration: cancel/invalid/save/reload/remove, other character preserved, 320/390px and no JS errors passed (synthetic).')
