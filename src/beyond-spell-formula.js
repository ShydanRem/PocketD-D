// Observed 2024 identities + independently checked structured base/scaling.
// Deliberately does not match a translated name or parse description text.
const known=new Map([
  [2619079,{level:1,kind:2,count:2,sides:8,ability:1,scaling:1,step:2,type:'bonus',sub:'hit-points'}],
  [2618890,{level:0,kind:1,count:1,sides:10,ability:0,scaling:2,step:1,type:'damage',sub:'fire'}],
  [2618967,{level:0,kind:1,count:1,sides:8,ability:0,scaling:2,step:1,type:'damage',sub:'radiant'}],
]);
const emptyHigher=h=>h&&typeof h==='object'&&!Array.isArray(h)&&Object.keys(h).every(k=>['higherLevelDefinitions','additionalAttacks','additionalTargets','areaOfEffect','duration','creatures','special','points','range'].includes(k)&&Array.isArray(h[k])&&!h[k].length);
const dieMatches=(die,count,sides)=>die&&die.diceCount===count&&die.diceValue===sides&&die.diceMultiplier==null&&(die.fixedValue==null||die.fixedValue===0)&&die.diceString===`${count}d${sides}`;
export function beyondSpellFormula(d,{warn=()=>{},evidence=()=>{}}={}){
  const rule=known.get(d?.id);if(!rule)return undefined;
  const reject=()=>{warn(`${d.name??'Magia'}: formula base non riconciliata; tiro manuale, nessuna formula automatica.`);return undefined;};
  if(d.isHomebrew!==false||!Array.isArray(d.sources)||!d.sources.some(s=>s.sourceId===145&&s.sourceType===1)||d.level!==rule.level||d.concentration!==false||d.duration?.durationType!=='Instantaneous'||d.duration.durationInterval!==0||d.duration.durationUnit!=null||d.scaleType!==(rule.scaling===1?'spellscale':'characterlevel')||!emptyHigher(d.atHigherLevels)||!Array.isArray(d.modifiers)||d.modifiers.length!==1)return reject();
  const m=d.modifiers[0];
  if(!m||m.type!==rule.type||m.subType!==rule.sub||m.restriction||m.statId!=null||m.requiresAttunement!==false||m.isGranted!==false||m.entityId!=null||m.entityTypeId!=null||!Array.isArray(m.bonusTypes)||m.bonusTypes.length||m.duration!=null||m.durationUnit!=null||m.count!==1||m.usePrimaryStat!==Boolean(rule.ability)||(m.value!=null&&m.value!==0)||(m.fixedValue!=null&&m.fixedValue!==0)||m.dice!=null||!dieMatches(m.die,rule.count,rule.sides))return reject();
  const higher=m.atHigherLevels,rows=higher?.higherLevelDefinitions;
  if(!Array.isArray(rows)||!emptyHigher({...higher,higherLevelDefinitions:[]}))return reject();
  const expected=rule.scaling===1?[[1,rule.step]]:[[5,2],[11,3],[17,4]];
  if(rows.length!==expected.length||!expected.every(([level,count],i)=>{const r=rows[i];return r?.level===level&&r.typeId===15&&(r.value==null||r.value===0)&&!r.details&&dieMatches(r.dice,count,rule.sides);}))return reject();
  evidence(`${d.name}: formula base strutturata ${rule.count}d${rule.sides}${rule.ability?' + caratteristica':''}; aumento ${rule.scaling===1?`+${rule.step}d${rule.sides} per livello slot`:'ai livelli PG 5/11/17'}. Colpire, TS, critici, bonus speciali e bersaglio restano al tavolo.`);
  return {kind:rule.kind,count:rule.count,sides:rule.sides,ability:rule.ability,scaling:rule.scaling,step:rule.step,source:1,bonus:0};
}
