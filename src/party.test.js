import {test} from 'node:test';
import assert from 'node:assert/strict';
import {PARTY_PROTOCOL,createParty,validateParty,heroProfile,actorId,applyPartyCommand,acceptPartySnapshot} from './party.js';
import {PartyConnection,PARTY_CHECKPOINT} from './party-transport.js';
import {initialState} from './model.js';

const id=n=>n.toString(16).padStart(24,'0');
const host=id(1),player=id(2),sessionId=id(100);
const hero=heroProfile(initialState(),12);
const command=(session,clientId,action,payload={},patch={})=>({protocol:PARTY_PROTOCOL,sessionId:session.id,clientId,seq:(session.receipts[clientId]?.seq??0)+1,baseRevision:session.revision,action,payload,...patch});
const apply=(s,c,a,p={})=>{const result=applyPartyCommand(s,command(s,c,a,p));assert.equal(result.error,null);return result.session};
function party(){let s=createParty(sessionId,host);s=apply(s,host,'join',hero);return apply(s,player,'join',{...hero,name:'Mira',initiative:8})}

test('ingresso associa identità distinte, senza copiare o mutare le schede personali',()=>{
  const original=initialState(),copy=structuredClone(original),s=party();
  assert.equal(s.members.length,2);assert.equal(s.encounter.participants[1].name,'Mira');
  assert.deepEqual(validateParty(JSON.parse(JSON.stringify(s))),s);
  assert.deepEqual(original,copy);assert.equal('inventory' in s,false);assert.equal(s.revision,2);
  const again=applyPartyCommand(s,command(s,player,'join',hero));assert.match(again.error,/già/);assert.equal(again.session.encounter.participants.length,2);
});
test('solo il coordinatore può gestire turni, PF, ostacoli, piazzamenti e avversari',()=>{
  let s=party();
  for(const action of ['start','next','stop','hp','wall','place','npc']){
    const result=applyPartyCommand(s,command(s,player,action,{actorId:actorId(host),delta:-1,cell:0}));
    assert.match(result.error,/coordinatore/);assert.deepEqual(result.session.encounter,s.encounter);assert.equal(result.session.revision,s.revision);s=result.session;
  }
  s=apply(s,player,'initiative',{initiative:18});assert.equal(s.encounter.participants[1].initiative,18);
  s=apply(s,host,'start');assert.equal(s.encounter.activeId,actorId(player));
  assert.match(applyPartyCommand(s,command(s,player,'initiative',{initiative:30})).error,/Termina/);
});
test('ritrasmissioni, comandi vecchi e sequenze mancanti non ripetono danni o avanzamenti',()=>{
  let s=party(),hurt=command(s,host,'hp',{actorId:actorId(player),delta:-5});
  const before=structuredClone(s);s=applyPartyCommand(s,hurt).session;
  assert.deepEqual(before,party());assert.equal(s.encounter.participants[1].hp,19);
  for(let i=0;i<8;i++){const retry=applyPartyCommand(s,hurt);assert.equal(retry.duplicate,true);assert.equal(retry.session,s)}
  s=apply(s,host,'start');const advance=command(s,host,'next');s=applyPartyCommand(s,advance).session;
  assert.equal(applyPartyCommand(s,advance).session,s);assert.equal(applyPartyCommand(s,hurt).session,s);
  assert.throws(()=>applyPartyCommand(s,{...advance,seq:advance.seq+2}),/Sequenza/);
  const restored=validateParty(JSON.parse(JSON.stringify(s)));assert.deepEqual(applyPartyCommand(restored,hurt).session,restored);
});
test('uno stato obsoleto viene aggiornato senza eseguire la scelta; rifiuti consumano la sequenza',()=>{
  let s=party();const stale=command(s,player,'initiative',{initiative:30});s=apply(s,host,'hp',{actorId:actorId(host),delta:-1});
  const result=applyPartyCommand(s,stale);assert.match(result.error,/tavolo è cambiato/);assert.equal(result.session.encounter.participants[1].initiative,8);
  const retry=applyPartyCommand(result.session,stale);assert.equal(retry.duplicate,true);assert.equal(retry.error,result.error);
  s=apply(result.session,player,'initiative',{initiative:30});assert.equal(s.encounter.participants[1].initiative,30);
});
test('mappa condivisa rispetta proprietario del turno, muri, collisioni e budget',()=>{
  let s=party();s=apply(s,host,'place',{actorId:actorId(host),cell:40});s=apply(s,host,'place',{actorId:actorId(player),cell:42});s=apply(s,host,'wall',{cell:31});s=apply(s,host,'start');
  let result=applyPartyCommand(s,command(s,player,'move',{cell:39}));assert.match(result.error,/tuo personaggio/);s=result.session;
  s=apply(s,host,'move',{cell:39});assert.equal(s.encounter.grid.spent,1);
  assert.match(applyPartyCommand(s,command(s,host,'move',{cell:31})).error,/raggiungibile/);
  s=apply(s,host,'next');assert.equal(s.encounter.grid.spent,0);s=apply(s,player,'move',{cell:43});assert.equal(s.encounter.grid.positions[actorId(player)],43);
  assert.match(applyPartyCommand(s,command(s,host,'wall',{cell:1})).error,/Termina/);
  s=apply(s,host,'stop');assert.equal(s.encounter.grid.positions[actorId(player)],43);
});
test('snapshot stranieri, regressivi, corrotti e identità non valide sono respinti',()=>{
  const s=party(),newer=apply(s,host,'hp',{actorId:actorId(player),delta:-2});
  assert.equal(acceptPartySnapshot(newer,s,sessionId,host),newer);
  const rejected=applyPartyCommand(s,command(s,player,'next')).session;
  assert.equal(acceptPartySnapshot(rejected,s,sessionId,host),rejected);
  assert.throws(()=>acceptPartySnapshot(s,{...s,id:id(999)},sessionId,host));
  assert.throws(()=>acceptPartySnapshot(s,{...s,coordinatorId:id(5)},sessionId,host));
  for(const patch of [{protocol:2},{members:[...s.members,s.members[0]]},{receipts:{}},{encounter:{...s.encounter,participants:[{id:'hero',initiative:2}]}},{revision:-1}])assert.throws(()=>validateParty({...s,...patch}));
  assert.throws(()=>applyPartyCommand(s,command(s,id(4),'hp')));
  assert.throws(()=>applyPartyCommand(s,command(s,host,'hp',{}, {sessionId:id(6)})));
});
test('limite di 12 partecipanti, ingresso durante combattimento e validazione sono atomici',()=>{
  let s=party();s=apply(s,host,'start');const late=applyPartyCommand(s,command(s,id(3),'join',hero));assert.match(late.error,/Termina/);assert.equal(late.session,s);
  s=apply(s,host,'stop');
  for(let i=3;i<=12;i++)s=apply(s,id(i),'join',hero);
  const full=applyPartyCommand(s,command(s,id(13),'join',hero));assert.match(full.error,/12/);assert.equal(full.session,s);assert.equal(Object.keys(s.receipts).length,12);
  assert.equal(applyPartyCommand(s,command(s,host,'hp',{actorId:actorId(host),delta:NaN})).session.revision,s.revision);
  assert.match(applyPartyCommand(s,command(s,player,'initiative',{initiative:51})).error,/Partecipante/);
});

