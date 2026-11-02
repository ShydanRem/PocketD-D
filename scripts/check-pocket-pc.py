#!/usr/bin/env python3
"""Production extension UI served locally with cookie/serial doubles only."""
import base64,http.server,json,threading,time,zlib
from functools import partial
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
class Handler(http.server.SimpleHTTPRequestHandler):
 def log_message(self,*args):pass
server=http.server.ThreadingHTTPServer(('127.0.0.1',0),partial(Handler,directory=str(ROOT/'extension')))
threading.Thread(target=server.serve_forever,daemon=True).start()
diagnostic=json.loads((ROOT/'firmware/tests/fixtures/diagnostics-v1.synthetic.json').read_text());diagnostic['firmware']='0.93.0-synthetic';diagnostic['formats']['usb']=21;diagnostic['formats']['state']=40
backup=(ROOT/'firmware/tests/fixtures/device-backup-v1.synthetic.json').read_text()
mock=r'''
window.chooserCalls=0;window.secretWrites=0;
let reader,pending='';const nonce='0123456789abcdef0123456789abcdef',d=__DIAGNOSTIC__,backup=BACKUP,crc=CRC,hash='d439cacf0de37f60c16d3dec4e59fefe7cb19e9bf0d9feb10e3fec651d31950f';
const push=text=>reader.enqueue(new TextEncoder().encode(text));window.approvePocket=()=>push('PDD ACCOUNT ACCEPT 1 '+nonce+'\n');
const port={open:async()=>{pending='';port.readable=new ReadableStream({start(c){reader=c}});port.writable=new WritableStream({write(bytes){pending+=new TextDecoder().decode(bytes);let at;while((at=pending.indexOf('\n'))>=0){const line=pending.slice(0,at);pending=pending.slice(at+1);if(line==='PDD HELLO')push('PDD READY 21\n');else if(line==='PDD DIAGNOSTICS GET 1')push('PDD DIAGNOSTICS '+JSON.stringify(d)+'\n');else if(line==='PDD BACKUP HELLO')push('PDD BACKUP_READY 1\n');else if(line==='PDD BACKUP GET 1')push('PDD WAIT_BACKUP\nPDD BACKUP_DATA '+new TextEncoder().encode(backup).length+' '+crc+'\n'+backup+'\nPDD BACKUP_DONE\n');else if(line==='PDD ASSET HELLO 1')push('PDD ASSET READY 1\n');else if(line.startsWith('PDD ASSET BEGIN'))push('PDD ASSET DONE 1 '+hash+'\n');else if(line==='PDD ACCOUNT HELLO 1')push('PDD ACCOUNT CODE 1 '+nonce+' 123456\n');else if(line.startsWith('PDD ACCOUNT OFFER')){window.secretWrites++;push('PDD ACCOUNT LINKED 1 '+nonce+'\n');}}}})},close:async()=>{},setSignals:async()=>{}};
Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{window.chooserCalls++;return port;}}});
window.chrome={permissions:{contains:async()=>true,request:async()=>true,remove:async()=>true},cookies:{get:async()=>({value:'synthetic-session-only',expirationDate:Date.now()/1000+3600})}};
'''
mock=mock.replace('__DIAGNOSTIC__',json.dumps(diagnostic)).replace('BACKUP,crc=',json.dumps(backup)+',crc=').replace('CRC,hash=',json.dumps(f'{zlib.crc32(backup.encode()):08x}')+',hash=')
try:
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);context=browser.new_context(accept_downloads=True,viewport={'width':1000,'height':1000});context.add_init_script(mock)
  local=f'http://127.0.0.1:{server.server_port}/'
  def route(r):
   u=r.request.url
   if u.startswith(local):r.continue_();return
   headers={'Access-Control-Allow-Origin':local.rstrip('/'),'Access-Control-Allow-Credentials':'true','Access-Control-Allow-Headers':'authorization,content-type','Access-Control-Allow-Methods':'POST,GET,OPTIONS','Content-Type':'application/json'}
   if r.request.method=='OPTIONS':r.fulfill(status=204,headers=headers,body='');return
   if u=='https://shydanrem.github.io/PocketD-D/assets/classes.pdc':r.fulfill(status=200,headers=headers,body=(ROOT/'public/assets/classes.pdc').read_bytes());return
   if u=='https://auth-service.dndbeyond.com/v1/cobalt-token':
    payload={'http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier':'1234','exp':int(time.time())+120};part=base64.urlsafe_b64encode(json.dumps(payload).encode()).decode().rstrip('=');data={'token':'e30.'+part+'.c3ludGhldGlj'}
   elif u=='https://character-service.dndbeyond.com/character/v5/characters/list?userId=1234':data={'success':True,'data':{'characters':[{'id':123456,'name':'Eroe sintetico','level':3}]}}
   else:raise AssertionError('Unexpected remote endpoint')
   r.fulfill(status=200,headers=headers,body=json.dumps(data))
  context.route('**/*',route);page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(local+'account.html')
  page.locator('#device-looks').click();expect(page.locator('#device-status')).to_contain_text('già aggiornati',timeout=15000);assert page.evaluate('chooserCalls')==1
  page.locator('#connect').click();expect(page.locator('#pair-code')).to_have_text('123456');assert page.evaluate('chooserCalls')==1 and page.evaluate('secretWrites')==0;expect(page.locator('#device-backup')).to_be_disabled()
  page.evaluate('approvePocket()');expect(page.locator('#link-status')).to_contain_text('Beyond collegato');assert page.evaluate('secretWrites')==1
  with page.expect_download() as saved:page.locator('#device-backup').click()
  assert Path(saved.value.path()).read_text()==backup;expect(page.locator('#device-status')).to_contain_text('Backup verificato')
  page.locator('#device-looks').click();expect(page.locator('#device-status')).to_contain_text('già aggiornati',timeout=15000);assert page.evaluate('chooserCalls')==1
  page.locator('#device-file').set_input_files({'name':'bad.json','mimeType':'application/json','buffer':b'{}'});expect(page.locator('#device-status')).not_to_contain_text('già aggiornati');assert page.evaluate('chooserCalls')==1
  page.locator('#connection-info>summary').click();page.locator('#device-change').click();expect(page.locator('#device-status')).to_contain_text('firmware 0.93.0-synthetic');assert page.evaluate('chooserCalls')==2
  assert 'synthetic-session-only' not in page.locator('body').inner_text() and page.evaluate('localStorage.length')==0
  for width in [1000,390,320]:page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  page.set_viewport_size({'width':1000,'height':1000});page.locator('#connection-info>summary').click();page.locator('main').screenshot(path='/workspace/PocketDD-PC-hub-preview.png');assert not errors,errors;browser.close()
 print('PASS Pocket PC: one chooser across real UI connection/account/native backup/SD, fresh physical approval before secret, exclusive operations, explicit change, malformed restore rejected, responsive and no secret in DOM/storage. All Chrome, Beyond and hardware responses synthetic.')
finally:server.shutdown();server.server_close()
