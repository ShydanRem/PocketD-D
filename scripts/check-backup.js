// Compare PC preview against a fresh real C encoder output, not a mirrored JS fixture.
import {readFileSync,writeFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import assert from 'node:assert/strict';
import {inspectDeviceBackup,crc32} from '../src/device-backup.js';
const raw=readFileSync('firmware/tests/out/backup-fixture.json','utf8'),p=inspectDeviceBackup(raw);
assert.equal(p.scene,1);assert.equal(p.name,'Elian');assert.equal(p.hp,17);assert.equal(p.maxHp,42);assert.equal(p.tempHp,7);assert.equal(p.armor,18);assert.equal(p.effects,2);assert.equal(p.powers,60);assert.equal(p.rolls.length,20);assert.equal(p.rolls[0].number,20);assert.equal(p.rolls[19].total,1);assert.equal(p.wallet.pp,999999999);assert.deepEqual(p.keys,[14,3,5]);assert.equal(p.brightness,80);assert.deepEqual(p.packet,JSON.parse(raw));assert.equal(p.nativeValidation,false);assert.equal(p.skillAbilities.intimidation,'COS');assert.equal(p.skillAbilities.acrobatics,'COS');
console.log('Native C -> PC: PDD34 mappings, all three scenes and 13 outfit IDs and key binding 14, exact records retained, HP/CA with local effects, wallet, settings, 60 powers and all 20 rolls preview passed; full semantic validation remains on device');
const inputs=[raw],expected=[true];
function mutate(key,fn){const copy=JSON.parse(raw),original=Uint8Array.from(atob(copy[key]),c=>c.charCodeAt(0)),modern=key==='state'&&(original[3]>=98&&original[3]<=100);let bytes=modern?new Uint8Array([...original.subarray(0,-1085),...original.subarray(-4)]):original;fn(bytes,new DataView(bytes.buffer));if(modern)bytes=new Uint8Array([...bytes.subarray(0,-4),...original.subarray(-1085,-4),...bytes.subarray(-4)]);new DataView(bytes.buffer).setUint32(bytes.length-4,crc32(bytes.subarray(0,-4)),true);copy[key]=btoa(String.fromCharCode(...bytes));return JSON.stringify(copy)}
for(let scene=0;scene<3;scene++){const q=mutate('state',b=>b[b.length-26]=scene);assert.equal(inspectDeviceBackup(q).scene,scene);inputs.push(q);expected.push(true);}
for(let lineage=0;lineage<21;lineage++)for(let body=0;body<2;body++){
 const q=mutate('state',b=>{b[b.length-25]=lineage;b[b.length-24]=body;});const preview=inspectDeviceBackup(q);assert.equal(preview.lineage,lineage);assert.equal(preview.body,body);inputs.push(q);expected.push(true);
}
for(let outfit=0;outfit<13;outfit++){const q=mutate('state',b=>b[b.length-23]=outfit);assert.equal(inspectDeviceBackup(q).outfit,outfit);inputs.push(q);expected.push(true);}
for(const [offset,value] of [[7,21],[6,2],[5,13]]){const q=mutate('state',b=>b[b.length-offset-18]=value);assert.throws(()=>inspectDeviceBackup(q));inputs.push(q);expected.push(false);}
const invalidScene=mutate('state',b=>b[b.length-26]=3);assert.throws(()=>inspectDeviceBackup(invalidScene));inputs.push(invalidScene);expected.push(false);
// Integrity can pass while a native field omitted from PC preview is invalid.
const invalidNative=mutate('state',b=>b[434]=2);assert.equal(inspectDeviceBackup(invalidNative).nativeValidation,false);
inputs.push(invalidNative);expected.push(false);
for(const [key,fn] of [['state',b=>b[64]=0],['history',(b,v)=>v.setUint32(8,123,true)],['history',b=>b[16]=21],['history',b=>b[122]=255]]){inputs.push(mutate(key,fn));expected.push(false)}
inputs.push(raw.replace('"version":1','"version":2'),raw.replace('"version":1','"version":1,"version":1'));expected.push(false,false);
const packet=JSON.parse(raw);inputs.push(JSON.stringify({history:packet.history,state:packet.state,version:1,format:packet.format}));expected.push(true);
const path='firmware/tests/out/backup-probe.jsonl';writeFileSync(path,inputs.join('\n')+'\n');
const run=spawnSync('firmware/tests/out/backup-probe',[path],{encoding:'utf8',timeout:15000});assert.equal(run.status,0,run.stderr);const rows=run.stdout.trim().split('\n').map(JSON.parse);assert.deepEqual(rows.map(r=>r.valid),expected);assert.deepEqual(rows[0],{valid:true,hp:17,maxHp:42,armor:18,powers:60,rolls:20,scene:1});
console.log(`PC -> native C restore: ${rows.length} integrity/semantic/epoch/duplicate/future-format cases passed, including CRC-valid invalid native data`);

// Corrupt the actual appended PDD34 values; retain valid CRC and exact envelope.
for(let i=0;i<18;i++){
 const packet=JSON.parse(raw),b=Uint8Array.from(atob(packet.state),c=>c.charCodeAt(0));b[b.length-22-(b[3]>=98?1081:0)+i]=7;new DataView(b.buffer).setUint32(b.length-4,crc32(b.subarray(0,-4)),true);packet.state=btoa(String.fromCharCode(...b));assert.throws(()=>inspectDeviceBackup(JSON.stringify(packet)));
}
