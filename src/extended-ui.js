import {damageNames,emptyDefences,defenceFlags,defenceLabel,completeNormalDefences} from './defences.js';
import {parseBackup,serialize} from './model.js';
export function importedSummary(state){
  const x=state.extended;if(!x)return '';
  return `<div class="extended-summary"><span>PF temporanei <b>${x.tempHp}</b> · ${x.conditions.length} condizioni · ${x.hitDice.reduce((sum,h)=>sum+h.max-h.used,0)} dadi vita disponibili</span><button data-action="extended-state">Stato, risorse e note</button></div>`;
}
export function openExtendedState({getState,commit,escapeHtml,focusDefences=false}){
  const state=getState(),x=state.extended;if(!x)return;
  const dialog=document.createElement('dialog');dialog.className='editor-dialog beyond-import';
  const input=(label,name,value,min,max)=>`<label>${label}<input type="number" name="${name}" min="${min}" max="${max}" value="${value}" required></label>`;
  dialog.innerHTML=`<div class="editor-heading"><h2>Stato e risorse</h2><button data-close aria-label="Chiudi">×</button></div>
    <p>PF temporanei assorbono i danni. Condizioni, dadi vita, tiri morte e monete sono conservati; le condizioni sono annotazioni. L’effetto Avvelenato è un comando separato; gli altri effetti richiedono il DM.</p>
    <form><div class="inventory-form-grid">${input('PF temporanei','tempHp',x.tempHp,0,9999)}${input('Salvezze morte riuscite','successes',x.deathSaves.successes,0,3)}${input('Salvezze morte fallite','failures',x.deathSaves.failures,0,3)}</div>
    <label class="checkbox-label"><input name="stabilized" type="checkbox" ${x.deathSaves.stabilized?'checked':''}>Stabilizzato</label>
    <label class="checkbox-label"><input name="poisoned-effect" type="checkbox" ${state.condition?'checked':''}>Avvelenato: svantaggio a prove e attacchi, separato dalle note</label>
    <label>Condizioni (una per riga: nome | livello 0–6)<textarea name="conditions" rows="4">${escapeHtml(x.conditions.map(c=>`${c.name} | ${c.level}`).join('\n'))}</textarea></label>
    <details data-defence-editor><summary>Difese ai danni · dispositivo</summary><p>Si applicano nel calcolo Danni del dispositivo, non ai pulsanti PF del PC. Un tipo da verificare non equivale a normale. Dopo cambi di equipaggiamento o effetti ricontrolla il profilo.</p><label class="checkbox-label"><input name="defences-enabled" type="checkbox" ${x.defences!==undefined?'checked':''}>Trasferisci questo profilo difese (firmware 0.71)</label><div class="inventory-form-grid">${damageNames.map((name,i)=>`<label>${name}<select name="defence-${i}" ${x.defences===undefined?'disabled':''}>${[null,0,1,2,3,4,5,6,7].map(flags=>`<option value="${flags??-1}" ${defenceFlags(x.defences??emptyDefences(),i)===flags?'selected':''}>${defenceLabel(flags)}</option>`).join('')}</select></label>`).join('')}</div></details>
    ${x.manualResources!==undefined?`<section data-manual-resources><h3>Contatori manuali</h3><p>Saldi separati dalle capacità: residuo sorgente = massimo meno valore usato. Non indicano necessariamente azioni disponibili. Nessun consumo o recupero automatico.</p>${x.manualResources.map((r,i)=>`<fieldset><legend>${escapeHtml(r.name)}</legend><div class="inventory-form-grid">${input('Residuo',`manual-${i}-current`,r.current,0,999)}${input('Massimo',`manual-${i}-max`,r.max,1,999)}</div></fieldset>`).join('')}</section>`:''}
    <h3>Dadi vita</h3>${x.hitDice.map((h,i)=>input(`${escapeHtml(h.name)} · ${h.max}d${h.die}, usati`,`hd-${i}`,h.used,0,h.max)).join('')||'<p>Nessun dado vita disponibile dal sorgente.</p>'}
    <h3>Monete</h3><label class="checkbox-label"><input name="currencies-known" type="checkbox" ${x.currenciesAvailable?'checked':''}>Saldo verificato, anche se zero</label><div class="inventory-form-grid">${Object.entries(x.currencies).map(([k,v])=>input(k.toUpperCase(),`coin-${k}`,v,0,999999999)).join('')}</div>
    <h3>Slot del patto</h3><div class="inventory-form-grid">${x.pactSlots.map((s,i)=>`${input(`${i+1}° attuali`,`pact-current-${i}`,s.current,0,9)}${input(`${i+1}° massimi`,`pact-max-${i}`,s.max,0,9)}`).join('')}</div>
    ${x.blockedChecks.length?'<label class="checkbox-label"><input type="checkbox" name="verify-checks">Ho corretto e verificato manualmente i tiri segnalati: sblocca i tiri.</label>':''}
    <p data-error role="alert"></p><button type="submit">Salva stato e risorse</button></form><h3>Dettagli importati</h3><ul data-facts></ul>`;
  const list=dialog.querySelector('[data-facts]');for(const value of x.facts){const li=document.createElement('li');li.textContent=value;list.append(li);}
  dialog.querySelector('[data-close]').onclick=()=>dialog.close();
  const defenceEditor=dialog.querySelector('[data-defence-editor]'),enabled=dialog.querySelector('[name=defences-enabled]');
  const setup=document.createElement('div');
  setup.innerHTML='<p data-defence-count role="status"></p><button type="button" data-defence-review>Completa i tipi da verificare…</button><section data-defence-completion hidden><p>Hai controllato la scheda e le regole del PG? Questi tipi saranno dichiarati Normali nel profilo base:</p><ul data-defence-missing></ul><p>Le difese già impostate restano conservate. Effetti temporanei o condizionali richiedono una gestione separata sul dispositivo. La scelta resta in bozza fino a Salva stato e risorse.</p><button type="button" data-defence-confirm>Ho verificato: imposta Normale</button><button type="button" data-defence-cancel>Annulla scelta</button></section>';
  defenceEditor.append(setup);
  const selectors=[...dialog.querySelectorAll('[name^=defence-]')],review=setup.querySelector('[data-defence-completion]');
  const readDefences=()=>{
    const d=emptyDefences();selectors.forEach((select,i)=>{const f=Number(select.value);if(!Number.isInteger(f)||f< -1||f>7)throw new Error('Difesa non valida');if(f>=0){const bit=1<<i;d.configured|=bit;if(f&1)d.resistance|=bit;if(f&2)d.vulnerability|=bit;if(f&4)d.immunity|=bit;}});return d;
  };
  const updateDefences=()=>{
    review.hidden=true;selectors.forEach(e=>e.disabled=!enabled.checked);
    const missing=selectors.filter(e=>e.value==='-1').length;
    setup.querySelector('[data-defence-count]').textContent=enabled.checked?`${13-missing}/13 tipi verificati · ${missing} da verificare`:'Profilo non trasferito. Abilitalo per configurare le difese.';
    setup.querySelector('[data-defence-review]').disabled=!enabled.checked||!missing;
  };
  enabled.onchange=updateDefences;selectors.forEach(e=>e.onchange=updateDefences);updateDefences();
  let reviewed;
  setup.querySelector('[data-defence-review]').onclick=()=>{
    reviewed=readDefences();const list=setup.querySelector('[data-defence-missing]');list.replaceChildren();
    damageNames.forEach((name,i)=>{if(defenceFlags(reviewed,i)===null){const li=document.createElement('li');li.textContent=name;list.append(li);}});review.hidden=false;
  };
  setup.querySelector('[data-defence-cancel]').onclick=()=>{review.hidden=true;};
  setup.querySelector('[data-defence-confirm]').onclick=()=>{
    if(!enabled.checked||JSON.stringify(readDefences())!==JSON.stringify(reviewed)){updateDefences();return;}
    const completed=completeNormalDefences(reviewed);selectors.forEach((e,i)=>e.value=String(defenceFlags(completed,i)));updateDefences();
  };
  dialog.querySelector('form').onsubmit=event=>{
    event.preventDefault();const data=new FormData(event.target);
    try{
      if(serialize(getState())!==serialize(state))throw new Error('La scheda è cambiata: riapri la gestione stato');
      const conditions=String(data.get('conditions')).split('\n').map(line=>line.trim()).filter(Boolean).map(line=>{const [name,level='0']=line.split('|').map(s=>s.trim());return{name,level:Number(level)}});
      const extended={...x,tempHp:Number(data.get('tempHp')),conditions,deathSaves:{successes:Number(data.get('successes')),failures:Number(data.get('failures')),stabilized:data.has('stabilized')},
        currenciesAvailable:data.has('currencies-known'),hitDice:x.hitDice.map((h,i)=>({...h,used:Number(data.get(`hd-${i}`))})),currencies:Object.fromEntries(Object.keys(x.currencies).map(k=>[k,Number(data.get(`coin-${k}`))])),
        pactSlots:x.pactSlots.map((s,i)=>({current:Number(data.get(`pact-current-${i}`)),max:Number(data.get(`pact-max-${i}`))})),blockedChecks:data.has('verify-checks')?[]:x.blockedChecks};
      delete extended.defences;
      if(x.manualResources!==undefined)extended.manualResources=x.manualResources.map((r,i)=>({...r,current:Number(data.get(`manual-${i}-current`)),max:Number(data.get(`manual-${i}-max`))}));
      if(data.has('defences-enabled'))extended.defences=readDefences();
      commit(parseBackup(serialize({...state,extended,condition:data.has('poisoned-effect')})));dialog.close();
    }catch(error){dialog.querySelector('[data-error]').textContent=error.message;}
  };
  document.body.append(dialog);dialog.addEventListener('close',()=>dialog.remove(),{once:true});dialog.showModal();
  if(focusDefences){defenceEditor.open=true;enabled.focus();enabled.scrollIntoView({block:'center'});}
}
