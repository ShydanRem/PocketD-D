// Invented DDB custom training -> real converter -> USB -> native checks and codec.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {beyondCoreFixture} from '../src/beyond-fixtures.js';
import {beyondSkillIds} from '../src/beyond-training.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
const packets=[],expected=[],keys=['FOR','DES','COS','INT','SAG','CAR'];
const base={FOR:2,DES:1,COS:3,INT:0,SAG:0,CAR:-1,athletics:2,acrobatics:1,sleightOfHand:1,stealth:1,arcana:0,history:0,investigation:0,nature:0,religion:0,animalHandling:0,insight:0,medicine:0,perception:0,survival:0,deception:-1,intimidation:-1,performance:-1,persuasion:-1};
const custom=(typeId,valueId,value)=>({typeId,valueId,value,valueTypeId:0});
for(let level=1;level<=20;level++)for(const grade of [1,2,3,4])for(const saveGrade of [1,3]){
 const p=beyondCoreFixture(),c=p.data;c.classes[0].level=level;c.modifiers.class=c.modifiers.class.filter(m=>m.id!=='future');c.characterValues=[];
 c.modifiers.feat.push({type:'half-proficiency',subType:'ability-checks'},{type:'proficiency',subType:'saving-throws'},{type:'bonus',subType:'saving-throws',value:-1});
 for(const [id,key] of Object.entries(beyondSkillIds)){
  c.modifiers.feat.push({type:'expertise',subType:key.replace(/[A-Z]/g,m=>'-'+m.toLowerCase())},{type:'bonus',subType:key.replace(/[A-Z]/g,m=>'-'+m.toLowerCase()),value:1});
  c.characterValues.push(custom(26,id,grade),custom(24,id,level%7-3),custom(25,id,grade-2));
 }
 for(let id=1;id<=6;id++)c.characterValues.push(custom(41,id,saveGrade),custom(39,id,2-level%5),custom(40,id,grade-3));
 const poisoned=level%3===0,blocked=level%4===0;c.conditions=poisoned?[{id:11}]:[];
 if(blocked)c.modifiers.feat.push({type:'bonus',subType:'perception',statId:4,value:null});
 const r=convertBeyond(p,'123456');assert.ok(r.canImport,r.conversion.blockers.join(';'));packets.push(projectDevice(r.draft).packet);expected.push({level,grade,saveGrade,poisoned,blocked});
}
const path='firmware/tests/out/beyond-training.synthetic.jsonl';writeFileSync(path,packets.map(JSON.stringify).join('\n')+'\n');
function probe(name){const r=spawnSync(`firmware/tests/out/${name}`,[path],{encoding:'utf8',maxBuffer:16*1024*1024,timeout:30000});assert.equal(r.status,0,r.stderr);const rows=r.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,expected.length);return rows;}
let compared=0;
probe('checks-probe').forEach((rows,i)=>{const e=expected[i],p=2+Math.floor((e.level-1)/4);assert.equal(rows.length,30);for(const row of rows){
 const rank=row.kind===1?(e.grade===4?2:e.grade===3?1:0):row.kind===2?Number(e.saveGrade===3):0;
 const half=row.kind===0||row.kind===1&&e.grade===2;
 const trained=rank*p+(half?Math.floor(p/2):0),extra=row.kind===1?1+e.level%7-3+e.grade-2:row.kind===2?-1+2-e.level%5+e.grade-3:0;
 assert.equal(row.base,base[row.key]);assert.equal(row.trained,trained);assert.equal(row.extra,extra);assert.equal(row.total,base[row.key]+trained+extra);assert.equal(row.blocked,row.kind===1&&row.key==='perception'&&e.blocked);
 for(let j=0;j<3;j++){const advantage=j===2,disadvantage=j===0||e.poisoned&&row.kind!==2;assert.equal(row.modes[j],advantage===disadvantage?0:advantage?1:-1);compared++;}
}});
probe('beyond-equipment-probe').forEach((r,i)=>{assert.ok(r.valid);assert.deepEqual(r.abilities,[14,12,17,10,11,8]);assert.equal(r.maxHp,20+3*expected[i].level);assert.equal(r.hp,16+3*expected[i].level);assert.equal(r.armor,14);});
console.log(`Beyond custom training -> USB9 -> C: ${expected.length} source states, all 18 skills and 6 saves, levels 1..20, exact bonus/rank/half/blocked/modes, ${compared} comparisons, native backup roundtrip passed. Synthetic only; no USB hardware or personal collection.`);
