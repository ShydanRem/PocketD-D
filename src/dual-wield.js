import {weaponStats} from './model.js';
export const emptyDual=()=>({weaponId:null,edition:0,style:false,bonusSpent:false});
export const expandDual=d=>({...emptyDual(),feat:false,lightSpent:false,featSpent:false,cleaveSpent:false,...d});
export const hasExtra=s=>s.inventory?.some(i=>(i.handling?.flags>>4)>=2)||s.dualWield&&['feat','lightSpent','featSpent','cleaveSpent'].some(k=>Object.hasOwn(s.dualWield,k));
export const hasDual=s=>s.dualWield!==undefined||s.inventory?.some(i=>(i.handling?.flags&8)||(i.handling?.flags>>4)>0);
export function validateDual(d,s){
 if(!d||Array.isArray(d)||!['bonusSpent edition style weaponId','bonusSpent cleaveSpent edition feat featSpent lightSpent style weaponId'].includes(Object.keys(d).sort().join(' '))||typeof d.style!=='boolean'||typeof d.bonusSpent!=='boolean'||![0,1,2].includes(d.edition))throw Error('Due armi: profilo non valido');
 const full=Object.hasOwn(d,'feat');if(full&&['feat','lightSpent','featSpent','cleaveSpent'].some(k=>typeof d[k]!=='boolean'))throw Error('Contatori attacchi extra non validi');
 if(d.feat&&(d.edition!==2||d.weaponId===null))throw Error('Dual Wielder richiede il profilo 2024');
 if(d.weaponId===null){if(d.edition||d.style)throw Error('Due armi: manca la secondaria');}
 else if(typeof d.weaponId!=='string'||!d.edition||!s.inventory.some(i=>i.id===d.weaponId&&i.type==='weapon'))throw Error('Due armi: arma secondaria assente');
 return {...d};
}
export function dualStats(s,method=0){
 if(![0,1,2].includes(method))return null;
 const d=validateDual(s.dualWield??emptyDual(),s),a=s.inventory.find(i=>i.id===s.equippedWeaponId),b=s.inventory.find(i=>i.id===d.weaponId);
 if(!s.hp||!a||!b||a.id===b.id||!a.quantity||!b.quantity||!(a.handling?.flags&8)||method!==2&&!(b.handling?.flags&8)||s.extended?.combat?.thrown||s.extended?.loadout?.items.some(g=>g.kind===5&&g.equipped))return null;
 const two=i=>i.handling.flags&1||i.handling.flags&2&&i.handling.grip;
 if(two(a)||two(b)||d.edition===1&&(a.profile?.ranged||b.profile?.ranged))return null;
 if(method===1&&(d.edition!==2||(b.handling.flags>>4)!==2)||method===2&&(d.edition!==2||!d.feat||b.profile?.ranged))return null;
 const w=weaponStats({...s,equippedWeaponId:b.id});if(!w)return null;
 const modifier=Math.floor((s.abilities[w.ability]-10)/2),damageBase=d.style?modifier:Math.min(0,modifier);
 return {...w,damageBonus:w.damageBonus-(w.addAbilityToDamage?modifier:0)+damageBase,damageBase};
}

export function extraReady(s,method=0){
 if(!dualStats(s,method))return false;const d=expandDual(s.dualWield);
 return method===1?!d.lightSpent:method===2?!d.bonusSpent&&!d.featSpent:!d.bonusSpent&&!d.lightSpent;
}
