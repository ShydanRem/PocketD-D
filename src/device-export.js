import {abilityKeys,parseBackup,serialize} from './model.js';
export const DEVICE_MAX_RESOURCES=8;
const ascii=(value,max)=>String(value).normalize('NFKD').replace(/[\u0300-\u036f]/g,'').replace(/[^\x20-\x7e]/g,'?').trim().slice(0,max)||'Eroe';
const integer=(n,min,max,label)=>{if(!Number.isInteger(n)||n<min||n>max)throw new Error(`${label}: fuori dai limiti del firmware (${min}–${max}).`);return n};
export function projectDevice(input){
  const s=parseBackup(serialize(input));
  const warnings=[
    'Firmware iniziale: una sola scheda attiva. La conferma sul display sostituisce la copia sul dispositivo; la raccolta PC resta conservata.',
    'Trasferisce nome, livello, aspetto dell’avatar, PF, PF temporanei, CA, sei caratteristiche e fino a 8 contatori manuali. Non trasferisce la risposta originale di Beyond.',
    'Avatar originale: carnagione, colore/taglio dei capelli e mantello scelti sul PC. Non rappresenta equipaggiamento o immagini di Beyond. Animazione disattivabile dal dispositivo in Aspetto.',
    'Non disponibili sul dispositivo: inventario, armi e attacchi, magie, abilità e salvezze con competenze, condizioni, effetti, tiri salvezza contro morte, dadi vita, monete, mappa, party e storico.',
    'Le prove sul dispositivo usano solo il modificatore della caratteristica: non applicano competenze, bonus extra o effetti. Vantaggio e svantaggio si scelgono manualmente.',
    'CA fotografata al momento dell’esportazione; non ricalcolata cambiando equipaggiamento. I contatori non applicano recuperi o effetti automatici.',
    'Nomi limitati ad ASCII: accenti traslitterati, altri simboli sostituiti; nome massimo 48 caratteri, contatori 32.'
  ];
  const counters=[...(!s.importInfo?[{name:'Slancio',current:s.resolve,maximum:1}]:[]),
    ...s.powers.filter(p=>p.type==='feature'&&!p.passive&&p.maxUses>0).map(p=>({name:p.name,current:p.remaining,maximum:p.maxUses})),
    ...s.slots.flatMap((slot,i)=>slot.max>0?[{name:`Slot livello ${i+1}`,current:slot.current,maximum:slot.max}]:[]),
    ...(s.extended?.pactSlots??[]).flatMap((slot,i)=>slot.max>0?[{name:`Patto livello ${i+1}`,current:slot.current,maximum:slot.max}]:[])];
  if(counters.length>DEVICE_MAX_RESOURCES)warnings.push(`Contatori esclusi per limite di 8: ${counters.slice(DEVICE_MAX_RESOURCES).map(r=>r.name).join(', ')}.`);
  const packet={format:'pocketdd-device',version:2,name:ascii(s.name,48),level:integer(s.level,1,20,'Livello'),hp:integer(s.hp,0,999,'PF'),
    maxHp:integer(s.maxHp,1,999,'PF massimi'),tempHp:integer(s.extended?.tempHp??0,0,999,'PF temporanei'),armorClass:integer(s.armorClass,1,40,'CA'),
    avatar:Object.fromEntries(['skin','hair','cloak','style'].map(k=>[k,integer(s.avatar[k],0,2,k)])),
    abilities:Object.fromEntries(abilityKeys.map(k=>[k,integer(s.abilities[k],1,30,k)])),
    resources:counters.slice(0,DEVICE_MAX_RESOURCES).map(r=>({name:ascii(r.name,32),current:integer(r.current,0,999,r.name),maximum:integer(r.maximum,1,999,r.name)}))};
  return {packet,warnings};
}
export const serializeDevice=packet=>JSON.stringify(packet);
