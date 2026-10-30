import {equipGear,gearTotals} from './equipment.js';
export const MAX_ITEMS = 60;
export const DIE_SIDES = [4,6,8,10,12];
const integer = (n,min,max) => Number.isInteger(n) && n>=min && n<=max;
const validId = id => typeof id==='string' && /^[a-zA-Z0-9_-]{1,64}$/.test(id);
const abilities = ['FOR','DES','COS','INT','SAG','CAR'];

export function startingInventory(legacy={}) {
  return {
    inventory:[
      {id:'starter-sword',type:'weapon',name:'Spada',notes:'Arma dimostrativa; verifica i valori con il DM.',quantity:1,
        ability:'FOR',proficient:true,diceCount:1,dieSides:8,attackModifier:0,damageModifier:0,addAbilityToDamage:true,reach:legacy.weaponReach??1,damageType:'Tagliente'},
      {id:'starter-potion',type:'healing',name:'Pozione',notes:'Recupera PF senza superare il massimo.',quantity:legacy.potions??2,diceCount:2,dieSides:4,healingModifier:2},
      {id:'starter-torch',type:'consumable',name:'Torcia',notes:'Consumo manuale: concorda durata ed effetti con il DM.',quantity:legacy.torches??3}
    ],
    equippedWeaponId:legacy.swordEquipped===false?null:'starter-sword'
  };
}

export function validateItem(input,extended=false,allowCharges=true) {
  if(!input || !validId(input.id) || !['gear','consumable','healing','weapon'].includes(input.type) || typeof input.name!=='string' || !input.name.trim() || input.name.length>40 || typeof input.notes!=='string' || input.notes.length>400 || !integer(input.quantity,0,999)) throw new Error('Oggetto non valido');
  const item={id:input.id,type:input.type,name:input.name.trim(),notes:input.notes,quantity:input.quantity};
  if(input.charges!==undefined){
    const c=input.charges;
    if(!allowCharges||!c||typeof c!=='object'||Array.isArray(c)||Object.keys(c).length!==3||!['current','max','recovery'].every(k=>Object.hasOwn(c,k))||!integer(c.max,1,999)||!integer(c.current,0,c.max)||!['manual','short','long'].includes(c.recovery))throw new Error('Cariche oggetto non valide');
    item.charges={current:c.current,max:c.max,recovery:c.recovery};
  }
  if(['weapon','healing'].includes(input.type)) {
    if(!integer(input.diceCount,1,6) || !DIE_SIDES.includes(input.dieSides)) throw new Error('Dadi non validi');
    Object.assign(item,{diceCount:input.diceCount,dieSides:input.dieSides});
  }
  if(input.type==='weapon') {
    if(!abilities.includes(input.ability) || typeof input.proficient!=='boolean' || typeof input.addAbilityToDamage!=='boolean' || !integer(input.attackModifier,-10,10) || !integer(input.damageModifier,-10,10) || !integer(input.reach,1,extended?120:9) || typeof input.damageType!=='string' || input.damageType.length>24) throw new Error('Arma non valida');
    Object.assign(item,{ability:input.ability,proficient:input.proficient,addAbilityToDamage:input.addAbilityToDamage,attackModifier:input.attackModifier,damageModifier:input.damageModifier,reach:input.reach,damageType:input.damageType});
  }
  if(input.type==='healing') {
    if(!integer(input.healingModifier,0,30)) throw new Error('Bonus cura non valido');
    item.healingModifier=input.healingModifier;
  }
  if(extended&&input.profile!==undefined){
    const p=input.profile;
    if(!p||typeof p.ranged!=='boolean'||typeof p.heavy!=='boolean'||!integer(p.thrownRange,0,120)||!integer(p.longRange,0,240)||!Array.isArray(p.properties)||p.properties.length>20||p.properties.some(t=>typeof t!=='string'||t.length>60))throw new Error('Proprietà arma non valide');
    item.profile={ranged:p.ranged,heavy:p.heavy,thrownRange:p.thrownRange,longRange:p.longRange,properties:[...p.properties]};
  }
  return item;
}

