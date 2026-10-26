import {test} from 'node:test';
import assert from 'node:assert/strict';
import {beyondCoreFixture,beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond,convertBeyondCore} from './beyond-conversion.js';
import {projectDevice} from './device-export.js';
import {parseBackup,serialize} from './model.js';
import {createLibrary,addCharacter} from './library.js';
const core=p=>convertBeyondCore(p,'123456'),full=p=>convertBeyond(p,'123456');
function chosen(p,{group='race',id='choice-score',ability=4,value=1}={}){
 const names=['strength','dexterity','constitution','intelligence','wisdom','charisma'],labels=['Strength','Dexterity','Constitution','Intelligence','Wisdom','Charisma'];
 const m={id,componentId:500,type:'bonus',subType:`${names[ability-1]}-score`,value,isGranted:false,entityId:ability};p.data.modifiers[group].push(m);
 p.data.choices[group]??=[];p.data.choices[group].push({id:`2-${id}`,componentId:500,type:2,subType:5,optionValue:12000+ability,optionIds:[12000+ability]});
 p.data.choices.choiceDefinitions??=[];p.data.choices.choiceDefinitions.push({options:[{id:12000+ability,label:labels[ability-1]+' Score'}]});return m;
}
function custom(p,{id=7001,name='Oggetto sintetico',quantity=2,merged=true}={}){
 p.data.customItems??=[];p.data.customItems.push({id,name,description:'Nota inventata.',quantity});
 if(merged)p.data.inventory.push({id:8000+id,quantity,equipped:false,definition:{id,name,isCustomItem:true,filterType:null,grantedModifiers:[],armorTypeId:null,armorClass:null,damage:null,fixedDamage:null}});
}
test('Reconciliation: optional toggles alone do not block; selected replacements and malformed data still do',()=>{
 const p=beyondCoreFixture();p.data.preferences={enableOptionalOrigins:true,enableOptionalClassFeatures:true};p.data.optionalOrigins=[];p.data.optionalClassFeatures=[];assert.ok(core(p).canImport);
 p.data.optionalOrigins=[{id:1}];assert.equal(core(p).canImport,false);p.data.optionalOrigins=[];p.data.optionalClassFeatures={id:1};assert.equal(core(p).canImport,false);
});
test('Reconciliation: selected ability bonuses across six stats and two amounts require matching IDs/options; source unchanged',()=>{
 const keys=['FOR','DES','COS','INT','SAG','CAR'],base={FOR:14,DES:12,COS:17,INT:10,SAG:11,CAR:8};
 for(const group of ['race','class'])for(let ability=1;ability<=6;ability++)for(const value of [1,2]){
  const p=beyondCoreFixture();chosen(p,{group,ability,value});const before=structuredClone(p),r=core(p);assert.ok(r.canImport,JSON.stringify(r.conversion.blockers));assert.deepEqual(r.draft.abilities,{...base,[keys[ability-1]]:base[keys[ability-1]]+value});assert.deepEqual(p,before);
 }
 for(const mutate of [p=>p.data.choices.race[0].optionValue=null,p=>p.data.choices.race[0].optionIds=[],p=>p.data.choices.choiceDefinitions[0].options[0].label='Strength Score',p=>p.data.modifiers.race.at(-1).entityId=2,p=>p.data.modifiers.race.at(-1).value=9,p=>p.data.choices.race.push(structuredClone(p.data.choices.race[0]))]){
  const p=beyondCoreFixture();chosen(p);mutate(p);assert.equal(core(p).canImport,false);
 }
});
test('Reconciliation: item names/notes are targeted cosmetic overrides; numeric/ambiguous/missing targets remain blocked',()=>{
 const p=beyondRichFixture();p.data.inventory[0].id=100;p.data.characterValues=[{typeId:8,valueId:'100',valueTypeId:'1439493548',value:'Nome locale sonda'},{typeId:9,valueId:'100',valueTypeId:'1439493548',value:'<b>Nota inventata</b><script>bad()</script>'}];const before=structuredClone(p),r=full(p);assert.ok(r.canImport);assert.equal(r.draft.inventory[0].name,'Nome locale sonda');assert.match(r.draft.inventory[0].notes,/Nota inventata/);assert.doesNotMatch(r.draft.inventory[0].notes,/<script>|bad\(\)/);assert.equal(r.draft.armorClass,14);assert.deepEqual(p,before);
 for(const mutate of [p=>p.data.characterValues[0].typeId=1,p=>p.data.characterValues[0].valueId='missing',p=>p.data.characterValues[0].valueTypeId=1,p=>p.data.characterValues.push({...p.data.characterValues[0],value:'Different'}),p=>p.data.characterValues[0].value='x'.repeat(41)]){const q=structuredClone(p);mutate(q);assert.equal(full(q).canImport,false);}
});
test('Reconciliation: custom rows merge by ID once, preserve zero and duplicate names, and include standalone definitions explicitly',()=>{
 const p=beyondRichFixture();custom(p,{id:7001,quantity:0});custom(p,{id:7002,quantity:3});custom(p,{id:7003,quantity:7,merged:false});const before=structuredClone(p),r=full(p);
 assert.ok(r.canImport);assert.equal(r.draft.inventory.length,6);assert.deepEqual(r.draft.inventory.slice(-3).map(i=>i.quantity),[0,3,7]);assert.equal(new Set(r.draft.inventory.slice(-3).map(i=>i.id)).size,3);assert.equal(r.inventory.items.at(-1).equipped,null);assert.match(r.draft.inventory.at(-1).notes,/equipaggiamento non disponibile/);assert.deepEqual(r.conversion.sourceCounts,{inventory:5,customDefinitions:3,addedCustom:1,mergedInventory:6});assert.ok(r.draft.importInfo.limitations.some(x=>x.includes('Oggetti personalizzati')));assert.deepEqual(p,before);
  assert.match(r.draft.inventory.at(-3).notes,/Nota inventata/);
  const projected=projectDevice(r.draft).packet;assert.equal(projected.inventory.items.length,6);assert.deepEqual(projected.inventory.items.slice(-3).map(i=>i.quantity),[0,3,7]);assert.ok(projected.inventory.items.slice(-3).every(i=>i.kind===0));
 const library=createLibrary(),next=addCharacter(library,r.draft,'imported-test');assert.deepEqual(next.characters[0],library.characters[0]);assert.deepEqual(parseBackup(serialize(r.draft)),r.draft);
});
test('Reconciliation: custom conflicts, missing definitions, mechanics and capacity overflow cannot be silently accepted',()=>{
 for(const mutate of [p=>p.data.customItems[0].quantity=4,p=>p.data.customItems.push(structuredClone(p.data.customItems[0])),p=>p.data.inventory.at(-1).definition.grantedModifiers=[{type:'bonus',subType:'armor-class',value:2}],p=>p.data.inventory.at(-1).definition.armorClass=13,p=>p.data.customItems=[],p=>p.data.inventory.push(structuredClone(p.data.inventory.at(-1)))]){const p=beyondCoreFixture();custom(p);mutate(p);assert.equal(core(p).canImport,false);}
 const p=beyondCoreFixture();p.data.inventory=Array.from({length:60},(_,i)=>({id:i,quantity:1,equipped:false,definition:{name:'Gear',filterType:'Other Gear'}}));custom(p,{merged:false});assert.equal(core(p).canImport,false);
});
test('Reconciliation: null death counters mean empty counters; invalid values remain rejected',()=>{
 const p=beyondRichFixture();p.data.deathSaves={successCount:null,failCount:null,isStabilized:false};assert.deepEqual(full(p).draft.extended.deathSaves,{successes:0,failures:0,stabilized:false});p.data.deathSaves.failCount='2';assert.throws(()=>full(p),/Dati estesi/);
});
test('Reconciliation: explicit single half-caster table provides slot maxima while source counters provide consumption',()=>{
 const p=beyondRichFixture(),cl=p.data.classes[0];cl.definition.canCastSpells=true;cl.definition.spellRules={multiClassSpellSlotDivisor:2,levelSpellSlots:Array.from({length:21},()=>[0,0,0,0,0,0,0,0,0])};cl.definition.spellRules.levelSpellSlots[cl.level]=[3,1,0,0,0,0,0,0,0];p.data.pactMagic=[];p.data.spellSlots=Array.from({length:9},(_,i)=>({level:i+1,available:0,used:i===1?1:0}));
 let r=full(p);assert.deepEqual(r.draft.slots.slice(0,2),[{max:3,current:3},{max:1,current:0}]);assert.deepEqual(projectDevice(r.draft).packet.magic.slots.slice(0,2),[{maximum:3,current:3},{maximum:1,current:0}]);
 p.data.spellSlots[1].used=2;r=full(p);assert.ok(r.conversion.omissions.some(x=>x.includes('Slot ordinari')));assert.ok(r.draft.slots.every(s=>s.max===0));
  p.data.spellSlots=p.data.spellSlots.filter(s=>s.level!==2);r=full(p);assert.ok(r.conversion.omissions.some(x=>x.includes('consumi mancanti')));assert.ok(r.draft.slots.every(s=>s.max===0));
  p.data.spellSlots.push({level:2,available:0,used:1});
  cl.definition.spellRules.levelSpellSlots[cl.level]=[3];r=full(p);assert.ok(r.conversion.omissions.some(x=>x.includes('Tabella slot')));
});
