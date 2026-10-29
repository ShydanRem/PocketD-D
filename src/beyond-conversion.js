import {finalizeBeyondReport} from './beyond-report.js';
import {addBeyondReference} from './beyond-reference.js';
import {reconcileAbilities,abilityFloorRestriction} from './beyond-abilities.js';
import {normalizeBeyondSource,selectedAbilityBonus} from './beyond-normalize.js';
import {addBeyondAppearance} from './beyond-avatar.js';
import {extendBeyond} from './beyond-details.js';
import {previewBeyond,beyondHitPointInputs} from './beyond.js';
import {initialState, parseBackup, serialize} from './model.js';

const keys = ['FOR','DES','COS','INT','SAG','CAR'];
const integer = (n,min,max) => Number.isSafeInteger(n) && n>=min && n<=max;
const mod = n => Math.floor((n-10)/2);
// A weapon's casting ability is not an increase to the character's score.
// These modifiers are retained for explicit manual weapon configuration below.
const coreModifier = m => m.type!=='replace-weapon-ability'&&m.subType!=='unarmored-movement'&&/score|hit-point|armor-class|armored|unarmored/i.test(m.subType ?? '');
export const BEYOND_LIMITATIONS = [
  'Importazione parziale: abilità e salvezze sono senza competenze; completarle prima di usarne i tiri.',
  'Armi importate come oggetti: attacchi, danni, proprietà e arma attiva sono da configurare.',
  'Magie, slot, capacità di classe, talenti, resistenze e vantaggi condizionali non vengono importati.',
  'PF temporanei, condizioni, dadi vita e tiri morte non sono rappresentati.',
  'Peso, contenitori, sintonia ed effetti degli oggetti non sono trasferiti. Lo stato equipaggiato sorgente è nelle note.',
  'La CA è una fotografia dei dati letti: cambiare equipaggiamento o caratteristiche richiede aggiornarla manualmente.',
];

