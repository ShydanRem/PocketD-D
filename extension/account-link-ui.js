import {preparePocketAccount,pairPocketAccount} from './account-link.js';
import {ACCOUNT_ORIGINS} from './account-check.js';
import {observeUsbPort,usbPortState} from './usb-port.js';
const find=id=>document.getElementById(id);let controller,generation=0,granted=false,linking=false;
function refresh(){const state=usbPortState(),version=/^(\d+)\.(\d+)\./.exec(state.firmware??''),old=version&&Number(version[1])===0&&Number(version[2])<92;
 if(old)find('link-status').textContent='Per Beyond aggiorna il firmware del Pocket alla 0.92 o successiva dal sito PocketD-D.';find('prepare').hidden=granted;find('prepare').disabled=linking||usbPortState().busy;find('connect').hidden=!granted;find('connect').disabled=!granted||linking||state.busy||Boolean(old);find('cancel-link').hidden=!linking;}
function progress(stage){
 const index={start:0,account:0,code:1,link:2,ready:3,error:-1}[stage];
 for(const step of document.querySelectorAll('[data-pair-step]')){const n=Number(step.dataset.pairStep);step.dataset.state=index===-1?(step.dataset.state==='current'?'error':step.dataset.state):n<index?'done':n===index?'current':'waiting';if(n===index)step.setAttribute('aria-current','step');else step.removeAttribute('aria-current');}
 find('pair-finish').hidden=stage!=='ready';find('pair-next').hidden=stage!=='code';
}
observeUsbPort(refresh);
async function permissions(){try{granted=await chrome.permissions.contains({permissions:['cookies'],origins:ACCOUNT_ORIGINS});}catch{granted=false;}refresh();}
export function cancelPocketLink(){generation++;controller?.abort();linking=false;find('pair-code').textContent='';progress('start');permissions();}
find('prepare').onclick=async()=>{try{granted=await chrome.permissions.request({permissions:['cookies'],origins:ACCOUNT_ORIGINS});find('link-status').textContent=granted?'Consenso ricevuto. Premi Collega Beyond.':'Consenso non concesso. Puoi comunque usare gli altri strumenti Pocket.';}catch{find('link-status').textContent='Controlla i permessi dell’estensione e riprova.';}refresh();};
find('connect').onclick=async()=>{
 if(linking||usbPortState().busy)return;const run=++generation;controller=new AbortController();linking=true;progress('account');refresh();
 try{await pairPocketAccount(()=>preparePocketAccount({signal:controller.signal,requestPermission:false}),{signal:controller.signal,onCode:code=>{if(run===generation){find('pair-code').textContent=code;progress('code');find('link-status').textContent='Confronta il codice sul Pocket e conferma sul display.';}},onStage:stage=>{if(run===generation){const labels={account:'Verifico il tuo accesso Beyond…',code:'Attendo il Pocket…',link:'Il Pocket verifica l’account via Wi-Fi…'};if(labels[stage]){progress(stage);find('link-status').textContent=labels[stage];}}}});if(run===generation){progress('ready');find('link-status').textContent='Beyond collegato. Scegli il PG sul Pocket: ora puoi scollegare il PC.';}}
 catch(error){if(run===generation){progress('error');find('link-status').textContent=error.message||'Collegamento non confermato. Controlla il display del Pocket.';permissions();}}
 finally{if(run===generation){linking=false;find('pair-code').textContent='';find('pair-next').hidden=true;refresh();}}
};
find('cancel-link').onclick=()=>{cancelPocketLink();find('link-status').textContent='Operazione interrotta. Controlla il Pocket se avevi già confermato.';};
window.addEventListener('pagehide',cancelPocketLink);progress('start');permissions();
