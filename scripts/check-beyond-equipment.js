// Synthetic source -> real converter -> USB -> actual native parser, persistence and damage math.
import assert from 'node:assert/strict';
import {writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {beyondCoreFixture} from '../src/beyond-fixtures.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
import {damageTypes,defenceFlags} from '../src/defences.js';
const packets=[];
for(let flags=1;flags<8;flags++)for(let target=0;target<13;target++){
 const p=beyondCoreFixture();for(const [kind,bit] of [['resistance',1],['vulnerability',2],['immunity',4]])if(flags&bit)p.data.modifiers.race.push({id:`${kind}-${target}`,type:kind,subType:damageTypes[target],isGranted:true});
 p.data.inventory.push({id:990,quantity:1,equipped:false,chargesUsed:2,limitedUse:{maxUses:7,numberUsed:2,resetType:'Long Rest'},definition:{name:'Charged probe',filterType:'Other Gear',grantedModifiers:[]}}, {id:991,quantity:4,equipped:false,limitedUse:{maxUses:1,numberUsed:0,resetType:'Consumable'},definition:{name:'Healing probe',filterType:'Potion',isConsumable:true,grantedModifiers:[{type:'bonus',subType:'hit-points',fixedValue:2,dice:{diceCount:2,diceValue:4,fixedValue:2,diceString:'2d4+2'}}]}});
 const report=convertBeyond(p,'123456');assert.ok(report.canImport);report.draft.avatar={...report.draft.avatar,lineage:target%21,body:target%2,outfit:flags};packets.push(projectDevice(report.draft).packet);
}
const valid=packets.length;
for(const change of [p=>delete p.defences,p=>p.defences.configured=8192,p=>p.defences.resistance=8192,p=>p.defences.immunity=2,p=>p.defences.extra=0,p=>p.defences.resistance='1',p=>p.defences.configured=-1,p=>p.version=13,p=>p.version=16,p=>delete p.avatar.outfit,p=>p.defences=null,p=>p.defences.resistance=.5]){const p=structuredClone(packets[0]);change(p);packets.push(p);}
const path='firmware/tests/out/beyond-equipment.jsonl';writeFileSync(path,packets.map(JSON.stringify).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/beyond-equipment-probe',[path],{encoding:'utf8'});assert.equal(run.status,0,run.stderr);const rows=run.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,packets.length);
for(let n=0;n<valid;n++){
 const row=rows[n],p=packets[n];assert.ok(row.valid);assert.deepEqual(row.defences,['configured','resistance','vulnerability','immunity'].map(k=>p.defences[k]));assert.deepEqual(row.avatar,[p.avatar.lineage,p.avatar.body,p.avatar.outfit]);
 assert.deepEqual(row.items.at(-2),[0,1,0,0,0,5,7,2]);assert.deepEqual(row.items.at(-1),[2,4,2,4,2,0,0,0]);
 for(let type=0;type<13;type++){const f=defenceFlags(p.defences,type),d=row.damage[type];assert.equal(d[0],f!==null);assert.equal(d[3],f!==null);if(f!==null){const expected=f&4?0:(f&1?4:9)*(f&2?2:1);assert.equal(d[1],f);assert.equal(d[2],expected);}}
}
for(const row of rows.slice(valid))assert.equal(row.valid,false);
console.log(`Beyond -> USB14 -> C: ${valid} R/V/I type combinations, charges/healing items, native backup roundtrip, odd damage rounding and ${rows.length-valid} atomic malformed/old-version rejections passed; synthetic data only.`);
