import {readDeviceDiagnostics,receiveDeviceBackup,restoreDeviceBackup,prepareSdAssets,installSdAssets} from './pocket-tools.js';
import {observeUsbPort,usbPortState,claimUsbPort} from './usb-port.js';
const find=id=>document.getElementById(id);let controller,preparing=false,wasKnown=false;
function refresh(state){if(wasKnown&&!state.known)text('Pocket scollegato. Ricollega il cavo e premi Collega Pocket.');wasKnown=state.known;find('device-tools').hidden=false;find('device-connect').disabled=state.busy||preparing||Boolean(controller);find('device-change').disabled=state.busy||preparing||Boolean(controller);for(const id of ['device-backup','device-looks'])find(id).disabled=state.busy||preparing||Boolean(controller);find('device-restore').disabled=state.busy||preparing||Boolean(controller)||!state.known;find('device-cancel').hidden=!controller;}
observeUsbPort(refresh);
const text=value=>find('device-status').textContent=value;
function save(raw){const url=URL.createObjectURL(new Blob([raw],{type:'application/json'})),a=document.createElement('a');a.href=url;a.download='PocketDD-PG-'+new Date().toISOString().slice(0,10)+'.json';a.click();setTimeout(()=>URL.revokeObjectURL(url),1000);}
async function prepare(task){const release=claimUsbPort();try{return await task();}finally{release();}}
async function run(task){if(controller||usbPortState().busy||preparing)return;controller=new AbortController();refresh(usbPortState());try{await task({signal:controller.signal,onProgress:stage=>{const messages={connect:'Collegamento al Pocket…','confirm-backup':'Conferma sul display l’invio del backup.','confirm-restore':'Controlla l’anteprima sul display e conferma il ripristino.',receive:'Ricevo e verifico il backup…','sd-check':'Controllo i look sulla SD…','sd-send':'Copio i look sulla SD…','sd-verify':'Verifico i file ricevuti sulla SD…'};if(messages[stage])text(messages[stage]);}});}catch(error){text(error.name==='AbortError'?'Operazione interrotta. Controlla il display prima di riprovare.':error.message||'Operazione non confermata: controlla il Pocket.');}finally{controller=null;refresh(usbPortState());}}
for(const id of ['device-connect','device-change'])find(id).onclick=()=>run(async options=>{const d=await readDeviceDiagnostics({...options,chooseNew:id==='device-change'});text(`Pocket riconosciuto · firmware ${d.firmware}. Scegli cosa fare qui sotto.`);find('device-tools').hidden=false;});
find('device-backup').onclick=()=>run(async options=>{const raw=await receiveDeviceBackup(options);save(raw);text('Backup verificato e scaricato. Conservalo sul tuo PC.');});
find('device-restore').onclick=()=>{if(!usbPortState().busy)find('device-file').click();};
find('device-file').onchange=event=>{const file=event.target.files[0];event.target.value='';if(!file)return;run(async options=>{if(file.size>110000)throw Error('File troppo grande: scegli un backup PG PocketD-D.');const raw=await prepare(()=>file.text());await restoreDeviceBackup(raw,options);text('Ripristino confermato e salvato dal Pocket.');});};
let lookPackage;
find('device-looks').onclick=()=>run(async options=>{
 if(!usbPortState().known){text('Collego il Pocket…');await readDeviceDiagnostics(options);}
 preparing=true;refresh(usbPortState());text('Scarico e verifico i look…');
 try{lookPackage??=await prepare(()=>prepareSdAssets({url:'https://shydanrem.github.io/PocketD-D/assets/classes.pdc',fetcher:(url,o)=>fetch(url,{...o,credentials:'omit',signal:AbortSignal.any([options.signal,o.signal])})}));options.signal.throwIfAborted();const result=await installSdAssets(lookPackage,{...options,onAssetProgress:n=>text(`Copio i look sulla SD · ${n}%`)});text(result.already?'I look sulla SD sono già aggiornati.':'Look verificati sulla SD. Scegli la Classe dal Pocket.');}finally{preparing=false;}
});
find('device-cancel').onclick=()=>controller?.abort();
window.addEventListener('pagehide',()=>controller?.abort());

window.addEventListener('beforeunload',e=>{if(controller||usbPortState().busy){e.preventDefault();e.returnValue='';}});
