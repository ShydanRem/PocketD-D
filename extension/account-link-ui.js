import {preparePocketAccount,pairPocketAccount} from './account-link.js';
import {ACCOUNT_ORIGINS} from './account-check.js';
import {observeUsbPort,usbPortState,forgetUsbPort} from './usb-port.js';
const find=id=>document.getElementById(id);
let controller,generation=0,permissionGeneration=0,granted=false,linking=false,preparing=false,stage='start';
const recovery={
 login:['Sul PC · accedi nuovamente a Beyond','Ho effettuato accesso · riprova'],
 permission:['Sul PC · consenti accesso Beyond','Riprova collegamento'],
 wifi:['Sul Pocket · collega il Wi-Fi, poi torna qui','Wi-Fi pronto · riprova'],
 clock:['Sul Pocket · controlla Wi-Fi e Internet','Riprova collegamento'],
 busy:['Sul Pocket · termina l’operazione aperta','Pocket pronto · riprova'],
 restart:['Sul Pocket · attendi la schermata del personaggio','Pocket pronto · riprova'],
 storage:['Sul Pocket · controlla il salvataggio e riavvia','Pocket riavviato · riprova'],
 memory:['Sul Pocket · riavvia e riprova','Pocket riavviato · riprova'],
 rate:['Attendi qualche minuto prima di riprovare','Riprova collegamento'],
 protocol:['Sul PC · aggiorna firmware ed estensione','Riprova collegamento'],
 chooser:['Sul PC · scegli la porta USB del Pocket','Scegli Pocket USB e collega →']
};
function focusBeyond(){
 const focus=location.hash==='#beyond';document.body.classList.toggle('beyond-focus',focus);
 find('show-tools').hidden=!focus;find('show-beyond').hidden=focus;
 find('workspace-title').textContent=focus?'Beyond sul Pocket, passo per passo.':'Il tuo Pocket, tutto qui.';
}
find('show-tools').onclick=()=>{location.hash='';focusBeyond();document.querySelector('.device-card').scrollIntoView({behavior:'smooth'});};
find('show-beyond').onclick=()=>{location.hash='beyond';focusBeyond();find('beyond').scrollIntoView({behavior:'smooth'});};
document.querySelector('a[href="#wifi-help"]').onclick=event=>{event.preventDefault();find('wifi-help').open=true;find('wifi-help').scrollIntoView({behavior:'smooth'});};
window.addEventListener('hashchange',focusBeyond);focusBeyond();
function refresh(){
 const state=usbPortState(),version=/^(\d+)\.(\d+)\./.exec(state.firmware??''),old=version&&Number(version[1])===0&&Number(version[2])<92;
 if(old)find('link-status').textContent='Per Beyond aggiorna il firmware del Pocket alla 0.92 o successiva dal sito PocketD-D.';
 const busy=linking||state.busy||preparing;
 find('prepare').hidden=granted||linking||stage==='ready';find('prepare').disabled=busy;
 find('connect').hidden=!granted||linking||stage==='ready';find('connect').disabled=!granted||busy||Boolean(old);
 find('cancel-link').hidden=!linking;find('pair-change-port').hidden=!state.known||linking||stage==='ready';find('pair-change-port').disabled=busy;
 if(stage==='start')find('pair-instruction').textContent=granted?'Sul PC · scegli il Pocket nella finestra USB':'Sul PC · consenti l’accesso al tuo account';
}
function progress(next){
 stage=next;const index={start:0,account:0,usb:0,challenge:0,code:1,link:2,ready:3,error:-1}[stage];
 for(const step of document.querySelectorAll('[data-pair-step]')){
  const n=Number(step.dataset.pairStep);step.dataset.state=index===-1?(step.dataset.state==='current'?'error':step.dataset.state):n<index?'done':n===index?'current':'waiting';
  if(n===index)step.setAttribute('aria-current','step');else step.removeAttribute('aria-current');
 }
 find('pair-finish').hidden=stage!=='ready';find('pair-again').hidden=stage!=='ready';find('pair-next').hidden=stage!=='code';find('pair-start').hidden=!['start','error'].includes(stage);find('link-retry-help').hidden=stage!=='error';
 find('pair-login').hidden=true;find('pair-update').hidden=true;
 const instructions={account:'Sul PC · verifico il tuo accesso Beyond',usb:'Collego il Pocket via USB',challenge:'Attendi · apro il codice sul Pocket',code:'Sul Pocket · confronta il codice e premi Collega',link:'Attendi · il Pocket verifica Beyond via Wi-Fi',ready:'Beyond collegato ✓',error:'Da completare · controlla cosa è successo'};
 if(instructions[stage])find('pair-instruction').textContent=instructions[stage];
 if(['code','ready','error'].includes(stage)){find('pair-instruction').focus({preventScroll:true});find('beyond').scrollIntoView({block:'start'});}
}
observeUsbPort(refresh);
async function permissions(){
 const run=++permissionGeneration;
 try{const value=await chrome.permissions.contains({permissions:['cookies'],origins:ACCOUNT_ORIGINS});if(run===permissionGeneration)granted=value;}catch{if(run===permissionGeneration)granted=false;}
 if(run===permissionGeneration)refresh();
}
export function cancelPocketLink(){generation++;controller?.abort();linking=false;find('pair-code').textContent='';progress('start');permissions();}
find('prepare').onclick=async()=>{
 if(preparing||linking||usbPortState().busy)return;permissionGeneration++;preparing=true;refresh();
 try{granted=await chrome.permissions.request({permissions:['cookies'],origins:ACCOUNT_ORIGINS});find('link-status').textContent=granted?'Accesso consentito. Ora premi Scegli Pocket USB e seleziona la porta del dispositivo.':'Accesso non consentito. Premi di nuovo Inizia e consenti l’accesso Beyond nel browser.';}
 catch{find('link-status').textContent='Controlla i permessi dell’estensione e riprova.';}
 finally{preparing=false;refresh();}
};
find('pair-again').onclick=()=>{if(linking||usbPortState().busy)return;forgetUsbPort();cancelPocketLink();find('connect').textContent='Scegli Pocket USB e collega →';find('link-status').textContent='Pronto per un altro Pocket. Il collegamento del dispositivo precedente resta conservato.';};
find('pair-change-port').onclick=()=>{
 if(linking||usbPortState().busy)return;forgetUsbPort();progress('start');find('connect').textContent='Scegli Pocket USB e collega →';
 find('link-status').textContent='Porta precedente dimenticata. Premi Scegli Pocket USB per selezionare il dispositivo corretto.';refresh();
};
find('connect').onclick=async()=>{
 if(linking||preparing||usbPortState().busy)return;const run=++generation;controller=new AbortController();linking=true;progress('usb');refresh();
 try{
  await pairPocketAccount(()=>preparePocketAccount({signal:controller.signal,requestPermission:false}),{
   signal:controller.signal,
   onCode:code=>{if(run===generation){find('pair-code').textContent=code;progress('code');find('link-status').textContent='Confronta il codice sul Pocket e conferma sul display entro due minuti.';}},
   onStage:next=>{if(run===generation){const labels={account:'Verifico il tuo accesso Beyond…',usb:'Collego la porta USB scelta…',challenge:'Attendo la risposta USB: il codice non è ancora pronto.',link:'Il Pocket verifica l’account via Wi-Fi…'};if(labels[next]){progress(next);find('link-status').textContent=labels[next];}}}
  });
  if(run===generation){progress('ready');find('link-status').textContent='Beyond collegato. Scegli il PG sul Pocket: ora puoi scollegare il PC.';}
 }catch(error){
  if(run===generation){progress('error');find('link-status').textContent=error.message||'Collegamento non confermato. Controlla il display del Pocket.';
   const advice=recovery[error.code];if(advice)find('pair-instruction').textContent=advice[0];find('connect').textContent=advice?.[1]??'Riprova collegamento';
   find('pair-login').hidden=!['login','identity'].includes(error.code);find('pair-update').hidden=!['protocol','catalog','format'].includes(error.code);permissions();}
 }finally{if(run===generation){linking=false;find('pair-code').textContent='';find('pair-next').hidden=true;refresh();}}
};
find('cancel-link').onclick=()=>{cancelPocketLink();find('link-status').textContent='Operazione interrotta. Controlla il Pocket se avevi già confermato.';};
chrome.permissions.onAdded?.addListener(permissions);chrome.permissions.onRemoved?.addListener(permissions);
window.addEventListener('focus',()=>{if(!linking&&!preparing)permissions();});
window.addEventListener('pagehide',cancelPocketLink);progress('start');permissions();
