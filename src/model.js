import {validateExtended,adjustHeroHp} from './extended.js';
import {startingInventory,validateInventory,equippedWeapon,adjustQuantity,DIE_SIDES} from './inventory.js';
import {emptyEncounter,validateEncounter} from './encounter.js';
import {emptyMagic,validateMagic} from './powers.js';
import {avatarIdentity} from './avatar-species.js';
export const abilityKeys = ['FOR', 'DES', 'COS', 'INT', 'SAG', 'CAR'];
export const skills = [
  {id:'acrobatics',name:'Acrobazia',ability:'DES'}, {id:'animalHandling',name:'Addestrare animali',ability:'SAG'},
  {id:'arcana',name:'Arcano',ability:'INT'}, {id:'athletics',name:'Atletica',ability:'FOR'},
  {id:'deception',name:'Inganno',ability:'CAR'}, {id:'history',name:'Storia',ability:'INT'},
  {id:'insight',name:'Intuizione',ability:'SAG'}, {id:'intimidation',name:'Intimidire',ability:'CAR'},
  {id:'investigation',name:'Indagare',ability:'INT'}, {id:'medicine',name:'Medicina',ability:'SAG'},
  {id:'nature',name:'Natura',ability:'INT'}, {id:'perception',name:'Percezione',ability:'SAG'},
  {id:'performance',name:'Intrattenere',ability:'CAR'}, {id:'persuasion',name:'Persuasione',ability:'CAR'},
  {id:'religion',name:'Religione',ability:'INT'}, {id:'sleightOfHand',name:'Rapidità di mano',ability:'DES'},
  {id:'stealth',name:'Furtività',ability:'DES'}, {id:'survival',name:'Sopravvivenza',ability:'SAG'},
];
export const modifier = score => Math.floor((score - 10) / 2);
export const proficiency = level => 2 + Math.floor((level - 1) / 4);
export function weaponStats(state) {
  const weapon=equippedWeapon(state);if(!weapon)return null;
  const abilityBonus=modifier(state.abilities[weapon.ability]);
  const effects=state.extended?.combat, thrown=Boolean(effects?.thrown&&weapon.profile?.thrownRange);
  const melee=!(weapon.profile?.ranged||thrown);
  const powerAttack=Boolean(effects?.greatWeaponMaster&&effects.greatWeaponMasterAvailable&&melee&&weapon.profile?.heavy&&weapon.proficient);
  const rage=effects?.raging&&melee&&weapon.ability==='FOR'?(effects.rageBonus??0):0;
  const extraAttack=powerAttack?-5:0, extraDamage=rage+(powerAttack?10:0);
  return {...weapon,reach:thrown?weapon.profile.thrownRange:weapon.reach,melee,powerAttack,rage,extraMeleeCriticalDice:melee?(effects?.extraMeleeCriticalDice??0):0,attackBonus:abilityBonus+(weapon.proficient?proficiency(state.level):0)+weapon.attackModifier+extraAttack,
    damageBonus:(weapon.addAbilityToDamage?abilityBonus:0)+weapon.damageModifier+extraDamage};
}
export const attackBonus = state => weaponStats(state)?.attackBonus??0;
export const initialState = () => ({
  ...emptyMagic(),
  encounter:emptyEncounter(),
  ...startingInventory(),
  hp: 24, maxHp: 32, condition: false, bonus: 3, mode: 'normal',
  name: 'Elian', title: 'Il custode del bosco', level: 3, armorClass: 16,
  abilities: {FOR:14, DES:16, COS:14, INT:10, SAG:12, CAR:8},
  avatar: {skin:0, hair:0, cloak:0, style:0},
  skillRanks: Object.fromEntries(skills.map(skill=>[skill.id,0])),
  saveProficiencies: Object.fromEntries(abilityKeys.map(key=>[key,false])),
  history: [],
  resolve: 1,
});
export function changeHp(state,delta){return adjustHeroHp(state,delta)}
export function rollDie(sides, random = () => crypto.getRandomValues(new Uint32Array(1))[0]) {
  if (!Number.isInteger(sides) || sides < 2 || sides > 100) throw new Error('Dado non valido');
  const limit = Math.floor(4294967296 / sides) * sides;
  let n;
  do { n = random(); } while (n >= limit);
  return n % sides + 1;
}
export function rollD20(bonus, mode = 'normal', random) {
  const die = () => rollDie(20, random);
  const dice = mode === 'normal' ? [die()] : [die(), die()];
  const natural = mode === 'advantage' ? Math.max(...dice) : mode === 'disadvantage' ? Math.min(...dice) : dice[0];
  return {dice, natural, bonus, total: natural + bonus};
}
export function effectiveMode(state, kind) {
  if (!state.condition || !['check','skill','attack'].includes(kind)) return state.mode;
  return state.mode === 'advantage' ? 'normal' : 'disadvantage';
}
export function describeRoll(state, context = {kind:'free'}) {
  const {kind,key}=context;
  if(kind==='free')return {label:'Dado libero',bonus:state.bonus,parts:`Modificatore manuale`,mode:state.mode};
  if(!['check','save','skill'].includes(kind))throw new Error('Tipo di prova non valido');
  const skill=kind==='skill'?skills.find(skill=>skill.id===key):null;
  const ability=skill?.ability ?? key;
  if(!abilityKeys.includes(ability) || (kind==='skill'&&!skill))throw new Error('Prova non valida');
  const base=modifier(state.abilities[ability]);
  const rank=kind==='skill'?state.skillRanks[key]:kind==='save'?(state.saveProficiencies[key]?1:0):0;
  const trained=rank*proficiency(state.level)+(kind==='check'&&state.extended?.halfAbilityChecks?Math.floor(proficiency(state.level)/2):0)+(kind==='skill'&&rank===0&&state.extended?.halfSkills.includes(key)?Math.floor(proficiency(state.level)/2):0);
  const extra=kind==='skill'?(state.extended?.skillBonuses[key]??0):kind==='save'?(state.extended?.saveBonuses[key]??0):kind==='check'?(state.extended?.checkBonuses[key]??0):0;
  const blocked=state.extended?.blockedChecks.includes(`${kind}:${key}`)??false;
  const advantage=state.mode==='advantage'||Boolean(state.extended?.combat.raging&&ability==='FOR');
  const disadvantage=state.mode==='disadvantage'||Boolean(state.condition&&['check','skill'].includes(kind));
  const mode=advantage&&disadvantage?'normal':advantage?'advantage':disadvantage?'disadvantage':'normal';
  const sign=n=>n>=0?`+${n}`:`${n}`;
  return {label:kind==='skill'?skill.name:kind==='save'?`Salvezza ${key}`:`Prova ${key}`,bonus:base+trained+extra,blocked,
    parts:`${ability} ${sign(base)} · ${rank===2?'maestria':rank===1?'competenza':'non competente'} ${sign(trained)}${extra?` · extra ${sign(extra)}`:''}${blocked?' · da verificare':''}`,
    mode};
}
export function recordRoll(state, label, outcome, kind='d20', mode='normal', timestamp=Date.now()) {
  const entry={label,dice:[...outcome.dice],bonus:outcome.bonus,total:outcome.total,kind,mode,timestamp,...(kind==='damage'?{sides:outcome.sides??8}:{})};
  if(!validHistoryEntry(entry,state.extended?18:12,state.extended?60:30))throw new Error('Voce storico non valida');
  return {...state,history:[entry,...state.history].slice(0,20)};
}
function validHistoryEntry(entry,maxDamageDice=12,maxBonus=30) {
  if(!entry || !['d20','damage'].includes(entry.kind) || !['normal','advantage','disadvantage'].includes(entry.mode) ||
    typeof entry.label!=='string' || entry.label.length<1 || entry.label.length>80 ||
    !validInteger(entry.timestamp,0,8640000000000000) || !validInteger(entry.bonus,-15,maxBonus) || !Array.isArray(entry.dice) || entry.dice.length<1 || entry.dice.length>(entry.kind==='damage'?maxDamageDice:2) ||
    !entry.dice.every(die=>validInteger(die,1,entry.kind==='damage'?(entry.sides??8):20)))return false;
  if(entry.kind==='damage')return DIE_SIDES.includes(entry.sides??8)&&entry.mode==='normal'&&entry.total===Math.max(0,entry.dice.reduce((a,b)=>a+b,entry.bonus));
  if(entry.dice.length!==(entry.mode==='normal'?1:2))return false;
  const natural=entry.mode==='advantage'?Math.max(...entry.dice):entry.mode==='disadvantage'?Math.min(...entry.dice):entry.dice[0];
  return entry.total===natural+entry.bonus;
}
export function attack(state, armorClass, random) {
  const weapon=weaponStats(state);if(!weapon)throw new Error('Equipaggia un’arma disponibile');
  if (!Number.isInteger(armorClass) || armorClass < 1 || armorClass > 30) throw new Error('CA non valida');
  const mode = effectiveMode(state,'attack');
  const hit = rollD20(weapon.attackBonus, mode, random);
  const critical = hit.natural === 20;
  return {...hit, weapon, mode, critical, hits: hit.natural !== 1 && (critical || hit.total >= armorClass)};
}
export function damage(critical = false, random, bonus = 2, count = 1, sides = 8) {
  if(!validInteger(count,1,6)||!DIE_SIDES.includes(sides)||!validInteger(bonus,-15,60))throw new Error('Danni non validi');
  const dice = Array.from({length: count*(critical?2:1)}, () => rollDie(sides, random));
  return {dice, bonus, sides, total: Math.max(0, dice.reduce((a, b) => a + b, bonus))};
}
export function weaponDamage(combat,random) {
  if(!combat?.hits||!combat.weapon)throw new Error('Nessun attacco riuscito');
  const weapon=combat.weapon;
  const base=damage(combat.critical,random,weapon.damageBonus,weapon.diceCount,weapon.dieSides);
  if(combat.critical&&weapon.extraMeleeCriticalDice){
    const extra=Array.from({length:weapon.extraMeleeCriticalDice},()=>rollDie(weapon.dieSides,random));
    return {...base,dice:[...base.dice,...extra],total:Math.max(0,base.dice.concat(extra).reduce((sum,d)=>sum+d,base.bonus))};
  }
  return base;
}
export function useItem(state,id,random) {
  const item=state.inventory.find(entry=>entry.id===id);
  if(!item||!['consumable','healing'].includes(item.type)||item.quantity===0)throw new Error('Oggetto non utilizzabile');
  if(item.type==='healing'&&state.hp===0&&state.extended?.deathSaves.failures===3)throw new Error('Tre fallimenti: correzione DM prima di curare');
  if(item.type==='healing'&&state.hp===state.maxHp)return {state,healing:null,gained:0};
  const healing=item.type==='healing'?Array.from({length:item.diceCount},()=>rollDie(item.dieSides,random)).reduce((a,b)=>a+b,item.healingModifier):null;
  const next=adjustQuantity(state,id,-1);
  const healed=healing===null?next:changeHp(next,healing);
  return {state:healed,healing,gained:healed.hp-state.hp};
}
function validInteger(value, min, max) {
  return Number.isInteger(value) && value >= min && value <= max;
}
export function parseBackup(raw) {
  const s = JSON.parse(raw);
  if (!s || ![1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13].includes(s.version)) throw new Error('Versione backup non supportata');
  if(s.version<10&&s.extended!==undefined)throw new Error('I dati estesi richiedono il formato v10');
  const base = initialState();
  if (s.version >= 3) {
    if (typeof s.name !== 'string' || !s.name.trim() || s.name.length > 24 || typeof s.title !== 'string' || s.title.length > 40 || !validInteger(s.level, 1, 20) || !validInteger(s.maxHp, 1, 999) || !validInteger(s.armorClass, 1, 30)) throw new Error('Profilo non valido');
    if (!s.abilities || !abilityKeys.every(key => validInteger(s.abilities[key], 1, 30))) throw new Error('Caratteristiche non valide');
    if (!s.avatar || !['skin','hair','cloak','style'].every(key => validInteger(s.avatar[key], 0, 2))) throw new Error('Avatar non valido');
    const identity=avatarIdentity(s.avatar);
    if((s.version<12&&(identity.lineage||identity.body))||(s.version>=12&&(s.avatar.lineage===undefined||s.avatar.body===undefined))||(s.version===12&&identity.lineage>=12))throw new Error('La silhouette richiede il formato v12');
    Object.assign(base, {name:s.name.trim(), title:s.title.trim(), level:s.level, maxHp:s.maxHp, armorClass:s.armorClass,
      abilities:Object.fromEntries(abilityKeys.map(key=>[key,s.abilities[key]])),
      avatar:{...Object.fromEntries(['skin','hair','cloak','style'].map(key=>[key,s.avatar[key]])),...(s.version>=12?identity:{})}});
  }
  if (s.version >= 4) {
    if(!s.skillRanks || !skills.every(skill=>validInteger(s.skillRanks[skill.id],0,2)) || !s.saveProficiencies || !abilityKeys.every(key=>typeof s.saveProficiencies[key]==='boolean'))throw new Error('Competenze non valide');
    if(!Array.isArray(s.history) || s.history.length>20 || !s.history.every(entry=>validHistoryEntry(entry,s.version>=10?18:12,s.version>=10?60:30)))throw new Error('Storico non valido');
    Object.assign(base,{
      skillRanks:Object.fromEntries(skills.map(skill=>[skill.id,s.skillRanks[skill.id]])),
      saveProficiencies:Object.fromEntries(abilityKeys.map(key=>[key,s.saveProficiencies[key]])),
      history:s.history.map(({label,dice,bonus,total,kind,mode,timestamp,sides})=>({label,dice:[...dice],bonus,total,kind,mode,timestamp,...(kind==='damage'?{sides:sides??8}:{})}))
    });
  }
  if(s.version>=5)Object.assign(base,validateMagic(s,s.version>=10));
  if(s.version>=6)base.encounter=validateEncounter(s.encounter,s.version>=7);
  if(s.version===8&&!validInteger(s.weaponReach,1,9))throw new Error('Portata arma non valida');
  if (!validInteger(s.hp, 0, base.maxHp) || !validInteger(s.bonus, -5, 15) || typeof s.condition !== 'boolean' || !['normal', 'advantage', 'disadvantage'].includes(s.mode)) throw new Error('Scheda non valida');
  const core = {hp: s.hp, bonus: s.bonus, condition: s.condition, mode: s.mode};
  if (s.version === 1) return {...base, ...core};
  if(!validInteger(s.resolve,0,1))throw new Error('Risorsa non valida');
  if(s.version>=9)Object.assign(base,validateInventory(s,s.version>=10,s.version>=11));
  else {
    if(typeof s.swordEquipped!=='boolean'||!validInteger(s.potions,0,99)||!validInteger(s.torches,0,99))throw new Error('Inventario precedente non valido');
    Object.assign(base,startingInventory({...s,weaponReach:s.version===8?s.weaponReach:1}));
  }
  if(s.version===10||(s.version>=11&&s.extended!==undefined))base.extended=validateExtended(s.extended);
  if(s.importInfo!==undefined){
    const info=s.importInfo;
    if(!info || info.provider!=='dndbeyond' || typeof info.characterId!=='string' || !/^[1-9]\d{0,14}$/.test(info.characterId) || !['core-v1',...(s.version>=10?['extended-v1']:[])].includes(info.scope) || !Array.isArray(info.limitations) || info.limitations.length>30 || info.limitations.some(text=>typeof text!=='string'||text.length>500))throw new Error('Provenienza importazione non valida');
    base.importInfo={provider:info.provider,characterId:info.characterId,scope:info.scope,limitations:[...info.limitations]};
  }
  return {...base,...core,resolve:s.resolve};
}
export function serialize(state) {
  const identity=avatarIdentity(state.avatar),variant=identity.lineage!==0||identity.body!==0;
  return JSON.stringify({...state,...(variant?{avatar:{...state.avatar,...identity}}:{}),version:identity.lineage>=12?13:variant?12:11},null,2);
}
export function restore(raw) {
  try { return parseBackup(raw); } catch { return initialState(); }
}
