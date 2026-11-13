import {validateHandling} from './handling.js';
// Structured property rows only. Never inspect weapon names or descriptions.
export function beyondWeaponHandling(definition,weapon,warn){
 if(definition.isHomebrew===true||!Array.isArray(definition.properties))return undefined;
 const rows=definition.properties.filter(p=>Number.isSafeInteger(p?.id)&&p.id>0&&typeof p.name==='string');
 const named=name=>rows.filter(p=>p.name===name),two=named('Two-Handed'),versatile=named('Versatile'),finesse=named('Finesse');
 if(!two.length&&!versatile.length&&!finesse.length)return undefined;
 if(two.length>1||versatile.length>1||finesse.length>1){warn('proprietà arma duplicate: impugnatura da configurare.');return undefined;}
 const notes=versatile[0]?.notes;const match=typeof notes==='string'?notes.match(/^1d(6|8|10|12)$/):null;
 if(versatile.length&&!match){warn('dado versatile non strutturato o non riconciliato: impugnatura da configurare.');return undefined;}
 const h={flags:(two.length?1:0)|(versatile.length?2:0)|(finesse.length?4:0),grip:0,versatileSides:match?Number(match[1]):0,ammoId:null};
 try{return validateHandling(h,weapon);}catch{warn('proprietà arma incompatibili con i valori convertiti: impugnatura da configurare.');return undefined;}
}
