import {abilityKeys,skills,parseBackup,serialize} from './model.js';
export const DEVICE_MAX_RESOURCES=8;
const ascii=(value,max)=>String(value).normalize('NFKD').replace(/[\u0300-\u036f]/g,'').replace(/[^\x20-\x7e]/g,'?').trim().slice(0,max)||'Eroe';
const integer=(n,min,max,label)=>{if(!Number.isInteger(n)||n<min||n>max)throw new Error(`${label}: fuori dai limiti del firmware (${min}–${max}).`);return n};
export function projectDevice(input){
  const s=parseBackup(serialize(input));
  const warnings=[
    'Firmware iniziale: una sola scheda attiva. La conferma sul display sostituisce la copia sul dispositivo; la raccolta PC resta conservata.',
    'Trasferisce nome, livello, aspetto dell’avatar, PF, PF temporanei, CA, caratteristiche, abilità, salvezze e fino a 8 contatori manuali. Non trasferisce la risposta originale di Beyond.',
    'Avatar originale: carnagione, colore/taglio dei capelli e mantello scelti sul PC. Non rappresenta equipaggiamento o immagini di Beyond. Animazione disattivabile dal dispositivo in Aspetto.',
    'Non disponibili sul dispositivo: inventario, armi e attacchi, magie, gestione completa di condizioni/effetti, tiri salvezza contro morte, dadi vita, monete, mappa, party e storico.',
    'Prove con competenze, maestrie, bonus extra e mezza competenza riconosciuti dal PC. Le prove da verificare restano bloccate; per eccezioni del DM è disponibile il dado libero.',
    'Avvelenato e vantaggio FOR da Ira sono fotografati al trasferimento. Non aggiornano condizioni, durate o utilizzi sul dispositivo. Effetti PC disattivabili per il singolo tiro; altri effetti restano da gestire con il DM.',
    'CA fotografata al momento dell’esportazione; non ricalcolata cambiando equipaggiamento. I contatori non applicano recuperi o effetti automatici.',
    'Nomi limitati ad ASCII: accenti traslitterati, altri simboli sostituiti; nome massimo 48 caratteri, contatori 32.'
  ];
  const counters=[...(!s.importInfo?[{name:'Slancio',current:s.resolve,maximum:1}]:[]),
    ...s.powers.filter(p=>p.type==='feature'&&!p.passive&&p.maxUses>0).map(p=>({name:p.name,current:p.remaining,maximum:p.maxUses})),
    ...s.slots.flatMap((slot,i)=>slot.max>0?[{name:`Slot livello ${i+1}`,current:slot.current,maximum:slot.max}]:[]),
    ...(s.extended?.pactSlots??[]).flatMap((slot,i)=>slot.max>0?[{name:`Patto livello ${i+1}`,current:slot.current,maximum:slot.max}]:[])];
  if(counters.length>DEVICE_MAX_RESOURCES)warnings.push(`Contatori esclusi per limite di 8: ${counters.slice(DEVICE_MAX_RESOURCES).map(r=>r.name).join(', ')}.`);
  const map=(names,source)=>Object.fromEntries(names.map(k=>[k,integer(source?.[k]??0,-30,30,k)]));
  const training={skillRanks:{...s.skillRanks},saveProficiencies:{...s.saveProficiencies},
    skillBonuses:map(skills.map(k=>k.id),s.extended?.skillBonuses),saveBonuses:map(abilityKeys,s.extended?.saveBonuses),checkBonuses:map(abilityKeys,s.extended?.checkBonuses),
    halfAbilityChecks:s.extended?.halfAbilityChecks??false,halfSkills:[...(s.extended?.halfSkills??[])],blockedChecks:[...(s.extended?.blockedChecks??[])],
    poisoned:s.condition,strengthAdvantage:Boolean(s.extended?.combat.raging)};
  const packet={format:'pocketdd-device',version:3,name:ascii(s.name,48),level:integer(s.level,1,20,'Livello'),hp:integer(s.hp,0,999,'PF'),
    maxHp:integer(s.maxHp,1,999,'PF massimi'),tempHp:integer(s.extended?.tempHp??0,0,999,'PF temporanei'),armorClass:integer(s.armorClass,1,40,'CA'),
    avatar:Object.fromEntries(['skin','hair','cloak','style'].map(k=>[k,integer(s.avatar[k],0,2,k)])),
    training,
    abilities:Object.fromEntries(abilityKeys.map(k=>[k,integer(s.abilities[k],1,30,k)])),
    resources:counters.slice(0,DEVICE_MAX_RESOURCES).map(r=>({name:ascii(r.name,32),current:integer(r.current,0,999,r.name),maximum:integer(r.maximum,1,999,r.name)}))};
  return {packet,warnings};
}
export const serializeDevice=packet=>JSON.stringify(packet);
