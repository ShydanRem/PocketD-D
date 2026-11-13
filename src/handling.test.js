import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup,weaponStats} from './model.js';
import {emptyExtended} from './extended.js';
import {emptyGear,equipGear} from './equipment.js';
import {emptyHandling,validateHandling} from './handling.js';
import {removeItem,equipWeapon,upsertItem} from './inventory.js';
import {projectDevice} from './device-export.js';
const state=()=>{const s=initialState();s.inventory[0].handling={flags:6,grip:1,versatileSides:10,ammoId:s.inventory[2].id};return s;};
test('Handling: v26/USB23 preserve explicit per-weapon choices and reject silent downgrade',()=>{
 const s=state(),before=structuredClone(s),raw=serialize(s);assert.equal(JSON.parse(raw).version,26);assert.deepEqual(parseBackup(raw).inventory,s.inventory);assert.deepEqual(s,before);
 const p=projectDevice(s).packet;assert.equal(p.version,23);assert.deepEqual(p.handling,[{flags:6,grip:1,sides:10,ammo:3},{flags:0,grip:0,sides:0,ammo:0},{flags:0,grip:0,sides:0,ammo:0}]);assert.ok(p.magic.powers.every(p=>p.formula));
 assert.throws(()=>parseBackup(raw.replace('"version": 26','"version": 25')));const legacy=initialState();assert.equal(JSON.parse(serialize(legacy)).version,11);assert.equal(projectDevice(legacy).packet.version,9);assert.ok(!('handling' in projectDevice(legacy).packet));
});
test('Handling: finesse chooses current physical ability, rage follows STR, versatile follows grip and throwing',()=>{
 const s=state();s.extended=emptyExtended();s.extended.combat.raging=true;s.extended.combat.rageBonus=2;
 let w=weaponStats(s);assert.equal(w.ability,'DES');assert.equal(w.dieSides,10);assert.equal(w.damageBonus,3);
 s.abilities.FOR=18;w=weaponStats(s);assert.equal(w.ability,'FOR');assert.equal(w.damageBonus,6);
 s.inventory[0].profile={ranged:false,heavy:false,thrownRange:4,longRange:12,properties:[]};s.extended.combat.thrown=true;w=weaponStats(s);assert.equal(w.dieSides,8);assert.equal(w.damageBonus,4);assert.equal(w.melee,false);
 s.inventory[0].handling.grip=0;s.extended.combat.thrown=false;assert.equal(weaponStats(s).dieSides,8);
});
test('Handling: shield conflicts block attacks and both directions of equipment changes without mutation',()=>{
 const s=state();s.extended={...emptyExtended(),loadout:{unarmored:1,items:[{...emptyGear(s.inventory[1].id),kind:5,armor:2,equipped:false}]}};s.inventory[1].type='gear';delete s.inventory[1].diceCount;delete s.inventory[1].dieSides;delete s.inventory[1].healingModifier;
 const before=structuredClone(s);assert.throws(()=>equipGear(s,s.inventory[1].id,true),/due mani/);assert.deepEqual(s,before);
 s.extended.loadout.items[0].equipped=true;assert.equal(weaponStats(s),null);assert.throws(()=>equipWeapon(s,s.inventory[0].id),/scudo/);s.inventory[0].handling.grip=0;assert.ok(weaponStats(s));
});
test('Handling: invalid metadata, missing/self bindings and incompatible dice are explicit errors',()=>{
 const s=state(),item=s.inventory[0];for(const patch of [{flags:8},{flags:3},{grip:2},{versatileSides:7},{versatileSides:8},{ammoId:item.id},{ammoId:4},{extra:true}])assert.throws(()=>validateHandling({...item.handling,...patch},item));
 const missing=state();missing.inventory[0].handling.ammoId='missing';assert.throws(()=>parseBackup(serialize(missing)),/assente/);
 const bow={...item,profile:{ranged:true},handling:{...item.handling}};assert.throws(()=>validateHandling(bow.handling,bow));
 assert.deepEqual(validateHandling(emptyHandling(),item),emptyHandling());
});
test('Handling: confirmed source deletion clears references by ID; reorder remaps USB indices',()=>{
 const s=state(),n=removeItem(s,s.inventory[2].id);assert.equal(n.inventory[0].handling.ammoId,null);assert.equal(s.inventory[0].handling.ammoId,s.inventory[2].id);
 const shuffled={...s,inventory:[s.inventory[2],s.inventory[0],s.inventory[1]]};assert.equal(projectDevice(shuffled).packet.handling[1].ammo,1);
 const edit={...s.inventory[0],name:'Edited'};assert.deepEqual(upsertItem(s,edit).inventory[0].handling,s.inventory[0].handling);
});