function harness(t){
  let serial=200;const channels=[],queue=[];
  const factory=name=>{
    const channel={name,closed:false,postMessage(data){for(const peer of channels)if(peer!==channel&&!peer.closed&&peer.name===name)queue.push([peer,structuredClone(data)])},close(){this.closed=true}};
    channels.push(channel);return channel;
  };
  const drain=()=>{let count=0;while(queue.length){assert.ok(++count<300,'Message loop');const [peer,data]=queue.shift();if(!peer.closed)peer.onmessage?.({data})}};
  const storage=()=>{const map=new Map();return {getItem:key=>map.get(key)??null,setItem:(key,value)=>map.set(key,value),removeItem:key=>map.delete(key)}};
  const clients=[];
  const make=(store=storage(),clock={time:0})=>{
    const c=new PartyConnection({newId:()=>id(serial++),storage:store,channelFactory:factory,now:()=>clock.time});clients.push(c);return c;
  };
  t.after(()=>clients.forEach(c=>c.dispose()));
  const dm=make();dm.create(hero);const p=make();p.join(dm.sessionId,{...hero,name:'Mira'});drain();
  return {dm,p,drain,queue,make,storage};
}
test('trasporto collega tre client, recupera una conferma persa e non applica due volte i PF',t=>{
  const h=harness(t),third=h.make();third.join(h.dm.sessionId,hero);h.drain();
  assert.deepEqual(h.dm.session,h.p.session);assert.deepEqual(h.p.session,third.session);
  h.p.send('initiative',{initiative:16});const pending=structuredClone(h.p.pending);
  const delivery=h.queue.shift();delivery[0].onmessage({data:delivery[1]});h.queue.length=0; // Drop every acknowledgement.
  assert.ok(h.p.pending);h.p.tick();h.drain();assert.equal(h.p.pending,null);assert.equal(h.dm.session.revision,4);
  h.dm.send('hp',{actorId:actorId(h.p.clientId),delta:-3});h.drain();const hp=h.p.session.encounter.participants[1].hp;
  h.dm.receive({protocol:1,sessionId:h.dm.sessionId,type:'command',command:pending});h.drain();
  assert.equal(h.p.session.encounter.participants[1].hp,hp);assert.equal(h.dm.session.revision,5);
});
test('client sospeso recupera lo stato corrente e mantiene una sola identità dopo reload',t=>{
  const h=harness(t),clientId=h.p.clientId;h.p.pause();h.dm.send('hp',{actorId:actorId(clientId),delta:-4});h.drain();
  assert.throws(()=>h.p.send('initiative',{initiative:20}));h.p.resume();h.drain();assert.deepEqual(h.p.session,h.dm.session);
  const storage=h.p.storage;h.p.dispose();const reloaded=h.make(storage);reloaded.restore();h.drain();
  assert.equal(reloaded.clientId,clientId);assert.equal(h.dm.session.members.length,2);assert.deepEqual(reloaded.session,h.dm.session);
});
test('reload coordinatore riprende checkpoint e deduplicazione; assenza blocca i comandi',t=>{
  const h=harness(t);h.dm.send('start');h.drain();const storage=h.dm.storage,session=structuredClone(h.dm.session),dmId=h.dm.clientId;
  h.dm.dispose();h.p.lastSeen=0;h.p.now=()=>10000;h.p.tick();assert.equal(h.p.status,'offline');assert.throws(()=>h.p.send('move',{cell:1}));
  const recovered=h.make(storage);recovered.restore();h.drain();assert.equal(recovered.clientId,dmId);assert.deepEqual(recovered.session,session);assert.equal(h.p.status,'connected');
  const retry={protocol:1,sessionId:session.id,clientId:dmId,seq:session.receipts[dmId].seq,baseRevision:session.revision-1,action:'start',payload:{}};
  recovered.process(retry);h.drain();assert.deepEqual(recovered.session,session);
});
test('ripresa ritrasmette un comando pendente una sola volta e rifiuta snapshot falsi',t=>{
  const h=harness(t);h.p.send('initiative',{initiative:19});h.queue.length=0;
  const storage=h.p.storage;h.p.dispose();const restored=h.make(storage);restored.restore();h.drain();assert.ok(restored.pending);restored.tick();h.drain();
  assert.equal(restored.pending,null);assert.equal(h.dm.session.encounter.participants[1].initiative,19);assert.equal(h.dm.session.revision,3);
  restored.receive({protocol:1,sessionId:h.dm.sessionId,type:'snapshot',from:id(999),session:{...h.dm.session,coordinatorId:id(999)}});assert.deepEqual(restored.session,h.dm.session);
});
test('memoria negata e checkpoint corrotti conservano i salvataggi e mostrano sessione temporanea',t=>{
  const h=harness(t),blocked={getItem:()=>null,setItem:()=>{throw new Error('Quota')},removeItem:()=>{throw new Error('Denied')}};
  const temporary=h.make(blocked);temporary.create(hero);assert.equal(temporary.connected,true);assert.equal(temporary.checkpointSaved,false);temporary.leave();
  const storage=h.storage();storage.setItem(PARTY_CHECKPOINT,'{broken');const corrupt=h.make(storage);corrupt.restore();assert.equal(corrupt.session,null);assert.match(corrupt.message,/non disponibile/);assert.equal(storage.getItem(PARTY_CHECKPOINT),'{broken');
});
