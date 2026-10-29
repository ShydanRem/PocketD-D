#!/usr/bin/env python3
"""Production PC UI against simulated Web Serial streams. No physical USB or personal collection."""
import json,os,subprocess,time,urllib.request
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1];url='http://127.0.0.1:8792/'
server=subprocess.Popen(['node','scripts/app-server.js'],cwd=ROOT,env={**os.environ,'POCKETDD_APP_PORT':'8792'},stdout=subprocess.DEVNULL)
raw=(ROOT/'firmware/tests/fixtures/device-backup-v1.synthetic.json').read_text()
mock=r'''
window.usbProbe={calls:0,sent:[],action:'pending',closed:0,cancelled:0,confirmed:0,boot:true,signals:[]};
const encode=x=>new TextEncoder().encode(x);
const crc=bytes=>{let c=0xffffffff;for(const b of bytes){c^=b;for(let i=0;i<8;i++)c=(c>>>1)^((c&1)?0xedb88320:0);}return ((c^0xffffffff)>>>0).toString(16).padStart(8,'0');};
Object.defineProperty(navigator,'serial',{value:{requestPort:async()=>{
 const probe=window.usbProbe;probe.calls++;if(probe.action==='chooser-cancel')throw new DOMException('No device selected','NotFoundError');
 let controller,frame='',hellos=0;const boot=probe.boot;probe.boot=false;const push=text=>{const bytes=encode(text);for(let i=0;i<bytes.length;i+=17)controller.enqueue(bytes.subarray(i,i+17));};
 const ack=line=>{probe.confirm=()=>{probe.confirmed++;push(probe.action==='cancel'?'PDD CANCELLED user\n':probe.action==='error'?'PDD ERROR storage\n':line);};if(probe.action!=='pending')probe.confirm();};
 return {readable:new ReadableStream({start(c){controller=c;},cancel(){probe.cancelled++;}}),writable:new WritableStream({write(bytes){frame+=new TextDecoder().decode(bytes);let at;while((at=frame.indexOf('\n'))>=0){const line=frame.slice(0,at);frame=frame.slice(at+1);probe.sent.push(line);
 if(line==='PDD HELLO'){if(!boot||++hellos>1){push(probe.action==='wrong-protocol'?'PDD READY 18\n':'PDD READY '+(probe.readRuntime?17:9)+'\n');if(boot)push('PDD READY '+(probe.readRuntime?17:9)+'\n');}}
 else if(line==='PDD DIAGNOSTICS GET 1')push('PDD DIAGNOSTICS '+JSON.stringify(window.syntheticDiagnostics)+'\n');
 else if(line==='PDD BACKUP HELLO')push('PDD BACKUP_READY 1\n');
 else if(line==='PDD BACKUP GET 1'){push('PDD WAIT_BACKUP\n');const raw=window.syntheticNative,body=probe.action==='corrupt'?raw.slice(1):raw;ack(`PDD BACKUP_DATA ${encode(body).length} ${probe.action==='corrupt'?'00000000':crc(encode(body))}\n${body}\nPDD BACKUP_DONE\n`);}
 else if(line.startsWith('{')){const p=JSON.parse(line),copy=p.format==='pocketdd-device';push(copy?'PDD WAIT_CONFIRM\n':'PDD WAIT_RESTORE\n');ack(copy?'PDD IMPORTED\n':'PDD RESTORED\n');}
 }},abort(){}}),open:async()=>{if(boot)push('ESP-ROM:esp32s3-20210327\r\nrst:0x15 (USB_UART_CHIP_RESET),boot:0x8 (SPI_FAST_FLASH_BOOT)\r\n'+('I (25) boot: synthetic-private-log\n'.repeat(120))+'\x1b[0;32mI (220) cpu_start: Starting scheduler\x1b[0m\r\n');},getInfo:()=>({usbVendorId:0x303a,usbProductId:0x1001,serialNumber:'private-serial'}),setSignals:async value=>probe.signals.push(value),close:async()=>{probe.closed++;}};
}}});
'''
try:
 for _ in range(60):
  try:urllib.request.urlopen(url,timeout=.5);break
  except Exception:time.sleep(.1)
 else:raise RuntimeError('App server did not start')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
  context=browser.new_context(viewport={'width':1280,'height':900});context.add_init_script('window.syntheticDiagnostics='+json.dumps(json.loads((ROOT/'firmware/tests/fixtures/diagnostics-v1.synthetic.json').read_text()))+';window.syntheticNative='+json.dumps(raw)+';'+mock)
  page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(url,wait_until='networkidle');assert page.evaluate('typeof window.usbProbe')=='object';assert not errors,errors
  page.locator('[data-hp="-1"]').click();before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
  page.locator('[data-action="device-export"]').click();dialog=page.locator('dialog');dialog.locator('[data-device-send]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('conferma entro 120')
  expect(dialog.locator('[data-device-close]')).to_be_disabled();expect(dialog.locator('[data-device-download]')).to_be_disabled();page.keyboard.press('Escape');expect(dialog).to_be_visible();page.screenshot(path='/workspace/PocketD-D-usb-invio.png',full_page=True)
  assert page.evaluate('JSON.parse(window.usbProbe.sent.find(x=>x.startsWith("{"))).version')==9
  assert page.evaluate('window.usbProbe.sent.filter(x=>x==="PDD HELLO").length')==2
  assert page.evaluate('window.usbProbe.sent.filter(x=>x.startsWith("{")).length')==1
  assert page.evaluate('window.usbProbe.signals')==[{'dataTerminalReady':False,'requestToSend':False}]
  page.evaluate('window.usbProbe.confirm()');expect(dialog.locator('[data-usb-status]')).to_contain_text('confermata e salvata');expect(dialog.locator('[data-device-close]')).to_be_enabled();assert page.evaluate('window.usbProbe.closed')==1;assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
  expect(dialog.locator('[data-usb-diagnostic-text]')).to_contain_text('Log: 123 righe')
  expect(dialog.locator('[data-usb-diagnostic-text]')).to_contain_text('0x303a / PID 0x1001')
  assert 'synthetic-private-log' not in dialog.locator('[data-usb-diagnostic-text]').inner_text()
  sent=page.evaluate('window.usbProbe.sent.length');dialog.locator('[data-usb-check]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('Nessun personaggio inviato')
  assert page.evaluate('(at)=>window.usbProbe.sent.slice(at)',sent)==['','PDD HELLO']
  expect(dialog.locator('[data-usb-diagnostic-text]')).to_contain_text('verifica collegamento, nessun PG inviato')
  assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
  sent=page.evaluate('window.usbProbe.sent.length');page.evaluate('window.usbProbe.action="wrong-protocol"');dialog.locator('[data-usb-check]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('PDD READY 18')
  expect(dialog.locator('[data-usb-diagnostic-text]')).to_contain_text('Esito: non confermato')
  assert page.evaluate('(at)=>window.usbProbe.sent.slice(at)',sent)==['','PDD HELLO']
  page.evaluate('window.usbProbe.action="pending"')
  # Explicit abort: never becomes saved; it releases the port and preserves the collection.
  dialog.locator('[data-device-send]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('conferma entro 120');dialog.locator('[data-usb-abort]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('interrotto');expect(dialog.locator('[data-device-close]')).to_be_enabled();dialog.locator('[data-device-close]').click()
  page.evaluate('window.usbProbe.action="ok"');page.locator('[data-action="device-backup"]').click();dialog=page.locator('dialog');dialog.locator('[data-native-receive]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('ricevuto e verificato');expect(dialog.locator('[data-native-values]')).to_contain_text('PF 17/42 (+7 temporanei) · CA 18')
  with page.expect_download() as event:dialog.locator('[data-native-download]').click()
  assert Path(event.value.path()).read_bytes()==raw.encode();assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
  page.evaluate('window.usbProbe.action="pending"');dialog.locator('[data-native-restore]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('conferma entro 120');expect(dialog.locator('[data-native-file]')).to_be_disabled();page.evaluate('window.usbProbe.confirm()');expect(dialog.locator('[data-usb-status]')).to_contain_text('Ripristino confermato');page.screenshot(path='/workspace/PocketD-D-usb-backup.png',full_page=True)
  # A corrupt incoming backup cannot replace a previously verified copy.
  page.evaluate('window.usbProbe.action="corrupt"');dialog.locator('[data-native-receive]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('corrotto');expect(dialog.locator('[data-native-restore]')).to_be_enabled()
  dialog.locator('[data-native-file]').set_input_files({'name':'bad.json','mimeType':'application/json','buffer':b'{}'});expect(dialog.locator('[data-native-restore]')).to_be_disabled();expect(dialog.locator('[data-native-download]')).to_be_disabled()
  # Fresh C PDD29 encoder output exercises the new field and shortcut in the actual UI.
  fresh=ROOT/'firmware/tests/out/backup-fixture.json';dialog.locator('[data-native-file]').set_input_files(fresh)
  expect(dialog.locator('[data-native-preview]')).to_contain_text('Sfondo: Rovine lunari')
  expect(dialog.locator('[data-native-preview]')).to_contain_text('Tasti: Aspetto')
  with page.expect_download() as event:dialog.locator('[data-native-download]').click()
  assert Path(event.value.path()).read_bytes()==fresh.read_bytes()
  assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before;assert not errors,errors
  dialog.locator('[data-close]').click();page.locator('[data-action="device-export"]').click();dialog=page.locator('dialog')
  # Retain the failed transfer while reading firmware; both attempts go in the report.
  page.evaluate("window.usbProbe.action='error'");dialog.locator('[data-device-send]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('non confermato')
  page.evaluate("window.usbProbe.action='pending';window.usbProbe.readRuntime=true;window.usbProbe.sent=[]")
  dialog.locator('[data-usb-read-diagnostics]').click();expect(dialog.locator('[data-usb-status]')).to_contain_text('Diagnosi firmware letta')
  assert page.evaluate('window.usbProbe.sent')==['','PDD HELLO','PDD DIAGNOSTICS GET 1']
  with page.expect_download() as report_download:dialog.locator('[data-usb-report]').click()
  report=report_download.value;assert report.suggested_filename=='PocketDD-diagnosi-USB.txt';text=Path(report.path()).read_text()
  assert '0.82.0-synthetic' in text and 'App PC:' in text and 'Esito: non confermato' in text
  assert all(secret not in text for secret in ['private-serial','synthetic-private-log','pocketdd-device','"name":','"abilities":'])
  assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before;assert not errors,errors
  other=browser.new_context();other.add_init_script('delete Navigator.prototype.serial');fallback=other.new_page();fallback.goto(url);fallback.locator('[data-action="device-export"]').click();expect(fallback.locator('[data-device-send]')).to_be_disabled();expect(fallback.locator('[data-device-download]')).to_be_enabled();expect(fallback.locator('[data-usb-status]')).to_contain_text('Chrome o Edge')
  browser.close()
 print('Browser USB UI: 123 startup logs/lost HELLO, one candidate, sanitized diagnostics, check-only success/failure sends HELLO only, frozen import/mandatory ACK, guards/abort, native backup/restore, integrity/fallback and unchanged disposable collection passed (simulated USB; no hardware)')
finally:server.terminate();server.wait(timeout=5)
