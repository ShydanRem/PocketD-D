// Synthetic Beyond -> actual converter -> actual USB parser / native codec / C checks.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {beyondCoreFixture} from '../src/beyond-fixtures.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
import {skills,abilityKeys} from '../src/model.js';
const names=['strength','dexterity','constitution','intelligence','wisdom','charisma'],packets=[],expected=[];
const modifier=n=>Math.floor((n-10)/2);
for(let target=0;target<6;target++)for(const level of [3,20])for(const cap of [0,2,4])for(const bonus of [0,2])for(const floor of [0,19,25])for(const override of [null,8]){
 const p=beyondCoreFixture(),c=p.data;c.modifiers.race=[];c.classes[0].level=level;c.baseHitPoints=level*5;
 // Remove unrelated future features: only the input under examination changes.
 c.modifiers.class=c.modifiers.class.filter(m=>m.id==='defense');
 c.stats[target].value=18;c.bonusStats[target].value=bonus;c.overrideStats[target].value=override;
 c.modifiers.feat.push({type:'bonus',subType:`${names[target]}-score`,value:4,isGranted:true});
 if(cap)c.modifiers.feat.push({type:'bonus',subType:'ability-score-maximum',statId:target+1,value:cap,isGranted:true});
 if(floor)c.modifiers.feat.push({type:'set',subType:`${names[target]}-score`,value:floor,restriction:'if not already higher',isGranted:true});
 const scores=c.stats.map(s=>s.value);scores[target]=override??Math.max(Math.min(20+cap,22)+bonus,floor);
 const maxHp=level*5+modifier(scores[2])*level,armor=10+modifier(scores[1])+Math.max(0,modifier(scores[2]));
 const r=convertBeyond(p,'123456');assert.ok(r.canImport,r.conversion.blockers.join(';'));assert.deepEqual(abilityKeys.map(k=>r.draft.abilities[k]),scores);assert.equal(r.draft.maxHp,maxHp);assert.equal(r.draft.armorClass,armor);
 packets.push(projectDevice(r.draft).packet);expected.push({scores,maxHp,hp:maxHp-4,armor});
}
const path='firmware/tests/out/beyond-abilities.synthetic.jsonl';writeFileSync(path,packets.map(JSON.stringify).join('\n')+'\n');
function probe(name){const r=spawnSync(`firmware/tests/out/${name}`,[path],{encoding:'utf8',maxBuffer:16*1024*1024,timeout:30000});assert.equal(r.status,0,r.stderr);const rows=r.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,expected.length);return rows;}
probe('beyond-equipment-probe').forEach((r,i)=>{const e=expected[i];assert.ok(r.valid);assert.deepEqual(r.abilities,e.scores);assert.equal(r.maxHp,e.maxHp);assert.equal(r.hp,e.hp);assert.equal(r.armor,e.armor);});
let comparisons=0;
probe('checks-probe').forEach((rows,i)=>{assert.equal(rows.length,30);for(const row of rows){const k=row.kind===1?skills.find(s=>s.id===row.key).ability:row.key,base=modifier(expected[i].scores[abilityKeys.indexOf(k)]);assert.equal(row.base,base);assert.equal(row.total,base);assert.equal(row.blocked,false);comparisons++;}});
console.log(`Beyond ability arithmetic -> USB -> C: ${expected.length} independent states, HP/AC/scores/native codec and ${comparisons} checks/skills/saves passed; synthetic only, no hardware or user collection.`);
