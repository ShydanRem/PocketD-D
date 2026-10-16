import {test} from 'node:test';
import assert from 'node:assert/strict';
import {beyondCharacterId,beyondEndpoint,previewBeyond} from './beyond.js';
import {beyondFixture} from './beyond-fixtures.js';
import {probeBeyond} from '../scripts/beyond-client.js';
import {createBeyondLabServer} from '../scripts/beyond-lab.js';
import {createServer} from 'node:http';

const json=(value=beyondFixture(),headers={})=>new Response(JSON.stringify(value),{headers:{'Content-Type':'application/json',...headers}});
const code=value=>error=>error.code===value;
test('Beyond: URL e ID ammessi producono soltanto la destinazione fissa prevista',()=>{
  for(const input of ['123456',' https://www.dndbeyond.com/characters/123456 ','https://dndbeyond.com/characters/123456/?foo=bar#sheet','https://www.dndbeyond.com/profile/test/characters/123456']){
    assert.equal(beyondCharacterId(input),'123456');assert.equal(beyondEndpoint(input),'https://character-service.dndbeyond.com/character/v5/character/123456?includeCustomItems=true');
  }
  for(const input of ['0','01','1e5','-1','1234567890123456','http://www.dndbeyond.com/characters/123456','https://www.dndbeyond.com.evil.test/characters/123456','https://evil.test@www.dndbeyond.com/characters/123456','https://www.dndbeyond.com:123/characters/123456','https://127.0.0.1/characters/123456','https://www.dndbeyond.com/characters/123456/../../admin','https://www.dndbeyond.com/characters/123456%2fextra','https://www.dndbeyond.com/characters/123456.json',null,{}])assert.throws(()=>beyondCharacterId(input),code('INVALID_URL'));
});
test('Beyond: anteprima sintetica calcola livello, base/bonus/override e PF senza mutare dati',()=>{
  const payload=beyondFixture(),before=structuredClone(payload);
  const preview=previewBeyond(payload,'123456');
  assert.equal(preview.character.level,3);assert.equal(preview.abilities.FOR.preview,15);assert.equal(preview.hp.previewMax,24);assert.equal(preview.hp.previewCurrent,19);
  assert.equal(preview.armorClass,null);assert.equal(preview.canImport,false);assert.equal(preview.version,1);assert.notEqual(preview.format,'pocketdd-library');assert.deepEqual(payload,before);
  payload.data.classes=[{level:2},{level:1}];payload.data.bonusStats[2].value=2;payload.data.overrideStats[0].value=19;payload.data.temporaryHitPoints=7;
  const multi=previewBeyond(payload,'123456');assert.equal(multi.abilities.COS.preview,16);assert.equal(multi.abilities.FOR.preview,19);assert.equal(multi.hp.previewMax,27);assert.equal(multi.hp.previewCurrent,22);assert.equal(multi.hp.temporary,7);
});
test('Beyond: PF mancanti non diventano valori demo; override, danni e limiti sono espliciti',()=>{
  const payload=beyondFixture();delete payload.data.baseHitPoints;
  assert.equal(previewBeyond(payload,'123456').hp,null);
  payload.data.baseHitPoints=18;payload.data.overrideHitPoints=40;payload.data.bonusHitPoints=5;payload.data.removedHitPoints=99;
  const preview=previewBeyond(payload,'123456');assert.equal(preview.hp.previewMax,45);assert.equal(preview.hp.previewCurrent,0);
  payload.data.name='Un nome di personaggio più lungo del limite attuale';payload.data.overrideStats[0].value=32;
  assert.match(previewBeyond(payload,'123456').warnings.join(' '),/24 caratteri/);assert.match(previewBeyond(payload,'123456').warnings.join(' '),/caratteristica supera/);
});
test('Beyond: modificatori non supportati e riepilogo inventario non sono dichiarati importati',()=>{
  const payload=beyondFixture();payload.data.modifiers.feat.push({type:'bonus',subType:'hit-points-per-level',value:2});
  payload.data.inventory=Array.from({length:62},(_,i)=>({definition:{name:`Oggetto ${i}`},quantity:i,equipped:false}));
  const preview=previewBeyond(payload,'123456');assert.equal(preview.inventory.total,62);assert.equal(preview.inventory.items.length,60);assert.equal(preview.inventory.items[1].quantity,1);
  assert.equal(preview.canImport,false);assert.match(preview.warnings.join(' '),/calcoli parziali/);assert.match(preview.warnings.join(' '),/oltre 60/);
  payload.data.inventory[0]={};assert.equal(previewBeyond(payload,'123456').inventory.items[0].quantity,null);
});
test('Beyond: schede invalide, ID estranei, livelli e caratteristiche incomplete sono respinti',()=>{
  const payload=beyondFixture();assert.throws(()=>previewBeyond(payload,'999'),code('ID_MISMATCH'));
  for(const patch of [{name:''},{classes:[]},{classes:[{level:21}]},{classes:[{level:20},{level:1}]},{stats:[]},{stats:Array(6).fill({id:1,value:14})},{bonusStats:[{id:1,value:'14'}]},{inventory:{}},{id:undefined}]){
    assert.throws(()=>previewBeyond({...payload,data:{...payload.data,...patch}},'123456'));
  }
  for(const value of [null,{},[],{success:false,data:payload.data},{data:payload.data}])assert.throws(()=>previewBeyond(value,'123456'),code('INVALID_DATA'));
});
test('Beyond: richiesta HTTP priva di credenziali, senza redirect, e resoconto solo di campi selezionati',async()=>{
  let request;
  const payload=beyondFixture();payload.secret='must-not-be-returned';payload.data.secret='also-not-returned';
  const report=await probeBeyond('123456',{fetcher:async(url,options)=>{request={url,options};return json(payload)},now:(()=>{let t=0;return()=>t+=5})()});
  assert.equal(request.url,beyondEndpoint('123456'));assert.equal(request.options.redirect,'manual');assert.equal(request.options.credentials,'omit');assert.deepEqual(request.options.headers,{Accept:'application/json'});
  assert.equal(report.transport.status,200);assert.equal(report.transport.authenticated,false);assert.equal(report.transport.elapsedMs,5);assert.ok(report.transport.bytes>0);assert.equal(JSON.stringify(report).includes('must-not-be-returned'),false);
});
test('Beyond: errori HTTP non vengono scambiati per una scheda privata o per import riuscito',async()=>{
  for(const [status,expected] of [[301,'REDIRECT'],[302,'REDIRECT'],[401,'ACCESS_DENIED'],[403,'ACCESS_DENIED'],[404,'NOT_FOUND'],[429,'RATE_LIMITED'],[500,'UPSTREAM_ERROR']])await assert.rejects(()=>probeBeyond('123456',{fetcher:async()=>new Response('not a character',{status})}),code(expected));
  await assert.rejects(()=>probeBeyond('123456',{fetcher:async()=>{throw new Error('proxy transport failure')}}),code('NETWORK_ERROR'));
});
test('Beyond: HTML, JSON illeggibile, scheda diversa e risposta oltre limite non vengono convertiti',async()=>{
  await assert.rejects(()=>probeBeyond('123456',{fetcher:async()=>new Response('<html>challenge</html>',{headers:{'Content-Type':'text/html'}})}),code('NOT_JSON'));
  await assert.rejects(()=>probeBeyond('123456',{fetcher:async()=>new Response('{broken',{headers:{'Content-Type':'application/json'}})}),code('INVALID_JSON'));
  await assert.rejects(()=>probeBeyond('999',{fetcher:async()=>json()}),code('ID_MISMATCH'));
  await assert.rejects(()=>probeBeyond('123456',{fetcher:async()=>json(undefined,{'Content-Length':'99999999'})}),code('TOO_LARGE'));
  let cancelled=false;
  const stream=new ReadableStream({pull(controller){controller.enqueue(new Uint8Array(20))},cancel(){cancelled=true}});
  await assert.rejects(()=>probeBeyond('123456',{maxBytes:30,fetcher:async()=>new Response(stream,{headers:{'Content-Type':'application/json'}})}),code('TOO_LARGE'));
  assert.equal(cancelled,true);
});
async function listen(t,server){
  await new Promise((resolve,reject)=>{server.once('error',reject);server.listen(0,'127.0.0.1',resolve)});
  t.after(()=>new Promise(resolve=>{server.closeAllConnections();server.close(resolve)}));
  return `http://127.0.0.1:${server.address().port}`;
}
test('Beyond: timeout reale copre attesa degli header e lettura del corpo',async t=>{
  const server=createServer((req,res)=>{if(req.url==='/body'){res.writeHead(200,{'Content-Type':'application/json'});res.write('{')} });
  const local=await listen(t,server);
  for(const path of ['/headers','/body'])await assert.rejects(()=>probeBeyond('123456',{timeoutMs:70,fetcher:(_url,options)=>fetch(local+path,options)}),code('TIMEOUT'));
});
test('Beyond: laboratorio HTTP locale serve la pagina e valida input e origine prima del connettore',async t=>{
  let calls=0;
  const local=await listen(t,createBeyondLabServer({probe:async id=>{calls++;return previewBeyond(beyondFixture(),id)}}));
  const page=await fetch(local);assert.equal(page.status,200);assert.match(await page.text(),/Proviamo il collegamento/);assert.ok(page.headers.get('content-security-policy').includes("connect-src 'self'"));
  const post=(body,headers={})=>fetch(local+'/api/probe',{method:'POST',headers:{'Content-Type':'application/json',...headers},body:JSON.stringify(body)});
  let response=await post({character:'https://evil.test/123456'});assert.equal(response.status,400);assert.equal(calls,0);
  response=await post({character:'123456'},{Origin:'https://evil.test'});assert.equal(response.status,403);assert.equal(calls,0);
  response=await post({character:'123456'});assert.equal(response.status,200);assert.equal(calls,1);assert.equal((await response.json()).report.character.name,'Eroe sonda');
  assert.equal((await fetch(local+'/api/probe')).status,404);assert.equal((await fetch(local+'/../package.json')).status,404);
  response=await post({character:'x'.repeat(2000)});assert.equal(response.status,413);
});
test('Beyond: laboratorio limita richieste concorrenti e restituisce errori strutturati',async t=>{
  let release,started;
  const waiting=new Promise(resolve=>started=resolve);
  const local=await listen(t,createBeyondLabServer({probe:async()=>{started();await new Promise(resolve=>release=resolve);throw Object.assign(new Error('Connessione di prova interrotta'),{code:'NETWORK_ERROR'})}}));
  const post=()=>fetch(local+'/api/probe',{method:'POST',headers:{'Content-Type':'application/json'},body:'{"character":"123456"}'});
  const first=post();await waiting;assert.equal((await post()).status,429);release();const response=await first;
  assert.equal(response.status,502);assert.equal((await response.json()).error.code,'NETWORK_ERROR');
});
