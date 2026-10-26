import {execFile} from 'node:child_process';
import {createBeyondLabServer} from './beyond-lab.js';
const port=Number(process.env.POCKETDD_APP_PORT??5173);
if(!Number.isInteger(port)||port<1||port>65535)throw new Error('POCKETDD_APP_PORT non valida');
const server=createBeyondLabServer({appRoot:new URL('../dist/',import.meta.url)});
server.on('error',error=>{console.error(error.code==='EADDRINUSE'?'Porta occupata: chiudi l’altra app PocketD-D e riprova.':`Avvio non riuscito: ${error.code??'errore'}`);process.exitCode=1;});
server.listen(port,'127.0.0.1',()=>{
  const url=`http://127.0.0.1:${port}/`;
  console.log(`PocketD-D PC: ${url}\nLascia questa finestra aperta. Ctrl+C per chiudere. Usa Chrome o Edge per l’USB.`);
  if(process.env.POCKETDD_NO_OPEN==='1')return;
  const command=process.platform==='win32'?'explorer.exe':process.platform==='darwin'?'open':'xdg-open';
  execFile(command,[url],()=>{}); // The printed URL remains available if opening a browser fails.
});
