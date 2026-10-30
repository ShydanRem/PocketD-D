import {usbSession,usbHandshake} from './device-usb.js';
import {crc32} from './device-backup.js';
export const SD_PACKAGE=Object.freeze({size:6195200,sha256:'d439cacf0de37f60c16d3dec4e59fefe7cb19e9bf0d9feb10e3fec651d31950f',chunk:2048});
const prepared=new WeakMap();
const hex=bytes=>Array.from(bytes,b=>b.toString(16).padStart(2,'0')).join('');
export async function prepareSdAssets({fetcher=fetch,url,bytes}={}){
  let data;
  if(bytes)data=new Uint8Array(bytes).slice();
  else{const r=await fetcher(url??new URL('assets/classes.pdc',globalThis.document?.baseURI??import.meta.url),{cache:'no-store',signal:AbortSignal.timeout(60000)});if(!r.ok)throw Error('Pacchetto look non disponibile. Riapri la pagina aggiornata.');data=new Uint8Array(await r.arrayBuffer());}
  if(data.length!==SD_PACKAGE.size||hex(new Uint8Array(await crypto.subtle.digest('SHA-256',data)))!==SD_PACKAGE.sha256)throw Error('Pacchetto look incompleto o alterato. Nessun collegamento USB avviato.');
  const handle=Object.freeze({size:data.length,sha256:SD_PACKAGE.sha256});prepared.set(handle,data);return handle;
}
export async function installSdAssets(handle,options={}){
  const bytes=prepared.get(handle);if(!bytes)throw Error('Prepara e verifica il pacchetto prima di collegare il Pocket.');
  return usbSession({...options,operation:'assets'},async(c,t)=>{
    options.onProgress?.('connect');const {duplicates}=await usbHandshake(c,t,'PDD HELLO',Array.from({length:21},(_,i)=>`PDD READY ${i+1}`));
    await c.send('PDD ASSET HELLO 1\n',t.send);
    try{await c.wait('PDD ASSET READY 1',t.hello,{duplicates});}catch(error){if(!error.assetCode&&error.message?.includes('rifiutata'))throw Error('Per installare sulla SD aggiorna PocketD-D al firmware 0.83. Nessun pacchetto inviato.');throw error;}
    let started=false,confirmed=false;
    try{
      started=true;options.onProgress?.('sd-check');
      await c.send(`PDD ASSET BEGIN 1 ${handle.size} ${handle.sha256}\n`,t.send);
      const done=`PDD ASSET DONE 1 ${handle.sha256}`,first=await c.wait(['PDD ASSET OFFSET 0',done],options.timeouts?.assetsBegin??90000);
      if(first===done){confirmed=true;options.onProgress?.('sd-complete');return {installed:true,already:true};}
      options.onProgress?.('sd-send');
      for(let offset=0;offset<bytes.length;offset+=SD_PACKAGE.chunk){
        const part=bytes.subarray(offset,Math.min(offset+SD_PACKAGE.chunk,bytes.length)),next=offset+part.length;
        await c.send(`PDD ASSET WRITE 1 ${offset} ${crc32(part).toString(16).padStart(8,'0')} ${hex(part)}\n`,t.send);
        await c.wait(`PDD ASSET OFFSET ${next}`,t.data);options.onAssetProgress?.(Math.floor(100*next/bytes.length));
      }
      options.onProgress?.('sd-verify');await c.send(`PDD ASSET FINISH 1 ${bytes.length}\n`,t.send);await c.wait(done,options.timeouts?.assetsFinish??90000);
      confirmed=true;options.onProgress?.('sd-complete');return {installed:true,already:false};
    }finally{if(started&&!confirmed)try{await c.cancelCommand('PDD ASSET ABORT 1\n');}catch{/* Firmware timeout also closes the staging file; never retries data. */}}
  });
}
