// Read-only feasibility preview. This is intentionally not a character backup.
export class BeyondError extends Error{
  constructor(code,message){super(message);this.name='BeyondError';this.code=code}
}
const fail=(code,message)=>{throw new BeyondError(code,message)};
const integer=(value,min,max)=>Number.isSafeInteger(value)&&value>=min&&value<=max;
export function beyondCharacterId(input){
  if(typeof input!=='string'||input.length>500)fail('INVALID_URL','Inserisci un URL Beyond o un ID numerico valido');
  const value=input.trim();
  if(/^[1-9]\d{0,14}$/.test(value))return value;
  let url;try{url=new URL(value)}catch{fail('INVALID_URL','URL Beyond non valido')}
  if(url.protocol!=='https:'||!['www.dndbeyond.com','dndbeyond.com'].includes(url.hostname)||url.port||url.username||url.password)fail('INVALID_URL','Sono ammessi solo URL HTTPS di dndbeyond.com');
  const match=url.pathname.match(/^\/(?:profile\/[^/]+\/)?characters\/([1-9]\d{0,14})\/?$/);
  if(!match)fail('INVALID_URL','Usa il link della scheda /characters/ID');
  return match[1];
}
export function beyondEndpoint(input){return `https://character-service.dndbeyond.com/character/v5/character/${beyondCharacterId(input)}?includeCustomItems=true`}

// Raw validated HP inputs are separate from a preliminary arithmetic preview.
export function beyondHitPointInputs(c){
  if(!integer(c.baseHitPoints,0,9999)||['bonusHitPoints','overrideHitPoints','removedHitPoints','temporaryHitPoints'].some(k=>c[k]!=null&&!integer(c[k],0,9999)))return null;
  return {base:c.baseHitPoints,override:c.overrideHitPoints||null,bonus:c.bonusHitPoints??0,removed:c.removedHitPoints??0,temporary:c.temporaryHitPoints??0};
}

export function previewBeyond(payload,expectedId){
  const c=payload?.data;
  if(payload?.success!==true||!c||typeof c!=='object'||Array.isArray(c))fail('INVALID_DATA','Risposta priva di una scheda Beyond valida');
  if(String(c.id)!==beyondCharacterId(expectedId))fail('ID_MISMATCH','La risposta appartiene a un altro personaggio');
  if(typeof c.name!=='string'||!c.name.trim()||c.name.length>300)fail('INVALID_DATA','Nome del personaggio non valido');
  if(!Array.isArray(c.classes)||c.classes.length<1||c.classes.length>20||c.classes.some(entry=>!integer(entry?.level,1,20)))fail('INVALID_DATA','Livelli delle classi non validi');
  const level=c.classes.reduce((sum,entry)=>sum+entry.level,0);
  if(!integer(level,1,20))fail('INVALID_DATA','Livello totale fuori intervallo');
  function scores(entries,required=false){
    if(entries==null&&!required)return new Map();
    if(!Array.isArray(entries)||entries.length!==6||entries.some(e=>!e||!integer(e.id,1,6)||!(e.value===null||integer(e.value,0,1000)))||new Set(entries.map(e=>e.id)).size!==6)fail('INVALID_DATA','Caratteristiche Beyond incomplete o non valide');
    return new Map(entries.map(e=>[e.id,e.value]));
  }
  const base=scores(c.stats,true),bonus=scores(c.bonusStats),override=scores(c.overrideStats);
  const abilities=Object.fromEntries(['FOR','DES','COS','INT','SAG','CAR'].map((key,index)=>{
    const id=index+1,b=base.get(id),extra=bonus.get(id)??0,over=override.get(id)??0;
    if(!integer(b,1,30))fail('INVALID_DATA','Caratteristica base fuori intervallo');
    return [key,{base:b,bonus:extra,override:over||null,preview:over||b+extra}];
  }));
  const warnings=[
    'Anteprima sperimentale: non crea o aggiorna schede PocketD-D.',
    'Caratteristiche e PF sono calcoli parziali: modificatori di classe, talenti, oggetti ed effetti richiedono ancora riconciliazione.',
    'CA, competenze, attacchi, magie, slot e condizioni non vengono convertiti da questa prova.'
  ];
  let hp=null;
  const hpInputs=beyondHitPointInputs(c);
  if(hpInputs){
    const constitution=Math.floor((abilities.COS.preview-10)/2)*level;
    const max=(hpInputs.override??hpInputs.base+constitution)+hpInputs.bonus;
    if(max>0)hp={...hpInputs,constitutionContribution:constitution,previewMax:max,previewCurrent:Math.max(0,max-hpInputs.removed)};
  }
  if(!hp)warnings.push('PF incompleti o incoerenti: nessun valore stimato.');
  if(c.name.trim().length>24)warnings.push('Il nome supera il limite attuale di 24 caratteri di PocketD-D.');
  if(Object.values(abilities).some(a=>!integer(a.preview,1,30)))warnings.push('Una caratteristica supera i limiti attuali di PocketD-D.');
  if(hp&&hp.previewMax>999)warnings.push('I PF massimi superano i limiti attuali di PocketD-D.');
  if(c.inventory!==undefined&&(!Array.isArray(c.inventory)||c.inventory.length>5000))fail('INVALID_DATA','Inventario sorgente non valido');
  const inventory=c.inventory??[];
  const items=inventory.slice(0,60).map(item=>({
    name:typeof item?.definition?.name==='string'?item.definition.name.slice(0,300):'Nome non disponibile',
    quantity:integer(item?.quantity,0,999999)?item.quantity:null,equipped:item?.equipped===true
  }));
  if(inventory.length>60)warnings.push('Inventario oltre 60 voci: il riepilogo mostra soltanto le prime 60.');
  return {format:'pocketdd-beyond-probe',version:1,character:{id:String(c.id),name:c.name.trim(),level},abilities,hp,armorClass:null,inventory:{total:inventory.length,items},canImport:false,warnings};
}
