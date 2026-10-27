import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondCoreFixture,beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {emptyExtended} from './extended.js';
import {damageTypes,emptyDefences,defenceFlags,validateDefences} from './defences.js';
import {initialState,serialize,parseBackup,useItem} from './model.js';
import {projectDevice} from './device-export.js';
import {createLibrary,addCharacter,serializeLibrary,parseLibrary} from './library.js';
const convert=p=>convertBeyond(p,'123456');
const resistance=(subType='fire',type='resistance',extra={})=>({id:`${type}-${subType}`,type,subType,isGranted:true,restriction:'',duration:null,...extra});
function charged({maxUses=7,numberUsed=2,resetType='Long Rest',resetTypeDescription='',quantity=1}={}){return {id:991,quantity,equipped:false,chargesUsed:numberUsed,limitedUse:{maxUses,numberUsed,resetType,resetTypeDescription},definition:{id:881,name:'Scettro sonda',filterType:'Wondrous item',isConsumable:false,grantedModifiers:[]}};}
function potion(){return {id:992,quantity:4,equipped:false,chargesUsed:0,limitedUse:{maxUses:1,numberUsed:0,resetType:'Consumable',resetTypeDescription:''},definition:{id:882,name:'Cura sonda',filterType:'Potion',isConsumable:true,canAttune:false,isHomebrew:false,grantedModifiers:[{type:'bonus',subType:'hit-points',statId:null,restriction:'',duration:null,requiresAttunement:false,isGranted:true,value:null,fixedValue:2,dice:{diceCount:2,diceValue:4,diceMultiplier:null,fixedValue:2,diceString:'2d4 + 2'}}]}};}
test('Beyond item charges: exact spent/max, zero and absent quantities, explicit rest recovery and no source mutation',()=>{
 for(const quantity of [0,1])for(const max of [1,7,999])for(const used of [0,max])for(const resetType of ['Short Rest','Long Rest','Dawn','Unknown',1]){
  const p=beyondCoreFixture(),row=charged({quantity,maxUses:max,numberUsed:used,resetType});p.data.inventory.push(row);const before=structuredClone(p),r=convert(p),i=r.draft.inventory.at(-1);
  assert.deepEqual(i.charges,{max,current:max-used,recovery:resetType==='Short Rest'?'short':resetType==='Long Rest'?'long':'manual'});assert.equal(i.quantity,quantity);assert.equal(r.draft.hp,25);assert.deepEqual(p,before);assert.deepEqual(projectDevice(r.draft).packet.inventory.items.at(-1).charges,{maximum:max,current:max-used,recovery:resetType==='Short Rest'?1:resetType==='Long Rest'?2:0});
 }
});
test('Beyond item charges: malformed/discordant/stacked/formula counters never become invented balances',()=>{
 for(const mutate of [r=>r.quantity=2,r=>r.chargesUsed=3,r=>r.limitedUse.maxUses=-1,r=>r.limitedUse.maxUses=1000,r=>r.limitedUse.numberUsed=8,r=>r.limitedUse.numberUsed='2',r=>r.limitedUse.statModifierUsesId=3,r=>{delete r.limitedUse.numberUsed;delete r.chargesUsed;}]){
  const p=beyondCoreFixture(),row=charged();mutate(row);p.data.inventory.push(row);const report=convert(p);assert.ok(report.canImport);assert.equal(report.draft.inventory.at(-1).charges,undefined);assert.ok(report.conversion.omissions.some(s=>/cariche|saldi/.test(s)));
 }
 const p=beyondCoreFixture(),row=charged();delete row.limitedUse.numberUsed;p.data.inventory.push(row);assert.equal(convert(p).draft.inventory.at(-1).charges.current,5);
 row.limitedUse.resetTypeDescription='Regain 1d6 + 1';assert.equal(convert(p).draft.inventory.at(-1).charges.recovery,'manual');row.limitedUse.resetTypeDescription='';row.limitedUse.resetDice='1d6';assert.equal(convert(p).draft.inventory.at(-1).charges.recovery,'manual');
});
test('Beyond consumables: structured healing formula, quantity consumption, unchanged HP until use and no name inference',()=>{
 const p=beyondCoreFixture(),row=potion();p.data.inventory.push(row);const original=structuredClone(p),r=convert(p),item=r.draft.inventory.at(-1);assert.equal(item.type,'healing');assert.equal(item.diceCount,2);assert.equal(item.dieSides,4);assert.equal(item.healingModifier,2);assert.equal(item.charges,undefined);assert.equal(item.quantity,4);assert.equal(r.draft.hp,25);
 const used=useItem(r.draft,item.id,()=>0);assert.equal(used.state.inventory.at(-1).quantity,3);assert.equal(used.state.hp,29);assert.equal(used.healing,4);assert.deepEqual(p,original);
 row.definition.name='Not a healing name';assert.equal(convert(p).draft.inventory.at(-1).type,'healing');row.definition.grantedModifiers=[];row.definition.name='Potion of Healing';assert.equal(convert(p).draft.inventory.at(-1).type,'consumable');assert.ok(convert(p).conversion.omissions.some(t=>t.includes('pozione')));
});
test('Beyond potions: restrictions, extra effects, homebrew, dice limits and disagreeing fixed values stay manual',()=>{
 for(const mutate of [r=>r.definition.isHomebrew=true,r=>r.definition.grantedModifiers[0].duration=1,r=>r.definition.grantedModifiers[0].restriction='only undead',r=>r.definition.grantedModifiers[0].fixedValue=3,r=>r.definition.grantedModifiers[0].dice.diceCount=8,r=>r.definition.grantedModifiers[0].dice.diceMultiplier=2,r=>r.definition.grantedModifiers[0].dice.diceString='3d4+2',r=>r.definition.grantedModifiers.push({type:'bonus',subType:'strength-score',value:1})]){
  const p=beyondCoreFixture(),row=potion();mutate(row);p.data.inventory.push(row);const r=convert(p);assert.ok(r.canImport);assert.equal(r.draft.inventory.at(-1).type,'consumable');assert.equal(r.draft.hp,25);
 }
});
test('Beyond defenses: unconditional grants, simultaneous R/V/I, unknown absence, strict versions and preserved backup/collection',()=>{
 const p=beyondCoreFixture();p.data.modifiers.race.push(resistance(),resistance('cold','immunity'),resistance('fire','vulnerability'));const before=structuredClone(p),r=convert(p),d=r.draft.extended.defences;
 assert.equal(defenceFlags(d,damageTypes.indexOf('fire')),3);assert.equal(defenceFlags(d,damageTypes.indexOf('cold')),4);assert.equal(defenceFlags(d,0),null);assert.deepEqual(p,before);
 const backup=JSON.parse(serialize(r.draft)),device=projectDevice(r.draft).packet;assert.equal(backup.version,16);assert.equal(device.version,14);assert.equal(device.avatar.outfit,0);assert.deepEqual(device.defences,d);assert.deepEqual(parseBackup(JSON.stringify(backup)),r.draft);
 for(const version of [11,12,13,14,15])assert.throws(()=>parseBackup(JSON.stringify({...backup,version})));
 const library=createLibrary(),next=addCharacter(library,r.draft,'new');assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.equal(library.characters.length,1);
 const old=initialState();assert.equal(JSON.parse(serialize(old)).version,11);assert.equal(projectDevice(old).packet.version,9);
});
test('Beyond defenses: conditional/duplicate/dynamic grants invalidate affected type without erasing unrelated resolved types',()=>{
 for(const extra of [{restriction:'while raging'},{duration:1},{tagConstraints:[{x:1}]},{statId:1},{isGranted:'yes'}]){
  const p=beyondCoreFixture();p.data.modifiers.race.push(resistance(),resistance('cold'),resistance('fire','immunity',extra));const r=convert(p);assert.ok(r.canImport);assert.equal(defenceFlags(r.draft.extended.defences,5),null);assert.equal(defenceFlags(r.draft.extended.defences,4),1);assert.ok(r.conversion.omissions.some(s=>s.includes('Difesa fire')));
 }
 const p=beyondCoreFixture();p.data.modifiers.race.push(resistance(),resistance('cold','immunity',{id:'resistance-fire'}));assert.equal(convert(p).draft.extended.defences,undefined);
 p.data.modifiers.race=[resistance('fire'),resistance('alien-force')];assert.equal(convert(p).draft.extended.defences,undefined);
});
test('Beyond defenses: equipment/attunement and class levels determine eligibility; condition immunity remains an explicit note',()=>{
 const p=beyondCoreFixture(),m=resistance('acid'),row=charged();row.equipped=true;row.isAttuned=true;row.definition.canAttune=true;row.definition.grantedModifiers=[m];p.data.inventory.push(row);p.data.modifiers.item.push(m);
 assert.equal(defenceFlags(convert(p).draft.extended.defences,3),1);row.isAttuned=false;assert.equal(convert(p).draft.extended.defences,undefined);row.isAttuned=true;row.equipped=false;assert.equal(convert(p).draft.extended.defences,undefined);
 const classMod=resistance('cold','immunity',{componentId:502});p.data.modifiers.class.push(classMod);assert.equal(convert(p).draft.extended.defences,undefined);classMod.componentId=500;assert.equal(defenceFlags(convert(p).draft.extended.defences,4),4);
 p.data.modifiers.race.push(resistance('poisoned','immunity'));assert.ok(convert(p).conversion.omissions.some(t=>t.includes('condizione poisoned')));
});
test('Defence masks are strictly bounded and unknown types cannot carry active flags',()=>{
 for(const bad of [null,[],{configured:8192,resistance:0,vulnerability:0,immunity:0},{configured:0,resistance:1,vulnerability:0,immunity:0},{...emptyDefences(),extra:true},{...emptyDefences(),resistance:'0'}])assert.throws(()=>validateDefences(bad));
 const s={...initialState(),extended:{...emptyExtended(),defences:emptyDefences()}};assert.equal(projectDevice(s).packet.version,14);assert.equal(JSON.parse(serialize(s)).version,16);assert.equal(defenceFlags(s.extended.defences,0),null);
});
test('Item spells/actions with limited uses stay notes to avoid duplicating the physical charge pool',()=>{
 const p=beyondRichFixture();p.data.inventory.push(charged());p.data.actions.item=[{name:'Item action',componentId:881,limitedUse:{maxUses:7,numberUsed:2,resetType:2}}];p.data.spells.item=[{id:880,componentId:881,spellCastingAbilityId:4,usesSpellSlot:false,limitedUse:{maxUses:7,numberUsed:2,resetType:2},definition:{name:'Item spell',level:1,description:'Synthetic'}}];
 const r=convert(p);for(const name of ['Item action','Item spell']){const a=r.draft.powers.find(p=>p.name===name);assert.ok(a.passive);assert.equal(a.limited,undefined);assert.equal(a.remaining,0);}assert.equal(r.draft.inventory.at(-1).charges.current,5);
});

test('Beyond item defence grants retain hidden source requirements even when the summary omits them',()=>{
 for(const extra of [{duration:1},{tagConstraints:[{}]},{statId:1},{isGranted:'yes'},{requiresAttunement:true}]){
  const p=beyondCoreFixture(),m=resistance('acid'),row=charged();row.equipped=true;row.isAttuned=false;row.definition.canAttune=false;row.definition.grantedModifiers=[{...m,...extra}];p.data.inventory.push(row);p.data.modifiers.item.push(m);assert.equal(convert(p).draft.extended.defences,undefined);
 }
});
test('Beyond consumable state contradictions preserve a generic item without inferred healing or charges',()=>{
 for(const mutate of [r=>r.chargesUsed=1,r=>r.limitedUse.numberUsed=1,r=>r.limitedUse.resetType='Long Rest',r=>r.definition.isConsumable=false]){
  const p=beyondCoreFixture(),row=potion();mutate(row);p.data.inventory.push(row);const r=convert(p);assert.equal(r.draft.inventory.at(-1).type,'gear');assert.equal(r.draft.inventory.at(-1).charges,undefined);assert.equal(r.draft.inventory.at(-1).quantity,4);
 }
});
