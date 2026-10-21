import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,parseBackup,serialize,attack,weaponDamage,weaponStats,useItem,recordRoll} from './model.js';
import {MAX_ITEMS,validateItem,validateInventory,upsertItem,removeItem,equipWeapon,adjustQuantity} from './inventory.js';
import {createLibrary,addCharacter,activeCharacter,selectCharacter,updateCharacter,serializeLibrary,parseLibrary} from './library.js';
import {rest} from './powers.js';
import {legacyBackup} from './test-fixtures.js';
const sequence=(...values)=>()=>{assert.ok(values.length,'Random exhausted');return values.shift()};
const weapon=(extra={})=>({id:'bow',type:'weapon',name:'Arco personale',notes:'Originale',quantity:1,ability:'DES',proficient:false,diceCount:2,dieSides:6,attackModifier:1,damageModifier:2,addAbilityToDamage:true,reach:6,damageType:'Perforante',...extra});
const equipped=(extra={})=>equipWeapon(upsertItem(initialState(),weapon(extra)),'bow');

test('v8 migra consumi, arma riposta, portata e combattimento senza ricaricare gli oggetti',()=>{
  const old={...legacyBackup(initialState(),8),potions:0,torches:1,swordEquipped:false,weaponReach:4,resolve:0};
  old.encounter.strike=null;
  const migrated=parseBackup(JSON.stringify(old));
  assert.equal(migrated.inventory[1].quantity,0);assert.equal(migrated.inventory[2].quantity,1);
  assert.equal(migrated.equippedWeaponId,null);assert.equal(migrated.inventory[0].reach,4);assert.equal(migrated.resolve,0);
  assert.deepEqual(migrated.encounter,old.encounter);
  assert.equal('potions' in migrated,false);assert.equal('weaponReach' in migrated,false);
  assert.deepEqual(parseBackup(serialize(migrated)),migrated);
});

test('v1–v7 ricevono gli oggetti originali e la portata predefinita',()=>{
  for(let version=1;version<=7;version++){
    const old=legacyBackup(initialState(),version);old.potions=0;old.torches=0;
    const state=parseBackup(JSON.stringify(old));
    assert.equal(state.inventory[0].reach,1);assert.equal(state.inventory[1].quantity,version===1?2:0);
  }
});

test('schema v9 e raccolte conservano armi configurate e inventari vuoti',()=>{
  const state=equipped({notes:'<test> Ω',ability:'CAR',proficient:true,addAbilityToDamage:false});
  assert.equal(JSON.parse(serialize(state)).version,11);assert.deepEqual(parseBackup(serialize(state)),state);
  const empty={...initialState(),inventory:[],equippedWeaponId:null};assert.deepEqual(parseBackup(serialize(empty)),empty);
  const library=addCharacter(createLibrary(state),empty,'empty');assert.deepEqual(parseLibrary(serializeLibrary(library)),library);
});

test('attacco usa caratteristica, competenza selezionata e bonus separati dai danni',()=>{
  const state=equipped();assert.equal(weaponStats(state).attackBonus,4);assert.equal(weaponStats(state).damageBonus,5);
  const combat=attack(state,14,sequence(9));assert.equal(combat.total,14);assert.equal(combat.hits,true);
  assert.equal(weaponStats(equipped({proficient:true})).attackBonus,6);
  assert.equal(weaponStats(equipped({addAbilityToDamage:false})).damageBonus,2);
  assert.equal(attack(state,1,sequence(0)).hits,false);assert.equal(attack(state,30,sequence(19)).hits,true);
});

test('critico raddoppia tutti i dadi dell’arma, lasciando singolo il bonus',()=>{
  const state=equipped(),critical=attack(state,30,sequence(19));
  const result=weaponDamage(critical,sequence(5,4,3,2));
  assert.deepEqual(result,{dice:[6,5,4,3],bonus:5,sides:6,total:23});
  const normal=weaponDamage(attack(state,1,sequence(9)),sequence(0,1));assert.equal(normal.total,8);assert.equal(normal.dice.length,2);
  assert.throws(()=>weaponDamage(attack(state,30,sequence(0))));
});

test('danni usano la configurazione dell’attacco e non una successiva arma o caratteristica',()=>{
  const state=equipped(),combat=attack(state,1,sequence(10));
  state.abilities.DES=30;state.inventory.find(item=>item.id==='bow').dieSides=12;
  const result=weaponDamage(combat,sequence(5,5));assert.equal(result.sides,6);assert.equal(result.bonus,5);
  assert.equal(weaponDamage(attack(equipped({ability:'CAR',damageModifier:-10}),1,sequence(10)),sequence(0,0)).total,0);
});

test('storico supporta dadi fino a d12 e critici fino a dodici dadi con totale validato',()=>{
  const state=equipped({diceCount:6,dieSides:12});const combat=attack(state,1,sequence(19));
  const result=weaponDamage(combat,sequence(...Array(12).fill(11)));
  const next=recordRoll(state,'Critico personalizzato',result,'damage');assert.deepEqual(parseBackup(serialize(next)),next);
  assert.equal(next.history[0].sides,12);assert.equal(next.history[0].dice.length,12);
  assert.throws(()=>recordRoll(state,'Danni',{dice:[7],bonus:0,total:7,sides:6},'damage'));
  assert.throws(()=>recordRoll(state,'Danni',{dice:Array(13).fill(1),bonus:0,total:13,sides:4},'damage'));
  const legacy=legacyBackup(initialState(),8);legacy.history=[{label:'Danni spada',dice:[8,1],bonus:2,total:11,kind:'damage',mode:'normal',timestamp:0}];
  assert.equal(parseBackup(JSON.stringify(legacy)).history[0].sides,8);
});

