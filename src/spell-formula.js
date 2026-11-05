// Portable base formulas. No rules are granted by names or description text.
const int=(n,a,b)=>Number.isInteger(n)&&n>=a&&n<=b;
export const emptySpellFormula=()=>({kind:0,count:0,sides:0,ability:0,scaling:0,step:0,source:0,bonus:0});
export function validateSpellFormula(input,power){
  const names=['kind','count','sides','ability','scaling','step','source','bonus'];
  if(!input||typeof input!=='object'||Array.isArray(input)||Object.keys(input).length!==8||names.some(k=>!Object.hasOwn(input,k)))throw Error('Formula magia non valida');
  const f=Object.fromEntries(names.map(k=>[k,input[k]]));
  if(names.some(k=>!Number.isInteger(f[k])))throw Error('Formula magia non valida');
  if(f.kind===0){if(names.some(k=>f[k]!==0))throw Error('Formula assente incoerente');return f;}
  const feature=power?.type!==undefined?power.type!=='spell':power?.kind!==0;
  if(!power||feature||power.effect||!int(f.kind,1,2)||!int(f.count,1,18)||![4,6,8,10,12,20].includes(f.sides)||!int(f.ability,0,1)||!int(f.scaling,0,2)||!int(f.source,0,1)||!int(f.bonus,-15,60)||(f.scaling?!int(f.step,1,18):f.step!==0)||(f.scaling===1&&!power.level)||(f.scaling===2&&power.level))throw Error('Formula magia non rappresentabile');
  return f;
}
export function resolveSpellFormula(state,power,slot=0,pact=false){
  const f=validateSpellFormula(power.formula,power);if(!f.kind)throw Error('Formula assente');
  if(power.level&&power.usesSpellSlot!==false){if(!int(slot,power.level,9))throw Error('Livello slot non valido');}else if(slot||pact)throw Error('Questa magia non usa uno slot');
  const steps=f.scaling===1?(slot||power.level)-power.level:f.scaling===2?Number(state.level>=5)+Number(state.level>=11)+Number(state.level>=17):0;
  const count=f.count+f.step*steps,ability=power.castingAbility??state.spellcastingAbility;
  const bonus=f.bonus+(f.ability?Math.floor((state.abilities[ability]-10)/2):0);
  if(!int(count,1,18)||!int(bonus,-15,60))throw Error('Formula oltre i limiti: nessun dado tagliato');
  return {kind:f.kind,count,sides:f.sides,bonus};
}
