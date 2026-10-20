// Synthetic PC -> USB -> native C parity; never accesses a personal collection.
import {spawnSync} from 'node:child_process';
import {writeFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {initialState,abilityKeys,weaponStats,attack,weaponDamage,useItem} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
const states=[];
for(let n=0;n<140;n++){
  const s=initialState();s.level=1+n%20;s.extended=emptyExtended();s.extended.tempHp=n%10;s.hp=n%9===0?s.maxHp:5;
  abilityKeys.forEach((key,i)=>s.abilities[key]=1+(n*7+i*3)%30);s.condition=n%3===0;
  const c=s.extended.combat;c.raging=n%2===0;c.rageBonus=2+n%9;c.extraMeleeCriticalDice=n%7;c.greatWeaponMasterAvailable=true;c.greatWeaponMaster=n%4<2;
  const base=s.inventory[0];base.ability=abilityKeys[n%6];base.diceCount=1+n%6;base.dieSides=[4,6,8,10,12][n%5];base.attackModifier=n%21-10;base.damageModifier=10-n%21;
  base.addAbilityToDamage=n%3!==0;base.proficient=n%4!==0;
  base.profile={ranged:n%3===0,heavy:n%2===0,thrownRange:n%3===1?4:0,longRange:12,properties:['Proprieta descrittiva da verificare']};
  c.thrown=Boolean(base.profile.thrownRange&&n%2);
  s.inventory.push({...structuredClone(base),id:'alternate',name:'Altra arma',profile:{ranged:!base.profile.ranged,heavy:true,thrownRange:0,longRange:12,properties:[]}});
  s.inventory[1].diceCount=1+n%6;s.inventory[1].healingModifier=n%31;s.inventory[1].quantity=n%4;
  s.inventory[2].quantity=n%4;
  if(n%11===0){s.inventory[0].quantity=0;s.equippedWeaponId=null;c.thrown=false}
  if(n===139)while(s.inventory.length<60)s.inventory.push({id:`gear-${s.inventory.length}`,name:'"Oggetto\\ da quaranta caratteri lungo abc'.slice(0,40),notes:'Descrizione '.repeat(20),type:'gear',quantity:999});
  states.push(s);
}
const input='firmware/tests/out/combat-input.jsonl';
writeFileSync(input,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/combat-probe',[input],{encoding:'utf8',maxBuffer:4*1024*1024,timeout:20000});
assert.equal(run.status,0,run.stderr);const lines=run.stdout.trim().split('\n');assert.equal(lines.length,states.length);
let comparisons=0;
for(let n=0;n<states.length;n++){
 const s=states[n],row=JSON.parse(lines[n]);assert.equal(row.items.length,s.inventory.length);
 row.items.forEach((v,i)=>{
   const item=s.inventory[i];if(item.type!=='weapon'||!item.quantity){assert.equal(v,null);return}
   const selected={...s,equippedWeaponId:item.id,extended:structuredClone(s.extended)};
   if(item.id!==s.equippedWeaponId)selected.extended.combat.thrown=false;
   const w=weaponStats(selected);
   assert.deepEqual(v,{attack:w.attackBonus,damage:w.damageBonus,reach:w.reach,criticalDice:w.extraMeleeCriticalDice,melee:Number(w.melee)},`weapon ${n}:${i}`);comparisons++;
 });
 row.attacks.forEach((v,i)=>{
   if(!s.equippedWeaponId){assert.equal(v,null);return}
   const mode=['disadvantage','normal','advantage'][Math.floor(i/3)],face=[0,19,9][i%3];
   const hit=attack({...s,mode},15,()=>face);const damage=hit.hits?weaponDamage(hit,()=>0):null;
   assert.deepEqual(v,[hit.dice.length,hit.total,Number(hit.hits),Number(hit.critical),hit.natural,damage?.dice.length??0,damage?.total??0],`attack ${n}:${i}`);comparisons++;
 });
 row.uses.forEach((v,i)=>{
   const item=s.inventory[i];if(!['healing','consumable'].includes(item.type)||!item.quantity||(item.type==='healing'&&s.hp===s.maxHp)){assert.equal(v,null);return}
   const used=useItem(s,item.id,()=>0),count=item.type==='healing'?item.diceCount:0;
   assert.deepEqual(v,[used.state.hp,s.extended.tempHp,item.quantity-1,used.gained??0,count,used.healing??0,Number(used.state.extended.combat.thrown)],`use ${n}:${i}`);comparisons++;
 });
}
console.log(`PC -> USB -> C inventory/combat: ${comparisons} comparisons, 140 synthetic states, 60-item boundary, attacks/critical/healing/effects passed.`);
