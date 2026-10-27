import {readFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import assert from 'node:assert/strict';
import {installClassLooks,classLookPixels} from '../src/class-looks.js';
import {crc32} from '../src/device-backup.js';
const b=new Uint8Array(readFileSync('public/assets/classes.pdc'));installClassLooks(b);
const run=spawnSync('firmware/tests/out/ui-build/class-looks',['public/assets/classes.pdc','--hashes'],{encoding:'utf8'});assert.equal(run.status,0,run.stderr);
const lines=run.stdout.trim().split('\n');assert.equal(lines.length,1512);
for(const line of lines){const [lineage,body,outfit,scale,hash]=line.split(' '),side=32*Number(scale),out=new Uint8Array(side*side*3),index=(Number(lineage)*2+Number(body))*12+Number(outfit)-1,frame=b.subarray(32+504*4+index*12288);
 const pixels=classLookPixels({lineage:Number(lineage),body:Number(body),outfit:Number(outfit)});assert.equal(pixels.rgba.length,16384);
 for(let y=0;y<side;y++)for(let x=0;x<side;x++){const src=Math.floor(y*64/side)*64+Math.floor(x*64/side),dst=y*side+x;out[dst*2]=frame[src*2];out[dst*2+1]=frame[src*2+1];out[side*side*2+dst]=frame[8192+src];}
 assert.equal(crc32(out).toString(16).padStart(8,'0'),hash);
}
console.log('Class catalog C/JS: 1512 exact RGB565A8 frame/scale comparisons passed (504 full looks); no personal character or SD hardware read');
