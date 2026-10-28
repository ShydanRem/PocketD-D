import test from 'node:test';
import assert from 'node:assert/strict';
import {emptyDefences,completeNormalDefences,defenceFlags} from './defences.js';
import {beyondCoreFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {serialize,parseBackup} from './model.js';
import {projectDevice} from './device-export.js';

test('Manual defence completion: all 8192 known/unknown subsets preserve every existing flag and source',()=>{
 for(let mask=0;mask<8192;mask++){
  const d={configured:mask,resistance:mask&0x1555,vulnerability:mask&0xccc,immunity:mask&0x1f0},before=structuredClone(d),done=completeNormalDefences(d);
  assert.deepEqual(d,before);assert.notEqual(done,d);assert.equal(done.configured,8191);
  for(let i=0;i<13;i++)assert.equal(defenceFlags(done,i),mask&(1<<i)?defenceFlags(d,i):0);
  assert.deepEqual(completeNormalDefences(done),done);
 }
});
test('Beyond absence stays unknown until explicit manual completion; PC backup and device preserve the decision',()=>{
 const source=beyondCoreFixture(),report=convertBeyond(source,'123456'),before=serialize(report.draft);
 assert.equal(report.draft.extended.defences,undefined);assert.equal(projectDevice(report.draft).packet.defences,undefined);
 const next={...report.draft,extended:{...report.draft.extended,defences:completeNormalDefences(emptyDefences())}};
 const restored=parseBackup(serialize(next));assert.equal(restored.extended.defences.configured,8191);
 assert.deepEqual(projectDevice(restored).packet.defences,{configured:8191,resistance:0,vulnerability:0,immunity:0});
 assert.equal(serialize(report.draft),before);assert.equal(convertBeyond(source,'123456').draft.extended.defences,undefined);
});
test('Manual defence completion rejects corrupt profiles without repairing or mutating them',()=>{
 for(const d of [null,undefined,[],{}, {...emptyDefences(),configured:8192},{...emptyDefences(),resistance:1},{...emptyDefences(),extra:0}]){
  const before=structuredClone(d);assert.throws(()=>completeNormalDefences(d));assert.deepEqual(d,before);
 }
});
