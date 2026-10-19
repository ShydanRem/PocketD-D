export function openBeyondImport({commit}) {
  const dialog=document.createElement('dialog');
  dialog.className='editor-dialog beyond-import';
  dialog.setAttribute('aria-label','Importa da D&D Beyond');
  dialog.innerHTML=`<div class="editor-heading"><h2>Importa da D&D Beyond</h2><button type="button" data-close aria-label="Chiudi">×</button></div>
    <p>Leggi un personaggio pubblico. L’anteprima non modifica la raccolta; la conferma aggiunge una nuova scheda parziale.</p>
    <p>Da PC: avvia PocketD-D con <code>npm start</code>. Usa sempre lo stesso indirizzo locale per ritrovare la raccolta di questo browser.</p>
    <form data-read><label>Link o ID del personaggio<input name="character" required maxlength="500" autocomplete="off" placeholder="https://www.dndbeyond.com/characters/…"></label><button type="submit">Leggi e verifica</button></form>
    <p data-status role="status" aria-live="polite"></p>
    <section data-preview hidden><h3 data-name></h3><p data-checks></p><dl data-values></dl>
      <h4>Verifica dei calcoli</h4><ul data-evidence></ul>
      <h4>Inventario sorgente</h4><ul data-items></ul>
      <h4>Limiti e dati da completare</h4><ul data-omissions></ul><ul data-blockers role="alert"></ul>
      <label class="checkbox-label"><input type="checkbox" data-accept>Ho letto le omissioni e accetto di creare una scheda parziale.</label>
      <button type="button" data-create disabled>Crea nuovo personaggio</button>
    </section>`;
  document.body.append(dialog);
  const $=s=>dialog.querySelector(s);
  let report=null, request=null, generation=0, creating=false;
  const list=(selector,values)=>{const root=$(selector);root.replaceChildren(...values.map(value=>{const li=document.createElement('li');li.textContent=value;return li}));};
  const clear=()=>{generation++;request?.abort();report=null;$('[data-preview]').hidden=true;$('[data-accept]').checked=false;$('[data-create]').disabled=true;$('[data-read] button').disabled=false;};
  const close=()=>dialog.close();
  $('[data-close]').onclick=close;
  dialog.addEventListener('close',()=>{clear();dialog.remove()},{once:true});
  $('[name=character]').addEventListener('input',()=>{clear();$('[data-status]').textContent='';});
  $('[data-read]').onsubmit=async event=>{
    event.preventDefault();clear();const token=generation;
    request=new AbortController();$('[data-read] button').disabled=true;$('[data-status]').textContent='Lettura e verifica in corso…';
    try{
      if(!['http:','https:'].includes(location.protocol))throw new Error('La lettura Beyond richiede l’app PC: esegui npm run build e npm start, poi apri l’indirizzo indicato.');
      const response=await fetch('/api/probe',{method:'POST',credentials:'omit',signal:request.signal,headers:{'Content-Type':'application/json'},body:JSON.stringify({character:$('[name=character]').value})});
      if(!response.headers.get('content-type')?.includes('application/json'))throw new Error('Connettore PC non disponibile. Avvia l’app con npm start.');
      const body=await response.json();
      if(token!==generation || !dialog.open)return;
      if(!response.ok || !body.ok)throw new Error(`${body.error?.code??'ERROR'} · ${body.error?.message??'Lettura non riuscita'}`);
      report=body.report;
      $('[data-name]').textContent=`${report.character.name} · livello ${report.character.level}`;
      $('[data-checks]').textContent=`${report.testFixture?'DATI SINTETICI DI TEST · ':''}HTTP ${report.transport.status} · formato valido · ${report.canImport?'conversione dei dati principali supportata; scheda parziale':'conversione bloccata'}`;
      const values=$('[data-values]');values.replaceChildren();
      for(const [label,value] of [...Object.entries(report.abilities).map(([k,v])=>[k,v.preview]),['PF',report.hp?`${report.hp.previewCurrent}/${report.hp.previewMax}`:'non risolti'],['CA',report.armorClass??'non risolta']]){
        const dt=document.createElement('dt'),dd=document.createElement('dd');dt.textContent=label;dd.textContent=String(value);values.append(dt,dd);
      }
      list('[data-evidence]',report.conversion?.evidence??[]);
      list('[data-items]',report.inventory.items.map(i=>`${i.name} ×${i.quantity??'?'} · ${i.equipped?'equipaggiato':'non equipaggiato'}`));
      list('[data-omissions]',report.conversion?.omissions??report.warnings);
      list('[data-blockers]',report.conversion?.blockers??['Questa versione del connettore non supporta la creazione.']);
      $('[data-preview]').hidden=false;
      $('[data-accept]').disabled=!report.canImport;
      $('[data-status]').textContent=report.canImport?'Anteprima pronta. Controlla valori e omissioni prima di confermare.':'Nessuna scheda creata: risolvere i blocchi indicati.';
    }catch(error){if(token===generation && error.name!=='AbortError')$('[data-status]').textContent=error.message;}
    finally{if(token===generation)$('[data-read] button').disabled=false;}
  };
  $('[data-accept]').onchange=()=>{$('[data-create]').disabled=!report?.canImport || !$('[data-accept]').checked;};
  $('[data-create]').onclick=()=>{
    if(creating || !report?.canImport || !report.draft || !$('[data-accept]').checked)return;
    if(!confirm(`Creare ${report.character.name} come nuovo personaggio parziale? Le schede esistenti restano conservate.`))return;
    creating=true;$('[data-create]').disabled=true;
    try{commit(report.draft);close();}
    catch(error){$('[data-status]').textContent=`Importazione non salvata: ${error.message}. La raccolta precedente resta conservata.`;creating=false;$('[data-create]').disabled=false;}
  };
  dialog.showModal();
}

export function showBeyondLimitations(info){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog beyond-import';
  const title=document.createElement('h2');title.textContent='Scheda parziale da D&D Beyond';
  const list=document.createElement('ul');for(const text of info.limitations){const li=document.createElement('li');li.textContent=text;list.append(li);}
  const close=document.createElement('button');close.textContent='Chiudi';close.onclick=()=>dialog.close();
  dialog.append(title,list,close);document.body.append(dialog);dialog.addEventListener('close',()=>dialog.remove(),{once:true});dialog.showModal();
}
