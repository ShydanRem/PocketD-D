import {skills} from './model.js';
import {avatarClasses} from './avatar-classes.js';
import {avatarSpecies,avatarBodies} from './avatar-species.js';
import {receiveDeviceBackup,restoreDeviceBackup} from './device-usb.js';
import {bindUsbDialog,usbStatusMarkup} from './device-usb-ui.js';
import {inspectDeviceBackup,DEVICE_BACKUP_MAX} from './device-backup.js';
export function openDeviceBackup(escapeHtml){
  const dialog=document.createElement('dialog');dialog.className='hero-dialog';let raw=null,revision=0;
  dialog.innerHTML=`<div class="editor-heading"><h2>Backup del dispositivo</h2><button data-close aria-label="Chiudi">×</button></div><p>Archivio completo dello stato sul dispositivo e degli ultimi 20 tiri. La raccolta PC resta conservata.</p><p>Ricevi una copia via USB oppure apri un file già salvato. Acquisizione e ripristino richiedono conferma sul display del dispositivo.</p><button data-native-receive>Ricevi backup via USB</button>${usbStatusMarkup}<label>Apri una copia del dispositivo<input type="file" data-native-file accept=".json,application/json"></label><p data-error role="alert"></p><div data-native-preview></div><button data-native-download disabled>Scarica copia verificata</button><button data-native-restore disabled>Ripristina questa copia via USB</button><p>Il ripristino sostituisce scheda, storico e preferenze del dispositivo con la copia mostrata. Controlla l’anteprima e poi conferma sul display.</p><details><summary>Alternativa con gli script Python</summary><p>Acquisizione: <code>scripts/device-backup.py read --output FILE --port PORTA</code>.</p><p>Usa <code>scripts/device-backup.py restore FILE --port PORTA</code>. Controlla nome, PF e storico sul display e conferma. Il ripristino sostituisce la scheda attiva, lo storico e le preferenze del dispositivo con quelli della copia.</p><p>L'integrità del file è verificata qui. Il dispositivo valida tutti i dati nativi prima di proporre la conferma. Nessuna sincronizzazione automatica con la raccolta PC o Beyond.</p></details>`;
  document.body.append(dialog);dialog.showModal();dialog.addEventListener('close',()=>dialog.remove());dialog.querySelector('[data-close]').onclick=()=>dialog.close();
  const usb=bindUsbDialog(dialog),download=dialog.querySelector('[data-native-download]'),restore=dialog.querySelector('[data-native-restore]');
  dialog.querySelector('[data-native-receive]').disabled=!usb.supported;
  const show=content=>{
    const p=inspectDeviceBackup(content);
      const keys=['Spento','Eroe','Scheda','Dadi','Zaino','Magie','Lancia','Riposi','Stato','Schermo','Effetti','Round','Danni','Storico','Aspetto'];
      const coins=['cp','sp','ep','gp','pp'].map(k=>`${k.toUpperCase()} ${p.wallet[k]}`).join(' · ');
      dialog.querySelector('[data-native-preview]').innerHTML=`<h3>${escapeHtml(p.name)}</h3><p data-native-values>Livello ${p.level} · PF ${p.hp}/${p.maxHp} (+${p.tempHp} temporanei) · CA ${p.armor}</p><p>Inventario ${p.items} · magie/usi ${p.powers} · contatori ${p.resources} · effetti locali ${p.effects}</p><p>${p.wallet.available?coins:'Monete non disponibili'}</p><p>Avatar: ${avatarSpecies[p.lineage].name} · ${avatarBodies[p.body]} · ${avatarClasses[p.outfit].name} (asset SD richiesti per i preset).</p><p>Sfondo: ${['Accampamento','Rovine lunari',"Porto all’alba"][p.scene]}. Scelta locale del dispositivo.</p><p>Riferimenti turno: ${p.turnNames.map((name,i)=>`${String.fromCharCode(65+i)}: ${escapeHtml(name||"senza nome")}`).join(" · ")}</p><p>Costi riduzioni: ${p.payments.map((mode,i)=>`${i+1}: ${mode?"una scelta":"tutte"}`).join(" · ")}</p><details data-native-skill-abilities><summary>Caratteristiche delle abilità · ${Object.keys(p.skillAbilities).length} personalizzate</summary><ul>${Object.entries(p.skillAbilities).map(([key,value])=>`<li>${escapeHtml(skills.find(s=>s.id===key).name)}: ${value}</li>`).join('')}</ul></details><p>Tasti: ${p.keys.map(k=>escapeHtml(keys[k])).join(' · ')}. Luminosità ${p.brightness}%.</p><details data-native-history><summary>Storico · ${p.rolls.length} tiri</summary><ol>${p.rolls.map(r=>`<li><strong>#${r.number} ${escapeHtml(r.label)}</strong>: ${r.dice.join(' / ')} ${r.bonus>=0?'+':''}${r.bonus} = ${r.total}${r.mode>0?' (vantaggio)':r.mode<0?' (svantaggio)':''}<br>${escapeHtml(r.note)}</li>`).join('')}</ol></details><p>Integrità verificata. La validazione completa avviene sul dispositivo prima della conferma. Il file conserva integralmente scheda e storico.</p>`;
    raw=content;download.disabled=false;restore.disabled=!usb.supported;dialog.querySelector('[data-error]').textContent='';
  };
  dialog.querySelector('[data-native-file]').onchange=async event=>{
    const current=++revision,file=event.target.files[0];raw=null;download.disabled=restore.disabled=true;dialog.querySelector('[data-error]').textContent='';dialog.querySelector('[data-native-preview]').innerHTML='';
    if(usb.supported)dialog.querySelector('[data-usb-status]').textContent='';
    if(!file)return;
    try{if(file.size>DEVICE_BACKUP_MAX)throw new Error('File oltre 110000 byte');const content=await file.text();if(current===revision)show(content);}
    catch(error){if(current===revision)dialog.querySelector('[data-error]').textContent=error.message;}
  };
  dialog.querySelector('[data-native-receive]').onclick=()=>{++revision;usb.run(options=>receiveDeviceBackup(options),show);};
  restore.onclick=()=>{if(raw){const snapshot=raw;usb.run(options=>restoreDeviceBackup(snapshot,options));}};
  dialog.querySelector('[data-native-download]').onclick=()=>{
    if(!raw)return;const url=URL.createObjectURL(new Blob([raw],{type:'application/json'})),a=document.createElement('a');a.href=url;a.download='pocketdd-device-backup.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
  };
}
