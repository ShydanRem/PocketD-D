import {inspectDeviceBackup,DEVICE_BACKUP_MAX} from './device-backup.js';
export function openDeviceBackup(escapeHtml){
  const dialog=document.createElement('dialog');dialog.className='hero-dialog';let raw=null,revision=0;
  dialog.innerHTML=`<div class="editor-heading"><h2>Backup del dispositivo</h2><button data-close aria-label="Chiudi">×</button></div><p>Archivio completo dello stato sul dispositivo e degli ultimi 20 tiri. La raccolta PC resta conservata.</p><p>Acquisisci la copia via USB con <code>scripts/device-backup.py read</code>, poi apri qui il file ricevuto.</p><label>Apri una copia del dispositivo<input type="file" data-native-file accept=".json,application/json"></label><p data-error role="alert"></p><div data-native-preview></div><button data-native-download disabled>Scarica copia verificata</button><details><summary>Ripristino via USB</summary><p>Usa <code>scripts/device-backup.py restore FILE --port PORTA</code>. Controlla nome, PF e storico sul display e conferma. Il ripristino sostituisce la scheda attiva, lo storico e le preferenze del dispositivo con quelli della copia.</p><p>L'integrità del file è verificata qui. Il dispositivo valida tutti i dati nativi prima di proporre la conferma. Nessuna sincronizzazione automatica con la raccolta PC o Beyond.</p></details>`;
  document.body.append(dialog);dialog.showModal();dialog.addEventListener('close',()=>dialog.remove());dialog.querySelector('[data-close]').onclick=()=>dialog.close();
  dialog.querySelector('[data-native-file]').onchange=async event=>{
    const current=++revision,file=event.target.files[0];raw=null;dialog.querySelector('[data-native-download]').disabled=true;dialog.querySelector('[data-error]').textContent='';dialog.querySelector('[data-native-preview]').innerHTML='';
    if(!file)return;
    try{
      if(file.size>DEVICE_BACKUP_MAX)throw new Error('File oltre 110000 byte');
      const content=await file.text();if(current!==revision)return;const p=inspectDeviceBackup(content);
      const keys=['Spento','Eroe','Scheda','Dadi','Zaino','Magie','Lancia','Riposi','Stato','Schermo','Effetti','Round','Danni','Storico'];
      const coins=['cp','sp','ep','gp','pp'].map(k=>`${k.toUpperCase()} ${p.wallet[k]}`).join(' · ');
      dialog.querySelector('[data-native-preview]').innerHTML=`<h3>${escapeHtml(p.name)}</h3><p data-native-values>Livello ${p.level} · PF ${p.hp}/${p.maxHp} (+${p.tempHp} temporanei) · CA ${p.armor}</p><p>Inventario ${p.items} · magie/usi ${p.powers} · contatori ${p.resources} · effetti locali ${p.effects}</p><p>${p.wallet.available?coins:'Monete non disponibili'}</p><p>Tasti: ${p.keys.map(k=>escapeHtml(keys[k])).join(' · ')}. Luminosità ${p.brightness}%.</p><details data-native-history><summary>Storico · ${p.rolls.length} tiri</summary><ol>${p.rolls.map(r=>`<li><strong>#${r.number} ${escapeHtml(r.label)}</strong>: ${r.dice.join(' / ')} ${r.bonus>=0?'+':''}${r.bonus} = ${r.total}${r.mode>0?' (vantaggio)':r.mode<0?' (svantaggio)':''}<br>${escapeHtml(r.note)}</li>`).join('')}</ol></details><p>Integrità verificata. La validazione completa avviene sul dispositivo prima della conferma. Il file conserva integralmente scheda e storico.</p>`;
      raw=content;dialog.querySelector('[data-native-download]').disabled=false;
    }catch(error){if(current===revision)dialog.querySelector('[data-error]').textContent=error.message;}
  };
  dialog.querySelector('[data-native-download]').onclick=()=>{
    if(!raw)return;const url=URL.createObjectURL(new Blob([raw],{type:'application/json'})),a=document.createElement('a');a.href=url;a.download='pocketdd-device-backup.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
  };
}
