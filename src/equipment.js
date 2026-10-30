// Explicit item properties. Names/icons never grant mechanical effects.
const integer=(n,a,b)=>Number.isSafeInteger(n)&&n>=a&&n<=b;
const keys=['FOR','DES','COS','INT','SAG','CAR'];
const fields=['ac','attack','damage','spellAttack','spellDc','saves','checks'];
export const emptyGear=id=>({id,kind:1,equipped:false,armor:0,ability:[0,0,0,0,0,0],ac:0,attack:0,damage:0,spellAttack:0,spellDc:0,saves:0,checks:0,maxHp:0});
export function validateLoadout(input){
 if(!input||Object.keys(input).length!==2||!integer(input.unarmored,0,3)||!Array.isArray(input.items)||input.items.length>60)throw Error('Equipaggiamento non valido');
 const ids=new Set();let body=0,shield=0;
 const items=input.items.map(g=>{
  if(!g||Object.keys(g).length!==(g.attunement!==undefined?14:13)||g.attunement!==undefined&&!integer(g.attunement,0,2)||typeof g.id!=='string'||!/^[a-zA-Z0-9_-]{1,64}$/.test(g.id)||ids.has(g.id)||!integer(g.kind,1,5)||typeof g.equipped!=='boolean'||!integer(g.armor,g.kind>=2?1:0,g.kind===1?0:g.kind===5?10:30)||!Array.isArray(g.ability)||g.ability.length!==6||g.ability.some(n=>!integer(n,-10,10))||fields.some(k=>!integer(g[k],-10,10))||!integer(g.maxHp,-999,999))throw Error('Proprietà oggetto non valide');
  ids.add(g.id);if(g.equipped){if(g.kind>=2&&g.kind<=4)body++;if(g.kind===5)shield++;}
  return {...g,ability:[...g.ability]};
 });
 if(body>1||shield>1)throw Error('Scegli una sola armatura e un solo scudo');
 return {unarmored:input.unarmored,items};
}
export function validateEquippedState(state){
 const loadout=state.extended?.loadout;if(!loadout)return;
 validateLoadout(loadout);for(const g of loadout.items){const item=state.inventory.find(i=>i.id===g.id);if(!item||item.type!=='gear'||(g.equipped||g.attunement===2)&&!item.quantity)throw Error('Oggetto equipaggiato non disponibile');}
}
const modifier=n=>Math.floor((n-10)/2);
function body(state){
 const l=state.extended?.loadout,dex=modifier(state.abilities.DES),active=l?.items.filter(g=>g.equipped)??[],armor=active.find(g=>g.kind>=2&&g.kind<=4),shield=active.find(g=>g.kind===5);
 return (armor?armor.armor+(armor.kind===2?dex:armor.kind===3?Math.min(2,dex):0):10+dex+(l?.unarmored===2?modifier(state.abilities.COS):l?.unarmored===3&&!shield?modifier(state.abilities.SAG):0))+(shield?.armor??0);
}
export function gearTotals(state){const t={ability:[0,0,0,0,0,0],maxHp:0,...Object.fromEntries(fields.map(k=>[k,0]))};for(const g of state.extended?.loadout?.items??[])if(g.equipped&&g.attunement!==1){g.ability.forEach((n,i)=>t.ability[i]+=n);for(const k of [...fields,'maxHp'])t[k]+=g[k];}return t;}
function changeGear(state,id,equipped,attunement){
 if(typeof equipped!=='boolean'||!state.extended)throw Error('Equipaggiamento non disponibile');
 const item=state.inventory.find(i=>i.id===id);if(!item||item.type!=='gear'||equipped&&!item.quantity)throw Error('Oggetto non disponibile');
 const old=gearTotals(state),before=body(state),loadout=validateLoadout(state.extended.loadout??{unarmored:0,items:[]});let g=loadout.items.find(g=>g.id===id);
 if(!g){g=emptyGear(id);loadout.items.push(g);}g.equipped=equipped;if(attunement!==undefined)g.attunement=attunement;
 if(equipped)for(const other of loadout.items)if(other!==g&&(g.kind>=2&&g.kind<=4&&other.kind>=2&&other.kind<=4||g.kind===5&&other.kind===5))other.equipped=false;
 let next={...state,abilities:{...state.abilities},extended:{...state.extended,loadout,checkBonuses:{...state.extended.checkBonuses},saveBonuses:{...state.extended.saveBonuses},skillBonuses:{...state.extended.skillBonuses},combat:{...state.extended.combat,thrown:false}},encounter:{...state.encounter,strike:null}};
 const t=gearTotals(next),con=modifier(state.abilities.COS),dex=modifier(state.abilities.DES);
 keys.forEach((k,i)=>{const n=state.abilities[k]+t.ability[i]-old.ability[i];if(!integer(n,1,30))throw Error('Caratteristica fuori limite');next.abilities[k]=n;});
 const after=body(next);if(!loadout.unarmored&&before!==after)throw Error('Configura prima la CA senza armatura');
 next.armorClass=state.armorClass+t.ac-old.ac+(loadout.unarmored?after-before:0);next.maxHp=state.maxHp+t.maxHp-old.maxHp+state.level*(modifier(next.abilities.COS)-con);
 if(!integer(next.armorClass,1,40)||!integer(next.maxHp,1,999))throw Error('CA o PF massimi fuori limite');next.hp=Math.min(state.hp,next.maxHp);
 next.inventory=state.inventory.map(i=>i.type==='weapon'?{...i,attackModifier:i.attackModifier+t.attack-old.attack,damageModifier:i.damageModifier+t.damage-old.damage}:i);
 if(next.inventory.some(i=>i.type==='weapon'&&(!integer(i.attackModifier,-10,10)||!integer(i.damageModifier,-10,10))))throw Error('Bonus arma fuori limite');
 next.powers=state.powers.map(p=>p.type==='spell'&&!['true-strike-2024','hex-2024','hunters-mark-2024','divine-smite-2024'].includes(p.effect)?{...p,attackModifier:(p.attackModifier??0)+t.spellAttack-old.spellAttack,saveModifier:(p.saveModifier??0)+t.spellDc-old.spellDc}:p.reference===6?{...p,attackModifier:p.attackModifier+modifier(next.abilities.DES)-dex+t.checks-old.checks}:p);
 if(next.powers.some(p=>p.type==='spell'&&(!integer(p.attackModifier,-15,30)||!integer(p.saveModifier,-15,30))||p.reference===6&&!integer(p.attackModifier,-15,30)))throw Error('Bonus magia fuori limite');
 for(const k of keys){next.extended.saveBonuses[k]+=t.saves-old.saves;next.extended.checkBonuses[k]+=t.checks-old.checks;}
 for(const k of Object.keys(next.extended.skillBonuses))next.extended.skillBonuses[k]+=t.checks-old.checks;
 if([next.extended.saveBonuses,next.extended.checkBonuses,next.extended.skillBonuses].some(map=>Object.values(map).some(n=>!integer(n,-30,30))))throw Error('Bonus tiri fuori limite');
 validateEquippedState(next);return next;
}

export const equipGear=(state,id,equipped)=>changeGear(state,id,equipped);
export function attuneGear(state,id,attuned){
 const g=state.extended?.loadout?.items.find(g=>g.id===id),i=state.inventory.find(i=>i.id===id);
 if(typeof attuned!=='boolean'||!g?.attunement||!i?.quantity)throw Error('Sintonia non disponibile');
 return changeGear(state,id,g.equipped,attuned?2:1);
}
