// Synthetic inputs only. The actual C parser, core codec and native backup run here.
import {spawnSync} from 'node:child_process';
import {writeFileSync,readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {initialState} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {emptyGear,equipGear,attuneGear} from '../src/equipment.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
import {inspectDeviceBackup,crc32} from '../src/device-backup.js';
const states=[];
for(let i=0;i<80;i++){
 let s=initialState();s.extended=emptyExtended();s.level=1+i%20;s.hp=1+i%20;s.maxHp=120;s.abilities={FOR:14,DES:10+i%10,COS:14,INT:12,SAG:16,CAR:18};s.armorClass=16;s.inventory.push({id:'armor',type:'gear',name:'Synthetic armor',notes:'',quantity:1},{id:'shield',type:'gear',name:'Synthetic shield',notes:'',quantity:1},{id:'gear',type:'gear',name:'Synthetic gear',notes:'',quantity:1});
 s.extended.loadout={unarmored:i%4,items:[{...emptyGear('armor'),kind:2+i%3,armor:16,equipped:true},{...emptyGear('shield'),kind:5,armor:2},{...emptyGear('gear'),ability:[i%3-1,2,2,0,0,2],ac:i%3-1,attack:1,damage:-1,spellAttack:2,spellDc:1,saves:1,checks:2,maxHp:i%9-4}]};
 s.powers=[{id:'spell',name:'Synthetic spell',type:'spell',description:'',level:1,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:true},{id:'ts',name:'Synthetic weapon cantrip',type:'spell',description:'',effect:'true-strike-2024',level:0,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:false}];
 if(i%2){const g=s.extended.loadout.items.at(-1);g.attunement=i%3===0?2:1;g.equipped=i%3!==0;if(i%4===1)s=attuneGear(s,g.id,true);}
 if(i%4===1){s.powers.push({...s.powers[0],id:'rider',name:'Anonymous effect',effect:i%8===1?'hex-2024':'hunters-mark-2024',concentration:true});s.concentration='rider';}
 states.push(s);
}
const packets=states.map(s=>projectDevice(s).packet),bad=[];
for(const mutate of [p=>p.version=17,p=>delete p.equipment,p=>p.equipment.items.pop(),p=>p.equipment.unarmored=4,p=>p.equipment.items[3].maxHp=-1000,p=>p.equipment.items[3].ability[0]=11,p=>p.equipment.items[3].equipped='true',p=>p.equipment.items[3].unknown=1,p=>p.equipment.items[0].kind=1,p=>p.magic.powers[1].flags|=4,p=>p.magic.powers[1].level=1]){const p=structuredClone(packets[0]);mutate(p);bad.push(p);}
for(const mutate of [p=>p.version=18,p=>delete p.equipment.items[3].attunement,p=>p.equipment.items[3].attunement=3,p=>p.equipment.items[0].attunement=1,p=>{p.equipment.items[3].attunement=2;p.inventory.items[3].quantity=0;p.equipment.items[3].equipped=false;}]){const p=structuredClone(packets[1]);mutate(p);bad.push(p);}
for(const mutate of [p=>p.version=19,p=>p.magic.powers.at(-1).effect=9,p=>p.magic.powers.at(-1).level=0,p=>p.magic.powers.at(-1).flags&=~1]){const p=structuredClone(packets[1]);mutate(p);bad.push(p);}
const input='firmware/tests/out/loadout-input.synthetic.jsonl',backup='firmware/tests/out/loadout-backup.synthetic.json';writeFileSync(input,[...packets.map(serializeDevice),...bad.map(JSON.stringify)].join('\n')+'\n');
const run=spawnSync('firmware/tests/out/loadout-probe',[input,backup],{encoding:'utf8',timeout:20000,maxBuffer:4*1024*1024});assert.equal(run.status,0,run.stderr);const rows=run.stdout.trim().split('\n').map(JSON.parse);assert.equal(rows.length,80+bad.length);
const fields=['FOR','DES','COS','INT','SAG','CAR'];let comparisons=0;
const snapshot=s=>({armor:s.armorClass,hp:s.hp,maxHp:s.maxHp,abilities:fields.map(k=>s.abilities[k]),weapons:s.inventory.map(i=>[i.attackModifier??0,i.damageModifier??0]),powers:s.powers.map(p=>[p.attackModifier??0,p.saveModifier??0]),checks:fields.map(k=>s.extended.checkBonuses[k]),saves:fields.map(k=>s.extended.saveBonuses[k]),skills:Object.values(s.extended.skillBonuses),equipped:s.inventory.map(i=>Number(s.extended.loadout.items.find(g=>g.id===i.id)?.equipped??false)),attunement:s.inventory.map(i=>s.extended.loadout.items.find(g=>g.id===i.id)?.attunement??0)});
for(let i=0;i<states.length;i++){assert.ok(rows[i].valid);const s=states[i],expected=[s,...s.extended.loadout.items.map(g=>{try{return equipGear(s,g.id,!g.equipped)}catch{return null}}),...s.extended.loadout.items.filter(g=>g.attunement).map(g=>{try{return attuneGear(s,g.id,g.attunement!==2)}catch{return null}})].map(s=>s?snapshot(s):null);assert.deepEqual(rows[i].rows,expected,`native parity ${i}`);comparisons+=expected.length;}
for(const row of rows.slice(states.length))assert.equal(row.valid,false);
const raw=readFileSync(backup,'utf8'),view=inspectDeviceBackup(raw);assert.equal(view.packet.state,JSON.parse(raw).state);assert.equal(view.name,states[0].name);assert.equal(view.armor,states[1].armorClass);assert.equal(view.loadout.items.length,3);assert.equal(view.loadout.unarmored,states[1].extended.loadout.unarmored);assert.equal(view.loadout.items.at(-1).attunement,2);assert.equal(view.loadout.items.at(-1).equipped,true);
const b=JSON.parse(raw),bytes=Uint8Array.from(Buffer.from(b.state,'base64'));assert.equal(String.fromCharCode(...bytes.slice(0,4)),'PDDd');bytes[bytes.length-4-1081]=4;new DataView(bytes.buffer).setUint32(bytes.length-4,crc32(bytes.subarray(0,-4)),true);b.state=Buffer.from(bytes).toString('base64');assert.throws(()=>inspectDeviceBackup(JSON.stringify(b)));
console.log(`Loadout: ${comparisons} JS/USB18-20/C/codec snapshots, ${bad.length} invalid packet rejections, PDDd backup exact original bytes and corrupt extension rejection passed; synthetic only.`);
