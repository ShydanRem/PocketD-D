import {test} from 'node:test';
import assert from 'node:assert/strict';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {beyondText} from './beyond-details.js';
import {serialize,parseBackup,initialState,describeRoll,weaponStats,attack,weaponDamage,changeHp,recordRoll} from './model.js';
import {usePower,rest,spellStats,upsertPower,availableSlots} from './powers.js';
import {setCombatOption} from './extended.js';
import {upsertParticipant,adjustParticipantHp,queueDamage,applyPendingDamage} from './encounter.js';
import {createLibrary,addCharacter,serializeLibrary,parseLibrary} from './library.js';
const report=(p=beyondRichFixture())=>convertBeyond(p,'123456');
const state=()=>report().draft;
const power=(s,name)=>s.powers.find(p=>p.name===name);

test('Importazione estesa: tiri scelti, salvezze, stato e sorgente immutabile',()=>{
  const p=beyondRichFixture(),before=structuredClone(p),r=report(p),s=r.draft;
  assert.ok(r.canImport);assert.equal(r.version,3);assert.equal(r.conversion.complete,false);
  assert.equal(s.skillRanks.perception,1);assert.equal(describeRoll(s,{kind:'skill',key:'perception'}).bonus,2);
  assert.equal(describeRoll(s,{kind:'save',key:'FOR'}).bonus,4);assert.equal(describeRoll(s,{kind:'save',key:'COS'}).bonus,5);
  assert.equal(s.extended.tempHp,4);assert.equal(s.condition,true);assert.equal(s.extended.hitDice[0].used,1);assert.equal(s.extended.currencies.gp,12);assert.equal(s.extended.conditions[1].level,1);
  assert.deepEqual(p,before);
});
test('Importazione estesa: competenza, maestria, mezza competenza e bonus additivi',()=>{
  const p=beyondRichFixture();p.data.modifiers.feat.push({type:'expertise',subType:'perception'},{type:'half-proficiency',subType:'ability-checks'},{type:'bonus',subType:'perception',value:2},{type:'bonus',subType:'saving-throws',value:1});
  const s=report(p).draft;
  assert.equal(describeRoll(s,{kind:'skill',key:'perception'}).bonus,6);
  assert.equal(describeRoll(s,{kind:'skill',key:'arcana'}).bonus,1);
  assert.equal(describeRoll(s,{kind:'check',key:'FOR'}).bonus,3);
  assert.equal(describeRoll(s,{kind:'save',key:'COS'}).bonus,6);
});
test('Importazione estesa: scelte discordanti e formule di tiro ignote non producono tiri dichiarati corretti',()=>{
  const p=beyondRichFixture();p.data.choices.choiceDefinitions[0].options[0].label='Arcana';p.data.modifiers.feat.push({type:'set',subType:'history',value:9});
  const s=report(p).draft;assert.ok(describeRoll(s,{kind:'skill',key:'perception'}).blocked);assert.ok(describeRoll(s,{kind:'skill',key:'history'}).blocked);
});
test('Armi: dati distinti, proprietà, dadi, competenza, gittata e modalità lancio',()=>{
  let s=state();assert.equal(s.inventory.filter(i=>i.type==='weapon').length,3);assert.equal(s.equippedWeaponId,null);
  s={...s,equippedWeaponId:s.inventory[0].id};assert.equal(weaponStats(s).attackBonus,4);assert.equal(weaponStats(s).damageBonus,2);assert.equal(weaponStats(s).reach,1);
  s=setCombatOption(s,'thrown',true);assert.equal(weaponStats(s).reach,6);assert.equal(weaponStats(s).melee,false);
  s={...s,equippedWeaponId:s.inventory[2].id};assert.equal(weaponStats(s).reach,16);assert.equal(weaponStats(s).attackBonus,3);
});
test('Armi: finesse sceglie DES migliore, bonus magico richiede sintonia e dati non standard restano oggetti',()=>{
  const p=beyondRichFixture();p.data.stats[1].value=18;
  const w=p.data.inventory[0];w.definition.properties=[{name:'Finesse'}];w.definition.canAttune=true;w.definition.grantedModifiers=[{type:'bonus',subType:'magic',value:1}];w.isAttuned=true;
  let s=report(p).draft;assert.equal(s.inventory[0].ability,'DES');assert.equal(weaponStats({...s,equippedWeaponId:s.inventory[0].id}).attackBonus,7);
  w.isAttuned=false;s=report(p).draft;assert.equal(weaponStats({...s,equippedWeaponId:s.inventory[0].id}).attackBonus,6);
  p.data.inventory[2].definition.damage.diceValue=3;s=report(p).draft;assert.equal(s.inventory[2].type,'gear');
});
test('Capacità: Ira consuma utilizzi e applica solo bonus pertinenti, GWM è una scelta esplicita',()=>{
  let s=state();s={...s,condition:false,equippedWeaponId:s.inventory[1].id};s=usePower(s,power(s,'Rage').id);
  assert.equal(power(s,'Rage').remaining,1);assert.equal(weaponStats(s).damageBonus,4);assert.equal(weaponStats(s).attackBonus,4);
  assert.equal(describeRoll(s,{kind:'save',key:'FOR'}).mode,'advantage');
  assert.throws(()=>usePower(s,power(s,'Rage').id));
  s=setCombatOption(s,'greatWeaponMaster',true);assert.equal(weaponStats(s).attackBonus,-1);assert.equal(weaponStats(s).damageBonus,14);
  const hit=attack(s,1,()=>19);const damage=weaponDamage(hit,()=>0);assert.equal(damage.dice.length,3);assert.equal(damage.total,17);
  assert.equal(weaponDamage(attack({...s,equippedWeaponId:s.inventory[2].id},1,()=>19),()=>0).dice.length,2);
});
test('Ira: due sorgenti di vantaggio non annullano due volte uno svantaggio',()=>{
  let s=state();s=usePower(s,power(s,'Rage').id);s.mode='advantage';
  assert.equal(describeRoll(s,{kind:'skill',key:'athletics'}).mode,'normal');
  s.condition=false;s.mode='disadvantage';assert.equal(describeRoll(s,{kind:'save',key:'FOR'}).mode,'normal');
});
test('Capacità: Resistenza Implacabile solo a zero PF, note passive senza falsi utilizzi',()=>{
  let s=state();const id=power(s,'Relentless Endurance').id;assert.throws(()=>usePower(s,id));
  s={...s,hp:0};s=usePower(s,id);assert.equal(s.hp,1);assert.equal(power(s,'Relentless Endurance').remaining,0);assert.equal(s.extended.deathSaves.failures,0);
  assert.throws(()=>usePower(s,power(s,'Savage Attacks').id));
});
test('Magie: caratteristica per incantatore, preparazione e contatori innati senza consumo slot',()=>{
  let s=state();const a=power(s,'Luce di prova'),b=power(s,'Scintilla innata');
  assert.deepEqual(spellStats(s,a),{ability:'INT',attack:2,dc:10});assert.deepEqual(spellStats(s,b),{ability:'CAR',attack:1,dc:9});
  assert.throws(()=>usePower(s,power(s,'Eco di prova').id,1));
  const slots=structuredClone(s.slots);s=usePower(s,b.id);assert.equal(power(s,b.name).limited.remaining,0);assert.deepEqual(s.slots,slots);assert.throws(()=>usePower(s,b.id));
});
test('Magie: slot ordinari e patto separati, riposo breve recupera solo patto e risorse previste',()=>{
  let s=state();const a=power(s,'Luce di prova');
  assert.ok(availableSlots(s,a).some(s=>s.value==='pact:2'));
  s=usePower(s,a.id,'pact:2');assert.equal(s.extended.pactSlots[1].current,0);assert.equal(s.slots[0].current,2);assert.equal(s.concentration,a.id);
  assert.throws(()=>usePower(s,a.id,'pact:2'));s=usePower(s,a.id,'1');assert.equal(s.slots[0].current,1);
  s=rest(s,'short');assert.equal(s.extended.pactSlots[1].current,2);assert.equal(s.slots[0].current,1);assert.equal(power(s,'Rage').remaining,2);
  s=rest(s,'long');assert.equal(s.slots[0].current,3);assert.equal(power(s,'Rage').remaining,3);assert.equal(s.extended.tempHp,0);
});
test('Magie: Ira interrompe concentrazione e impedisce lancio; fine Ira non ricrea slot o usi',()=>{
  let s=state();s=usePower(s,power(s,'Luce di prova').id,1);s=usePower(s,power(s,'Rage').id);assert.equal(s.concentration,null);
  assert.throws(()=>usePower(s,power(s,'Luce di prova').id,1));const uses=power(s,'Rage').remaining;
  s=setCombatOption(s,'raging',false);assert.equal(power(s,'Rage').remaining,uses);assert.equal(s.slots[0].current,1);
});
test('Magie: contatori e caratteristica non risolti restano note, senza falsi lanci disponibili',()=>{
  const p=beyondRichFixture();p.data.spells.race[0].limitedUse.maxUses=-1;p.data.classes[0].definition.spellCastingAbilityId=null;
  const r=report(p);assert.equal(r.draft.powers.filter(p=>p.type==='spell').length,0);assert.ok(r.conversion.omissions.some(t=>t.includes('non risolta')));
});
test('PF temporanei: assorbimento coerente tra eroe e combattimento, danni applicati una volta',()=>{
  let s=state();const hp=s.hp;assert.equal(changeHp(s,-3).hp,hp);assert.equal(changeHp(s,-3).extended.tempHp,1);
  s=upsertParticipant(s,{id:'hero',initiative:1});s=applyPendingDamage(queueDamage(s,'hero',6,'hit'));
  assert.equal(s.hp,hp-2);assert.equal(s.extended.tempHp,0);assert.throws(()=>applyPendingDamage(s));
  assert.equal(adjustParticipantHp(upsertParticipant(state(),{id:'hero',initiative:1}),'hero',-1).hp,hp);
});
test('Schema v11: backup e raccolta conservano dati estesi; v9 resta compatibile e schede precedenti identiche',()=>{
  const s=state(),raw=serialize(s);assert.equal(JSON.parse(raw).version,11);assert.deepEqual(parseBackup(raw),s);
  assert.throws(()=>parseBackup(JSON.stringify({...JSON.parse(raw),version:9})));
  const old=createLibrary(),before=serializeLibrary(old),next=addCharacter(old,s,'rich');assert.equal(serializeLibrary(old),before);
  const restored=parseLibrary(serializeLibrary(next));assert.deepEqual(restored.characters[0],old.characters[0]);assert.deepEqual(restored.characters[1].state,s);assert.equal(JSON.parse(serialize(initialState())).version,11);
  for(const extended of [{...s.extended,tempHp:-1},{...s.extended,pactSlots:[]},{...s.extended,deathSaves:{successes:4,failures:0,stabilized:false}}])assert.throws(()=>parseBackup(serialize({...s,extended})));
});
test('Testi importati: HTML rimosso, entità decodificate, nessuna interpretazione di codice',()=>{
  assert.equal(beyondText('<script>alert(1)</script><p>Prova &amp; &#x1F31F;</p>'),'Prova & 🌟');
  const p=beyondRichFixture();p.data.classSpells[0].spells[0].definition.description='<p>&lt;img src=x onerror=alert(1)&gt;</p>';
  assert.equal(power(report(p).draft,'Luce di prova').description,'<img src=x onerror=alert(1)>');
});

