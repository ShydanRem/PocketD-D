import {readFileSync} from 'node:fs';import vm from 'node:vm';
import test from 'node:test';import assert from 'node:assert/strict';
import {chooseUsbPort,rememberUsbPort,forgetUsbPort,claimUsbPort,observeUsbPort,usbPortState} from './usb-port.js';
import {pocketSite} from '../extension/helper-origin.js';
import {checkDeviceConnection} from './device-usb.js';
test('USB selection reuses only the explicitly proven port in this page, supports change and clears on matching disconnect',async()=>{
 forgetUsbPort();let chosen=0,handler;const a={},b={},serial={requestPort:()=>Promise.resolve(++chosen===1?a:b),addEventListener:(name,fn)=>handler=fn};
 assert.equal(await chooseUsbPort(serial),a);rememberUsbPort(a,serial);assert.equal(await chooseUsbPort(serial),a);assert.equal(chosen,1);assert.equal(await chooseUsbPort(serial,{chooseNew:true}),b);assert.equal(chosen,2);handler({port:b});assert.ok(usbPortState().known);handler({port:a});assert.equal(usbPortState().known,false);forgetUsbPort();
});
test('USB exclusive lease prevents concurrent tools and always recovers, even with a failing UI subscriber',()=>{
 const stop=observeUsbPort(()=>{if(usbPortState().busy)throw Error('View failure');}),release=claimUsbPort();assert.throws(claimUsbPort,/occupato/);release();release();assert.equal(usbPortState().busy,false);stop();
});
test('Two real channel handshakes reopen the same remembered port with one chooser; changing device prompts again',async()=>{
 forgetUsbPort();let chooser=0,opens=0,controller;const port={async open(){opens++;this.readable=new ReadableStream({start(c){controller=c;}});this.writable=new WritableStream({write(data){if(new TextDecoder().decode(data).includes('PDD HELLO'))controller.enqueue(new TextEncoder().encode('PDD READY 21\n'));}});},async close(){},async setSignals(){}};
 const serial={requestPort:async()=>{chooser++;return port;}};await checkDeviceConnection({serial});await checkDeviceConnection({serial});assert.equal(chooser,1);assert.equal(opens,2);await checkDeviceConnection({serial,chooseNew:true});assert.equal(chooser,2);forgetUsbPort();
});
test('Site helper permits only Pocket site paths and local tools; never arbitrary remote pages or Beyond',()=>{
 for(const u of ['https://shydanrem.github.io/PocketD-D/','http://127.0.0.1:5173/','http://127.0.0.1:8271/install/'])assert.ok(pocketSite(u));
 for(const u of ['https://evil.test/PocketD-D/','https://shydanrem.github.io/other/','https://www.dndbeyond.com/','http://127.0.0.1:5173/other/','https://shydanrem.github.io/PocketD-D/../other/','file:///tmp/index.html'])assert.equal(pocketSite(u),false);
});

test('Site relay handles only presence/open, rejects foreign messages and never forwards credential-shaped fields',()=>{
 let receive;const posts=[],sent=[],win={addEventListener:(name,fn)=>receive=fn,postMessage:value=>posts.push(value)},location={href:'https://shydanrem.github.io/PocketD-D/',origin:'https://shydanrem.github.io'};
 const chrome={runtime:{getManifest:()=>({version:'0.93.0'}),sendMessage:(m,reply)=>{sent.push(m);reply({ok:true});}}};
 vm.runInNewContext(readFileSync(new URL('../extension/site-helper.js',import.meta.url),'utf8'),{window:win,top:win,location,chrome,URL});
 const m={type:'pocketdd-helper-request',id:'a'.repeat(32),action:'open',cookie:'never-forward-me'};
 receive({source:{},origin:location.origin,data:m});receive({source:win,origin:'https://evil.test',data:m});receive({source:win,origin:location.origin,data:{...m,action:'cookies'}});assert.equal(sent.length,0);
 receive({source:win,origin:location.origin,data:m});assert.equal(JSON.stringify(sent),'[{"type":"open-pocket-tools"}]');assert.equal(JSON.stringify(posts).includes('never-forward-me'),false);
});
test('Opening Pocket PC deduplicates its own tab and never hijacks a tab navigated elsewhere',async()=>{
 const {openPocketTools}=await import('../extension/open-tools.js');let created=0,focused=0,url='chrome-extension://synthetic/account.html';
 const api={runtime:{getURL:()=>url},tabs:{create:async()=>{created++;return {id:created};},get:async()=>({url}),update:async()=>{focused++;}}};
 await Promise.all([openPocketTools(api),openPocketTools(api)]);assert.equal(created,1);await openPocketTools(api);assert.equal(focused,1);api.tabs.get=async()=>({url:'https://elsewhere.test/'});await openPocketTools(api);assert.equal(created,2);assert.equal(focused,1);
});
test('Pocket PC tab can be found again after worker restart without storing a port or account',async()=>{
 const {openPocketTools}=await import('../extension/open-tools.js');let saved={},created=0,focused=0;const url='chrome-extension://synthetic/account.html',storage={session:{get:async()=>saved,set:async data=>saved=data}},tabs={create:async()=>{created++;return {id:5};},get:async()=>({url}),update:async()=>{focused++;}};
 await openPocketTools({runtime:{getURL:()=>url},storage,tabs});await openPocketTools({runtime:{getURL:()=>url},storage,tabs});assert.equal(created,1);assert.equal(focused,1);assert.deepEqual(saved,{pocketToolsTab:5});
});
