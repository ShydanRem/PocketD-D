// Real PC projection and production C parser/codec, synthetic characters only.
import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {mkdtempSync,writeFileSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {initialState,serialize,parseBackup} from '../src/model.js';
import {dualStats} from '../src/dual-wield.js';
import {projectDevice} from '../src/device-export.js';
import {inspectDeviceBackup} from '../src/device-backup.js';
const directory=mkdtempSync(join(tmpdir(),'pocket-dual-'));let cases=0;
try{
 const run=packet=>{writeFileSync(join(directory,'packet.json'),JSON.stringify(packet));const r=spawnSync('firmware/tests/out/ui-build/handling-probe',[join(directory,'packet.json')],{encoding:'utf8',maxBuffer:300000});assert.equal(r.status,0,r.stderr);return JSON.parse(r.stdout);};
 for(const str of [1,8,10,16,30])for(const dex of [1,8,16,30])for(const style of [false,true])for(const bonusSpent of [false,true]){
  const s=initialState();s.abilities.FOR=str;s.abilities.DES=dex;s.inventory[0].handling={flags:12,grip:0,versatileSides:0,ammoId:null};s.inventory.push({...s.inventory[0],id:'second',name:'Second',dieSides:4,damageModifier:2});s.dualWield={weaponId:'second',edition:1,style,bonusSpent};
  const packet=projectDevice(parseBackup(serialize(s))).packet,out=run(packet),w=dualStats(s);assert.equal(packet.version,24);assert.ok(out.dual.valid);assert.equal(out.dual.attack,w.attackBonus);assert.equal(out.dual.damage,w.damageBonus);assert.equal(out.dual.sides,4);
  const preview=inspectDeviceBackup(JSON.stringify(out.backup));assert.deepEqual(preview.dual,packet.dual);assert.equal(Buffer.from(out.backup.state,'base64').subarray(0,4).toString(),'PDDh');
  for(const [field,value] of [['weapon',61],['edition',3],['style',2],['bonusSpent',2]])assert.equal(run({...packet,dual:{...packet.dual,[field]:value}}),null);
  assert.equal(run({...packet,version:23}),null);cases++;
 }
 for(let score=1;score<=30;score++){
  const s=initialState();s.abilities.FOR=score;s.inventory[0].handling={flags:16,grip:0,versatileSides:0,ammoId:null};s.inventory[0].damageModifier=10;
  const packet=projectDevice(s).packet,out=run(packet),preview=inspectDeviceBackup(JSON.stringify(out.backup));assert.equal(out.graze,Math.max(0,Math.floor((score-10)/2)));assert.equal(preview.weaponHandling[0].flags,16);assert.equal(preview.dual.weapon,0);cases++;
 }
 console.log(`Dual wield: ${cases} PC/USB24/C/native backup comparisons, negative/positive bonuses, persisted turn state and malformed/no-downgrade rejection passed; synthetic.`);
}finally{rmSync(directory,{recursive:true,force:true});}
