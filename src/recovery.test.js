import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState} from './model.js';
import {emptyExtended} from './extended.js';
import {hitDiceBudget,recoverRest,spendHitDie} from './recovery.js';
import {projectDevice} from './device-export.js';
const hero=()=>({...initialState(),extended:{...emptyExtended(),hitDice:[{name:'Classe A',die:8,max:3,used:3},{name:'Classe B',die:12,max:2,used:1}]}});
test('rest policy is explicit; multiclass 2014 budget rounds down and never picks a pool silently',()=>{
  const s=hero(),before=structuredClone(s);assert.equal(hitDiceBudget(s,'2014'),2);assert.equal(hitDiceBudget(s,'2024'),4);
  assert.deepEqual(recoverRest(s,'long').extended.hitDice,s.extended.hitDice);
  assert.deepEqual(recoverRest(s,'long','2014',[1,1]).extended.hitDice.map(h=>h.used),[2,0]);
  assert.deepEqual(recoverRest(s,'long','2024').extended.hitDice.map(h=>h.used),[0,0]);
  for(const args of [['short','2024'],['long','2014',[3,0]],['long','2014',[0,2]],['long','manual',[1]],['long','2024',[1]],['long','2014',[-1]],['long','2014',[.5]]])assert.throws(()=>recoverRest(s,...args));
  assert.deepEqual(s,before);
  const one=hero();one.extended.hitDice=[{name:'Uno',die:6,max:1,used:1}];assert.equal(hitDiceBudget(one,'2014'),1);
});
test('one hit die heals with CON, clamps to zero/max, consumes even a zero cure and preserves temporary HP',()=>{
  const s=hero();s.extended.hitDice[0].used=0;s.extended.tempHp=7;s.abilities.COS=1;
  const next=spendHitDie(s,0,1);assert.equal(next.hp,s.hp);assert.equal(next.extended.hitDice[0].used,1);assert.equal(next.extended.tempHp,7);
  s.abilities.COS=30;assert.equal(spendHitDie(s,0,8).hp,s.maxHp);assert.equal(s.extended.hitDice[0].used,0);
  for(const [state,index,face] of [[{...s,hp:0},0,1],[{...s,hp:s.maxHp},0,1],[s,2,1],[s,0,9],[s,0,.5]])assert.throws(()=>spendHitDie(state,index,face));
});
test('all twenty hit die pools project without modifying source; unavailable data stays unavailable',()=>{
  const s=hero();s.extended.hitDice=Array.from({length:20},(_,i)=>({name:'É'+String(i)+'W'.repeat(70),die:[6,8,10,12][i%4],max:20,used:i}));
  const before=structuredClone(s),{packet}=projectDevice(s);assert.equal(packet.recovery.pools.length,20);assert(packet.recovery.available);
  packet.recovery.pools.forEach((h,i)=>{assert(h.name.length<=40);assert.equal(h.used,i);assert.equal(h.die,s.extended.hitDice[i].die)});assert.deepEqual(s,before);
  assert.deepEqual(projectDevice(initialState()).packet.recovery,{available:false,pools:[]});
});
