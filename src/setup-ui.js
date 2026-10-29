import {deviceAvatarSvg} from './device-avatar.js';
import './setup.css';
import {deviceUsbSupported} from './device-usb.js';
import {projectDevice,serializeDevice} from './device-export.js';

// Session-only guidance: no device success flags or character data in storage.
export function openPocketSetup({getState,getId,getCharacters=()=>[],selectCharacter,getStatus=()=>({}),openImport,openAppearance,openExport,openNew,openReview,openSd}){
  const existing=document.querySelector('.pocket-setup');if(existing){existing.focus();return;}
  const dialog=document.createElement('dialog');dialog.className='editor-dialog pocket-setup';
  dialog.setAttribute('aria-label','Prepara il tuo Pocket');
  let step=0,installOpen=false,installBusy=false,firmware='unknown',receipt=null;
  const escape=s=>String(s).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
  const changed=()=>{if(!receipt)return false;try{return receipt.id!==getId()||receipt.stamp!==serializeDevice(projectDevice(getState()).packet)}catch{return true}};
  function draw(){
    const state=getState(),storage=getStatus(),blocked=storage.blocked||Boolean(storage.warning),serial=deviceUsbSupported();
    dialog.innerHTML=`<div class="editor-heading"><div><span class="eyebrow">DAL PC AL TAVOLO</span><h2>Prepara il tuo Pocket</h2></div><button data-setup-close aria-label="Chiudi guida" ${installBusy?'disabled':''}>×</button></div>
      <ol class="setup-progress" aria-label="Passaggi">${['Pocket','Personaggio','Al tavolo'].map((s,i)=>`<li ${i===step?'aria-current="step"':''}><b>${i+1}</b>${s}</li>`).join('')}</ol>
      ${serial?'':'<p class="setup-storage-warning">Puoi preparare il PG qui. Per installare e inviare via USB apri questo stesso indirizzo in Chrome o Edge su PC.</p>'}<p class="setup-storage-warning" role="status">${blocked?escape(storage.warning||'La raccolta non è accessibile. Chiudi la guida e recupera i dati dalla schermata principale prima di proseguire.'):''}</p><section data-setup-content>${step===0?`<h3>Il dispositivo è pronto?</h3><p>Collega il Pocket con un cavo USB dati. Se mostra già l’eroe puoi passare al personaggio.</p>
        ${installOpen?`<iframe title="Installa PocketD-D" src="/install/?guided=1" allow="serial"></iframe><p data-install-observation role="status">${firmware==='written'?'Firmware scritto e verificato. Controlla il display: il personaggio si trasferisce nel passo successivo.':'L’installazione non trasferisce ancora il PG. Attendi la verifica dei file e controlla il display.'}</p><button data-setup-ready ${installBusy?'disabled':''}>Vedo l’eroe: continua</button>`:`<button class="setup-primary" data-setup-ready>Mostra già l’eroe → continua</button><button data-setup-install ${serial?'':'disabled'}>Installa o aggiorna PocketD-D</button><button class="setup-quiet" data-setup-later>Preparo il PG, installerò dopo</button>`}`:
      step===1?`<h3>Portiamo il tuo eroe qui.</h3><p>Incolla il link Beyond, controlla i valori e scegli l’avatar. La conferma crea una nuova scheda e conserva le altre.</p><button class="setup-primary" data-setup-import>Importa da D&D Beyond</button><div class="setup-existing"><div>${deviceAvatarSvg(state.avatar)}</div><p><strong>${escape(state.name)}</strong><br>PG attivo nella raccolta PC${getCharacters().length>1?`<label>Scegli dalla raccolta<select data-setup-character>${getCharacters().map(c=>`<option value="${escape(c.id)}" ${c.id===getId()?'selected':''}>${escape(c.name)}</option>`).join('')}</select></label>`:''}</p><button data-setup-current>Usa questo personaggio</button></div><details><summary>Non uso Beyond</summary><button data-setup-new>Crea un personaggio manualmente</button><p>Puoi anche chiudere la guida e aggiungere un backup dalla raccolta.</p></details>`:
      `<h3>${receipt&&!changed()?'Copia salvata sul Pocket.':'Ultimo passo: portalo sul Pocket.'}</h3><div class="setup-existing"><div>${deviceAvatarSvg(state.avatar)}</div><p><strong>${escape(state.name)}</strong><br>Livello ${state.level} · PF ${state.hp}/${state.maxHp} · CA ${state.armorClass}</p></div>
       ${receipt&&!changed()?`<p role="status">Il dispositivo ha confermato il salvataggio di ${escape(receipt.name)}. Per conservarne i progressi usa il backup del dispositivo prima di sostituirlo.</p>`:`<p>Invia la copia via USB e conferma <strong>sul display</strong>. È la copia attiva del dispositivo a essere sostituita; la raccolta PC resta conservata.</p>${firmware==='later'?'<p>Hai rimandato l’installazione: prima dell’invio torna a Pocket e installa il firmware.</p>':''}${changed()?'<p>Il PG attivo è cambiato: la conferma precedente non vale per questa scheda.</p>':''}<button class="setup-primary" data-setup-send ${serial?'':'disabled'}>Invia ${escape(state.name)} al Pocket</button>`}
       ${openReview?'<button data-setup-review>Controlla dati e regole del PG</button>':''}<button data-setup-appearance>Personalizza solo l’aspetto</button>${openSd?'<button data-setup-sd>Installa look sulla SD via USB</button>':''}
       <details><summary>Come si usa e personalizza sul dispositivo?</summary><ul><li>Arma e Magie si aprono dalla home; i PF aprono Danni e Cura.</li><li>Scorri le liste con il dito. Premi a lungo un’icona per leggerne il nome.</li><li>Tocca l’avatar per cambiare aspetto e sfondo. Per i look delle classi usa Installa look sulla SD via USB. In alternativa: <a href="assets/PocketDD-SD.zip" download>scarica i look</a>.</li><li>Menu → ingranaggio → Tasti: configura le tue scorciatoie.</li></ul><p>Wi-Fi: Menu → ingranaggio → Wi-Fi → Cerca reti, poi password sul display. Wi-Fi e ponte Beyond sono opzionali: per giocare offline puoi scollegare il PC.</p></details>
       <button class="setup-quiet" data-setup-finish>${receipt&&!changed()?'Vai al tavolo':'Chiudi, invierò dopo'}</button>`}</section>
      <p class="setup-error" role="alert"></p><div class="setup-bottom">${step?'<button data-setup-back>← Indietro</button>':''}<small>La raccolta personale resta nello stesso browser e indirizzo locale.</small></div>`;
    const $=s=>dialog.querySelector(s);
    $('[data-setup-close]').onclick=()=>{if(!installBusy)dialog.close();};
    const bind=(s,fn)=>{const b=$(s);if(b)b.onclick=()=>{try{fn()}catch(e){$('.setup-error').textContent=e.message;}};};
    bind('[data-setup-ready]',()=>{firmware=firmware==='written'?'observed':'user-ready';step=1;draw();});
    bind('[data-setup-install]',async()=>{try{const response=await fetch('/api/setup'),info=await response.json();if(!response.ok||info.kind!=='pocketdd-setup'||!info.installer)throw Error('Installer non incluso: riapri Avvia-PocketDD.cmd dal pacchetto PC completo.');if(dialog.open){installOpen=true;draw();}}catch(e){if(dialog.isConnected)dialog.querySelector('.setup-error').textContent=e.message;}});
    bind('[data-setup-later]',()=>{firmware='later';step=1;draw();});
    bind('[data-setup-back]',()=>{step--;installOpen=false;draw();});
    if(blocked)for(const name of ['import','current','new','send','appearance','review']){const button=$('[data-setup-'+name+']');if(button)button.disabled=true;}
    const characters=$('[data-setup-character]');if(characters){characters.disabled=Boolean(blocked);characters.onchange=()=>{try{selectCharacter?.(characters.value);receipt=null;draw();}catch(e){$('.setup-error').textContent=e.message;}};}
    bind('[data-setup-import]',()=>openImport(()=>{receipt=null;step=2;draw();}));
    bind('[data-setup-current]',()=>{receipt=null;step=2;draw();});
    bind('[data-setup-new]',()=>openNew(()=>{receipt=null;step=2;draw();}));
    bind('[data-setup-sd]',()=>openSd?.());
    bind('[data-setup-review]',()=>{const review=openReview?.();review?.addEventListener('close',()=>{if(dialog.open)draw();},{once:true});});
    bind('[data-setup-appearance]',()=>openAppearance(()=>{draw();}));
    bind('[data-setup-send]',()=>openExport(result=>{receipt=result;draw();}));
    bind('[data-setup-finish]',()=>dialog.close());
  }
  const message=e=>{
    const frame=dialog.querySelector('iframe');if(e.origin!==location.origin||!frame||e.source!==frame.contentWindow||e.data?.type!=='pocketdd:install')return;
    if(e.data.phase==='busy'){installBusy=e.data.value===true;dialog.querySelector('[data-setup-close]').disabled=installBusy;dialog.querySelector('[data-setup-ready]').disabled=installBusy;}
    if(e.data.phase==='verified'){firmware='written';dialog.querySelector('[data-install-observation]').textContent='Firmware scritto e verificato. Controlla che l’eroe compaia sul display, poi continua.';}
  };
  dialog.addEventListener('cancel',e=>{if(installBusy)e.preventDefault();});
  dialog.addEventListener('close',()=>{window.removeEventListener('message',message);dialog.remove();},{once:true});
  window.addEventListener('message',message);document.body.append(dialog);draw();dialog.showModal();
}
