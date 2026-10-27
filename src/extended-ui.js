import {damageNames,emptyDefences,defenceFlags,defenceLabel} from './defences.js';
import {parseBackup,serialize} from './model.js';
export function importedSummary(state){
  const x=state.extended;if(!x)return '';
  return `<div class="extended-summary"><span>PF temporanei <b>${x.tempHp}</b> · ${x.conditions.length} condizioni · ${x.hitDice.reduce((sum,h)=>sum+h.max-h.used,0)} dadi vita disponibili</span><button data-action="extended-state">Stato, risorse e note</button></div>`;
}
export function openExtendedState({getState,commit,escapeHtml}){
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
    <h3>Dadi vita</h3>${x.hitDice.map((h,i)=>input(`${escapeHtml(h.name)} · ${h.max}d${h.die}, usati`,`hd-${i}`,h.used,0,h.max)).join('')||'<p>Nessun dado vita disponibile dal sorgente.</p>'}
    <h3>Monete</h3><label class="checkbox-label"><input name="currencies-known" type="checkbox" ${x.currenciesAvailable?'checked':''}>Saldo verificato, anche se zero</label><div class="inventory-form-grid">${Object.entries(x.currencies).map(([k,v])=>input(k.toUpperCase(),`coin-${k}`,v,0,999999999)).join('')}</div>
    <h3>Slot del patto</h3><div class="inventory-form-grid">${x.pactSlots.map((s,i)=>`${input(`${i+1}° attuali`,`pact-current-${i}`,s.current,0,9)}${input(`${i+1}° massimi`,`pact-max-${i}`,s.max,0,9)}`).join('')}</div>
    ${x.blockedChecks.length?'<label class="checkbox-label"><input type="checkbox" name="verify-checks">Ho corretto e verificato manualmente i tiri segnalati: sblocca i tiri.</label>':''}
    <p data-error role="alert"></p><button type="submit">Salva stato e risorse</button></form><h3>Dettagli importati</h3><ul data-facts></ul>`;
  const list=dialog.querySelector('[data-facts]');for(const value of x.facts){const li=document.createElement('li');li.textContent=value;list.append(li);}
  dialog.querySelector('[data-close]').onclick=()=>dialog.close();
  dialog.querySelector('[name=defences-enabled]').onchange=event=>dialog.querySelectorAll('[name^=defence-]').forEach(e=>e.disabled=!event.target.checked);
  dialog.querySelector('form').onsubmit=event=>{
    event.preventDefault();const data=new FormData(event.target);
    try{
      if(serialize(getState())!==serialize(state))throw new Error('La scheda è cambiata: riapri la gestione stato');
      const conditions=String(data.get('conditions')).split('\n').map(line=>line.trim()).filter(Boolean).map(line=>{const [name,level='0']=line.split('|').map(s=>s.trim());return{name,level:Number(level)}});
      const extended={...x,tempHp:Number(data.get('tempHp')),conditions,deathSaves:{successes:Number(data.get('successes')),failures:Number(data.get('failures')),stabilized:data.has('stabilized')},
        currenciesAvailable:data.has('currencies-known'),hitDice:x.hitDice.map((h,i)=>({...h,used:Number(data.get(`hd-${i}`))})),currencies:Object.fromEntries(Object.keys(x.currencies).map(k=>[k,Number(data.get(`coin-${k}`))])),
        pactSlots:x.pactSlots.map((s,i)=>({current:Number(data.get(`pact-current-${i}`)),max:Number(data.get(`pact-max-${i}`))})),blockedChecks:data.has('verify-checks')?[]:x.blockedChecks};
      delete extended.defences;
      if(data.has('defences-enabled')){const d=emptyDefences();for(let i=0;i<13;i++){const f=Number(data.get(`defence-${i}`));if(!Number.isInteger(f)||f< -1||f>7)throw new Error('Difesa non valida');if(f>=0){const bit=1<<i;d.configured|=bit;if(f&1)d.resistance|=bit;if(f&2)d.vulnerability|=bit;if(f&4)d.immunity|=bit;}}extended.defences=d;}
      commit(parseBackup(serialize({...state,extended,condition:data.has('poisoned-effect')})));dialog.close();
    }catch(error){dialog.querySelector('[data-error]').textContent=error.message;}
  };
  document.body.append(dialog);dialog.addEventListener('close',()=>dialog.remove(),{once:true});dialog.showModal();
}
