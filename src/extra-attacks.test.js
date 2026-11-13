import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup,weaponStats} from './model.js';
import {expandDual,dualStats,extraReady} from './dual-wield.js';
import {projectDevice} from './device-export.js';
import {removeItem} from './inventory.js';
import {validateHandling} from './handling.js';
const state=()=>{const s=initialState();s.inventory[0].handling={flags:12,grip:0,versatileSides:0,ammoId:null};s.inventory.push({...s.inventory[0],id:'other',name:'Other',dieSides:4,handling:{flags:44,grip:0,versatileSides:0,ammoId:null}});s.dualWield={...expandDual(),weaponId:'other',edition:2,feat:true};return s;};
test('Extra attacks use explicit v28/USB25 and reject older-schema smuggling while legacy formats stay unchanged',()=>{
 const s=state(),before=structuredClone(s),raw=serialize(s);assert.equal(JSON.parse(raw).version,28);assert.deepEqual(parseBackup(raw).dualWield,s.dualWield);assert.deepEqual(s,before);assert.throws(()=>parseBackup(raw.replace('"version": 28','"version": 27')));
 const p=projectDevice(s).packet;assert.equal(p.version,25);assert.deepEqual(p.dual,{weapon:4,edition:2,style:0,bonusSpent:0,feat:1,lightSpent:0,featSpent:0,cleaveSpent:0});assert.equal(projectDevice(initialState()).packet.version,9);
});
test('Nick and feat have independent budgets; normal Light shares Nick budget and bonus action',()=>{
 const s=state();assert.deepEqual([0,1,2].map(m=>extraReady(s,m)),[true,true,true]);s.dualWield.bonusSpent=true;assert.deepEqual([0,1,2].map(m=>extraReady(s,m)),[false,true,false]);s.dualWield.lightSpent=true;assert.deepEqual([0,1,2].map(m=>extraReady(s,m)),[false,false,false]);s.dualWield.bonusSpent=false;assert.deepEqual([0,1,2].map(m=>extraReady(s,m)),[false,false,true]);s.dualWield.featSpent=true;assert.equal(extraReady(s,2),false);
});
test('Dual Wielder 2024 permits a non-Light melee secondary; Nick and Light do not inherit that permission',()=>{
 const s=state();s.inventory[3].handling.flags=4;assert.equal(dualStats(s,0),null);assert.equal(dualStats(s,1),null);assert.ok(dualStats(s,2));s.inventory[3].profile={ranged:true,heavy:false,thrownRange:0,longRange:10,properties:[]};assert.equal(dualStats(s,2),null);s.inventory[3].profile.ranged=false;s.dualWield.edition=1;assert.throws(()=>parseBackup(serialize(s)),/2024/);
});
test('One mastery per weapon: Nick/Cleave/other masteries never accidentally activate Graze',()=>{
 const s=state();for(let m=0;m<9;m++){s.inventory[0].handling.flags=m<<4;validateHandling(s.inventory[0].handling,s.inventory[0]);const w=weaponStats(s);assert.equal('grazeDamageOnMiss' in w,m===1);assert.equal(JSON.parse(serialize(s)).version,28);}s.inventory[0].handling.flags=144;assert.throws(()=>validateHandling(s.inventory[0].handling,s.inventory[0]));
});
test('Malformed counters, unsupported feat editions and missing secondary reject; deleting the link preserves spent budgets',()=>{
 const s=state();for(const patch of [{feat:1},{lightSpent:1},{featSpent:1},{cleaveSpent:1},{edition:1},{weaponId:null}])assert.throws(()=>parseBackup(serialize({...s,dualWield:{...s.dualWield,...patch}})));
 s.dualWield.lightSpent=s.dualWield.featSpent=s.dualWield.cleaveSpent=true;const next=removeItem(s,'other');assert.equal(next.dualWield.weaponId,null);assert.equal(next.dualWield.feat,false);assert.ok(next.dualWield.lightSpent&&next.dualWield.featSpent&&next.dualWield.cleaveSpent);assert.deepEqual(parseBackup(serialize(next)).dualWield,next.dualWield);
});
