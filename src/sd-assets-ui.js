import {prepareSdAssets,installSdAssets} from './sd-assets.js';
import {deviceUsbSupported} from './device-usb.js';
import {formatUsbDiagnostics} from './device-usb-diagnostics.js';
export function bindSdInstaller(root,{isBlocked=()=>false,onBusy=()=>{}}={}){
  const button=root.querySelector('[data-sd-install]'),status=root.querySelector('[data-sd-status]'),progress=root.querySelector('[data-sd-progress]'),abort=root.querySelector('[data-sd-abort]');
  let handle=null,controller=null;button.disabled=true;
  const preventUnload=e=>{e.preventDefault();e.returnValue='';};
  if(!deviceUsbSupported()){status.textContent='Per preparare la SD via USB usa Chrome o Edge su PC.';return;}
  status.textContent='Preparo e verifico i look…';
  prepareSdAssets().then(value=>{if(root.isConnected){handle=value;button.disabled=false;status.textContent='Pacchetto verificato · 504 look. Inserisci la microSD FAT32 nel Pocket e collega il cavo USB.';}}).catch(error=>{status.textContent=error.message;});
  abort.onclick=()=>controller?.abort();
  root.addEventListener('cancel',e=>{if(controller)e.preventDefault();});
  root.addEventListener('close',()=>controller?.abort());
  button.onclick=async()=>{
    if(!handle||controller||isBlocked())return;
    controller=new AbortController();window.addEventListener('beforeunload',preventUnload);onBusy(true);const controls=new Map([...root.querySelectorAll('button')].map(el=>[el,el.disabled]));for(const el of controls.keys())el.disabled=true;
    abort.hidden=false;abort.disabled=false;progress.hidden=false;progress.value=0;root.dataset.usbBusy='true';
    const messages={connect:'Scegli la porta del Pocket avviato…','sd-check':'Controllo il pacchetto sulla SD…','sd-send':'Copia dei look sulla SD…','sd-verify':'Ricezione completata. Verifico i file sulla SD…','sd-complete':'Look verificati sulla SD. Sul Pocket scegli la Classe e conferma l’aspetto.'};
    try{await installSdAssets(handle,{signal:controller.signal,onProgress:p=>status.textContent=messages[p]??'Preparazione SD…',onAssetProgress:n=>{progress.value=n;status.textContent=`Copia dei look sulla SD · ${n}%`;},onDiagnostic:d=>{const diagnostic=root.querySelector('[data-sd-diagnostic]');if(diagnostic){diagnostic.hidden=false;diagnostic.querySelector('pre').textContent=formatUsbDiagnostics(d);}}});progress.value=100;}
    catch(error){status.textContent=error.message||'Installazione SD non confermata. Controlla il display prima di riprovare.';}
    finally{controller=null;window.removeEventListener('beforeunload',preventUnload);onBusy(false);delete root.dataset.usbBusy;abort.hidden=true;for(const [el,disabled] of controls)el.disabled=disabled;}
  };
}
export const sdInstallerMarkup=`<p>Lascia la microSD FAT32 nel Pocket e collega il cavo USB. Il browser prepara e installa tutti i look: non serve estrarre la scheda o copiare cartelle.</p><p>Richiede firmware PocketD-D 0.83. Il PG, i progressi e gli altri file della SD restano conservati.</p><button data-sd-install disabled>Installa look sulla SD</button><p data-sd-status role="status" aria-live="polite"></p><progress data-sd-progress max="100" value="0" hidden></progress><button data-sd-abort hidden>Interrompi installazione SD</button><details data-sd-diagnostic hidden><summary>Dettagli USB della SD</summary><pre style="white-space:pre-wrap;overflow-wrap:anywhere"></pre></details>`;
export function openSdInstaller(){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog beyond-import sd-installer';dialog.setAttribute('aria-label','Prepara i look sulla SD');
  dialog.innerHTML=`<div class="editor-heading"><h2>Prepara i look sulla SD</h2><button data-sd-close aria-label="Chiudi preparazione SD">×</button></div>${sdInstallerMarkup}`;
  dialog.querySelector('[data-sd-close]').onclick=()=>dialog.close();dialog.addEventListener('close',()=>dialog.remove(),{once:true});document.body.append(dialog);dialog.showModal();bindSdInstaller(dialog);return dialog;
}
