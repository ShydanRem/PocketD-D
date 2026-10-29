import {abilityKeys,skills,describeRoll,weaponStats} from './model.js';
import {equippedWeapon} from './inventory.js';
import {damageNames,defenceFlags,emptyDefences} from './defences.js';
import {projectDevice} from './device-export.js';

// Derive the review from the current sheet. Never save a blanket "verified" flag
// or remove the source limitations after a local correction.
export function importReview(state){
  const blocked=[...abilityKeys.map(key=>({kind:'check',key})),...skills.map(s=>({kind:'skill',key:s.id})),...abilityKeys.map(key=>({kind:'save',key}))].map(r=>describeRoll(state,r)).filter(r=>r.blocked).map(r=>r.label);
  const defences=state.extended?.defences??emptyDefences();
  const missing=damageNames.filter((_,i)=>defenceFlags(defences,i)===null);
  const weapon=equippedWeapon(state),stats=weaponStats(state);
  let transfer;try{const {packet}=projectDevice(state);transfer={available:true,version:packet.version,items:packet.inventory.items.length,powers:packet.magic.powers.length};}catch(error){transfer={available:false,error:error.message};}
  return {blocked,missing,weapon:weapon?{name:weapon.name,attack:stats.attackBonus}:null,transfer,limitations:[...(state.importInfo?.limitations??[])]};
}

export function openImportReview({getState,actions={}}){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog beyond-import import-review';dialog.setAttribute('aria-label','Controlla la copia per il Pocket');
  const esc=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const button=(action,label)=>actions[action]?`<button type="button" data-review-action="${action}">${label}</button>`:'';
  function draw(){
    const s=getState(),r=importReview(s);
    dialog.innerHTML=`<div class="editor-heading"><h2>Controlla la copia per il Pocket</h2><button data-review-close aria-label="Chiudi controllo">×</button></div>
      <p><strong>${esc(s.name)}</strong> · valori della scheda attuale. Aprire o chiudere questo controllo non modifica il PG.</p>
      <section class="review-card"><h3>Valori principali</h3><p>PF ${s.hp}/${s.maxHp} · CA ${s.armorClass} · ${abilityKeys.map(k=>`${k} ${s.abilities[k]}`).join(' · ')}</p><p>CA e PF massimi sono fotografie: confrontali con la scheda dopo modifiche a livello, caratteristiche o oggetti.</p>${button('values','Correggi valori e competenze')}</section>
      <section class="review-card"><h3>Tiri ${r.blocked.length?'· da verificare':'· dati configurati'}</h3><p>${r.blocked.length?esc(r.blocked.join(', '))+'. Controlla competenze e bonus; sblocca solo dopo la verifica manuale.':'Nessuno dei 30 tiri è segnalato come bloccato. Le regole situazionali restano da controllare al tavolo.'}</p>${button('values','Apri competenze e bonus')}${r.blocked.length?button('state','Ho verificato i tiri: apri lo sblocco'):''}</section>
      <section class="review-card"><h3>Difese · ${13-r.missing.length}/13 tipi configurati</h3><p>${r.missing.length?'Da verificare: '+esc(r.missing.join(', '))+'. Un tipo sconosciuto non equivale a Normale.':'Profilo base configurato; resistenze condizionali ed effetti speciali richiedono ancora controllo.'}</p>${button('defences','Controlla le difese')}</section>
      <section class="review-card"><h3>Arma e inventario</h3><p>${r.weapon?`${esc(r.weapon.name)} · attacco ${r.weapon.attack>=0?'+':''}${r.weapon.attack}`:'Nessuna arma attiva. Sceglila nello zaino se il PG deve usarla.'} · ${s.inventory.length} voci.</p>${button('inventory','Configura arma e oggetti')}</section>
      <section class="review-card"><h3>Magie, capacità e risorse</h3><p>${s.powers.filter(p=>p.reference===undefined).length} magie/capacità. Verifica preparazione, slot, utilizzi e recuperi; le descrizioni non eseguono automaticamente gli effetti.</p>${button('powers','Configura magie e capacità')}${button('state','Stato, dadi vita e contatori')}</section>
      <details><summary>Limiti della lettura Beyond · ${r.limitations.length}</summary><p>Resoconto conservato dall’importazione: le correzioni locali non dimostrano che il convertitore supporti quelle regole.</p><ul>${r.limitations.map(v=>`<li>${esc(v)}</li>`).join('')||'<li>Nessun resoconto Beyond su questa scheda.</li>'}</ul></details>
      <section class="review-card" data-review-transfer><h3>${r.transfer.available?'Copia generabile':'Trasferimento bloccato'}</h3><p>${r.transfer.available?`USB${r.transfer.version} · ${r.transfer.items} oggetti · ${r.transfer.powers} voci magiche. Questo verifica il formato trasferibile, non tutte le regole D&D né il salvataggio sul Pocket.`:esc(r.transfer.error)}</p>${r.transfer.available?button('send','Apri anteprima e invio USB'):''}</section><p data-review-error role="alert"></p>`;
    dialog.querySelector('[data-review-close]').onclick=()=>dialog.close();
    for(const b of dialog.querySelectorAll('[data-review-action]'))b.onclick=()=>{
      try{actions[b.dataset.reviewAction]();const child=[...document.querySelectorAll('dialog[open]')].findLast(d=>d!==dialog);if(child)child.addEventListener('close',()=>{if(dialog.open)draw();},{once:true});else draw();}
      catch(error){dialog.querySelector('[data-review-error]').textContent=error.message;}
    };
  }
  dialog.addEventListener('close',()=>dialog.remove(),{once:true});document.body.append(dialog);draw();dialog.showModal();return dialog;
}
