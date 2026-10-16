import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,changeHp,rollD20,restore,attack,damage,drinkPotion,serialize,parseBackup,modifier,proficiency,attackBonus,describeRoll,recordRoll} from './model.js';
const sequence=(...values)=>()=>{assert.ok(values.length,'Sorgente casuale esaurita');return values.shift()};
test('PF entro i limiti',()=>{assert.equal(changeHp(initialState(),-99).hp,0);assert.equal(changeHp(initialState(),99).hp,32)});
test('vantaggio e svantaggio selezionano il dado e applicano bonus',()=>{assert.deepEqual(rollD20(3,'advantage',sequence(1,18)),{dice:[2,19],natural:19,bonus:3,total:22});assert.equal(rollD20(-1,'disadvantage',sequence(1,18)).total,1)});
test('rifiuta valori che renderebbero il d20 sbilanciato',()=>{assert.equal(rollD20(0,'normal',sequence(4294967295,19)).natural,20)});
test('backup completo, migrazione v1 e recupero da corruzione',()=>{const state={...initialState(),hp:7,potions:0,resolve:0};assert.deepEqual(parseBackup(serialize(state)),state);assert.equal(restore('{').hp,24);assert.equal(parseBackup(JSON.stringify({version:1,hp:9,condition:true,bonus:2,mode:'normal'})).potions,2);assert.throws(()=>parseBackup(JSON.stringify({...state,version:2,potions:-1})));assert.throws(()=>parseBackup(JSON.stringify({...state,version:99})));assert.throws(()=>parseBackup(JSON.stringify({...state,version:2,hp:100})));});
test('attacco: soglia CA, 1 manca sempre, 20 colpisce sempre',()=>{const s=initialState();assert.equal(attack(s,14,sequence(9)).hits,true);assert.equal(attack(s,14,sequence(8)).hits,false);assert.equal(attack(s,1,sequence(0)).hits,false);assert.equal(attack(s,30,sequence(19)).critical,true);assert.throws(()=>attack({...s,swordEquipped:false},14));});
test('avvelenato impone svantaggio e annulla il vantaggio',()=>{const s={...initialState(),condition:true};assert.equal(attack(s,14,sequence(18,1)).natural,2);assert.equal(attack({...s,mode:'advantage'},14,sequence(10)).mode,'normal');});
test('critico raddoppia i dadi, non il modificatore',()=>{assert.deepEqual(damage(true,sequence(7,0)),{dice:[8,1],bonus:2,total:11});assert.equal(damage(false,sequence(7)).total,10)});
test('pozione: cura limitata, consumo singolo e nessun consumo a PF pieni',()=>{const s={...initialState(),hp:31};const next=drinkPotion(s,sequence(3,3));assert.equal(next.healing,10);assert.equal(next.state.hp,32);assert.equal(next.state.potions,1);assert.equal(s.potions,2);assert.equal(drinkPotion(next.state).healing,null);assert.equal(drinkPotion({...s,potions:0}).healing,null)});

test('profilo v3 conserva aspetto e PF massimi personalizzati',()=>{
  const state={...initialState(),name:'Mira <&>',title:'Viandante',level:9,maxHp:75,hp:60,armorClass:19,avatar:{skin:2,hair:1,cloak:2,style:1},abilities:{FOR:18,DES:12,COS:16,INT:10,SAG:14,CAR:8}};
  assert.deepEqual(parseBackup(serialize(state)),state);
  assert.equal(attackBonus(state),8);
  assert.equal(attack(state,20,sequence(11)).total,20);
  assert.equal(proficiency(20),6);assert.equal(modifier(9),-1);
  for(const patch of [{avatar:{skin:3,hair:0,cloak:0,style:0}},{name:'   '},{abilities:{...state.abilities,FOR:31}},{maxHp:50},{level:21}])assert.throws(()=>parseBackup(serialize({...state,...patch})));
});
test('migrazione v2 assegna un profilo senza perdere consumi',()=>{
  const state={...initialState(),potions:0,torches:1,resolve:0};
  delete state.name;delete state.avatar;delete state.abilities;
  const restored=parseBackup(JSON.stringify({...state,version:2}));
  assert.equal(restored.name,'Elian');assert.equal(restored.potions,0);assert.equal(restored.torches,1);
});
test('danni con Forza negativa non scendono sotto zero',()=>assert.equal(damage(false,sequence(0),-5).total,0));

test('abilità senza competenza, competenza e maestria usano il livello',()=>{
  const state=initialState();state.level=9;state.skillRanks.stealth=2;state.skillRanks.athletics=1;
  assert.equal(describeRoll(state,{kind:'skill',key:'stealth'}).bonus,11);
  assert.equal(describeRoll(state,{kind:'skill',key:'athletics'}).bonus,6);
  assert.equal(describeRoll(state,{kind:'skill',key:'arcana'}).bonus,0);
  assert.throws(()=>describeRoll(state,{kind:'skill',key:'unknown'}));
});
test('avvelenato influenza prove e abilità ma non salvezza o dado libero',()=>{
  const state={...initialState(),condition:true,mode:'advantage'};
  state.saveProficiencies.COS=true;
  assert.equal(describeRoll(state,{kind:'save',key:'COS'}).bonus,4);
  assert.equal(describeRoll(state,{kind:'save',key:'COS'}).mode,'advantage');
  assert.equal(describeRoll(state,{kind:'check',key:'FOR'}).mode,'normal');
  assert.equal(describeRoll(state,{kind:'skill',key:'stealth'}).mode,'normal');
  assert.equal(describeRoll({...state,mode:'normal'},{kind:'skill',key:'stealth'}).mode,'disadvantage');
  assert.equal(describeRoll(state).mode,'advantage');
});
test('storico limitato a 20 tiri, ordinato e indipendente dagli array di input',()=>{
  let state=initialState();const outcome={dice:[15,2],bonus:3,total:18};
  for(let i=0;i<25;i++)state=recordRoll(state,'Prova',outcome,'d20','advantage',i);
  assert.equal(state.history.length,20);assert.equal(state.history[0].timestamp,24);assert.equal(state.history.at(-1).timestamp,5);
  outcome.dice[0]=1;assert.equal(state.history[0].dice[0],15);
  assert.deepEqual(parseBackup(serialize(state)),state);
});
test('v4 rifiuta competenze e storico incoerenti',()=>{
  const state=initialState();const valid=recordRoll(state,'Danni',{dice:[1,2],bonus:2,total:5},'damage');
  assert.equal(parseBackup(serialize(valid)).history.length,1);
  assert.throws(()=>parseBackup(serialize({...state,skillRanks:{...state.skillRanks,stealth:3}})));
  assert.throws(()=>parseBackup(serialize({...state,saveProficiencies:{...state.saveProficiencies,COS:1}})));
  assert.throws(()=>parseBackup(serialize({...valid,history:[{...valid.history[0],total:999}]})));
  assert.throws(()=>recordRoll(state,'Prova',{dice:[20,20],bonus:0,total:20}));
});
test('migrazione v3 conserva il profilo e inizializza competenze e storico',()=>{
  const state={...initialState(),name:'Nora'};
  delete state.skillRanks;delete state.saveProficiencies;delete state.history;
  const migrated=parseBackup(JSON.stringify({...state,version:3}));
  assert.equal(migrated.name,'Nora');assert.equal(migrated.skillRanks.stealth,0);assert.equal(migrated.saveProficiencies.COS,false);assert.deepEqual(migrated.history,[]);
});
