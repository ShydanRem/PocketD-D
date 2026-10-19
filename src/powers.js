// Local resources and explicit effects; no bundled external catalogue.
const integer=(n,min,max)=>Number.isInteger(n)&&n>=min&&n<=max;
const keys=['FOR','DES','COS','INT','SAG','CAR'];
export const emptyMagic=()=>({powers:[],slots:Array.from({length:9},()=>({max:0,current:0})),concentration:null,spellcastingAbility:'INT'});
function resource(input){
  if(!input||!integer(input.maxUses,1,99)||!integer(input.remaining,0,input.maxUses)||!['short','long','manual'].includes(input.recovery))throw new Error('Utilizzi non validi');
  return {maxUses:input.maxUses,remaining:input.remaining,recovery:input.recovery};
}
export function validatePower(power,extended=false){
  if(!power||typeof power.id!=='string'||!/^[a-zA-Z0-9_-]{1,64}$/.test(power.id)||!['spell','feature'].includes(power.type)||typeof power.name!=='string'||!power.name.trim()||power.name.length>40||typeof power.description!=='string'||power.description.length>(extended?6000:600))throw new Error('Nome o descrizione non validi');
  const base={id:power.id,type:power.type,name:power.name.trim(),description:power.description};
  if(power.type==='spell'){
    if(!integer(power.level,0,9)||typeof power.concentration!=='boolean')throw new Error('Incantesimo non valido');
    Object.assign(base,{level:power.level,concentration:power.concentration});
    if(extended&&power.castingAbility!==undefined){
      if(!keys.includes(power.castingAbility)||!integer(power.attackModifier,-15,30)||!integer(power.saveModifier,-15,30)||typeof power.prepared!=='boolean'||typeof power.usesSpellSlot!=='boolean')throw new Error('Lancio non valido');
      Object.assign(base,{castingAbility:power.castingAbility,attackModifier:power.attackModifier,saveModifier:power.saveModifier,prepared:power.prepared,usesSpellSlot:power.usesSpellSlot});
    }
    if(extended&&power.limited!==undefined)base.limited=resource(power.limited);
  }else if(extended&&power.passive===true){
    Object.assign(base,{maxUses:0,remaining:0,recovery:'manual',passive:true});
  }else{
    Object.assign(base,resource(power));
    if(extended&&power.effect!==undefined){
      if(!['rage','relentless-endurance'].includes(power.effect))throw new Error('Effetto non valido');
      base.effect=power.effect;
    }
  }
  return base;
}
export function validateMagic(input,extended=false){
  if(!input||!Array.isArray(input.powers)||input.powers.length>(extended?120:40)||!Array.isArray(input.slots)||input.slots.length!==9||!keys.includes(input.spellcastingAbility))throw new Error('Risorse magiche non valide');
  const powers=input.powers.map(p=>validatePower(p,extended));
  if(new Set(powers.map(p=>p.id)).size!==powers.length)throw new Error('Identificatori duplicati');
  const slots=input.slots.map(s=>{if(!s||!integer(s.max,0,9)||!integer(s.current,0,s.max))throw new Error('Slot non validi');return{max:s.max,current:s.current}});
  if(input.concentration!==null&&!powers.some(p=>p.type==='spell'&&p.concentration&&p.id===input.concentration))throw new Error('Concentrazione non valida');
  if((input.hp===0||input.extended?.combat?.raging)&&input.concentration!==null)throw new Error('Concentrazione incompatibile con lo stato');
  return {powers,slots,concentration:input.concentration,spellcastingAbility:input.spellcastingAbility};
}
export function spellStats(state,power){
  const ability=power.castingAbility??state.spellcastingAbility;
  const base=Math.floor((state.abilities[ability]-10)/2)+2+Math.floor((state.level-1)/4);
  return {ability,attack:base+(power.attackModifier??0),dc:8+base+(power.saveModifier??0)};
}
export function availableSlots(state,power){
  if(power.level===0||power.usesSpellSlot===false)return [];
  const entries=[];
  state.slots.forEach((s,i)=>{if(i+1>=power.level&&s.current>0)entries.push({value:String(i+1),label:`${i+1}° · ${s.current} ordinari`})});
  state.extended?.pactSlots.forEach((s,i)=>{if(i+1>=power.level&&s.current>0)entries.push({value:`pact:${i+1}`,label:`${i+1}° · ${s.current} patto`})});
  return entries;
}
export function upsertPower(state,input){
  const power=validatePower(input,Boolean(state.extended)),exists=state.powers.some(p=>p.id===power.id),max=state.extended?120:40;
  if(!exists&&state.powers.length>=max)throw new Error(`Massimo ${max} voci`);
  return {...state,powers:exists?state.powers.map(p=>p.id===power.id?power:p):[...state.powers,power],concentration:state.concentration===power.id&&(power.type!=='spell'||!power.concentration)?null:state.concentration};
}
export function deletePower(state,id){return {...state,powers:state.powers.filter(p=>p.id!==id),concentration:state.concentration===id?null:state.concentration}}
export function usePower(state,id,slotLevel=0){
  const power=state.powers.find(p=>p.id===id);if(!power)throw new Error('Voce non trovata');
  if(power.type==='feature'){
    if(power.passive)throw new Error('Capacità informativa: effetto da risolvere al tavolo');
    if(power.remaining<1)throw new Error('Utilizzi esauriti');
    if(power.effect==='relentless-endurance'&&state.hp!==0)throw new Error('Usa questa capacità soltanto a zero PF, dopo verifica col DM');
    let next={...state,powers:state.powers.map(p=>p.id===id?{...p,remaining:p.remaining-1}:p)};
    if(power.effect==='rage'){
      if(!state.extended?.combat.rageBonus||state.hp===0||state.extended.combat.raging)throw new Error('Ira non disponibile o già attiva');
      next={...next,extended:{...state.extended,combat:{...state.extended.combat,raging:true}},concentration:null};
    }
    if(power.effect==='relentless-endurance')next={...next,hp:1,extended:{...state.extended,deathSaves:{successes:0,failures:0,stabilized:false}}};
    return next;
  }
  if(state.extended?.combat.raging)throw new Error('Termina Ira prima di lanciare una magia');
  if(power.prepared===false)throw new Error('Magia non preparata: verifica la preparazione');
  if(power.concentration&&state.hp===0)throw new Error('Non puoi concentrarti a zero PF');
  if(power.limited&&power.limited.remaining<1)throw new Error('Utilizzi esauriti');
  let next={...state};
  if(power.level>0&&power.usesSpellSlot!==false){
    const pact=typeof slotLevel==='string'&&slotLevel.startsWith('pact:');
    const level=Number(pact?slotLevel.slice(5):slotLevel),slots=pact?state.extended?.pactSlots:state.slots;
    if(!integer(level,power.level,9)||!slots||slots[level-1].current<1)throw new Error('Slot non disponibile');
    const updated=slots.map((s,i)=>i===level-1?{...s,current:s.current-1}:s);
    if(pact)next.extended={...state.extended,pactSlots:updated};else next.slots=updated;
  }
  if(power.limited)next.powers=state.powers.map(p=>p.id===id?{...p,limited:{...p.limited,remaining:p.limited.remaining-1}}:p);
  return {...next,concentration:power.concentration?power.id:state.concentration};
}
export function rest(state,kind){
  if(!['short','long'].includes(kind))throw new Error('Riposo non valido');
  const recover=r=>r&&(r.recovery===kind||(kind==='long'&&r.recovery==='short'))?{...r,remaining:r.maxUses}:r;
  return {...state,hp:kind==='long'?state.maxHp:state.hp,resolve:kind==='long'&&!state.importInfo?1:state.resolve,concentration:kind==='long'?null:state.concentration,
    slots:kind==='long'?state.slots.map(s=>({...s,current:s.max})):state.slots,
    ...(state.extended?{extended:{...state.extended,tempHp:kind==='long'?0:state.extended.tempHp,deathSaves:kind==='long'?{successes:0,failures:0,stabilized:false}:state.extended.deathSaves,pactSlots:state.extended.pactSlots.map(s=>({...s,current:s.max})),combat:{...state.extended.combat,raging:false,greatWeaponMaster:false}}}:{}),
    powers:state.powers.map(p=>p.type==='feature'&&!p.passive?{...p,...recover(p)}:p.type==='spell'&&p.limited?{...p,limited:recover(p.limited)}:p)};
}
