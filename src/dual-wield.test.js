import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup,weaponStats} from './model.js';
import {dualStats,emptyDual,validateDual} from './dual-wield.js';
import {projectDevice} from './device-export.js';
import {removeItem,upsertItem} from './inventory.js';
import {beyondWeaponHandling} from './beyond-handling.js';
import {usePower} from './powers.js';
import {emptyExtended} from './extended.js';
const state=()=>{const s=initialState();s.inventory[0].handling={flags:12,grip:0,versatileSides:0,ammoId:null};s.inventory.push({...s.inventory[0],id:'secondary',name:'Secondaria',dieSides:4,damageModifier:2});s.dualWield={weaponId:'secondary',edition:1,style:false,bonusSpent:false};return s;};
test('Dual wield: explicit v27/USB24 metadata, no downgrade, legacy unchanged and secondary indices follow IDs',()=>{
 const s=state(),copy=structuredClone(s),raw=serialize(s);assert.equal(JSON.parse(raw).version,27);assert.deepEqual(parseBackup(raw).dualWield,s.dualWield);assert.deepEqual(s,copy);
 const p=projectDevice(s).packet;assert.equal(p.version,24);assert.deepEqual(p.dual,{weapon:4,edition:1,style:0,bonusSpent:0});
 assert.throws(()=>parseBackup(raw.replace('"version": 27','"version": 26')));assert.equal(JSON.parse(serialize(initialState())).version,11);
 const shuffled={...s,inventory:[s.inventory[3],...s.inventory.slice(0,3)]};assert.equal(projectDevice(shuffled).packet.dual.weapon,1);assert.equal(shuffled.equippedWeaponId,s.equippedWeaponId);
});
test('Dual wield: independent negative/positive modifiers, style, primary isolation and ranged edition guard',()=>{
 for(const str of [1,8,10,16,30])for(const dex of [1,8,10,16,30])for(const style of [false,true]){
  const s=state();s.abilities.FOR=str;s.abilities.DES=dex;s.dualWield.style=style;const w=dualStats(s),base=Math.floor((Math.max(str,dex)-10)/2);assert.equal(w.damageBonus,(style?base:Math.min(0,base))+2);assert.equal(w.dieSides,4);assert.equal(s.equippedWeaponId,'starter-sword');
 }
 const s=state();s.inventory[3].profile={ranged:true,heavy:false,thrownRange:0,longRange:10,properties:[]};assert.equal(dualStats(s),null);s.dualWield.edition=2;assert.ok(dualStats(s));
 s.inventory[3].quantity=0;assert.equal(dualStats(s),null);
});
test('Dual wield: deletion clears only the secondary link; malformed profiles and Light smuggling reject',()=>{
 const s=state(),next=removeItem(s,'secondary');assert.deepEqual(next.dualWield,emptyDual());assert.equal(s.dualWield.weaponId,'secondary');const changed=upsertItem(s,{id:'secondary',type:'gear',name:'Manuale',notes:'',quantity:1});assert.equal(parseBackup(serialize(changed)).dualWield.weaponId,null);
 for(const patch of [{weaponId:'missing'},{edition:3},{style:1},{bonusSpent:1},{extra:true}])assert.throws(()=>validateDual({...s.dualWield,...patch},s));
 const light=state();delete light.dualWield;assert.equal(projectDevice(light).packet.version,24);assert.throws(()=>parseBackup(serialize(light).replace('"version": 27','"version": 26')));
});
test('Beyond Light recognition is structured and identified; names, prose, homebrew and incompatible two hands do not grant it',()=>{
 const weapon=state().inventory[0],warnings=[];assert.equal(beyondWeaponHandling({properties:[{id:4,name:'Light'}]},weapon,m=>warnings.push(m)).flags,8);
 for(const d of [{name:'Light',properties:[]},{properties:[{name:'Light'}]},{isHomebrew:true,properties:[{id:4,name:'Light'}]},{properties:[{id:4,name:'Light'},{id:4,name:'Light'}]},{properties:[{id:4,name:'Light'},{id:5,name:'Two-Handed'}]}])assert.equal(beyondWeaponHandling(d,weapon,m=>warnings.push(m)),undefined);
 assert.equal(warnings.length,2);
});
test('Graze metadata: damage is ability only, zero for negative, no extra weapon damage and no older-format smuggling',()=>{
 for(const score of [1,8,10,16,30]){const s=initialState();s.abilities.FOR=score;s.inventory[0].handling={flags:16,grip:0,versatileSides:0,ammoId:null};s.inventory[0].damageModifier=10;assert.equal(weaponStats(s).grazeDamageOnMiss,Math.max(0,Math.floor((score-10)/2)));assert.equal(projectDevice(s).packet.version,24);assert.equal(JSON.parse(serialize(s)).version,27);assert.throws(()=>parseBackup(serialize(s).replace('"version": 27','"version": 26')));}
});
test('Known 2024 bonus spells atomically mark the bonus action, reject a second cast and preserve old state on failure',()=>{
 for(const effect of ['hex-2024','hunters-mark-2024']){const s=initialState();s.powers=[{id:'spell',type:'spell',name:'Spell',description:'',level:1,concentration:true,prepared:true,effect,usesSpellSlot:true}];s.slots[0]={current:2,max:2};const before=structuredClone(s),next=usePower(s,'spell',1);assert.equal(next.slots[0].current,1);assert.equal(next.dualWield.bonusSpent,true);assert.deepEqual(s,before);assert.throws(()=>usePower(next,'spell',1),/bonus/);}
});

test('Rage activation shares the bonus action, spends no use on rejection and remains active across own-turn recovery',()=>{
 const s=initialState();s.extended=emptyExtended();s.extended.combat.rageBonus=2;s.powers=[{id:'rage',type:'feature',name:'Ira',description:'',effect:'rage',maxUses:2,remaining:2,recovery:'long'}];s.dualWield={...emptyDual(),bonusSpent:true};const before=structuredClone(s);assert.throws(()=>usePower(s,'rage'),/bonus/);assert.deepEqual(s,before);s.dualWield.bonusSpent=false;const next=usePower(s,'rage');assert.equal(next.powers[0].remaining,1);assert.equal(next.dualWield.bonusSpent,true);assert.equal(next.extended.combat.raging,true);assert.deepEqual(parseBackup(serialize(next)).dualWield,next.dualWield);
});
