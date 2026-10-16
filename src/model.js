import {emptyEncounter,validateEncounter} from './encounter.js';
import {emptyMagic,validateMagic} from './powers.js';
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
export const attackBonus = state => modifier(state.abilities.FOR) + proficiency(state.level);
export const initialState = () => ({
  ...emptyMagic(),
  encounter:emptyEncounter(),
  weaponReach:1,
  hp: 24, maxHp: 32, condition: false, bonus: 3, mode: 'normal',
  name: 'Elian', title: 'Il custode del bosco', level: 3, armorClass: 16,
  abilities: {FOR:14, DES:16, COS:14, INT:10, SAG:12, CAR:8},
  avatar: {skin:0, hair:0, cloak:0, style:0},
  skillRanks: Object.fromEntries(skills.map(skill=>[skill.id,0])),
  saveProficiencies: Object.fromEntries(abilityKeys.map(key=>[key,false])),
  history: [],
  swordEquipped: true, potions: 2, torches: 3, resolve: 1,
});
export function changeHp(state, delta) {
  const hp=Math.max(0, Math.min(state.maxHp, state.hp + delta));
  return {...state,hp,concentration:hp===0?null:state.concentration};
}
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
  const trained=rank*proficiency(state.level);
  const sign=n=>n>=0?`+${n}`:`${n}`;
  return {label:kind==='skill'?skill.name:kind==='save'?`Salvezza ${key}`:`Prova ${key}`,bonus:base+trained,
    parts:`${ability} ${sign(base)} · ${rank===2?'maestria':rank===1?'competenza':'non competente'} ${sign(trained)}`,
    mode:effectiveMode(state,kind)};
}
export function recordRoll(state, label, outcome, kind='d20', mode='normal', timestamp=Date.now()) {
  const entry={label,dice:[...outcome.dice],bonus:outcome.bonus,total:outcome.total,kind,mode,timestamp};
  if(!validHistoryEntry(entry))throw new Error('Voce storico non valida');
  return {...state,history:[entry,...state.history].slice(0,20)};
}
function validHistoryEntry(entry) {
  if(!entry || !['d20','damage'].includes(entry.kind) || !['normal','advantage','disadvantage'].includes(entry.mode) ||
    typeof entry.label!=='string' || entry.label.length<1 || entry.label.length>80 ||
    !validInteger(entry.timestamp,0,8640000000000000) || !validInteger(entry.bonus,-5,30) || !Array.isArray(entry.dice) || entry.dice.length<1 || entry.dice.length>2 ||
    !entry.dice.every(die=>validInteger(die,1,entry.kind==='damage'?8:20)))return false;
  if(entry.kind==='damage')return entry.mode==='normal'&&entry.total===Math.max(0,entry.dice.reduce((a,b)=>a+b,entry.bonus));
  if(entry.dice.length!==(entry.mode==='normal'?1:2))return false;
  const natural=entry.mode==='advantage'?Math.max(...entry.dice):entry.mode==='disadvantage'?Math.min(...entry.dice):entry.dice[0];
  return entry.total===natural+entry.bonus;
}
export function attack(state, armorClass, random) {
  if (!state.swordEquipped) throw new Error('Equipaggia la spada');
  if (!Number.isInteger(armorClass) || armorClass < 1 || armorClass > 30) throw new Error('CA non valida');
  const mode = effectiveMode(state,'attack');
  const hit = rollD20(attackBonus(state), mode, random);
  const critical = hit.natural === 20;
  return {...hit, mode, critical, hits: hit.natural !== 1 && (critical || hit.total >= armorClass)};
}
export function damage(critical = false, random, bonus = 2) {
  const dice = Array.from({length: critical ? 2 : 1}, () => rollDie(8, random));
  return {dice, bonus, total: Math.max(0, dice.reduce((a, b) => a + b, bonus))};
}
export function drinkPotion(state, random) {
  if (state.potions <= 0 || state.hp === state.maxHp) return {state, healing: null};
  const healing = rollDie(4, random) + rollDie(4, random) + 2;
  return {state: {...changeHp(state, healing), potions: state.potions - 1}, healing};
}
function validInteger(value, min, max) {
  return Number.isInteger(value) && value >= min && value <= max;
}
export function parseBackup(raw) {
  const s = JSON.parse(raw);
  if (!s || ![1, 2, 3, 4, 5, 6, 7, 8].includes(s.version)) throw new Error('Versione backup non supportata');
  const base = initialState();
  if (s.version >= 3) {
    if (typeof s.name !== 'string' || !s.name.trim() || s.name.length > 24 || typeof s.title !== 'string' || s.title.length > 40 || !validInteger(s.level, 1, 20) || !validInteger(s.maxHp, 1, 999) || !validInteger(s.armorClass, 1, 30)) throw new Error('Profilo non valido');
    if (!s.abilities || !abilityKeys.every(key => validInteger(s.abilities[key], 1, 30))) throw new Error('Caratteristiche non valide');
    if (!s.avatar || !['skin','hair','cloak','style'].every(key => validInteger(s.avatar[key], 0, 2))) throw new Error('Avatar non valido');
    Object.assign(base, {name:s.name.trim(), title:s.title.trim(), level:s.level, maxHp:s.maxHp, armorClass:s.armorClass,
      abilities:Object.fromEntries(abilityKeys.map(key=>[key,s.abilities[key]])),
      avatar:Object.fromEntries(['skin','hair','cloak','style'].map(key=>[key,s.avatar[key]]))});
  }
  if (s.version >= 4) {
    if(!s.skillRanks || !skills.every(skill=>validInteger(s.skillRanks[skill.id],0,2)) || !s.saveProficiencies || !abilityKeys.every(key=>typeof s.saveProficiencies[key]==='boolean'))throw new Error('Competenze non valide');
    if(!Array.isArray(s.history) || s.history.length>20 || !s.history.every(validHistoryEntry))throw new Error('Storico non valido');
    Object.assign(base,{
      skillRanks:Object.fromEntries(skills.map(skill=>[skill.id,s.skillRanks[skill.id]])),
      saveProficiencies:Object.fromEntries(abilityKeys.map(key=>[key,s.saveProficiencies[key]])),
      history:s.history.map(({label,dice,bonus,total,kind,mode,timestamp})=>({label,dice:[...dice],bonus,total,kind,mode,timestamp}))
    });
  }
  if(s.version>=5)Object.assign(base,validateMagic(s));
  if(s.version>=6)base.encounter=validateEncounter(s.encounter,s.version>=7);
  if(s.version>=8){if(!validInteger(s.weaponReach,1,9))throw new Error('Portata arma non valida');base.weaponReach=s.weaponReach}
  if (!validInteger(s.hp, 0, base.maxHp) || !validInteger(s.bonus, -5, 15) || typeof s.condition !== 'boolean' || !['normal', 'advantage', 'disadvantage'].includes(s.mode)) throw new Error('Scheda non valida');
  const core = {hp: s.hp, bonus: s.bonus, condition: s.condition, mode: s.mode};
  if (s.version === 1) return {...base, ...core};
  if (typeof s.swordEquipped !== 'boolean' || !validInteger(s.potions, 0, 99) || !validInteger(s.torches, 0, 99) || !validInteger(s.resolve, 0, 1)) throw new Error('Inventario non valido');
  // Selezione esplicita: nessun campo esterno viene inserito nel DOM.
  return {...base, ...core, swordEquipped: s.swordEquipped, potions: s.potions, torches: s.torches, resolve: s.resolve};
}
export function serialize(state) { return JSON.stringify({...state, version: 8}, null, 2); }
export function restore(raw) {
  try { return parseBackup(raw); } catch { return initialState(); }
}
