import {emptyGear,validateLoadout} from './equipment.js';
const integer=(n,a,b)=>Number.isSafeInteger(n)&&n>=a&&n<=b;
const array=v=>Array.isArray(v)?v:[];
const abilities=['strength','dexterity','constitution','intelligence','wisdom','charisma'];
const keys=['FOR','DES','COS','INT','SAG','CAR'];
const fields={'armor-class':'ac','weapon-attacks':'attack','weapon-damage':'damage','spell-attacks':'spellAttack','spell-save-dc':'spellDc','saving-throws':'saves','ability-checks':'checks','hit-points':'maxHp'};
const conditional=g=>g.duration!=null||g.dice!=null||g.restriction!=null&&(typeof g.restriction!=='string'||g.restriction.trim())||g.tagConstraints!=null&&(!Array.isArray(g.tagConstraints)||g.tagConstraints.length)||g.isGranted!=null&&typeof g.isGranted!=='boolean'||g.requiresAttunement!=null&&typeof g.requiresAttunement!=='boolean';
const agrees=(a,b)=>a.type===b.type&&a.subType===b.subType&&a.value===b.value&&Boolean(a.requiresAttunement)===Boolean(b.requiresAttunement)&&(a.statId??null)===(b.statId??null)&&(a.entityId??null)===(b.entityId??null)&&!conditional(a)&&a.isGranted!==false&&(a.fixedValue==null||a.fixedValue===a.value);
// Explicit modifier identities/values, never character/item names or prose.
// The imported numbers already contain active bonuses: these are adopted, not added.
export function beyondEquipment(source,state,{warn,evidence,modifiers=[],report}){
 const rows=array(source.inventory),summaries=array(source.modifiers?.item),profiles=[];
 for(let index=0;index<rows.length;index++){
  const row=rows[index],d=row?.definition,item=state.inventory[index];if(!d||!item||item.type!=='gear')continue;
  const grants=array(d.grantedModifiers),armor=d.filterType==='Armor',requires=d.canAttune===true||grants.some(g=>g?.requiresAttunement===true);
  if(!armor&&!grants.length&&!requires)continue;
  let reason='';const reject=r=>{reason||=r;};
  if(d.isHomebrew===true||d.isCustomItem===true)reject('definizione personalizzata');
  if(d.canAttune!=null&&typeof d.canAttune!=='boolean'||typeof row.equipped!=='boolean'||requires&&typeof row.isAttuned!=='boolean')reject('equipaggiamento/sintonia non risolti');
  const profile={...emptyGear(item.id),equipped:row.equipped===true&&item.quantity>0,...(requires?{attunement:row.isAttuned===true&&item.quantity>0?2:1}:{})};
  if(armor){if(![1,2,3,4].includes(d.armorTypeId)||!integer(d.armorClass,1,d.armorTypeId===4?10:30)||requires)reject('formula armatura non riconciliata');else Object.assign(profile,{kind:d.armorTypeId+1,armor:d.armorClass});}
  const active=profile.equipped&&(!requires||profile.attunement===2);
  const ids=new Set();
  for(const g of grants){
   if(!g||g.id==null||ids.has(String(g.id))||conditional(g)||g.isGranted===false||g.type!=='bonus'||g.statId!=null||g.fixedValue!=null&&g.fixedValue!==g.value){reject('proprietà condizionali o non additive');continue;}
   ids.add(String(g.id));
   const owners=rows.filter(r=>array(r?.definition?.grantedModifiers).some(m=>m?.id!=null&&String(m.id)===String(g.id)));
   const matches=summaries.filter(m=>m?.id!=null&&String(m.id)===String(g.id));
   if(owners.length!==1||matches.length>1||active&&matches.length!==1||matches.some(m=>!agrees(m,g))){reject('origine o riepilogo dei bonus discordanti');continue;}
   const at=abilities.findIndex(n=>g.subType===`${n}-score`),field=fields[g.subType];
   if(g.entityId!=null&&(at<0||g.entityId!==at+1)){reject('bersaglio del bonus non riconciliato');continue;}
   if(at>=0){
    const a=report?.abilities?.[keys[at]];
    // A fixed/overridden/clamped score is nonlinear; adding/subtracting the raw
    // bonus would be wrong. Reject the whole automatic profile in that case.
    const inactivePotential=rows.filter(r=>!(r.equipped&&(!r.definition?.canAttune||r.isAttuned))).flatMap(r=>array(r.definition?.grantedModifiers)).filter(m=>m?.type==='bonus'&&m.subType===g.subType&&integer(m.value,1,10)).reduce((sum,m)=>sum+m.value,0);
    if(!integer(g.value,1,10)||!a||a.override!==null||a.setFloor||a.base+a.modifierBonus>a.maximum||a.preview+inactivePotential>a.maximum||at===2&&report.hp?.override!==null)reject('caratteristica con massimo, minimo o override da gestire manualmente');
    else profile.ability[at]+=g.value;
   }else if(field){
    if(!integer(g.value,field==='maxHp'?-999:-10,field==='maxHp'?999:10)||field==='maxHp'&&report?.hp?.override!==null)reject('bonus numerico o PF con override non riconciliati');
    else profile[field]+=g.value;
   }else reject('proprietà non ancora rappresentata dal profilo');
  }
  if(!reason){try{validateLoadout({unarmored:1,items:[profile]});}catch{reject('somma dei bonus oltre i limiti locali');}}
  if(reason){warn(`${item.name}: profilo automatico non applicabile (${reason}); configura e verifica le proprietà sul Pocket.`);continue;}
  profiles.push(profile);
 }
 if(!profiles.length)return;
 const known=new Set(profiles.map(g=>g.id));rows.forEach((row,index)=>{const item=state.inventory[index];if(item?.type==='gear'&&!known.has(item.id)&&row?.equipped===true&&item.quantity>0&&!row.definition?.canAttune&&!array(row.definition?.grantedModifiers).length)profiles.push({...emptyGear(item.id),equipped:true});});
 const formulas=modifiers.filter(m=>m.type==='set'&&m.subType==='unarmored-armor-class'&&[3,5].includes(m.statId)&&m.value==null);
 const rules=[...new Set(formulas.map(m=>m.statId===3?2:3))];
 state.extended.loadout=validateLoadout({unarmored:rules.length===1?rules[0]:rules.length>1?0:1,items:profiles});
 if(rules.length>1)warn('Più formule senza armatura: scegli quella applicabile sul Pocket prima di togliere l armatura.');
 const attuned=profiles.filter(g=>g.attunement!==undefined).length;
 evidence.push(`Equipaggiamento: ${profiles.length} profili operativi, ${profiles.filter(g=>g.kind>=2).length} armature/scudi e ${attuned} oggetti con sintonia distinta. Valori sorgente adottati senza raddoppiare i bonus; attivazione da stato equipaggiato e sintonia.`);
 if(attuned)warn('Sintonia: stato e bonus permanenti riconciliati; limiti di classe, requisiti, tempi e interruzioni della sintonia richiedono conferma al tavolo.');
}
export function beyondTrueStrike2024(definition){
 return definition?.id===2619204&&definition.level===0&&definition.concentration!==true&&Array.isArray(definition.sources)&&definition.sources.some(s=>s?.sourceId===145&&s.sourceType===1);
}
