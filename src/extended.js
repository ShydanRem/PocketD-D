const integer=(n,min,max)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
const keys=['FOR','DES','COS','INT','SAG','CAR'];
export const skillIds=['acrobatics','animalHandling','arcana','athletics','deception','history','insight','intimidation','investigation','medicine','nature','perception','performance','persuasion','religion','sleightOfHand','stealth','survival'];
export const emptyExtended=()=>({tempHp:0,conditions:[],deathSaves:{successes:0,failures:0,stabilized:false},hitDice:[],currenciesAvailable:false,currencies:{cp:0,sp:0,ep:0,gp:0,pp:0},pactSlots:Array.from({length:9},()=>({max:0,current:0})),skillBonuses:Object.fromEntries(skillIds.map(k=>[k,0])),saveBonuses:Object.fromEntries(keys.map(k=>[k,0])),checkBonuses:Object.fromEntries(keys.map(k=>[k,0])),halfAbilityChecks:false,halfSkills:[],blockedChecks:[],combat:{rageBonus:0,raging:false,greatWeaponMaster:false,greatWeaponMasterAvailable:false,extraMeleeCriticalDice:0,thrown:false},facts:[]});
export function validateExtended(input){
  const fail=()=>{throw new Error('Dati estesi non validi')};
  if(!input||!integer(input.tempHp,0,9999))fail();
  if(!Array.isArray(input.conditions)||input.conditions.length>30)fail();
  const conditions=input.conditions.map(c=>{if(!c||typeof c.name!=='string'||!c.name.trim()||c.name.length>60||!integer(c.level,0,6))fail();return {name:c.name,level:c.level}});
  const d=input.deathSaves;if(!d||!integer(d.successes,0,3)||!integer(d.failures,0,3)||typeof d.stabilized!=='boolean')fail();
  if(!Array.isArray(input.hitDice)||input.hitDice.length>20)fail();
  const hitDice=input.hitDice.map(h=>{if(!h||typeof h.name!=='string'||h.name.length>80||![6,8,10,12].includes(h.die)||!integer(h.max,1,20)||!integer(h.used,0,h.max))fail();return {name:h.name,die:h.die,max:h.max,used:h.used}});
  const currencies=Object.fromEntries(['cp','sp','ep','gp','pp'].map(k=>{if(!integer(input.currencies?.[k],0,999999999))fail();return[k,input.currencies[k]]}));
  if(input.currenciesAvailable!==undefined&&typeof input.currenciesAvailable!=='boolean')fail();
  const currenciesAvailable=input.currenciesAvailable??Object.values(currencies).some(n=>n!==0);
  if(!Array.isArray(input.pactSlots)||input.pactSlots.length!==9)fail();
  const pactSlots=input.pactSlots.map(s=>{if(!s||!integer(s.max,0,9)||!integer(s.current,0,s.max))fail();return{max:s.max,current:s.current}});
  const map=(source,ids)=>Object.fromEntries(ids.map(k=>{if(!integer(source?.[k],-30,30))fail();return[k,source[k]]}));
  if(typeof input.halfAbilityChecks!=='boolean')fail();
  if(!Array.isArray(input.halfSkills)||new Set(input.halfSkills).size!==input.halfSkills.length||input.halfSkills.some(k=>!skillIds.includes(k)))fail();
  const allowedChecks=[...skillIds.map(k=>`skill:${k}`),...keys.map(k=>`save:${k}`),...keys.map(k=>`check:${k}`)];
  if(!Array.isArray(input.blockedChecks)||input.blockedChecks.some(k=>!allowedChecks.includes(k)))fail();
  const c=input.combat;if(!c||!integer(c.rageBonus,0,10)||!integer(c.extraMeleeCriticalDice,0,6)||['raging','greatWeaponMaster','greatWeaponMasterAvailable','thrown'].some(k=>typeof c[k]!=='boolean')||(c.greatWeaponMaster&&!c.greatWeaponMasterAvailable)||(c.raging&&!c.rageBonus))fail();
  if(!Array.isArray(input.facts)||input.facts.length>100||input.facts.some(t=>typeof t!=='string'||t.length>500))fail();
  return {tempHp:input.tempHp,conditions,deathSaves:{successes:d.successes,failures:d.failures,stabilized:d.stabilized},hitDice,currenciesAvailable,currencies,pactSlots,skillBonuses:map(input.skillBonuses,skillIds),saveBonuses:map(input.saveBonuses,keys),checkBonuses:map(input.checkBonuses,keys),halfAbilityChecks:input.halfAbilityChecks,halfSkills:[...input.halfSkills],blockedChecks:[...new Set(input.blockedChecks)],combat:{rageBonus:c.rageBonus,raging:c.raging,greatWeaponMaster:c.greatWeaponMaster,greatWeaponMasterAvailable:c.greatWeaponMasterAvailable,extraMeleeCriticalDice:c.extraMeleeCriticalDice,thrown:c.thrown},facts:[...input.facts]};
}
export function setCombatOption(state,key,value){
  if(!state.extended||!['raging','greatWeaponMaster','thrown'].includes(key)||typeof value!=='boolean')throw new Error('Opzione non disponibile');
  if(key==='raging'&&value)throw new Error('Attiva Ira usando la capacità: consuma un utilizzo');
  const extended=validateExtended({...state.extended,combat:{...state.extended.combat,[key]:value}});
  return {...state,extended,encounter:{...state.encounter,strike:null}};
}

export function adjustHeroHp(state,delta){
  if(!Number.isInteger(delta))throw new Error('Variazione PF non valida');
  if(delta>0&&state.hp===0&&state.extended?.deathSaves.failures===3)return state;
  const absorbed=delta<0?Math.min(state.extended?.tempHp??0,-delta):0;
  const hp=Math.max(0,Math.min(state.maxHp,state.hp+delta+absorbed));
  if(!state.extended)return {...state,hp,concentration:hp===0?null:state.concentration};
  const extended={...state.extended,tempHp:state.extended.tempHp-absorbed,combat:{...state.extended.combat,raging:hp===0?false:state.extended.combat.raging},deathSaves:((hp>0&&state.hp===0)||(state.hp>0&&hp===0))?{successes:0,failures:0,stabilized:false}:state.extended.deathSaves};
  return {...state,hp,extended,concentration:hp===0?null:state.concentration};
}
