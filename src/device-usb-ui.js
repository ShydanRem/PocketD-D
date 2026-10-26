import {deviceUsbSupported} from './device-usb.js';
const messages={connect:'Collegamento USB…',send:'Invio della copia…','confirm-import':'Controlla nome e valori sul dispositivo e conferma entro 120 secondi. Sostituisce solo la copia sul dispositivo.',imported:'Copia confermata e salvata dal dispositivo. Raccolta PC conservata.','confirm-backup':'Conferma sul dispositivo l’invio del backup entro 120 secondi.',receive:'Ricezione e verifica del backup…',received:'Backup ricevuto e verificato. Scarica il file per conservarlo.','confirm-restore':'Controlla il backup sul dispositivo e conferma entro 120 secondi. Sostituisce scheda, storico e preferenze del dispositivo.',restored:'Ripristino confermato e salvato dal dispositivo. Raccolta PC conservata.'};
export function bindUsbDialog(dialog){
  const status=dialog.querySelector('[data-usb-status]'),abort=dialog.querySelector('[data-usb-abort]');let controller=null;
  const supported=deviceUsbSupported();
  if(!supported)status.textContent='USB dal browser: usa Chrome o Edge su PC, tramite HTTPS o localhost. Restano disponibili file e script Python.';
  abort.onclick=()=>controller?.abort();
  dialog.addEventListener('cancel',event=>{if(controller)event.preventDefault();});
  dialog.addEventListener('close',()=>controller?.abort());
  return {supported,async run(task,complete=()=>{}){
    if(controller)return;
    controller=new AbortController();const controls=new Map(Array.from(dialog.querySelectorAll('button,input'),el=>[el,el.disabled]));
    for(const el of controls.keys())el.disabled=true;abort.hidden=false;abort.disabled=false;dialog.dataset.usbBusy='true';
    let result,success=false;
    try{result=await task({signal:controller.signal,onProgress:phase=>status.textContent=messages[phase]??'Operazione USB…'});success=true;}
    catch(error){status.textContent=error.message||'Collegamento USB non riuscito. Controlla il display prima di riprovare.';}
    finally{controller=null;delete dialog.dataset.usbBusy;for(const [el,disabled] of controls)el.disabled=disabled;abort.hidden=true;}
    if(success&&dialog.isConnected)complete(result);
  }};
}
export const usbStatusMarkup='<p data-usb-status role="status" aria-live="polite"></p><button data-usb-abort hidden>Interrompi collegamento USB</button>';
