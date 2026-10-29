import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {prepareSdAssets,installSdAssets,SD_PACKAGE} from './sd-assets.js';
import {sendDeviceCopy} from './device-usb.js';
import {projectDevice} from './device-export.js';
import {initialState} from './model.js';
import {crc32} from './device-backup.js';
const original=new Uint8Array(readFileSync(new URL('../public/assets/classes.pdc',import.meta.url)));
const supplied=original.slice(),handle=await prepareSdAssets({bytes:supplied});supplied.fill(0);
const timeouts={open:1000,hello:1000,send:1000,data:1000,assetsBegin:1000,assetsFinish:1000};
function serial({already=false,old=false,failAt='',noDone=false,wrongAck=false,onChunk=()=>{},lostHello=false}={}){
 let output,frame='',offset=0,closed=0,chosen=0,chunks=0,hellos=0;const commands=[],data=new Uint8Array(original.length),enc=new TextEncoder();
 const push=text=>{output.enqueue(enc.encode(text));};
 const port={readable:new ReadableStream({start(c){output=c;}}),writable:new WritableStream({write(bytes){frame+=new TextDecoder().decode(bytes);let i;while((i=frame.indexOf('\n'))>=0){const line=frame.slice(0,i);frame=frame.slice(i+1);
  if(!line.startsWith('PDD ASSET WRITE'))commands.push(line);
  if(line==='PDD HELLO'){hellos++;if(!lostHello||hellos>1)push(lostHello?'PDD READY 17\nPDD READY 17\n':'PDD READY 17\n');}
  else if(line==='PDD ASSET HELLO 1')push(old?'PDD ERROR packet\n':'PDD ASSET READY 1\n');
  else if(line.startsWith('PDD ASSET BEGIN')){assert.equal(line,`PDD ASSET BEGIN 1 ${original.length} ${SD_PACKAGE.sha256}`);push(failAt==='begin'?'PDD ASSET ERROR sd\n':already?`PDD ASSET DONE 1 ${SD_PACKAGE.sha256}\n`:'PDD ASSET OFFSET 0\n');}
  else if(line.startsWith('PDD ASSET WRITE')){const match=/^PDD ASSET WRITE 1 (\d+) ([a-f0-9]{8}) ([a-f0-9]+)$/.exec(line);assert.ok(match);const part=Uint8Array.from(Buffer.from(match[3],'hex'));assert.equal(Number(match[1]),offset);assert.equal(crc32(part),Number.parseInt(match[2],16));assert.ok(part.length<=2048);data.set(part,offset);offset+=part.length;chunks++;onChunk();push(failAt==='write'?'PDD ASSET ERROR storage\n':`PDD ASSET OFFSET ${wrongAck?offset+1:offset}\n`);}
  else if(line.startsWith('PDD ASSET FINISH')){assert.equal(offset,original.length);assert.deepEqual(data,original);if(!noDone)push(failAt==='finish'?'PDD ASSET ERROR integrity\n':`PDD ASSET DONE 1 ${SD_PACKAGE.sha256}\n`);}
 }},abort(){}}),open:async()=>{},close:async()=>{closed++},setSignals:async()=>{}};
 return {serial:{requestPort:async()=>{chosen++;return port;}},commands,get closed(){return closed},get chosen(){return chosen},get chunks(){return chunks}};
}
test('SD package is pinned and private; a mutated caller buffer cannot change bytes sent, and DONE is required',async()=>{
 const f=serial(),progress=[],phases=[];const result=await installSdAssets(handle,{serial:f.serial,timeouts,onAssetProgress:n=>progress.push(n),onProgress:p=>phases.push(p)});
 assert.deepEqual(result,{installed:true,already:false});assert.equal(f.chunks,3025);assert.equal(progress.at(-1),100);assert.equal(phases.at(-1),'sd-complete');assert.equal(f.closed,1);assert.ok(!f.commands.some(s=>s.startsWith('{')));
});
test('SD preflight rejects corrupt/truncated/download failures and forged handles before choosing any port',async()=>{
 const bad=original.slice();bad[2048]^=1;await assert.rejects(prepareSdAssets({bytes:bad}),/alterato/);await assert.rejects(prepareSdAssets({bytes:new Uint8Array(1)}),/alterato/);
 await assert.rejects(prepareSdAssets({fetcher:async()=>({ok:false})}),/non disponibile/);
 const f=serial();await assert.rejects(installSdAssets({size:original.length,sha256:SD_PACKAGE.sha256},{serial:f.serial,timeouts}),/Prepara/);assert.equal(f.chosen,0);
});
test('SD old firmware and device errors do not send a PG or claim success; exact already-installed result skips writes',async()=>{
 const old=serial({old:true});await assert.rejects(installSdAssets(handle,{serial:old.serial,timeouts}),/firmware 0.83/);assert.equal(old.commands.length,3);assert.equal(old.chunks,0);
 const done=serial({already:true});assert.deepEqual(await installSdAssets(handle,{serial:done.serial,timeouts}),{installed:true,already:true});assert.equal(done.chunks,0);
 for(const settings of [{failAt:'begin'},{failAt:'write'},{wrongAck:true}]){const f=serial(settings),phases=[];await assert.rejects(installSdAssets(handle,{serial:f.serial,timeouts,onProgress:p=>phases.push(p)}));assert.ok(!phases.includes('sd-complete'));assert.ok(f.commands.includes('PDD ASSET ABORT 1'));assert.equal(f.closed,1);assert.ok(!f.commands.some(s=>s.startsWith('{')));}
});
test('SD cancellation shares the character USB lock and sends one best-effort abort without replaying chunks',async()=>{
 const controller=new AbortController(),f=serial({onChunk:()=>controller.abort()}),other=serial();
 const task=installSdAssets(handle,{serial:f.serial,timeouts,signal:controller.signal});await assert.rejects(sendDeviceCopy(projectDevice(initialState()).packet,{serial:other.serial,timeouts}),/già in corso/);assert.equal(other.chosen,0);
 await assert.rejects(task,/interrotto/);assert.equal(f.chunks,1);assert.equal(f.closed,1);assert.ok(f.commands.includes('PDD ASSET ABORT 1'));
});

test('SD 100 percent received is not success without final integrity confirmation',async()=>{
 for(const settings of [{failAt:'finish'},{noDone:true}]){
  const f=serial(settings),progress=[],phases=[];
  await assert.rejects(installSdAssets(handle,{serial:f.serial,timeouts:{...timeouts,assetsFinish:20},onAssetProgress:n=>progress.push(n),onProgress:p=>phases.push(p)}));
  assert.equal(progress.at(-1),100);assert.ok(phases.includes('sd-verify'));assert.ok(!phases.includes('sd-complete'));
  assert.equal(f.commands.filter(s=>s==='PDD ASSET ABORT 1').length,1);assert.equal(f.closed,1);
 }
});

test('SD capability check consumes only the duplicate HELLO replies caused by its own retry',async()=>{
 const f=serial({already:true,lostHello:true});
 const result=await installSdAssets(handle,{serial:f.serial,timeouts:{...timeouts,helloAttempt:10}});
 assert.deepEqual(result,{installed:true,already:true});assert.equal(f.chunks,0);assert.equal(f.commands.filter(s=>s==='PDD HELLO').length,2);
});
