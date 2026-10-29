import {avatarClasses,avatarOutfit} from './avatar-classes.js';
import {classLooksAvailable} from './class-looks.js';
import {avatarSpecies,avatarBodies,avatarIdentity} from './avatar-species.js';
import {deviceAvatarSvg} from './device-avatar.js';
export function openBeyondImport({commit,guided=false,onCreated}) {
  const dialog=document.createElement('dialog');
  dialog.className='editor-dialog beyond-import';
  dialog.setAttribute('aria-label','Importa da D&D Beyond');
  dialog.innerHTML=`<div class="editor-heading"><h2>Importa da D&D Beyond</h2><button type="button" data-close aria-label="Chiudi">×</button></div>
    <p>Leggi un personaggio pubblico. L’anteprima non modifica la raccolta; la conferma aggiunge una nuova scheda parziale.</p>
    <p>Da Windows: avvia <code>Avvia-PocketDD.cmd</code> dal pacchetto PC estratto e lascia aperta la finestra. Usa sempre lo stesso indirizzo locale per ritrovare la raccolta di questo browser.</p>
    <form data-read><label>Link o ID del personaggio<input name="character" required maxlength="500" autocomplete="off" placeholder="https://www.dndbeyond.com/characters/…"></label><button type="submit">Leggi e verifica</button></form>
    <p data-status role="status" aria-live="polite"></p>
    <section data-preview hidden><h3 data-name></h3><p data-checks></p><dl data-values></dl>
      <section data-appearance><h4>Avatar sul dispositivo</h4><div data-appearance-sprite></div><p data-appearance-status></p><label>Specie · famiglia visiva<select data-lineage></select></label><label>Silhouette<select data-body></select></label><label>Look classe · solo estetica<select data-outfit></select></label><p data-class-status></p><p>Solo aspetto: non cambia caratteristiche, capacità o regole. Puoi modificarlo anche sul dispositivo.</p></section><h4>Verifica dei calcoli</h4><ul data-evidence></ul>
      <details data-readiness><summary>Pronto per la prima versione?</summary><p data-readiness-status></p><ul data-manual></ul><ul data-snapshots></ul></details><details data-reference><summary>Profilo e dotazioni sul dispositivo</summary><ul data-reference-rows></ul></details><h4>Campi compilati</h4><ul data-coverage></ul><ul data-training></ul><ul data-attacks></ul><ul data-powers></ul><ul data-manual-resources></ul>
      <label data-weapon-label hidden>Arma attiva<select data-weapon></select></label>
      <details data-defence-preview><summary>Difese ai danni · tipi verificati e mancanti</summary><p>Solo le concessioni incondizionate riconciliate vengono proposte al dispositivo. Gli altri tipi non sono dichiarati normali.</p><ul data-defences></ul></details><h4>Inventario sorgente e uso locale</h4><ul data-items></ul>
      <h4 data-blocker-title hidden>Problemi che bloccano l’importazione</h4><ul data-blockers role="alert"></ul><h4>Limiti e dati da completare</h4><ul data-omissions></ul>
      <p data-consent-status role="status"></p><label class="checkbox-label" data-consent><input type="checkbox" data-accept disabled>Ho letto le omissioni e accetto di creare una scheda parziale.</label>
      <button type="button" data-create disabled>Crea nuovo personaggio</button>
    </section>`;
  if(guided){
    dialog.classList.add('guided-beyond');
    dialog.querySelectorAll(':scope > p')[1].hidden=true;
    const details=document.createElement('details'),summary=document.createElement('summary');summary.textContent='Calcoli, dati importati e inventario';details.append(summary);
    for(const selector of ['[data-evidence]','[data-reference]','[data-coverage]','[data-training]','[data-attacks]','[data-powers]','[data-manual-resources]','[data-defence-preview]','[data-items]']){const node=dialog.querySelector(selector);if(node.previousElementSibling?.tagName==='H4')details.append(node.previousElementSibling);details.append(node);}
    dialog.querySelector('[data-weapon-label]').before(details);
    const omissions=document.createElement('details'),title=document.createElement('summary');title.textContent='Tutti i limiti e i dati da completare';const omissionList=dialog.querySelector('[data-omissions]'),omissionHeading=omissionList.previousElementSibling;omissions.append(title,omissionList);
    const brief=document.createElement('ul');brief.dataset.guidedLimits='';dialog.querySelector('[data-blockers]').after(omissionHeading,brief,omissions);
    dialog.querySelector('[data-create]').textContent='Crea PG e continua →';
  }
  document.body.append(dialog);
  const $=s=>dialog.querySelector(s);
  let report=null, request=null, generation=0, creating=false;
  const allowed=()=>report?.canImport===true&&Boolean(report.draft)&&Array.isArray(report.conversion?.blockers)&&report.conversion.blockers.length===0;
  // Make real conversion blockers visible before appearance/details in both flows.
  $('[data-values]').after($('[data-blocker-title]'),$('[data-blockers]'));
  const list=(selector,values)=>{const root=$(selector);root.replaceChildren(...values.map(value=>{const li=document.createElement('li');li.textContent=value;return li}));};
  const paintAppearance=()=>{if(report?.draft){$('[data-appearance-sprite]').innerHTML=deviceAvatarSvg(report.draft.avatar);$('[data-class-status]').textContent=(report.appearance?.class?.reason??'Look classe selezionabile manualmente.')+(avatarOutfit(report.draft.avatar)?classLooksAvailable()?' Pacchetto richiesto sulla SD. Colori e taglio del preset sono fissi.':' Asset assenti: anteprima originale; scelta conservata.':'');}};
  for(const [selector,names] of [['[data-lineage]',avatarSpecies.map(s=>s.name)],['[data-body]',avatarBodies],['[data-outfit]',avatarClasses.map(c=>c.name)]]){
    for(const [i,name] of names.entries()){const option=document.createElement('option');option.value=String(i);option.textContent=name;$(selector).append(option);}
    $(selector).onchange=()=>{if(!report?.draft)return;Object.assign(report.draft.avatar,{lineage:Number($('[data-lineage]').value),body:Number($('[data-body]').value),outfit:Number($('[data-outfit]').value)});paintAppearance();};
  }
  const clear=()=>{generation++;request?.abort();report=null;$('[data-preview]').hidden=true;$('[data-accept]').checked=false;$('[data-accept]').disabled=true;$('[data-create]').disabled=true;$('[data-read] button').disabled=false;};
  const close=()=>dialog.close();
  $('[data-close]').onclick=close;
  dialog.addEventListener('close',()=>{clear();dialog.remove()},{once:true});
  $('[name=character]').addEventListener('input',()=>{clear();$('[data-status]').textContent='';});
  $('[data-read]').onsubmit=async event=>{
    event.preventDefault();clear();const token=generation;
    request=new AbortController();$('[data-read] button').disabled=true;$('[data-status]').textContent='Lettura e verifica in corso…';
    try{
      if(!['http:','https:'].includes(location.protocol))throw new Error('La lettura Beyond richiede l’app PC: apri Avvia-PocketDD.cmd e usa l’indirizzo indicato.');
      const response=await fetch('/api/probe',{method:'POST',credentials:'omit',signal:request.signal,headers:{'Content-Type':'application/json'},body:JSON.stringify({character:$('[name=character]').value})});
      if(!response.headers.get('content-type')?.includes('application/json'))throw new Error('Connettore PC non disponibile. Riapri Avvia-PocketDD.cmd dal pacchetto estratto.');
      const body=await response.json();
      if(token!==generation || !dialog.open)return;
      if(!response.ok || !body.ok)throw new Error(`${body.error?.code??'ERROR'} · ${body.error?.message??'Lettura non riuscita'}`);
      report=body.report;
      const canCreate=allowed();
      $('[data-name]').textContent=`${report.character.name} · livello ${report.character.level}`;
      $('[data-checks]').textContent=`${report.testFixture?'DATI SINTETICI DI TEST · ':''}${guided?(canCreate?'Dati letti e convertiti: verifica valori, aspetto e limiti.':'Conversione bloccata: nessun personaggio creato.'):`HTTP ${report.transport.status} · formato valido · ${canCreate?'dati convertiti; verifica il resoconto per questo PG':'conversione bloccata'}`}`;
      const values=$('[data-values]');values.replaceChildren();
      for(const [label,value] of [...Object.entries(report.abilities).map(([k,v])=>[k,v.preview]),['PF',report.hp?`${report.hp.previewCurrent}/${report.hp.previewMax}`:'non risolti'],['CA',report.armorClass??'non risolta']]){
        const dt=document.createElement('dt'),dd=document.createElement('dd');dt.textContent=label;dd.textContent=String(value);values.append(dt,dd);
      }
      $('[data-appearance]').hidden=!report.draft;
      if(report.draft){const identity=avatarIdentity(report.draft.avatar);$('[data-lineage]').value=String(identity.lineage);$('[data-body]').value=String(identity.body);const a=report.appearance;$('[data-outfit]').value=String(avatarOutfit(report.draft.avatar));
        $('[data-appearance-status]').textContent=(a?.status==='matched'?`Famiglia visiva riconosciuta: ${a.source}.`:a?.status==='ambiguous'?`Specie ambigua (${a.source}): scegli la famiglia visiva.`:`Specie non riconosciuta (${a?.source||'non indicata'}): scegli la famiglia visiva; Umano è provvisorio.`)+(a?.bodySource==='explicit'?' Silhouette proposta dal genere esplicito della sorgente, modificabile.':' Genere non riconosciuto o non indicato: silhouette maschile provvisoria, scegli liberamente.');paintAppearance();}
      $('[data-readiness-status]').textContent=!canCreate?'Importazione bloccata.':report.readiness?.state==='review-required'?`${report.readiness.unresolved.length} verifiche o eccezioni specifiche di questo PG. HTTP e JSON validi non le risolvono.`:'Dati entro l’ambito della prima versione. Controlla le regole manuali; conversione completa di tutte le regole D&D non dichiarata.';
      list('[data-manual]',report.readiness?.manual??[]);list('[data-snapshots]',report.readiness?.snapshots??[]);
      list('[data-reference-rows]',report.draft?.powers.filter(p=>p.reference!==undefined).map(p=>`${p.name}: ${p.description}`)??[]);
      list('[data-evidence]',report.conversion?.evidence??[]);
      list('[data-coverage]',(report.conversion?.coverage??[]).map(c=>`${c.section}: ${c.count} · ${c.status==='converted'?'compilati':c.status==='snapshot'?'fotografia importata':c.status==='data-only'?'dati conservati, effetti manuali':'da verificare'}`));
      list('[data-training]',[...(report.details?.skills??[]).filter(s=>s.rank||s.half||s.extra||s.customized||s.blocked).map(s=>`${s.name}: ${s.bonus>=0?'+':''}${s.bonus}${s.half?' · mezza competenza':''}${s.customized?' · personalizzato':''} · ${s.parts}${s.blocked?' · tiro da verificare':''}`),...(report.details?.saves??[]).filter(s=>s.proficient||s.extra||s.customized||s.blocked).map(s=>`${s.label}: ${s.bonus>=0?'+':''}${s.bonus}${s.half?' · mezza competenza':''}${s.customized?' · personalizzato':''} · ${s.parts}${s.blocked?' · tiro da verificare':''}`)]);
      list('[data-attacks]',(report.details?.weapons??[]).map(w=>`${w.name}: attacco ${w.attack>=0?'+':''}${w.attack} · ${w.damage} · portata ${w.reach} caselle`));
      list('[data-powers]',(report.draft?.powers.filter(p=>p.reference===undefined)??[]).map(p=>`${p.name}: ${p.type==='spell'?`magia ${p.level}° · ${p.castingAbility??'caratteristica locale'}${p.limited?` · usi ${p.limited.remaining}/${p.limited.maxUses}`:''}`:p.passive?'nota/effetto manuale':`${p.remaining}/${p.maxUses} utilizzi`}${p.effect==='eldritch-cannon'?' · creazione gratuita; poi slot a scelta':''}`));
      list('[data-manual-resources]',(report.draft?.extended?.manualResources??[]).map(r=>`${r.name}: contatore manuale · residuo ${r.current}/${r.max} · separato dagli utilizzi`));
      const weapons=report.draft?.inventory.filter(i=>i.type==='weapon'&&i.quantity>0)??[];
      $('[data-weapon-label]').hidden=!weapons.length;
      $('[data-weapon]').replaceChildren();
      for(const item of [{id:'',name:'Scegli in seguito'},...weapons]){const option=document.createElement('option');option.value=item.id;option.textContent=item.name;option.selected=item.id===(report.draft?.equippedWeaponId??'');$('[data-weapon]').append(option);}
      list('[data-defences]',(report.details?.defences??[]).map(d=>`${d.name}: ${d.label}`));
      list('[data-items]',report.inventory.items.map((i,index)=>{const item=report.draft?.inventory[index];return `${i.name} ×${i.quantity??'?'} · ${i.equipped===true?'equipaggiato':i.equipped===false?'non equipaggiato':'equipaggiamento non disponibile'}${item?.type==='healing'?` · cura ${item.diceCount}d${item.dieSides}+${item.healingModifier}`:item?.type==='consumable'?' · consumo quantità':''}${item?.charges?` · cariche ${item.charges.current}/${item.charges.max}, recupero ${item.charges.recovery==='short'?'breve':item.charges.recovery==='long'?'lungo':'manuale'}`:''}`;}));
      list('[data-omissions]',report.conversion?.omissions??report.warnings);
      if(guided)list('[data-guided-limits]',(report.readiness?.unresolved?.length?report.readiness.unresolved:report.conversion?.omissions??report.warnings??[]).slice(0,4));
      const blockers=report.conversion?.blockers?.length?report.conversion.blockers:canCreate?[]:['Il connettore non ha prodotto una scheda convertibile. Rileggi il personaggio con il pacchetto PC aggiornato.'];
      list('[data-blockers]',blockers);
      $('[data-blocker-title]').hidden=canCreate;
      $('[data-preview]').hidden=false;
      $('[data-accept]').disabled=!canCreate;$('[data-consent]').hidden=!canCreate;$('[data-create]').hidden=!canCreate;
      $('[data-consent-status]').textContent=canCreate?'Controlla le omissioni, poi accetta per creare la scheda parziale.':'La conferma non è disponibile perché la conversione è bloccata. I problemi sono elencati sopra, prima dell’avatar e dei dettagli. Nessuna scheda creata.';
      $('[data-status]').textContent=canCreate?'Anteprima pronta. Controlla valori e omissioni prima di confermare.':`Importazione ferma: ${blockers.length} problemi da risolvere, elencati sotto i valori del PG.`;
    }catch(error){if(token===generation && error.name!=='AbortError')$('[data-status]').textContent=error.message;}
    finally{if(token===generation)$('[data-read] button').disabled=false;}
  };
  $('[data-weapon]').onchange=()=>{if(report?.draft)report.draft.equippedWeaponId=$('[data-weapon]').value||null;};
  $('[data-accept]').onchange=()=>{$('[data-create]').disabled=!allowed() || !$('[data-accept]').checked;};
  $('[data-create]').onclick=()=>{
    if(creating || !allowed() || !$('[data-accept]').checked)return;
    if(!guided&&!confirm(`Creare ${report.character.name} come nuovo personaggio parziale? Le schede esistenti restano conservate.`))return;
    creating=true;$('[data-create]').disabled=true;
    try{commit(report.draft);close();onCreated?.();}
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
