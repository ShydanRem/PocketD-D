import {BeyondError,beyondEndpoint,beyondCharacterId} from '../src/beyond.js';
import {convertBeyond} from '../src/beyond-conversion.js';
export const MAX_BEYOND_BYTES=2*1024*1024;

export async function probeBeyond(input,{fetcher=globalThis.fetch,timeoutMs=10000,maxBytes=MAX_BEYOND_BYTES,now=()=>Date.now()}={}){
  const id=beyondCharacterId(input),endpoint=beyondEndpoint(id),start=now();
  const abort=new AbortController(),timer=setTimeout(()=>abort.abort(),timeoutMs);
  let response,reader;
  try{
    response=await fetcher(endpoint,{method:'GET',headers:{Accept:'application/json'},redirect:'manual',credentials:'omit',signal:abort.signal});
    if(response.status>=300&&response.status<400)throw new BeyondError('REDIRECT','Il servizio ha richiesto un reindirizzamento: la prova si ferma');
    if(response.status===401||response.status===403)throw new BeyondError('ACCESS_DENIED','Accesso rifiutato dal servizio o dal proxy: verificare visibilità della scheda e accesso di rete');
    if(response.status===404)throw new BeyondError('NOT_FOUND','Personaggio non trovato dal servizio');
    if(response.status===429)throw new BeyondError('RATE_LIMITED','Limite di richieste raggiunto: riprova più tardi');
    if(!response.ok)throw new BeyondError('UPSTREAM_ERROR',`Il servizio ha risposto HTTP ${response.status}`);
    if(!/\bapplication\/(?:[\w.-]+\+)?json\b/i.test(response.headers.get('content-type')??''))throw new BeyondError('NOT_JSON','Risposta diversa da JSON: potrebbe essere una pagina di accesso o protezione');
    const length=response.headers.get('content-length');
    if(length&&Number(length)>maxBytes)throw new BeyondError('TOO_LARGE','La risposta supera il limite di 2 MiB della prova');
    if(!response.body)throw new BeyondError('INVALID_DATA','Risposta vuota');
    reader=response.body.getReader();let bytes=0;const chunks=[];
    while(true){const {done,value}=await reader.read();if(done)break;bytes+=value.byteLength;if(bytes>maxBytes)throw new BeyondError('TOO_LARGE','La risposta supera il limite di 2 MiB della prova');chunks.push(Buffer.from(value))}
    let payload;try{payload=JSON.parse(Buffer.concat(chunks).toString('utf8'))}catch{throw new BeyondError('INVALID_JSON','Il servizio ha restituito JSON non leggibile')}
    let converted;
    try{converted=convertBeyond(payload,id)}catch(error){
      if(error instanceof BeyondError)throw error;
      throw new BeyondError('CONVERSION_ERROR','Risposta ricevuta, ma formato o regole di conversione non gestiti. Nessuna scheda creata');
    }
    return {...converted,checks:{...converted.checks,http:'ok'},transport:{endpoint,status:response.status,bytes,elapsedMs:Math.max(0,now()-start),authenticated:false}};
  }catch(error){
    if(error instanceof BeyondError)throw error;
    if(abort.signal.aborted)throw new BeyondError('TIMEOUT','Tempo massimo di lettura superato');
    throw new BeyondError('NETWORK_ERROR','Connessione non riuscita. Verifica rete, proxy e accesso a character-service.dndbeyond.com');
  }finally{clearTimeout(timer);abort.abort();if(reader)try{await reader.cancel()}catch{}}
}
