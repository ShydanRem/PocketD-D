import {createServer} from 'node:http';
import {readFile,access} from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
import {probeBeyond} from './beyond-client.js';
import {beyondCharacterId} from '../src/beyond.js';

const assets=new Map([['/',['index.html','text/html']],['/lab.js',['lab.js','text/javascript']],['/lab.css',['lab.css','text/css']]]);
const reply=(res,status,data)=>{res.writeHead(status,{'Content-Type':'application/json; charset=utf-8'});res.end(JSON.stringify(data))};
export function createBeyondLabServer({probe=probeBeyond, appRoot=null}={}){
  let busy=false;
  const server=createServer(async(req,res)=>{
    res.setHeader('Cache-Control','no-store');res.setHeader('X-Content-Type-Options','nosniff');res.setHeader('Referrer-Policy','no-referrer');
    res.setHeader('Content-Security-Policy',"default-src 'self'; connect-src 'self'; style-src 'self' 'unsafe-inline'; img-src 'self' data:; object-src 'none'; base-uri 'none'; frame-ancestors 'none'");
    const port=server.address()?.port,hosts=[`127.0.0.1:${port}`,`localhost:${port}`];
    if(!hosts.includes(req.headers.host)){reply(res,403,{error:{code:'HOST_DENIED',message:'Usa l’indirizzo locale indicato all’avvio'}});return}
    const origin=req.headers.origin;
    if(origin&&!hosts.some(host=>origin===`http://${host}`)){reply(res,403,{error:{code:'ORIGIN_DENIED',message:'Richiesta da un’altra origine rifiutata'}});return}
    let pathname;try{pathname=new URL(req.url,'http://127.0.0.1').pathname}catch{reply(res,400,{error:{code:'INVALID_PATH',message:'Percorso non valido'}});return;}
    if(appRoot&&req.method==='GET'&&pathname==='/api/setup'){
      let installer=false;try{await access(new URL('install/manifest.json',appRoot));installer=true}catch{}
      const pkg=JSON.parse(await readFile(new URL('../package.json',import.meta.url),'utf8'));
      reply(res,200,{kind:'pocketdd-setup',version:1,appVersion:pkg.version,installer});return;
    }
    if(appRoot&&req.method==='GET'&&pathname.startsWith('/install/')){
      const name=pathname==='/install/'?'index.html':pathname.slice('/install/'.length);
      if(!/^(?:index\.html|manifest\.json|assets\/[\w.-]+\.(?:js|css|png|pdc)|firmware\/(?:bootloader|partitions|app)\.bin|licenses\/[\w.-]+\.txt)$/.test(name)){reply(res,404,{error:{code:'NOT_FOUND',message:'Percorso non disponibile'}});return;}
      try{const data=await readFile(new URL('install/'+name,appRoot)),ext=name.split('.').at(-1);const mime={html:'text/html; charset=utf-8',js:'text/javascript; charset=utf-8',css:'text/css; charset=utf-8',json:'application/json',png:'image/png',bin:'application/octet-stream',pdc:'application/octet-stream',txt:'text/plain; charset=utf-8'}[ext];res.setHeader('Content-Security-Policy',"default-src 'self'; connect-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; object-src 'none'; base-uri 'none'; frame-ancestors 'self'");res.writeHead(200,{'Content-Type':mime});res.end(data);}catch{reply(res,404,{error:{code:'INSTALLER_MISSING',message:'Installer assente. Usa il pacchetto PC completo.'}});}return;
    }
    if(appRoot && req.method==='GET' && (pathname==='/' || /^\/assets\/[a-zA-Z0-9_.-]+$/.test(pathname))){
      const path=pathname==='/'?'index.html':pathname.slice(1);
      const mime=path.endsWith('.html')?'text/html':path.endsWith('.js')?'text/javascript':path.endsWith('.css')?'text/css':'application/octet-stream';
      try{const data=await readFile(new URL(path,appRoot));res.writeHead(200,{'Content-Type':`${mime}; charset=utf-8`});res.end(data)}
      catch{reply(res,404,{error:{code:'ASSET_ERROR',message:'Build assente: esegui npm run build'}})}return;
    }
    if(!appRoot&&req.method==='GET'&&assets.has(req.url)){
      try{const [name,mime]=assets.get(req.url);const data=await readFile(new URL(`../tools/beyond-lab/${name}`,import.meta.url));res.writeHead(200,{'Content-Type':`${mime}; charset=utf-8`});res.end(data)}catch{reply(res,500,{error:{code:'ASSET_ERROR',message:'Pagina di prova non disponibile'}})}return;
    }
    if(req.url!=='/api/probe'||req.method!=='POST'){reply(res,404,{error:{code:'NOT_FOUND',message:'Percorso non disponibile'}});return}
    if(!/^application\/json(?:;|$)/i.test(req.headers['content-type']??'')){reply(res,415,{error:{code:'CONTENT_TYPE',message:'Invia JSON'}});return}
    if(busy){reply(res,429,{error:{code:'BUSY',message:'Una prova è già in corso'}});return}
    busy=true;
    try{
      let raw='',bytes=0;
      for await(const chunk of req){bytes+=chunk.length;if(bytes>1024){reply(res,413,{error:{code:'REQUEST_TOO_LARGE',message:'Richiesta troppo grande'}});return}raw+=chunk.toString('utf8')}
      let body;try{body=JSON.parse(raw)}catch{reply(res,400,{error:{code:'INVALID_JSON',message:'Richiesta JSON non valida'}});return}
      const id=beyondCharacterId(body?.character);
      const report=await probe(id);reply(res,200,{ok:true,report});
    }catch(error){reply(res,error.code==='INVALID_URL'?400:502,{ok:false,error:{code:error.code??'PROBE_ERROR',message:error.code?error.message:'Prova non riuscita'}})}finally{busy=false}
  });
  server.requestTimeout=15000;server.headersTimeout=10000;
  return server;
}
if(process.argv[1]&&import.meta.url===pathToFileURL(process.argv[1]).href){
  const port=Number(process.env.POCKETDD_BEYOND_PORT??5174);
  if(!Number.isInteger(port)||port<1||port>65535){console.error('POCKETDD_BEYOND_PORT non valida');process.exitCode=1}
  else{
    const server=createBeyondLabServer();
    server.on('error',error=>{console.error(`Avvio non riuscito: ${error.code??'errore'}`);process.exitCode=1});
    server.listen(port,'127.0.0.1',()=>console.log(`Banco di prova Beyond: http://127.0.0.1:${port} · sola lettura, nessuna modifica alle schede`));
  }
}
