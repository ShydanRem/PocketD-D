import test from 'node:test';import assert from 'node:assert/strict';
import {mkdtemp,mkdir,writeFile,rm} from 'node:fs/promises';import {tmpdir} from 'node:os';import {join} from 'node:path';import {pathToFileURL} from 'node:url';import {once} from 'node:events';
import {request} from 'node:http';
import {createBeyondLabServer} from '../scripts/beyond-lab.js';
test('Setup: one origin serves the guided app, embedded firmware and capability status without exposing runtime/private paths',async t=>{
 const dir=await mkdtemp(join(tmpdir(),'pdd-setup-'));t.after(()=>rm(dir,{recursive:true,force:true}));
 await mkdir(join(dir,'install/firmware'),{recursive:true});await mkdir(join(dir,'install/assets'));
 await writeFile(join(dir,'index.html'),'<h1>Synthetic app</h1>');await writeFile(join(dir,'install/index.html'),'<h1>Synthetic installer</h1>');await writeFile(join(dir,'install/manifest.json'),'{}');await writeFile(join(dir,'install/firmware/app.bin'),new Uint8Array([0,233,255]));await writeFile(join(dir,'install/assets/main.js'),'// fixture');await writeFile(join(dir,'secret.txt'),'private fixture');
 const server=createBeyondLabServer({appRoot:pathToFileURL(dir+'/')});server.listen(0,'127.0.0.1');await once(server,'listening');t.after(()=>new Promise(r=>server.close(r)));const base=`http://127.0.0.1:${server.address().port}`;
 assert.match(await (await fetch(base+'/?setup=1')).text(),/Synthetic app/);
 const capability=await (await fetch(base+'/api/setup')).json();assert.equal(capability.kind,'pocketdd-setup');assert.equal(capability.installer,true);
 const response=await fetch(base+'/install/?guided=1');assert.equal(response.status,200);assert.ok(response.headers.get('content-security-policy').includes("frame-ancestors 'self'"));
 assert.deepEqual(new Uint8Array(await (await fetch(base+'/install/firmware/app.bin')).arrayBuffer()),new Uint8Array([0,233,255]));
 for(const path of ['/install/secret.txt','/install/../secret.txt','/install/%2e%2e/secret.txt','/runtime/node.exe','/install/downloads/PocketDD-PC.zip'])assert.equal((await fetch(base+path)).status,404);
 assert.equal((await fetch(base+'/api/setup',{headers:{Origin:'https://evil.test'}})).status,403);
 assert.equal(await new Promise((resolve,reject)=>{const req=request(base+'/install/',{headers:{Host:'evil.test'}},r=>{r.resume();resolve(r.statusCode)});req.on('error',reject);req.end();}),403);
});
test('Setup: a missing installer is reported as missing, never as installed',async t=>{
 const dir=await mkdtemp(join(tmpdir(),'pdd-no-install-'));t.after(()=>rm(dir,{recursive:true,force:true}));const server=createBeyondLabServer({appRoot:pathToFileURL(dir+'/')});server.listen(0,'127.0.0.1');await once(server,'listening');t.after(()=>new Promise(r=>server.close(r)));const base=`http://127.0.0.1:${server.address().port}`;
 assert.equal((await (await fetch(base+'/api/setup')).json()).installer,false);assert.equal((await fetch(base+'/install/manifest.json')).status,404);
});
