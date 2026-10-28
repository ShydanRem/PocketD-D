const arr=v=>Array.isArray(v)?v:[];
const integer=(n,min,max)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
const slug=s=>String(s??'').trim().toLowerCase().replace(/[’']/g,'').replace(/[^a-z0-9]+/g,'-').replace(/^-|-$/g,'');
// A decision about source activity only. Rule-specific restrictions are handled by each converter.
export function modifierOrigin(c,group,m){
 const unknown=reason=>({status:'unknown',reason}),inactive={status:'inactive'};
 if(!m||m.restriction!=null&&typeof m.restriction!=='string'||!['race','class','feat','background','item'].includes(group))return unknown('origine non supportata');
 const choices=arr(c.choices?.[group]).filter(x=>m.id!=null&&typeof x?.id==='string'&&x.id.endsWith(`-${m.id}`));
 if(choices.length>1)return unknown('scelta duplicata');
 if(choices.length&&choices[0].optionValue==null)return inactive;
 if(m.isGranted===false){
  if(!choices.length)return inactive;
  const opts=arr(c.choices?.choiceDefinitions).flatMap(x=>arr(x?.options)).filter(o=>o?.id===choices[0].optionValue);
  if(!opts.length||opts.some(o=>slug(o.label)!==m.subType))return unknown('scelta discordante');
 }else if(m.isGranted!=null&&typeof m.isGranted!=='boolean')return unknown('concessione non valida');
 if(group==='class'){
  const owners=arr(c.classes).flatMap(cl=>arr(cl.classFeatures).filter(f=>f?.definition?.id===m.componentId).map(f=>({cl,f:f.definition})));
  if(owners.length!==1||!integer(owners[0].cl.level,1,20)||!integer(owners[0].f.requiredLevel,1,20))return unknown('origine/livello di classe non univoci');
  if(owners[0].f.requiredLevel>owners[0].cl.level)return inactive;
 }
 if(group==='item'){
  const owners=arr(c.inventory).flatMap(i=>arr(i?.definition?.grantedModifiers).filter(g=>m.id!=null&&g?.id!=null&&String(g.id)===String(m.id)).map(g=>({i,g})));
  if(owners.length!==1)return unknown('origine oggetto non univoca');
  const {i,g}=owners[0];
  if([i.definition.canAttune,g.requiresAttunement,m.requiresAttunement,g.isGranted].some(v=>v!=null&&typeof v!=='boolean'))return unknown('requisiti oggetto non validi');
  if(i.quantity===0||i.equipped===false||(i.definition.canAttune===true||g.requiresAttunement===true||m.requiresAttunement===true)&&i.isAttuned===false)return inactive;
  if(!integer(i.quantity,1,999)||i.equipped!==true||(i.definition.canAttune||g.requiresAttunement||m.requiresAttunement)&&i.isAttuned!==true)return unknown('equipaggiamento o sintonia da verificare');
  if(g.type!==m.type||g.subType!==m.subType||(g.value??null)!==(m.value??null)||(g.statId??null)!==(m.statId??null)||(g.restriction??'')!==(m.restriction??'')||g.isGranted===false||g.duration!=null||g.dice!=null||g.tagConstraints!=null&&(!Array.isArray(g.tagConstraints)||g.tagConstraints.length))return unknown('riepilogo e concessione oggetto discordanti');
 }
 if(m.duration!=null||m.dice!=null||m.tagConstraints!=null&&(!Array.isArray(m.tagConstraints)||m.tagConstraints.length)||m.requiresAttunement===true&&group!=='item')return unknown('durata o requisito non rappresentato');
 return {status:'active'};
}
