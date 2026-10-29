import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyondFull} from './beyond-conversion.js';
import {projectDevice} from './device-export.js';
const full=p=>convertBeyondFull(p,'123456');
const source=()=>{const p=beyondRichFixture();p.data.inventory[0].id=100;return p;};
const field=(typeId,value)=>({typeId,valueId:'100',valueTypeId:'1439493548',value});

test('Weapon ability replacement does not change a score or require an ASI origin; weapon stays manual',()=>{
 const p=source(),ordinary=full(p);p.data.modifiers.class.push({id:'synthetic-pact',componentId:999999,type:'replace-weapon-ability',subType:'charisma-score',statId:6,value:null,isGranted:true});
 const before=structuredClone(p),r=full(p);assert.ok(r.canImport);assert.deepEqual(r.draft.abilities,ordinary.draft.abilities);assert.equal(r.draft.maxHp,ordinary.draft.maxHp);assert.equal(r.draft.armorClass,ordinary.draft.armorClass);
 assert.equal(r.draft.inventory[0].type,'gear');assert.equal(r.draft.equippedWeaponId,null);assert.ok(r.conversion.omissions.some(s=>/caratteristica sostituita/.test(s)));
 const packet=projectDevice(r.draft).packet;assert.equal(packet.inventory.items[0].kind,0);assert.equal(packet.inventory.equipped,0);assert.deepEqual(p,before);
});
test('Item-scoped weapon customizations import the sheet without inventing attack, damage or property semantics',()=>{
 for(const [type,value] of [[10,-3],[10,1],[12,1],[12,10],[20,false],[20,true]]){
  const p=source(),ordinary=full(p);p.data.characterValues=[field(type,value)];const before=structuredClone(p),r=full(p);
  assert.ok(r.canImport);assert.deepEqual(r.draft.abilities,ordinary.draft.abilities);assert.equal(r.draft.hp,ordinary.draft.hp);assert.equal(r.draft.armorClass,ordinary.draft.armorClass);assert.equal(r.draft.inventory.length,ordinary.draft.inventory.length);assert.equal(r.draft.inventory[0].quantity,2);assert.equal(r.draft.inventory[0].type,'gear');assert.equal(r.draft.equippedWeaponId,r.draft.inventory[1].id);assert.equal(r.draft.inventory[1].type,'weapon');assert.deepEqual(p,before);
 }
});
test('Malformed weapon customizations, ambiguous targets and unknown overrides remain blockers',()=>{
 for(const change of [v=>v.valueId='missing',v=>v.valueTypeId=0,v=>v.value='1',v=>v.value=Infinity,v=>v.value=1001,v=>v.typeId=12345]){
  const p=source(),v=field(10,1);change(v);p.data.characterValues=[v];assert.equal(full(p).canImport,false);
 }
 const duplicate=source();duplicate.data.inventory.push(structuredClone(duplicate.data.inventory[0]));duplicate.data.characterValues=[field(10,1)];assert.equal(full(duplicate).canImport,false);
 const wrong=source();wrong.data.inventory[0].definition.filterType='Other Gear';wrong.data.characterValues=[field(12,1)];assert.equal(full(wrong).canImport,false);
 const scalar=source();scalar.data.modifiers.class.push({id:'unresolved-score',componentId:999999,type:'bonus',subType:'charisma-score',value:2,isGranted:true});assert.equal(full(scalar).canImport,false);
});
test('Unset weapon fields preserve ordinary conversion; source cannot inject the internal manual flag',()=>{
 const p=source();p.data.characterValues=[field(10,null),field(12,null),field(20,null)];p.data.inventory[0].pocketddWeaponManual=true;
 const r=full(p);assert.ok(r.canImport);assert.equal(r.draft.inventory[0].type,'weapon');assert.equal(r.draft.equippedWeaponId,null);
});
