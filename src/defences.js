// Order is the firmware's stable damage-type order. Flags: R=1, V=2, I=4.
export const damageTypes=Object.freeze(['bludgeoning','piercing','slashing','acid','cold','fire','force','lightning','necrotic','poison','psychic','radiant','thunder']);
export const damageNames=Object.freeze(['Contundente','Perforante','Tagliente','Acido','Freddo','Fuoco','Forza','Fulmine','Necrotico','Veleno','Psichico','Radiante','Tuono']);
export const emptyDefences=()=>({configured:0,resistance:0,vulnerability:0,immunity:0});
export function validateDefences(input){
 const keys=['configured','resistance','vulnerability','immunity'];
 if(!input||Array.isArray(input)||Object.keys(input).length!==4||keys.some(k=>!Object.hasOwn(input,k)||!Number.isInteger(input[k])||input[k]<0||input[k]>8191)||((input.resistance|input.vulnerability|input.immunity)&~input.configured))throw new Error('Profilo difese non valido');
 return Object.fromEntries(keys.map(k=>[k,input[k]]));
}
export function defenceFlags(profile,type){const d=validateDefences(profile),bit=1<<type;if(!Number.isInteger(type)||type<0||type>=13)throw new Error('Tipo di danno non valido');return !(d.configured&bit)?null:(d.resistance&bit?1:0)|(d.vulnerability&bit?2:0)|(d.immunity&bit?4:0);}
export const defenceLabel=flags=>flags===null?'Da verificare':flags===0?'Normale':[flags&1?'Resistenza':'',flags&2?'Vulnerabilità':'',flags&4?'Immunità':''].filter(Boolean).join(' + ');
