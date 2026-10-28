import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondSkillIds,parseBeyondTraining} from './beyond-training.js';
import {beyondCoreFixture,beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond,convertBeyondCore} from './beyond-conversion.js';
import {describeRoll,serialize,parseBackup,proficiency} from './model.js';
import {projectDevice} from './device-export.js';
import {createLibrary,addCharacter,parseLibrary,serializeLibrary} from './library.js';
const convert=p=>convertBeyond(p,'123456'),custom=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
test('Beyond custom training: all 18 stable skill IDs, levels 1..20 and four explicit grades override source ranks and half proficiency',()=>{
 for(const [id,key] of Object.entries(beyondSkillIds))for(const level of [1,5,9,13,17,20])for(const grade of [1,2,3,4]){
  const p=beyondCoreFixture(),c=p.data;c.classes[0].level=level;c.modifiers.class=c.modifiers.class.filter(m=>m.id!=='future');c.modifiers.feat.push({type:'expertise',subType:key.replace(/[A-Z]/g,m=>'-'+m.toLowerCase())},{type:'half-proficiency',subType:'ability-checks'});c.characterValues=[custom(26,id,grade)];const before=structuredClone(p),r=convert(p);assert.ok(r.canImport,r.conversion.blockers.join(';'));const s=r.draft;assert.equal(s.skillRanks[key],grade===4?2:grade===3?1:0);assert.equal(s.extended.halfSkills.includes(key),grade===2);assert.equal(s.extended.halfAbilityChecks,true);assert.deepEqual(p,before);const rank=grade===4?2:grade===3?1:0;const n=describeRoll(s,{kind:'skill',key}).bonus;const base={athletics:2,acrobatics:1,sleightOfHand:1,stealth:1,arcana:0,history:0,investigation:0,nature:0,religion:0,animalHandling:0,insight:0,medicine:0,perception:0,survival:0,deception:-1,intimidation:-1,performance:-1,persuasion:-1};assert.equal(n,base[key]+rank*proficiency(level)+(grade===2?Math.floor(proficiency(level)/2):0));assert.deepEqual(parseBackup(serialize(s)),s);assert.equal(projectDevice(s).packet.training.skillRanks[key],rank);
 }
});
test('Beyond custom bonuses: two distinct fields add once including negatives/zero and exact numeric strings; save overrides preserve false',()=>{
 const p=beyondRichFixture();p.data.modifiers.feat.push({type:'bonus',subType:'perception',value:2},{type:'bonus',subType:'saving-throws',value:1});p.data.characterValues=[custom(24,14,' +3 '),custom(25,'14','-2'),custom(39,3,-2),custom(40,3,3),custom(41,3,1),custom(41,2,3),custom(24,6,0)];const r=convert(p);assert.ok(r.canImport);assert.equal(r.draft.extended.skillBonuses.perception,3);assert.equal(r.draft.extended.saveBonuses.COS,2);assert.equal(r.draft.saveProficiencies.COS,false);assert.equal(r.draft.saveProficiencies.DES,true);assert.equal(describeRoll(r.draft,{kind:'skill',key:'perception'}).bonus,5);assert.equal(describeRoll(r.draft,{kind:'save',key:'COS'}).bonus,5);
});
test('Beyond custom training: unset is different from explicit no proficiency; identical fields deduplicate once and conflicts block',()=>{
 for(const value of [null,0,'0']){const p=beyondRichFixture();p.data.characterValues=[custom(41,3,value)];assert.equal(convert(p).draft.saveProficiencies.COS,true);}
 const p=beyondRichFixture();p.data.characterValues=[custom(24,14,2),custom(24,'14','2')];assert.equal(convert(p).draft.extended.skillBonuses.perception,2);p.data.characterValues[1].value=3;assert.equal(convert(p).canImport,false);p.data.characterValues[1].value=null;assert.equal(convert(p).canImport,false);
});
test('Beyond custom training: malformed values/targets/namespaces/formulas and save expertise/half stay explicit blockers',()=>{
 for(const entry of [custom(24,1,2),custom(39,7,2),custom(26,14,5),custom(41,3,2),custom(41,3,4),custom(24,14,'2+1'),custom(24,14,'2d4'),custom(24,14,''),custom(24,14,true),custom(24,14,1.5),custom(24,14,31),custom(24,14,undefined),{...custom(24,14,2),valueTypeId:1439493548},{...custom(24,14,2),restriction:'while hidden'},{...custom(24,14,2),duration:1},{...custom(24,14,2),tagConstraints:[{}]}]){const p=beyondCoreFixture();p.data.characterValues=[entry];const r=convert(p);assert.equal(r.canImport,false,JSON.stringify(entry));assert.equal(r.draft,null);assert.ok(r.conversion.blockers.length);}
});
test('Beyond custom training: total bonus bounds are checked after source modifiers, never clamped or hidden as an HTTP-only success',()=>{
 const p=beyondCoreFixture();p.data.modifiers.feat.push({type:'bonus',subType:'perception',value:1});p.data.characterValues=[custom(24,14,30)];let r=convert(p);assert.equal(r.canImport,false);assert.equal(r.draft,null);assert.match(r.conversion.blockers.join(';'),/Bonus totale/);p.data.characterValues.push(custom(25,14,-1));r=convert(p);assert.ok(r.canImport);assert.equal(r.draft.extended.skillBonuses.perception,30);
});
test('Beyond custom training: customization never removes a block caused by an unresolved source roll modifier',()=>{
 const p=beyondCoreFixture();p.data.modifiers.feat.push({type:'bonus',subType:'perception',statId:4,value:null});p.data.characterValues=[custom(26,14,4),custom(24,14,2)];const r=convert(p);assert.ok(r.canImport);assert.equal(describeRoll(r.draft,{kind:'skill',key:'perception'}).blocked,true);assert.ok(projectDevice(r.draft).packet.training.blockedChecks.includes('skill:perception'));
});
test('Beyond custom training: unsupported fields stay blocked, cosmetic edits coexist and old local formats/collection stay unchanged',()=>{
 const p=beyondRichFixture();p.data.inventory[0].id=123;p.data.characterValues=[custom(24,14,2),{typeId:8,valueId:123,valueTypeId:1439493548,value:'Lancia locale'}];let r=convert(p);assert.ok(r.canImport);assert.equal(r.draft.inventory[0].name,'Lancia locale');assert.equal(JSON.parse(serialize(r.draft)).version,11);assert.equal(projectDevice(r.draft).packet.version,9);const library=createLibrary(),next=addCharacter(library,r.draft,'import');assert.deepEqual(next.characters[0],library.characters[0]);assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.equal(convertBeyondCore(p,'123456').canImport,false);
 for(const typeId of [1,23,27,42]){p.data.characterValues=[custom(typeId,14,2)];assert.equal(convert(p).canImport,false);}
});
test('Beyond custom training parser accepts only the documented numeric namespace, preserving unknown entries for core validation',()=>{
 const unknown={typeId:27,valueId:14,value:4},r=parseBeyondTraining([unknown,custom(24,14,null),custom(24,6,0)]);assert.deepEqual(r.remaining,[unknown]);assert.equal(r.training.length,1);assert.deepEqual(r.blockers,[]);
});
test('Beyond custom training: zero-quantity and inactive items cannot leak skill bonuses, and source origin ambiguity keeps rolls blocked',()=>{
 for(const patch of [{quantity:0},{equipped:false},{isAttuned:false}]){const p=beyondCoreFixture(),m={id:'item-training',type:'bonus',subType:'perception',value:5};p.data.modifiers.item.push(m);p.data.inventory.push({quantity:1,equipped:true,isAttuned:true,definition:{name:'Sonda tiri',filterType:'Other Gear',canAttune:true,grantedModifiers:[m]},...patch});p.data.characterValues=[custom(24,14,2)];assert.equal(convert(p).draft.extended.skillBonuses.perception,2);}
 for(const patch of [{duration:1},{dice:{diceString:'1d4'}},{tagConstraints:[{}]},{isGranted:'yes'},{requiresAttunement:'yes'}]){const p=beyondCoreFixture(),m={id:'hidden',type:'bonus',subType:'perception',value:3,isGranted:true};p.data.modifiers.item.push(m);p.data.inventory.push({quantity:1,equipped:true,isAttuned:true,definition:{name:'Sonda requisiti',filterType:'Other Gear',canAttune:true,grantedModifiers:[{...m,...patch}]}});p.data.characterValues=[custom(24,14,2)];assert.equal(describeRoll(convert(p).draft,{kind:'skill',key:'perception'}).blocked,true);}
 const unselected=beyondCoreFixture();unselected.data.modifiers.class.push({componentId:999999,type:'proficiency',subType:'perception',isGranted:false});assert.equal(describeRoll(convert(unselected).draft,{kind:'skill',key:'perception'}).blocked,false);
 const unknown=beyondCoreFixture();unknown.data.modifiers.unknown=[{type:'bonus',subType:'perception',value:3}];unknown.data.characterValues=[custom(24,14,2)];assert.equal(describeRoll(convert(unknown).draft,{kind:'skill',key:'perception'}).blocked,true);
 const p=beyondCoreFixture(),m={id:'unknown-skill',componentId:999999,type:'proficiency',subType:'perception'};p.data.modifiers.class.push(m);p.data.characterValues=[custom(26,14,3)];const r=convert(p);assert.ok(r.canImport);assert.equal(describeRoll(r.draft,{kind:'skill',key:'perception'}).blocked,true);
});
test('Beyond custom training: preview retains explicit none/zero and half grades even without proficiency or a nonzero bonus',()=>{
 const p=beyondRichFixture();p.data.characterValues=[custom(26,14,1),custom(26,3,2),custom(41,3,1),custom(24,6,0)];const r=convert(p);assert.ok(r.canImport);assert.equal(r.details.skills.find(s=>s.id==='perception').customized,true);assert.equal(r.details.skills.find(s=>s.id==='perception').rank,0);assert.equal(r.details.skills.find(s=>s.id==='acrobatics').half,true);assert.equal(r.details.saves.find(s=>s.id==='COS').proficient,false);assert.equal(r.details.saves.find(s=>s.id==='COS').customized,true);
});