test('Portata importata: griglia e modalità lancio usano la stessa gittata, anche oltre nove caselle',async()=>{
  const {placeToken,targetGeometry}=await import('./grid.js');
  let s=state();s.equippedWeaponId=s.inventory[0].id;
  s=upsertParticipant(s,{id:'hero',initiative:1});s=upsertParticipant(s,{id:'target',name:'Bersaglio',initiative:0,hp:10,maxHp:10,armorClass:10,side:'enemy'});
  s=placeToken(placeToken(s,'hero',0),'target',8);
  assert.equal(targetGeometry(s,'target').inRange,false);
  s=setCombatOption(s,'thrown',true);assert.equal(targetGeometry(s,'target').inRange,false);
  s.equippedWeaponId=s.inventory[2].id;assert.equal(targetGeometry(s,'target').inRange,true);
  s.equippedWeaponId=s.inventory[0].id;s=placeToken(s,'target',5);assert.equal(targetGeometry(s,'target').inRange,true);
});
test('Critici estesi: fino a diciotto dadi e bonus danni elevati rimangono serializzabili senza cambiare v9',()=>{
  let s=state();s.condition=false;s.inventory[1]={...s.inventory[1],diceCount:6,damageModifier:10};s.abilities.FOR=30;s.equippedWeaponId=s.inventory[1].id;
  s.extended.combat={...s.extended.combat,raging:true,rageBonus:10,greatWeaponMaster:true,extraMeleeCriticalDice:6};
  const result=weaponDamage(attack(s,1,()=>19),()=>0);assert.equal(result.bonus,40);assert.equal(result.dice.length,18);
  s=recordRoll(s,'Critico esteso',result,'damage');assert.deepEqual(parseBackup(serialize(s)).history,s.history);
});

test('Risorse con costo variabile non diventano capacità da un utilizzo arbitrario',()=>{
  const p=beyondRichFixture();p.data.actions.class[0].limitedUse.minNumberConsumed=2;
  const r=report(p),s=r.draft;assert.equal(power(s,'Rage').passive,true);assert.equal(s.extended.combat.rageBonus,0);
  assert.ok(r.conversion.omissions.some(t=>t.includes('costo variabile')));
});
