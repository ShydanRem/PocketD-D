// Source type 27 -> production conversion -> wire -> native parser/calculation/codec.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {beyondCoreFixture} from '../src/beyond-fixtures.js';
import {beyondSkillIds} from '../src/beyond-training.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
const packets=[],expected=[],cv=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
for(const [id,key] of Object.entries(beyondSkillIds))for(let a=0;a<6;a++)for(const level of [1,5,20]){
 const p=beyondCoreFixture(),c=p.data;c.stats.forEach((v,i)=>v.value=8+2*i);c.modifiers.race=[];c.modifiers.class=c.modifiers.class.filter(m=>m.id!=='future');c.classes[0].level=level;const grade=a%4+1;c.characterValues=[cv(27,id,a+1),cv(26,id,grade),cv(24,id,-2),cv(25,id,1)];
 const r=convertBeyond(p,'123456');assert.ok(r.canImport);r.draft.extended.combat={...r.draft.extended.combat,rageBonus:2,raging:true};r.draft.condition=level===5;
 packets.push(projectDevice(r.draft).packet);expected.push({key,a,grade,level});
}
const valid=packets.length;
for(const mutate of [p=>delete p.training.skillAbilities,p=>p.training.skillAbilities.perception=7,p=>p.training.skillAbilities.perception=-1,p=>p.training.skillAbilities.perception='1',p=>p.training.skillAbilities.perception=.5,p=>delete p.training.skillAbilities.perception,p=>p.training.skillAbilities.extra=0,p=>p.training.skillAbilities=null,p=>p.training.skillAbilities=[],p=>p.version=14,p=>p.version=17]){const p=structuredClone(packets[0]);mutate(p);packets.push(p);}
const path='firmware/tests/out/skill-abilities.synthetic.jsonl';writeFileSync(path,packets.map(JSON.stringify).join('\n')+'\n');
const native=spawnSync('firmware/tests/out/beyond-equipment-probe',[path],{encoding:'utf8',maxBuffer:16*1024*1024});assert.equal(native.status,0,native.stderr);const records=native.stdout.trim().split('\n').map(JSON.parse);assert.equal(records.length,packets.length);assert.ok(records.slice(0,valid).every(r=>r.valid));assert.ok(records.slice(valid).every(r=>!r.valid));
writeFileSync(path,packets.slice(0,valid).map(JSON.stringify).join('\n')+'\n');
const checks=spawnSync('firmware/tests/out/checks-probe',[path],{encoding:'utf8',maxBuffer:16*1024*1024});assert.equal(checks.status,0,checks.stderr);const rows=checks.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,valid);
rows.forEach((list,i)=>{const e=expected[i],r=list.find(r=>r.kind===1&&r.key===e.key),p=2+Math.floor((e.level-1)/4),rank=e.grade===4?2:e.grade===3?1:0;assert.equal(r.base,e.a-1);assert.equal(r.trained,rank*p+(e.grade===2?Math.floor(p/2):0));assert.equal(r.extra,-1);assert.equal(r.total,e.a-2+rank*p+(e.grade===2?Math.floor(p/2):0));for(let j=0;j<3;j++){const advantage=j===2||e.a===0,disadvantage=j===0||e.level===5;assert.equal(r.modes[j],advantage===disadvantage?0:advantage?1:-1);}});
console.log(`Skill associations -> USB15 -> C: ${valid} states (18 skills x 6 abilities x 3 levels), exact rank/half/extra, STR/poison modes, native persistence and ${packets.length-valid} atomic invalid-packet rejections passed; synthetic only.`);
