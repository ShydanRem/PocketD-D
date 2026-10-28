import {test} from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {initialState} from './model.js';
import {beyondCannonFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {projectDevice} from './device-export.js';
import {crc32} from './device-backup.js';
import {sendDeviceCopy,receiveDeviceBackup,restoreDeviceBackup,checkDeviceConnection} from './device-usb.js';
import {createUsbDiagnostics,formatUsbDiagnostics} from './device-usb-diagnostics.js';
const raw=readFileSync(new URL('../firmware/tests/fixtures/device-backup-v1.synthetic.json',import.meta.url),'utf8');
const packet=()=>projectDevice(initialState()).packet,encode=x=>new TextEncoder().encode(x);
function fake({version=9,fragment=23,backup=raw,header='',error='',cancel=false,noAck=false,eof=false,unexpected='',openError=false,confirm=null,writeError=false,startup='',missHello=0,lateReady=false,partialReady=false}={}){
 const log=[],sent=[],writes=[],signals=[];let controller,frame='',chooser=0,closed=0,cancelled=0,aborted=0,opened=false,hellos=0,lateLeft=missHello,partialLeft=partialReady;
 const push=text=>{const bytes=typeof text==='string'?encode(text):text;for(let i=0;i<bytes.length;i+=fragment)controller.enqueue(bytes.subarray(i,i+fragment));};
 const ack=text=>{if(error)push('PDD ERROR '+error+'\n');else if(cancel)push('PDD CANCELLED user\n');else if(!noAck)push(unexpected||text);if(eof)controller.close();};
 const port={
  readable:new ReadableStream({start(c){controller=c;},cancel(){cancelled++;}}),
  writable:new WritableStream({write(bytes){if(writeError)throw Error('USB write failed');writes.push(bytes.slice());frame+=new TextDecoder().decode(bytes);let at;while((at=frame.indexOf('\n'))>=0){const line=frame.slice(0,at);frame=frame.slice(at+1);sent.push(line);
   if(line==='PDD HELLO'||line==='PDD BACKUP HELLO'){
    if(++hellos<=missHello){if(partialReady)push('PDD RE');continue;}
    const ready=line==='PDD HELLO'?`PDD READY ${version}`:`PDD BACKUP_READY ${version===9?1:version}`;
    if(partialLeft){push(`ADY ${version}\n`);partialLeft=false;}
    push(ready+'\r\n');if(lateReady){for(let n=0;n<lateLeft;n++)push(ready+'\n');lateLeft=0;}
   }
   else if(line==='PDD BACKUP GET 1'){
    push('PDD WAIT_BACKUP\n');confirm?.();
    if(cancel||error||noAck||unexpected)ack('');else{push(header||`PDD BACKUP_DATA ${encode(backup).length} ${crc32(encode(backup)).toString(16).padStart(8,'0')}\n`);push(backup);push('\nPDD BACKUP_DONE\n');if(eof)controller.close();}
   }else if(line.startsWith('{')){const data=JSON.parse(line);push(data.format==='pocketdd-device'?'PDD WAIT_CONFIRM\n':'PDD WAIT_RESTORE\n');confirm?.();ack(data.format==='pocketdd-device'?'PDD IMPORTED\n':'PDD RESTORED\n');}
  }},abort(){aborted++;}}),
  async open(options){log.push(options);if(openError)throw Error('USB busy');opened=true;if(startup)push(startup);},async setSignals(value){signals.push({...value});},async close(){closed++;opened=false;}
 };
 const serial={async requestPort(){chooser++;return port;}};
 return {serial,port,push,sent,writes,log,signals,get chooser(){return chooser},get closed(){return closed},get cancelled(){return cancelled},get aborted(){return aborted},get opened(){return opened}};
}
const times={open:500,hello:500,send:500,confirm:30,data:2000};
function options(f,onProgress){return {serial:f.serial,timeouts:times,onProgress};}
function released(f){assert.equal(f.closed,1);assert.equal(f.port.readable.locked,false);assert.equal(f.port.writable.locked,false);assert.equal(f.cancelled,1);assert.equal(f.aborted,1);assert.deepEqual(f.log,[{baudRate:115200,bufferSize:8192}]);assert.deepEqual(f.signals,[{dataTerminalReady:false,requestToSend:false}]);}
test('USB browser: fragmented real streams negotiate USB9, send the frozen copy and require the final hardware ACK',async()=>{
 for(const fragment of [1,7,4096]){
  const f=fake({fragment}),states=[],p=packet(),before=JSON.stringify(p);await sendDeviceCopy(p,options(f,s=>states.push(s)));
  assert.deepEqual(states,['connect','send','confirm-import','imported']);assert.deepEqual(f.sent.slice(0,2),['','PDD HELLO']);assert.equal(f.sent[2],before);assert.equal(JSON.stringify(p),before);assert.ok(f.writes.every(w=>w.length<=1024));released(f);
 }
});
test('USB browser: old/future protocol, malformed copy, chooser cancellation and occupied port do not send a candidate',async()=>{
 for(const version of [1,8,17]){const f=fake({version});await assert.rejects(sendDeviceCopy(packet(),options(f)));assert.deepEqual(f.sent,['','PDD HELLO']);released(f);}
 for(const p of [{},null,{format:'pocketdd-device',version:true},{format:'pocketdd-device',version:9,name:'é'},{format:'pocketdd-device',version:9,name:'x'.repeat(262144)}]){const f=fake();await assert.rejects(sendDeviceCopy(p,options(f)));assert.equal(f.chooser,0);}
 await assert.rejects(sendDeviceCopy(packet(),{serial:{requestPort:async()=>{throw new DOMException('No port','NotFoundError')}}}),/annullata/);
 const busy=fake({openError:true});await assert.rejects(sendDeviceCopy(packet(),options(busy)),/busy/);assert.equal(busy.writes.length,0);
});
test('USB browser: device cancellation/storage/busy errors, timeout, wrong ACK and unplug are never reported as success',async()=>{
 for(const settings of [{cancel:true},{error:'storage'},{error:'busy'},{noAck:true},{unexpected:'PDD RESTORED\n'},{noAck:true,eof:true},{writeError:true}]){
  const f=fake(settings),states=[];await assert.rejects(sendDeviceCopy(packet(),options(f,s=>states.push(s))));assert.ok(!states.includes('imported'));assert.equal(f.port.readable.locked,false);assert.equal(f.port.writable.locked,false);assert.equal(f.closed,1);
 }
});
test('USB browser: native backup exact bytes/CRC and integrity verified across fragmented headers and payload',async()=>{
 // Byte-by-byte fragmentation uses the normal transfer deadline, including on busy build hosts.
 for(const fragment of [1,13,4096]){const f=fake({fragment}),states=[];assert.equal(await receiveDeviceBackup({...options(f,s=>states.push(s)),timeouts:{...times,data:30000}}),raw);assert.deepEqual(f.sent,['','PDD BACKUP HELLO','PDD BACKUP GET 1']);assert.deepEqual(states,['connect','confirm-backup','receive','received']);released(f);}
});
test('USB browser: oversized/corrupt/truncated backup or unsupported protocol never yields a verified copy',async()=>{
 const corrupt=JSON.parse(raw);corrupt.state='AAAA'+corrupt.state.slice(4);
 for(const settings of [{header:'PDD BACKUP_DATA 110001 00000000\n'},{header:`PDD BACKUP_DATA ${encode(raw).length} 00000000\n`},{header:`PDD BACKUP_DATA ${encode(raw).length+10} 00000000\n`,eof:true},{backup:JSON.stringify(corrupt)},{backup:'{}'},{version:2},{cancel:true},{error:'backup-history'},{noAck:true}]){
  const f=fake(settings),states=[];await assert.rejects(receiveDeviceBackup(options(f,s=>states.push(s))));assert.ok(!states.includes('received'));assert.equal(f.closed,1);
 }
});
test('USB browser: native restore validates before choosing a port and requires the separate restore confirmation/ACK',async()=>{
 const f=fake(),states=[];await restoreDeviceBackup(raw,options(f,s=>states.push(s)));assert.deepEqual(JSON.parse(f.sent[2]),JSON.parse(raw));assert.deepEqual(states,['connect','send','confirm-restore','restored']);released(f);
 for(const bad of ['{}',raw+'garbage',raw.replace('"version":1','"version":2')]){const f=fake();await assert.rejects(restoreDeviceBackup(bad,options(f)));assert.equal(f.chooser,0);}
 const old=fake({version:2});await assert.rejects(restoreDeviceBackup(raw,options(old)));assert.deepEqual(old.sent,['','PDD BACKUP HELLO']);
 for(const settings of [{cancel:true},{error:'storage'},{noAck:true},{unexpected:'PDD IMPORTED\n'}]){const f=fake(settings),states=[];await assert.rejects(restoreDeviceBackup(raw,options(f,s=>states.push(s))));assert.ok(!states.includes('restored'));}
});
test('USB browser: one task at a time, explicit abort releases the port, and a new transaction works afterward',async()=>{
 const controller=new AbortController(),f=fake({noAck:true,confirm:()=>controller.abort()}),states=[];
 const first=sendDeviceCopy(packet(),{...options(f,s=>states.push(s)),signal:controller.signal});
 const g=fake();await assert.rejects(receiveDeviceBackup(options(g)),/già in corso/);assert.equal(g.chooser,0);await assert.rejects(first,/interrotto/);assert.ok(!states.includes('imported'));released(f);
 await sendDeviceCopy(packet(),options(g));released(g);
});
test('USB browser: bounded line/buffer, stale ACK and corrupt negotiation are rejected',async()=>{
 for(const text of ['x'.repeat(257)+'\n','PDD IMPORTED\n','\u0000\n','PDD READY 17\n','x'.repeat(112000)]){
  const f=fake();f.serial.requestPort=async()=>{f.push(text);return f.port;};await assert.rejects(sendDeviceCopy(packet(),options(f)));assert.ok(!f.sent.some(x=>x.startsWith('{')));
 }
});
test('USB browser: payload/version stay frozen while the chooser is open; late opening after timeout is closed',async()=>{
 const p=packet(),original=JSON.stringify(p),f=fake();f.serial.requestPort=async()=>{p.hp=0;return f.port;};await sendDeviceCopy(p,options(f));assert.equal(f.sent[2],original);
 const q=packet(),old=fake({version:8});old.serial.requestPort=async()=>{q.version=1;return old.port;};await assert.rejects(sendDeviceCopy(q,options(old)));assert.deepEqual(old.sent,['','PDD HELLO']);
 const late=fake();let closedAfterOpening=false;const close=late.port.close;
 late.port.open=()=>new Promise(resolve=>setTimeout(()=>{closedAfterOpening=false;resolve();},25));late.port.close=async()=>{closedAfterOpening=true;return close();};
 await assert.rejects(sendDeviceCopy(packet(),{...options(late),timeouts:{...times,open:5}}));await new Promise(r=>setTimeout(r,40));assert.equal(closedAfterOpening,true);assert.equal(late.sent.length,0);
});
test('USB browser: bounded ESP32 startup lines and one lost HELLO recover without repeating the import or backup request',async()=>{
 const startup='ESP-ROM:esp32s3-20210327\r\nBuild:Mar 27 2021\r\nrst:0x15 (USB_UART_CHIP_RESET),boot:0x8 (SPI_FAST_FLASH_BOOT)\r\nSPIWP:0xee\r\nmode:DIO, clock div:1\r\nload:0x3fce2820,len:0x1708\r\nentry 0x403c8700\r\n\x1b[0;32mI (220) cpu_start: Starting scheduler\x1b[0m\r\n';
 for(const task of ['import','receive','restore']){
  const f=fake({startup,missHello:1,lateReady:true,fragment:1}),states=[],o={...options(f,s=>states.push(s)),timeouts:{...times,helloAttempt:10,data:30000}};
  if(task==='import')await sendDeviceCopy(packet(),o);else if(task==='receive')assert.equal(await receiveDeviceBackup(o),raw);else await restoreDeviceBackup(raw,o);
  assert.equal(f.sent.filter(s=>s.endsWith('HELLO')).length,2);
  assert.equal(f.sent.filter(s=>s.startsWith('{')||s==='PDD BACKUP GET 1').length,1);
  assert.ok(states.includes(task==='import'?'imported':task==='receive'?'received':'restored'));released(f);
 }
 const partial=fake({missHello:1,partialReady:true});await sendDeviceCopy(packet(),{...options(partial),timeouts:{...times,helloAttempt:10}});assert.equal(partial.sent.filter(s=>s.startsWith('{')).length,1);released(partial);
});
test('USB browser: bootloader, unrecognized text, excess logs and unknown READY remain errors with no character data exposed',async()=>{
 for(const [startup,pattern] of [['ESP-ROM:esp32s3-20210327\nwaiting for download\n',/bootloader/],['ESP-ROM:esp32s3-20210327\n'.repeat(513),/Limite dei log/],['{"name":"private-sheet-marker"}\n',/testo non riconosciuto/],['PDD READY 17\n',/PDD READY 17/]]){
  const f=fake({startup});let message='';await assert.rejects(sendDeviceCopy(packet(),options(f)),e=>{message=e.message;return pattern.test(message);});assert.doesNotMatch(message,/private-sheet-marker/);assert.ok(!f.sent.some(s=>s.startsWith('{')));released(f);
 }
 const f=fake({startup:'I (24) boot: starting\n',unexpected:'PDD RESTORED\n'});
 await assert.rejects(sendDeviceCopy(packet(),options(f)),/salvataggio della scheda \(PDD RESTORED\)/);assert.equal(f.sent.filter(s=>s.startsWith('{')).length,1);
});
test('USB browser: unavailable firmware has a bounded HELLO retry; later missing ACK never retries the candidate',async()=>{
 const silent=fake({missHello:1000});await assert.rejects(sendDeviceCopy(packet(),{...options(silent),timeouts:{...times,hello:35,helloAttempt:10}}),/non risponde/);assert.ok(!silent.sent.some(s=>s.startsWith('{')));released(silent);
 const f=fake({noAck:true,missHello:1}),states=[];await assert.rejects(sendDeviceCopy(packet(),{...options(f,s=>states.push(s)),timeouts:{...times,helloAttempt:10}}),/non confermato/);assert.equal(f.sent.filter(s=>s.startsWith('{')).length,1);assert.ok(!states.includes('imported'));released(f);
});
test('USB diagnosis: a long single startup is not a reset-loop verdict and check sends HELLO only',async()=>{
 const startup='ESP-ROM:esp32s3-20210327\n'+'I (50) boot: synthetic log\n'.repeat(120)+'rst:0x15 (USB_UART_CHIP_RESET),boot:0x8 (SPI_FAST_FLASH_BOOT)\n';
 const f=fake({startup});f.port.getInfo=()=>({usbVendorId:0x303a,usbProductId:0x1001,serialNumber:'private-serial',name:'private-sheet-marker'});let report;
 const result=await checkDeviceConnection({...options(f),onDiagnostic:d=>report=d});assert.equal(result.usbVersion,9);assert.deepEqual(f.sent,['','PDD HELLO']);released(f);
 assert.equal(report.logLines,122);assert.equal(report.romBanners,1);assert.deepEqual(report.resets,[{code:'0x15',reason:'USB_UART_CHIP_RESET'}]);assert.equal(report.outcome,'completed');
 assert.equal(report.usbVendorId,0x303a);assert.equal(report.usbProductId,0x1001);assert.match(formatUsbDiagnostics(report),/nessun PG inviato/);assert.doesNotMatch(JSON.stringify(report),/private-|synthetic log/);
});
test('USB diagnosis: failed handshake and failed save retain distinct phase, reset/fault metadata and unconfirmed outcome',async()=>{
 const f=fake({missHello:1000,startup:'ESP-ROM:esp32s3-20210327\nrst:0xc (SW_CPU_RESET),boot:0x8\nGuru Meditation Error: Core 0 panic\'ed (LoadProhibited)\nBacktrace: 0x40371234:0x3fc91234 0x42012345:0x3fc91250\n'});let report;
 await assert.rejects(checkDeviceConnection({...options(f),onDiagnostic:d=>report=d,timeouts:{...times,hello:35,helloAttempt:10}}),e=>e.usbDiagnostic?.fault==='LoadProhibited');
 assert.equal(report.outcome,'unconfirmed');assert.equal(report.phase,'collegamento');assert.equal(report.usbVersion,null);assert.equal(report.romBanners,1);assert.equal(report.backtrace.length,4);assert.ok(f.sent.every(s=>s===''||s==='PDD HELLO'));released(f);
 const g=fake({noAck:true});await assert.rejects(sendDeviceCopy(packet(),{...options(g),onDiagnostic:d=>report=d}));assert.equal(report.phase,'salvataggio della scheda');assert.equal(report.usbVersion,9);assert.equal(report.outcome,'unconfirmed');
});
test('USB diagnosis: byte/line budgets stay bounded and arbitrary logs/JSON/register text are not disclosed',()=>{
 const d=createUsbDiagnostics('check');assert.equal(d.line('{"name":"private-sheet-marker"}'),false);
 assert.equal(d.line('I (12) app: private-sheet-marker'),true);assert.equal(d.line('A0      : 0x12345678 A1 : 0x87654321'),true);
 assert.equal(d.line('rst:0xc (not a public reason <b>)'),true);assert.deepEqual(d.snapshot().resets,[]);
 assert.doesNotMatch(formatUsbDiagnostics(d.snapshot()),/private-sheet-marker|12345678|87654321|<b>/);
 const e=createUsbDiagnostics('check');assert.throws(()=>{for(let i=0;i<100;i++)e.line('I (1) app: '+'x'.repeat(490));},/Limite dei log/);assert.ok(e.snapshot().logLines<100);
});

test('USB10 species: supported firmware receives exact identity; older firmware receives no character',async()=>{
 const state=initialState();Object.assign(state.avatar,{lineage:4,body:1});const {packet:p}=projectDevice(state);assert.equal(p.version,10);
 const old=fake({version:9});await assert.rejects(sendDeviceCopy(p,options(old)),/USB10/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:10});await sendDeviceCopy(p,options(current));assert.equal(JSON.parse(current.sent[2]).avatar.body,1);assert.equal(JSON.parse(current.sent[2]).avatar.lineage,4);released(current);
});

