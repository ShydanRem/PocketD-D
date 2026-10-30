import {ACCOUNT_ORIGINS, checkAccount} from './account-check.js';
const find = id => document.getElementById(id);
let controller, generation = 0;
const labels = {authHTTP:'Accesso HTTP al login',authJSON:'Login: formato JSON',session:'Token: identità e scadenza',listHTTP:'Accesso HTTP all’elenco',listJSON:'Elenco: formato JSON',catalog:'Elenco: struttura e dati minimi'};
function reset() {
  generation++; controller?.abort(); controller = undefined;
  find('check').disabled = false;
  find('checks').replaceChildren(); find('characters').replaceChildren(); find('catalog').hidden = true;
}
find('clear').onclick = () => {reset();find('status').textContent='Risultato cancellato. Nessun dato salvato; la verifica non ha modificato i PG.';};
find('check').onclick = async () => {
  reset(); const run = generation; controller = new AbortController(); const signal = controller.signal;
  find('check').disabled = true;find('status').textContent='Verifica in corso…';
  try {
    // Permission request is directly tied to this click, never to content-script ticks.
    const granted = await chrome.permissions.request({origins:ACCOUNT_ORIGINS});
    if (run !== generation) return;
    if (!granted) {find('status').textContent='Permessi non concessi. Nessuna richiesta Beyond eseguita.';return;}
    const result = await checkAccount({signal});
    if (run !== generation) return;
    for (const [key, label] of Object.entries(labels)) {
      const term=document.createElement('dt'), value=document.createElement('dd');
      term.textContent=label;value.textContent=result.checks[key]===null?'Da fare':result.checks[key]?'OK':'Fallito';
      find('checks').append(term,value);
    }
    find('status').textContent = result.ok ? 'Login ed elenco letti. Conversione e importazione sul Pocket ancora da realizzare.'
      : result.message + (result.status[result.stage] ? ' HTTP '+result.status[result.stage]+'.' : '');
    if (!result.ok) return;
    find('catalog').hidden=false;
    find('completeness').textContent = result.completeness === 'partial'
      ? 'Elenco parziale segnalato dal servizio. Non sono stati caricati tutti i personaggi.'
      : `${result.characters.length} personaggi letti. Confrontali con «I miei personaggi» su Beyond: la completezza deve essere confermata.`;
    for (const pg of result.characters) {
      const item=document.createElement('li'), name=document.createElement('strong'), detail=document.createElement('span');
      name.textContent=pg.name;detail.textContent=[pg.level===null?'':`Livello ${pg.level}`,pg.race,pg.class].filter(Boolean).join(' · ');
      item.append(name,detail);find('characters').append(item);
    }
  } catch { if(run===generation)find('status').textContent='Verifica non disponibile. Riprova dall’estensione PocketD-D in Chrome o Edge.'; }
  finally { if(run===generation){controller=undefined;find('check').disabled=false;} }
};
find('revoke').onclick = async () => {
  reset();const run=generation;
  try {
    const removed=await chrome.permissions.remove({origins:ACCOUNT_ORIGINS});
    if(run===generation)find('status').textContent=removed?'Permessi di verifica rimossi. Il login Beyond del browser è conservato.':'Il browser non ha rimosso i permessi. Controlla la gestione estensioni.';
  } catch { if(run===generation)find('status').textContent='Apri la gestione estensioni del browser per rimuovere i permessi.'; }
};
window.addEventListener('pagehide',()=>controller?.abort());