export function validateInventory(input,extended=false,allowCharges=true) {
  if(!input || !Array.isArray(input.inventory) || input.inventory.length>MAX_ITEMS) throw new Error(`Massimo ${MAX_ITEMS} oggetti`);
  const inventory=input.inventory.map(item=>validateItem(item,extended,allowCharges));
  if(new Set(inventory.map(item=>item.id)).size!==inventory.length) throw new Error('Oggetti duplicati');
  if(input.equippedWeaponId!==null && !inventory.some(item=>item.id===input.equippedWeaponId && item.type==='weapon' && item.quantity>0)) throw new Error('Arma equipaggiata non valida');
  return {inventory,equippedWeaponId:input.equippedWeaponId};
}

export function equippedWeapon(state) {
  return state.inventory.find(item=>item.id===state.equippedWeaponId && item.type==='weapon' && item.quantity>0)??null;
}

// Equipment and quantity edits invalidate a previously prepared strike.
function changed(state,inventory,equippedWeaponId=state.equippedWeaponId) {
  if(!inventory.some(item=>item.id===equippedWeaponId && item.type==='weapon' && item.quantity>0)) equippedWeaponId=null;
  return {...state,...(state.extended?{extended:{...state.extended,combat:{...state.extended.combat,thrown:false}}}:{}),inventory,equippedWeaponId,encounter:{...state.encounter,strike:null}};
}

export function upsertItem(state,input) {
  let item=validateItem(input,Boolean(state.extended));const exists=state.inventory.some(other=>other.id===item.id);
  if(!exists && state.inventory.length>=MAX_ITEMS) throw new Error(`Massimo ${MAX_ITEMS} oggetti`);
  if(item.type!=='gear'&&state.extended?.loadout?.items.some(g=>g.id===item.id)){
    const current=state.extended.loadout.items.find(g=>g.id===item.id);if(current.equipped)state=equipGear(state,item.id,false);
    state={...state,extended:{...state.extended,loadout:{...state.extended.loadout,items:state.extended.loadout.items.filter(g=>g.id!==item.id)}}};
  }
  if(item.type==='weapon'&&(!exists||state.inventory.find(i=>i.id===item.id)?.type!=='weapon')&&state.extended?.loadout){const t=gearTotals(state);item=validateItem({...item,attackModifier:item.attackModifier+t.attack,damageModifier:item.damageModifier+t.damage},true);}
  return changed(state,exists?state.inventory.map(other=>other.id===item.id?item:other):[...state.inventory,item]);
}

export function removeItem(state,id) {
  if(!state.inventory.some(item=>item.id===id)) throw new Error('Oggetto non trovato');
  if(state.extended?.loadout?.items.some(g=>g.id===id)){if(state.extended.loadout.items.find(g=>g.id===id).equipped)state=equipGear(state,id,false);state={...state,extended:{...state.extended,loadout:{...state.extended.loadout,items:state.extended.loadout.items.filter(g=>g.id!==id)}}};}
  return changed(state,state.inventory.filter(item=>item.id!==id));
}

export function equipWeapon(state,id) {
  if(id!==null && !state.inventory.some(item=>item.id===id && item.type==='weapon' && item.quantity>0)) throw new Error('Arma non disponibile');
  return changed(state,state.inventory,id);
}

export function adjustQuantity(state,id,delta) {
  const item=state.inventory.find(item=>item.id===id);
  if(!item || !integer(delta,-999,999) || !integer(item.quantity+delta,0,999)) throw new Error('Quantità non valida');
  if(item.quantity+delta===0&&state.extended?.loadout?.items.some(g=>g.id===id&&g.equipped))state=equipGear(state,id,false);
  return changed(state,state.inventory.map(other=>other.id===id?{...other,quantity:item.quantity+delta}:other));
}

export function weaponReach(state){
  const weapon=equippedWeapon(state);
  return state.extended?.combat.thrown&&weapon?.profile?.thrownRange?weapon.profile.thrownRange:weapon?.reach??1;
}

// One shared charge pool per inventory row; an absent object never recovers.
export function recoverItemCharges(inventory,kind){
  return inventory.map(item=>item.quantity>0&&item.charges&&(item.charges.recovery===kind||(kind==='long'&&item.charges.recovery==='short'))?{...item,charges:{...item.charges,current:item.charges.max}}:item);
}
