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
    <p>PF temporanei assorbono i danni. Condizioni, dadi vita, tiri morte e monete sono conservati; gli effetti delle condizioni richiedono il DM, salvo Avvelenato.</p>
    <form><div class="inventory-form-grid">${input('PF temporanei','tempHp',x.tempHp,0,9999)}${input('Salvezze morte riuscite','successes',x.deathSaves.successes,0,3)}${input('Salvezze morte fallite','failures',x.deathSaves.failures,0,3)}</div>
    <label class="checkbox-label"><input name="stabilized" type="checkbox" ${x.deathSaves.stabilized?'checked':''}>Stabilizzato</label>
    <label>Condizioni (una per riga: nome | livello 0–6)<textarea name="conditions" rows="4">${escapeHtml(x.conditions.map(c=>`${c.name} | ${c.level}`).join('\n'))}</textarea></label>
    <h3>Dadi vita</h3>${x.hitDice.map((h,i)=>input(`${escapeHtml(h.name)} · ${h.max}d${h.die}, usati`,`hd-${i}`,h.used,0,h.max)).join('')||'<p>Nessun dado vita disponibile dal sorgente.</p>'}
    <h3>Monete</h3><div class="inventory-form-grid">${Object.entries(x.currencies).map(([k,v])=>input(k.toUpperCase(),`coin-${k}`,v,0,999999999)).join('')}</div>
    <h3>Slot del patto</h3><div class="inventory-form-grid">${x.pactSlots.map((s,i)=>`${input(`${i+1}° attuali`,`pact-current-${i}`,s.current,0,9)}${input(`${i+1}° massimi`,`pact-max-${i}`,s.max,0,9)}`).join('')}</div>
    ${x.blockedChecks.length?'<label class="checkbox-label"><input type="checkbox" name="verify-checks">Ho corretto e verificato manualmente i tiri segnalati: sblocca i tiri.</label>':''}
    <p data-error role="alert"></p><button type="submit">Salva stato e risorse</button></form><h3>Dettagli importati</h3><ul data-facts></ul>`;
  const list=dialog.querySelector('[data-facts]');for(const value of x.facts){const li=document.createElement('li');li.textContent=value;list.append(li);}
  dialog.querySelector('[data-close]').onclick=()=>dialog.close();
  dialog.querySelector('form').onsubmit=event=>{
    event.preventDefault();const data=new FormData(event.target);
    try{
      if(serialize(getState())!==serialize(state))throw new Error('La scheda è cambiata: riapri la gestione stato');
      const conditions=String(data.get('conditions')).split('\n').map(line=>line.trim()).filter(Boolean).map(line=>{const [name,level='0']=line.split('|').map(s=>s.trim());return{name,level:Number(level)}});
      const extended={...x,tempHp:Number(data.get('tempHp')),conditions,deathSaves:{successes:Number(data.get('successes')),failures:Number(data.get('failures')),stabilized:data.has('stabilized')},
        hitDice:x.hitDice.map((h,i)=>({...h,used:Number(data.get(`hd-${i}`))})),currencies:Object.fromEntries(Object.keys(x.currencies).map(k=>[k,Number(data.get(`coin-${k}`))])),
        pactSlots:x.pactSlots.map((s,i)=>({current:Number(data.get(`pact-current-${i}`)),max:Number(data.get(`pact-max-${i}`))})),blockedChecks:data.has('verify-checks')?[]:x.blockedChecks};
      commit(parseBackup(serialize({...state,extended,condition:conditions.some(c=>['poisoned','avvelenato'].includes(c.name.toLowerCase()))})));dialog.close();
    }catch(error){dialog.querySelector('[data-error]').textContent=error.message;}
  };
  document.body.append(dialog);dialog.addEventListener('close',()=>dialog.remove(),{once:true});dialog.showModal();
}
