// Standard DDB skill identifiers are not positions in the local alphabetic list.
export const beyondSkillIds=Object.freeze({2:'athletics',3:'acrobatics',4:'sleightOfHand',5:'stealth',6:'arcana',7:'history',8:'investigation',9:'nature',10:'religion',11:'animalHandling',12:'insight',13:'medicine',14:'perception',15:'survival',16:'deception',17:'intimidation',18:'performance',19:'persuasion'});
const abilities=['FOR','DES','COS','INT','SAG','CAR'];
const supported=new Set([24,25,26,27,39,40,41]);
const integer=v=>{if(typeof v==='string'&&/^[+-]?(?:0|[1-9]\d*)$/.test(v.trim()))v=Number(v.trim());return Number.isSafeInteger(v)?v:null;};
const conditional=v=>['restriction','duration','dice','statId'].some(k=>v[k]!=null&&v[k]!=='')||['conditions','tagConstraints'].some(k=>v[k]!=null&&(!Array.isArray(v[k])||v[k].length));
export function parseBeyondTraining(values){
 const remaining=[],training=[],blockers=[],seen=new Map();
 const block=t=>{if(!blockers.includes(t))blockers.push(t);};
 for(const v of values){
  if(!v||!supported.has(v.typeId)){remaining.push(v);continue;}
  const skill=v.typeId<30,id=integer(v.valueId),key=skill?beyondSkillIds[id]:abilities[id-1],value=integer(v.value),proficiency=v.typeId===26||v.typeId===41,mapped=v.typeId===27;
  // Numeric standard fields use an absent/null/zero namespace; item entity IDs
  // and other unverified namespaces are not repurposed as a skill/save target.
  if(!key||conditional(v)||v.valueTypeId!=null&&v.valueTypeId!==0&&v.valueTypeId!=='0'||v.value!==null&&value===null){block(`Personalizzazione tiro tipo ${v.typeId}: bersaglio, valore o requisiti non riconciliati.`);continue;}
  const unset=v.value===null||(proficiency||mapped)&&value===0;
  if(!unset&&(mapped?![1,2,3,4,5,6].includes(value):proficiency?(skill?![1,2,3,4].includes(value):![1,3].includes(value)):value< -30||value>30)){block(`Personalizzazione tiro ${key}: grado o bonus non rappresentabile.`);continue;}
  const identity=`${v.typeId}:${key}`;
  const normalized=unset?null:value;
  if(seen.has(identity)){if(seen.get(identity)!==normalized)block(`Personalizzazioni discordanti dello stesso tiro: ${key}, tipo ${v.typeId}.`);continue;}
  seen.set(identity,normalized);if(unset)continue; // Explicit unset preserves source-derived training.
  training.push({type:v.typeId,kind:skill?'skill':'save',key,value});
 }
 return {remaining,training,blockers};
}
export function applyBeyondTraining(state,training){
 const ranks={...state.skillRanks},saves={...state.saveProficiencies},x=state.extended,skillBonuses={...x.skillBonuses},saveBonuses={...x.saveBonuses},half=new Set(x.halfSkills),blockers=[];
 const skillAbilities={...(x.skillAbilities??{})};let changedAbilities=false;
 for(const t of training){
  if(t.type===27){skillAbilities[t.key]=abilities[t.value-1];changedAbilities=true;}
  else if(t.type===26){ranks[t.key]=t.value===4?2:t.value===3?1:0;half.delete(t.key);if(t.value===2)half.add(t.key);}
  else if(t.type===41)saves[t.key]=t.value===3;
  else (t.kind==='skill'?skillBonuses:saveBonuses)[t.key]+=t.value;
 }
 for(const [kind,values] of [['abilità',skillBonuses],['salvezza',saveBonuses]])for(const [key,value] of Object.entries(values))if(!Number.isSafeInteger(value)||value< -30||value>30)blockers.push(`Bonus totale ${kind} ${key} fuori dai limiti locali (-30..30): nessuna personalizzazione troncata.`);
 if(blockers.length)return blockers;
 if(changedAbilities)x.skillAbilities=skillAbilities;
 state.skillRanks=ranks;state.saveProficiencies=saves;Object.assign(x,{skillBonuses,saveBonuses,halfSkills:[...half]});return [];
}