test('USB11: new animal species never send a candidate to USB10 firmware',async()=>{
 const state=initialState();Object.assign(state.avatar,{lineage:15,body:1});const {packet:p}=projectDevice(state);assert.equal(p.version,11);
 const old=fake({version:10});await assert.rejects(sendDeviceCopy(p,options(old)),/USB11/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:11});await sendDeviceCopy(p,options(current));assert.deepEqual(JSON.parse(current.sent[2]).avatar,p.avatar);released(current);
});

test('USB12: extended species never send a candidate to USB11 firmware',async()=>{
 const state=initialState();Object.assign(state.avatar,{lineage:20,body:1});const {packet:p}=projectDevice(state);assert.equal(p.version,12);
 const old=fake({version:11});await assert.rejects(sendDeviceCopy(p,options(old)),/USB12/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:12});await sendDeviceCopy(p,options(current));assert.deepEqual(JSON.parse(current.sent[2]).avatar,p.avatar);released(current);
});

// New class looks must never be silently dropped by an older firmware.
test('USB13 class preset preserves species/body and rejects older firmware before sending',async()=>{
 const state=initialState();Object.assign(state.avatar,{lineage:20,body:1,outfit:9});const {packet:p}=projectDevice(state);assert.equal(p.version,13);
 const old=fake({version:12});await assert.rejects(sendDeviceCopy(p,options(old)),/USB13/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:13});await sendDeviceCopy(p,options(current));assert.deepEqual(JSON.parse(current.sent[2]).avatar,p.avatar);released(current);
});

