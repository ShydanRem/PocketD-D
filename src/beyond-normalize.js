// Reconcile structural/cosmetic source fields. Never infer mechanics from prose.
import {beyondText} from './beyond-details.js';
const array=v=>Array.isArray(v)?v:[];
const integer=(v,min,max)=>Number.isSafeInteger(v)&&v>=min&&v<=max;
export function normalizeBeyondSource(payload){
  const result={payload:structuredClone(payload),blockers:[],evidence:[],omissions:[]};
  const c=result.payload?.data;if(!c||typeof c!=='object'||Array.isArray(c))return result;
  const block=t=>{if(!result.blockers.includes(t))result.blockers.push(t);};
  if(!Array.isArray(c.inventory))return result;
  for(const row of c.inventory)if(row&&typeof row==='object')for(const key of ['pocketddCustom','pocketddEquipmentUnknown','pocketddNote','pocketddOriginalName'])delete row[key];
  const original=c.inventory.length,custom=array(c.customItems),seen=new Set();let added=0,linked=0;
  for(const item of custom){
    if(!item||!integer(item.id,1,Number.MAX_SAFE_INTEGER)||seen.has(item.id)||typeof item.name!=='string'||!item.name.trim()||!integer(item.quantity,0,999)||[item.description,item.notes].some(t=>t!=null&&typeof t!=='string')){
      block('Oggetto personalizzato non valido o identificativo duplicato.');continue;
    }
    seen.add(item.id);
    const rows=c.inventory.filter(i=>i?.definition?.isCustomItem===true&&i.definition.id===item.id);
    if(rows.length>1){block('Più righe per lo stesso oggetto personalizzato: riconciliazione ambigua.');continue;}
    if(rows.length){
      const row=rows[0],d=row.definition;
      if(d.name!==item.name||row.quantity!==item.quantity||array(d.grantedModifiers).length||d.armorTypeId!=null||d.armorClass!=null||d.damage!=null||d.fixedDamage!=null){block('Oggetto personalizzato con dati discordanti o meccaniche da verificare.');continue;}
      d.filterType='Other Gear';row.pocketddCustom=true;row.pocketddNote=[...new Set([d.description,item.description,item.notes].filter(t=>typeof t==='string'&&t))].join(' ');linked++;
    }else{
      c.inventory.push({id:`custom-${item.id}`,quantity:item.quantity,equipped:false,pocketddCustom:true,pocketddEquipmentUnknown:true,
        definition:{id:item.id,name:item.name,description:item.description,isCustomItem:true,filterType:'Other Gear',grantedModifiers:[]},pocketddNote:[item.description,item.notes].filter(t=>typeof t==='string'&&t).join(' ')});added++;
    }
  }
  // A merged custom row without its source definition is ambiguous, not generic loot.
  for(const row of c.inventory)if(row?.definition?.isCustomItem===true&&!row.pocketddCustom)block('Definizione originale di un oggetto personalizzato non riconciliata.');
  const remaining=[],overrides=new Map();
  for(const value of array(c.characterValues)){
    if(!value||![8,9].includes(value.typeId)){remaining.push(value);continue;}
    const rows=c.inventory.filter(i=>i?.id!=null&&String(i.id)===String(value.valueId));
    if(rows.length!==1||String(value.valueTypeId)!=='1439493548'||typeof value.value!=='string'){
      block('Nome o nota personalizzata senza un oggetto univoco o formato valido.');continue;
    }
    const key=`${value.typeId}:${value.valueId}`;
    if(overrides.has(key)){if(overrides.get(key)!==value.value)block('Personalizzazioni discordanti dello stesso oggetto.');continue;}
    overrides.set(key,value.value);const row=rows[0];
    if(value.typeId===8){row.pocketddOriginalName??=row.definition.name;row.definition.name=value.value;}
    else row.pocketddNote=[row.pocketddCustom?row.pocketddNote:'',value.value].filter(Boolean).join(' ');
  }
  if(Array.isArray(c.characterValues))c.characterValues=remaining;
  for(const row of c.inventory){
    if(row?.pocketddCustom){const note=beyondText(row.pocketddNote??row.definition.description??'');row.pocketddNote=note;}
    else if(row?.pocketddNote!=null)row.pocketddNote=beyondText(row.pocketddNote);
    if(row?.pocketddNote?.length>250){row.pocketddNote=row.pocketddNote.slice(0,250);result.omissions.push('Note personalizzate lunghe: conservato un estratto nella copia PC.');}
  }
  if(custom.length){result.evidence.push(`Oggetti personalizzati: ${linked} già presenti riconciliati per ID, ${added} aggiunti dalla lista custom; quantità conservate.`);result.omissions.push('Oggetti personalizzati conservati come oggetti generici: effetti, prezzi, peso e proprietà descrittive restano manuali. Per le voci presenti solo nella lista custom lo stato equipaggiato non è disponibile.');}
  if(overrides.size)result.evidence.push(`${overrides.size} nomi/note personalizzati degli oggetti riconciliati; nessuna modifica numerica da questi campi.`);
  if((c.preferences?.enableOptionalOrigins||c.preferences?.enableOptionalClassFeatures)&&!array(c.optionalOrigins).length&&!array(c.optionalClassFeatures).length)result.evidence.push('Preferenze opzionali abilitate, senza sostituzioni di origini o capacità selezionate nelle liste sorgente.');
  if(Array.isArray(c.customItems))c.customItems=[]; // All valid definitions were reconciled; failures remain blockers.
  result.sourceCounts={inventory:original,customDefinitions:custom.length,addedCustom:added,mergedInventory:c.inventory.length};
  return result;
}
export function selectedAbilityBonus(c,group,m){
  const abilities=['strength','dexterity','constitution','intelligence','wisdom','charisma'];
  const index=abilities.findIndex(n=>m.subType===`${n}-score`);
  if(index<0||m.type!=='bonus'||!integer(m.value,1,2)||m.statId!=null||m.id==null)return false;
  if(m.entityId!=null&&m.entityId!==index+1)return false;
  const choices=array(c.choices?.[group]).filter(x=>typeof x?.id==='string'&&x.id.endsWith(`-${m.id}`));
  if(choices.length!==1)return false;const choice=choices[0];
  if(choice.type!==2||choice.subType!==5||!integer(choice.optionValue,1,Number.MAX_SAFE_INTEGER)||!array(choice.optionIds).includes(choice.optionValue))return false;
  const options=array(c.choices?.choiceDefinitions).flatMap(d=>array(d?.options)).filter(o=>o?.id===choice.optionValue);
  return options.length>0&&options.every(o=>typeof o.label==='string'&&o.label.trim().toLowerCase().replace(/\s+/g,'-')===m.subType);
}
