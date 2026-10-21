import {legacyBackup} from './test-fixtures.js';
import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState, serialize, recordRoll} from './model.js';
import {upsertPower,usePower} from './powers.js';
import {upsertParticipant,startEncounter,queueDamage,applyPendingDamage} from './encounter.js';
import {placeToken,moveActive} from './grid.js';
import {LIBRARY_KEY,LEGACY_KEY,MAX_CHARACTERS,MAX_LIBRARY_BYTES,createLibrary,activeCharacter,updateCharacter,addCharacter,selectCharacter,removeCharacter,parseLibrary,serializeLibrary,loadLibrary,storeLibrary} from './library.js';

function session(){
  let s={...initialState(),name:'Mira <&>',condition:true};s.inventory[1].quantity=0;
  s=upsertPower(s,{id:'light',type:'spell',name:'Luce',description:'Nota',level:0,concentration:true});
  s=usePower(s,'light');
  s=upsertParticipant(s,{id:'hero',initiative:12});
  s=upsertParticipant(s,{id:'enemy',name:'Nemico',initiative:1,hp:20,maxHp:20,armorClass:12,side:'enemy'});
  s=placeToken(placeToken(s,'hero',40),'enemy',80);
  s=moveActive(startEncounter(s),41);
  s=applyPendingDamage(queueDamage(s,'enemy',4,'hit'));
  return recordRoll(s,'Prova',{dice:[10],bonus:2,total:12},'d20','normal',1000);
}
function memory(entries=[]){
  const data=new Map(entries);let writes=0;
  return {data,get writes(){return writes},getItem:key=>data.get(key)??null,setItem(key,value){writes++;data.set(key,value)}};
}

test('passaggio tra personaggi conserva l’intera sessione senza contaminare gli altri',()=>{
  const original=session();let library=createLibrary(original,'mira');
  library=addCharacter(library,{...initialState(),name:'Nora'},'nora');
  const nora=activeCharacter(library);nora.hp=2;nora.inventory[1].quantity=1;
  library=updateCharacter(library,nora);
  assert.deepEqual(activeCharacter(selectCharacter(library,'mira')),original);
  assert.equal(activeCharacter(selectCharacter(library,'nora')).hp,2);
  assert.equal(original.encounter.grid.positions.hero,41);
  assert.equal(original.encounter.strike.applied,true);
  assert.equal(original.concentration,'light');
});

test('copie e selezioni sono indipendenti anche per risorse e mappe annidate',()=>{
  const original=session();const library=createLibrary(original,'mira');
  original.encounter.grid.positions.hero=0;
  const copy=activeCharacter(library);copy.powers[0].name='Mutato';copy.encounter.grid.positions.hero=1;
  const duplicated=addCharacter(library,activeCharacter(library),'copy');
  const changed=activeCharacter(duplicated);changed.slots[0].max=2;changed.history.length=0;
  const updated=updateCharacter(duplicated,changed);
  assert.equal(activeCharacter(library).powers[0].name,'Luce');
  assert.equal(activeCharacter(library).encounter.grid.positions.hero,41);
  assert.deepEqual(activeCharacter(selectCharacter(updated,'mira')),activeCharacter(library));
});

test('backup della raccolta ripristina tutti i personaggi e quello attivo con schede v9 e legge quelle precedenti',()=>{
  const library=addCharacter(createLibrary(session(),'mira'),initialState(),'elian');
  const raw=serializeLibrary(library);
  assert.deepEqual(parseLibrary(raw),library);
  assert.equal(JSON.parse(raw).characters[0].state.version,11);
  const legacy=JSON.parse(raw);legacy.characters[1].state=legacyBackup(legacy.characters[1].state,7);
  assert.equal(activeCharacter(parseLibrary(JSON.stringify(legacy))).inventory[0].reach,1);
});

test('migrazione legge la vecchia chiave senza modificarla e dà priorità alla raccolta nuova',()=>{
  const old=JSON.stringify(legacyBackup(session(),8));const storage=memory([[LEGACY_KEY,old]]);
  const loaded=loadLibrary(storage);assert.equal(storage.writes,0);assert.equal(loaded.raw,null);
  assert.deepEqual(activeCharacter(loaded.library),session());
  const raw=storeLibrary(storage,loaded.library,loaded.raw);
  assert.equal(storage.data.get(LEGACY_KEY),old);
  storage.data.set(LEGACY_KEY,'{corrotto');
  assert.equal(loadLibrary(storage).raw,raw);
});

