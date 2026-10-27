import {avatarOutfit} from './avatar-classes.js';
import {avatarIdentity} from './avatar-species.js';
import {abilityKeys,skills,parseBackup,serialize} from './model.js';
export const DEVICE_MAX_RESOURCES=8;
const ascii=(value,max)=>String(value).normalize('NFKD').replace(/[\u0300-\u036f]/g,'').replace(/[^\x20-\x7e]/g,'?').trim().slice(0,max)||'Eroe';
const integer=(n,min,max,label)=>{if(!Number.isInteger(n)||n<min||n>max)throw new Error(`${label}: fuori dai limiti del firmware (${min}–${max}).`);return n};
export function projectDevice(input){
  const s=parseBackup(serialize(input));
  const warnings=[
    'Firmware iniziale: una sola scheda attiva. La conferma sul display sostituisce la copia sul dispositivo; la raccolta PC resta conservata.',
    'Trasferisce nome, livello, aspetto dell’avatar, PF, PF temporanei, CA, caratteristiche, abilità, salvezze, inventario (60 voci), attacchi, magie/capacità, dadi vita, condizioni e contatori morte e fino a 8 contatori manuali. Non trasferisce la risposta originale di Beyond.',
    'Avatar originale: famiglia visiva, silhouette maschile/femminile, carnagione, capelli e mantello modificabili sul PC o sul dispositivo. Non rappresenta equipaggiamento o immagini di Beyond. Animazione disattivabile dal dispositivo in Aspetto.',
    'Non disponibili sul dispositivo: gestione completa degli effetti delle condizioni, mappa e party. Lo storico locale degli ultimi 20 tiri richiede firmware v0.52 ed è incluso nel backup nativo dalla v0.53.',
    'Prove con competenze, maestrie, bonus extra e mezza competenza riconosciuti dal PC. Le prove da verificare restano bloccate; per eccezioni del DM è disponibile il dado libero.',
    'Avvelenato e vantaggio FOR da Ira sono fotografati al trasferimento. Ira riconosciuta attivabile dalla capacità; altre condizioni e durate restano manuali. Effetti PC disattivabili per il singolo tiro; altri effetti restano da gestire con il DM.',
    'CA fotografata al momento dell’esportazione; non ricalcolata cambiando equipaggiamento. I contatori non applicano recuperi o effetti automatici.',
    'Nomi limitati ad ASCII: accenti traslitterati, altri simboli sostituiti; nome massimo 48 caratteri, contatori 32.'
  ];
  // Structured resources belong to magic, never to duplicated manual counters.
  const counters=!s.importInfo?[{name:'Slancio',current:s.resolve,maximum:1}]:[];
  const map=(names,source)=>Object.fromEntries(names.map(k=>[k,integer(source?.[k]??0,-30,30,k)]));
  const training={skillRanks:{...s.skillRanks},saveProficiencies:{...s.saveProficiencies},
    skillBonuses:map(skills.map(k=>k.id),s.extended?.skillBonuses),saveBonuses:map(abilityKeys,s.extended?.saveBonuses),checkBonuses:map(abilityKeys,s.extended?.checkBonuses),
    halfAbilityChecks:s.extended?.halfAbilityChecks??false,halfSkills:[...(s.extended?.halfSkills??[])],blockedChecks:[...(s.extended?.blockedChecks??[])],
    poisoned:s.condition,strengthAdvantage:Boolean(s.extended?.combat.raging)};
  const inventory={items:s.inventory.map(v=>({name:ascii(v.name,40),damageType:v.damageType?ascii(v.damageType,24):'',quantity:v.quantity,charges:{current:v.charges?.current??0,maximum:v.charges?.max??0,recovery:['manual','short','long'].indexOf(v.charges?.recovery??'manual')},
    kind:['gear','consumable','healing','weapon'].indexOf(v.type),ability:v.type==='weapon'?abilityKeys.indexOf(v.ability):0,
    count:v.diceCount??0,sides:v.dieSides??0,flags:(v.proficient?1:0)|(v.addAbilityToDamage?2:0)|(v.profile?.ranged?4:0)|(v.profile?.heavy?8:0),
    reach:v.reach??0,thrownRange:v.profile?.thrownRange??0,attackModifier:v.attackModifier??0,damageModifier:v.damageModifier??0,healingModifier:v.healingModifier??0})),
    equipped:s.equippedWeaponId?s.inventory.findIndex(v=>v.id===s.equippedWeaponId)+1:0,
    combat:{rageBonus:s.extended?.combat.rageBonus??0,criticalDice:s.extended?.combat.extraMeleeCriticalDice??0,
      raging:s.extended?.combat.raging??false,powerAttack:Boolean(s.extended?.combat.greatWeaponMaster&&s.extended?.combat.greatWeaponMasterAvailable),thrown:s.extended?.combat.thrown??false}};
  warnings.push('Zaino: nomi, quantità, armi e cure riconosciute dal PC. Note e proprietà descrittive restano sul PC; gli oggetti generici non acquisiscono effetti automatici.');
  warnings.push('Cariche: un saldo esplicito per voce inventario, distinto dalla quantità. Configurazione manuale sul PC, correzione e spesa sul dispositivo; recupero manuale o pieno da riposo breve/lungo. Nessuna carica dedotta automaticamente da Beyond o dalle note.');
  warnings.push('Attacchi: bonus e danni secondo il modello PC, avvelenato, Ira, attacco potente e critico extra fotografati al trasferimento. CA bersaglio manuale. Nessun consumo automatico di munizioni, resistenze, danni aggiuntivi o regole speciali dalle note.');
  warnings.push('Cambiare arma non ricalcola CA o avatar. Cure e consumabili richiedono conferma; Annulla sull’Eroe ripristina PF e quantità insieme. I progressi possono essere conservati come backup nativo USB; la raccolta PC non viene sincronizzata automaticamente.');
  const magic={powers:s.powers.map(p=>{
    const description=String(p.description).normalize('NFKD').replace(/[\u0300-\u036f]/g,'').replace(/\s+/g,' ').replace(/[^\x20-\x7e]/g,'?').trim();
    const excerpt=description.length>500;
    if(excerpt)warnings.push(`Descrizione in estratto (500 caratteri): ${p.name}. Testo completo conservato sul PC.`);
    const uses=p.type==='feature'?p:p.limited;
    return {name:ascii(p.name,40),description:description.slice(0,500),kind:p.type==='feature'?1:0,level:p.level??0,
      flags:(p.concentration?1:0)|(p.prepared!==false?2:0)|(p.usesSpellSlot!==false?4:0)|(p.limited?8:0)|(p.passive?16:0)|(excerpt?32:0),
      ability:abilityKeys.indexOf(p.castingAbility??s.spellcastingAbility),maximum:uses?.maxUses??0,remaining:uses?.remaining??0,
      recovery:['manual','short','long'].indexOf(uses?.recovery??'manual'),effect:p.effect==='rage'?1:p.effect==='relentless-endurance'?2:0,
      attackModifier:p.attackModifier??0,saveModifier:p.saveModifier??0};
  }),slots:s.slots.map(v=>({current:v.current,maximum:v.max})),pact:(s.extended?.pactSlots??Array.from({length:9},()=>({current:0,max:0}))).map(v=>({current:v.current,maximum:v.max})),
    concentration:s.concentration?s.powers.findIndex(p=>p.id===s.concentration)+1:0};
  warnings.push('Magie e capacità: fino a 120 voci, descrizioni ASCII fino a 500 caratteri con estratti segnalati, slot ordinari/patto, utilizzi innati, preparazione e concentrazione. Consumo solo dopo conferma; effetti, danni e salvezze delle magie da risolvere con il DM.');
  const recovery={available:Boolean(s.extended),pools:(s.extended?.hitDice??[]).map(h=>({name:h.name.trim()?ascii(h.name,40):'Classe senza nome',die:h.die,maximum:h.max,used:h.used}))};
  warnings.push('Riposi con anteprima e conferma: patto e risorse con recupero breve/lungo riconosciuto. Dadi vita importati senza inventare riserve; recupero lungo 2014, 2024 o manuale scelto sul dispositivo. Requisiti, eccezioni e condizioni da verificare con il DM. Contatori generici manuali.');
  warnings.push('Dadi vita: nomi ASCII fino a 40 caratteri, d6/d8/d10/d12 e utilizzi per classe. Spesa singola durante il riposo breve con cura base dado + COS, minimo zero; bonus speciali e regole opzionali manuali. Tasti configurabili; la copia conserva le preferenze locali.');
  const status={available:Boolean(s.extended),successes:s.extended?.deathSaves.successes??0,failures:s.extended?.deathSaves.failures??0,stabilized:s.extended?.deathSaves.stabilized??false,
    conditions:(s.extended?.conditions??[]).map(c=>({name:ascii(c.name,60),level:c.level}))};
  warnings.push('Stato: fino a 30 condizioni come annotazioni con livello 0–6; nessun effetto dedotto dal nome. Avvelenato è un effetto separato e configurabile sul dispositivo. Rimuovere una nota non disattiva effetti.');
  warnings.push('Morte: contatori trasferiti esattamente; d20 base senza bonus, vantaggi o talenti, con conferma e correzioni DM. Tre fallimenti a zero PF bloccano cure ordinarie e riposo lungo fino a correzione esplicita. Nessuna morte istantanea o gestione automatica dei danni a zero PF.');
  const wallet={available:s.extended?.currenciesAvailable??false,...Object.fromEntries(['cp','sp','ep','gp','pp'].map(k=>[k,s.extended?.currenciesAvailable?integer(s.extended.currencies[k],0,999999999,k):0]))};
  warnings.push(wallet.available?'Monete: cinque saldi verificati, separati e modificabili con conferma sul dispositivo. Nessun cambio o acquisto automatico.':'Monete: saldo non verificato, nessuna ricchezza inventata. Verificalo in Stato e risorse sul PC oppure configura il borsello sul dispositivo.');
  const identity=avatarIdentity(s.avatar),outfit=avatarOutfit(s.avatar),variant=outfit||identity.lineage||identity.body;
  const packet={format:'pocketdd-device',version:outfit?13:identity.lineage>=16?12:identity.lineage>=12?11:variant?10:9,name:ascii(s.name,48),level:integer(s.level,1,20,'Livello'),hp:integer(s.hp,0,999,'PF'),
    maxHp:integer(s.maxHp,1,999,'PF massimi'),tempHp:integer(s.extended?.tempHp??0,0,999,'PF temporanei'),armorClass:integer(s.armorClass,1,40,'CA'),
    avatar:Object.fromEntries(['skin','hair','cloak','style'].map(k=>[k,integer(s.avatar[k],0,2,k)])),
    training,inventory,magic,recovery,status,wallet,
    abilities:Object.fromEntries(abilityKeys.map(k=>[k,integer(s.abilities[k],1,30,k)])),
    resources:counters.slice(0,DEVICE_MAX_RESOURCES).map(r=>({name:ascii(r.name,32),current:integer(r.current,0,999,r.name),maximum:integer(r.maximum,1,999,r.name)}))};
  if(outfit){packet.avatar.outfit=outfit;warnings.push('Look classe: richiede firmware 0.70 / USB13 e pocketdd/classes.pdc sulla SD. Senza asset viene mostrato lo stile originale, mantenendo la scelta. Colori e taglio del preset sono fissi; nessun valore di gioco cambia.');}
  if(variant){Object.assign(packet.avatar,identity);warnings.push(identity.lineage>=16?'Questa specie richiede USB12 / firmware 0.69 o successivo. Aspetto cosmetico: nessun bonus o equipaggiamento dedotto.':identity.lineage>=12?'Questa specie richiede USB11 / firmware 0.68 o successivo. Aspetto cosmetico: nessun bonus o equipaggiamento dedotto.':'Specie e silhouette maschile/femminile richiedono USB10 / firmware 0.67 o successivo. Aspetto cosmetico: nessun bonus o equipaggiamento dedotto.');}
  if(serializeDevice(packet).length>262144)throw new Error('Copia oltre 262144 byte: nessun inventario troncato.');
  return {packet,warnings};
}
export const serializeDevice=packet=>JSON.stringify(packet);
