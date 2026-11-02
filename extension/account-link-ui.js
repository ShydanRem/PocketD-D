import {preparePocketAccount,pairPocketAccount} from './account-link.js';
const find=id=>document.getElementById(id);let prepared,controller,generation=0;
export function cancelPocketLink(){generation++;controller?.abort();prepared=undefined;find('connect').disabled=true;find('pair-code').textContent='';find('prepare').disabled=false;}
find('prepare').onclick=async()=>{
 cancelPocketLink();const run=generation;controller=new AbortController();find('prepare').disabled=true;find('link-status').textContent='Verifico l’accesso per questo collegamento…';
 try{const access=await preparePocketAccount({signal:controller.signal});if(run!==generation)return;prepared=access;setTimeout(()=>{if(run===generation&&prepared===access){prepared=undefined;access.cookie='';find('connect').disabled=true;find('link-status').textContent='Preparazione scaduta: premi Prepara accesso.';}},Math.max(1,access.expires-Date.now()));find('connect').disabled=false;find('link-status').textContent=`Account verificato (${access.count} PG). Collega il Pocket via USB e premi Collega Pocket entro tre minuti.`;}
 catch{if(run===generation)find('link-status').textContent='Accesso non preparato. Verifica il login Beyond, i permessi dell’estensione e riprova.';}
 finally{if(run===generation)find('prepare').disabled=false;}
};
find('connect').onclick=async()=>{
 const access=prepared;if(!access)return;prepared=undefined;const run=generation;controller=new AbortController();find('connect').disabled=find('prepare').disabled=true;
 try{await pairPocketAccount(access,{signal:controller.signal,onCode:code=>{if(run===generation){find('pair-code').textContent=code;find('link-status').textContent='Confronta il codice sul Pocket, scegli se ricordare l’accesso e conferma sul display.';}},onStage:stage=>{if(run===generation&&stage==='link')find('link-status').textContent='Il Pocket verifica l’account via Wi-Fi. Attendi la conferma.';}});if(run===generation)find('link-status').textContent='Pocket collegato: scegli il PG dal suo display. Il PC può essere scollegato.';}
 catch{if(run===generation)find('link-status').textContent='Collegamento non confermato. Controlla il display del Pocket prima di riprovare.';}
 finally{access.cookie='';if(run===generation){find('prepare').disabled=false;find('pair-code').textContent='';}}
};
find('cancel-link').onclick=()=>{cancelPocketLink();find('link-status').textContent='Operazione interrotta. Se avevi già confermato, controlla l’esito sul Pocket.';};
window.addEventListener('pagehide',cancelPocketLink);