test('salvataggi corrotti o non leggibili non provocano scritture o fallback distruttivi',()=>{
  for(const entries of [[[LIBRARY_KEY,'{'],[LEGACY_KEY,serialize(initialState())]],[[LEGACY_KEY,'{']]]){
    const storage=memory(entries);assert.throws(()=>loadLibrary(storage));assert.equal(storage.writes,0);assert.deepEqual([...storage.data],entries);
  }
  assert.throws(()=>loadLibrary({getItem(){throw new Error('Negato')}}));
});

test('una scheda browser non aggiornata non sovrascrive i progressi salvati da un’altra',()=>{
  const storage=memory();const a=loadLibrary(storage),b=loadLibrary(storage);
  const raw=storeLibrary(storage,a.library,a.raw);
  assert.throws(()=>storeLibrary(storage,addCharacter(b.library,initialState(),'other'),b.raw),/altra/);
  assert.equal(storage.data.get(LIBRARY_KEY),raw);assert.equal(storage.writes,1);
});

test('quota esaurita lascia inalterato il salvataggio persistente e permette un backup in memoria',()=>{
  const previous=serializeLibrary(createLibrary());const storage=memory([[LIBRARY_KEY,previous]]);
  const loaded=loadLibrary(storage),next=addCharacter(loaded.library,session(),'mira');
  storage.setItem=()=>{throw new Error('QuotaExceededError')};
  assert.throws(()=>storeLibrary(storage,next,loaded.raw));
  assert.equal(storage.data.get(LIBRARY_KEY),previous);
  assert.deepEqual(parseLibrary(serializeLibrary(next)),next);
});

test('una raccolta piena con note e nomi Unicode resta esportabile e ripristinabile',()=>{
  const hero=initialState();hero.name='界'.repeat(24);hero.title='界'.repeat(40);
  hero.powers=Array.from({length:40},(_,i)=>({id:`power-${i}`.padEnd(64,'x'),type:'spell',name:'界'.repeat(40),description:'界'.repeat(600),level:1,concentration:false}));
  for(let i=0;i<20;i++)hero.history.push({label:'界'.repeat(80),dice:[20],bonus:30,total:50,kind:'d20',mode:'normal',timestamp:i});
  for(let i=0;i<12;i++)hero.encounter.participants.push({id:`enemy-${i}`.padEnd(64,'x'),name:'界'.repeat(40),hp:9999,maxHp:9999,armorClass:30,initiative:50,side:'enemy',speed:18});
  hero.inventory=Array.from({length:60},(_,i)=>({id:`item-${i}`.padEnd(64,'x'),type:'gear',name:'界'.repeat(40),notes:'界'.repeat(400),quantity:999}));hero.equippedWeaponId=null;
  let library=createLibrary(hero);for(let i=1;i<MAX_CHARACTERS;i++)library=addCharacter(library,hero,`hero-${i}`);
  const raw=serializeLibrary(library);
  assert.ok(new TextEncoder().encode(raw).length<MAX_LIBRARY_BYTES);
  assert.deepEqual(parseLibrary(raw),library);
});

test('rimozione del personaggio attivo sceglie uno dei rimanenti e protegge l’ultimo',()=>{
  let library=addCharacter(createLibrary(session(),'mira'),initialState(),'elian');
  assert.equal(removeCharacter(library,'mira').activeId,'elian');
  library=removeCharacter(library,'elian');assert.equal(library.activeId,'mira');
  assert.throws(()=>removeCharacter(library,'mira'));
  assert.throws(()=>removeCharacter(library,'missing'));
  assert.throws(()=>selectCharacter(library,'missing'));
});

test('limite della raccolta, identità duplicate e schede non valide sono rifiutati senza mutazione',()=>{
  let library=createLibrary();for(let i=1;i<MAX_CHARACTERS;i++)library=addCharacter(library,initialState(),`hero-${i}`);
  const before=serializeLibrary(library);
  assert.throws(()=>addCharacter(library,initialState(),'overflow'));
  assert.equal(serializeLibrary(library),before);
  for(const id of ['first-hero','bad"id','__invalid space'])assert.throws(()=>addCharacter(createLibrary(),initialState(),id));
  assert.throws(()=>updateCharacter(library,{...initialState(),hp:-1}));
});

test('importazioni invalide sono atomiche: versioni, duplicati, attivo, dimensioni e dati annidati',()=>{
  const raw=serializeLibrary(createLibrary());const base=JSON.parse(raw);
  for(const patch of [{type:'other'},{version:2},{characters:[]},{characters:[...base.characters,...base.characters]},{activeId:'missing'},{characters:[{...base.characters[0],state:{...base.characters[0].state,hp:9999}}]}]){
    assert.throws(()=>parseLibrary(JSON.stringify({...base,...patch})));
  }
  assert.throws(()=>parseLibrary(' '.repeat(MAX_LIBRARY_BYTES+1)));
  assert.equal(serializeLibrary(createLibrary()),raw);
});
