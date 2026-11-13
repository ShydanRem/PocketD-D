// Explicit weapon handling; properties in notes never enable rules.
export const emptyHandling=()=>({flags:0,grip:0,versatileSides:0,ammoId:null});
export const hasHandling=s=>s.inventory?.some(i=>i.handling!==undefined)??false;
export function validateHandling(h,item){
 const int=(n,a,b)=>Number.isInteger(n)&&n>=a&&n<=b;
 if(!h||Array.isArray(h)||Object.keys(h).length!==4||!['flags','grip','versatileSides','ammoId'].every(k=>Object.hasOwn(h,k))||item.type!=='weapon'||!int(h.flags,0,31)||!int(h.grip,0,1)||h.ammoId!==null&&(typeof h.ammoId!=='string'||!/^[a-zA-Z0-9_-]{1,64}$/.test(h.ammoId)||h.ammoId===item.id))throw Error('Impugnatura o munizioni non valide');
 if((h.flags&16)&&item.profile?.ranged)throw Error('Scalfire richiede arma da mischia');
 if((h.flags&1)&&(h.flags&10)||!(h.flags&2)&&(h.grip||h.versatileSides)||(h.flags&2)&&(item.profile?.ranged||item.diceCount!==1||![6,8,10,12].includes(h.versatileSides)||h.versatileSides<=item.dieSides)||(h.flags&4)&&!['FOR','DES'].includes(item.ability))throw Error('Proprietà arma incompatibili');
 return {...h};
}
export function twoHands(s,item){const h=item?.handling;return Boolean(h&&((h.flags&1)||((h.flags&2)&&h.grip&&!(s.equippedWeaponId===item.id&&s.extended?.combat?.thrown&&item.profile?.thrownRange))));}
export function handsReady(s,item){return !twoHands(s,item)||!s.extended?.loadout?.items.some(g=>g.kind===5&&g.equipped);}

export function equipHandsReady(s,item){const h=item?.handling;return !(h&&((h.flags&1)||((h.flags&2)&&h.grip)))||!s.extended?.loadout?.items.some(g=>g.kind===5&&g.equipped);}
