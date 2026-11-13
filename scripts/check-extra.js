// Real PC projection/C parser/native codec; every character here is synthetic.
import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {mkdtempSync,writeFileSync,rmSync} from 'node:fs';
import {tmpdir} from 'node:os';
import {join} from 'node:path';
import {initialState,parseBackup,serialize} from '../src/model.js';
import {expandDual,dualStats,extraReady} from '../src/dual-wield.js';
import {projectDevice} from '../src/device-export.js';
import {inspectDeviceBackup} from '../src/device-backup.js';
const directory=mkdtempSync(join(tmpdir(),'pocket-extra-'));let cases=0;
try{
 const run=packet=>{writeFileSync(join(directory,'packet.json'),JSON.stringify(packet));const r=spawnSync('firmware/tests/out/ui-build/handling-probe',[join(directory,'packet.json')],{encoding:'utf8',maxBuffer:300000});assert.equal(r.status,0,r.stderr);return JSON.parse(r.stdout);};
 for(const score of [8,16])for(const style of [false,true])for(const light of [false,true])for(let budget=0;budget<16;budget++){
  const s=initialState();s.abilities.FOR=s.abilities.DES=score;s.inventory[0].handling={flags:12,grip:0,versatileSides:0,ammoId:null};s.inventory.push({...s.inventory[0],id:'other',name:'Other',handling:{flags:36+(light?8:0),grip:0,versatileSides:0,ammoId:null}});s.dualWield={...expandDual(),weaponId:'other',edition:2,feat:true,style,bonusSpent:!!(budget&1),lightSpent:!!(budget&2),featSpent:!!(budget&4),cleaveSpent:!!(budget&8)};
  const packet=projectDevice(parseBackup(serialize(s))).packet,out=run(packet);assert.equal(packet.version,25);assert.ok(out);
  for(let m=0;m<3;m++){const w=dualStats(s,m);assert.equal(out.methods[m].valid,!!w);assert.equal(out.methods[m].ready,extraReady(s,m));if(w){assert.equal(out.methods[m].attack,w.attackBonus);assert.equal(out.methods[m].damage,w.damageBonus);}}
  assert.deepEqual(inspectDeviceBackup(JSON.stringify(out.backup)).dual,packet.dual);assert.equal(Buffer.from(out.backup.state,'base64').subarray(0,4).toString(),'PDDi');
  for(const field of ['feat','lightSpent','featSpent','cleaveSpent'])assert.equal(run({...packet,dual:{...packet.dual,[field]:2}}),null);assert.equal(run({...packet,version:24}),null);cases++;
 }
 console.log(`Extra attacks: ${cases} PC/USB25/C/native backup parity states, three methods, all budget subsets, negative modifiers and malformed/no-downgrade rejection passed; synthetic.`);
}finally{rmSync(directory,{recursive:true,force:true});}
