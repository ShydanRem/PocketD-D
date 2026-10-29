#!/usr/bin/env python3
"""Compiled SD installer with a simulated USB peer; no physical SD/Beyond access."""
import os,subprocess,time,urllib.request
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
FAKE=r"""
window.sd={mode:'normal',chosen:0,writes:0,closed:0,lines:[]};
Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{
 const s=window.sd;s.chosen++;let output,pending='',offset=0;const enc=new TextEncoder();
 const push=t=>output.enqueue(enc.encode(t+'\n'));s.done=()=>push('PDD ASSET DONE 1 d439cacf0de37f60c16d3dec4e59fefe7cb19e9bf0d9feb10e3fec651d31950f');
 return {getInfo:()=>({usbVendorId:0x303a,usbProductId:0x1001}),open:async()=>{},setSignals:async signals=>{if(signals.dataTerminalReady||signals.requestToSend)throw Error('unexpected reset')},close:async()=>{s.closed++},
 readable:new ReadableStream({start(c){output=c}}),writable:new WritableStream({write(bytes){pending+=new TextDecoder().decode(bytes);let i;while((i=pending.indexOf('\n'))>=0){const l=pending.slice(0,i);pending=pending.slice(i+1);if(!l)continue;
 if(l.startsWith('PDD ASSET WRITE ')){const m=/^PDD ASSET WRITE 1 (\d+) [a-f0-9]{8} ([a-f0-9]+)$/.exec(l);if(!m||Number(m[1])!==offset||m[2].length>4096)throw Error('invalid chunk');offset+=m[2].length/2;s.writes++;if(s.mode!=='stall')push('PDD ASSET OFFSET '+offset);continue;}
 s.lines.push(l);
 if(l==='PDD HELLO')push('PDD READY 17');
 else if(l==='PDD ASSET HELLO 1')push(s.mode==='old'?'PDD ERROR packet':'PDD ASSET READY 1');
 else if(l.startsWith('PDD ASSET BEGIN ')){if(s.mode==='missing')push('PDD ASSET ERROR sd');else if(s.mode==='already')s.done();else push('PDD ASSET OFFSET 0');}
 else if(l.startsWith('PDD ASSET FINISH ')){if(offset!==6195200)throw Error('incomplete');s.finished=true;}
 }},abort(){}})}
}}});
"""
servers=[]
try:
 for script,var,port in [('app-server.js','POCKETDD_APP_PORT',8793),('installer-server.js','POCKETDD_INSTALLER_PORT',8794)]:
  proc=subprocess.Popen(['node','scripts/'+script],cwd=ROOT,env={**os.environ,var:str(port)},stdout=subprocess.DEVNULL);servers.append(proc)
  for _ in range(60):
   try:urllib.request.urlopen(f'http://127.0.0.1:{port}/',timeout=.5);break
   except Exception:time.sleep(.1)
  else:raise RuntimeError('local server not ready')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
  context=browser.new_context(viewport={'width':1100,'height':900});context.add_init_script(FAKE)
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
  page.goto('http://127.0.0.1:8793/');page.locator('[data-action=sd-install]').wait_for();before=page.evaluate('JSON.stringify({...localStorage})')
  def open_main():
   page.locator('[data-action=sd-install]').click();root=page.locator('.sd-installer');expect(root.locator('[data-sd-install]')).to_be_enabled(timeout=15000);return root
  root=open_main();root.locator('[data-sd-install]').click();page.wait_for_function('sd.finished===true',timeout=60000)
  expect(root.locator('[data-sd-status]')).to_contain_text('Verifico');assert root.locator('progress').evaluate('(e)=>e.value')==100
  expect(root.locator('[data-sd-close]')).to_be_disabled();page.keyboard.press('Escape');expect(root).to_be_visible()
  page.screenshot(path='/workspace/PocketD-D-sd-verify.png',full_page=True)
  page.evaluate('sd.done()');expect(root.locator('[data-sd-status]')).to_contain_text('Look verificati');expect(root.locator('[data-sd-close]')).to_be_enabled()
  assert page.evaluate('sd.writes')==3025;assert page.evaluate('JSON.stringify({...localStorage})')==before
  root.locator('[data-sd-close]').click()
  for mode,message in [('missing','MicroSD non disponibile'),('old','firmware 0.83'),('already','Look verificati')]:
   page.evaluate('(mode)=>{sd.mode=mode;sd.writes=0;sd.lines=[]}',mode);root=open_main();root.locator('[data-sd-install]').click();expect(root.locator('[data-sd-status]')).to_contain_text(message)
   expect(root.locator('[data-sd-close]')).to_be_enabled();assert page.evaluate('sd.writes')==0;assert page.evaluate('sd.lines.every(l=>l.startsWith("PDD "))');assert page.evaluate('JSON.stringify({...localStorage})')==before;root.locator('[data-sd-close]').click()
  page.evaluate("sd.mode='stall';sd.writes=0;sd.lines=[]");root=open_main();root.locator('[data-sd-install]').click();page.wait_for_function('sd.writes===1');root.locator('[data-sd-abort]').click();expect(root.locator('[data-sd-status]')).to_contain_text('interrotto');expect(root.locator('[data-sd-close]')).to_be_enabled();assert page.evaluate('sd.lines.filter(l=>l==="PDD ASSET ABORT 1").length')==1;root.locator('[data-sd-close]').click()
  page.route('**/assets/classes.pdc',lambda route:route.fulfill(body=b'corrupt'));chosen=page.evaluate('sd.chosen');page.locator('[data-action=sd-install]').click();root=page.locator('.sd-installer');expect(root.locator('[data-sd-status]')).to_contain_text('alterato');expect(root.locator('[data-sd-install]')).to_be_disabled();assert page.evaluate('sd.chosen')==chosen;root.locator('[data-sd-close]').click();page.unroute('**/assets/classes.pdc')
  # Entry point inside the editor must retain the unsaved appearance draft.
  page.locator('[data-action=edit]').click();editor=page.locator('.hero-editor-dialog');editor.locator('[name=outfit]').select_option('9');editor.locator('[data-editor-sd]').click();root=page.locator('.sd-installer');expect(root.locator('[data-sd-install]')).to_be_enabled();root.locator('[data-sd-close]').click();expect(editor.locator('[name=outfit]')).to_have_value('9');editor.get_by_role('button',name='Annulla',exact=True).click();assert page.evaluate('JSON.stringify({...localStorage})')==before
  # Hosted page uses the same uploader and prevents firmware operations concurrently.
  page.goto('http://127.0.0.1:8794/');expect(page.locator('#status')).to_contain_text('Firmware pronto');page.locator('#board').check();expect(page.locator('#install')).to_be_enabled()
  root=page.locator('[data-sd-root]');expect(root.locator('[data-sd-install]')).to_be_enabled();root.locator('[data-sd-install]').click();page.wait_for_function('sd.finished===true',timeout=60000);expect(page.locator('#install')).to_be_disabled();expect(root.locator('[data-sd-status]')).to_contain_text('Verifico')
  for width in [1100,390,320]:
   page.set_viewport_size({'width':width,'height':900});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.screenshot(path='/workspace/PocketD-D-sd-installer-mobile.png',full_page=True);page.evaluate('sd.done()');expect(root.locator('[data-sd-status]')).to_contain_text('Look verificati');expect(page.locator('#install')).to_be_enabled()
  assert not errors,errors;browser.close()
 print('PASS compiled SD web UI: full 3025-chunk transfer, final ACK distinct from 100%, protected modal, cancel, old firmware, absent SD, already installed, corrupt download before chooser, editor draft/collection preserved, installer interlock and 1100/390/320 layout. USB peer simulated; no hardware SD/Beyond test.')
finally:
 for server in servers:server.terminate();server.wait(timeout=5)
