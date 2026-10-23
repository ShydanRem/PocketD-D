import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {inspectDeviceBackup,crc32} from './device-backup.js';
import {parseBackup} from './model.js';
const raw=readFileSync(new URL('../firmware/tests/fixtures/device-backup-v1.synthetic.json',import.meta.url),'utf8');
function edit(key,change){const packet=JSON.parse(raw),bytes=Uint8Array.from(atob(packet[key]),c=>c.charCodeAt(0));change(bytes,new DataView(bytes.buffer));new DataView(bytes.buffer).setUint32(bytes.length-4,crc32(bytes.subarray(0,-4)),true);packet[key]=btoa(String.fromCharCode(...bytes));return JSON.stringify(packet)}
test('actual C snapshot: native effects, money, preferences and all twenty rolls preview exactly',()=>{
 const p=inspectDeviceBackup(raw);assert.equal(p.name,'Elian');assert.equal(p.hp,17);assert.equal(p.maxHp,42);assert.equal(p.baseHp,32);assert.equal(p.armor,18);assert.equal(p.baseArmor,16);assert.equal(p.tempHp,7);assert.equal(p.effects,2);assert.equal(p.rolls.length,20);assert.equal(p.rolls[0].number,20);assert.deepEqual(p.rolls[0].dice,[20]);assert.equal(p.rolls[19].total,1);assert.equal(p.wallet.pp,999999999);assert.deepEqual(p.keys,[13,3,5]);assert.equal(p.brightness,80);assert.equal(p.nativeValidation,false);
});
test('preview preserves the complete original records and never turns into a PC character',()=>{
 const p=inspectDeviceBackup(raw);assert.deepEqual(p.packet,JSON.parse(raw));assert.equal(p.packet.format,'pocketdd-device-backup');assert.equal(p.packet.version,1);assert(!('inventory' in p.packet));assert.throws(()=>parseBackup(raw));p.packet.state='';assert.equal(inspectDeviceBackup(raw).name,'Elian');
});
test('wrong formats, future versions, duplicate keys and unknown fields reject',()=>{
 const p=JSON.parse(raw);for(const version of [0,2,true,'1',1.5])assert.throws(()=>inspectDeviceBackup(JSON.stringify({...p,version})));
 assert.throws(()=>inspectDeviceBackup(raw.replace('"version":1','"version":1.0')));assert.throws(()=>inspectDeviceBackup(raw.replace('"version":1','"version":1,"version":1')));assert.throws(()=>inspectDeviceBackup(JSON.stringify({...p,extra:0})));assert.throws(()=>inspectDeviceBackup(raw+'null'));assert.throws(()=>inspectDeviceBackup(raw.replace('"state"','"sta\\u0074e"')));
});
test('all truncations and corrupt encoded data reject without a partial preview',()=>{
 for(let i=0;i<64;i++)assert.throws(()=>inspectDeviceBackup(raw.slice(0,i<32?i:raw.length-(i-31))));
 const p=JSON.parse(raw);p.state='!'+p.state.slice(1);assert.throws(()=>inspectDeviceBackup(JSON.stringify(p)));p.state=JSON.parse(raw).state.replace(/^./,c=>c==='A'?'B':'A');assert.throws(()=>inspectDeviceBackup(JSON.stringify(p)));
 assert.throws(()=>inspectDeviceBackup(' '.repeat(110001)));
});
test('canonical base64 padding and bounded native headers are required',()=>{
 const p=JSON.parse(raw),alphabet='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';const pos=p.state.length-2;assert.equal(p.state.at(-1),'=');const index=alphabet.indexOf(p.state[pos]);p.state=p.state.slice(0,pos)+alphabet[index+1]+'=';assert.throws(()=>inspectDeviceBackup(JSON.stringify(p)));
 assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[3]=82)));assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[5123]=121)));assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[5123]=120)));
});
test('even with valid CRC, mismatching epochs and malformed roll math reject',()=>{
 assert.throws(()=>inspectDeviceBackup(edit('history',(b,v)=>v.setUint32(8,1,true))));
 for(const offset of [16,21,22,23,24,25,28,122])assert.throws(()=>inspectDeviceBackup(edit('history',b=>b[offset]=255)));
});
test('selected character values and known versus unknown money remain explicit',()=>{
 for(const at of [63,64,71,435,7913])assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[at]=255)));
 assert.throws(()=>inspectDeviceBackup(edit('state',(b,v)=>v.setUint32(b.length-28,1000000000,true))));
 assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[b.length-29]=0)));
 const unknown=inspectDeviceBackup(edit('state',b=>b.fill(0,b.length-29,b.length-8)));assert.equal(unknown.wallet.available,false);assert.equal(unknown.wallet.gp,0);
});
test('reordered fields and outside whitespace are accepted by the native envelope',()=>{
 const p=JSON.parse(raw);const reordered=JSON.stringify({history:p.history,state:p.state,version:1,format:p.format},null,2);assert.equal(inspectDeviceBackup('\n'+reordered+'\n').rolls.length,20);
});

test('PDD26 preserves packed turn subjects; PDD25 remains supported and invalid selectors reject',()=>{
 const base=inspectDeviceBackup(raw);assert.deepEqual(base.turnSubjects,[0,0,0]);
 for(let owner=1;owner<=3;owner++)for(const tick of [1,2]){
  const q=edit('state',b=>{b[3]=81;b[8375]=(owner<<3)|tick});const preview=inspectDeviceBackup(q);
  assert.equal(preview.turnSubjects[owner-1],1);assert.deepEqual(preview.packet,JSON.parse(q));
 }
 for(const packed of [5,7,8,11,12,33,255])assert.throws(()=>inspectDeviceBackup(edit('state',b=>{b[3]=81;b[8375]=packed})));
 assert.throws(()=>inspectDeviceBackup(edit('state',b=>b[8375]=9)));
});
