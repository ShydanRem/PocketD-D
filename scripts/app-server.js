import {createBeyondLabServer} from './beyond-lab.js';
const port=Number(process.env.POCKETDD_APP_PORT ?? 5173);
if(!Number.isInteger(port)||port<1||port>65535){console.error('POCKETDD_APP_PORT non valida');process.exitCode=1}
else{
  const server=createBeyondLabServer({appRoot:new URL('../dist/',import.meta.url)});
  server.on('error',error=>{console.error(`Avvio non riuscito: ${error.code??'errore'}`);process.exitCode=1});
  server.listen(port,'127.0.0.1',()=>console.log(`PocketD-D PC: http://127.0.0.1:${port} · importazione Beyond, nessuna scrittura remota`));
}
