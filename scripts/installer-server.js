import {createServer} from 'node:http';
import {readFile} from 'node:fs/promises';
import {pathToFileURL} from 'node:url';
export function createInstallerServer(root=new URL('../dist/install/',import.meta.url)){
  const server=createServer(async(req,res)=>{
    res.setHeader('X-Content-Type-Options','nosniff');res.setHeader('Cache-Control','no-store');res.setHeader('Referrer-Policy','no-referrer');
    res.setHeader('Content-Security-Policy',"default-src 'self'; connect-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; object-src 'none'; base-uri 'none'; frame-ancestors 'none'");
    const port=server.address().port;
    if(![`localhost:${port}`,`127.0.0.1:${port}`].includes(req.headers.host)){res.writeHead(403);res.end('Usa localhost.');return;}
    if(req.headers.origin&&![`http://localhost:${port}`,`http://127.0.0.1:${port}`].includes(req.headers.origin)){res.writeHead(403);res.end();return;}
    if(req.method!=='GET'&&req.method!=='HEAD'){res.writeHead(405);res.end();return;}
    const name=req.url==='/'?'index.html':req.url.slice(1);
    if(!/^(index\.html|manifest\.json|assets\/[\w.-]+\.(js|css)|firmware\/(bootloader|partitions|app)\.bin|licenses\/[\w.-]+\.txt)$/.test(name)){res.writeHead(404);res.end();return;}
    try{
      const data=await readFile(new URL(name,root));const ext=name.split('.').pop(),type={html:'text/html; charset=utf-8',js:'text/javascript; charset=utf-8',css:'text/css; charset=utf-8',json:'application/json',bin:'application/octet-stream',txt:'text/plain; charset=utf-8'}[ext];
      res.writeHead(200,{'Content-Type':type,'Content-Length':data.length});res.end(req.method==='HEAD'?undefined:data);
    }catch{res.writeHead(404);res.end('Installer assente. Esegui npm run installer:build oppure usa il pacchetto compilato.');}
  });server.headersTimeout=10000;server.requestTimeout=15000;return server;
}
if(process.argv[1]&&import.meta.url===pathToFileURL(process.argv[1]).href){
  const port=Number(process.env.POCKETDD_INSTALLER_PORT??8787);
  if(!Number.isInteger(port)||port<1||port>65535)throw new Error('Porta non valida');
  const server=createInstallerServer();server.on('error',e=>{console.error(`Installer: ${e.code}`);process.exitCode=1;});
  server.listen(port,'127.0.0.1',()=>console.log(`PocketD-D installer: http://127.0.0.1:${port} · apri in Chrome o Edge. Ctrl+C per chiudere.`));
}
