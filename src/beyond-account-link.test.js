import test from 'node:test';
import assert from 'node:assert/strict';
import {preparePocketAccount,pairPocketAccount} from '../extension/account-link.js';
const now=()=>1800000000000,cookie='synthetic-session-only',user='1234',nonce='0123456789abcdef0123456789abcdef';
function preparation({grant=true,changed=false}={}){let reads=0;const calls=[];return {calls,api:{permissions:{request:async request=>{calls.push(request);return grant;}},cookies:{get:async request=>{calls.push(request);return {value:changed&&reads++?cookie+'-changed':cookie,expirationDate:2000000000};}}},check:async({onIdentity})=>{onIdentity(user);return {ok:true,characters:[{id:'123456'}]};},now};}
test('Account association explicitly requests cookie permission, reads fixed cookie twice, binds checked identity and keeps secrets out of check results',async()=>{
 const options=preparation(),result=await preparePocketAccount(options);assert.equal(result.cookie,cookie);assert.equal(result.user,user);assert.equal(result.count,1);assert.ok(options.calls[0].permissions.includes('cookies'));assert.deepEqual(options.calls[1],{url:'https://auth-service.dndbeyond.com/v1/cobalt-token',name:'CobaltSession'});
 const denied=preparation({grant:false});await assert.rejects(preparePocketAccount(denied));assert.equal(denied.calls.length,1);
 await assert.rejects(preparePocketAccount(preparation({changed:true})),/login è cambiato/);
 const c=new AbortController();c.abort();await assert.rejects(preparePocketAccount({...preparation(),signal:c.signal}));
});
function usb({accept=true,badNonce=false,fail=false,driverFail=false}={}) {
 let controller;const sent=[],signals=[];let closed=0;
 const enqueue=text=>controller.enqueue(new TextEncoder().encode(text+'\n'));
 const port={readable:new ReadableStream({start(c){controller=c;}}),writable:new WritableStream({write(bytes){const line=new TextDecoder().decode(bytes);sent.push(line);if(line==='PDD ACCOUNT HELLO 1\n')enqueue(`PDD ACCOUNT CODE 1 ${nonce} 123456`);if(line.startsWith('PDD ACCOUNT OFFER'))enqueue(`PDD ACCOUNT ${fail?'FAILED':'LINKED'} 1 ${badNonce?'f'.repeat(32):nonce}`);}}),open:async()=>{if(driverFail)throw Error(cookie);},setSignals:async value=>signals.push(value),close:async()=>{closed++;}};
 return {sent,signals,enqueue,get closed(){return closed;},serial:{requestPort:async()=>port},onCode:code=>{assert.equal(code,'123456');assert.equal(sent.some(s=>s.startsWith('PDD ACCOUNT OFFER')),false);enqueue(`PDD ACCOUNT ${accept?'ACCEPT':'CANCELLED'} 1 ${nonce}`);}};
}
test('Account USB transfers only after matching physical approval, no modem reset, and succeeds only on matching durable LINKED',async()=>{
 const mock=usb();const result=await pairPocketAccount({cookie,user,expires:now()+100000},{...mock,now});assert.deepEqual(result,{linked:true});assert.deepEqual(mock.signals,[{dataTerminalReady:false,requestToSend:false}]);assert.equal(mock.sent.filter(s=>s.startsWith('PDD ACCOUNT OFFER')).length,1);assert.ok(mock.sent[1].includes(cookie));
});
test('Account USB rejects cancelled, foreign or failed acknowledgement without claiming association or replaying a secret',async()=>{
 for(const options of [{accept:false},{badNonce:true},{fail:true},{driverFail:true}]){const mock=usb(options);await assert.rejects(pairPocketAccount({cookie,user,expires:now()+100000},{...mock,now}),error=>!error.message.includes(cookie));assert.ok(mock.sent.filter(s=>s.startsWith('PDD ACCOUNT OFFER')).length<=1);if(options.accept===false)assert.equal(mock.sent.some(s=>s.includes(cookie)),false);}
});
test('Account USB rejects expired preparations and unsafe cookie syntax before choosing a port',async()=>{
 let chosen=0;for(const access of [{cookie,user,expires:now()-1},{cookie:'bad;header',user,expires:now()+10000}])await assert.rejects(pairPocketAccount(access,{now,serial:{requestPort(){chosen++;}}}));assert.equal(chosen,0);
});
