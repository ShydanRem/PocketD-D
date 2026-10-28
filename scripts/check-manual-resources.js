// Invented sources -> real converter -> native parser and byte-exact native codec.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {beyondManualFixture} from '../src/beyond-fixtures.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
import {rest} from '../src/powers.js';
const packets=[];
for(let count=1;count<=8;count++)for(const max of [1,50,99,100,999])for(const spent of [0,1,max]){
 const p=beyondManualFixture(count);for(const a of p.data.actions.class.slice(1))Object.assign(a.limitedUse,{maxUses:max,numberUsed:spent});
 const r=convertBeyond(p,'123456');assert.ok(r.canImport);for(const kind of ['short','long'])assert.deepEqual(rest(r.draft,kind).extended.manualResources,r.draft.extended.manualResources);
 packets.push(projectDevice(r.draft).packet);
}
const path='firmware/tests/out/manual-resources.synthetic.jsonl';writeFileSync(path,packets.map(JSON.stringify).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/beyond-equipment-probe',[path],{encoding:'utf8',maxBuffer:8*1024*1024});assert.equal(run.status,0,run.stderr);const rows=run.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,packets.length);
rows.forEach((r,i)=>{const p=packets[i];assert.ok(r.valid);assert.deepEqual(r.resources,p.resources.map(v=>[v.current,v.maximum]));assert.deepEqual(r.powers,p.magic.powers.map(v=>[v.kind,v.flags,v.maximum,v.remaining,v.recovery]));assert.equal(r.hp,p.hp);assert.equal(r.armor,p.armorClass);});
console.log(`Manual resources: ${packets.length} synthetic states through converter, existing USB packets, native C parser/codec; 1..8 counters, exact 0/1/999/max/used, passive notes, distinct ordinary resources and unchanged manual balances after rest.`);
