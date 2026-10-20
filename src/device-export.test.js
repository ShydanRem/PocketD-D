import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState} from './model.js';
import {projectDevice,serializeDevice} from './device-export.js';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';

test('device export is a bounded projection and never mutates the source',()=>{
  const s=initialState(),before=structuredClone(s);
  s.name='Élian 🧙';before.name=s.name;
  const {packet,warnings}=projectDevice(s);
  assert.equal(packet.name,'Elian ??');assert.equal(packet.hp,24);assert.equal(packet.armorClass,16);
  assert.deepEqual(packet.abilities,s.abilities);assert.deepEqual(s,before);
  assert.equal(packet.format,'pocketdd-device');assert.equal(packet.version,2);
  assert.equal(packet.resources[0].current,1);assert(warnings.some(w=>w.includes('inventario')));
  assert(!serializeDevice(packet).includes('inventory'));
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
  for(const p of s.powers.filter(p=>p.type==='feature'&&!p.passive&&p.maxUses>0)){
    const resource=packet.resources.find(r=>r.name===p.name);assert(resource);
    assert.equal(resource.current,p.remaining);assert.equal(resource.maximum,p.maxUses);
  }
  assert.deepEqual(s,before);assert(warnings.some(w=>w.includes('condizioni')));
  assert(!serializeDevice(packet).includes('importInfo'));
});
