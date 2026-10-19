import {adjustHeroHp} from './extended.js';
import {emptyGrid,validateGrid} from './grid.js';
const integer=(n,min,max)=>Number.isInteger(n)&&n>=min&&n<=max;
const validId=id=>typeof id==='string'&&/^[a-zA-Z0-9_-]{1,64}$/.test(id);
export const emptyEncounter=()=>({participants:[],activeId:null,round:0,strike:null,grid:emptyGrid()});
export function validateParticipant(input){
  if(!input||!validId(input.id)||!integer(input.initiative,-10,50))throw new Error('Partecipante non valido');
  const speed=input.speed??6;if(!integer(speed,0,18))throw new Error('Velocità non valida');
  if(input.id==='hero')return {id:'hero',initiative:input.initiative,speed};
  if(typeof input.name!=='string'||!input.name.trim()||input.name.length>40||!integer(input.maxHp,1,9999)||!integer(input.hp,0,input.maxHp)||!integer(input.armorClass,1,30)||!['ally','enemy'].includes(input.side))throw new Error('Dati del partecipante non validi');
  return {id:input.id,name:input.name.trim(),initiative:input.initiative,maxHp:input.maxHp,hp:input.hp,armorClass:input.armorClass,side:input.side,speed};
}
export function validateEncounter(input,requireGrid=false){
  if(!input||!Array.isArray(input.participants)||input.participants.length>12)throw new Error('Massimo 12 partecipanti');
  const participants=input.participants.map(validateParticipant);
  const ids=new Set(participants.map(item=>item.id));
  if(ids.size!==participants.length)throw new Error('Partecipanti duplicati');
  if(!integer(input.round,0,9999)||(input.round===0?input.activeId!==null:participants.length<2||!ids.has(input.activeId)))throw new Error('Turno non valido');
  let strike=null;
  if(input.strike!==null){
    const hit=input.strike;
    if(!hit||!validId(hit.id)||!ids.has(hit.targetId)||!integer(hit.amount,0,999)||typeof hit.applied!=='boolean')throw new Error('Danni in attesa non validi');
    strike={id:hit.id,targetId:hit.targetId,amount:hit.amount,applied:hit.applied};
  }
  if(requireGrid&&!input.grid)throw new Error('Mappa mancante');
  const grid=input.grid?validateGrid(input.grid,participants,input.activeId,input.round):emptyGrid();
  return {participants,activeId:input.activeId,round:input.round,strike,grid};
}
export function participantView(state,id){
  const entry=state.encounter.participants.find(item=>item.id===id);
  if(!entry)throw new Error('Partecipante non trovato');
  return id==='hero'?{...entry,name:state.name,hp:state.hp,maxHp:state.maxHp,armorClass:state.armorClass,side:'ally'}:entry;
}
export function turnOrder(state){
  return [...state.encounter.participants].sort((a,b)=>b.initiative-a.initiative).map(item=>participantView(state,item.id));
}
export function upsertParticipant(state,input){
  if(state.encounter.round>0)throw new Error('Termina il combattimento prima di modificare l’iniziativa');
  const participant=validateParticipant(input);
  const exists=state.encounter.participants.some(item=>item.id===participant.id);
  if(!exists&&state.encounter.participants.length>=12)throw new Error('Massimo 12 partecipanti');
  const participants=exists?state.encounter.participants.map(item=>item.id===participant.id?participant:item):[...state.encounter.participants,participant];
  return {...state,encounter:{...state.encounter,participants,strike:null}};
}
export function removeParticipant(state,id){
  if(state.encounter.round>0)throw new Error('Termina il combattimento prima di rimuovere partecipanti');
  return {...state,encounter:{...state.encounter,participants:state.encounter.participants.filter(item=>item.id!==id),strike:null,grid:{...state.encounter.grid,positions:Object.fromEntries(Object.entries(state.encounter.grid.positions).filter(([other])=>other!==id))}}};
}
export function startEncounter(state){
  if(state.encounter.round>0)throw new Error('Combattimento già iniziato');
  const order=turnOrder(state);
  if(order.length<2)throw new Error('Servono almeno due partecipanti');
  return {...state,encounter:{...state.encounter,activeId:order[0].id,round:1,strike:null,grid:{...state.encounter.grid,spent:0}}};
}
export function nextTurn(state){
  if(state.encounter.round===0)throw new Error('Combattimento non iniziato');
  const order=turnOrder(state);const index=order.findIndex(item=>item.id===state.encounter.activeId);
  if(index===order.length-1&&state.encounter.round===9999)throw new Error('Limite dei round raggiunto');
  return {...state,encounter:{...state.encounter,activeId:order[(index+1)%order.length].id,round:state.encounter.round+(index===order.length-1?1:0),strike:null,grid:{...state.encounter.grid,spent:0}}};
}
export function stopEncounter(state){return {...state,encounter:{...state.encounter,activeId:null,round:0,strike:null,grid:{...state.encounter.grid,spent:0}}}}
export function adjustParticipantHp(state,id,delta){
  if(!integer(delta,-9999,9999))throw new Error('Variazione PF non valida');
  const participant=participantView(state,id);
  const hp=Math.max(0,Math.min(participant.maxHp,participant.hp+delta));
  if(id==='hero')return adjustHeroHp(state,delta);
  return {...state,encounter:{...state.encounter,participants:state.encounter.participants.map(item=>item.id===id?{...item,hp}:item)}};
}
export function queueDamage(state,targetId,amount,id){
  participantView(state,targetId);
  if(!validId(id)||!integer(amount,0,999)||state.encounter.strike?.id===id)throw new Error('Danni non validi');
  return {...state,encounter:{...state.encounter,strike:{id,targetId,amount,applied:false}}};
}
export function applyPendingDamage(state){
  const strike=state.encounter.strike;
  if(!strike||strike.applied)throw new Error('Danni già applicati o assenti');
  const next=adjustParticipantHp(state,strike.targetId,-strike.amount);
  return {...next,encounter:{...next.encounter,strike:{...strike,applied:true}}};
}
