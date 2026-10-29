import {execFile} from 'node:child_process';
import {createBeyondLabServer} from './beyond-lab.js';
const port=Number(process.env.POCKETDD_APP_PORT??5173);
if(!Number.isInteger(port)||port<1||port>65535)throw new Error('POCKETDD_APP_PORT non valida');
const server=createBeyondLabServer({appRoot:new URL('../dist/',import.meta.url)});
function openBrowser(url){if(process.env.POCKETDD_NO_OPEN==='1')return;const command=process.platform==='win32'?'explorer.exe':process.platform==='darwin'?'open':'xdg-open';execFile(command,[url],()=>{});}
server.on('error',async error=>{
 if(error.code==='EADDRINUSE'){
  try{const r=await fetch(`http://127.0.0.1:${port}/api/setup`,{signal:AbortSignal.timeout(2000)}),info=await r.json();const pkg=JSON.parse(await (await import('node:fs/promises')).readFile(new URL('../package.json',import.meta.url),'utf8'));if(r.ok&&info.kind==='pocketdd-setup'&&info.appVersion===pkg.version){console.log('PocketD-D è già aperto. Riprendo la guida nello stesso indirizzo.');openBrowser(`http://127.0.0.1:${port}/?setup=1`);return;}}catch{}
  console.error('Porta occupata: chiudi la vecchia app PocketD-D e riapri Avvia-PocketDD.cmd.');
 }else console.error('Avvio non riuscito: '+(error.code??'errore'));
 process.exitCode=1;
});
server.listen(port,'127.0.0.1',()=>{
  const url=`http://127.0.0.1:${port}/?setup=1`;
  console.log(`PocketD-D PC: ${url}\nLascia questa finestra aperta. Ctrl+C per chiudere. Usa Chrome o Edge per l’USB.`);
  openBrowser(url);
});
