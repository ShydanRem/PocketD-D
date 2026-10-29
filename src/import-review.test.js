import test from 'node:test';
import assert from 'node:assert/strict';
import {importReview} from './import-review.js';
import {initialState} from './model.js';
import {emptyExtended} from './extended.js';

test('Import review uses current values without mutating a sheet or declaring manual rules resolved',()=>{
  const s=initialState();s.extended=emptyExtended();s.extended.blockedChecks=['skill:arcana','save:COS'];
  s.importInfo={provider:'dndbeyond',characterId:'123456',scope:'extended-v1',limitations:['Regola sorgente non risolta']};const before=structuredClone(s);
  const r=importReview(s);assert.equal(r.blocked.length,2);assert.equal(r.missing.length,13);assert.equal(r.transfer.available,true);
  assert.deepEqual(s,before);s.extended.blockedChecks=[];s.extended.defences={configured:8191,resistance:0,vulnerability:0,immunity:0};
  const fixed=importReview(s);assert.equal(fixed.blocked.length,0);assert.equal(fixed.missing.length,0);assert.deepEqual(fixed.limitations,r.limitations);
  assert.equal(s.importInfo.limitations.length,1);
});
test('Import review keeps missing weapon and native capacity limits explicit',()=>{
  const s=initialState();s.equippedWeaponId=null;assert.equal(importReview(s).weapon,null);
  s.inventory=Array.from({length:61},(_,i)=>({...s.inventory[0],id:`row-${i}`}));
  const before=structuredClone(s);const r=importReview(s);assert.equal(r.transfer.available,false);assert.match(r.transfer.error,/60/);assert.deepEqual(s,before);
});