test('Beyond handling: structured identified properties enable supported rules, descriptions and malformed variants do not',async()=>{
 const {beyondWeaponHandling}=await import('./beyond-handling.js');const item=initialState().inventory[0],warnings=[];
 assert.deepEqual(beyondWeaponHandling({properties:[{id:1,name:'Versatile',notes:'1d10'},{id:2,name:'Finesse'}]},item,s=>warnings.push(s)),{flags:6,grip:0,versatileSides:10,ammoId:null});
 assert.equal(beyondWeaponHandling({name:'Two-Handed',properties:[]},item,()=>{}),undefined);
 for(const props of [[{id:1,name:'Versatile',notes:'adds 1d10'}],[{id:1,name:'Two-Handed'},{id:2,name:'Versatile',notes:'1d10'}],[{id:1,name:'Finesse'},{id:1,name:'Finesse'}]])assert.equal(beyondWeaponHandling({properties:props},item,s=>warnings.push(s)),undefined);
 assert.equal(beyondWeaponHandling({isHomebrew:true,properties:[{id:1,name:'Two-Handed'}]},item,()=>{}),undefined);assert.equal(warnings.length,3);
});
test('Beyond handling: full conversion exports native rules without auto-selecting a grip or ammunition',async()=>{
 const {beyondRichFixture}=await import('./beyond-fixtures.js');const {convertBeyondFull}=await import('./beyond-conversion.js');const p=beyondRichFixture();p.data.inventory[1].definition.properties=[{id:1,name:'Heavy'},{id:2,name:'Two-Handed'}];const before=structuredClone(p),r=convertBeyondFull(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.inventory[1].handling.flags,1);assert.equal(r.draft.inventory[1].handling.ammoId,null);assert.equal(projectDevice(r.draft).packet.version,23);assert.deepEqual(p,before);
});

test('Handling: returning from thrown mode cannot bypass hand checks when equipping a weapon or shield',()=>{
 const s=state();s.inventory[0].profile={ranged:false,heavy:false,thrownRange:4,longRange:12,properties:[]};s.extended={...emptyExtended(),loadout:{unarmored:1,items:[{...emptyGear(s.inventory[1].id),kind:5,armor:2,equipped:false}]}};s.extended.combat.thrown=true;s.inventory[1].type='gear';
 const before=structuredClone(s);assert.throws(()=>equipGear(s,s.inventory[1].id,true),/due mani/);assert.deepEqual(s,before);s.extended.loadout.items[0].equipped=true;assert.throws(()=>equipWeapon(s,s.inventory[0].id),/scudo/);assert.equal(s.extended.combat.thrown,true);
});
test('Handling: an imported hand conflict remains reviewable without presenting a usable attack',async()=>{
 const {importReview}=await import('./import-review.js');const s=state();s.extended={...emptyExtended(),loadout:{unarmored:1,items:[{...emptyGear(s.inventory[1].id),kind:5,armor:2,equipped:true}]}};s.inventory[1].type='gear';const r=importReview(s);assert.equal(r.weapon.blocked,true);assert.equal(r.weapon.attack,null);assert.equal(r.transfer.available,true);
});
