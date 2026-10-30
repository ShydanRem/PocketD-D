import {serializeDevice} from './device-export.js';
import {crc32,inspectDeviceBackup,DEVICE_BACKUP_MAX} from './device-backup.js';
import {createUsbDiagnostics} from './device-usb-diagnostics.js';
import {validateRuntimeDiagnostics} from './device-runtime-diagnostics.js';
const encoder=new TextEncoder(),decoder=new TextDecoder('utf-8',{fatal:true});
let active=false;
export const deviceUsbSupported=()=>globalThis.isSecureContext===true&&globalThis.location?.protocol!=='file:'&&Boolean(globalThis.navigator?.serial?.requestPort);
const uncertain=()=>new Error('Esito USB non confermato. Controlla il display prima di riprovare.');
const aborted=()=>new Error('Collegamento interrotto. Controlla il display: una richiesta può essere ancora in attesa o già salvata.');
const timedOut=()=>Object.assign(uncertain(),{code:'USB_TIMEOUT'});
const phase=expected=>String(expected).startsWith('PDD ASSET')?'installazione look sulla SD':String(expected).startsWith('PDD DIAGNOSTICS')?'lettura diagnosi firmware':Array.isArray(expected)||String(expected).includes('READY')?'collegamento':expected==='PDD WAIT_CONFIRM'?'invio della copia':expected==='PDD IMPORTED'?'salvataggio della scheda':String(expected).includes('RESTORE')||expected==='PDD RESTORED'?'ripristino del backup':'trasferimento del backup';
function unexpected(expected,line){
  const response=/^PDD (?:READY [0-9]+|BACKUP_READY [0-9]+|WAIT_CONFIRM|WAIT_BACKUP|WAIT_RESTORE|IMPORTED|RESTORED|BACKUP_DONE)$/.test(line)?line:'testo non riconosciuto';
  return new Error(`Risposta USB inattesa durante ${phase(expected)} (${response}). Controlla che PocketD-D sia avviato e che la porta scelta sia quella del dispositivo. Nessun salvataggio confermato.`);
}
function limited(promise,ms,signal){
  return new Promise((resolve,reject)=>{
    let timer;const abort=()=>finish(reject,aborted()),finish=(fn,value)=>{clearTimeout(timer);signal?.removeEventListener('abort',abort);fn(value);};
    if(signal?.aborted){reject(aborted());return;}
    timer=setTimeout(()=>finish(reject,timedOut()),Math.max(0,ms));signal?.addEventListener('abort',abort,{once:true});
    Promise.resolve(promise).then(v=>finish(resolve,v),e=>finish(reject,e));
  });
}
class Channel{
  constructor(port,signal,diagnostics){
    this.diagnostics=diagnostics;this.signal=signal;this.reader=port.readable.getReader();this.writer=port.writable.getWriter();this.chunks=[];this.bytes=0;this.offset=0;this.ended=false;this.error=null;this.closing=false;this.lineBytes=[];
    this.pump=this.receive();
  }
  async receive(){
    try{while(!this.closing){const {value,done}=await this.reader.read();if(done){this.ended=true;break;}if(value?.length){if(this.bytes+value.length>DEVICE_BACKUP_MAX+1024)throw new Error('Risposta USB oltre il limite.');this.chunks.push(value);this.bytes+=value.length;this.wake?.();this.wake=null;}}}
    catch(error){this.error=error;}finally{this.wake?.();this.wake=null;}
  }
  async available(deadline){
    if(Date.now()>=deadline)throw timedOut();
    while(!this.bytes){
      if(this.error)throw this.error;if(this.ended||this.closing)throw uncertain();
      await limited(new Promise(r=>this.wake=r),deadline-Date.now(),this.signal);
    }
    if(this.error)throw this.error;if(this.signal?.aborted)throw aborted();
  }
  async line(timeout,maxLine=256){
    const deadline=Date.now()+timeout,chars=this.lineBytes;
    for(;;){await this.available(deadline);const byte=this.chunks[0][this.offset++];this.bytes--;if(this.offset===this.chunks[0].length){this.chunks.shift();this.offset=0;}
      if(byte===10){
        const text=String.fromCharCode(...chars).replace(/\r$/,'');this.lineBytes=[];
        if(text==='waiting for download')throw new Error('Il dispositivo è nel bootloader. Rilascia BOOT, riavvia con RESET e attendi la schermata PocketD-D prima di riprovare. Nessun salvataggio confermato.');
        if(this.diagnostics.line(text))return '';
        if(/[^\x20-\x7e]/.test(text)||text.length>maxLine)throw new Error('Risposta USB non valida.');
        return text;
      }
      if(byte<32&&byte!==13&&byte!==27||byte>126||chars.length>=Math.max(512,maxLine))throw new Error('Risposta USB non valida.');chars.push(byte);
    }
  }
  async exact(size,timeout){
    const out=new Uint8Array(size),deadline=Date.now()+timeout;let at=0;
    while(at<size){await this.available(deadline);const chunk=this.chunks[0],take=Math.min(size-at,chunk.length-this.offset);out.set(chunk.subarray(this.offset,this.offset+take),at);at+=take;this.offset+=take;this.bytes-=take;if(this.offset===chunk.length){this.chunks.shift();this.offset=0;}}
    return out;
  }
  async send(text,timeout){
    const bytes=encoder.encode(text),deadline=Date.now()+timeout;
    for(let at=0;at<bytes.length;at+=1024){if(this.signal?.aborted)throw aborted();await limited(this.writer.write(bytes.subarray(at,at+1024)),deadline-Date.now(),this.signal);}
  }
  async cancelCommand(text){await limited(this.writer.write(encoder.encode(text)),500);}
  async wait(expected,timeout,{hello=false,prefix=false,duplicates=null,maxLine=256}={}){
    this.diagnostics.data.phase=phase(expected);
    const deadline=Date.now()+timeout;
    for(;;){let line;
      try{line=await this.line(deadline-Date.now(),maxLine);}catch(error){if(error.code==='USB_TIMEOUT')error.message=`Nessuna conferma USB durante ${phase(expected)}. Controlla il display prima di riprovare: esito non confermato.`;throw error;}
      if(!line)continue;
      if(line.startsWith('PDD ASSET ERROR ')){
        const code=line.slice('PDD ASSET ERROR '.length),messages={sd:'MicroSD non disponibile: inserisci una scheda FAT32 nel Pocket e riprova.',space:'Spazio insufficiente sulla microSD per il pacchetto e la copia di recupero.',storage:'Scrittura SD non riuscita. Controlla la scheda prima di riprovare.',integrity:'Verifica dei file SD non riuscita. Aggiornamento non confermato.',chunk:'Blocco SD incompleto o discordante. Riprova l’installazione.',incomplete:'Pacchetto SD incompleto. Aggiornamento non confermato.',busy:'Pocket occupato: termina la richiesta sul display e riprova.',cancelled:'Installazione SD interrotta.',timeout:'Installazione SD scaduta; controlla il display.',metadata:'Pacchetto SD non compatibile con questo firmware.'};
        throw Object.assign(new Error(messages[code]??'Installazione SD non confermata. Controlla il display.'),{assetCode:Object.hasOwn(messages,code)?code:'unknown'});
      }
      if(hello&&['PDD ERROR packet','PDD ERROR size'].includes(line))continue;
      if(duplicates?.remaining>0&&line===duplicates.line){duplicates.remaining--;continue;}
      if(line.startsWith('PDD CANCELLED'))throw new Error('Richiesta annullata o scaduta sul dispositivo. Nessuna conferma ricevuta.');
      if(line.startsWith('PDD ERROR')){
        const detail=line.split(' ').slice(2).join(' '),messages={busy:'Dispositivo occupato: chiudi la conferma aperta sul display.',history:'Storico da salvare sul dispositivo: controlla il display.',storage:'Salvataggio non confermato dal dispositivo: controlla il display.',packet:'Copia rifiutata dal dispositivo.',size:'Copia oltre il limite del firmware.'};
        throw new Error(messages[detail]??'Richiesta USB rifiutata dal dispositivo. Controlla il display.');
      }
      if(prefix?line.startsWith(expected):Array.isArray(expected)?expected.includes(line):line===expected)return line;
      throw unexpected(expected,line);
    }
  }
  async close(){
    this.closing=true;this.wake?.();
    try{await limited(this.reader.cancel(),1000);}catch{/* Port may already be disconnected. */}
    try{await limited(this.pump,1000);}catch{/* Do not trap the dialog on a broken driver. */}
    try{this.reader.releaseLock();}catch{/* A pending driver read may hold the lock. */}
    const stopping=this.writer.abort();
    try{this.writer.releaseLock();}catch{/* Already released/disconnected. */}
    try{await limited(stopping,1000);}catch{/* A broken driver may not finish aborting. */}
  }
}
async function session(options,task){
  if(active)throw new Error('Un altro collegamento USB PocketD-D è già in corso.');
  const serial=options.serial??globalThis.navigator?.serial;
  if(!serial?.requestPort)throw new Error('Per l’USB usa Chrome o Edge su PC, su HTTPS o localhost.');
  if(options.signal?.aborted)throw aborted();
  active=true;let port,channel;const diagnostics=createUsbDiagnostics(options.operation);
  try{
    // Called synchronously from the UI click; no await before the chooser.
    port=await serial.requestPort();
    diagnostics.port(port.getInfo?.());
    if(options.signal?.aborted)throw aborted();
    const opening=port.open({baudRate:115200,bufferSize:8192});
    try{await limited(opening,options.timeouts?.open??10000,options.signal);}
    catch(error){Promise.resolve(opening).then(()=>port.close()).catch(()=>{});throw error;}
    // Release modem-control lines together. Never run a bootloader/reset sequence.
    if(typeof port.setSignals==='function')await limited(port.setSignals({dataTerminalReady:false,requestToSend:false}),options.timeouts?.open??10000,options.signal);
    if(!port.readable||!port.writable)throw new Error('Porta USB non disponibile. Chiudi gli altri programmi che la usano.');
    channel=new Channel(port,options.signal,diagnostics);const result=await task(channel,{hello:10000,send:30000,confirm:125000,data:30000,...options.timeouts});diagnostics.data.outcome='completed';return result;
  }catch(error){
    error.usbDiagnostic=diagnostics.snapshot();
    if(error?.name==='NotFoundError')throw new Error('Scelta della porta annullata. Nessun invio avviato.');
    throw error;
  }finally{
    try{if(channel)await channel.close();}finally{try{if(port)await limited(port.close(),1000);}catch{/* Already disconnected/closed. */}active=false;options.onDiagnostic?.(diagnostics.snapshot());}
  }
}
async function handshake(c,t,command,expected){
  const deadline=Date.now()+t.hello;let attempts=0;
  for(;;){
    const remaining=deadline-Date.now();if(remaining<=0)throw new Error('PocketD-D non risponde al collegamento USB. Attendi la schermata del PG, chiudi l’installer e riprova sulla porta del dispositivo. Nessuna copia inviata.');
    await c.send('\n'+command+'\n',Math.min(t.send,remaining));attempts++;c.diagnostics.data.helloAttempts=attempts;
    try{
      const ready=await c.wait(expected,Math.min(t.helloAttempt??1000,deadline-Date.now()),{hello:true});
      c.diagnostics.data.usbVersion=Number(ready.split(' ').at(-1));c.diagnostics.data.protocol=command.includes('BACKUP')?'backup':'character';
      return {ready,duplicates:{line:ready,remaining:attempts-1}};
    }catch(error){if(error.code!=='USB_TIMEOUT')throw error;}
  }
}
// One port lock for character, backup, diagnostics and SD operations.
export {session as usbSession,handshake as usbHandshake};
export async function checkDeviceConnection(options={}){
  return session({...options,operation:'check'},async(c,t)=>{
    options.onProgress?.('connect');const {ready}=await handshake(c,t,'PDD HELLO',Array.from({length:19},(_,i)=>`PDD READY ${i+1}`));
    options.onProgress?.('connection-checked');return {usbVersion:Number(ready.split(' ').at(-1))};
  });
}
export async function readDeviceDiagnostics(options={}){
  return session({...options,operation:'diagnostics'},async(c,t)=>{
    options.onProgress?.('connect');
    const {ready,duplicates}=await handshake(c,t,'PDD HELLO',Array.from({length:19},(_,i)=>`PDD READY ${i+1}`));
    options.onProgress?.('diagnostics');await c.send('PDD DIAGNOSTICS GET 1\n',t.send);
    const line=await c.wait('PDD DIAGNOSTICS ',t.hello,{prefix:true,duplicates,maxLine:2080});
    let value;try{value=validateRuntimeDiagnostics(JSON.parse(line.slice('PDD DIAGNOSTICS '.length)));}catch{throw new Error('Diagnosi firmware non valida o non supportata. Nessun dato della scheda inviato.');}
    if(value.formats.usb!==Number(ready.split(' ').at(-1)))throw new Error('Versioni USB discordanti nella diagnosi. Nessun dato della scheda inviato.');
    c.diagnostics.data.runtime=value;options.onProgress?.('diagnostics-read');return value;
  });
}
export async function sendDeviceCopy(packet,options={}){
  const text=serializeDevice(packet);
  if(packet?.format!=='pocketdd-device'||!Number.isInteger(packet.version)||packet.version<1||packet.version>19||typeof text!=='string'||text.length>262144||/[^\x20-\x7e]/.test(text))throw new Error('Copia per dispositivo non valida o fuori limite.');
  const version=packet.version;
  return session({...options,operation:'import'},async(c,t)=>{
    options.onProgress?.('connect');
    const {ready,duplicates}=await handshake(c,t,'PDD HELLO',Array.from({length:19},(_,i)=>`PDD READY ${i+1}`));
    if(version>Number(ready.split(' ').at(-1)))throw new Error(`Aggiorna il firmware: questa copia richiede USB${version}. Nessuna copia inviata.`);
    options.onProgress?.('send');await c.send(text+'\n',t.send);await c.wait('PDD WAIT_CONFIRM',t.hello,{duplicates});options.onProgress?.('confirm-import');
    await c.wait('PDD IMPORTED',t.confirm);options.onProgress?.('imported');
  });
}
async function negotiate(c,t){return handshake(c,t,'PDD BACKUP HELLO','PDD BACKUP_READY 1');}
export async function receiveDeviceBackup(options={}){
  return session({...options,operation:'receive'},async(c,t)=>{
    options.onProgress?.('connect');const {duplicates}=await negotiate(c,t);await c.send('PDD BACKUP GET 1\n',t.send);await c.wait('PDD WAIT_BACKUP',t.hello,{duplicates});options.onProgress?.('confirm-backup');
    const header=await c.wait('PDD BACKUP_DATA ',t.confirm,{prefix:true}),match=/^PDD BACKUP_DATA ([1-9][0-9]{0,5}) ([0-9a-f]{8})$/.exec(header);
    if(!match||Number(match[1])>DEVICE_BACKUP_MAX)throw new Error('Intestazione del backup USB non valida.');
    options.onProgress?.('receive');const bytes=await c.exact(Number(match[1]),t.data);
    if(bytes.some(b=>b>127)||crc32(bytes)!==Number.parseInt(match[2],16))throw new Error('Backup USB incompleto o corrotto. Nessun backup verificato.');
    await c.wait('PDD BACKUP_DONE',t.hello);const raw=decoder.decode(bytes);inspectDeviceBackup(raw);options.onProgress?.('received');return raw;
  });
}
export async function restoreDeviceBackup(raw,options={}){
  const text=JSON.stringify(inspectDeviceBackup(raw).packet);
  return session({...options,operation:'restore'},async(c,t)=>{
    options.onProgress?.('connect');const {duplicates}=await negotiate(c,t);options.onProgress?.('send');await c.send(text+'\n',t.send);await c.wait('PDD WAIT_RESTORE',t.hello,{duplicates});options.onProgress?.('confirm-restore');
    await c.wait('PDD RESTORED',t.confirm);options.onProgress?.('restored');
  });
}
