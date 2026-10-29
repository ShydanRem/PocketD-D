import {ESPLoader,Transport} from 'esptool-js';
import CryptoJS from 'crypto-js';
import {prepareFirmware,installFirmware} from './flasher.js';
import './style.css';
import {preparePcPackage} from './package-download.js';
const $=s=>document.querySelector(s),status=$('#status'),button=$('#install'),progress=$('#progress');
let prepared=null,busy=false;
const guided=new URLSearchParams(location.search).get('guided')==='1';
if(guided){document.body.classList.add('guided-install');document.querySelector('input[value=update]').checked=true;}
const announce=(phase,value)=>{if(guided&&parent!==window)parent.postMessage({type:'pocketdd:install',phase,value},location.origin);};
if(!guided){
  const fold=(node,title,cls)=>{if(!node)return;const details=document.createElement('details'),summary=document.createElement('summary');details.className=cls;summary.textContent=title;node.before(details);details.append(summary,node);};
  fold(document.querySelector('.card'),'Installa soltanto il firmware dal browser','direct-install');
  fold(document.querySelector('#look-sd'),'Look delle classi e scheda SD · opzionale','sd-choice');
  const extras=[...document.querySelectorAll('main>section.after')];if(extras.length){const details=document.createElement('details'),summary=document.createElement('summary');details.className='release-history';summary.textContent='Anteprime, novità e limiti delle versioni';extras[0].before(details);details.append(summary,...extras);}
}

const supported=window.isSecureContext&&'serial' in navigator;
function ready(){button.disabled=!supported||!prepared||busy||!$('#board').checked;}
function message(text,error=false){status.textContent=text;status.dataset.error=error?'true':'false';}
function phase(name,value){if(name==='done'||name==='done-reset')announce('verified');progress.value=value;const labels={connect:'Connessione al dispositivo…',layout:'Controllo delle partizioni…',write:`Installazione e verifica · ${value}%`,restart:'Firmware verificato. Riavvio…',done:'Installazione completata. Controlla che PocketD-D compaia sullo schermo.','done-reset':'Firmware scritto e verificato. Riavvia il dispositivo con PWR e controlla lo schermo.'};message(labels[name]);}
async function prepare(){
  message('Preparo il firmware…');$('#retry').hidden=true;
  try{prepared=await prepareFirmware(new URL('./manifest.json',location.href));$('#version').textContent=`Firmware ${prepared.manifest.version}`;message(supported?'Firmware pronto. Collega il dispositivo e premi Installa.':'Apri questa pagina in Chrome o Edge su PC, tramite HTTPS o localhost.');}
  catch(error){message(error.message,true);$('#retry').hidden=false;}
  ready();
}
$('#board').addEventListener('change',ready);$('#retry').addEventListener('click',prepare);
button.addEventListener('click',async()=>{
  if(button.disabled||busy)return;
  busy=true;announce('busy',true);ready();$('#retry').hidden=true;for(const el of document.querySelectorAll('input'))el.disabled=true;
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
    window.removeEventListener('beforeunload',unload);busy=false;announce('busy',false);for(const el of document.querySelectorAll('input'))el.disabled=false;ready();
  }
});
prepare();

const pcLink=document.querySelector('[download="PocketDD-PC.zip"]');
if(pcLink)pcLink.addEventListener('click',async event=>{event.preventDefault();if(pcLink.getAttribute('aria-disabled')==='true')return;const status=document.querySelector('#pc-download-status');pcLink.setAttribute('aria-disabled','true');status.textContent='Preparo il download…';try{const data=await preparePcPackage(new URL('./downloads/pc-package.json',location.href),{fetcher:url=>fetch(url,{signal:AbortSignal.timeout(90000),cache:'no-store'}),onProgress:n=>status.textContent=`Scarico e verifico il pacchetto · ${n}%`});const url=URL.createObjectURL(new Blob([data],{type:'application/zip'})),a=document.createElement('a');a.href=url;a.download='PocketDD-PC.zip';a.click();setTimeout(()=>URL.revokeObjectURL(url),60000);status.textContent='Pacchetto verificato. Estrai tutto lo ZIP e apri Avvia-PocketDD.cmd.';}catch(error){status.textContent=/^(Pacchetto|Descrizione|Download|Verifica)/.test(error.message??'')?error.message:'Download non riuscito. Ricarica la pagina e riprova.';}finally{pcLink.removeAttribute('aria-disabled');}});
