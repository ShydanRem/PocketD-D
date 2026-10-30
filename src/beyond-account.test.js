import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {checkAccount, ACCOUNT_ORIGINS} from '../extension/account-check.js';
const claim='http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier';
const now=()=>1800000000000;
const token=(claims={})=>[Buffer.from('{}').toString('base64url'),Buffer.from(JSON.stringify({[claim]:'123456',exp:now()/1000+120,...claims})).toString('base64url'),'synthetic_signature'].join('.');
const auth=()=>({token:token(),ttl:120,privateField:'must never leave the response'});
const envelope=(characters=[])=>({success:true,data:{characters}});
const pg={id:71,name:'Eroe sintetico',level:6,raceName:'Elfo',classDescription:'Mago 6',secret:'do not return'};
const json=(value,status=200)=>new Response(JSON.stringify(value),{status,headers:{'content-type':'application/json'}});
function fixture({authentication=auth(),list=envelope([pg]),authResponse,listResponse}={}) {
  const calls=[];
  const fetcher=async(url,options)=>{calls.push({url,options});return calls.length===1?(authResponse??json(authentication)):(listResponse??json(list));};
  return {calls,run:options=>checkAccount({fetcher,now,...options})};
}
test('Account check: auth with browser session, bearer-only list, distinct checks, bounded projection; no full PG reads',async()=>{
  const h=fixture(),r=await h.run();assert.equal(r.ok,true);assert.equal(r.completeness,'unverified');
  assert.deepEqual(r.characters,[{id:'71',name:'Eroe sintetico',level:6,race:'Elfo',class:'Mago 6'}]);
  assert.deepEqual(Object.values(r.checks),[true,true,true,true,true,true]);
  assert.equal(h.calls.length,2);assert.equal(h.calls[0].url,'https://auth-service.dndbeyond.com/v1/cobalt-token');
  assert.equal(h.calls[0].options.credentials,'include');assert.equal(h.calls[0].options.method,'POST');assert.equal(h.calls[0].options.body,'{}');
  assert.equal(h.calls[1].url,'https://character-service.dndbeyond.com/character/v5/characters/list?userId=123456');
  assert.equal(h.calls[1].options.credentials,'omit');assert.equal(h.calls[1].options.method,'GET');assert.equal(h.calls[1].options.headers.Authorization,'Bearer '+token());
  for(const {options} of h.calls){assert.equal(options.redirect,'error');assert.equal(options.cache,'no-store');assert.equal(options.referrerPolicy,'no-referrer');assert.ok(options.signal);assert.equal(options.headers.Cookie,undefined);}
  const output=JSON.stringify(r);for(const text of [token(),'123456','must never','privateField','secret'])assert.ok(!output.includes(text));
});
test('Account check: no silent campaign fallback, empty is valid but not completeness proof',async()=>{
  const r=await fixture({list:envelope()}).run();assert.equal(r.ok,true);assert.deepEqual(r.characters,[]);assert.equal(r.completeness,'unverified');
});
test('Account check: pagination explicitly reported as partial, not imported or assumed complete',async()=>{
  for(const pagination of [{total:2},{pages:2}]){const r=await fixture({list:{...envelope([pg]),pagination}}).run();assert.equal(r.ok,true);assert.equal(r.completeness,'partial');}
});
test('Account check: valid HTTP is not valid JSON or valid authenticated identity',async()=>{
  const h=fixture({authResponse:new Response('<html>login page</html>',{headers:{'content-type':'text/html'}})}),r=await h.run();
  assert.equal(r.ok,false);assert.equal(r.code,'format');assert.equal(r.checks.authHTTP,true);assert.equal(r.checks.authJSON,false);assert.equal(r.checks.session,null);assert.equal(r.checks.listHTTP,null);assert.equal(h.calls.length,1);
});
test('Account check: malformed JSON and unrecognized successful envelope are separate failures',async()=>{
  const h=fixture({listResponse:new Response('{"broken":',{headers:{'content-type':'application/json'}})}),r=await h.run();assert.equal(r.checks.listHTTP,true);assert.equal(r.checks.listJSON,false);assert.equal(r.checks.catalog,null);
  const invalidUTF=await fixture({authResponse:new Response(new Uint8Array([255]),{headers:{'content-type':'application/json'}})}).run();assert.equal(invalidUTF.code,'format');assert.equal(invalidUTF.checks.authJSON,false);
  for(const list of [{success:false,data:{characters:[pg]}},{success:true,data:[pg]},envelope([pg,{...pg}]),envelope([{...pg,id:'71'}]),envelope([{...pg,level:21}]),envelope([{...pg,name:'\u0000'}])]){
    const result=await fixture({list}).run();assert.equal(result.ok,false);assert.equal(result.code,'catalog');assert.equal(result.checks.listJSON,true);assert.equal(result.checks.catalog,false);assert.deepEqual(result.characters,[]);
  }
});
test('Account check: reject anonymous/malformed/expired tokens before catalogue access',async()=>{
  for(const t of ['', 'opaque_secret', 'a.!.b', token({[claim]:''}), token({[claim]:'0'}), token({[claim]:'https://evil.test'}),token({exp:null}),token({exp:now()/1000-1}),token({exp:now()/1000+10})]){
    const h=fixture({authentication:{token:t}}),r=await h.run();assert.equal(r.ok,false);assert.equal(r.checks.authJSON,true);assert.equal(r.checks.session,false);assert.equal(r.checks.listHTTP,null);assert.equal(h.calls.length,1);assert.deepEqual(r.characters,[]);
  }
});
test('Account check: token expiry during list prevents publishing stale catalogue',async()=>{
  let count=0;const h=fixture(),r=await h.run({now:()=>now()+(count++?130000:0)});assert.equal(r.ok,false);assert.equal(r.code,'expired');assert.equal(r.checks.listJSON,true);assert.deepEqual(r.characters,[]);
});
test('Account check: auth rejection, service outage and network error are distinguished without exception/body disclosure',async()=>{
  for(const stage of ['auth','list'])for(const status of [401,403,429,500]){
    const response=json({secret:token()},status),h=fixture({[stage==='auth'?'authResponse':'listResponse']:response}),r=await h.run();
    assert.equal(r.stage,stage);assert.equal(r.status[stage],status);assert.equal(r.code,status===401||status===403?'rejected':'http');assert.equal(r.checks[stage+'HTTP'],false);assert.equal(r.checks[stage+'JSON'],null);assert.ok(!JSON.stringify(r).includes(token()));assert.deepEqual(r.characters,[]);
  }
  const r=await checkAccount({fetcher:async()=>{throw Error('Cookie=secret; '+token());},now});assert.equal(r.code,'network');assert.ok(!JSON.stringify(r).includes('Cookie='));assert.equal(r.checks.authHTTP,null);
});
test('Account check: body size checked both declared and streamed, no silently cut list',async()=>{
  for(const authResponse of [new Response('{}',{headers:{'content-type':'application/json','content-length':'999999'}}),new Response(' '.repeat(32769),{headers:{'content-type':'application/json'}})]){
    const r=await fixture({authResponse}).run();assert.equal(r.code,'size');assert.equal(r.checks.authHTTP,true);assert.equal(r.checks.authJSON,false);
  }
  const r=await fixture({list:envelope(Array.from({length:301},(_,i)=>({...pg,id:i+1})))}).run();assert.equal(r.code,'size');assert.deepEqual(r.characters,[]);
});
test('Account check: timeout and external cancellation, no retries or cached session across checks',async()=>{
  const fetcher=async(_url,{signal})=>new Promise((_,reject)=>{signal.addEventListener('abort',()=>reject(signal.reason),{once:true});if(signal.aborted)reject(signal.reason);});
  const keepAlive=setTimeout(()=>{},1000);
  try{assert.equal((await checkAccount({fetcher,now,timeoutMs:5})).code,'timeout');}finally{clearTimeout(keepAlive);}
  const controller=new AbortController();controller.abort();assert.equal((await checkAccount({fetcher,now,signal:controller.signal})).code,'cancelled');
  let calls=0;const f=async()=>{calls++;return calls===1||calls===3?json(auth()):json(envelope());};await checkAccount({fetcher:f,now});await checkAccount({fetcher:f,now});assert.equal(calls,4);
});
test('Account check: UI projects only plain text even if character names contain markup',async()=>{
  const r=await fixture({list:envelope([{...pg,name:'<img src=x onerror=alert(1)>'}])}).run();assert.equal(r.ok,true);assert.equal(r.characters[0].name,'<img src=x onerror=alert(1)>');
  const source=readFileSync(new URL('../extension/account.js',import.meta.url),'utf8');assert.ok(!/innerHTML|localStorage|storage\.|sendMessage|console\./.test(source));assert.match(source,/name\.textContent=pg\.name/);
});
test('Account check: opt-in host permissions only, no cookie API / content injection / bridge dependency',()=>{
  const manifest=JSON.parse(readFileSync(new URL('../extension/manifest.json',import.meta.url),'utf8'));
  assert.deepEqual(manifest.optional_host_permissions,ACCOUNT_ORIGINS);assert.ok(!manifest.permissions.includes('cookies'));
  assert.ok(ACCOUNT_ORIGINS.every(x=>!manifest.host_permissions.includes(x)));
  const source=readFileSync(new URL('../extension/account-check.js',import.meta.url),'utf8');assert.ok(!/chrome\.cookies|storage\.|console\.|127\.0\.0\.1/.test(source));
});
