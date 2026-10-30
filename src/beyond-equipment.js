import {emptyGear} from './equipment.js';
// Import only armor formulas already reconciled by the core converter. Flat
// item boosts, attunement, caps, fixed scores and conditional grants need an
// explicit local profile until their full dependency can be represented.
export function beyondEquipment(source,state,{warn,evidence,modifiers=[]}){
 const rows=source.inventory??[],profiles=[];
 rows.forEach((row,index)=>{
  const d=row?.definition,item=state.inventory[index];if(!d||!item||item.type!=='gear'||d.filterType!=='Armor')return;
  if(![1,2,3,4].includes(d.armorTypeId)||!Number.isSafeInteger(d.armorClass)||d.armorClass<1||d.armorClass>(d.armorTypeId===4?10:30)||d.canAttune||d.isHomebrew||(d.grantedModifiers??[]).some(Boolean)){
   warn(`${item.name}: profilo equipaggiamento con effetti speciali da configurare sul Pocket.`);return;
  }
  profiles.push({...emptyGear(item.id),kind:d.armorTypeId+1,armor:d.armorClass,equipped:row.equipped===true&&item.quantity>0});
 });
 if(!profiles.length)return;
 const known=new Set(profiles.map(g=>g.id));rows.forEach((row,index)=>{const item=state.inventory[index];if(item?.type==='gear'&&!known.has(item.id)&&row?.equipped===true&&item.quantity>0)profiles.push({...emptyGear(item.id),equipped:true});});
 const formulas=modifiers.filter(m=>m.type==='set'&&m.subType==='unarmored-armor-class'&&[3,5].includes(m.statId)&&m.value==null);
 const rules=[...new Set(formulas.map(m=>m.statId===3?2:3))];
 state.extended.loadout={unarmored:rules.length===1?rules[0]:rules.length>1?0:1,items:profiles};
 if(rules.length>1)warn('Più formule senza armatura: scegli quella applicabile sul Pocket prima di togliere l armatura.');
 evidence.push(`Equipaggiamento: ${profiles.filter(g=>g.kind>=2).length} armature/scudi con formula operativa, senza applicare due volte la CA importata. Oggetti generici segnati equipaggiati senza inventare effetti.`);
}
export function beyondTrueStrike2024(definition){
 return definition?.id===2619204&&definition.level===0&&definition.concentration!==true&&Array.isArray(definition.sources)&&definition.sources.some(s=>s?.sourceId===145&&s.sourceType===1);
}
