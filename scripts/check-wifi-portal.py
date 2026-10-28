#!/usr/bin/env python3
"""Production portal HTML with a fake SDK/backend. No network/radio or real password."""
import subprocess,urllib.parse
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
html=subprocess.check_output([str(ROOT/'firmware/tests/out/ui-build/wifi-service'),'--page'],text=True)
assert '<html' in html and 'SYNTHETIC-PASSWORD' not in html
state={'saved':False,'failed':False,'storage':False};accepted=True;posts=[]
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);ctx=browser.new_context(viewport={'width':390,'height':844});page=ctx.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 def route(r):
  if r.request.url=='http://192.168.4.1/':r.fulfill(status=200,body=html,content_type='text/html; charset=utf-8')
  elif r.request.url.endswith('/connect'):
   posts.append(urllib.parse.parse_qs(r.request.post_data,keep_blank_values=True));r.fulfill(status=202 if accepted else 400,body='ok' if accepted else 'invalid')
  elif r.request.url.endswith('/state'):r.fulfill(status=200,json=state)
  else:r.abort()
 ctx.route('**/*',route);page.goto('http://192.168.4.1/');result=page.locator('#result');page.locator('[name=ssid]').fill('Rete_sintetica');page.locator('[name=password]').fill('SYNTHETIC-PASSWORD');page.locator('button').click();expect(result).to_contain_text('Connessione in corso');assert posts[-1]['password']==['SYNTHETIC-PASSWORD'] and len(posts[-1]['token'][0])==32;expect(page.locator('[name=password]')).to_have_value('');state['saved']=True;expect(result).to_contain_text('Rete collegata e salvata',timeout=5000)
 # A rejected later request must not be overwritten by the old successful status.
 accepted=False;page.locator('[name=password]').fill('12345678');page.locator('button').click();expect(result).to_contain_text('Dati non validi');page.wait_for_timeout(2200);expect(result).to_contain_text('Dati non validi')
 # Explicit open-network choice clears a prior password and still sends the empty field.
 accepted=True;state.update(saved=False,failed=False,storage=False);page.locator('[name=open]').check();expect(page.locator('[name=password]')).to_have_value('');assert not page.locator('[name=password]').evaluate('(e)=>e.required');page.locator('button').click();expect(result).to_contain_text('Connessione in corso');assert posts[-1]['open']==['1'] and posts[-1]['password']==[''];state['failed']=True;expect(result).to_contain_text('Connessione non riuscita',timeout=5000)
 state.update(saved=False,failed=False,storage=False);page.locator('[name=open]').uncheck();assert page.locator('[name=password]').evaluate('(e)=>e.required');page.locator('[name=password]').fill('12345678');page.locator('button').click();expect(result).to_contain_text('Connessione in corso');state['storage']=True;expect(result).to_contain_text('Salvataggio della rete non confermato',timeout=5000)
 for width in [320,390,1280]:
  page.set_viewport_size({'width':width,'height':900});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
 assert not errors,errors;browser.close()
print('PASS production Wi-Fi portal: synthetic credentials only, WPA2/open form semantics, token, async success/failure/storage, no stale success after rejected form, mobile widths and no JavaScript errors. No physical RF/NVS tested.')
