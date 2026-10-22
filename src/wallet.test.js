import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,parseBackup,serialize} from './model.js';
import {emptyExtended} from './extended.js';
import {projectDevice} from './device-export.js';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
const coins={cp:1,sp:27,ep:300,gp:4021,pp:999999999};
test('USB9 transfers five known coin counts exactly and leaves the PC character unchanged',()=>{
 const s=initialState();s.extended={...emptyExtended(),currenciesAvailable:true,currencies:{...coins}};
 const before=structuredClone(s),{packet}=projectDevice(s);
 assert.equal(packet.version,9);assert.deepEqual(packet.wallet,{available:true,...coins});assert.deepEqual(s,before);
 packet.wallet.gp=0;assert.deepEqual(s,before);
});
test('verified zero and unknown balances remain different across backup and USB projection',()=>{
 for(const available of [false,true]){
  const s=initialState();s.extended={...emptyExtended(),currenciesAvailable:available};
  const restored=parseBackup(serialize(s));assert.equal(restored.extended.currenciesAvailable,available);
  assert.deepEqual(projectDevice(restored).packet.wallet,{available,cp:0,sp:0,ep:0,gp:0,pp:0});
 }
 assert.equal(projectDevice(initialState()).packet.wallet.available,false);
});
test('old PC backups infer knowledge only from a nonzero balance, without changing the amounts',()=>{
 const s=initialState();s.extended=emptyExtended();delete s.extended.currenciesAvailable;
 assert.equal(parseBackup(serialize(s)).extended.currenciesAvailable,false);
 s.extended.currencies.gp=42;
 const restored=parseBackup(serialize(s));assert.equal(restored.extended.currenciesAvailable,true);assert.equal(restored.extended.currencies.gp,42);
});
test('unverified PC amounts are retained locally and explicitly excluded from the device copy',()=>{
 const s=initialState();s.extended={...emptyExtended(),currencies:{...coins}};const before=structuredClone(s);
 const {packet,warnings}=projectDevice(s);assert.deepEqual(packet.wallet,{available:false,cp:0,sp:0,ep:0,gp:0,pp:0});
 assert(warnings.some(w=>w.includes('saldo non verificato')));assert.deepEqual(s,before);
});
test('synthetic Beyond conversion preserves known zero and missing currency data separately',()=>{
 const known=beyondRichFixture();known.data.currencies={cp:0,sp:0,ep:0,gp:0,pp:0};
 assert.equal(convertBeyond(known,'123456').draft.extended.currenciesAvailable,true);
 delete known.data.currencies;assert.equal(convertBeyond(known,'123456').draft.extended.currenciesAvailable,false);
 known.data.currencies={...coins,cp:-1};assert.throws(()=>convertBeyond(known,'123456'));
});
test('malformed money or availability rejects without silently clamping balances',()=>{
 const s=initialState();s.extended=emptyExtended();
 for(const value of [-1,1000000000,1.5,'2']){s.extended.currencies.gp=value;assert.throws(()=>projectDevice(s));}
 s.extended.currencies.gp=0;s.extended.currenciesAvailable='yes';assert.throws(()=>projectDevice(s));
});
