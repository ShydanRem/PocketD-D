// Synthetic PC -> USB22 -> C -> native/backup parity. Never touches USB/accounts.
import assert from 'node:assert/strict';
import {spawnSync} from 'node:child_process';
import {readFileSync,writeFileSync} from 'node:fs';
import {projectDevice} from '../src/device-export.js';
import {formulaState,formulaDefinition} from '../src/spell-formula-fixtures.js';
import {emptySpellFormula,resolveSpellFormula} from '../src/spell-formula.js';
import {convertBeyondFull} from '../src/beyond-conversion.js';
import {beyondRichFixture} from '../src/beyond-fixtures.js';
import {inspectDeviceBackup,crc32} from '../src/device-backup.js';
const cases=[],expected=[];
const fields=['kind','count','sides','ability','scaling','step','source','bonus'];
for(let level=1;level<=20;level++)for(let kind=1;kind<=2;kind++)for(let scaling=0;scaling<=2;scaling++){
  const s=formulaState();s.level=level;s.abilities.CAR=level%2?3:20;const p=s.powers[0];p.level=scaling===2?0:1;p.usesSpellSlot=p.level>0;p.formula={kind,count:scaling===0?18:1,sides:level%2?4:20,ability:1,scaling,step:scaling?2:0,source:level%2,bonus:level%3-1};
  if(level===20){for(let i=1;i<120;i++)s.powers.push({...structuredClone(p),id:'formula-'+i});}
  const packet=projectDevice(s).packet,formulas=s.powers.map(p=>fields.map(k=>p.formula[k])),resolved=[];
  s.powers.forEach((p,i)=>{for(let slot=p.level;slot<=(p.level?9:0);slot++){try{const f=resolveSpellFormula(s,p,slot);resolved.push([i,slot,f.kind,f.count,f.sides,f.bonus]);}catch{resolved.push([i,slot,null]);}}});
  cases.push(packet);expected.push({valid:true,formulas,resolved});
}
/* Real source identities with invented names, sheet and counters. */
for(const id of [2619079,2618890,2618967]){
  const payload=beyondRichFixture();payload.data.classSpells[0].spells[0].definition=formulaDefinition(id);const r=convertBeyondFull(payload,'123456');assert.equal(r.canImport,true);const p=r.draft.powers.find(p=>p.formula);assert(p&&p.formula.source===1);
  const packet=projectDevice(r.draft).packet;assert.equal(packet.version,22);const formulas=r.draft.powers.map(p=>fields.map(k=>(p.formula??emptySpellFormula())[k])),resolved=[];
  r.draft.powers.forEach((p,i)=>{if(!p.formula)return;for(let slot=p.level;slot<=(p.level?9:0);slot++){const f=resolveSpellFormula(r.draft,p,slot);resolved.push([i,slot,f.kind,f.count,f.sides,f.bonus]);}});cases.push(packet);expected.push({valid:true,formulas,resolved});
}
const valid=cases.length;
for(const patch of [{kind:3},{count:0},{sides:7},{ability:2},{scaling:3},{step:1},{source:2},{bonus:61},{bonus:-16},{bonus:null},{bonus:'1'},{bonus:true},{extra:1}]){const p=structuredClone(cases[0]);Object.assign(p.magic.powers[0].formula,patch);cases.push(p);expected.push({valid:false});}
for(const mode of ['legacy','missing','feature','effect']){const p=structuredClone(cases[0]);if(mode==='legacy')p.version=21;if(mode==='missing')delete p.magic.powers[0].formula;if(mode==='feature')p.magic.powers[0].kind=1;if(mode==='effect')p.magic.powers[0].effect=9;cases.push(p);expected.push({valid:false});}
const packetPath='firmware/tests/out/formula-packets.synthetic.ndjson';
writeFileSync(packetPath,cases.map(p=>JSON.stringify(p)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/spell-formula-probe',[packetPath,'firmware/tests/out/formula-backup.synthetic.json'],{encoding:'utf8',timeout:30000,maxBuffer:8*1024*1024});assert.equal(run.status,0,run.error?.message??run.stderr);const rows=run.stdout.trim().split('\n').map(v=>JSON.parse(v));assert.deepEqual(rows,expected);
const raw=readFileSync('firmware/tests/out/formula-backup.synthetic.json','utf8'),view=inspectDeviceBackup(raw);assert.equal(view.spellFormulas.length,1);assert.deepEqual(fields.map(k=>view.spellFormulas[0][k]),expected[0].formulas[0]);assert.deepEqual(JSON.parse(raw),view.packet);
const envelope=JSON.parse(raw),bytes=Buffer.from(envelope.state,'base64');assert.equal(bytes.subarray(0,4).toString(),'PDDf');
for(let field=0;field<8;field++){
  const corrupt=Buffer.from(bytes),start=bytes.length-4-8;corrupt[start+field]=[3,0,7,2,3,1,2,255][field];new DataView(corrupt.buffer,corrupt.byteOffset,corrupt.byteLength).setUint32(corrupt.length-4,crc32(corrupt.subarray(0,-4)),true);const bad={...envelope,state:corrupt.toString('base64')};assert.throws(()=>inspectDeviceBackup(JSON.stringify(bad)));
}
console.log(`Spell formulas: ${valid} PC/USB22/C formula + scaling cases, max120 rows, ${cases.length-valid} malformed rejections, exact native backup/profile preview and CRC-valid corrupt fields passed; synthetic.`);
