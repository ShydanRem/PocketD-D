import {ESPLoader,Transport} from 'esptool-js';
import CryptoJS from 'crypto-js';
import {prepareFirmware,installFirmware} from './flasher.js';
import './style.css';
const $=s=>document.querySelector(s),status=$('#status'),button=$('#install'),progress=$('#progress');
let prepared=null,busy=false;
const supported=window.isSecureContext&&'serial' in navigator;
function ready(){button.disabled=!supported||!prepared||busy||!$('#board').checked;}
function message(text,error=false){status.textContent=text;status.dataset.error=error?'true':'false';}
function phase(name,value){progress.value=value;const labels={connect:'Connessione al dispositivo…',layout:'Controllo delle partizioni…',write:`Installazione e verifica · ${value}%`,restart:'Firmware verificato. Riavvio…',done:'Installazione completata. Controlla che PocketD-D compaia sullo schermo.','done-reset':'Firmware scritto e verificato. Riavvia il dispositivo con PWR e controlla lo schermo.'};message(labels[name]);}
async function prepare(){
  message('Preparo il firmware…');$('#retry').hidden=true;
  try{prepared=await prepareFirmware(new URL('./manifest.json',location.href));$('#version').textContent=`Firmware ${prepared.manifest.version}`;message(supported?'Firmware pronto. Collega il dispositivo e premi Installa.':'Apri questa pagina in Chrome o Edge su PC, tramite HTTPS o localhost.');}
  catch(error){message(error.message,true);$('#retry').hidden=false;}
  ready();
}
$('#board').addEventListener('change',ready);$('#retry').addEventListener('click',prepare);
button.addEventListener('click',async()=>{
  if(button.disabled||busy)return;
  busy=true;ready();$('#retry').hidden=true;for(const el of document.querySelectorAll('input'))el.disabled=true;
  const unload=event=>{event.preventDefault();event.returnValue='';};window.addEventListener('beforeunload',unload);
  try{
    // requestPort must run directly from this click, before asynchronous work.
    const port=await navigator.serial.requestPort();
    await installFirmware({prepared,port,mode:$('input[name=mode]:checked').value,Loader:ESPLoader,TransportClass:Transport,
      md5:data=>CryptoJS.MD5(CryptoJS.lib.WordArray.create(data)).toString(),onStatus:phase});
  }catch(error){
    if(error.name==='NotFoundError')message('Scelta della porta annullata. Puoi riprovare.');
    else message(`${error.message} Chiudi le altre app USB e consulta “Se non si collega”.`,true);
  }finally{
    window.removeEventListener('beforeunload',unload);busy=false;for(const el of document.querySelectorAll('input'))el.disabled=false;ready();
  }
});
prepare();
