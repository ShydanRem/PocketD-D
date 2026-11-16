import {chooseUsbPort,rememberUsbPort,forgetUsbPort,claimUsbPort} from './usb-port.js';
import {ACCOUNT_ORIGINS,checkAccount} from './account-check.js';
class LinkError extends Error {constructor(message,code='usb'){super(message);this.code=code;}}
const deviceFailures={
 wifi:'Il Pocket non è collegato al Wi-Fi. Collega la rete da Wi-Fi → Cerca reti sul dispositivo, poi riprova qui.',
 clock:'Il Pocket non riesce a sincronizzare l’orologio. Controlla che il Wi-Fi abbia accesso a Internet e riprova.',
 login:'Beyond ha rifiutato l’accesso del Pocket. Accedi nuovamente a Beyond in questo browser e riprova.',
 identity:'Il Pocket non ha verificato l’identità Beyond. Accedi nuovamente a Beyond e riprova.',
 catalog:'Il Pocket non riconosce l’elenco Beyond. Non è un collegamento riuscito: riprova più tardi o aggiorna il firmware.',
 storage:'Il Pocket non conferma il salvataggio dell’accesso. Controlla il display e riavvia il dispositivo prima di riprovare. I PG restano conservati.',
 memory:'Memoria del Pocket insufficiente. Riavvia il dispositivo e riprova: nessun PG viene cancellato.',
 timeout:'Il Pocket ha terminato il tempo di attesa. Controlla Wi-Fi e Internet, poi riprova.',
 busy:'Il Pocket è occupato. Termina o annulla l’operazione sul display, poi riprova.',
 rate:'Beyond sta limitando le richieste. Attendi qualche minuto prima di riprovare.',
 format:'Risposta Beyond non compatibile sul Pocket. Controlla gli aggiornamenti; la sola risposta HTTP non conferma il collegamento.',
 network:'Il Pocket non raggiunge Beyond. Controlla Wi-Fi e accesso a Internet, poi riprova.'
};
const cookieOK = value => typeof value==='string' && value.length>0 && value.length<=4096 && /^[\x21\x23-\x2b\x2d-\x3a\x3c-\x5b\x5d-\x7e]+$/.test(value);
export async function preparePocketAccount({api=chrome,signal,now=Date.now,check=checkAccount,requestPermission=true}={}) {
  // Called directly by the user's preparation click. No cookie API at startup,
  // in the bridge, or in the existing read-only account check.
  if(!await api.permissions[requestPermission?'request':'contains']({permissions:['cookies'],origins:ACCOUNT_ORIGINS}))throw new LinkError('Consenso non concesso. Nessun accesso trasferito.','permission');
  signal?.throwIfAborted();
  const read=()=>api.cookies.get({url:'https://auth-service.dndbeyond.com/v1/cobalt-token',name:'CobaltSession'});
  let before=await read(),identity;
  if(!cookieOK(before?.value)||before.expirationDate>0&&before.expirationDate*1000<=now())throw new LinkError('Accedi a Beyond nello stesso browser e riprova.','login');
  const result=await check({signal,onIdentity:id=>identity=id});signal?.throwIfAborted();
  if(!result.ok||!identity||!/^[1-9]\d{0,14}$/.test(identity))throw new LinkError(result.ok?'Account non verificabile.':result.message,['rejected','session','expired'].includes(result.code)?'login':result.code??'identity');
  const after=await read();signal?.throwIfAborted();
  if(after?.value!==before.value)throw new LinkError('Il login è cambiato durante la verifica. Riprepara il collegamento.','login');
  const expires=Math.min(now()+180000,after.expirationDate>0?after.expirationDate*1000:Infinity);
  if(expires<=now()+15000)throw new LinkError('Accesso in scadenza. Accedi nuovamente a Beyond.','login');
  const access={user:identity,cookie:before.value,expires,count:result.characters.length};before=null;return access;
}
export async function pairPocketAccount(access,{serial=navigator.serial,signal,onCode=()=>{},onStage=()=>{},timeout=120000,now=Date.now}={}) {
  const valid=value=>value&&/^[1-9]\d{0,14}$/.test(value.user)&&cookieOK(value.cookie)&&now()<value.expires;
  if(typeof access!=='function'&&!valid(access))throw new LinkError('Riprepara l’accesso prima di collegare il Pocket.');
  if(!serial?.requestPort)throw new LinkError('Web Serial non disponibile: usa Chrome o Edge sul PC.');
  const release=claimUsbPort();
  let ownedAccess=false,checkingAccount=false;let port,reader,writer,nonce,linked=false,closing=false,bytes=0,waiting,readError;
  const lines=[];let partial='';const encoder=new TextEncoder();
  async function send(text){const data=encoder.encode(text);try{await bounded(writer.write(data),10000);}finally{data.fill(0);}}
  function bounded(promise,ms){return new Promise((resolve,reject)=>{const timer=setTimeout(()=>finish(reject,new LinkError('Tempo scaduto: controlla il Pocket prima di riprovare.','timeout')),ms);const abort=()=>finish(reject,new LinkError('Collegamento interrotto: controlla il Pocket per l’esito.','cancelled'));function finish(fn,v){clearTimeout(timer);signal?.removeEventListener('abort',abort);fn(v);}if(signal?.aborted)return abort();signal?.addEventListener('abort',abort,{once:true});Promise.resolve(promise).then(v=>finish(resolve,v),()=>finish(reject,new LinkError('USB non disponibile. Controlla il Pocket.')));});}
  async function pump(){try{while(!closing){const {done,value}=await reader.read();if(done)throw new LinkError('closed');for(const byte of value){if(byte===10){if(lines.length>=100)throw new LinkError('overflow');lines.push(partial.replace(/\r$/,''));partial='';waiting?.();waiting=null;}else{if(partial.length>=512)throw new LinkError('line');partial+=String.fromCharCode(byte);}}bytes+=value.length;if(bytes>16384)throw new LinkError('size');}}catch{readError=true;waiting?.();waiting=null;}}
  let bootMessages=0;
  async function line(ms){
    const end=now()+ms;
    for(;;){
      signal?.throwIfAborted();
      if(now()>=end)throw new LinkError('Tempo scaduto: controlla il Pocket prima di riprovare.','timeout');
      if(readError)throw new LinkError('Connessione USB interrotta: controlla il Pocket.');
      if(!lines.length){await bounded(new Promise(r=>waiting=r),Math.max(1,end-now()));continue;}
      const text=lines.shift();
      if(text==='PDD ACCOUNT ERROR busy')throw new LinkError(deviceFailures.busy,'busy');
      if(text.startsWith('PDD ACCOUNT ERROR'))throw new LinkError('Richiesta USB rifiutata. Aggiorna firmware ed estensione Pocket PC, poi riprova.','protocol');
      if(text.startsWith('PDD ACCOUNT '))return text;
      if(/^(ESP-ROM:|rst:)/.test(text)&&++bootMessages>=3)throw new LinkError('Il Pocket sembra riavviarsi. Controlla alimentazione e cavo USB; attendi la schermata del personaggio prima di riprovare.','restart');
      if(!text||/^(?:\x1b\[|[IWE] \(|ESP-ROM:|Build:|rst:|Saved PC|SPIWP:|mode:|load:|entry )/.test(text))continue;
      throw new LinkError('Protocollo USB non compatibile: aggiorna il firmware PocketD-D.','protocol');
    }
  }
  let pumping;
  try {
    // No await before the chooser. No bootloader or reset sequence.
    port=await chooseUsbPort(serial);signal?.throwIfAborted();if(typeof access==='function'){onStage('account');checkingAccount=true;access=await access();ownedAccess=true;if(!valid(access))throw new LinkError('Accesso non pronto. Accedi a Beyond e riprova.','login');checkingAccount=false;}onStage('usb');const opening=port.open({baudRate:115200,bufferSize:8192});try{await bounded(opening,10000);}catch(error){Promise.resolve(opening).then(()=>port.close()).catch(()=>{});throw error;}
    if(port.setSignals)await bounded(port.setSignals({dataTerminalReady:false,requestToSend:false}),10000);
    reader=port.readable.getReader();writer=port.writable.getWriter();pumping=pump();onStage('challenge');await send('PDD ACCOUNT HELLO 1\n');
    const challenge=await line(15000),match=/^PDD ACCOUNT CODE 1 ([0-9a-f]{32}) (\d{6})$/.exec(challenge);if(!match)throw new LinkError('Risposta USB non compatibile: aggiorna il firmware.','protocol');nonce=match[1];rememberUsbPort(port,serial);onCode(match[2]);
    const approvalDeadline=now()+timeout;let response;do{const remaining=approvalDeadline-now();if(remaining<=0)throw new LinkError('Codice scaduto. Ripeti il collegamento e conferma sul Pocket entro due minuti.','timeout');response=await line(remaining);}while(response===challenge);
    if(response.startsWith(`PDD ACCOUNT FAILED 1 ${nonce} `)){const code=response.slice(`PDD ACCOUNT FAILED 1 ${nonce} `.length);if(Object.hasOwn(deviceFailures,code))throw new LinkError(deviceFailures[code],code);}
    if(response!==`PDD ACCOUNT ACCEPT 1 ${nonce}`)throw new LinkError('Collegamento non confermato sul Pocket.');
    if(now()>=access.expires)throw new LinkError('Preparazione scaduta: ripeti il collegamento.','login');
    signal?.throwIfAborted();onStage('link');await send(`PDD ACCOUNT OFFER 1 ${nonce} ${access.user} ${access.cookie}\n`);
    response=await line(90000);
    if(response.startsWith(`PDD ACCOUNT FAILED 1 ${nonce} `)){const code=response.slice(`PDD ACCOUNT FAILED 1 ${nonce} `.length);if(Object.hasOwn(deviceFailures,code))throw new LinkError(deviceFailures[code],code);}
    if(response!==`PDD ACCOUNT LINKED 1 ${nonce}`)throw new LinkError('Accesso non confermato: leggi il messaggio sul Pocket.');linked=true;onStage('linked');return {linked:true};
  }catch(error){if(!nonce&&!checkingAccount&&error?.name!=='NotFoundError')forgetUsbPort(port);if(error?.name==='NotFoundError')throw new LinkError('Scelta della porta annullata. Premi Riprova per scegliere il Pocket.','chooser');if(error?.name==='AbortError')throw new LinkError('Collegamento interrotto: controlla il Pocket per l’esito.','cancelled');if(error instanceof LinkError)throw error;throw new LinkError('USB non disponibile. Controlla il Pocket.');}
  finally {
    if(writer&&!linked&&nonce)try{await Promise.race([writer.write(encoder.encode(`PDD ACCOUNT CANCEL 1 ${nonce}\n`)),new Promise(r=>setTimeout(r,500))]);}catch{/* best effort */}
    const settle=p=>Promise.race([Promise.resolve(p).catch(()=>{}),new Promise(r=>setTimeout(r,1000))]);
    closing=true;try{await settle(reader?.cancel());}catch{}try{await settle(pumping);}catch{}try{reader?.releaseLock();}catch{}try{await settle(writer?.abort());}catch{}try{writer?.releaseLock();}catch{}try{await settle(port?.close());}catch{}if(ownedAccess&&access)access.cookie='';release();
  }
}
