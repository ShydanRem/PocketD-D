// Risorse manuali: nessun catalogo o effetto di incantesimi incorporato.
const integer=(n,min,max)=>Number.isInteger(n)&&n>=min&&n<=max;
export const emptyMagic=()=>({powers:[],slots:Array.from({length:9},()=>({max:0,current:0})),concentration:null,spellcastingAbility:'INT'});
export function validatePower(power) {
  if(!power || typeof power.id!=='string' || !/^[a-zA-Z0-9_-]{1,64}$/.test(power.id) || !['spell','feature'].includes(power.type) || typeof power.name!=='string' || !power.name.trim() || power.name.length>40 || typeof power.description!=='string' || power.description.length>600)throw new Error('Nome o descrizione non validi');
  const base={id:power.id,type:power.type,name:power.name.trim(),description:power.description};
  if(power.type==='spell') {
    if(!integer(power.level,0,9)||typeof power.concentration!=='boolean')throw new Error('Incantesimo non valido');
    return {...base,level:power.level,concentration:power.concentration};
  }
  if(!integer(power.maxUses,1,99)||!integer(power.remaining,0,power.maxUses)||!['short','long','manual'].includes(power.recovery))throw new Error('Utilizzi non validi');
  return {...base,maxUses:power.maxUses,remaining:power.remaining,recovery:power.recovery};
}
export function validateMagic(input) {
  if(!input || !Array.isArray(input.powers)||input.powers.length>40 || !Array.isArray(input.slots)||input.slots.length!==9 || !['FOR','DES','COS','INT','SAG','CAR'].includes(input.spellcastingAbility))throw new Error('Risorse magiche non valide');
  const powers=input.powers.map(validatePower);
  if(new Set(powers.map(power=>power.id)).size!==powers.length)throw new Error('Identificatori duplicati');
  const slots=input.slots.map(slot=>{
    if(!slot || !integer(slot.max,0,9)||!integer(slot.current,0,slot.max))throw new Error('Slot non validi');
    return {max:slot.max,current:slot.current};
  });
  if(input.concentration!==null&&!powers.some(power=>power.type==='spell'&&power.concentration&&power.id===input.concentration))throw new Error('Concentrazione non valida');
  if(input.hp===0&&input.concentration!==null)throw new Error('Concentrazione a zero PF');
  return {powers,slots,concentration:input.concentration,spellcastingAbility:input.spellcastingAbility};
}
export function upsertPower(state, input) {
  const power=validatePower(input);
  const exists=state.powers.some(item=>item.id===power.id);
  if(!exists&&state.powers.length>=40)throw new Error('Massimo 40 voci');
  const powers=exists?state.powers.map(item=>item.id===power.id?power:item):[...state.powers,power];
  const concentration=state.concentration===power.id&&(power.type!=='spell'||!power.concentration)?null:state.concentration;
  return {...state,powers,concentration};
}
export function deletePower(state,id) {
  return {...state,powers:state.powers.filter(item=>item.id!==id),concentration:state.concentration===id?null:state.concentration};
}
export function usePower(state,id,slotLevel=0) {
  const power=state.powers.find(item=>item.id===id);
  if(!power)throw new Error('Voce non trovata');
  if(power.type==='feature') {
    if(power.remaining<1)throw new Error('Utilizzi esauriti');
    return {...state,powers:state.powers.map(item=>item.id===id?{...item,remaining:item.remaining-1}:item)};
  }
  if(power.concentration&&state.hp===0)throw new Error('Non puoi concentrarti a zero PF');
  let slots=state.slots;
  if(power.level>0) {
    if(!integer(slotLevel,power.level,9)||state.slots[slotLevel-1].current<1)throw new Error('Slot non disponibile');
    slots=state.slots.map((slot,i)=>i===slotLevel-1?{...slot,current:slot.current-1}:slot);
  }
  return {...state,slots,concentration:power.concentration?power.id:state.concentration};
}
export function rest(state,kind) {
  if(!['short','long'].includes(kind))throw new Error('Riposo non valido');
  return {...state,
    hp:kind==='long'?state.maxHp:state.hp,resolve:kind==='long'?1:state.resolve,
    concentration:kind==='long'?null:state.concentration,
    slots:kind==='long'?state.slots.map(slot=>({...slot,current:slot.max})):state.slots,
    powers:state.powers.map(power=>power.type==='feature'&&(power.recovery===kind||(kind==='long'&&power.recovery==='short'))?{...power,remaining:power.maxUses}:power)
  };
}
