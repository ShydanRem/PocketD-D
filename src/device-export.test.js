import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,skills,abilityKeys} from './model.js';
import {emptyExtended} from './extended.js';
import {projectDevice,serializeDevice} from './device-export.js';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';

test('device export is a bounded projection and never mutates the source',()=>{
  const s=initialState(),before=structuredClone(s);
  s.name='Élian 🧙';before.name=s.name;
  const {packet,warnings}=projectDevice(s);
  assert.equal(packet.name,'Elian ??');assert.equal(packet.hp,24);assert.equal(packet.armorClass,16);
  assert.deepEqual(packet.abilities,s.abilities);assert.deepEqual(s,before);
  assert.equal(packet.format,'pocketdd-device');assert.equal(packet.version,3);
  assert.equal(packet.resources[0].current,1);assert(warnings.some(w=>w.includes('inventario')));
  assert(!serializeDevice(packet).includes('inventory'));
});
test('training projection preserves ranks, half proficiency, extras, blocked checks and effect snapshots',()=>{
  const s=initialState();s.extended=emptyExtended();s.condition=true;s.level=20;
  s.extended.combat.raging=true;s.extended.combat.rageBonus=2;s.extended.halfAbilityChecks=true;
  s.extended.halfSkills=['acrobatics','perception'];s.extended.blockedChecks=['skill:arcana','save:DES'];
  skills.forEach(({id},i)=>{s.skillRanks[id]=i%3;s.extended.skillBonuses[id]=i%2?-30:30});
  abilityKeys.forEach((key,i)=>{s.saveProficiencies[key]=i%2===0;s.extended.saveBonuses[key]=-i;s.extended.checkBonuses[key]=i});
  const before=structuredClone(s),{packet}=projectDevice(s),t=packet.training;
  assert.deepEqual(t.skillRanks,s.skillRanks);assert.deepEqual(t.saveProficiencies,s.saveProficiencies);
  assert.deepEqual(t.skillBonuses,s.extended.skillBonuses);assert.deepEqual(t.halfSkills,s.extended.halfSkills);
  assert.deepEqual(t.blockedChecks,s.extended.blockedChecks);assert(t.poisoned&&t.strengthAdvantage&&t.halfAbilityChecks);
  assert.deepEqual(s,before);t.skillRanks.arcana=2;t.halfSkills.push('history');assert.deepEqual(s,before);
  assert(serializeDevice(packet).length<4096);
});
test('all avatar choices transfer as independent indices without changing any character data',()=>{
  for(let skin=0;skin<3;skin++)for(let hair=0;hair<3;hair++)for(let cloak=0;cloak<3;cloak++)for(let style=0;style<3;style++){
    const s=initialState();s.avatar={skin,hair,cloak,style};const before=structuredClone(s);
    const {packet}=projectDevice(s);assert.deepEqual(packet.avatar,s.avatar);assert.deepEqual(s,before);
    packet.avatar.skin=(skin+1)%3;assert.deepEqual(s,before);
  }
  const s=initialState();s.avatar.style=3;assert.throws(()=>projectDevice(s),/Avatar non valido/);
});
test('slot counters retain current values and excluded counters are named',()=>{
  const s=initialState();s.slots=s.slots.map((_,i)=>({max:3,current:i%3}));
  const {packet,warnings}=projectDevice(s);
  assert.equal(packet.resources.length,8);assert.equal(packet.resources[1].current,0);
  assert.equal(packet.resources[2].current,1);
  assert(warnings.some(w=>w.includes('Slot livello 8')&&w.includes('Slot livello 9')));
});
test('firmware limits reject values without silently clamping',()=>{
  const s=initialState();s.maxHp=1000;
  assert.throws(()=>projectDevice(s),/Profilo non valido/);
});

test('extended imported projection preserves HP, temp HP and resources without demo counters',()=>{
  const s=convertBeyond(beyondRichFixture(),'123456').draft,before=structuredClone(s);
  const {packet,warnings}=projectDevice(s);
  assert.equal(packet.hp,s.hp);assert.equal(packet.maxHp,s.maxHp);assert.equal(packet.tempHp,4);
  assert.equal(packet.armorClass,s.armorClass);assert.deepEqual(packet.abilities,s.abilities);
  assert(!packet.resources.some(r=>r.name==='Slancio'));
  assert.deepEqual(packet.training.skillRanks,s.skillRanks);assert.deepEqual(packet.training.saveProficiencies,s.saveProficiencies);
  assert.deepEqual(packet.training.blockedChecks,s.extended.blockedChecks);
  for(const p of s.powers.filter(p=>p.type==='feature'&&!p.passive&&p.maxUses>0)){
    const resource=packet.resources.find(r=>r.name===p.name);assert(resource);
    assert.equal(resource.current,p.remaining);assert.equal(resource.maximum,p.maxUses);
  }
  assert.deepEqual(s,before);assert(warnings.some(w=>w.includes('condizioni')));
  assert(!serializeDevice(packet).includes('importInfo'));
});
