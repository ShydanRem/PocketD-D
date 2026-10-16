import {legacyBackup} from './test-fixtures.js';
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,parseBackup,serialize} from './model.js';
import {participantView,turnOrder,upsertParticipant,removeParticipant,startEncounter,nextTurn,stopEncounter,adjustParticipantHp,queueDamage,applyPendingDamage,validateEncounter} from './encounter.js';
const enemy=(id='enemy',initiative=10)=>({id,name:'Guardiano originale',initiative,hp:15,maxHp:15,armorClass:12,side:'enemy'});
function roster(){let s=upsertParticipant(initialState(),{id:'hero',initiative:12});return upsertParticipant(s,enemy())}
test('iniziativa, parità stabili e rotazione dei round includono partecipanti a zero PF',()=>{
 let s=upsertParticipant(roster(),{...enemy('second',12),hp:0});assert.deepEqual(turnOrder(s).map(p=>p.id),['hero','second','enemy']);
 s=startEncounter(s);assert.equal(s.encounter.activeId,'hero');s=nextTurn(s);assert.equal(s.encounter.activeId,'second');s=nextTurn(s);s=nextTurn(s);assert.equal(s.encounter.round,2);assert.equal(s.encounter.activeId,'hero');
});
test('blocco roster in combattimento e arresto conservano PF',()=>{
 const s=startEncounter(roster());assert.throws(()=>upsertParticipant(s,enemy('other')));assert.throws(()=>removeParticipant(s,'enemy'));assert.throws(()=>startEncounter(s));
 const stopped=stopEncounter(adjustParticipantHp(s,'enemy',-4));assert.equal(stopped.encounter.round,0);assert.equal(participantView(stopped,'enemy').hp,11);assert.equal(removeParticipant(stopped,'enemy').encounter.participants.length,1);
 assert.throws(()=>startEncounter(initialState()));assert.throws(()=>nextTurn(stopped));
});
test('eroe collegato alla scheda, limiti PF e concentrazione',()=>{
 const s={...roster(),name:'Mira',armorClass:19,hp:7,concentration:'spell'};
 assert.equal(participantView(s,'hero').name,'Mira');assert.equal(participantView(s,'hero').armorClass,19);
 const hurt=adjustParticipantHp(s,'hero',-100);assert.equal(hurt.hp,0);assert.equal(hurt.concentration,null);assert.equal(s.hp,7);
 assert.equal(participantView(adjustParticipantHp(s,'hero',100),'hero').hp,32);assert.equal(participantView(adjustParticipantHp(s,'enemy',100),'enemy').hp,15);
});
test('danni confermati una volta sola, senza modificare il bersaglio in attesa',()=>{
 let s=queueDamage(roster(),'enemy',8,'hit-1');assert.equal(participantView(s,'enemy').hp,15);
 const applied=applyPendingDamage(s);assert.equal(participantView(applied,'enemy').hp,7);assert.equal(participantView(s,'enemy').hp,15);
 assert.throws(()=>applyPendingDamage(applied));assert.throws(()=>queueDamage(applied,'enemy',8,'hit-1'));
 assert.equal(participantView(applyPendingDamage(queueDamage(applied,'enemy',999,'hit-2')),'enemy').hp,0);
});
test('danni in attesa annullati da cambio turno e rimozione, bersagli invalidi rifiutati',()=>{
 const s=queueDamage(startEncounter(roster()),'enemy',8,'hit');assert.equal(nextTurn(s).encounter.strike,null);
 assert.equal(removeParticipant(stopEncounter(s),'enemy').encounter.strike,null);assert.throws(()=>queueDamage(s,'missing',2,'other'));assert.throws(()=>queueDamage(s,'enemy',-1,'other'));
});
test('backup v6 ripristina turno e danni già applicati; v5 migra senza combattimento',()=>{
 const s=applyPendingDamage(queueDamage(startEncounter(roster()),'enemy',4,'hit'));
 const restored=parseBackup(serialize(s));assert.deepEqual(restored,s);assert.throws(()=>applyPendingDamage(restored));
 const legacy=legacyBackup(s,5);delete legacy.encounter;assert.deepEqual(parseBackup(JSON.stringify(legacy)).encounter.participants,[]);
});
test('validazione respinge turni incoerenti, duplicati e PF fuori limite',()=>{
 const e=roster().encounter;
 for(const patch of [{activeId:'missing',round:1},{activeId:'hero',round:0},{participants:[enemy(),enemy()]},{participants:[{...enemy(),hp:20}]},{strike:{id:'hit',targetId:'missing',amount:1,applied:false}}])assert.throws(()=>validateEncounter({...e,...patch}));
 let s=initialState();for(let i=0;i<12;i++)s=upsertParticipant(s,enemy(`p${i}`));assert.throws(()=>upsertParticipant(s,enemy('overflow')));
});
