import {writeFile} from 'node:fs/promises';
import {probeBeyond} from './beyond-client.js';

const [input,...options]=process.argv.slice(2);
if(!input||(options.length&&!(options.length===2&&options[0]==='--out'))){
  console.error('Uso: npm run beyond:probe -- "URL_o_ID" [--out /percorso/report.json]');process.exitCode=1;
}else{
  try{
    const report=await probeBeyond(input),raw=JSON.stringify(report,null,2)+'\n';
    if(options.length)await writeFile(options[1],raw,{flag:'wx',mode:0o600});
    console.log(raw);
  }catch(error){console.error(JSON.stringify({ok:false,code:error.code??'OUTPUT_ERROR',message:error.code==='EEXIST'?'Il file di destinazione esiste già: scegli un altro percorso':error.message}));process.exitCode=1}
}
