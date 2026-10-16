import {emptyEncounter,validateEncounter,validateParticipant,upsertParticipant,startEncounter,nextTurn,stopEncounter,adjustParticipantHp} from './encounter.js';
import {placeToken,toggleWall,moveActive} from './grid.js';

export const PARTY_PROTOCOL=1;
const id=value=>typeof value==='string'&&/^[a-f0-9]{24}$/.test(value);
const integer=(n,min=0,max=Number.MAX_SAFE_INTEGER)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
const fail=message=>{throw new Error(message)};
export const actorId=clientId=>`p_${clientId}`;
export function createParty(sessionId,coordinatorId){
  if(!id(sessionId)||!id(coordinatorId))fail('Identità del party non valida');
  return {protocol:PARTY_PROTOCOL,id:sessionId,coordinatorId,revision:0,encounter:emptyEncounter(),members:[],receipts:{}};
}
// Snapshots and checkpoints use the same bounded, canonical representation.
export function validateParty(input){
  if(!input||input.protocol!==PARTY_PROTOCOL||!id(input.id)||!id(input.coordinatorId)||!integer(input.revision))fail('Sessione party non valida');
  const encounter=validateEncounter(input.encounter,true);
  if(encounter.participants.some(p=>p.id==='hero')||encounter.strike!==null)fail('Partecipanti party non validi');
  if(!Array.isArray(input.members)||input.members.length>12)fail('Membri non validi');
  const members=input.members.map(m=>{
    if(!m||!id(m.clientId)||m.actorId!==actorId(m.clientId)||!encounter.participants.some(p=>p.id===m.actorId&&p.side==='ally'))fail('Membro non valido');
    return {clientId:m.clientId,actorId:m.actorId};
  });
  if(new Set(members.map(m=>m.clientId)).size!==members.length)fail('Membri duplicati');
  const allowed=new Set([input.coordinatorId,...members.map(m=>m.clientId)]);
  if(!input.receipts||typeof input.receipts!=='object'||Array.isArray(input.receipts))fail('Conferme non valide');
  const receipts=Object.fromEntries(Object.entries(input.receipts).map(([client,r])=>{
    if(!allowed.has(client)||!r||!integer(r.seq,1)||!(r.error===null||typeof r.error==='string'&&r.error.length<=200))fail('Conferma non valida');
    return [client,{seq:r.seq,error:r.error}];
  }));
  if(members.some(m=>!receipts[m.clientId]))fail('Conferma di ingresso mancante');
  return {protocol:PARTY_PROTOCOL,id:input.id,coordinatorId:input.coordinatorId,revision:input.revision,encounter,members,receipts};
}
export function heroProfile(hero,initiative=0){
  return {name:hero.name,hp:hero.hp,maxHp:hero.maxHp,armorClass:hero.armorClass,initiative,speed:6};
}
export function applyPartyCommand(session,command){
  // The caller is the coordinator. Invalid envelopes never consume a sequence.
  const c=command;
  if(!c||c.protocol!==PARTY_PROTOCOL||c.sessionId!==session.id||!id(c.clientId)||!integer(c.seq,1)||!integer(c.baseRevision)||typeof c.action!=='string')fail('Comando party non valido');
  const previous=session.receipts[c.clientId];
  if(c.seq<=(previous?.seq??0))return {session,duplicate:true,error:c.seq===previous.seq?previous.error:'Comando già elaborato'};
  if(c.seq!==(previous?.seq??0)+1)fail('Sequenza incompleta: riconnetti il client');
  const coordinator=c.clientId===session.coordinatorId;
  const member=session.members.find(m=>m.clientId===c.clientId);
  if(!coordinator&&!member&&c.action!=='join')fail('Entra nel party prima di inviare comandi');
  let encounter=session.encounter,members=session.members,error=null;
  const p=c.payload??{};
  try{
    if(session.revision===Number.MAX_SAFE_INTEGER)fail('Limite della sessione raggiunto');
    if(c.action!=='join'&&c.baseRevision!==session.revision)fail('Il tavolo è cambiato: stato aggiornato, ripeti la scelta');
    if(c.action==='join'){
      if(member)fail('Client già nel party');
      encounter=upsertParticipant({encounter},{...p,id:actorId(c.clientId),side:'ally'}).encounter;
      members=[...members,{clientId:c.clientId,actorId:actorId(c.clientId)}];
    }else if(c.action==='initiative'){
      const actor=encounter.participants.find(a=>a.id===member?.actorId);
      if(!actor)fail('Personaggio non presente');
      encounter=upsertParticipant({encounter},{...actor,initiative:p.initiative}).encounter;
    }else if(c.action==='move'){
      if(!coordinator&&encounter.activeId!==member?.actorId)fail('Puoi muovere solo il tuo personaggio durante il suo turno');
      encounter=moveActive({encounter},p.cell).encounter;
    }else{
      if(!coordinator)fail('Questa azione spetta al coordinatore');
      if(c.action==='start')encounter=startEncounter({encounter}).encounter;
      else if(c.action==='next')encounter=nextTurn({encounter}).encounter;
      else if(c.action==='stop')encounter=stopEncounter({encounter}).encounter;
      else if(c.action==='hp')encounter=adjustParticipantHp({encounter},p.actorId,p.delta).encounter;
      else if(c.action==='place')encounter=placeToken({encounter},p.actorId,p.cell).encounter;
      else if(c.action==='wall')encounter=toggleWall({encounter},p.cell).encounter;
      else if(c.action==='npc'){
        if(typeof p.id!=='string'||!/^n_[a-f0-9]{24}$/.test(p.id)||encounter.participants.some(a=>a.id===p.id))fail('Avversario non valido');
        const actor=validateParticipant({...p,side:'enemy'});
        encounter=upsertParticipant({encounter},actor).encounter;
      }else fail('Azione non riconosciuta');
    }
  }catch(e){error=e.message}
  // A rejected first join must not reserve an unbounded receipt entry.
  if(error&&!coordinator&&!member)return {session,duplicate:false,error};
  const next={...session,encounter,members,revision:session.revision+(error?0:1),receipts:{...session.receipts,[c.clientId]:{seq:c.seq,error}}};
  return {session:next,duplicate:false,error};
}

export function acceptPartySnapshot(current,incoming,sessionId,coordinatorId=null){
  const snapshot=validateParty(incoming);
  if(snapshot.id!==sessionId||(coordinatorId&&snapshot.coordinatorId!==coordinatorId))fail('Snapshot di un altro party');
  if(current&&snapshot.revision<current.revision)return current;
  if(current&&Object.entries(current.receipts).some(([client,r])=>(snapshot.receipts[client]?.seq??0)<r.seq))return current;
  return snapshot;
}
