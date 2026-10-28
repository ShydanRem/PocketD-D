export const referenceGroups=['identity','movement','senses','languages','tools'];
export const referenceNames=['Profilo','Movimento','Sensi e passivi','Lingue','Strumenti'];
const integer=(n,min,max)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
export function validateReference(r){
 if(!r||Object.keys(r).length!==3||!Array.isArray(r.groups)||r.groups.length>5||!Array.isArray(r.equipment)||r.equipment.length>60||!r.initiative)throw new Error('Dati di consultazione non validi');
 const seen=new Set(),groups=r.groups.map(g=>{if(!g||!referenceGroups.includes(g.key)||seen.has(g.key)||!Array.isArray(g.rows)||g.rows.length>64||g.rows.some(s=>typeof s!=='string'||!s.trim()||s.length>300))throw new Error('Sezione della scheda non valida');seen.add(g.key);return {key:g.key,rows:[...g.rows]};});
 const i=r.initiative;if(typeof i.verified!=='boolean'||i.bonus!==null&&!integer(i.bonus,-15,30)||i.verified&&i.bonus===null||typeof i.parts!=='string'||i.parts.length>200)throw new Error('Iniziativa non valida');
 const ids=new Set(),equipment=r.equipment.map(e=>{
  if(!e||typeof e.id!=='string'||!/^[a-zA-Z0-9_-]{1,64}$/.test(e.id)||ids.has(e.id)||['equipped','attuned','canAttune'].some(k=>e[k]!==null&&typeof e[k]!=='boolean')||e.weight!==null&&(!Number.isFinite(e.weight)||e.weight<0||e.weight>1000000)||e.container!==null&&(typeof e.container!=='string'||e.container.length>100))throw new Error('Dati equipaggiamento non validi');ids.add(e.id);return {id:e.id,equipped:e.equipped,attuned:e.attuned,canAttune:e.canAttune,weight:e.weight,container:e.container};
 });
 return {groups,initiative:{verified:i.verified,bonus:i.bonus,parts:i.parts},equipment};
}
export function referencePages(state){
 const r=state.extended?.reference;if(!r)return [];
 const result=[],chunk=(key,title,rows)=>{
  let current='';const flush=()=>{if(current){result.push({key,title,description:current});current='';}};
  for(const row of rows){if(current.length+row.length+1>500)flush();current+=(current?'\n':'')+row;}flush();
 };
 r.groups.forEach(g=>chunk(referenceGroups.indexOf(g.key),referenceNames[referenceGroups.indexOf(g.key)],g.rows));
 const rows=r.equipment.flatMap(e=>{const i=state.inventory.find(x=>x.id===e.id);if(!i)return [];return [`${i.name} x${i.quantity}: ${e.equipped===null?'equip. ?':e.equipped?'equipaggiato':'non equip.'}; ${e.canAttune===false?'nessuna sintonia':e.attuned===null?'sintonia ?':e.attuned?'in sintonia':'non in sintonia'}; peso ${e.weight===null?'?':`${e.weight} lb/unita`}; contenitore ${e.container??'?'}.`];});
 chunk(5,'Equipaggiamento',rows);
 const counts={};result.forEach(p=>{counts[p.key]=(counts[p.key]??0)+1;});const at={};result.forEach(p=>{at[p.key]=(at[p.key]??0)+1;if(counts[p.key]>1)p.title+=` ${at[p.key]}/${counts[p.key]}`;});
 return result;
}
