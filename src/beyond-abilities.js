// Bounded source arithmetic, independent of names/descriptions and of the local sheet.
const keys=['FOR','DES','COS','INT','SAG','CAR'];
const names=['strength','dexterity','constitution','intelligence','wisdom','charisma'];
const integer=(v,min,max)=>Number.isSafeInteger(v)&&v>=min&&v<=max;
export const isAbilityModifier=m=>m.subType==='ability-score-maximum'||names.some(n=>m.subType===`${n}-score`);
export const abilityFloorRestriction=m=>m.type==='set'&&names.some(n=>m.subType===`${n}-score`)&&m.restriction==='if not already higher';
const conditional=m=>m.duration!=null||m.dice!=null||m.tagConstraints!=null&&(!Array.isArray(m.tagConstraints)||m.tagConstraints.length)||m.isGranted!=null&&typeof m.isGranted!=='boolean'||m.requiresAttunement!=null&&typeof m.requiresAttunement!=='boolean';
export function reconcileAbilities(abilities,active,{block,evidence,used,inventory}){
 const candidates=active.filter(isAbilityModifier),valid=[];
 for(const m of candidates){
  let problem=m.group==='condition'||conditional(m)||!integer(m.value,-1000,1000)||m.fixedValue!=null&&m.fixedValue!==m.value;
  const target=names.findIndex(n=>m.subType===`${n}-score`)+1;
  if(target)problem||=m.statId!=null||m.entityId!=null&&m.entityId!==target||!['bonus','set'].includes(m.type)||m.type==='set'&&!integer(m.value,1,30);
  else problem||=m.type!=='bonus'||!integer(m.statId,1,6)||!integer(m.value,1,10)||m.entityId!=null&&m.entityId!==m.statId;
  if(m.requiresAttunement===true&&m.group!=='item')problem=true;
  if(m.group==='item'){
   const owners=inventory.filter(i=>m.id!=null&&Array.isArray(i?.definition?.grantedModifiers)&&i.definition.grantedModifiers.some(g=>g?.id!=null&&String(g.id)===String(m.id)));
   if(owners.length!==1)problem=true;
   else{
    const i=owners[0],grants=i.definition.grantedModifiers.filter(g=>g?.id!=null&&String(g.id)===String(m.id)),g=grants[0];
    problem||=i.definition.canAttune!=null&&typeof i.definition.canAttune!=='boolean'||grants.length!==1||conditional(g)||g.isGranted===false||g.type!==m.type||g.subType!==m.subType||g.value!==m.value||(g.statId??null)!==(m.statId??null)||(g.entityId??null)!==(m.entityId??null)||(g.restriction??'')!==(m.restriction??'')||g.fixedValue!=null&&g.fixedValue!==m.value;
    problem||=(g.requiresAttunement===true||m.requiresAttunement===true||i.definition.canAttune===true)&&i.isAttuned!==true;
   }
  }
  if(problem){block(`Modificatore caratteristica non riconciliato: ${m.type} ${m.subType}.`);continue;}
  used.add(m);valid.push(m);
 }
 for(let i=0;i<6;i++){
  const a=abilities[keys[i]],mods=valid.filter(m=>m.subType===`${names[i]}-score`),caps=valid.filter(m=>m.subType==='ability-score-maximum'&&m.statId===i+1);
  const maximum=20+caps.reduce((s,m)=>s+m.value,0),modifierBonus=mods.filter(m=>m.type==='bonus').reduce((s,m)=>s+m.value,0),floor=Math.max(0,...mods.filter(m=>m.type==='set').map(m=>m.value));
  if(!integer(maximum,20,30))block(`Massimo ${keys[i]} fuori dai limiti locali (20–30).`);
  if(a.override===null&&a.base>maximum)block(`Punteggio base ${keys[i]} oltre il massimo riconciliato: serve verifica.`);
  // DDB bonusStats is a custom bonus AFTER the ordinary cap; item set values
  // are floors (never reduce an already higher score); overrideStats wins last.
  const calculated=Math.min(maximum,a.base+modifierBonus)+a.bonus;
  a.modifierBonus=modifierBonus;a.maximum=maximum;a.setFloor=floor;a.calculated=calculated;
  a.preview=a.override??Math.max(calculated,floor);
  if(!integer(a.preview,1,30))block(`Caratteristica ${keys[i]} fuori dai limiti locali (1–30).`);
  evidence.push(`${keys[i]}: base ${a.base} + modificatori ${modifierBonus}, massimo ${maximum}; bonus personalizzato successivo ${a.bonus}${floor?`; minimo fissato ${floor}`:''}${a.override!==null?`; override finale ${a.override}`:''} = ${a.preview}.`);
 }
}
