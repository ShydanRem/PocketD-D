import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup} from './model.js';
import {validateItem,upsertItem,adjustQuantity,recoverItemCharges} from './inventory.js';
import {emptyExtended} from './extended.js';
import {rest} from './powers.js';
import {projectDevice} from './device-export.js';
import {createLibrary,addCharacter,updateCharacter,serializeLibrary,parseLibrary} from './library.js';
const item=(charges={current:2,max:7,recovery:'short'})=>({id:'charged',name:'Scudo sintetico',notes:'',quantity:1,type:'gear',charges});
test('cariche esplicite: validazione stretta e copie indipendenti',()=>{
 const v=item(),copy=validateItem(v);assert.deepEqual(copy,v);copy.charges.current=0;assert.equal(v.charges.current,2);
 for(const charges of [null,[],{current:0,max:0,recovery:'manual'},{current:8,max:7,recovery:'short'},{current:-1,max:7,recovery:'short'},{current:1.5,max:7,recovery:'short'},{current:0,max:1000,recovery:'manual'},{current:0,max:7,recovery:'dawn'},{current:0,max:7,recovery:'manual',extra:true},{max:7,recovery:'manual'}])assert.throws(()=>validateItem(item(charges)));
 assert.equal(validateItem({...item(),charges:undefined}).charges,undefined);
});
test('backup v11 conserva cariche; v9 e v10 senza cariche restano leggibili',()=>{
 const s=upsertItem(initialState(),item());assert.deepEqual(parseBackup(serialize(s)),s);assert.equal(JSON.parse(serialize(s)).version,11);
 for(const version of [9,10]){
  const raw=JSON.parse(serialize(initialState()));raw.version=version;
  if(version===10)raw.extended=emptyExtended();
  assert.deepEqual(parseBackup(JSON.stringify(raw)),{...initialState(),...(version===10?{extended:raw.extended}:{})});
  raw.inventory.push(item());assert.throws(()=>parseBackup(JSON.stringify(raw)));
 }
});
test('quantità e cariche sono indipendenti, nessun consumo implicito',()=>{
 const s=upsertItem(initialState(),item()),raw=serialize(s),next=adjustQuantity(s,'charged',-1);
 assert.equal(next.inventory.at(-1).quantity,0);assert.equal(next.inventory.at(-1).charges.current,2);assert.equal(serialize(s),raw);
 const cleared=upsertItem(s,{...item(),charges:undefined});assert.equal(cleared.inventory.at(-1).charges,undefined);
});
test('riposi: solo oggetti presenti, recupero pieno secondo la politica, quantità invariata',()=>{
 const inventory=['manual','short','long'].flatMap(recovery=>[0,1,999].map(quantity=>({...item({current:2,max:7,recovery}),id:`${recovery}-${quantity}`,quantity})));
 const s={...initialState(),inventory,equippedWeaponId:null},raw=serialize(s);
 for(const kind of ['short','long']){
  const next=rest(s,kind);assert.deepEqual(next.inventory,recoverItemCharges(inventory,kind));
  next.inventory.forEach((v,i)=>{const old=inventory[i];assert.equal(v.quantity,old.quantity);assert.equal(v.charges.current,v.quantity>0&&(v.charges.recovery==='short'||kind==='long'&&v.charges.recovery==='long')?7:2)});
 }
 assert.equal(serialize(s),raw);
});
test('proiezione USB9 trasferisce esattamente 60 saldi senza alterare la sorgente',()=>{
 const s={...initialState(),inventory:Array.from({length:60},(_,i)=>({...item({current:i,max:999,recovery:['manual','short','long'][i%3]}),id:`row-${i}`,quantity:i})),equippedWeaponId:null},raw=serialize(s);
 const {packet}=projectDevice(s);assert.equal(packet.version,9);assert.equal(packet.inventory.items.length,60);
 packet.inventory.items.forEach((v,i)=>{assert.deepEqual(v.charges,{current:i,maximum:999,recovery:i%3});assert.equal(v.quantity,i)});assert.equal(serialize(s),raw);
 const plain=projectDevice(initialState()).packet;assert.deepEqual(plain.inventory.items[0].charges,{current:0,maximum:0,recovery:0});
});
test('raccolta: configurare cariche conserva le altre schede e il backup le ripristina',()=>{
 const original=addCharacter(createLibrary(),initialState(),'second');
 const next=updateCharacter(original,upsertItem(initialState(),item()));assert.deepEqual(next.characters[0],original.characters[0]);
 assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.equal(original.characters[1].state.inventory.length,3);
});
