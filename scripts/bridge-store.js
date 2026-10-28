import {mkdirSync,readFileSync,openSync,writeFileSync,fsyncSync,closeSync,renameSync,chmodSync} from 'node:fs';
import {join} from 'node:path';import {createHash,randomBytes,randomUUID,timingSafeEqual} from 'node:crypto';
import {syncSnapshot,vitals,sameVitals,brokerRoll,syncRoll} from '../src/sync-protocol.js';
export const digest=x=>createHash('sha256').update(x).digest('hex');
export const secret=()=>randomBytes(32).toString('hex');
export function equalsSecret(token,hash){return typeof token==='string'&&/^[a-f0-9]{64}$/.test(token)&&typeof hash==='string'&&/^[a-f0-9]{64}$/.test(hash)&&timingSafeEqual(Buffer.from(digest(token),'hex'),Buffer.from(hash,'hex'));}
function validateStored(s){
 if(!s||s.version!==1||!Array.isArray(s.bindings)||s.bindings.length>4||!Array.isArray(s.events)||s.events.length>256||s.extensionHash!==null&&!/^[a-f0-9]{64}$/.test(s.extensionHash))throw Error('Formato ponte non supportato');
 const ids=new Set(),characters=new Set();for(const b of s.bindings){if(!b||!/^[a-f0-9]{32}$/.test(b.id)||ids.has(b.id)||characters.has(b.characterId)||!/^[1-9]\d{0,14}$/.test(b.characterId)||!/^[a-f0-9]{64}$/.test(b.hash)||typeof b.enabled!=='boolean'||!['user','game'].includes(b.scope)||!Number.isSafeInteger(b.cursor)||b.cursor<0||b.cursor>4294967295||!Array.isArray(b.seenBoots)||b.seenBoots.length>64||b.seenBoots.some(v=>!/^[a-f0-9]{16}$/.test(v)))throw Error('Abbinamento non valido');ids.add(b.id);characters.add(b.characterId);if(b.snapshot)syncSnapshot(b.snapshot);if(b.remote)vitals(b.remote);}
 const events=new Set();for(const e of s.events){if(!e||typeof e.id!=='string'||e.id.length>150||events.has(e.id)||!ids.has(e.binding)||!['queued','claimed','observed','verified','uncertain','conflict','unsupported','manual'].includes(e.status)||!['hp','roll'].includes(e.kind))throw Error('Evento non valido');events.add(e.id);if(e.kind==='roll')syncRoll(e.roll);else{vitals(e.before);vitals(e.after);}if(e.status==='claimed'&&(!Number.isSafeInteger(e.claimedAt)||!/^[a-f0-9]{64}$/.test(e.lease)))throw Error('Ricevuta non valida');}
}
export class BridgeStore{
 constructor(directory,{now=()=>Date.now(),save=null}={}){
  this.live=new Map();this.observations=new Map();this.now=now;this.directory=directory;mkdirSync(directory,{recursive:true,mode:0o700});
  this.file=join(directory,'bridge-state.json');this.save=save??(v=>{const p=this.file+'.new',fd=openSync(p,'w',0o600);try{writeFileSync(fd,JSON.stringify(v));fsyncSync(fd)}finally{closeSync(fd)}renameSync(p,this.file);chmodSync(this.file,0o600);});
  try{this.data=JSON.parse(readFileSync(this.file,'utf8'));validateStored(this.data);}catch(e){if(e.code!=='ENOENT')throw Error('Memoria ponte non leggibile: conservata senza sovrascriverla');this.data={version:1,bindings:[],events:[],extensionHash:null};this.save(this.data);}
 }
 change(fn){const next=structuredClone(this.data),result=fn(next);this.save(next);this.data=next;return result;}
 binding(token){return this.data.bindings.find(b=>equalsSecret(token,b.hash));}
 addBinding(characterId,scope='user'){
  if(!/^[1-9]\d{0,14}$/.test(characterId)||!['user','game'].includes(scope))throw Error('PG o destinazione non validi');const token=secret(),id=randomBytes(16).toString('hex');
  this.change(s=>{if(s.bindings.length>=4)throw Error('Massimo quattro abbinamenti; revoca un abbinamento precedente');if(s.bindings.some(b=>b.characterId===characterId))throw Error('PG già abbinato: revoca prima il collegamento precedente');s.bindings.push({id,characterId,scope,hash:digest(token),enabled:false,paused:false,reason:'baseline',snapshot:null,cursor:0,remote:null,seenBoots:[]});});return {id,token,characterId};
 }
 extension(){const token=secret();this.change(s=>{s.extensionHash=digest(token);for(const b of s.bindings)b.enabled=false;for(const e of s.events)if(e.status==='claimed')e.status='uncertain';});return token;}
 observe(id,raw){const snapshot=syncSnapshot(raw);const old=this.data.bindings.find(b=>b.id===id);if(old?.snapshot&&JSON.stringify(old.snapshot)===JSON.stringify(snapshot)){this.live.set(id,this.now());return this.statusOf(this.data,old);}const result=this.change(s=>{
  const b=s.bindings.find(b=>b.id===id);if(!b)throw Error('Abbinamento revocato');const prev=b.snapshot;
  if(prev&&prev.boot===snapshot.boot&&snapshot.counter<=prev.counter){if(snapshot.counter===prev.counter&&JSON.stringify(snapshot)===JSON.stringify(prev))return this.statusOf(s,b);throw Error('Osservazione vecchia o discordante');}
  if(prev&&(prev.boot!==snapshot.boot||prev.epoch!==snapshot.epoch||snapshot.sequence<prev.sequence)){if(b.seenBoots.includes(snapshot.boot)&&prev.boot!==snapshot.boot)throw Error('Sessione dispositivo precedente');b.enabled=false;b.reason='baseline';}
  if(prev&&prev.boot!==snapshot.boot){b.seenBoots.push(prev.boot);if(b.seenBoots.length>64)throw Error('Rinnova l’abbinamento: troppe sessioni');}
  b.snapshot=snapshot;
  if(!b.enabled)return this.statusOf(s,b);
  if(snapshot.sequence-b.cursor>20){b.enabled=false;b.reason='history-gap';return this.statusOf(s,b);}
  const add=e=>{if(s.events.length>=256)throw Error('Coda piena: risolvere o archiviare gli eventi');s.events.push({...e,binding:b.id,characterId:b.characterId,scope:b.scope,status:'queued',created:this.now()});};
  for(const r of [...snapshot.rolls].reverse())if(r.sequence>b.cursor){const key=`${b.id}:${snapshot.epoch}:roll:${r.sequence}`;if(!s.events.some(e=>e.id===key)){let supported=true;try{brokerRoll(r)}catch{supported=false;}add({id:key,remoteId:randomUUID(),kind:'roll',roll:r,...(!supported?{status:'unsupported'}:{})});if(!supported)s.events.at(-1).status='unsupported';}}
  b.cursor=snapshot.sequence;
  if(prev&&!sameVitals(prev.vitals,snapshot.vitals))add({id:`${b.id}:${snapshot.boot}:hp:${snapshot.counter}`,kind:'hp',before:prev.vitals,after:snapshot.vitals});
  return this.statusOf(s,b);
 });this.live.set(id,this.now());return this.statusOf(this.data,this.data.bindings.find(b=>b.id===id));}
 statusOf(s,b){const events=s.events.filter(e=>e.binding===b.id),problem=events.find(e=>['uncertain','conflict','unsupported'].includes(e.status));return {state:!b.enabled?b.reason:problem?problem.status:b.paused?'paused':[this.live.get(b.id),this.observations.get(b.id)?.at].some(at=>at===undefined||this.now()-at<0||this.now()-at>10000)?'offline':'ready',queued:events.filter(e=>e.status==='queued').length,observed:events.filter(e=>e.status==='observed'||e.status==='verified').length};}
 approve(id){return this.change(s=>{const b=s.bindings.find(b=>b.id===id),ob=this.observations.get(id);
  if(b?.enabled&&b.paused){if(!ob||this.now()-ob.at>10000||this.now()<ob.at||!sameVitals(b.remote,ob.vitals)||s.events.some(e=>e.binding===id&&['claimed','uncertain','conflict','unsupported'].includes(e.status)))throw Error('Risolvi gli eventi e verifica i PF Beyond prima di riprendere la coda');b.paused=false;return;}
  if(!b?.snapshot||!ob||this.now()-ob.at>10000||this.now()<ob.at||!sameVitals(b.snapshot.vitals,ob.vitals))throw Error('Apri la gestione PF Beyond: i valori devono coincidere prima dell’avvio');if(s.events.some(e=>e.binding===id&&['claimed','uncertain','conflict','queued'].includes(e.status)))throw Error('Risolvi prima gli eventi precedenti');b.remote=ob.vitals;b.cursor=b.snapshot.sequence;b.enabled=true;b.paused=false;b.reason='ready';});}
 next(observation,owner){
  if(!observation||!/^[1-9]\d{0,14}$/.test(observation.characterId)||typeof owner!=='string'||!/^[a-zA-Z0-9:_-]{1,100}$/.test(owner))throw Error('Pagina non valida');const hp=vitals(observation.vitals),b=this.data.bindings.find(b=>b.characterId===observation.characterId);if(!b)return null;
  this.observations.set(b.id,{vitals:hp,at:this.now()});this.expireClaims();
  const events=this.data.events.filter(e=>e.binding===b.id),age=this.now()-(this.live.get(b.id)??0);
  if(!b.enabled||b.paused||age<0||age>10000||events.some(e=>['claimed','uncertain','conflict','unsupported'].includes(e.status)))return null;
  const next=events.find(e=>e.status==='queued');if(!next)return null;
  return this.change(s=>{const e=s.events.find(e=>e.id===next.id);if(!sameVitals(hp,e.kind==='hp'?e.before:b.remote)){e.status='conflict';return null;}e.status='claimed';e.owner=owner;e.lease=secret();e.claimedAt=this.now();return structuredClone(e);});
 }
 report(id,lease,outcome,verified=null){return this.change(s=>{const e=s.events.find(e=>e.id===id);if(!e)return {state:'revoked'};if(!equalsSecret(lease,digest(e.lease??'')))throw Error('Ricevuta non riconosciuta');if(['observed','verified','uncertain','unsupported','conflict','manual'].includes(e.status))return {state:e.status};if(e.status!=='claimed')throw Error('Evento non consegnato');
  e.status=e.kind==='roll'&&outcome==='broker-observed'?'observed':e.kind==='hp'&&['dom-updated','uncertain'].includes(outcome)&&sameVitals(verified,e.after)?'verified':outcome==='unsupported'?'unsupported':outcome==='conflict'?'conflict':'uncertain';
  if(e.status==='verified')s.bindings.find(b=>b.id===e.binding).remote=e.after;return {state:e.status};});}
 pause(id){this.change(s=>{const b=s.bindings.find(b=>b.id===id);if(!b)throw Error('Abbinamento assente');b.paused=true;});}
 // Explicit user reconciliation, never a resend. Current Pocket and freshly read remote must match.
 reconcile(id){this.expireClaims();this.change(s=>{if(s.events.some(e=>e.binding===id&&e.status==='claimed'))throw Error('Risolvi dopo la conclusione dell’invio in corso');const b=s.bindings.find(b=>b.id===id),ob=this.observations.get(id);if(!b?.snapshot||!ob||this.now()-ob.at>10000||this.now()<ob.at||!sameVitals(b.snapshot.vitals,ob.vitals))throw Error('PF non coincidenti: correggili e aggiorna la pagina');for(const e of s.events)if(e.binding===id&&e.status!=='verified'&&e.status!=='observed')e.status='manual';b.enabled=false;b.reason='baseline';});}
 revoke(id){this.change(s=>{s.bindings=s.bindings.filter(b=>b.id!==id);s.events=s.events.filter(e=>e.binding!==id);});}
 archive(){this.change(s=>{s.events=s.events.filter(e=>!['verified','observed','manual'].includes(e.status));});}
 expireClaims(){if(this.data.events.some(e=>e.status==='claimed'&&(this.now()-e.claimedAt>20000||this.now()<e.claimedAt)))this.change(s=>{for(const e of s.events)if(e.status==='claimed'&&(this.now()-e.claimedAt>20000||this.now()<e.claimedAt))e.status='uncertain';});}
 publicState(){this.expireClaims();return {bindings:this.data.bindings.map(b=>({id:b.id,characterId:b.characterId,scope:b.scope,enabled:b.enabled,status:this.statusOf(this.data,b),snapshot:b.snapshot?{name:b.snapshot.name,vitals:b.snapshot.vitals,sequence:b.snapshot.sequence}:null,remote:this.observations.get(b.id)?.vitals??null})),events:this.data.events.map(({id,binding,kind,status,roll,before,after})=>({id,binding,kind,status,label:roll?.label,total:roll?.total,before,after}))};}
}
