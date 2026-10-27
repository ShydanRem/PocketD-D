import catalog from './avatar-species.json' with {type:'json'};
export const avatarSpecies=Object.freeze(catalog.map(s=>Object.freeze(s)));
export const avatarBodies=['Maschile','Femminile'];
export function avatarIdentity(avatar){
  const lineage=avatar?.lineage===undefined?0:avatar.lineage,body=avatar?.body===undefined?0:avatar.body;
  if(!Number.isInteger(lineage)||lineage<0||lineage>=catalog.length||!Number.isInteger(body)||body<0||body>1)throw new Error('Silhouette avatar non valida');
  return {lineage,body};
}
export function withAvatarIdentity(avatar,lineage,body){
  const a={...avatar,lineage,body};avatarIdentity(a);return a;
}
const normalized=value=>typeof value==='string'?value.normalize('NFKD').replace(/[\u0300-\u036f]/g,'').trim().toLowerCase().replace(/\s*\((?:2014|2024)\)\s*$/,'').replace(/[-_\s]+/g,' '):'';
const aliases=new Map(catalog.flatMap(s=>s.aliases.map(name=>[normalized(name),s.id])));
// Names identify only a visual family. Never infer class, bonuses or personal appearance.
export function suggestBeyondAvatar(race,gender){
  const source=typeof race?.fullName==='string'?race.fullName.slice(0,120):typeof race?.baseName==='string'?race.baseName.slice(0,120):'';
  const names=[race?.baseName,race?.fullName].filter(n=>typeof n==='string'&&n.trim());
  const candidates=[...new Set(names.map(n=>aliases.get(normalized(n))).filter(n=>n!==undefined))];
  const homebrew=race?.isHomebrew===true||race?.isCustomRace===true;
  const knownBody=new Map([['male',0],['maschio',0],['maschile',0],['man',0],['female',1],['femmina',1],['femminile',1],['woman',1]]).get(normalized(gender));
  return {source,status:homebrew?'manual':candidates.length===1?'matched':candidates.length>1?'ambiguous':'manual',lineage:!homebrew&&candidates.length===1?candidates[0]:0,body:knownBody??0,bodySource:knownBody===undefined?'manual':'explicit'};
}