test('USB14 defence profile never travels to USB13 and retains the exact verified masks',async()=>{
 const {emptyExtended}=await import('./extended.js');const s={...initialState(),extended:{...emptyExtended(),defences:{configured:32,resistance:32,vulnerability:0,immunity:0}}};const {packet:p}=projectDevice(s);assert.equal(p.version,14);
 const old=fake({version:13});await assert.rejects(sendDeviceCopy(p,options(old)),/USB14/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:14});await sendDeviceCopy(p,options(current));assert.deepEqual(JSON.parse(current.sent[2]).defences,p.defences);released(current);
});

test('USB15 skill associations reject USB14 before sending and preserve every mapped ability',async()=>{
 const {emptyExtended}=await import('./extended.js');const s={...initialState(),extended:{...emptyExtended(),skillAbilities:{intimidation:'FOR',perception:'INT'}}};const p=projectDevice(s).packet;assert.equal(p.version,15);
 const old=fake({version:14});await assert.rejects(sendDeviceCopy(p,options(old)),/USB15/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:15});await sendDeviceCopy(p,options(current));assert.deepEqual(JSON.parse(current.sent[2]).training.skillAbilities,p.training.skillAbilities);released(current);
});

test('USB16 cannon: older firmware receives HELLO only; current firmware receives exact feature effect and requires final ACK',async()=>{
 const p=projectDevice(convertBeyond(beyondCannonFixture(),'123456').draft).packet;assert.equal(p.version,16);
 const old=fake({version:15});await assert.rejects(sendDeviceCopy(p,options(old)),/USB16/);assert.deepEqual(old.sent,['','PDD HELLO']);released(old);
 const current=fake({version:16});await sendDeviceCopy(p,options(current));assert.equal(JSON.parse(current.sent[2]).magic.powers.find(v=>v.effect===3).kind,1);released(current);
});
