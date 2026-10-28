import {modifier,proficiency,describeRoll,serialize,parseBackup} from './model.js';
import {modifierOrigin} from './beyond-origin.js';
import {beyondText} from './beyond-details.js';
import {validateReference,referencePages} from './reference.js';
const arr=v=>Array.isArray(v)?v:[],int=(n,min,max)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
const movement=['walk','fly','swim','climb','burrow'],moves=['Passo','Volo','Nuoto','Scalata','Scavo'];
const senseIds=['darkvision','blindsight','tremorsense','truesight'],senseNames=['Scurovisione','Vista cieca','Percezione tellurica','Vista pura'];
export function addBeyondReference(report,c){
 if(!report.draft)return report;
 const s=structuredClone(report.draft),rows={identity:[],movement:[],senses:[],languages:[],tools:[]},issues=[];
 const warn=t=>{if(!issues.includes(t))issues.push(t);};
 rows.identity.push(`Specie: ${beyondText(c.race?.fullName||c.race?.baseName||'da verificare').slice(0,150)}`);
 for(const cl of arr(c.classes))rows.identity.push(`${beyondText(cl.definition?.name||'Classe da verificare').slice(0,100)} ${cl.level}${cl.subclassDefinition?.name?' / '+beyondText(cl.subclassDefinition.name).slice(0,100):''}`);
 rows.identity.push(`Background: ${beyondText(c.background?.definition?.name||c.background?.customBackground?.name||'da verificare').slice(0,150)}`);
 const rawMods=Object.entries(c.modifiers??{}).flatMap(([group,v])=>arr(v).filter(m=>m&&typeof m.subType==='string').map(m=>({...m,group,origin:modifierOrigin(c,group,m)})));
 const mods=[],seen=new Map();
 for(const m of rawMods){
  const key=m.id==null?null:`${m.group}:${m.id}`;
  if(key&&seen.has(key)){
   const prior=seen.get(key),fields=['type','subType','value','statId','restriction','componentId','isGranted','requiresAttunement','duration','dice','tagConstraints'];
   if(fields.some(k=>JSON.stringify(prior[k]??null)!==JSON.stringify(m[k]??null))){prior.origin={status:'unknown'};m.origin={status:'unknown'};mods.push(m);warn(`Modificatore della scheda duplicato e discordante: ${m.subType}.`);}continue;
  }
  if(key)seen.set(key,m);mods.push(m);
 }
 const active=mods.filter(m=>m.origin.status==='active'),maybe=mods.filter(m=>m.origin.status==='unknown');
 const values=movement.map(k=>int(c.race?.weightSpeeds?.normal?.[k],0,999)?c.race.weightSpeeds.normal[k]:null),bad=new Set();
 const senseValues=[0,0,0,0],senseAdds=[0,0,0,0],senseBad=new Set(),moveFloors=[0,0,0,0,0],moveAdds=[0,0,0,0,0];
 let init=modifier(s.abilities.DES)+(s.extended.halfAbilityChecks?Math.floor(proficiency(s.level)/2):0)+(s.extended.checkBonuses.DES??0),initVerified=!s.extended.blockedChecks.includes('check:DES');
 const walkingMatches=new Set();
 const passive={perception:0,insight:0,investigation:0},passiveBad=new Set();
 for(const m of [...active,...maybe]){
  const restriction=typeof m.restriction==='string'?m.restriction.trim():m.restriction==null?'':null,conditional=restriction!=='',uncertain=m.origin.status!=='active';
  if(m.subType==='initiative'){
   if(uncertain||conditional||m.type!=='bonus'||m.statId!=null||!int(m.value,-30,30)){initVerified=false;warn('Iniziativa: effetto condizionale o formula da verificare.');}else init+=m.value;
  }
  for(const k of Object.keys(passive))if(m.subType===`passive-${k}`){if(uncertain||conditional||m.type!=='bonus'||m.statId!=null||!int(m.value,-30,30))passiveBad.add(k);else passive[k]+=m.value;}
  if(m.type==='language'||m.type==='proficiency'&&/tool|kit|supplies|instruments|vehicles/.test(m.subType)){
   const key=m.type==='language'?'languages':'tools',label=beyondText(m.friendlySubtypeName||m.subType).slice(0,180);rows[key].push(`${label}${uncertain||conditional?' (da verificare)':''}`);if(uncertain||conditional)warn(`Lingua/strumento ${label}: fonte condizionale o non riconciliata.`);
  }
  const at=movement.findIndex(k=>m.subType===`innate-speed-${k==='fly'?'flying':k==='walk'?'walking':k==='swim'?'swimming':k==='climb'?'climbing':'burrowing'}`);
  const all=m.subType==='speed';
  if(/speed|movement/.test(m.subType)&&at<0&&!all){movement.forEach((_,i)=>bad.add(i));warn(`Movimento ${m.subType}: regola non riconciliata.`);}
  if(at>=0||all){
   const targets=all?movement.map((_,i)=>i):[at];let restrictionOkay=!conditional;
   if(restriction==='Cannot be used if wearing medium or heavy armor.'){
    restrictionOkay=true;if(arr(c.inventory).some(i=>i.equipped===true&&i.quantity>0&&[2,3].includes(i.definition?.armorTypeId))){if(at>=0)values[at]=0;continue;}
    if(at>=0)rows.movement.push(`${moves[at]}: richiede assenza di armature medie/pesanti.`);
   }
   if(at>=0&&m.type==='set'&&m.value==null&&!uncertain&&restrictionOkay&&m.statId==null){walkingMatches.add(at);continue;}
   if(uncertain||!restrictionOkay||m.statId!=null||!int(m.value,m.type==='bonus'?-999:0,999)||!['set','bonus'].includes(m.type)){targets.forEach(i=>bad.add(i));warn(`Movimento ${m.subType}: effetto da verificare.`);}else targets.forEach(i=>{if(values[i]===null){bad.add(i);return;}if(m.type==='set')moveFloors[i]=Math.max(moveFloors[i],m.value);else moveAdds[i]+=m.value;});
  }
  const atSense=senseIds.indexOf(m.subType);
  if(atSense>=0){
   const additive=restriction==='plus 60 feet if wearer already has Darkvision'&&atSense===0&&m.value===60;
   if(uncertain||conditional&&!additive||m.statId!=null||!int(m.value,0,999)||!['sense','set','set-base','bonus'].includes(m.type)){senseBad.add(atSense);warn(`Senso ${m.subType}: effetto da verificare.`);}else if(m.type==='bonus'||additive)senseAdds[atSense]+=m.value;else senseValues[atSense]=Math.max(senseValues[atSense],m.value);
  }
 }
 values.forEach((v,i)=>{if(v!==null){const base=Math.max(v,moveFloors[i]);values[i]=base+(base>0?moveAdds[i]:0);}});senseValues.forEach((v,i)=>senseValues[i]=v+senseAdds[i]);
 const custom=(entries,ids,values,bad,label,idKey)=>{
  const seen=new Map();
  for(const row of arr(entries)){
   const at=ids.indexOf(row?.[idKey]);
   if(at<0||!int(row.distance,0,999)||Object.keys(row).some(k=>![idKey,'distance'].includes(k))){ids.forEach((_,i)=>bad.add(i));warn(`${label} personalizzati da verificare.`);continue;}
   if(seen.has(at)&&seen.get(at)!==row.distance){bad.add(at);warn(`${label} personalizzati discordanti.`);continue;}
   seen.set(at,row.distance);values[at]=row.distance;
  }
 };
 custom(c.customSpeeds,[1,4,5,3,2],values,bad,'Movimenti','movementId');
 for(const at of walkingMatches){if(bad.has(0)||values[0]===null)bad.add(at);else if(values[at]!==null)values[at]=Math.max(values[at],values[0]);}
 custom(c.customSenses,[2,1,3,4],senseValues,senseBad,'Sensi','senseId');
 rows.movement.push('Velocita base: condizioni, carico e terreno da applicare al tavolo.');
 movement.forEach((_,i)=>rows.movement.push(`${moves[i]}: ${bad.has(i)||!int(values[i],0,999)?'da verificare':values[i]+' ft'}`));
 senseIds.forEach((_,i)=>{if(senseValues[i]||senseBad.has(i))rows.senses.push(`${senseNames[i]}: ${senseBad.has(i)||senseValues[i]>999?'da verificare':senseValues[i]+' ft'}`);});
 if(!rows.senses.length)rows.senses.push('Nessun senso speciale riconciliato.');
 for(const [k,label] of [['perception','Percezione'],['insight','Intuizione'],['investigation','Indagare']]){const roll=describeRoll({...s,mode:'normal'},{kind:'skill',key:k});const value=10+roll.bonus+passive[k]+(roll.mode==='advantage'?5:roll.mode==='disadvantage'?-5:0);rows.senses.push(`${label} passiva: ${roll.blocked||passiveBad.has(k)?'da verificare':value} (fotografia all importazione)`);}
 for(const k of ['languages','tools']){rows[k]=[...new Set(rows[k])];if(!rows[k].length)rows[k].push('Nessuna voce riconciliata; verificare il sorgente.');}
 if(!int(init,-15,30))initVerified=false;
 const equipment=s.inventory.map((item,index)=>{
  const i=c.inventory?.[index],d=i?.definition,container=arr(c.inventory).find(x=>x.id!=null&&x.id===i?.containerEntityId);
  return {id:item.id,equipped:typeof i?.equipped==='boolean'&&!i?.pocketddEquipmentUnknown?i.equipped:null,attuned:typeof i?.isAttuned==='boolean'?i.isAttuned:null,canAttune:typeof d?.canAttune==='boolean'?d.canAttune:null,weight:Number.isFinite(d?.weight)&&d.weight>=0&&d.weight<=1000000&&(d.weightMultiplier==null||d.weightMultiplier===1)?d.weight:null,container:i?.containerEntityId===c.id?'Personaggio':container?String(container.definition?.name??'Contenitore').slice(0,100):null};
 });
 if(equipment.some(e=>e.equipped===null||e.attuned===null||e.weight===null||e.container===null))warn('Equipaggiamento: alcuni stati, pesi o contenitori non disponibili; contrassegnati con ?.');
 if([...bad].length||values.some(v=>v===null))warn('Movimento: almeno un valore richiede verifica.');
 if(senseBad.size||passiveBad.size)warn('Sensi o valori passivi da verificare.');
 if(!initVerified)warn('Iniziativa da verificare: nessun bonus utilizzabile viene inventato.');
 const reference=validateReference({groups:Object.entries(rows).map(([key,rows])=>({key,rows:rows.map(x=>x.slice(0,300))})),initiative:{verified:initVerified,bonus:initVerified?init:null,parts:`DES ${modifier(s.abilities.DES)}; bonus riconciliati ${init-modifier(s.abilities.DES)}. Fotografia: ricontrollare dopo modifiche.`},equipment});
 const pages=referencePages({extended:{reference},inventory:s.inventory});
 s.powers.push(...pages.map((p,i)=>({id:`pdd-reference-${i+1}`,type:'feature',name:p.title,description:p.description,passive:true,maxUses:0,remaining:0,recovery:'manual',reference:p.key})));
 s.powers.push({id:'pdd-reference-initiative',type:'feature',name:'Iniziativa',description:reference.initiative.verified?`Bonus ${reference.initiative.bonus>=0?'+':''}${reference.initiative.bonus}. ${reference.initiative.parts}`:'Da verificare. Effetti o formule non riconciliati: usare il dado libero dopo controllo.',passive:true,maxUses:0,remaining:0,recovery:'manual',reference:reference.initiative.verified?6:0,...(reference.initiative.verified?{attackModifier:reference.initiative.bonus}:{})});
 if(s.powers.length>120)throw new Error('Scheda e dati di consultazione oltre 120 voci: nessun dato viene tagliato.');
 const draft=parseBackup(serialize(s));
 return {...report,draft,conversion:{...report.conversion,omissions:[...report.conversion.omissions,...issues],evidence:[...report.conversion.evidence,'Profilo, movimento, sensi/passivi, lingue, strumenti ed equipaggiamento conservati per consultazione.'],coverage:[...(report.conversion.coverage??[]),{section:'Dati della scheda',status:issues.length?'partial':'data-only',count:rows.identity.length+rows.movement.length+rows.senses.length+rows.languages.length+rows.tools.length}]},details:{...report.details,reference}};
}