test('equipaggiamento, esaurimento, eliminazione e cambio tipo ripuliscono arma e danni in attesa',()=>{
  const base={...equipped(),encounter:{...initialState().encounter,strike:{id:'hit',targetId:'hero',amount:3,applied:false}}};
  assert.equal(equipWeapon(base,null).encounter.strike,null);
  assert.throws(()=>equipWeapon(base,'starter-potion'));
  for(const next of [adjustQuantity(base,'bow',-1),removeItem(base,'bow'),upsertItem(base,{id:'bow',type:'gear',name:'Ricordo',notes:'',quantity:1})]){
    assert.equal(next.equippedWeaponId,null);assert.equal(next.encounter.strike,null);assert.throws(()=>attack(next,12));
  }
  assert.equal(base.equippedWeaponId,'bow');assert.equal(base.inventory.find(item=>item.id==='bow').quantity,1);
});

test('consumo manuale e cura sono singoli, immutabili e rispettano i PF massimi',()=>{
  const state={...initialState(),hp:31};const used=useItem(state,'starter-potion',sequence(3,3));
  assert.equal(used.healing,10);assert.equal(used.gained,1);assert.equal(used.state.hp,32);assert.equal(used.state.inventory[1].quantity,1);
  assert.equal(state.inventory[1].quantity,2);assert.equal(useItem(used.state,'starter-potion').state,used.state);
  const torch=useItem(state,'starter-torch');assert.equal(torch.state.hp,31);assert.equal(torch.state.inventory[2].quantity,2);
  assert.throws(()=>useItem(adjustQuantity(state,'starter-torch',-3),'starter-torch'));
  assert.throws(()=>useItem(state,'starter-sword'));
  const custom=upsertItem(state,{id:'custom-heal',type:'healing',name:'Rimedio',notes:'',quantity:1,diceCount:1,dieSides:12,healingModifier:4});
  assert.equal(useItem({...custom,hp:0},'custom-heal',sequence(11)).state.hp,16);
});

test('riposi e altri personaggi non ricaricano consumi o cambiano l’arma equipaggiata',()=>{
  let state=equipped();state=useItem(state,'starter-torch').state;
  for(const kind of ['short','long'])assert.deepEqual(rest(state,kind).inventory,state.inventory);
  let library=addCharacter(createLibrary(state,'first'),initialState(),'other');
  library=updateCharacter(library,useItem(activeCharacter(library),'starter-torch').state);
  assert.deepEqual(activeCharacter(selectCharacter(library,'first')).inventory,state.inventory);
  assert.equal(activeCharacter(selectCharacter(library,'first')).equippedWeaponId,'bow');
});

test('validazione rifiuta identità duplicate, armi assenti e campi incoerenti prima della sostituzione',()=>{
  const state=equipped();
  for(const patch of [{ability:'BAD'},{diceCount:0},{diceCount:7},{dieSides:20},{attackModifier:11},{damageModifier:-11},{proficient:1},{reach:0},{quantity:-1},{name:' '},{notes:'x'.repeat(401)}])assert.throws(()=>validateItem(weapon(patch)));
  for(const patch of [{healingModifier:-1},{dieSides:20},{diceCount:7}])assert.throws(()=>validateItem({id:'heal',type:'healing',name:'Cura',notes:'',quantity:1,diceCount:1,dieSides:4,healingModifier:0,...patch}));
  for(const patch of [{inventory:[weapon(),weapon()]},{equippedWeaponId:'missing'},{inventory:[weapon({quantity:0})],equippedWeaponId:'bow'},{inventory:[weapon({type:'gear'})],equippedWeaponId:'bow'}])assert.throws(()=>validateInventory({...state,...patch}));
  const raw=JSON.parse(serialize(state));raw.inventory[0].quantity=-1;assert.throws(()=>parseBackup(JSON.stringify(raw)));
  assert.throws(()=>adjustQuantity(state,'bow',-2));assert.throws(()=>adjustQuantity(state,'missing',1));
});

test('limite inventario e modifica conservano esplicitamente le quantità correnti',()=>{
  let state={...initialState(),inventory:[],equippedWeaponId:null};
  for(let i=0;i<MAX_ITEMS;i++)state=upsertItem(state,{id:`item-${i}`,type:'gear',name:'Oggetto',notes:'',quantity:0});
  assert.throws(()=>upsertItem(state,weapon()));
  const next=upsertItem(state,{id:'item-0',type:'consumable',name:'Oggetto modificato',notes:'Nota',quantity:2});
  assert.equal(next.inventory.length,MAX_ITEMS);assert.equal(next.inventory[0].quantity,2);assert.equal(state.inventory[0].quantity,0);
});
