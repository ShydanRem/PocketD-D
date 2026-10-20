import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,changeHp,useItem} from './model.js';
import {emptyExtended} from './extended.js';
import {deathFace,deathChange,deathReady,conditionChange} from './status.js';
import {rest,usePower} from './powers.js';
import {projectDevice} from './device-export.js';
const hero=()=>({...initialState(),hp:0,extended:emptyExtended()});
test('base death save outcomes: 1/20, threshold ten, stabilization and terminal guards',()=>{
 const s=hero(),before=structuredClone(s);s.extended.deathSaves={successes:2,failures:2,stabilized:false};
 const stable=deathFace(s,10);assert.deepEqual(stable.extended.deathSaves,{successes:0,failures:0,stabilized:true});assert(!deathReady(stable));
 const dead=deathFace(s,1);assert.equal(dead.extended.deathSaves.failures,3);assert(!deathReady(dead));assert.throws(()=>deathFace(dead,20));
 const revived=deathFace(s,20);assert.equal(revived.hp,1);assert.deepEqual(revived.extended.deathSaves,before.extended.deathSaves);
 assert.equal(deathFace(s,9).extended.deathSaves.failures,3);assert.equal(s.extended.deathSaves.failures,2);
 for(const face of [0,21,1.5])assert.throws(()=>deathFace(s,face));
});
test('three failures cannot be bypassed by ordinary healing, rest or endurance; explicit correction is available',()=>{
 const s=hero();s.extended.deathSaves.failures=3;const before=structuredClone(s);let calls=0;
 assert.deepEqual(changeHp(s,1),s);assert.throws(()=>useItem(s,'starter-potion',()=>{calls++;return 0}));assert.equal(calls,0);assert.throws(()=>rest(s,'long'));
 s.powers=[{id:'endurance',name:'Resistenza',description:'',type:'feature',effect:'relentless-endurance',maxUses:1,remaining:1,recovery:'long'}];assert.throws(()=>usePower(s,'endurance'));
 const corrected=deathChange(s,6);assert.equal(corrected.extended.deathSaves.failures,2);assert.equal(changeHp(corrected,1).hp,1);assert.equal(before.extended.deathSaves.failures,3);
});
test('HP transitions clear old counters; damage at zero remains an explicit DM correction',()=>{
 const s=hero();s.extended.deathSaves={successes:1,failures:2,stabilized:false};s.extended.tempHp=2;
 assert.deepEqual(changeHp(s,-1).extended.deathSaves,s.extended.deathSaves);
 const healed=changeHp(s,1);assert.equal(healed.hp,1);assert.deepEqual(healed.extended.deathSaves,{successes:0,failures:0,stabilized:false});
 healed.extended.deathSaves.failures=2;healed.extended.tempHp=0;assert.equal(changeHp(healed,-1).extended.deathSaves.failures,0);
 const stable=deathChange(s,1);assert.equal(deathChange(stable,4).extended.deathSaves.failures,2);assert(!deathChange(stable,4).extended.deathSaves.stabilized);
});
test('all thirty condition notes transfer exactly with levels; poison flag is independent',()=>{
 const s=hero();s.extended.conditions=Array.from({length:30},(_,i)=>({name:'W'.repeat(60),level:i%7}));s.condition=true;
 const before=structuredClone(s),{packet}=projectDevice(s);assert.equal(packet.status.conditions.length,30);assert.deepEqual(packet.status.conditions,s.extended.conditions);assert(packet.training.poisoned);
 const changed=conditionChange(s,29,99);assert.equal(changed.extended.conditions[29].level,6);assert(changed.condition);assert.deepEqual(s,before);
 assert.equal(projectDevice(initialState()).packet.status.available,false);
});
