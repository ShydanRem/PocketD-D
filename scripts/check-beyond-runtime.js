// Raw source and native records stay in memory. Never print/persist a full PG.
import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import {beyondCoreFixture,beyondRichFixture} from '../src/beyond-fixtures.js';
import {convertBeyondFull} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';
import {probeBeyond} from './beyond-client.js';
const binary=new URL('../firmware/tests/out/ui-build/beyond-runtime',import.meta.url);
function native(source,id,heap=4194304){return new Promise((resolve,reject)=>{
  const child=spawn(binary.pathname,[id,String(heap),'--record'],{stdio:['pipe','pipe','pipe']});let output='';
  child.stderr.resume();child.stdout.on('data',part=>{output+=part;if(output.length>512*1024){child.kill();reject(Error('Unexpected native output size'));}});
  child.stdin.on('error',()=>{});child.stdin.end(source);
  const timer=setTimeout(()=>{child.kill();reject(Error('Native conversion timed out'));},15000);
  child.once('error',error=>{clearTimeout(timer);reject(error);});
  child.once('close',code=>{clearTimeout(timer);try{assert.equal(code,0,'Native converter exit');resolve(JSON.parse(output));}catch{reject(Error('Native converter did not return a valid bounded result'));}});
});}
async function compare(source,id){
  const raw=typeof source==='string'?source:JSON.stringify(source),payload=typeof source==='string'?JSON.parse(source):source;
  const report=convertBeyondFull(payload,id),actual=await native(raw,id);
  assert.equal(actual.status,report.canImport?0:1,'PC/native readiness mismatch');
  assert.equal(actual.blockers,report.conversion.blockers.length,'PC/native blocker count mismatch');
  const limitations=new Set([...(report.readiness?.manual??[]),...(report.readiness?.snapshots??[]),...(report.readiness?.unresolved??[])]);
  assert.equal(actual.limits,limitations.size,'PC/native limitation count mismatch');
  assert.ok(actual.peak<=4194304,'Native interpreter exceeded its allocation budget');
  if(!report.canImport)return {accepted:false,peak:actual.peak};
  const expected=await native(JSON.stringify(projectDevice(report.draft).packet),'--packet');
  assert.ok(actual.record===expected.record,'PC/native complete record mismatch');
  const {record,...summary}=actual;return {...summary,accepted:true,exactNativeRecordMatches:true};
}
if(process.argv.includes('--real')){
  const id=process.argv.at(-1);let source;
  const report=await probeBeyond(id,{maxBytes:1024*1024,timeoutMs:15000,fetcher:async(url,options)=>{
    const response=await fetch(url,options);
    if(!response.ok||!/^application\/json\b/i.test(response.headers.get('content-type')??''))return response;
    const reader=response.body.getReader(),chunks=[];let bytes=0;
    try{while(true){const {value,done}=await reader.read();if(done)break;bytes+=value.byteLength;if(bytes>1024*1024)throw Error('Source above native cap');chunks.push(value);}}finally{await reader.cancel();}
    source=Buffer.concat(chunks).toString('utf8');return new Response(source,{status:response.status,headers:response.headers});
  }});
  assert.ok(report.canImport,'Real source is blocked by current conversion rules');
  const result=await compare(source,id);
  console.log(JSON.stringify({kind:'real-public-http-and-host-runtime',http:report.transport.status,sourceBytes:report.transport.bytes,...result,physicalESP32:false,personalCollectionModified:false}));
}else{
  let cases=0,accepted=0,blocked=0,peak=0;
  for(const builder of [beyondCoreFixture,beyondRichFixture])for(let level=1;level<=20;level++){
    const p=builder();p.data.classes[0].level=level;
    const result=await compare(p,'123456');cases++;accepted+=result.accepted?1:0;blocked+=result.accepted?0:1;peak=Math.max(peak,result.peak);
  }
  for(let ability=0;ability<6;ability++)for(const score of [1,8,10,18,30]){
    const p=beyondRichFixture();p.data.stats[ability].value=score;
    const result=await compare(p,'123456');cases++;accepted+=result.accepted?1:0;blocked+=result.accepted?0:1;peak=Math.max(peak,result.peak);
  }
  for(const count of [0,1,60,61]){
    const p=beyondCoreFixture();p.data.inventory=Array.from({length:count},(_,i)=>({...structuredClone(p.data.inventory[0]),id:i+1}));
    const result=await compare(p,'123456');cases++;accepted+=result.accepted?1:0;blocked+=result.accepted?0:1;peak=Math.max(peak,result.peak);
  }
  const memory=await native(JSON.stringify(beyondRichFixture()),'123456',32768);assert.equal(memory.status,3,'Small heap must fail, not create a reduced PG');
  console.log(JSON.stringify({kind:'synthetic-native-parity',cases,accepted,blocked,peakBytes:peak,allAcceptedNativeRecordsIdentical:true,smallHeapRejected:true,physicalESP32:false}));
}
