// PC -> real USB parser -> native firmware rules. Synthetic only.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {initialState,abilityKeys} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {usePower,spellStats} from '../src/powers.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
const states=[];
for(let n=0;n<50;n++){
 const s=initialState();s.extended=emptyExtended();s.level=1+n%20;s.hp=n%7===0?0:20;
 abilityKeys.forEach((key,i)=>s.abilities[key]=1+(n*3+i*7)%30);
 s.spellcastingAbility=abilityKeys[n%6];s.slots=s.slots.map((_,i)=>({max:3,current:(i+n)%4}));s.extended.pactSlots=s.extended.pactSlots.map((_,i)=>({max:2,current:(i+n)%3}));
 s.extended.combat.raging=n%11===0;s.extended.combat.rageBonus=2;
 s.powers=Array.from({length:5},(_,i)=>({id:`spell-${i}`,type:'spell',name:`Magia sintetica ${i}`,description:'Descrizione sintetica. '.repeat(n%30),level:i===0?0:1+i%3,concentration:i%2===1,castingAbility:abilityKeys[(n+i)%6],attackModifier:(n+i)%46-15,saveModifier:30-(n+i)%46,prepared:(n+i)%5!==0,usesSpellSlot:i!==4,...(i>=3?{limited:{maxUses:3,remaining:n%4,recovery:'long'}}:{})}));
 s.powers.push({id:'rage',name:'Ira sintetica',description:'',type:'feature',effect:'rage',maxUses:3,remaining:n%4,recovery:'long'},
 {id:'passive',name:'Nota sintetica',description:'',type:'feature',passive:true,maxUses:0,remaining:0,recovery:'manual'},
 {id:'endurance',name:'Resistenza sintetica',description:'',type:'feature',effect:'relentless-endurance',maxUses:1,remaining:n%2,recovery:'long'});
 if(s.hp&&!s.extended.combat.raging&&n%3===0)s.concentration='spell-1';
 states.push(s);
}
const full=structuredClone(states[1]);while(full.powers.length<120)full.powers.push({id:`max-${full.powers.length}`,type:'spell',name:'W'.repeat(40),description:'"\\'.repeat(3000),level:0,concentration:false});states.push(full);
const input='firmware/tests/out/magic-input.jsonl';writeFileSync(input,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/magic-probe',[input],{encoding:'utf8',maxBuffer:8*1024*1024,timeout:30000});assert.equal(run.status,0,run.stderr);
const rows=run.stdout.trim().split('\n');assert.equal(rows.length,states.length);let checked=0;
rows.forEach((row,n)=>{
 const s=states[n],powers=JSON.parse(row);assert.equal(powers.length,s.powers.length);
 powers.forEach((actual,i)=>{
  const p=s.powers[i],stats=p.type==='spell'?spellStats(s,p):null;
  assert.deepEqual(actual.stats,stats?[1,stats.attack,stats.dc]:[0,0,0]);checked++;
  actual.uses.forEach((result,option)=>{
   const slot=option>9?option-9:option,pact=option>9;let next;
   try{next=usePower(s,p.id,pact?`pact:${slot}`:slot)}catch{assert.equal(result,null,`${n}:${i}:${option}`);checked++;return}
   const concentration=next.concentration?next.powers.findIndex(p=>p.id===next.concentration)+1:0;
   const power=next.powers[i];
   assert.deepEqual(result,[next.hp,concentration,Number(next.extended.combat.raging),Number(next.extended.combat.raging),slot?next.slots[slot-1].current:0,slot?next.extended.pactSlots[slot-1].current:0,p.type==='feature'?power.remaining:power.limited?.remaining??0],`${n}:${i}:${option}`);checked++;
  });
 });
});
console.log(`PC -> USB -> C magic: ${checked} comparisons, 51 synthetic states, all 120 powers, spell stats/costs/preparation/concentration/rage/endurance passed.`);
