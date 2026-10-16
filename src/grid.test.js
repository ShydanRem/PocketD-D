import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,parseBackup,serialize} from './model.js';
import {upsertParticipant,startEncounter,nextTurn,stopEncounter,removeParticipant} from './encounter.js';
import {placeToken,unplaceToken,toggleWall,reachableCells,moveActive,validateGrid,viewportCells,centerViewport,panViewport,sightCells,targetGeometry} from './grid.js';
function setup(speed=6){let s=upsertParticipant(initialState(),{id:'hero',initiative:12,speed});s=upsertParticipant(s,{id:'enemy',name:'Guardiano',initiative:10,hp:10,maxHp:10,armorClass:12,side:'enemy',speed:4});return placeToken(placeToken(s,'hero',40),'enemy',80)}
test('spostamenti e diagonali costano una casella, senza avvolgere i bordi',()=>{
 let s=startEncounter(setup(2));assert.equal(reachableCells(s).get(60),2);s=moveActive(s,60);assert.equal(s.encounter.grid.spent,2);assert.equal(s.encounter.grid.positions.hero,60);assert.throws(()=>moveActive(s,61));
 s=stopEncounter(setup(1));s=placeToken(s,'hero',8);s=startEncounter(s);assert.equal(reachableCells(s).has(9),false);
});
test('muri causano deviazioni e non consentono tagli diagonali',()=>{
 let s=toggleWall(setup(),41);s=startEncounter(s);assert.equal(reachableCells(s).get(42),4);assert.equal(reachableCells(s).get(50),2);assert.equal(reachableCells(s).has(41),false);
 s=setup();for(let y=0;y<9;y++)s=toggleWall(s,y*9+5);s=startEncounter(s);assert.equal(reachableCells(s).has(42),false);
});
test('pedine bloccano caselle e angoli; piazzamento non sovrappone muri o altre pedine',()=>{
 let s=placeToken(setup(),'enemy',41);assert.throws(()=>placeToken(s,'hero',41));assert.throws(()=>toggleWall(s,41));
 const active=startEncounter(s);assert.equal(reachableCells(active).has(41),false);assert.equal(reachableCells(active).get(50),2);
 s=unplaceToken(s,'enemy');assert.equal(Object.hasOwn(s.encounter.grid.positions,'enemy'),false);
});
test('movimento accumulato, rinnovo a ogni turno e annullabilità senza mutazione',()=>{
 const s=startEncounter(setup(2));const first=moveActive(s,41);const second=moveActive(first,42);
 assert.equal(second.encounter.grid.spent,2);assert.equal(s.encounter.grid.positions.hero,40);assert.equal(first.encounter.grid.positions.hero,41);
 const enemyTurn=nextTurn(second);assert.equal(enemyTurn.encounter.grid.spent,0);const movedEnemy=moveActive(enemyTurn,79);assert.equal(movedEnemy.encounter.grid.spent,1);
 const nextRound=nextTurn(movedEnemy);assert.equal(nextRound.encounter.round,2);assert.equal(nextRound.encounter.grid.spent,0);
});
test('zero PF, pedina assente e velocità zero non consentono movimento',()=>{
 const s=startEncounter(setup());assert.equal(reachableCells({...s,hp:0}).size,0);assert.throws(()=>moveActive({...s,hp:0},41));
 assert.equal(reachableCells(startEncounter(unplaceToken(setup(),'hero'))).size,0);assert.throws(()=>moveActive(startEncounter(setup(0)),41));
});
test('editing vietato durante il combattimento, rimozione partecipante pulisce la mappa',()=>{
 const s=startEncounter(setup());assert.throws(()=>placeToken(s,'hero',0));assert.throws(()=>toggleWall(s,0));assert.throws(()=>unplaceToken(s,'hero'));
 const removed=removeParticipant(stopEncounter(s),'enemy');assert.equal(Object.hasOwn(removed.encounter.grid.positions,'enemy'),false);
});
test('backup v7 mantiene mappa e movimento; v6 migra senza griglia',()=>{
 const s=moveActive(startEncounter(toggleWall(setup(),0)),41);assert.deepEqual(parseBackup(serialize(s)),s);
 const legacy={...s,version:6,encounter:{...s.encounter}};delete legacy.encounter.grid;
 const restored=parseBackup(JSON.stringify(legacy));assert.deepEqual(restored.encounter.grid,{walls:[],positions:{},spent:0});assert.equal(restored.encounter.round,1);
 const invalid={...s,version:7,encounter:{...s.encounter}};delete invalid.encounter.grid;assert.throws(()=>parseBackup(JSON.stringify(invalid)));
});
test('validazione rifiuta celle fuori mappa, sovrapposizioni e movimento incoerente',()=>{
 const s=startEncounter(setup());const grid=s.encounter.grid;
 for(const patch of [{walls:[81]},{walls:[1,1]},{positions:{hero:40,enemy:40}},{walls:[40]},{positions:{missing:0}},{spent:7},{positions:{enemy:80},spent:1}])assert.throws(()=>validateGrid({...grid,...patch},s.encounter.participants,'hero',1));
 assert.throws(()=>validateGrid({...grid,spent:1},s.encounter.participants,null,0));
});

