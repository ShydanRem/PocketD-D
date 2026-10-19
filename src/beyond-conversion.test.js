import {test} from 'node:test';
import assert from 'node:assert/strict';
import {convertBeyondCore as convertBeyond} from './beyond-conversion.js';
import {beyondCoreFixture} from './beyond-fixtures.js';
import {parseBackup,serialize} from './model.js';
import {createLibrary,addCharacter,serializeLibrary,parseLibrary,storeLibrary,LIBRARY_KEY} from './library.js';
const convert=p=>convertBeyond(p,'123456');
const blocked=p=>{const r=convert(p);assert.equal(r.canImport,false);assert.equal(r.draft,null);assert.ok(r.conversion.blockers.length);return r;};

test('Conversione: bonus razziali, talento senza ASI e livelli futuri non si sommano indiscriminatamente',()=>{
  const p=beyondCoreFixture(),before=structuredClone(p),r=convert(p);
  assert.equal(r.canImport,true);assert.equal(r.conversion.complete,false);
  assert.deepEqual(r.draft.abilities,{FOR:14,DES:12,COS:17,INT:10,SAG:11,CAR:8});
  assert.equal(r.draft.maxHp,29);assert.equal(r.draft.hp,25);assert.equal(r.draft.armorClass,14);
  assert.deepEqual(p,before);
});
test('Conversione: ASI irrisolto e origine di classe sconosciuta bloccano la creazione',()=>{
  const p=beyondCoreFixture();p.data.choices.class[0].optionValue=695;blocked(p);
  p.data.choices.class[0].optionValue=696;p.data.modifiers.class[0].componentId=9999;blocked(p);
});
test('Conversione: PF usano COS finale, bonus per livello e override del massimo senza doppi bonus',()=>{
  const p=beyondCoreFixture();p.data.modifiers.feat.push({type:'bonus',subType:'hit-points-per-level',value:2});
  assert.equal(convert(p).draft.maxHp,35);
  p.data.bonusHitPoints=3;p.data.overrideHitPoints=40;p.data.removedHitPoints=99;
  assert.equal(convert(p).draft.maxHp,40);assert.equal(convert(p).draft.hp,0);
});
test('Conversione: bonus PF di classe usa i livelli della classe, non quelli totali',()=>{
  const p=beyondCoreFixture();p.data.classes.push({level:2});
  p.data.modifiers.class.push({componentId:500,type:'bonus',subType:'hit-points-per-level',value:2});
  assert.equal(convert(p).draft.maxHp,20+3*5+2*3);
});
test('Conversione: override caratteristica e massimo ordinario; cap modificati non vengono indovinati',()=>{
  const p=beyondCoreFixture();p.data.stats[0].value=20;
  assert.equal(convert(p).draft.abilities.FOR,20);
  p.data.overrideStats[0].value=22;assert.equal(convert(p).draft.abilities.FOR,22);
  p.data.modifiers.feat.push({type:'bonus',subType:'strength-score-maximum',value:2});blocked(p);
});
test('Conversione: effetti condizionali e formule sconosciute bloccano i valori principali',()=>{
  for(const m of [{type:'bonus',subType:'armor-class',value:2,restriction:'while raging'},
    {type:'set',subType:'strength-score',value:19},
    {type:'bonus',subType:'hit-points',statId:3,value:null},
    {type:'bonus',subType:'constitution-score',value:2,isGranted:false}]){
    const p=beyondCoreFixture();p.data.modifiers.feat.push(m);blocked(p);
  }
});
test('Conversione: bonus oggetto solo se equipaggiato e in sintonia, provenienza irrisolta blocca',()=>{
  const p=beyondCoreFixture(),m={id:'item-effect',type:'bonus',subType:'armor-class',value:1};
  p.data.modifiers.item.push(m);p.data.inventory.push({quantity:1,equipped:false,isAttuned:false,definition:{name:'Amuleto sonda',filterType:'Other Gear',canAttune:true,grantedModifiers:[m]}});
  assert.equal(convert(p).draft.armorClass,14);
  p.data.inventory[2].equipped=true;assert.equal(convert(p).draft.armorClass,14);
  p.data.inventory[2].isAttuned=true;assert.equal(convert(p).draft.armorClass,15);
  p.data.inventory[2].definition.grantedModifiers=[];blocked(p);
});
test('Conversione: CA con armatura media, scudo, DES negativa e difesa senza armatura alternativa',()=>{
  const p=beyondCoreFixture();p.data.stats[1].value=18;
  p.data.inventory.push({quantity:1,equipped:true,definition:{name:'Armatura sonda',filterType:'Armor',armorTypeId:2,armorClass:14}});
  p.data.inventory.push({quantity:1,equipped:true,definition:{name:'Scudo sonda',filterType:'Armor',armorTypeId:4,armorClass:2}});
  assert.equal(convert(p).draft.armorClass,18);
  p.data.stats[1].value=8;assert.equal(convert(p).draft.armorClass,15);
  p.data.inventory.push(structuredClone(p.data.inventory[2]));blocked(p);
});
test('Conversione: inventario conserva duplicati, quantità ed equipaggiamento sorgente senza armi demo',()=>{
  const r=convert(beyondCoreFixture()),s=r.draft;
  assert.equal(s.inventory.length,2);assert.equal(s.inventory[0].quantity,2);assert.equal(s.inventory[1].quantity,1);
  assert.notEqual(s.inventory[0].id,s.inventory[1].id);assert.equal(s.inventory[0].name,s.inventory[1].name);
  assert.match(s.inventory[0].notes,/Beyond: equipaggiato/);assert.match(s.inventory[1].notes,/Beyond: non equipaggiato/);
  assert.equal(s.equippedWeaponId,null);assert.ok(s.inventory.every(i=>i.type==='gear'));
  assert.equal(s.powers.length,0);assert.equal(s.resolve,0);assert.equal(s.bonus,0);assert.deepEqual(s.history,[]);
});
test('Conversione: nomi, quantità e inventari fuori limite bloccano senza troncamenti',()=>{
  for(const patch of [p=>p.data.inventory[0].quantity=1000,p=>p.data.inventory[0].definition.name='x'.repeat(41),
    p=>p.data.inventory=Array.from({length:61},()=>structuredClone(p.data.inventory[0])),
    p=>p.data.customItems=[{name:'custom'}],p=>p.data.name='x'.repeat(25)]){
    const p=beyondCoreFixture();patch(p);blocked(p);
  }
});
test('Conversione: dati principali mancanti, duplicati o personalizzati non diventano valori plausibili',()=>{
  for(const patch of [p=>delete p.data.modifiers,p=>delete p.data.inventory,p=>delete p.data.baseHitPoints,
    p=>p.data.modifiers.race.push(structuredClone(p.data.modifiers.race[0])),p=>p.data.characterValues=[{typeId:1,value:3}],
    p=>p.data.preferences={enableOptionalOrigins:true},p=>p.data.modifiers.race={},p=>p.data.inventory=[null],p=>p.data.feats={},p=>p.data.choices.class={}]){
    const p=beyondCoreFixture();patch(p);blocked(p);
  }
});
test('Conversione: provenienza e omissioni sopravvivono a backup e raccolta, vecchie schede conservate',()=>{
  const r=convert(beyondCoreFixture()),old=createLibrary(),before=serializeLibrary(old);
  const next=addCharacter(old,r.draft,'imported');
  assert.equal(serializeLibrary(old),before);
  const restored=parseLibrary(serializeLibrary(next));assert.deepEqual(restored.characters[0],old.characters[0]);
  assert.deepEqual(restored.characters[1].state.importInfo,r.draft.importInfo);
  assert.equal(restored.characters[1].state.importInfo.scope,'core-v1');
  for(const info of [{...r.draft.importInfo,characterId:123},{...r.draft.importInfo,provider:'other'}, {...r.draft.importInfo,limitations:['x'.repeat(501)]}]){
    assert.throws(()=>parseBackup(serialize({...r.draft,importInfo:info})));
  }
});
test('Conversione: conflitto di storage e quota non sovrascrivono la raccolta durante aggiunta',()=>{
  const old=createLibrary(),raw=serializeLibrary(old),next=addCharacter(old,convert(beyondCoreFixture()).draft,'imported');
  let saved=raw;const storage={getItem:()=>saved,setItem:()=>{throw new Error('quota')}};
  assert.throws(()=>storeLibrary(storage,next,raw),/quota/);assert.equal(saved,raw);
  saved='changed';assert.throws(()=>storeLibrary(storage,next,raw),/altra scheda|altra|un’altra/);assert.equal(saved,'changed');
});

test('Conversione: un effetto principale equipaggiato non può mancare nel riepilogo dei modificatori',()=>{
  const p=beyondCoreFixture();
  p.data.inventory.push({quantity:1,equipped:true,definition:{name:'Oggetto sonda',filterType:'Other Gear',grantedModifiers:[{id:'missing',type:'bonus',subType:'armor-class',value:3}]}});
  blocked(p);
});
