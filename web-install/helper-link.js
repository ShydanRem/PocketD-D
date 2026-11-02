// The site only learns whether the helper is present. No session, credential,
// character or serial messages are accepted through the extension bridge.
export function helperRequest(action,{win=window,timeout=1500}={}){
 if(!['ping','open'].includes(action))return Promise.resolve(null);
 const id=Array.from(crypto.getRandomValues(new Uint8Array(16)),b=>b.toString(16).padStart(2,'0')).join('');
 return new Promise(resolve=>{let timer;function done(value){clearTimeout(timer);win.removeEventListener('message',receive);resolve(value);}function receive(e){if(e.source!==win||e.origin!==win.location.origin||e.data?.type!=='pocketdd-helper-response'||e.data.id!==id)return;const d=e.data;done(action==='ping'&&d.installed===true&&/^\d+\.\d+\.\d+$/.test(d.version)?{installed:true,version:d.version}:action==='open'?{opened:d.opened===true}:null);}win.addEventListener('message',receive);timer=setTimeout(()=>done(null),timeout);try{win.postMessage({type:'pocketdd-helper-request',id,action},win.location.origin);}catch{done(null);}});
}
export function bindPocketHub(root,{blocked=()=>false}={}){
 const open=root.querySelector('[data-open-pocket]'),help=root.querySelector('[data-helper-help]'),status=root.querySelector('[data-helper-status]');let active=false;
 if(!globalThis.isSecureContext||!globalThis.navigator?.serial?.requestPort){open.disabled=true;status.textContent='Apri questo link in Chrome o Edge su PC, tramite HTTPS o localhost.';return;}
 helperRequest('ping').then(result=>{if(result?.installed){open.textContent='Apri Pocket PC';status.textContent='Componente Pocket rilevato. Apri gli strumenti e scegli la porta una volta.';}});
 open.onclick=async()=>{if(active||blocked())return;active=true;open.disabled=true;status.textContent='Apro Pocket PC…';try{const result=await helperRequest('open');if(result?.opened){help.hidden=true;status.textContent='Pocket PC aperto. Continua nella nuova scheda: collegamento, Beyond, SD e backup sono lì.';}else{help.hidden=false;status.textContent='La prima volta aggiungi il componente Pocket a Chrome o Edge. Poi apri Pocket PC.';}}finally{active=false;open.disabled=false;}};
 root.querySelector('[data-helper-reload]').onclick=()=>{if(!blocked())location.reload();};
}