test('zoom e panoramica mantengono coordinate globali e bordi',()=>{
 assert.equal(viewportCells().length,81);assert.equal(viewportCells(5,{x:2,y:2})[0],20);assert.equal(viewportCells(5,{x:2,y:2}).at(-1),60);
 assert.deepEqual(centerViewport(40,5),{x:2,y:2});assert.deepEqual(centerViewport(80,5),{x:4,y:4});
 assert.deepEqual(panViewport({x:4,y:4},9,9,5),{x:4,y:4});assert.deepEqual(panViewport({x:0,y:0},-1,-1,5),{x:0,y:0});
 assert.deepEqual(viewportCells(5,{x:99,y:99}),viewportCells(5,{x:4,y:4}));assert.throws(()=>viewportCells(6));
});
test('linea geometrica include angoli e risulta simmetrica in tutte le direzioni',()=>{
 assert.deepEqual(sightCells(40,42),[40,41,42]);
 assert.deepEqual(new Set(sightCells(40,50)),new Set([40,41,49,50]));
 for(let start=0;start<81;start++)for(let end=0;end<81;end++){
  const cells=sightCells(start,end);assert.ok(cells.every(cell=>cell>=0&&cell<81));assert.ok(cells.includes(start)&&cells.includes(end));
  assert.deepEqual(new Set(cells),new Set(sightCells(end,start)));
 }
});
test('portata usa distanza in caselle e i muri bloccano la vista anche agli angoli',()=>{
 let s=placeToken(setup(),'enemy',42);assert.equal(targetGeometry(s,'enemy').distance,2);assert.equal(targetGeometry(s,'enemy').inRange,false);assert.equal(targetGeometry(s,'enemy',2).inRange,true);assert.equal(targetGeometry(s,'enemy').visible,true);
 s=toggleWall(s,41);assert.equal(targetGeometry(s,'enemy',9).visible,false);
 s=placeToken(s,'enemy',50);assert.equal(targetGeometry(s,'enemy').distance,1);assert.equal(targetGeometry(s,'enemy').visible,false);
 assert.equal(targetGeometry(unplaceToken(s,'hero'),'enemy').known,false);assert.throws(()=>targetGeometry(s,'missing'));
});
test('altre pedine non bloccano la vista geometrica; nessuna modifica alle distanze durante zoom',()=>{
 let s=placeToken(setup(),'enemy',42);s=upsertParticipant(s,{id:'ally',name:'Alleato',initiative:0,speed:6,hp:10,maxHp:10,armorClass:10,side:'ally'});s=placeToken(s,'ally',41);
 const before=targetGeometry(s,'enemy',2);assert.equal(before.visible,true);viewportCells(5,centerViewport(40,5));assert.deepEqual(targetGeometry(s,'enemy',2),before);
});
test('backup v8 conserva portata; v7 migra a una casella senza perdere la mappa',()=>{
 const s={...setup(),weaponReach:3};assert.deepEqual(parseBackup(serialize(s)),s);
 const old={...s,version:7};delete old.weaponReach;const migrated=parseBackup(JSON.stringify(old));assert.equal(migrated.weaponReach,1);assert.deepEqual(migrated.encounter,s.encounter);
 for(const reach of [0,10,1.5,'2'])assert.throws(()=>parseBackup(serialize({...s,weaponReach:reach})));
});
