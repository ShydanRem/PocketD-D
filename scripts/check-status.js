// Actual parser and C state transitions against the PC model; synthetic data only.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {initialState,changeHp,useItem} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {deathChange,deathFace,conditionChange} from '../src/status.js';
import {rest} from '../src/powers.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
const states=[];
for(let hp=0;hp<2;hp++)for(let stabilized=0;stabilized<2;stabilized++)for(let successes=0;successes<4;successes++)for(let failures=0;failures<4;failures++){
 const n=states.length,s={...initialState(),extended:emptyExtended(),hp,level:1+n%20,condition:n%2===0};
 s.abilities.COS=1+n%30;s.extended.deathSaves={successes,failures,stabilized:Boolean(stabilized)};s.extended.tempHp=n%4;s.mode=['normal','advantage','disadvantage'][n%3];
 s.extended.conditions=Array.from({length:n%31},(_,i)=>({name:`Nota sintetica ${i}`,level:i%7}));states.push(s);
}
const input='firmware/tests/out/status-input.jsonl';writeFileSync(input,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/status-probe',[input],{encoding:'utf8',maxBuffer:8*1024*1024,timeout:30000});assert.equal(run.status,0,run.stderr);
const rows=run.stdout.trim().split('\n');assert.equal(rows.length,states.length);let checked=0;
const expected=s=>[s.hp,s.extended.tempHp,s.extended.deathSaves.successes,s.extended.deathSaves.failures,Number(s.extended.deathSaves.stabilized),1];
const compare=(actual,operation,label)=>{let next;try{next=operation()}catch{assert.equal(actual,null,label);checked++;return}assert.deepEqual(actual,expected(next),label);checked++};
rows.forEach((row,n)=>{
 const actual=JSON.parse(row),s=states[n];actual.rolls.forEach((r,i)=>compare(r,()=>deathFace(s,i+1),`${n}:face${i+1}`));actual.changes.forEach((r,i)=>compare(r,()=>deathChange(s,i),`${n}:change${i}`));
 actual.hp.forEach((r,i)=>compare(r,()=>changeHp(s,i-1),`${n}:hp${i-1}`));
 actual.notes.forEach((r,i)=>r.forEach((level,j)=>{assert.equal(level,conditionChange(s,i,j-7).extended.conditions[i].level);checked++}));
 compare(actual.long,()=>rest(s,'long'),`${n}:long`);
 compare(actual.potion,()=>useItem(s,'starter-potion',()=>19).state,`${n}:potion`); // Probe reuses face=20 entropy after its death-roll loop.
});
console.log(`PC -> USB -> C status: ${checked} comparisons, 64 counter/HP/stability combinations, all d20 faces, 30 condition notes, poison independence and healing/rest guards passed.`);
