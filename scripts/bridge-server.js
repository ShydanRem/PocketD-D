import {createServer as httpServer} from 'node:http';import {createServer as httpsServer} from 'node:https';
import {readFileSync,writeFileSync,existsSync} from 'node:fs';import {join} from 'node:path';import {networkInterfaces} from 'node:os';
import {BridgeStore,digest,secret,equalsSecret} from './bridge-store.js';import {createBridgeCertificate} from './bridge-certificate.js';import {beyondCharacterId} from '../src/beyond.js';
import {probeBeyond} from './beyond-client.js';
export function privateIP(ip,{test=false}={}){const n=ip.split('.').map(Number);return /^\d{1,3}(\.\d{1,3}){3}$/.test(ip)&&n.every(x=>x<=255)&&(n[0]===10||n[0]===192&&n[1]===168||n[0]===172&&n[1]>=16&&n[1]<=31||test&&ip==='127.0.0.1');}
export function createBridge({directory,ip,devicePort=8788,adminPort=8787,test=false,now=()=>Date.now(),verifyHP=async id=>{const r=await probeBeyond(id);if(!r.draft)throw Error('PF non riconciliabili dal servizio pubblico');return {hp:r.draft.hp,max:r.draft.maxHp,temp:r.draft.extended.tempHp};}}){
 if(!privateIP(ip,{test}))throw Error('Scegli un indirizzo IPv4 della rete privata del PC');
 const store=new BridgeStore(directory,{now}),certPath=join(directory,'bridge-cert.pem'),keyPath=join(directory,'bridge-key.pem'),hostPath=join(directory,'bridge-host.txt');
 if(!existsSync(certPath)||!existsSync(keyPath)||!existsSync(hostPath)||readFileSync(hostPath,'utf8')!==ip){const c=createBridgeCertificate(ip);writeFileSync(certPath,c.cert,{mode:0o600});writeFileSync(keyPath,c.key,{mode:0o600});writeFileSync(hostPath,ip,{mode:0o600});for(const b of [...store.data.bindings])store.revoke(b.id);}
 const cert=readFileSync(certPath,'utf8'),key=readFileSync(keyPath,'utf8'),csrf=secret();let extensionCode=null;
 const reply=(r,code,data)=>{r.writeHead(code,{'Content-Type':'application/json; charset=utf-8','Cache-Control':'no-store','X-Content-Type-Options':'nosniff'});r.end(JSON.stringify(data));};
 const body=async req=>{if(!/^application\/json(?:;|$)/.test(req.headers['content-type']??''))throw Error('Invia JSON');let bytes=0,parts=[];for await(const p of req){bytes+=p.length;if(bytes>16000)throw Error('Messaggio oltre limite');parts.push(p);}return JSON.parse(Buffer.concat(parts));};
 const auth=req=>req.headers.authorization?.match(/^Bearer ([a-f0-9]{64})$/)?.[1];let activeRequests=0;
 const device=httpsServer({key,cert,minVersion:'TLSv1.2'},async(req,res)=>{
  if(req.headers.host!==`${ip}:${device.address().port}`||req.headers.origin||req.method!=='POST'||req.url!=='/device/snapshot')return reply(res,403,{error:'Richiesta non disponibile'});
  const b=store.binding(auth(req));if(!b)return reply(res,401,{error:'Abbinamento non riconosciuto'});if(activeRequests>=2)return reply(res,429,{error:'Occupato'});activeRequests++;
  try{reply(res,200,store.observe(b.id,await body(req)));}catch{reply(res,409,{state:'blocked',error:'Dati discordanti, memoria o coda non disponibili: controlla il ponte PC'});}finally{activeRequests--;}
 });
 const admin=httpServer(async(req,res)=>{
  const hosts=[`127.0.0.1:${admin.address().port}`,`localhost:${admin.address().port}`],origin=req.headers.origin;
  if(!hosts.includes(req.headers.host))return reply(res,403,{error:'Host non ammesso'});
  const extensionOrigin=typeof origin==='string'&&/^chrome-extension:\/\/[a-p]{32}$/.test(origin);
  if(origin&&!hosts.some(h=>origin===`http://${h}`)&&!extensionOrigin)return reply(res,403,{error:'Origine non ammessa'});
  if(extensionOrigin){res.setHeader('Access-Control-Allow-Origin',origin);res.setHeader('Vary','Origin');if(req.method==='OPTIONS'){res.setHeader('Access-Control-Allow-Headers','Content-Type, Authorization');res.setHeader('Access-Control-Allow-Methods','POST');res.writeHead(204);res.end();return;}}
  if(req.url==='/'&&req.method==='GET'){
   let html=readFileSync(new URL('../tools/bridge/index.html',import.meta.url),'utf8').replace('CSRF_PLACEHOLDER',csrf);
   res.writeHead(200,{'Content-Type':'text/html; charset=utf-8','Cache-Control':'no-store','Content-Security-Policy':"default-src 'self'; script-src 'self'; style-src 'self'; frame-ancestors 'none'; base-uri 'none'; form-action 'self'"});res.end(html);return;
  }
  if(['/bridge.js','/bridge.css'].includes(req.url)&&req.method==='GET'){res.writeHead(200,{'Content-Type':req.url.endsWith('.js')?'text/javascript':'text/css','Cache-Control':'no-store'});res.end(readFileSync(new URL('../tools/bridge'+req.url,import.meta.url)));return;}
  if(req.method!=='POST')return reply(res,404,{error:'Percorso non disponibile'});
  try{
   const data=await body(req);
   if(req.url==='/extension/connect'){
    if(!extensionCode||extensionCode.until<now()||extensionCode.tries++>=5||!equalsSecret(digest(String(data.code)),extensionCode.hash))return reply(res,403,{error:'Codice scaduto o non valido'});
    extensionCode=null;return reply(res,200,{token:store.extension()});
   }
   if(req.url.startsWith('/extension/')){
    if(!store.data.extensionHash||!equalsSecret(auth(req),store.data.extensionHash))return reply(res,401,{error:'Estensione non abbinata'});
    if(req.url==='/extension/next')return reply(res,200,{event:store.next(data.observation,data.owner)});
    if(req.url==='/extension/report'){
     let verified=null;const e=store.data.events.find(e=>e.id===data.id);if(e?.kind==='hp'&&e.status==='claimed'&&['dom-updated','uncertain'].includes(data.outcome)&&data.lease===e.lease){try{verified=await verifyHP(e.characterId);}catch{}}
     return reply(res,200,store.report(data.id,data.lease,data.outcome,verified));
    }return reply(res,404,{error:'Percorso non disponibile'});
   }
   if(extensionOrigin||!equalsSecret(data.csrf,digest(csrf)))return reply(res,403,{error:'Apri il ponte dal suo indirizzo locale'});
   if(req.url==='/api/status')return reply(res,200,{...store.publicState(),ip,devicePort:device.address().port,fingerprint:digest(cert)});
   if(req.url==='/api/bind'){
    const b=store.addBinding(beyondCharacterId(String(data.characterId)),data.scope);return reply(res,200,{bundle:JSON.stringify({version:1,id:b.id,host:ip,port:device.address().port,characterId:b.characterId,token:b.token,certificate:cert,time:Math.floor(now()/1000)})});
   }
   if(req.url==='/api/extension-code'){const code=String(Math.floor(100000000+Number.parseInt(secret().slice(0,12),16)%900000000));extensionCode={hash:digest(digest(code)),until:now()+120000,tries:0};return reply(res,200,{code});}
   if(req.url==='/api/approve')store.approve(data.id);
   else if(req.url==='/api/pause')store.pause(data.id);
   else if(req.url==='/api/reconcile')store.reconcile(data.id);
   else if(req.url==='/api/revoke')store.revoke(data.id);
   else if(req.url==='/api/archive')store.archive();
   else return reply(res,404,{error:'Percorso non disponibile'});
   reply(res,200,{ok:true});
  }catch(error){reply(res,409,{error:/^(PF|Apri|Risolvi|Abbinamento|PG |Massimo|Rinnova|Coda)/.test(error.message)?error.message:'Operazione non riuscita: configurazione o memoria non disponibili'});}
 });
 for(const s of [admin,device]){s.requestTimeout=8000;s.headersTimeout=5000;s.maxConnections=8;}
 return {store,admin,device,cert,csrf,listen:async()=>{await new Promise((r,j)=>{device.once('error',j);device.listen(devicePort,ip,r)});try{await new Promise((r,j)=>{admin.once('error',j);admin.listen(adminPort,'127.0.0.1',r)})}catch(e){device.close();throw e;}},close:async()=>{await Promise.all([admin,device].map(s=>new Promise(r=>s.close(r))));}};
}
export function bridgeAddresses(){return Object.values(networkInterfaces()).flat().filter(x=>x.family==='IPv4'&&privateIP(x.address)&&!x.internal).map(x=>x.address);}
