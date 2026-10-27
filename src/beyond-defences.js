import {damageTypes,damageNames,emptyDefences,defenceFlags,defenceLabel} from './defences.js';
const array=v=>Array.isArray(v)?v:[],integer=(v,min,max)=>Number.isSafeInteger(v)&&v>=min&&v<=max;
const kinds={resistance:1,vulnerability:2,immunity:4};
const conditions=new Set(['blinded','charmed','deafened','exhaustion','frightened','grappled','incapacitated','invisible','paralyzed','petrified','poisoned','prone','restrained','stunned','unconscious']);
export function beyondDefences(c,{warn,fact,evidence}){
 const d=emptyDefences(),flags=Array(13).fill(0),unknown=new Set(),seen=new Map();let unresolved=0;
 const uncertain=(m,reason)=>{const at=damageTypes.indexOf(m.subType);if(at>=0)unknown.add(at);else if(!conditions.has(m.subType))for(let i=0;i<13;i++)unknown.add(i);unresolved++;warn(`Difesa ${m.subType}: ${reason}; da verificare manualmente.`);};
 for(const [group,values] of Object.entries(c.modifiers??{}))for(const m of array(values)){
  if(!m||!Object.hasOwn(kinds,m.type))continue;
  const type=damageTypes.indexOf(m.subType),choice=array(c.choices?.[group]).some(x=>m.id!=null&&typeof x?.id==='string'&&x.id.endsWith(`-${m.id}`)&&x.optionValue!=null);
  if(m.isGranted===false&&!choice)continue;
  if(group==='item'){
   const owners=array(c.inventory).filter(i=>m.id!=null&&array(i?.definition?.grantedModifiers).some(g=>g?.id!=null&&String(g.id)===String(m.id)));
   if(owners.length!==1){uncertain(m,'origine oggetto non univoca');continue;}
   const i=owners[0],g=i.definition.grantedModifiers.find(g=>String(g?.id)===String(m.id));
   if(g.isGranted===false||g.type!==m.type||g.subType!==m.subType||(g.restriction??'')!==(m.restriction??'')){uncertain(m,'riepilogo e oggetto discordanti');continue;}
   if(i.quantity===0||i.equipped===false||i.definition.canAttune===true&&i.isAttuned===false||(m.requiresAttunement===true||g.requiresAttunement===true)&&i.isAttuned===false)continue;
   if(i.equipped!==true||!integer(i.quantity,1,999)||(i.definition.canAttune||m.requiresAttunement||g.requiresAttunement)&&i.isAttuned!==true){uncertain(m,'equipaggiamento o sintonia non risolti');continue;}
   if(g.duration!=null||g.statId!=null||g.tagConstraints!=null&&(!Array.isArray(g.tagConstraints)||g.tagConstraints.length)||g.isGranted!=null&&typeof g.isGranted!=='boolean'){uncertain(m,'requisiti della concessione oggetto non rappresentati');continue;}
  }else if(group==='class'){
   const owners=array(c.classes).filter(cl=>array(cl.classFeatures).some(f=>f?.definition?.id===m.componentId));
   if(owners.length!==1){uncertain(m,'origine di classe non univoca');continue;}
   const cl=owners[0],f=cl.classFeatures.find(f=>f?.definition?.id===m.componentId).definition;
   if(!integer(f.requiredLevel,1,20)||!integer(cl.level,1,20)){uncertain(m,'livello della capacità non risolto');continue;}
   if(f.requiredLevel>cl.level)continue;
  }else if(!['race','feat','background'].includes(group)){uncertain(m,'effetto temporaneo o origine non supportata');continue;}
  if(m.isGranted!=null&&typeof m.isGranted!=='boolean'||m.statId!=null||m.isGranted===false||m.restriction!=null&&(typeof m.restriction!=='string'||m.restriction.trim())||m.duration!=null||m.tagConstraints!=null&&(!Array.isArray(m.tagConstraints)||m.tagConstraints.length)||m.requiresAttunement===true&&group!=='item'){
   uncertain(m,'scelta, durata o requisito condizionale non rappresentato');continue;
  }
  if(conditions.has(m.subType)){fact(`Immunità/resistenza alla condizione ${m.subType}: annotazione, non automatizzata.`);warn(`Difesa dalla condizione ${m.subType}: non rappresentata nel profilo dei danni.`);continue;}
  if(type<0){uncertain(m,'tipo di danno non riconosciuto');continue;}
  if(m.id!=null){const key=`${group}:${m.id}`,old=seen.get(key);if(old&&(old.type!==m.type||old.subType!==m.subType)){uncertain(old,'identificativo duplicato discordante');uncertain(m,'identificativo duplicato discordante');continue;}seen.set(key,m);}
  flags[type]|=kinds[m.type];
 }
 for(let i=0;i<13;i++)if(flags[i]&&!unknown.has(i)){const bit=1<<i;d.configured|=bit;if(flags[i]&1)d.resistance|=bit;if(flags[i]&2)d.vulnerability|=bit;if(flags[i]&4)d.immunity|=bit;}
 const rows=damageNames.map((name,type)=>({type,name,flags:defenceFlags(d,type),label:defenceLabel(defenceFlags(d,type))}));
 if(d.configured)evidence.push(`Difese ai danni: ${rows.filter(r=>r.flags!==null).length} tipi con concessioni incondizionate riconciliate. Gli altri tipi restano da verificare, non sono dichiarati normali.`);
 return {profile:d.configured?d:undefined,rows,unresolved};
}