// Deliberately bounded conversion. Unknown core effects block creation instead of
// turning a successful HTTP request into a claim of mechanical equivalence.
function convertCoreNormalized(payload, expectedId, normalization, extended=false) {
  const report = previewBeyond(payload, expectedId), c = {...payload.data};
  report.inventory.items.forEach((item,i)=>{if(c.inventory?.[i]?.pocketddEquipmentUnknown)item.equipped=null;});
  const blockers = [...normalization.blockers], evidence = [...normalization.evidence];
  const block = message => { if (!blockers.includes(message)) blockers.push(message); };
  for (const key of ['feats','characterValues','customDefenseAdjustments','customItems','optionalOrigins','optionalClassFeatures']) {
    if (c[key]!=null && !Array.isArray(c[key])) block(`Elenco ${key} non valido.`);
    c[key]=Array.isArray(c[key])?c[key]:[];
  }
  if(normalization.training.length&&!extended)block('Le personalizzazioni dei tiri richiedono la conversione estesa, non la sola scheda di base.');
  const groups = c.modifiers;
  if (!groups || typeof groups !== 'object' || Array.isArray(groups)) block('Modificatori mancanti: impossibile verificare i valori finali.');
  for (const group of ['race','class','background','feat','item']) {
    if (!Array.isArray(groups?.[group])) block(`Gruppo modificatori ${group} mancante o non valido.`);
  }
  if ((c.optionalOrigins?.length ?? 0)>0 || (c.optionalClassFeatures?.length ?? 0)>0) {
    block('Origini o capacità opzionali richiedono una riconciliazione non ancora supportata.');
  }
  if ((c.characterValues?.length ?? 0)>0 || (c.customDefenseAdjustments?.length ?? 0)>0) {
    block('Override personalizzati non ancora riconciliati: controllare caratteristiche, PF e CA.');
  }
  const inventorySource = Array.isArray(c.inventory) ? c.inventory : [];
  if (!Array.isArray(c.inventory)) block('Inventario mancante: impossibile verificare gli oggetti.');
  for (const item of inventorySource) {
    if (!item || typeof item.definition?.filterType!=='string' || typeof item.equipped!=='boolean' ||
        (item.definition.armorTypeId!=null && item.definition.filterType!=='Armor')) block('Tipo o equipaggiamento di un oggetto non risolto.');
    if (item?.equipped && (!item.definition?.canAttune || item.isAttuned)) {
      for (const grant of Array.isArray(item.definition?.grantedModifiers)?item.definition.grantedModifiers:[]) {
        if (grant && coreModifier(grant) && !(Array.isArray(groups?.item)?groups.item:[]).some(m=>m?.id!=null && String(m.id)===String(grant?.id) && m.type===grant.type && m.subType===grant.subType && m.value===grant.value && m.statId===grant.statId)) block('Modificatori di un oggetto equipaggiato assenti o discordanti nel riepilogo.');
      }
    }
  }
  const active = [], modifierIds = new Set();
  for (const [group, entries] of Object.entries(groups ?? {})) {
    if (!Array.isArray(entries)) { block(`Modificatori ${group} non validi.`); continue; }
    for (const m of entries) {
      if (!m || typeof m.type !== 'string' || typeof m.subType !== 'string') { block('Modificatore non valido.'); continue; }
      if (!coreModifier(m)) continue;
      if (m.id!=null) {
        const id=`${group}:${m.id}`;
        if(modifierIds.has(id)) { block('Modificatori principali duplicati.'); continue; }
        modifierIds.add(id);
      }
      let classLevel = report.character.level;
      if (group === 'class') {
        const owners = c.classes.filter(cl => Array.isArray(cl.classFeatures) && cl.classFeatures.some(f => f?.definition?.id===m.componentId));
        const owner=owners.length===1?owners[0]:null;
        const feature = owner?.classFeatures.find(f => f?.definition?.id===m.componentId)?.definition;
        if (!owner || !integer(feature.requiredLevel,1,20)) { block(`Origine di classe non risolta: ${m.subType}.`); continue; }
        if (feature.requiredLevel>owner.level) continue;
        classLevel = owner.level;
      }
      if (m.subType==='choose-an-ability-score') {
        const featChoice = (Array.isArray(c.choices?.class)?c.choices.class:[]).some(choice => choice?.componentId===m.componentId && choice.type===1 && choice.optionValue===696);
        if (group==='class' && featChoice) { evidence.push('Avanzamento: talento scelto, nessun incremento di caratteristica da questa opzione.'); continue; }
        block('Scelta di incremento caratteristica non ancora risolta.'); continue;
      }
      if (group==='item') {
        const owners = inventorySource.filter(item => Array.isArray(item?.definition?.grantedModifiers) && item.definition.grantedModifiers.some(grant => String(grant?.id)===String(m.id)));
        if (owners.length!==1) { block(`Origine oggetto non univoca: ${m.subType}.`); continue; }
        const item=owners[0];
        if(item.quantity===0||item.equipped===false||item.definition.canAttune===true&&item.isAttuned===false)continue;
        if(!integer(item.quantity,1,999)||item.equipped!==true||item.definition.canAttune&&item.isAttuned!==true){block(`Equipaggiamento o sintonia non risolti: ${m.subType}.`);continue;}
      }
      const chosen=m.isGranted===false&&selectedAbilityBonus(c,group,m);
      if(chosen)evidence.push(`Aumento ${m.subType}: scelta esplicita e opzione concordante, +${m.value}.`);
      if (!['race','class','background','feat','item','condition'].includes(group) ||
          (m.isGranted===false&&!chosen) || (!abilityFloorRestriction(m)&&(typeof m.restriction==='string' ? m.restriction.trim() : m.restriction!=null))) {
        block(`Effetto sui valori principali non risolto: ${m.type} ${m.subType}.`); continue;
      }
      active.push({...m, group, classLevel});
    }
  }
  const used = new Set();
  const sums = (subType) => active.reduce((sum,m) => {
    if (m.type!=='bonus' || m.subType!==subType) return sum;
    if (!integer(m.value,-1000,1000) || m.statId!=null) { block(`Bonus non numerico o dipendente da caratteristica: ${subType}.`); return sum; }
    used.add(m);
    return sum + m.value;
  },0);
  reconcileAbilities(report.abilities,active,{block,evidence,used,inventory:inventorySource});
  let perLevel = 0;
  for (const m of active) {
    if (m.type==='bonus' && m.subType==='hit-points-per-level' && integer(m.value,-1000,1000) && m.statId==null) {
      used.add(m); perLevel += m.value*m.classLevel;
    }
  }
  const flatHp = sums('hit-points');
  report.hp=beyondHitPointInputs(c);
  if (!report.hp) block('PF mancanti o incoerenti.');
  else {
    const hp = report.hp;
    hp.constitutionContribution = mod(report.abilities.COS.preview)*report.character.level;
    hp.modifierContribution = perLevel+flatHp;
    // Beyond's override replaces the computed maximum, including bonuses.
    hp.previewMax = hp.override ?? hp.base+hp.constitutionContribution+hp.bonus+hp.modifierContribution;
    hp.previewCurrent = Math.max(0,hp.previewMax-hp.removed);
    if (!integer(hp.previewMax,1,999)) block('PF massimi fuori dai limiti locali.');
    evidence.push(`PF: ${hp.override!==null?`override ${hp.override}`:`base ${hp.base} + COS ${hp.constitutionContribution} + bonus ${hp.bonus} + effetti ${hp.modifierContribution}`} = ${hp.previewMax}; danni ${hp.removed}, attuali ${hp.previewCurrent}.`);
  }
  const dex = mod(report.abilities.DES.preview);
  const equippedArmor = inventorySource.filter(item => item?.equipped && item.quantity>0 && item.definition?.filterType==='Armor');
  const armor = equippedArmor.filter(item => item.definition.armorTypeId!==4);
  const shields = equippedArmor.filter(item => item.definition.armorTypeId===4);
  if (armor.length>1 || shields.length>1) block('Più armature o scudi equipaggiati: CA ambigua.');
  let ac = 10+dex, formula = `10 + DES ${dex}`;
  if (armor.length===1) {
    const d = armor[0].definition;
    if (![1,2,3].includes(d.armorTypeId) || !integer(d.armorClass,1,30) || d.canAttune || d.isHomebrew) block('Armatura non supportata per il calcolo CA.');
    else { const contribution = d.armorTypeId===1?dex:d.armorTypeId===2?Math.min(2,dex):0; ac=d.armorClass+contribution; formula=`armatura ${d.armorClass} + DES applicata ${contribution}`; }
  }
  for (const m of active) {
    if (m.type==='set' && m.subType==='unarmored-armor-class' && [3,5].includes(m.statId) && m.value==null) {
      used.add(m);
      if (!armor.length && !(m.statId===5 && shields.length)) {
        const extra = mod(report.abilities[keys[m.statId-1]].preview), candidate=10+dex+extra;
        if (candidate>ac) { ac=candidate; formula=`10 + DES ${dex} + ${keys[m.statId-1]} ${extra}`; }
      }
    }
  }
  if (shields.length===1) {
    const d=shields[0].definition;
    if (!integer(d.armorClass,1,10) || d.canAttune || d.isHomebrew) block('Scudo non supportato per il calcolo CA.');
    else { ac+=d.armorClass; formula+=` + scudo ${d.armorClass}`; }
  }
  const acBonus=sums('armor-class'); ac+=acBonus; if(acBonus) formula+=` + bonus ${acBonus}`;
  for (const m of active) if (!used.has(m)) block(`Modificatore principale non supportato: ${m.type} ${m.subType}.`);
  if (!integer(ac,1,30)) block('CA fuori dai limiti locali.');
  report.armorClass=ac;
  evidence.push(`CA: ${formula} = ${ac}.`);
  if (inventorySource.length>60) block('Inventario oltre 60 voci: importazione bloccata senza tagli.');
  if ((c.customItems?.length ?? 0)>0) block('Oggetti personalizzati non ancora convertibili.');
  const inventory = inventorySource.map((item,index) => {
    const name=item?.definition?.name;
    if (typeof name!=='string' || !name.trim() || name.length>40 || !integer(item?.quantity,0,999)) block('Nome o quantità di un oggetto non rappresentabile (40 caratteri, 0–999).');
    return {id:`beyond-item-${index+1}`, type:'gear', name:typeof name==='string'?name:'?', quantity:item?.quantity,
      notes:`Beyond: ${item?.pocketddEquipmentUnknown?'equipaggiamento non disponibile':item?.equipped?'equipaggiato':'non equipaggiato'}${item?.isAttuned?' · in sintonia':''}. ${item?.definition?.filterType==='Weapon'?'Arma: attacco e danni da configurare.':'Effetti da gestire manualmente.'}${item?.pocketddNote?' '+item.pocketddNote:''}`.slice(0,400)};
  });
  if (report.character.name.length>24) block('Nome oltre 24 caratteri: scegliere un nome locale in una futura versione.');
  evidence.push(`Inventario: ${inventorySource.length} voci distinte, nomi e quantità conservati; equipaggiamento annotato, nessuna arma attivata.`);
  const omissions=[...BEYOND_LIMITATIONS,...normalization.omissions];
  if (report.hp?.temporary) omissions.push(`PF temporanei sorgente: ${report.hp.temporary}; non inclusi nei PF attuali.`);
  const featureNames = (c.feats ?? []).map(f=>f?.definition?.name).filter(n=>typeof n==='string').map(n=>n.slice(0,100));
  if (featureNames.length) omissions.push(`Talenti da gestire manualmente: ${featureNames.slice(0,3).join(', ')}${featureNames.length>3?` (totale ${featureNames.length})`:''}.`);
  let draft=null;
  if (!blockers.length) {
    const base=initialState();
    draft=parseBackup(serialize({...base,name:report.character.name,title:'Da D&D Beyond · import parziale',level:report.character.level,
      abilities:Object.fromEntries(keys.map(k=>[k,report.abilities[k].preview])),hp:report.hp.previewCurrent,maxHp:report.hp.previewMax,armorClass:ac,
      inventory,equippedWeaponId:null,bonus:0,resolve:0,condition:false,history:[],
      importInfo:{provider:'dndbeyond',characterId:report.character.id,scope:'core-v1',limitations:omissions}}));
  }
  return {...report,version:2,checks:{http:'reported-by-transport',format:'valid',conversion:blockers.length?'blocked':'supported-core'},
    conversion:{scope:'core-v1',complete:false,sourceCounts:normalization.sourceCounts,blockers,evidence:[...new Set(evidence)],omissions},draft,
    canImport:draft!==null,warnings:[...blockers,...omissions]};
}

export function convertBeyondCore(payload,expectedId){const n=normalizeBeyondSource(payload);return convertCoreNormalized(n.payload,expectedId,n);}
export function convertBeyond(payload,expectedId){const n=normalizeBeyondSource(payload);return addBeyondAppearance(extendBeyond(convertCoreNormalized(n.payload,expectedId,n,true),n.payload.data,n.training),n.payload.data);}

// Full first-version import. The historical bounded converter remains available for
// adapters/tests that explicitly need the old projection without reference pages.
export function convertBeyondFull(payload,expectedId){const n=normalizeBeyondSource(payload);return finalizeBeyondReport(addBeyondReference(convertBeyond(payload,expectedId),n.payload.data));}
