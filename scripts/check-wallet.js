// Synthetic PC -> actual strict USB9 C parser. Never reads a personal character.
import {spawnSync} from 'node:child_process';
import {writeFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {initialState} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {projectDevice} from '../src/device-export.js';
const keys=['cp','sp','ep','gp','pp'],rates=[1n,10n,50n,100n,1000n],inputs=[],expected=[];
const values=[0,1,9,10,49,50,99,100,999,65535,65536,999999998,999999999];
for(let offset=0;offset<values.length;offset++)for(let focus=0;focus<5;focus++){
 const s=initialState();s.extended=emptyExtended();s.extended.currenciesAvailable=true;
 keys.forEach((k,i)=>s.extended.currencies[k]=values[i===focus?offset:(i*3+offset*7)%values.length]);
 const before=structuredClone(s),packet=projectDevice(s).packet;assert.deepEqual(s,before);
 inputs.push(JSON.stringify(packet));const coins=keys.map(k=>s.extended.currencies[k]);
 expected.push({valid:true,available:true,coins,copper:Number(coins.reduce((n,c,i)=>n+BigInt(c)*rates[i],0n))});
}
const base=projectDevice(initialState()).packet;
inputs.push(JSON.stringify(base));expected.push({valid:true,available:false,coins:[0,0,0,0,0],copper:0});
for(const key of keys)for(const value of [-1,1000000000,4294967295,1.5,'1',null]){
 const packet=structuredClone(base);packet.wallet.available=true;packet.wallet[key]=value;inputs.push(JSON.stringify(packet));expected.push({valid:false});
}
for(const key of ['available',...keys]){const packet=structuredClone(base);delete packet.wallet[key];inputs.push(JSON.stringify(packet));expected.push({valid:false});}
inputs.push(JSON.stringify(base).replace('"available":false,"cp"','"available":false,"available":false,"cp"'));expected.push({valid:false});
// USB8 is an exact prior schema; deeper old schemas are covered by core fixtures.
const old=structuredClone(base);old.version=8;delete old.wallet;inputs.push(JSON.stringify(old));expected.push({valid:true,available:false,coins:[0,0,0,0,0],copper:0});
const phantom=structuredClone(base);phantom.wallet.cp=1;inputs.push(JSON.stringify(phantom));expected.push({valid:false});
const path='firmware/tests/out/wallet-input.jsonl';writeFileSync(path,inputs.join('\n')+'\n');
const run=spawnSync('firmware/tests/out/wallet-probe',[path],{encoding:'utf8',maxBuffer:4*1024*1024,timeout:15000});
assert.equal(run.status,0,run.stderr);const rows=run.stdout.trim().split('\n').map(JSON.parse);assert.deepEqual(rows,expected);
console.log(`PC -> USB9 -> C wallet: ${rows.length} exact balances/64-bit totals and strict malformed/legacy cases passed`);
