import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {installClassLooks,classLookPixels} from './class-looks.js';
import {crc32} from './device-backup.js';
test('Class pack: verified full sprites, species/body/class identity, strict corrupted assets and retained last verified pack',()=>{
 const source=new Uint8Array(readFileSync(new URL('../public/assets/classes.pdc',import.meta.url)));installClassLooks(source);
 assert.equal(classLookPixels({outfit:0}),null);
 for(let lineage=0;lineage<21;lineage++)for(let body=0;body<2;body++){
  const distinct=new Set();for(let outfit=1;outfit<13;outfit++){const {rgba,width,height}=classLookPixels({lineage,body,outfit});assert.equal(width,64);assert.equal(height,64);assert.equal(rgba.length,16384);assert.ok(rgba.some((a,i)=>i%4===3&&a===255));distinct.add(crc32(rgba));}assert.equal(distinct.size,12);
 }
 const original=classLookPixels({lineage:0,body:0,outfit:9});
 for(const mutate of [b=>b.subarray(0,20),b=>{b[6]=65;return b},b=>{b[28]^=1;return b},b=>{b[32]^=1;return b},b=>{b[2048]^=1;return b},b=>new Uint8Array(b.length+1)])assert.throws(()=>installClassLooks(mutate(source.slice())));
 assert.deepEqual(classLookPixels({lineage:0,body:0,outfit:9}),original);
 for(const avatar of [{outfit:13},{outfit:1,lineage:21},{outfit:1,body:2}])assert.throws(()=>classLookPixels(avatar));
});
