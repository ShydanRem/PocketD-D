import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {inflateSync} from 'node:zlib';
import vm from 'node:vm';
import {convertBeyond,convertBeyondFull} from './beyond-conversion.js';
import {normalizeBeyondSource} from './beyond-normalize.js';
import {addBeyondReference} from './beyond-reference.js';
import {finalizeBeyondReport} from './beyond-report.js';
import {projectDevice} from './device-export.js';
import {beyondCoreFixture,beyondRichFixture} from './beyond-fixtures.js';

test('Beyond normalizes the full source once with exactly the former report/values/limitations',()=>{
  for(const fixture of [beyondCoreFixture,beyondRichFixture])for(let level=1;level<=20;level++){
    const p=fixture();p.data.classes[0].level=level;const before=structuredClone(p);
    const former=finalizeBeyondReport(addBeyondReference(convertBeyond(p,'123456'),normalizeBeyondSource(p).payload.data));
    assert.deepEqual(convertBeyondFull(p,'123456'),former);assert.deepEqual(p,before);
  }
});
test('Embedded source runs without browser/OS/RNG globals, matches PC packets and keeps __proto__ as data',()=>{
  const header=readFileSync(new URL('../firmware/core/beyond_bundle.h',import.meta.url),'utf8');
  const array=header.split('pd_beyond_bundle[]={')[1].split('};')[0],packed=Buffer.from([...array.matchAll(/0x([0-9a-f]{2})/g)].map(x=>parseInt(x[1],16)));
  const source=inflateSync(packed).toString('utf8');const context=vm.createContext({});vm.runInContext(source,context,{timeout:3000});
  for(const fixture of [beyondCoreFixture,beyondRichFixture]){
    const p=fixture(),before=structuredClone(p);context.input=JSON.stringify(p);
    const actual=vm.runInContext('pocketBeyondConvert(JSON.parse(input),"123456")',context,{timeout:3000});const expected=convertBeyondFull(p,'123456');
    assert.equal(actual.ready,expected.canImport);assert.deepEqual(JSON.parse(JSON.stringify(actual.packet)),projectDevice(expected.draft).packet);assert.deepEqual(p,before);
  }
  context.input='{"__proto__":{"pocketPolluted":true}}';assert.equal(vm.runInContext('Object.getPrototypeOf(structuredClone(JSON.parse(input)))===Object.prototype && !({}).pocketPolluted',context),true);
  assert.equal(vm.runInContext('typeof fetch+":"+typeof document+":"+typeof crypto+":"+typeof Math.random',context),'undefined:undefined:undefined:undefined');
});
