// Synthetic PC -> USB8 -> actual C parser/rest, no external or personal data.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {initialState} from '../src/model.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
import {rest} from '../src/powers.js';
const states=Array.from({length:30},(_,n)=>({...initialState(),equippedWeaponId:null,inventory:Array.from({length:60},(_,i)=>({id:`item-${i}`,type:'gear',name:`Oggetto sintetico ${i}`,notes:'',quantity:(n+i)%4===0?0:1+(n+i)%999,...((n+i)%5===0?{}:{charges:{current:(n+i)%10,max:999,recovery:['manual','short','long'][(n+i)%3]}})}))}));
const path='firmware/tests/out/charges.jsonl';writeFileSync(path,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const process=spawnSync('firmware/tests/out/charges-probe',[path],{encoding:'utf8'});assert.equal(process.status,0,process.stderr);
let comparisons=0;
process.stdout.trim().split('\n').forEach((line,n)=>JSON.parse(line).forEach(([recovered,items],k)=>{
 const old=states[n],next=rest(old,k?'long':'short');let delta=0;
 next.inventory.forEach((v,i)=>{const c=v.charges;assert.deepEqual(items[i],[v.quantity,c?.current??0,c?.max??0,['manual','short','long'].indexOf(c?.recovery??'manual')]);delta+=(c?.current??0)-(old.inventory[i].charges?.current??0);comparisons++;});assert.equal(recovered,delta);comparisons++;
}));
console.log(`PC -> USB8 -> C charges: ${comparisons} comparisons, 30 synthetic states x 60 rows, absent pools, zero quantities and separate rest policies passed.`);
