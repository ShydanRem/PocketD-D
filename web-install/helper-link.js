// The site only learns whether the helper is present. No session, credential,
// character or serial messages are accepted through the extension bridge.
export function helperRequest(action,{win=window,timeout=1500,view}={}){
 if(!['ping','open'].includes(action))return Promise.resolve(null);
 const id=Array.from(crypto.getRandomValues(new Uint8Array(16)),b=>b.toString(16).padStart(2,'0')).join('');
 return new Promise(resolve=>{let timer;function done(value){clearTimeout(timer);win.removeEventListener('message',receive);resolve(value);}function receive(e){if(e.source!==win||e.origin!==win.location.origin||e.data?.type!=='pocketdd-helper-response'||e.data.id!==id)return;const d=e.data;done(action==='ping'&&d.installed===true&&/^\d+\.\d+\.\d+$/.test(d.version)?{installed:true,version:d.version,...(d.beyondEntry===true?{beyondEntry:true}:{})}:action==='open'?{opened:d.opened===true}:null);}win.addEventListener('message',receive);timer=setTimeout(()=>done(null),timeout);try{win.postMessage({type:'pocketdd-helper-request',id,action,...(view==='beyond'?{view}:{})},win.location.origin);}catch{done(null);}});
}
export function supportsCurrentHelper(version){
 if(typeof version!=='string'||!/^\d+\.\d+\.\d+$/.test(version))return false;
 const parts=version.split('.').map(Number),minimum=[0,116,0];for(let i=0;i<3;i++){if(parts[i]!==minimum[i])return parts[i]>minimum[i];}return true;
}
export function bindPocketHub(root,{blocked=()=>false}={}){
 const update='Aggiorna il componente Pocket PC: segui le istruzioni qui sotto, poi premi Continua.';
 const open=root.querySelector('[data-open-pocket]'),help=root.querySelector('[data-helper-help]'),status=root.querySelector('[data-helper-status]');let active=false,interacted=false,helpView='beyond';const beyond=root.querySelector('[data-open-beyond]');
 const resumeKey='pocketdd-continue-view';let resume;
 try{resume=sessionStorage.getItem(resumeKey);sessionStorage.removeItem(resumeKey);}catch{/* Storage is optional; manual buttons remain available. */}
 if(['tools','beyond'].includes(resume))helpView=resume;
 if(!globalThis.isSecureContext||!globalThis.navigator?.serial?.requestPort){open.disabled=true;if(beyond)beyond.disabled=true;status.textContent='Apri questo link in Chrome o Edge su PC, tramite HTTPS o localhost.';return;}
 const reveal=message=>{help.hidden=false;status.textContent=message;help.scrollIntoView({behavior:'smooth',block:'nearest'});help.focus({preventScroll:true});};
 helperRequest('ping').then(async result=>{
  if(interacted||blocked())return;
  if(['tools','beyond'].includes(resume)){
   interacted=true;
   if(!result?.installed){reveal('Estensione non rilevata dopo il ricaricamento. Controlla i passaggi qui sotto, poi premi Continua.');return;}
   if(!supportsCurrentHelper(result.version)||helpView==='beyond'&&!result.beyondEntry){reveal(update);return;}
   active=true;open.disabled=true;if(beyond)beyond.disabled=true;
   try{const opened=await helperRequest('open',helpView==='beyond'?{view:'beyond'}:{});
    if(opened?.opened){help.hidden=true;status.textContent=helpView==='beyond'?'Collegamento Beyond aperto. Continua nella nuova scheda.':'Pocket PC aperto. Continua con backup e look nella nuova scheda.';}else reveal('Estensione rilevata. Premi Continua per aprire Pocket PC.');
   }finally{active=false;open.disabled=false;if(beyond)beyond.disabled=false;}
   return;
  }
  if(result?.installed){if(!supportsCurrentHelper(result.version)){help.hidden=false;status.textContent=update;return;}status.textContent='Estensione pronta. Premi Associa Beyond: il codice si aprirà sul Pocket.';}
 });
 open.onclick=async()=>{if(active||blocked())return;helpView='tools';active=true;interacted=true;open.disabled=true;status.textContent='Apro Pocket PC…';try{const helper=await helperRequest('ping');if(helper?.installed&&!supportsCurrentHelper(helper.version)){help.hidden=false;status.textContent=update;return;}const result=await helperRequest('open');if(result?.opened){help.hidden=true;status.textContent='Pocket PC aperto. Continua nella nuova scheda: collegamento, Beyond, SD e backup sono lì.';}else{help.hidden=false;status.textContent='La prima volta aggiungi il componente Pocket a Chrome o Edge. Poi apri Pocket PC.';}}finally{active=false;open.disabled=false;}};
 if(beyond)beyond.onclick=async()=>{
  if(active||blocked())return;helpView='beyond';active=true;interacted=true;open.disabled=beyond.disabled=true;status.textContent='Apro il collegamento Beyond…';
  try{const helper=await helperRequest('ping');
   if(helper?.installed&&(!helper.beyondEntry||!supportsCurrentHelper(helper.version))){reveal(update);return;}
   const result=helper?.installed?await helperRequest('open',{view:'beyond'}):null;
   if(result?.opened){help.hidden=true;status.textContent='Collegamento Beyond aperto. Continua nella nuova scheda e conferma il codice sul Pocket.';}
   else{reveal('Prima volta: aggiungi l’estensione con i passaggi qui sotto. Poi premi Continua.');}
  }finally{active=false;open.disabled=beyond.disabled=false;}
 };
 root.querySelector('[data-helper-reload]').onclick=async()=>{
  if(active||blocked())return;active=true;interacted=true;const button=root.querySelector('[data-helper-reload]');button.disabled=true;
  try{const helper=await helperRequest('ping');if(!helper?.installed){status.textContent='Ricarico la pagina per rilevare l’estensione e continuare.';try{sessionStorage.setItem(resumeKey,helpView);}catch{}location.reload();return;}
   if(!helper.beyondEntry||!supportsCurrentHelper(helper.version)){reveal(update);return;}
   const result=await helperRequest('open',helpView==='beyond'?{view:'beyond'}:{});if(result?.opened){help.hidden=true;status.textContent=helpView==='beyond'?'Collegamento Beyond aperto. Continua nella nuova scheda.':'Pocket PC aperto. Continua con backup e look nella nuova scheda.';}else reveal('Estensione presente, ma la pagina non si apre. Ricarica l’estensione e questa pagina, poi riprova.');
  }finally{active=false;button.disabled=false;}
 };
}
