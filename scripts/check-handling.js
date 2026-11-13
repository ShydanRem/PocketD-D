// Synthetic PC/USB23/native record/backup parity. No USB, account or personal PG.
import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {mkdtempSync,writeFileSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {initialState,serialize,parseBackup,weaponStats} from '../src/model.js';
import {projectDevice} from '../src/device-export.js';
import {inspectDeviceBackup} from '../src/device-backup.js';
const directory=mkdtempSync(join(tmpdir(),'pocket-handling-'));let cases=0;
try{
 for(const identity of ['', '123456789012345'])for(const finesse of [0,4])for(const kind of [0,1,2])for(const grip of [0,1])for(const ability of ['FOR','DES']){
  if(kind!==2&&grip)continue;
  const s=initialState();if(identity)s.importInfo={provider:'dndbeyond',characterId:identity,scope:'core-v1',limitations:[]};s.inventory[0].ability=ability;s.inventory[0].handling={flags:kind|finesse,grip,versatileSides:kind===2?10:0,ammoId:s.inventory[2].id};const p=projectDevice(parseBackup(serialize(s))).packet;
  writeFileSync(join(directory,'packet.json'),JSON.stringify(p));const r=spawnSync('firmware/tests/out/ui-build/handling-probe',[join(directory,'packet.json')],{encoding:'utf8',maxBuffer:300000});assert.equal(r.status,0,r.stderr);const out=JSON.parse(r.stdout);assert.ok(out);
  assert.deepEqual(out.handling,p.handling.map(h=>[h.flags,h.grip,h.sides,h.ammo]));const stats=weaponStats(s);assert.equal(out.stats.sides,stats.dieSides);assert.equal(out.stats.attack,stats.attackBonus);assert.equal(out.stats.damage,stats.damageBonus);
  const preview=inspectDeviceBackup(JSON.stringify(out.backup));assert.equal(preview.sourceId,identity);assert.deepEqual(preview.weaponHandling,[{index:0,...p.handling[0]}]);
  for(const sourceId of ['0','bad','1234567890123456','12\0hidden',42]){
   const bad={...p,sourceId};writeFileSync(join(directory,'packet.json'),JSON.stringify(bad));const rejected=spawnSync('firmware/tests/out/ui-build/handling-probe',[join(directory,'packet.json')],{encoding:'utf8'});assert.equal(rejected.status,0);assert.equal(JSON.parse(rejected.stdout),null);
  }
  const raw=Buffer.from(out.backup.state,'base64');assert.equal(raw.subarray(0,4).toString(),'PDDg');cases++;
  for(const field of ['flags','grip','sides','ammo']){
   const bad=structuredClone(p);bad.handling[0][field]={flags:8,grip:2,sides:7,ammo:61}[field];writeFileSync(join(directory,'packet.json'),JSON.stringify(bad));const rejected=spawnSync('firmware/tests/out/ui-build/handling-probe',[join(directory,'packet.json')],{encoding:'utf8'});assert.equal(rejected.status,0);assert.equal(JSON.parse(rejected.stdout),null);
  }
 }
 console.log(`Handling: ${cases} PC/USB23/C/backup comparisons, per-ID ammunition remapping, physical stats and four malformed fields per case passed; synthetic.`);
}finally{rmSync(directory,{recursive:true,force:true});}
