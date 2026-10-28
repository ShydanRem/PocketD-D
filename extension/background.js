import {pageAdapter} from './page-adapter.js';import {brokerRoll} from './protocol.js';
const local=chrome.storage.local;
async function api(path,data,connection){const r=await fetch(connection.url+path,{method:'POST',headers:{'Content-Type':'application/json',...(connection.token?{Authorization:'Bearer '+connection.token}:{})},body:JSON.stringify(data),signal:AbortSignal.timeout(10000)});if(!r.ok)throw Error('Ponte non disponibile o abbinamento scaduto');return r.json();}
const busy=new Set();
chrome.runtime.onMessage.addListener((m,sender,reply)=>{(async()=>{
 if(m.type==='connect'&&!sender.tab){if(!/^http:\/\/127\.0\.0\.1:\d{4,5}$/.test(m.url)||!/^\d{9}$/.test(m.code))throw Error('Indirizzo locale o codice non validi');const c={url:m.url};const r=await api('/extension/connect',{code:m.code},c);await local.set({connection:{url:m.url,token:r.token},receipts:{}});return {ok:true};}
 if(m.type!=='tick'||!sender.tab||!/^https:\/\/www\.dndbeyond\.com\/(?:profile\/[^/]+\/)?characters\/[1-9]\d*/.test(sender.url??''))throw Error('Pagina non supportata');
 const {connection,receipts={}}=await local.get(['connection','receipts']);if(!connection)return {state:'Abbina l’estensione dal popup'};
 if(busy.has(sender.tab.id))return {state:'Invio in corso'};busy.add(sender.tab.id);
 try{
  // Only receipts are retried; execution is never retried after a lost acknowledgement.
  for(const [id,r] of Object.entries(receipts))if(r.pending){await api('/extension/report',{id,lease:r.lease,outcome:r.outcome},connection);r.pending=false;await local.set({receipts});}
  const [read]=await chrome.scripting.executeScript({target:{tabId:sender.tab.id},world:'MAIN',func:pageAdapter,args:[{type:'read'}]});if(!read?.result)return {state:'Apri la gestione PF Beyond (layout supportato)'};
  const {event}=await api('/extension/next',{observation:read.result,owner:`${sender.tab.id}:${m.instance}`},connection);if(!event)return {state:'In attesa del ponte'};
  if(receipts[event.id])return {state:'Evento già tentato; controlla il ponte'};
  receipts[event.id]={lease:event.lease,outcome:'uncertain',pending:true};await local.set({receipts});
  let outcome='uncertain';try{if(Date.now()-event.claimedAt>10000||Date.now()<event.claimedAt)throw Error('Tentativo scaduto');const roll=event.kind==='roll'?brokerRoll(event.roll):null;const [result]=await chrome.scripting.executeScript({target:{tabId:sender.tab.id},world:'MAIN',func:pageAdapter,args:[{type:'apply',event:{...event,lease:undefined,owner:undefined},roll}]});outcome=result?.result?.outcome??'uncertain';}catch{outcome='uncertain';}
  receipts[event.id].outcome=outcome;await local.set({receipts});const r=await api('/extension/report',{id:event.id,lease:event.lease,outcome},connection);receipts[event.id].pending=false;
  const keys=Object.keys(receipts);if(keys.length>256)for(const id of keys.slice(0,keys.length-256))if(!receipts[id].pending)delete receipts[id];await local.set({receipts});return {state:r.state};
 }finally{busy.delete(sender.tab.id);}
})().then(reply,e=>reply({error:e.message??'Ponte non disponibile'}));return true;});
