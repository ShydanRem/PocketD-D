import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondCoreFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {beyondSkillIds} from './beyond-training.js';
import {initialState,skills,abilityKeys,describeRoll,parseBackup,serialize} from './model.js';
import {emptyExtended,validateSkillAbilities} from './extended.js';
import {projectDevice} from './device-export.js';
import {createLibrary,addCharacter,parseLibrary,serializeLibrary} from './library.js';
const cv=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
test('Beyond skill ability: all 18 targets x six abilities keep proficiency/extras distinct and dynamically follow the selected score',()=>{
 for(const [id,key] of Object.entries(beyondSkillIds))for(let a=0;a<6;a++){
  const p=beyondCoreFixture();p.data.stats.forEach((s,i)=>s.value=8+2*i);p.data.modifiers.race=[];p.data.characterValues=[cv(27,id,a+1),cv(26,id,4),cv(24,id,2)];const before=structuredClone(p),r=convertBeyond(p,'123456');assert.ok(r.canImport,r.conversion.blockers.join(';'));const s=r.draft;
  assert.equal(s.extended.skillAbilities[key],abilityKeys[a]);assert.equal(describeRoll(s,{kind:'skill',key}).bonus,a-1+4+2);const changed={...s,abilities:{...s.abilities,[abilityKeys[a]]:20}};assert.equal(describeRoll(changed,{kind:'skill',key}).bonus,5+4+2);assert.equal(describeRoll(changed,{kind:'save',key:abilityKeys[a]}).bonus,5);assert.deepEqual(p,before);
  const b=JSON.parse(serialize(s)),wire=projectDevice(s).packet;assert.equal(b.version,17);assert.equal(wire.version,15);assert.equal(wire.training.skillAbilities[key],a+1);assert.equal(wire.avatar.outfit,0);assert.deepEqual(parseBackup(JSON.stringify(b)),s);
 }
});
test('Skill ability: defaults, explicit default association, null/automatic and independent skills are preserved without new metadata on old states',()=>{
 const s=initialState();assert.equal(JSON.parse(serialize(s)).version,11);assert.equal(projectDevice(s).packet.version,9);
 for(const value of [null,0,'0']){const p=beyondCoreFixture();p.data.characterValues=[cv(27,14,value)];const r=convertBeyond(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.extended.skillAbilities,undefined);}
 const p=beyondCoreFixture();p.data.characterValues=[cv(27,14,'1'),cv(27,17,4)];const r=convertBeyond(p,'123456');assert.ok(r.canImport);assert.deepEqual(r.draft.extended.skillAbilities,{perception:'FOR',intimidation:'INT'});assert.equal(describeRoll(r.draft,{kind:'skill',key:'acrobatics'}).bonus,1);
});
test('Skill ability: invalid maps, unknown targets, old/future backups, formulas and conflicting choices never silently fall back',()=>{
 for(const map of [null,[],{unknown:'FOR'},{perception:'STR'},{perception:1},{perception:null}])assert.throws(()=>validateSkillAbilities(map));
 const s={...initialState(),extended:{...emptyExtended(),skillAbilities:{intimidation:'FOR'}}},b=JSON.parse(serialize(s));for(const version of [11,12,13,14,15,16,18])assert.throws(()=>parseBackup(JSON.stringify({...b,version})));const missing=structuredClone(b);delete missing.extended.skillAbilities;assert.throws(()=>parseBackup(JSON.stringify(missing)));
 for(const value of [-1,7,1.5,true,'FOR','1d6','']){const p=beyondCoreFixture();p.data.characterValues=[cv(27,14,value)];assert.equal(convertBeyond(p,'123456').canImport,false);}
 const p=beyondCoreFixture();p.data.characterValues=[cv(27,14,1),cv(27,14,2)];assert.equal(convertBeyond(p,'123456').canImport,false);p.data.characterValues[1].value=1;assert.ok(convertBeyond(p,'123456').canImport);
});
test('Skill ability: selected STR controls rage advantage, poison cancellation and original unresolved-roll blocks',()=>{
 const p=beyondCoreFixture();p.data.characterValues=[cv(27,17,1)];const r=convertBeyond(p,'123456'),s=r.draft;s.extended.combat={...s.extended.combat,rageBonus:2,raging:true};assert.equal(describeRoll(s,{kind:'skill',key:'intimidation'}).mode,'advantage');s.condition=true;assert.equal(describeRoll(s,{kind:'skill',key:'intimidation'}).mode,'normal');
 s.extended.skillAbilities.intimidation='INT';assert.equal(describeRoll(s,{kind:'skill',key:'intimidation'}).mode,'disadvantage');s.extended.blockedChecks=['skill:intimidation'];assert.equal(describeRoll(s,{kind:'skill',key:'intimidation'}).blocked,true);
});
test('Skill ability: defence profile, species/class, collection, preparation and quantities retain exact independent data',()=>{
 const s={...initialState(),avatar:{...initialState().avatar,lineage:20,body:1,outfit:12},extended:{...emptyExtended(),skillAbilities:{athletics:'INT'},defences:{configured:32,resistance:32,vulnerability:0,immunity:0}}};
 const p=projectDevice(s).packet;assert.equal(p.version,15);assert.deepEqual(p.defences,s.extended.defences);assert.equal(p.avatar.lineage,20);assert.equal(p.avatar.outfit,12);const l=createLibrary(),next=addCharacter(l,s,'mapped');assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.deepEqual(next.characters[0],l.characters[0]);assert.deepEqual(s.inventory,initialState().inventory);
});
