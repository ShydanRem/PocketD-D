import {test} from 'node:test';
import assert from 'node:assert/strict';
import {createHash,webcrypto} from 'node:crypto';
import {request} from 'node:http';
import {mkdtemp,writeFile,mkdir,rm} from 'node:fs/promises';
import {tmpdir} from 'node:os';
import {pathToFileURL} from 'node:url';
import {BOARD,validateManifest,prepareFirmware,installFirmware} from '../web-install/flasher.js';
import {createInstallerServer} from '../scripts/installer-server.js';
const hash=d=>createHash('sha256').update(d).digest('hex');
function fixture(){
 const parts=[{address:0,data:new Uint8Array(64).fill(233)},{address:0x8000,data:new Uint8Array(0xc00).fill(170)},{address:0x10000,data:new Uint8Array(512).fill(42)}];
 return {manifest:{schema:1,name:'PocketD-D',version:'0.64.0',board:BOARD,chip:'ESP32-S3',flash_size:0x1000000,parts:parts.map((p,i)=>({id:['bootloader','partitions','app'][i],path:`firmware/${['bootloader','partitions','app'][i]}.bin`,offset:p.address,size:p.data.length,sha256:hash(p.data)}))},parts};
}
function fake(prepared,{chip='ESP32-S3',size='16MB',badLayout=false,failure='',resetError=false}={}){
 const trace=[],writes=[];
 class Transport{async disconnect(){trace.push('disconnect');}}
 class Loader{
  chip={CHIP_NAME:chip};
  async main(){trace.push('main');if(failure==='main')throw Error('connect failed');}
  async detectFlashSize(){return size;}
  async readFlash(address,length){trace.push(['read',address,length]);if(failure==='read')throw Error('read failed');return badLayout?new Uint8Array(length):prepared.parts[1].data;}
  async writeFlash(options){
   trace.push('write');writes.push(options);assert.equal(options.eraseAll,false);assert.equal(options.flashMode,'keep');assert.equal(options.flashFreq,'keep');assert.equal(options.flashSize,'keep');assert.equal(options.compress,true);assert.equal(typeof options.calculateMD5Hash,'function');
   assert.deepEqual(options.fileArray,prepared.parts);assert.equal(options.calculateMD5Hash(new Uint8Array([1,2,3])),'test-md5');
   for(let i=0;i<3;i++)options.reportProgress(i,1,2);
   if(failure==='write')throw Error('MD5 of file does not match data in flash!');
  }
  async after(mode){trace.push(mode);if(resetError)throw Error('reset failed');}
 }
 return {trace,writes,Loader,TransportClass:Transport};
}
const md5=()=> 'test-md5';
test('Installer: known target, exact firmware regions and sector boundaries reject writes into saved data',()=>{
 assert.equal(validateManifest(fixture().manifest).chip,'ESP32-S3');
 for(const mutate of [m=>m.schema=2,m=>m.board+='C',m=>m.chip='ESP32',m=>m.flash_size=0x800000,m=>m.parts.pop(),m=>m.parts[0].offset=0x9000,m=>m.parts[0].size=0x8001,m=>m.parts[1].size=0x1001,m=>m.parts[2].size=0x400001,m=>m.parts[2].offset=0x410000,m=>m.parts[0].path='../secret.bin',m=>m.parts[0].path='https://example.com/x.bin',m=>m.parts[0].sha256='0',m=>m.parts[0].size=1.5,m=>m.version='x']){
  const m=fixture().manifest;mutate(m);assert.throws(()=>validateManifest(m));
 }
});
test('Installer: all firmware bytes are downloaded and SHA256 checked before connection',async()=>{
 const p=fixture(),calls=[],fetcher=async u=>{calls.push(String(u));return String(u).endsWith('manifest.json')?{ok:true,json:async()=>p.manifest}:{ok:true,arrayBuffer:async()=>p.parts[p.manifest.parts.findIndex(part=>String(u).endsWith(part.path))].data.buffer};};
 const output=await prepareFirmware(new URL('https://installer.example/nested/manifest.json'),{fetcher,cryptoProvider:webcrypto});assert.deepEqual(output,p);assert.equal(calls.length,4);assert.equal(calls[3],'https://installer.example/nested/firmware/app.bin');
 await assert.rejects(prepareFirmware(new URL('https://installer.example/manifest.json'),{fetcher:async()=>({ok:false}),cryptoProvider:webcrypto}),/non disponibile/);
 await assert.rejects(prepareFirmware(new URL('https://installer.example/manifest.json'),{fetcher:async u=>String(u).endsWith('manifest.json')?{ok:true,json:async()=>p.manifest}:{ok:false},cryptoProvider:webcrypto}),/Download/);
 const corrupt=async u=>String(u).endsWith('manifest.json')?{ok:true,json:async()=>p.manifest}:{ok:true,arrayBuffer:async()=>new Uint8Array(64).buffer};
 await assert.rejects(prepareFirmware(new URL('https://installer.example/manifest.json'),{fetcher:corrupt,cryptoProvider:webcrypto}),/incompleto o alterato/);
});
test('Installer: first/update write three separate regions without full erase; update compares the actual device layout',async()=>{
 for(const mode of ['first','update']){
  const prepared=fixture(),f=fake(prepared),states=[];const result=await installFirmware({prepared,port:{},mode,...f,md5,onStatus:(s,v)=>states.push([s,v])});
  assert.equal(result.restarted,true);assert.equal(f.writes.length,1);assert.equal(f.trace.at(-1),'disconnect');assert.equal(states.at(-1)[0],'done');assert.equal(f.trace.some(e=>Array.isArray(e)),mode==='update');assert.ok(states.filter(([s])=>s==='write').every(([,v])=>v<100));
 }
});
test('Installer: unsupported chip/flash/layout, connect/read errors and MD5 failures do not report success; port is released',async()=>{
 for(const options of [{chip:'ESP32'},{size:'8MB'},{badLayout:true},{failure:'main'},{failure:'read'},{failure:'write'}]){
  const prepared=fixture(),f=fake(prepared,options),states=[];
  await assert.rejects(installFirmware({prepared,port:{},mode:'update',...f,md5,onStatus:s=>states.push(s)}));
  assert.equal(f.writes.length,options.failure==='write'?1:0);assert.equal(f.trace.at(-1),'disconnect');assert.ok(!states.includes('done')&&!states.includes('restart'));
 }
});
test('Installer: reset failure after verified writing is distinct from write failure; malformed draft is rejected before connecting',async()=>{
 const prepared=fixture(),f=fake(prepared,{resetError:true}),states=[];
 const result=await installFirmware({prepared,port:{},mode:'first',...f,md5,onStatus:s=>states.push(s)});assert.equal(result.restarted,false);assert.equal(states.at(-1),'done-reset');
 const bad=fixture();bad.parts[2].address=0x410000;const g=fake(bad);await assert.rejects(installFirmware({prepared:bad,mode:'first',...g,md5}),/File/);assert.deepEqual(g.trace,[]);
});
test('Installer server: loopback, binary MIME, invalid paths/origins/hosts and write requests',async()=>{
 const dir=await mkdtemp(`${tmpdir()}/pdd-installer-`);await mkdir(`${dir}/firmware`);await writeFile(`${dir}/index.html`,'<!doctype html>');await writeFile(`${dir}/firmware/app.bin`,new Uint8Array([0,255,233]));
 const server=createInstallerServer(pathToFileURL(dir+'/'));await new Promise(r=>server.listen(0,'127.0.0.1',r));
 try{
  const root=`http://127.0.0.1:${server.address().port}`;
  assert.equal((await fetch(root+'/')).status,200);
  const bin=await fetch(root+'/firmware/app.bin');assert.equal(bin.headers.get('content-type'),'application/octet-stream');assert.equal(bin.headers.get('x-content-type-options'),'nosniff');assert.ok((await bin.arrayBuffer()).byteLength>0);
  for(const path of ['/../package.json','/%2e%2e/package.json','/firmware/../../package.json','/.git/config','/manifest.json?x','/assets/../../package.json'])assert.equal((await fetch(root+path)).status,404);
  assert.equal((await fetch(root+'/',{headers:{origin:'https://evil.example'}})).status,403);
  const denied=await new Promise((resolve,reject)=>{const req=request(root+'/',{headers:{host:'evil.example'}},res=>{res.resume();resolve(res.statusCode);});req.on('error',reject);req.end();});assert.equal(denied,403);
  assert.equal((await fetch(root+'/',{method:'POST'})).status,405);
 }finally{await new Promise(r=>server.close(r));await rm(dir,{recursive:true,force:true});}
});
