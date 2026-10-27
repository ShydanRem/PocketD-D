import catalog from './avatar-classes.json' with {type:'json'};
export const avatarClasses=Object.freeze(catalog.map(c=>Object.freeze(c)));
export function avatarOutfit(avatar){const value=avatar?.outfit===undefined?0:avatar.outfit;if(!Number.isInteger(value)||value<0||value>=catalog.length)throw new Error('Preset classe non valido');return value;}
const normalized=s=>typeof s==='string'?s.trim().toLowerCase().replace(/\s*\((?:2014|2024)\)\s*$/,''):'';
export function suggestBeyondClass(classes){
  if(!Array.isArray(classes)||classes.length!==1)return {outfit:0,status:'manual',reason:'Classe assente o multiclasse: scegli il look manualmente.'};
  const source=classes[0]?.definition,name=normalized(source?.name),match=catalog.find(c=>c.id&&c.aliases.some(a=>normalized(a)===name));
  if(!match||source?.isHomebrew===true||classes[0]?.isHomebrew===true)return {outfit:0,status:'manual',reason:'Classe personalizzata o non riconosciuta: scegli il look manualmente.'};
  return {outfit:match.id,status:'matched',reason:`Look proposto: ${match.name}. Solo estetica; armi, CA e capacità non cambiano.`};
}
