import test from 'node:test';import assert from 'node:assert/strict';
import {beyondRichFixture,beyondProfileFixture as profileFixture} from './beyond-fixtures.js';
import {convertBeyondFull} from './beyond-conversion.js';import {projectDevice} from './device-export.js';import {parseBackup,serialize} from './model.js';

const full=p=>convertBeyondFull(p,'123456'),group=(r,key)=>r.details.reference.groups.find(g=>g.key===key).rows.join('\n');
test('Full v1: reference data survives PC backup20/USB17, exact quantities and independent snapshots',()=>{
 const p=profileFixture(),before=structuredClone(p),r=full(p);assert.ok(r.canImport);assert.equal(r.details.reference.initiative.bonus,1);assert.match(group(r,'movement'),/Passo: 30 ft/);assert.match(group(r,'senses'),/Percezione passiva: 10/);assert.match(group(r,'identity'),/Classe sonda 3/);
 const s=parseBackup(serialize(r.draft)),wire=projectDevice(s).packet;assert.deepEqual(p,before);assert.equal(JSON.parse(serialize(s)).version,20);assert.equal(wire.version,17);assert.equal(wire.inventory.items[0].quantity,2);assert.equal(wire.magic.powers.find(v=>v.effect===5).attackModifier,1);assert.ok(wire.magic.powers.filter(v=>v.effect>=4).every(v=>!(v.flags&4)&&v.maximum===0));assert.match(s.powers.find(v=>v.reference===5).description,/5 lb\/unita/);assert.ok(r.readiness.snapshots.length&&r.conversion.complete===false);
 for(const version of [11,19])assert.throws(()=>parseBackup(JSON.stringify({...s,version})));assert.throws(()=>parseBackup(JSON.stringify({...s,powers:s.powers.filter(v=>v.reference===undefined),version:20})));
});
test('Reference: active choices, inactive/future/unequipped sources, duplicate bonuses and conflicting duplicates',()=>{
 const p=profileFixture(),c=p.data;c.modifiers.race=[{id:'language',type:'language',subType:'elvish',isGranted:false},{id:'initiative',type:'bonus',subType:'initiative',value:2,isGranted:true}];c.choices.race=[{id:'2-language',optionValue:1}];c.choices.choiceDefinitions=[{options:[{id:1,label:'Elvish'}]}];c.modifiers.race.push({...c.modifiers.race[1]});let r=full(p);assert.equal(r.details.reference.initiative.bonus,3);assert.match(group(r,'languages'),/elvish/);
 c.modifiers.race[2].value=3;r=full(p);assert.equal(r.details.reference.initiative.verified,false);assert.ok(!r.draft.powers.some(v=>v.reference===6));assert.equal(r.readiness.state,'review-required');
 c.modifiers.race=[];c.modifiers.item=[{id:77,type:'bonus',subType:'initiative',value:5,isGranted:true}];c.inventory[0].definition.grantedModifiers=[{...c.modifiers.item[0]}];assert.equal(full(p).details.reference.initiative.bonus,1);c.inventory[0].equipped=true;assert.equal(full(p).details.reference.initiative.bonus,6);
});
test('Reference: movement floors, bonus, walking match, armor restriction, explicit overrides and unknown formulas',()=>{
 const p=profileFixture(),c=p.data;c.modifiers.race=[{id:1,type:'bonus',subType:'speed',value:5,isGranted:true},{id:2,type:'set',subType:'innate-speed-flying',value:30,restriction:'Cannot be used if wearing medium or heavy armor.',isGranted:true}];let r=full(p);assert.match(group(r,'movement'),/Passo: 35 ft/);assert.match(group(r,'movement'),/Volo: 35 ft/);
 c.inventory[0].equipped=true;c.inventory[0].definition.armorTypeId=2;c.inventory[0].definition.filterType="Armor";c.inventory[0].definition.armorClass=14;r=full(p);assert.match(group(r,'movement'),/Volo: 0 ft/);delete c.inventory[0].definition.armorTypeId;c.inventory[0].definition.filterType="Adventuring Gear";delete c.inventory[0].definition.armorClass;
 c.modifiers.race=[{id:3,type:'set',subType:'innate-speed-flying',value:null,isGranted:true}];c.customSpeeds=[{movementId:1,distance:40}];assert.match(group(full(p),'movement'),/Volo: 40 ft/);c.customSpeeds=[{movementId:1,distance:0}];assert.match(group(full(p),'movement'),/Passo: 0 ft/);
 c.modifiers.race.push({id:4,type:'bonus',subType:'unarmored-movement',value:10,isGranted:true});assert.match(group(full(p),'movement'),/Passo: da verificare/);
});
test('Reference: sight floors/additions, custom sense mapping, poison passives and unresolved initiative',()=>{
 const p=profileFixture(),c=p.data;c.modifiers.race=[{id:1,type:'set-base',subType:'darkvision',value:60,isGranted:true},{id:2,type:'sense',subType:'darkvision',value:60,restriction:'plus 60 feet if wearer already has Darkvision',isGranted:true}];let r=full(p);assert.match(group(r,'senses'),/Scurovisione: 120 ft/);c.customSenses=[{senseId:2,distance:90},{senseId:1,distance:10}];r=full(p);assert.match(group(r,'senses'),/Scurovisione: 90 ft/);assert.match(group(r,'senses'),/Vista cieca: 10 ft/);
 c.conditions=[{id:11,level:0}];assert.match(group(full(p),'senses'),/Percezione passiva: 5/);c.modifiers.race.push({id:3,type:'advantage',subType:'initiative',value:null,isGranted:true});assert.equal(full(p).details.reference.initiative.verified,false);
 c.customSenses=[{senseId:99,distance:20}];assert.match(group(full(p),'senses'),/Scurovisione: da verificare/);
});
test('Reference: unavailable data never become fabricated zero; unknown remains on report and PC collection is untouched',()=>{
 const p=profileFixture();delete p.data.race.weightSpeeds;p.data.inventory[0].definition.weight=null;const r=full(p);assert.match(group(r,'movement'),/Passo: da verificare/);assert.match(r.draft.powers.find(v=>v.reference===5).description,/peso \?/);assert.ok(r.readiness.unresolved.some(t=>/Movimento/.test(t)));assert.deepEqual(r.draft.importInfo.limitations,r.conversion.omissions);assert.equal(r.checks.http,'reported-by-transport');
});
test('Resources: duplicate source actions/spells deduplicate exactly; conflicting balances block',()=>{
 let p=beyondRichFixture(),r=full(p),n=r.draft.powers.length;p.data.actions.class.push(structuredClone(p.data.actions.class[0]));assert.equal(full(p).draft.powers.length,n);p.data.actions.class[1].limitedUse.numberUsed++;assert.throws(()=>full(p),/duplicata e discordante/);
 p=beyondRichFixture();r=full(p);n=r.draft.powers.length;p.data.classSpells[0].spells.push(structuredClone(p.data.classSpells[0].spells[0]));assert.equal(full(p).draft.powers.length,n);p.data.classSpells[0].spells[2].prepared=false;assert.throws(()=>full(p),/duplicata e discordante/);
});

test('Reference report: conditional languages and unresolved senses remain visibly partial; accepted excerpts are not numeric errors',()=>{
 const p=profileFixture();p.data.modifiers.race=[{id:1,type:'language',subType:'elvish',isGranted:true,restriction:'Only in a special form'},{id:2,type:'sense',subType:'darkvision',value:30,isGranted:true,restriction:'Only in darkness'}];let r=full(p);assert.equal(r.conversion.coverage.find(v=>v.section==='Dati della scheda').status,'partial');assert.match(group(r,'languages'),/da verificare/);assert.ok(r.readiness.unresolved.some(v=>/Lingua/.test(v)));
 p.data.race.racialTraits=[{definition:{id:100,name:'Testo lungo sonda',description:'Parole sintetiche. '.repeat(600)}}];r=full(p);assert.ok(r.readiness.excerpts.length);assert.ok(!r.readiness.unresolved.some(v=>/estratto/.test(v)));assert.ok(r.conversion.omissions.some(v=>/estratto/.test(v)));
});
