// A single verified ZIP for the user; bounded hosting objects behind the download.
const hash=async(bytes,crypto)=>Array.from(new Uint8Array(await crypto.subtle.digest('SHA-256',bytes)),b=>b.toString(16).padStart(2,'0')).join('');
export async function preparePcPackage(url,{fetcher=fetch,cryptoProvider=globalThis.crypto,onProgress=()=>{}}={}){
  const response=await fetcher(url);if(!response.ok)throw Error('Pacchetto non disponibile. Ricarica la pagina e riprova.');
  const m=await response.json(),hex=s=>typeof s==='string'&&/^[a-f0-9]{64}$/.test(s),integer=(v,a,b)=>Number.isSafeInteger(v)&&v>=a&&v<=b;
  if(m?.version!==1||m.filename!=='PocketDD-PC.zip'||!integer(m.size,1,128*1024*1024)||!hex(m.sha256)||!Array.isArray(m.parts)||m.parts.length<1||m.parts.length>8||m.parts.some((p,i)=>!p||typeof p!=='object'||p.path!==`PocketDD-PC.part${i}.bin`||!integer(p.size,1,16*1024*1024)||!hex(p.sha256))||m.parts.reduce((n,p)=>n+p.size,0)!==m.size)throw Error('Descrizione del pacchetto non valida. Nessun file scaricato.');
  const output=new Uint8Array(m.size);let offset=0;
  for(const part of m.parts){const r=await fetcher(new URL(part.path,url));if(!r.ok)throw Error('Download incompleto. Premi di nuovo Scarica per riprovare.');const data=new Uint8Array(await r.arrayBuffer());if(data.length!==part.size||await hash(data,cryptoProvider)!==part.sha256)throw Error('Download incompleto o alterato. Nessun ZIP creato; riprova.');output.set(data,offset);offset+=data.length;onProgress(Math.floor(100*offset/m.size));}
  if(await hash(output,cryptoProvider)!==m.sha256)throw Error('Verifica del pacchetto non riuscita. Nessun ZIP creato.');
  return output;
}
