// Synthetic PC -> actual USB parser -> device rest and hit die rules.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {initialState} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {recoverRest,hitDiceBudget,spendHitDie} from '../src/recovery.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
const states=Array.from({length:80},(_,n)=>{
 const s=initialState();s.extended=emptyExtended();s.level=1+n%20;s.hp=n%5===0?0:n%7===0?s.maxHp:1+n%31;s.extended.tempHp=n%10;s.abilities.COS=1+n%30;
 s.extended.hitDice=Array.from({length:1+n%20},(_,i)=>({name:`Classe sintetica ${i}`,die:[6,8,10,12][i%4],max:1+i%20,used:(i+n)%(2+i%20)}));
 s.extended.combat.rageBonus=2;s.extended.combat.raging=s.hp>0&&n%3===0;s.extended.combat.greatWeaponMasterAvailable=true;s.extended.combat.greatWeaponMaster=n%2===0;
 s.slots=s.slots.map((_,i)=>({max:9,current:(n+i)%10}));s.extended.pactSlots=s.extended.pactSlots.map((_,i)=>({max:3,current:(n+i)%4}));
 s.powers=['manual','short','long'].flatMap((recovery,i)=>[
  {id:`feature-${i}`,type:'feature',name:'Risorsa sintetica',description:'',maxUses:5,remaining:n%6,recovery},
  {id:`innate-${i}`,type:'spell',name:'Innato sintetico',description:'',level:1,concentration:true,usesSpellSlot:false,limited:{maxUses:3,remaining:n%4,recovery}},
 ]);s.powers.push({id:'passive',type:'feature',name:'Informativa',description:'',passive:true,maxUses:0,remaining:0,recovery:'manual'});
 if(s.hp&&!s.extended.combat.raging&&n%2===0)s.concentration='innate-0';
 return s;
});
const input='firmware/tests/out/rest-input.jsonl';writeFileSync(input,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/rest-probe',[input],{encoding:'utf8',maxBuffer:8*1024*1024,timeout:30000});assert.equal(run.status,0,run.stderr);
const rows=run.stdout.trim().split('\n');assert.equal(rows.length,states.length);let checked=0;
rows.forEach((row,n)=>{
 const s=states[n],actual=JSON.parse(row);assert.deepEqual(actual.budget,['2014','2024'].map(p=>hitDiceBudget(s,p)));checked+=2;
 actual.rests.forEach((r,option)=>{
  const policy=['manual','manual','2014','2024'][option],regain=Array(s.extended.hitDice.length).fill(0);let budget=hitDiceBudget(s,policy);
  if(policy==='2014')for(let i=0;i<regain.length;i++){const at=(i+n)%regain.length;regain[at]=Math.min(budget,s.extended.hitDice[at].used);budget-=regain[at]}
  const next=recoverRest(s,option?'long':'short',policy,regain),x=next.extended;
  assert.deepEqual(r,[next.hp,x.tempHp,next.concentration?next.powers.findIndex(p=>p.id===next.concentration)+1:0,Number(x.combat.raging),Number(x.combat.raging),Number(x.combat.greatWeaponMaster),next.slots.map(v=>v.current),x.pactSlots.map(v=>v.current),next.powers.map(p=>p.type==='feature'?p.remaining:p.limited?.remaining??0),x.hitDice.map(h=>h.used)],`${n}:${option}`);checked++;
 });
 let index=0;
 s.extended.hitDice.forEach((h,i)=>{for(let face=1;face<=h.die;face++){
  const actualRoll=actual.spends[index++];let next;
  try{next=spendHitDie(s,i,face)}catch{assert.equal(actualRoll,null,`${n}:${i}:${face}`);checked++;continue}
  assert.deepEqual(actualRoll,[1,next.hp,s.extended.tempHp,h.used+1,next.hp-s.hp,face,Math.max(0,face+Math.floor((s.abilities.COS-10)/2))]);checked++;
 }});assert.equal(actual.spends.length,index);
});
console.log(`PC -> USB -> C rests/hit dice: ${checked} comparisons, 80 synthetic states, 20 class pools, all die faces, CON 1..30, explicit policies and zero/full/exhaustion guards passed.`);
