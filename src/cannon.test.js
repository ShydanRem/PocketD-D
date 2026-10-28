import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondCannonFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {usePower,availableSlots,rest,validatePower} from './powers.js';
import {serialize,parseBackup} from './model.js';
import {projectDevice} from './device-export.js';
import {createLibrary,addCharacter,parseLibrary,serializeLibrary} from './library.js';
const state=remaining=>convertBeyond(beyondCannonFixture(remaining),'123456').draft;
const cannon=s=>s.powers.find(p=>p.effect==='eldritch-cannon');
test('Cannon: only recognized class/subclass/source/action identities and explicit ordinary counter enable the slot fallback',()=>{
 const p=beyondCannonFixture(),before=structuredClone(p),r=convertBeyond(p,'123456');assert.ok(cannon(r.draft));assert.deepEqual(p,before);
 for(const mutate of [c=>c.classes[0].definition.isHomebrew=true,c=>c.classes[0].definition.id++,c=>c.classes[0].subclassDefinition.id++,c=>c.classes[0].subclassDefinition.isHomebrew=true,c=>c.classes[0].subclassDefinition.sources=[],c=>c.classes[0].classFeatures.at(-1).definition.sources=[],c=>c.classes[0].classFeatures.at(-1).definition.requiredLevel=1,c=>c.actions.class.at(-1).id='other',c=>c.actions.class.at(-1).componentTypeId++,c=>c.actions.class.at(-1).activation.activationType=3,c=>c.actions.class.at(-1).limitedUse.maxUses=2,c=>c.actions.class.at(-1).limitedUse.resetDice='1d4']){const q=beyondCannonFixture();mutate(q.data);const v=convertBeyond(q,'123456');assert.ok(v.canImport);assert.equal(cannon(v.draft),undefined);}
});
test('Cannon: free use first, then exactly one chosen ordinary/pact slot at every level; no other counters, HP, concentration or effects change',()=>{
 let s=state(1),p=cannon(s);assert.deepEqual(availableSlots(s,p),[]);assert.throws(()=>usePower(s,p.id,1));
 const after=usePower(s,p.id);assert.equal(cannon(after).remaining,0);assert.deepEqual(after.slots,s.slots);assert.deepEqual(after.extended,s.extended);
 for(let level=1;level<=9;level++)for(const pact of [false,true])for(const count of [1,9]){
  s=state(0);s.slots.forEach(v=>{v.max=v.current=9;});s.extended.pactSlots.forEach(v=>{v.max=v.current=9;});const pool=pact?s.extended.pactSlots:s.slots;pool[level-1].current=count;
  const before=structuredClone(s),next=usePower(s,cannon(s).id,pact?`pact:${level}`:level),expected=structuredClone(before);(pact?expected.extended.pactSlots:expected.slots)[level-1].current--;
  assert.deepEqual(next,expected);assert.deepEqual(s,before);assert.equal(cannon(next).remaining,0);
 }
});
test('Cannon: missing/malformed/exhausted choices, zero HP and invalid profiles never charge a resource; rest restores only the free use',()=>{
 const s=state(0),p=cannon(s),before=structuredClone(s);for(const choice of [0,-1,10,1.5,true,'01','pact:0','pact:10','1d6'])assert.throws(()=>usePower(s,p.id,choice));assert.deepEqual(s,before);
 const empty=state(0);empty.slots.forEach(v=>v.current=0);empty.extended.pactSlots.forEach(v=>v.current=0);assert.deepEqual(availableSlots(empty,cannon(empty)),[]);assert.throws(()=>usePower(empty,cannon(empty).id,1));
 for(const remaining of [0,1]){const dead=state(remaining);dead.hp=0;assert.throws(()=>usePower(dead,cannon(dead).id,remaining?0:1));}
 assert.equal(cannon(rest(s,'short')).remaining,0);assert.equal(cannon(rest(s,'long')).remaining,1);assert.deepEqual(rest(s,'long').extended.manualResources,s.extended.manualResources);
 for(const change of [{maxUses:2},{recovery:'short'},{passive:true},{type:'spell'}])assert.throws(()=>validatePower({...p,...change},true));
});
test('Cannon: backup v19 and USB16 retain feature type, exact costs and all unrelated data; old/future forms reject new effect',()=>{
 const s=state(0),b=JSON.parse(serialize(s));assert.equal(b.version,19);assert.deepEqual(parseBackup(serialize(s)),s);
 for(const version of [11,17,18,20])assert.throws(()=>parseBackup(JSON.stringify({...b,version})));
 const p=projectDevice(s).packet;assert.equal(p.version,16);assert.ok(p.training.skillAbilities);assert.deepEqual(p.defences,{configured:0,resistance:0,vulnerability:0,immunity:0});const c=p.magic.powers.find(p=>p.effect===3);assert.equal(c.kind,1);assert.equal(c.level,0);assert.equal(c.maximum,1);assert.equal(c.recovery,2);assert.equal(c.flags&13,0);
 const l=createLibrary(),next=addCharacter(l,s,'cannon');assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.deepEqual(next.characters[0],l.characters[0]);
});
