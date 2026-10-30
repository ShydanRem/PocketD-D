import {serialize,parseBackup} from './model.js';
// v1 means a bounded offline sheet, not an evaluator for every D&D rule.
export function finalizeBeyondReport(report){
 if(!report.draft)return {...report,readiness:{state:'blocked',version:1,manual:[],snapshots:[],unresolved:report.conversion?.blockers??[]}};
 const s=structuredClone(report.draft),x=s.extended;
 const generic=[/^Effetti condizionali, armi versatili/,/^Condizioni, dadi vita e tiri morte sono/,/^Difese ai danni: importate solo/,/^Cariche: un saldo per oggetto/,/^CA e PF massimi sono una fotografia/];
 const specific=[...new Set(report.conversion.omissions.filter(t=>!generic.some(re=>re.test(t))))];
 const excerpts=specific.filter(t=>/estratto/.test(t)),unresolved=specific.filter(t=>!excerpts.includes(t));
 const manual=[];
 const notePowers=s.powers.filter(p=>p.reference===undefined&&p.type==='feature'&&p.passive);
 if(notePowers.length)manual.push(`${notePowers.length} capacità/azioni conservate come note: gli effetti si risolvono al tavolo.`);
 if(s.powers.some(p=>['hex-2024','hunters-mark-2024'].includes(p.effect)))manual.push('Hex/Marchio 2024: danni base su bersaglio confermato e concentrazione attiva; durata, azioni, capacità che potenziano il dado e altri effetti sul bersaglio restano manuali.');
 const weaponSpells=s.powers.filter(p=>p.effect==='true-strike-2024');if(weaponSpells.length)manual.push('True Strike 2024 operativo con arma e caratteristica da incantatore; altre proprietà speciali dell arma da verificare.');
 const spells=s.powers.filter(p=>p.type==='spell'&&p.effect!=='true-strike-2024');if(spells.length)manual.push(`${spells.length} magie: preparazione, slot e utilizzi gestiti; danni, bersagli ed effetti non eseguiti automaticamente.`);
 if(x.manualResources?.length)manual.push(`${x.manualResources.length} contatori manuali distinti dagli utilizzi: nessun collegamento o recupero dedotto.`);
 const unknownDefences=(report.details?.defences??[]).filter(d=>d.flags===null).map(d=>d.name);
 if(unknownDefences.length)manual.push(`Difese ai danni: verificare ${unknownDefences.join(', ')}. Non sono dichiarate Normali.`);
 if(x.conditions.length)manual.push(`${x.conditions.length} condizioni annotate. Gli effetti di tiro del dispositivo si impostano separatamente; Avvelenato è riconciliato.`);
 if(x.hitDice.length)manual.push('Riposo lungo: scegli la politica 2014/2024/manuale dei dadi vita. Recuperi speciali e stabilizzazione richiedono controllo.');
 if(s.inventory.some(i=>i.type==='weapon'))manual.push('Attacchi: versatilità, due armi, maestrie, munizioni e danni aggiuntivi speciali da gestire al tavolo.');
 if(s.inventory.some(i=>i.charges))manual.push('Cariche per voce: recuperi giornalieri/variabili e spese condivise non riconciliate restano manuali.');
 if(s.powers.some(p=>p.effect==='eldritch-cannon'))manual.push('Cannone: creazione gratuita/slot riconciliata; scelta, PF, durata e modalità del cannone restano manuali.');
 const snapshots=[x.loadout?'CA e PF massimi: i profili espliciti aggiornano i valori collegati sul Pocket; effetti non rappresentati e cambi di livello richiedono ricontrollo.':'CA e PF massimi: fotografia sorgente; configura i profili degli oggetti sul Pocket per collegare bonus e formule.','Profilo, movimento base, sensi/passivi, iniziativa e dotazioni: fotografia del sorgente. Non applicano automaticamente carico, terreno, nuove condizioni o cambi di oggetti.'];
 const policy=['Testi conservati in estratto secondo i limiti concordati.','Gioco offline: nessuna scrittura o sincronizzazione con Beyond; nessuna dipendenza da Foundry.'];
 const omissions=[...unresolved,...manual,...snapshots,...excerpts,...policy];
 s.importInfo.limitations=omissions.length<=30?omissions:omissions.slice(0,29).concat(`Altre ${omissions.length-29} eccezioni nel resoconto dell’importazione.`);
 const coverage=report.conversion.coverage.map(row=>row.section==='Difese ai danni'?{...row,status:unknownDefences.length?'partial':'converted'}:row.section==='Dati della scheda'?{...row,status:unresolved.some(t=>/Movimento|Sens[oi]|Iniziativa|Equipaggiamento|Lingua\/strumento/.test(t))?'partial':'snapshot'}:row);
 // Complete retains its original meaning: every source rule is represented.
 // Readiness reports the separate, deliberately narrower first-version contract.
 return {...report,draft:parseBackup(serialize(s)),conversion:{...report.conversion,complete:false,omissions,coverage},warnings:omissions,readiness:{version:1,state:unresolved.length?'review-required':'ready-with-manual-rules',manual,snapshots,excerpts,unresolved,fullRulesAutomation:false}};
}
