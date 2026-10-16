import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,changeHp,parseBackup,serialize} from './model.js';
import {upsertPower,deletePower,usePower,rest,validateMagic} from './powers.js';
const spell=(id='spell',extra={})=>({id,type:'spell',name:'Luce inventata',description:'Nota originale',level:1,concentration:true,...extra});
const feature=(id='feature',recovery='short')=>({id,type:'feature',name:'Passo del viandante',description:'Nota originale',maxUses:2,remaining:1,recovery});
const withSpell=()=>{let s=upsertPower(initialState(),spell());s.slots[0]={max:2,current:2};s.slots[1]={max:1,current:1};return s};
test('lancio e sovralancio consumano solo lo slot selezionato',()=>{
 const s=withSpell();const next=usePower(s,'spell',2);
 assert.equal(next.slots[1].current,0);assert.equal(next.slots[0].current,2);assert.equal(s.slots[1].current,1);assert.equal(next.concentration,'spell');
 assert.throws(()=>usePower(next,'spell',2));assert.throws(()=>usePower(s,'spell',0));
 assert.throws(()=>usePower(upsertPower(s,spell('higher',{level:3})),'higher',2));
});
test('trucchetti non consumano slot e magie non concentrate conservano la concentrazione',()=>{
 let s=usePower(withSpell(),'spell',1);s=upsertPower(s,spell('cantrip',{level:0,concentration:false}));const next=usePower(s,'cantrip');
 assert.deepEqual(next.slots,s.slots);assert.equal(next.concentration,'spell');
});
test('concentrazione unica, cancellazione/modifica e zero PF',()=>{
 let s=usePower(withSpell(),'spell',1);s=upsertPower(s,spell('second'));s=usePower(s,'second',2);assert.equal(s.concentration,'second');
 assert.equal(deletePower(s,'second').concentration,null);assert.equal(upsertPower(s,spell('second',{concentration:false})).concentration,null);
 assert.equal(changeHp(s,-99).concentration,null);assert.throws(()=>usePower({...s,hp:0,concentration:null},'spell',1));
});
test('capacità esaurite non scendono sotto zero; riposi distinguono i recuperi',()=>{
 let s=initialState();for(const kind of ['short','long','manual'])s=upsertPower(s,feature(kind,kind));
 s=usePower(s,'short');assert.equal(s.powers[0].remaining,0);assert.throws(()=>usePower(s,'short'));
 s={...s,hp:1,resolve:0};s.slots[0]={max:2,current:0};
 const brief=rest(s,'short');assert.equal(brief.hp,1);assert.equal(brief.resolve,0);assert.equal(brief.slots[0].current,0);assert.deepEqual(brief.powers.map(p=>p.remaining),[2,1,1]);
 const long=rest(s,'long');assert.equal(long.hp,32);assert.equal(long.slots[0].current,2);assert.deepEqual(long.powers.map(p=>p.remaining),[2,2,1]);assert.equal(long.potions,2);assert.equal(s.powers[0].remaining,0);
});
test('backup v5 completo e migrazione v4 senza inventare magie',()=>{
 const s=usePower(withSpell(),'spell',1);assert.deepEqual(parseBackup(serialize(s)),s);
 const legacy={...s,version:4};delete legacy.powers;delete legacy.slots;delete legacy.concentration;delete legacy.spellcastingAbility;
 const restored=parseBackup(JSON.stringify(legacy));assert.deepEqual(restored.powers,[]);assert.equal(restored.slots.length,9);assert.equal(restored.concentration,null);
});
test('rifiuto dati incoerenti, duplicati e riferimenti concentrazione invalidi',()=>{
 const s=withSpell();for(const patch of [
  {slots:[{max:1,current:2},...s.slots.slice(1)]},
  {powers:[spell(),spell()]},{concentration:'missing'},{hp:0,concentration:'spell'},
  {powers:[feature('a','unknown')]},{powers:[spell('x',{level:10})]},
  {powers:[spell('bad"id')]},{spellcastingAbility:'INVALID'}
 ])assert.throws(()=>validateMagic({...s,...patch}));
});
test('limite catalogo e modifica conservano contatori espliciti',()=>{
 let s=initialState();for(let i=0;i<40;i++)s=upsertPower(s,feature(`f${i}`));
 assert.throws(()=>upsertPower(s,feature('overflow')));
 const edited=upsertPower(s,{...feature('f0'),remaining:0});assert.equal(edited.powers.length,40);assert.equal(edited.powers[0].remaining,0);
 assert.equal(deletePower(edited,'f0').powers.length,39);
});
