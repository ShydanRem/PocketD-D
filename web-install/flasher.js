// Only firmware sectors are writable. User NVS at 0x9000 and 0x410000 is excluded.
export const BOARD='Waveshare ESP32-S3-Touch-AMOLED-1.75';
const regions=[['bootloader',0,0x8000],['partitions',0x8000,0x9000],['app',0x10000,0x410000]];
export function validateManifest(m){
  if(m?.schema!==1||m.name!=='PocketD-D'||m.board!==BOARD||m.chip!=='ESP32-S3'||m.flash_size!==0x1000000||!/^\d+\.\d+\.\d+$/.test(m.version)||!Array.isArray(m.parts)||m.parts.length!==3)throw new Error('Pacchetto firmware non riconosciuto.');
  for(let i=0;i<3;i++){
    const p=m.parts[i],[id,start,end]=regions[i];
    if(p.id!==id||p.offset!==start||!Number.isInteger(p.size)||p.size<1||Math.ceil(p.size/4096)*4096+start>end||!/^firmware\/[a-z0-9-]+\.bin$/.test(p.path)||!/^([a-f0-9]{64})$/.test(p.sha256))throw new Error('Aree del firmware non valide.');
  }
  if(m.parts[1].size!==0xc00)throw new Error('Tabella partizioni non riconosciuta.');
  return m;
}
export async function prepareFirmware(manifestURL,{fetcher=fetch,cryptoProvider=crypto}={}){
  const response=await fetcher(manifestURL,{cache:'no-store'});if(!response.ok)throw new Error('Firmware non disponibile: apri l’installer del pacchetto compilato.');
  const manifest=validateManifest(await response.json()),parts=[];
  for(const part of manifest.parts){
    const response=await fetcher(new URL(part.path,manifestURL),{cache:'no-store'});if(!response.ok)throw new Error('Download del firmware non riuscito. Riprova.');
    const data=new Uint8Array(await response.arrayBuffer());
    const hash=Array.from(new Uint8Array(await cryptoProvider.subtle.digest('SHA-256',data)),b=>b.toString(16).padStart(2,'0')).join('');
    if(data.length!==part.size||hash!==part.sha256)throw new Error('Firmware incompleto o alterato. Scarica di nuovo il pacchetto.');
    parts.push({data,address:part.offset});
  }
  return {manifest,parts};
}
export async function installFirmware({prepared,port,mode,Loader,TransportClass,md5,onStatus=()=>{}}){
  validateManifest(prepared.manifest);
  if(!Array.isArray(prepared.parts)||prepared.parts.length!==3||prepared.parts.some((p,i)=>!(p.data instanceof Uint8Array)||p.data.length!==prepared.manifest.parts[i].size||p.address!==prepared.manifest.parts[i].offset)||typeof md5!=='function')throw new Error('File del firmware non validi.');
  if(mode!=='first'&&mode!=='update')throw new Error('Scegli prima installazione o aggiornamento.');
  const transport=new TransportClass(port),loader=new Loader({transport,baudrate:460800,enableTracing:false,
    terminal:{clean(){},write(){},writeLine(){}}});
  let result=null;
  try{
    onStatus('connect',0);await loader.main();
    if(loader.chip?.CHIP_NAME!=='ESP32-S3')throw new Error('Dispositivo diverso da ESP32-S3. Nessun firmware scritto.');
    if(await loader.detectFlashSize()!=='16MB')throw new Error('Serve la scheda con flash da 16 MB. Nessun firmware scritto.');
    if(mode==='update'){
      onStatus('layout',0);const expected=prepared.parts[1].data,actual=await loader.readFlash(0x8000,expected.length);
      if(actual.length!==expected.length||!actual.every((v,i)=>v===expected[i]))throw new Error('Le partizioni sono diverse: aggiornamento interrotto. Conserva un backup e verifica il firmware installato.');
    }
    onStatus('write',0);
    const total=prepared.parts.reduce((n,p)=>n+p.data.length,0),prefix=prepared.parts.map((_,i)=>prepared.parts.slice(0,i).reduce((n,p)=>n+p.data.length,0));
    await loader.writeFlash({fileArray:prepared.parts,flashSize:'keep',flashMode:'keep',flashFreq:'keep',
      eraseAll:false,compress:true,calculateMD5Hash:md5,
      reportProgress(i,written,size){onStatus('write',Math.min(99,Math.floor((prefix[i]+prepared.parts[i].data.length*written/size)/total*100)));}});
    // esptool checks each written region's MD5 before resolving writeFlash.
    onStatus('restart',100);
    try{await loader.after('hard_reset');result={restarted:true};}
    catch{result={restarted:false};}
  }finally{
    try{await transport.disconnect();}catch{/* Device may disconnect when it resets. */}
  }
  onStatus(result.restarted?'done':'done-reset',100);return result;
}
