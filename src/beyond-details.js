import {skills,abilityKeys,modifier,proficiency,parseBackup,serialize,describeRoll,weaponStats} from './model.js';
import {emptyExtended} from './extended.js';
import {validatePower,spellStats} from './powers.js';
import {DIE_SIDES} from './inventory.js';

const integer=(n,min,max)=>Number.isSafeInteger(n)&&n>=min&&n<=max;
const slug=s=>String(s??'').trim().toLowerCase().replace(/[’']/g,'').replace(/[^a-z0-9]+/g,'-').replace(/^-|-$/g,'');
const abilityNames=['strength','dexterity','constitution','intelligence','wisdom','charisma'];
const skillBySlug=new Map(skills.map(s=>[slug(s.id.replace(/[A-Z]/g,c=>`-${c}`)),s.id]));
const array=v=>Array.isArray(v)?v:[];
const entities={amp:'&',lt:'<',gt:'>',quot:'"',apos:"'",nbsp:' ',rsquo:'’',lsquo:'‘',rdquo:'”',ldquo:'“',ndash:'–',mdash:'—',hellip:'…'};
export function beyondText(html){
  return String(html??'').replace(/<(script|style)\b[^>]*>[\s\S]*?<\/\1\s*>/gi,'').replace(/<br\s*\/?\s*>|<\/(p|div|li|h[1-6])\s*>/gi,'\n').replace(/<[^>]*>/g,'').replace(/&(#x[0-9a-f]+|#\d+|[a-z]+);/gi,(all,key)=>{
    if(key[0]!=='#')return entities[key.toLowerCase()]??all;
    const code=key[1].toLowerCase()==='x'?parseInt(key.slice(2),16):Number(key.slice(1));
    return code>0&&code<=0x10ffff&&!(code>=0xd800&&code<=0xdfff)?String.fromCodePoint(code):'�';
  }).replace(/\r/g,'').replace(/[ \t]+/g,' ').replace(/\n\s*\n+/g,'\n\n').trim();
}

export function extendBeyond(report,c){
  if(!report.draft)return {...report,version:3,conversion:{...report.conversion,scope:'extended-v1',coverage:[]}};
  const state=structuredClone(report.draft),ext=emptyExtended(),issues=[],evidence=[],facts=[];
  state.extended=ext;
  const warn=t=>{if(!issues.includes(t))issues.push(t)};
  const fact=t=>{if(t&&!facts.includes(t))facts.push(String(t).slice(0,500))};
  const text=(value,label)=>{const result=beyondText(value);if(result.length>6000)warn(`Descrizione di ${label}: oltre 6000 caratteri, conservato estratto.`);return result.slice(0,6000)};
  const choices=(group)=>array(c.choices?.[group]);
  const selected=(group,m)=>choices(group).find(choice=>choice?.id?.endsWith(`-${m.id}`));
  const options=array(c.choices?.choiceDefinitions).flatMap(x=>array(x?.options));
  const mods=[];
  for(const [group,values] of Object.entries(c.modifiers??{}))for(const original of array(values)){
    if(!original||typeof original.subType!=='string')continue;
    let m={...original,group};
    if(group==='class'){
      const owner=array(c.classes).find(cl=>array(cl.classFeatures).some(f=>f?.definition?.id===m.componentId));
      if(!owner)continue;
      const feature=owner.classFeatures.find(f=>f?.definition?.id===m.componentId).definition;
      if(feature.requiredLevel>owner.level)continue;
    }
    if(group==='item'){
      if(!array(c.inventory).some(i=>i?.equipped&&(!i.definition?.canAttune||i.isAttuned)&&array(i.definition?.grantedModifiers).some(g=>g?.id!=null&&String(g.id)===String(m.id))))continue;
    }
    const choice=selected(group,m);
    if(choice&&choice.optionValue==null){
      if(m.type==='proficiency'&&m.subType.startsWith('choose-'))warn('Una scelta di competenza è ancora vuota nel sorgente Beyond: non è stata inventata.');
      continue;
    }
    if(m.isGranted===false&&!choice)continue;
    if(m.isGranted===false&&choice&&choice.optionValue!=null&&!m.subType.startsWith('choose-')){
      const option=options.find(o=>o.id===choice.optionValue);
      if(!option||slug(option.label)!==m.subType){
        const skill=skillBySlug.get(m.subType);if(skill)ext.blockedChecks.push(`skill:${skill}`);
        warn('Scelta di competenza discordante: tiro da verificare.');continue;
      }
    }
    if(m.subType.startsWith('choose-')){
      if(choice?.optionValue==null)continue;
      const option=options.find(o=>o.id===choice.optionValue);
      if(!option){warn('Scelta di competenza non riconosciuta: controllare le competenze importate.');continue;}
      m.subType=slug(option.label);
    }
    mods.push(m);
  }
  const skillRanks={...state.skillRanks},saves={...state.saveProficiencies};
  const targetChecks=sub=>sub==='ability-checks'?[...skills.map(s=>`skill:${s.id}`),...abilityKeys.map(k=>`check:${k}`)]:sub==='saving-throws'?abilityKeys.map(k=>`save:${k}`):skillBySlug.has(sub)?[`skill:${skillBySlug.get(sub)}`]:abilityNames.some(n=>sub===`${n}-saving-throws`)?[`save:${abilityKeys[abilityNames.findIndex(n=>sub===`${n}-saving-throws`)]}`]:[];
  for(const m of mods){
    const targets=targetChecks(m.subType);
    if(m.type==='language'||(m.type==='proficiency'&&!targets.length&&!m.subType.startsWith('choose-')))fact(`${m.type==='language'?'Lingua':'Competenza'}: ${m.friendlySubtypeName??m.subType}`);
    if(!targets.length)continue;
    if(m.restriction){fact(`${m.friendlySubtypeName??m.subType}: ${m.type}; ${beyondText(m.restriction)}`);continue;}
    for(const target of targets){
      const [kind,key]=target.split(':');
      if(m.type==='proficiency'&&kind!=='check'){if(kind==='skill')skillRanks[key]=Math.max(1,skillRanks[key]);else saves[key]=true;}
      else if(m.type==='expertise'&&kind==='skill')skillRanks[key]=2;
      else if(m.type==='half-proficiency'&&kind==='skill'){if(!ext.halfSkills.includes(key))ext.halfSkills.push(key);}
      else if(m.type==='half-proficiency'&&kind==='check')ext.halfAbilityChecks=true;
      else if(m.type==='bonus'&&m.statId==null&&integer(m.value,-30,30)){(kind==='skill'?ext.skillBonuses:kind==='save'?ext.saveBonuses:ext.checkBonuses)[key]+=m.value;}
      else if(!['advantage','disadvantage'].includes(m.type)){
        if(!ext.blockedChecks.includes(target))ext.blockedChecks.push(target);
        warn(`Tiro ${key}: modificatore ${m.type} non risolto, tiro bloccato fino a correzione manuale.`);
      }else fact(`${key}: ${m.type}, da scegliere manualmente nella modalità del dado.`);
    }
  }
  if(array(c.customProficiencies).length)warn('Competenze personalizzate non standard: controllare i tiri; consultare il sorgente Beyond.');
  state.skillRanks=skillRanks;state.saveProficiencies=saves;
  evidence.push(`Competenze: ${Object.values(skillRanks).filter(Boolean).length} abilità, ${Object.values(saves).filter(Boolean).length} salvezze; scelte risolte tramite identificativi, senza inferenza dalla classe.`);

  const weaponIssues=[];
  state.inventory=state.inventory.map((item,index)=>{
    const src=c.inventory[index],d=src.definition;
    if(d.filterType!=='Weapon')return item;
    const propertyNames=array(d.properties).map(p=>p?.name).filter(n=>typeof n==='string');
    const has=name=>propertyNames.some(p=>slug(p)===name);
    const note=message=>{weaponIssues.push(message);warn(`${item.name}: ${message}`);return item};
    if(!integer(d.damage?.diceCount,1,6)||!DIE_SIDES.includes(d.damage?.diceValue)||![1,2].includes(d.attackType)||d.fixedDamage||d.damage.fixedValue) return note('danno o attacco non standard; oggetto conservato, arma da configurare.');
    const ranged=d.attackType===2;
    const ability=ranged?'DES':has('finesse')&&state.abilities.DES>state.abilities.FOR?'DES':'FOR';
    const proficient=mods.some(m=>m.type==='proficiency'&&!m.restriction&&(m.subType===`${d.categoryId===1?'simple':'martial'}-weapons`&&[1,2].includes(d.categoryId)||m.subType===slug(d.type)||m.subType===slug(d.name)||m.subType===slug(src.pocketddOriginalName)));
    const grants=array(d.grantedModifiers);
    let magic=0;
    for(const m of grants){
      if(m.type==='bonus'&&m.subType==='magic'&&integer(m.value,0,10)&&!m.restriction){if(!d.canAttune||src.isAttuned)magic+=m.value;}
      else if(/attack|damage|magic/.test(m.subType??'')&&!/armor-class/.test(m.subType??''))return note('bonus arma non risolto; oggetto conservato, arma da configurare.');
    }
    let attackModifier=magic,damageModifier=magic;
    for(const m of mods){
      if(!['weapon-attacks',ranged?'ranged-attacks':'melee-attacks','weapon-damage',ranged?'ranged-weapon-damage':'melee-weapon-damage'].includes(m.subType))continue;
      if(m.restriction||m.type==='advantage'||m.type==='disadvantage'){fact(`${item.name}: ${m.type} ${m.subType}, ${beyondText(m.restriction)||'da gestire manualmente'}`);continue;}
      if(m.type!=='bonus'||m.statId!=null||!integer(m.value,-10,10))return note('modificatore generale di attacco/danno non risolto.');
      if(m.subType.endsWith('attacks'))attackModifier+=m.value;else damageModifier+=m.value;
    }
    if(!integer(attackModifier,-10,10)||!integer(damageModifier,-10,10))return note('bonus oltre i limiti locali.');
    const range=ranged?d.range:has('reach')?10:5;
    if(!integer(range,5,600)||range%5||has('thrown')&&(!integer(d.range,5,600)||d.range%5))return note('portata non rappresentabile in caselle da 5 piedi.');
    const weapon={...item,type:'weapon',ability,proficient,addAbilityToDamage:true,diceCount:d.damage.diceCount,dieSides:d.damage.diceValue,attackModifier,damageModifier,reach:range/5,damageType:typeof d.damageType==='string'?d.damageType:'',profile:{ranged,heavy:has('heavy'),thrownRange:has('thrown')?d.range/5:0,longRange:integer(d.longRange,0,1200)?Math.ceil(d.longRange/5):0,properties:propertyNames}};
    if(weapon.damageType.length>24||propertyNames.some(n=>n.length>60)||propertyNames.length>20)return note('proprietà o tipo di danno oltre i limiti locali.');
    weapon.notes=`Beyond: ${src.equipped?'equipaggiato':'non equipaggiato'}. Proprietà: ${propertyNames.join(', ')||'nessuna'}. Modalità primaria; maestrie, due armi e danni aggiuntivi condizionali al DM.${src.pocketddNote?' '+src.pocketddNote:''}`.slice(0,400);
    return weapon;
  });
  const equipped=state.inventory.filter((item,index)=>item.type==='weapon'&&item.quantity>0&&c.inventory[index].equipped);
  state.equippedWeaponId=equipped.length===1?equipped[0].id:null;
  if(equipped.length>1)warn('Più armi equipaggiate su Beyond: scegli l’arma attiva nell’anteprima o nelle Azioni; tutte le configurazioni sono conservate.');
  const weapons=state.inventory.filter(i=>i.type==='weapon');
  evidence.push(`Armi: ${weapons.length} configurate con competenza, caratteristica, dadi, bonus e portata; ${equipped.length} equipaggiate nel sorgente.`);

  const powers=[],featureDefs=[],actionKeys=new Set();
  for(const cl of array(c.classes)){
    fact(`${cl.definition?.name??'Classe'} · livello ${cl.level}`);
    for(const f of array(cl.classFeatures))if(f?.definition&&f.definition.requiredLevel<=cl.level)featureDefs.push({group:'class',d:f.definition,scale:f.levelScale,owner:cl});
  }
  for(const f of array(c.race?.racialTraits))if(f?.definition)featureDefs.push({group:'race',d:f.definition});
  for(const f of array(c.feats))if(f?.definition)featureDefs.push({group:'feat',d:f.definition});
  if(c.race?.fullName)fact(`Specie: ${c.race.fullName}`);
  if(c.background?.definition?.name)fact(`Background: ${c.background.definition.name}`);
  const addPower=power=>{
    if(typeof power.name!=='string'||!power.name.trim()||power.name.length>40){warn('Nome di una magia/capacità oltre 40 caratteri: voce non importata.');return;}
    try{powers.push(validatePower(power,true))}catch{warn(`${power.name}: risorsa non rappresentabile; consultare il sorgente.`);}
  };
  const getResource=(limited,name)=>{
    if(!limited)return null;
    if(limited.minNumberConsumed!=null&&limited.minNumberConsumed!==1||limited.maxNumberConsumed!=null&&limited.maxNumberConsumed!==1){warn(`${name}: costo variabile o condiviso da gestire manualmente.`);return null;}
    let max=limited.maxUses;
    if(!integer(max,0,99)){warn(`${name}: formula degli utilizzi non risolta, capacità conservata come nota.`);return null;}
    if(limited.statModifierUsesId!=null){
      if(!integer(limited.statModifierUsesId,1,6)||![1,2].includes(limited.operator)){warn(`${name}: formula degli utilizzi non risolta.`);return null;}
      const value=modifier(state.abilities[abilityKeys[limited.statModifierUsesId-1]]);max=limited.operator===2?max*value:max+value;
    }
    if(limited.useProficiencyBonus){
      if(![1,2].includes(limited.proficiencyBonusOperator)){warn(`${name}: formula di competenza non risolta.`);return null;}
      max=limited.proficiencyBonusOperator===2?max*proficiency(state.level):max+proficiency(state.level);
    }
    if(!integer(max,1,99)||!integer(limited.numberUsed,0,max)){warn(`${name}: contatore degli utilizzi incompleto o incoerente.`);return null;}
    const recovery=limited.resetType===1?'short':limited.resetType===2?'long':'manual';
    if(recovery==='manual'||limited.resetDice)warn(`${name}: recupero giornaliero/variabile da gestire manualmente.`);
    return {maxUses:max,remaining:max-limited.numberUsed,recovery:limited.resetDice?'manual':recovery};
  };
  for(const [group,values] of Object.entries(c.actions??{}))for(const a of array(values)){
    if(!a||typeof a.name!=='string')continue;
    const f=featureDefs.find(f=>f.group===group&&f.d.id===a.componentId);
    if(group==='class'&&!f){warn(`${a.name}: origine o livello non risolti, azione non importata.`);continue;}
    const resource=getResource(a.limitedUse,a.name);
    let description=text(a.description||f?.d.description||a.snippet,a.name);
    if(f?.scale?.fixedValue!=null)description=description.replace(/\{\{scalevalue#signed\}\}/g,`+${f.scale.fixedValue}`).replace(/\{\{scalevalue\}\}/g,String(f.scale.fixedValue));
    if(/\{\{/.test(description))warn(`${a.name}: descrizione con formula dinamica non risolta.`);
    const power={id:`ddb-action-${group}-${powers.length+1}`,type:'feature',name:a.name,description,...(resource??{maxUses:0,remaining:0,recovery:'manual',passive:true})};
    if(resource&&group==='class'&&a.componentId===51&&f?.owner?.definition?.id===9&&integer(f.scale?.fixedValue,1,10)){
      const heavy=array(c.inventory).some(i=>i.equipped&&i.definition?.armorTypeId===3);
      if(!heavy){ext.combat.rageBonus=f.scale.fixedValue;power.effect='rage';}
    }
    if(resource&&group==='race'&&a.componentId===58&&f?.d.name==='Relentless Endurance')power.effect='relentless-endurance';
    addPower(power);actionKeys.add(`${group}:${a.componentId}:${slug(a.name)}`);
  }
  for(const f of featureDefs){
    if(actionKeys.has(`${f.group}:${f.d.id}:${slug(f.d.name)}`))continue;
    addPower({id:`ddb-feature-${f.group}-${powers.length+1}`,type:'feature',name:f.d.name,description:text(f.d.description,f.d.name),maxUses:0,remaining:0,recovery:'manual',passive:true});
  }
  const bg=c.background?.definition;
  if(bg?.featureName)addPower({id:'ddb-background-feature',type:'feature',name:bg.featureName,description:text(bg.featureDescription,bg.featureName),maxUses:0,remaining:0,recovery:'manual',passive:true});
  ext.combat.greatWeaponMasterAvailable=featureDefs.some(f=>f.group==='feat'&&f.d.id===21&&f.d.name==='Great Weapon Master');
  ext.combat.extraMeleeCriticalDice=featureDefs.some(f=>f.group==='race'&&f.d.id===59&&f.d.name==='Savage Attacks')?1:0;

  const slotArray=(values,label,maxima=null)=>{
    const result=Array.from({length:9},()=>({max:0,current:0})),seen=new Set();
    if(values!=null&&!Array.isArray(values)){warn(`${label}: elenco non valido.`);return null;}
    for(const s of array(values)){
      if(!integer(s?.level,1,9)||seen.has(s.level)||!integer(s.available,0,9)||!integer(s.used,0,maxima?.[s.level-1]??s.available)||(maxima&&s.available!==0&&s.available!==maxima[s.level-1])){warn(`${label}: contatore non risolto, importazione slot bloccata.`);return null;}
      seen.add(s.level);const maximum=maxima?.[s.level-1]??s.available;result[s.level-1]={max:maximum,current:maximum-s.used};
    }
    if(maxima?.some((n,i)=>n>0&&!seen.has(i+1))){warn(`${label}: consumi mancanti per un livello della tabella, importazione slot bloccata.`);return null;}
    return result;
  };
  const sole=array(c.classes).length===1?c.classes[0]:null,rules=sole?.definition?.spellRules;
  const tableCandidate=sole?.definition?.canCastSpells===true&&rules?.multiClassSpellSlotDivisor===2&&array(c.pactMagic).every(s=>s.available===0&&s.used===0);
  const row=tableCandidate?rules.levelSpellSlots?.[sole.level]:null;
  const ordinaryTable=Array.isArray(row)&&row.length===9&&row.every(n=>integer(n,0,9))?row:null;
  if(tableCandidate&&!ordinaryTable)warn('Tabella slot della classe non valida: slot ordinari da configurare.');
  const slots=tableCandidate&&!ordinaryTable?null:slotArray(c.spellSlots,'Slot ordinari',ordinaryTable),pact=slotArray(c.pactMagic,'Slot del patto');
  if(slots&&ordinaryTable)evidence.push(`Slot ordinari: massimi dalla tabella della classe al livello ${sole.level}; consumi dal registro sorgente.`);
  state.slots=slots??state.slots;ext.pactSlots=pact??ext.pactSlots;
  const spellEntries=[];
  for(const group of array(c.classSpells)){
    const owner=array(c.classes).find(cl=>cl.id===group.characterClassId);
    for(const s of array(group.spells))spellEntries.push({s,group:'class',owner,ownerId:group.characterClassId});
  }
  for(const [group,entries] of Object.entries(c.spells??{}))for(const s of array(entries)){
    if(group==='class'&&spellEntries.some(entry=>String(entry.s.id)===String(s.id)))continue;
    spellEntries.push({s,group,owner:null,ownerId:s.componentId});
  }
  const spellIds=new Set(),casters=[];
  for(const {s,group,owner,ownerId} of spellEntries){
    const d=s?.definition;if(!d||typeof d.name!=='string'){warn('Magia priva di definizione: non importata.');continue;}
    const identity=`${group}:${ownerId}:${s.id??d.id}`;if(spellIds.has(identity))continue;spellIds.add(identity);
    const stat=s.spellCastingAbilityId??s.spellcastingAbilityId??owner?.definition?.spellCastingAbilityId??owner?.subclassDefinition?.spellCastingAbilityId;
    const resource=getResource(s.limitedUse,d.name);
    const usesSlot=s.usesSpellSlot!==false&&d.level>0;
    const validCaster=integer(stat,1,6);
    if(!validCaster)warn(`${d.name}: caratteristica da incantatore non risolta, magia conservata come nota.`);
    if(!integer(d.level,0,9)||!validCaster||(s.limitedUse&&!resource)||(usesSlot&&(!slots||!pact))){
      addPower({id:`ddb-spell-note-${powers.length+1}`,type:'feature',name:d.name,description:text(d.description,d.name),passive:true});continue;
    }
    const ability=abilityKeys[stat-1];casters.push(ability);
    let attackModifier=0,saveModifier=0,unresolved=false;
    for(const m of mods){
      if(!['spell-attacks','spell-save-dc'].includes(m.subType))continue;
      if(m.restriction||m.type!=='bonus'||m.statId!=null||!integer(m.value,-15,30)){unresolved=true;continue;}
      if(m.subType==='spell-attacks')attackModifier+=m.value;else saveModifier+=m.value;
    }
    if(unresolved){warn(`${d.name}: bonus di lancio non risolto, magia conservata come nota.`);addPower({id:`ddb-spell-note-${powers.length+1}`,type:'feature',name:d.name,description:text(d.description,d.name),passive:true});continue;}
    addPower({id:`ddb-spell-${powers.length+1}`,type:'spell',name:d.name,description:text(d.description,d.name),level:d.level,concentration:d.concentration===true,castingAbility:ability,attackModifier,saveModifier,prepared:s.alwaysPrepared===true||s.prepared!==false&&s.isPrepared!==false,usesSpellSlot:usesSlot,...(resource?{limited:resource}:{})});
  }
  if(casters.length)state.spellcastingAbility=casters[0];
  if(powers.length>120)throw new Error('Oltre 120 magie/capacità: conversione non rappresentabile');
  state.powers=powers;
  for(const p of powers)if(/\{\{/.test(p.description))warn(`${p.name}: descrizione con formula dinamica non risolta.`);
  ext.tempHp=report.hp?.temporary??0;
  const conditionNames={1:'Blinded',2:'Charmed',3:'Deafened',4:'Exhaustion',5:'Frightened',6:'Grappled',7:'Incapacitated',8:'Invisible',9:'Paralyzed',10:'Petrified',11:'Poisoned',12:'Prone',13:'Restrained',14:'Stunned',15:'Unconscious'};
  ext.conditions=array(c.conditions).map(x=>({name:x.definition?.name??x.name??conditionNames[x.id]??`Beyond ${x.id}`,level:x.level??0}));
  state.condition=ext.conditions.some(x=>slug(x.name)==='poisoned');
  if(c.deathSaves)ext.deathSaves={successes:c.deathSaves.successCount??0,failures:c.deathSaves.failCount??0,stabilized:c.deathSaves.isStabilized??false};
  for(const cl of array(c.classes))if(cl.definition?.hitDice!=null)ext.hitDice.push({name:cl.definition.name??'Classe',die:cl.definition.hitDice,max:cl.level,used:cl.hitDiceUsed??0});
  if(c.currencies){ext.currencies=c.currencies;ext.currenciesAvailable=true;}
  for(const a of array(c.customActions)){
    addPower({id:`ddb-custom-${powers.length+1}`,type:'feature',name:a.name,description:text(a.description||a.snippet,a.name),passive:true});
    warn('Azioni personalizzate conservate come note: tiri ed effetti da configurare manualmente.');
  }
  // Custom-action notes are appended to the same list before final validation.
  state.powers=powers;
  evidence.push(`Magie: ${powers.filter(p=>p.type==='spell').length}; capacità e note: ${powers.filter(p=>p.type==='feature').length}. I contatori usano limitedUse, non limiti inventati dai testi.`);
  const omissions=[...issues,...report.conversion.omissions.filter(t=>/Oggetti personalizzati|Note personalizzate/.test(t)),
    'Effetti condizionali, armi versatili, due armi e maestrie restano al DM; Ira, Grande Maestro d’Armi 2014 e dado critico aggiuntivo riconosciuti hanno comandi dedicati.',
    'Condizioni, dadi vita e tiri morte sono conservati e modificabili; solo Avvelenato applica automaticamente svantaggio. Recupero dei dadi vita e stabilizzazione restano manuali.',
    'CA e PF massimi sono una fotografia: dopo modifiche locali vanno aggiornati manualmente. Peso, contenitori, munizioni e sincronizzazione Beyond non sono automatizzati.'
  ];
  if(omissions.length>100)throw new Error('Troppe eccezioni da rappresentare');
  ext.facts=[...facts,...issues].map(t=>t.slice(0,500));
  if(ext.facts.length>100)throw new Error('Troppe note da rappresentare');
  state.importInfo={...state.importInfo,scope:'extended-v1',limitations:omissions.length<=30?omissions:[...omissions.slice(0,29),`Altre ${omissions.length-29} eccezioni nei dettagli del personaggio.`]};
  state.title='Da D&D Beyond';
  const draft=parseBackup(serialize(state));
  const skillSummary=skills.map(s=>({id:s.id,name:s.name,rank:draft.skillRanks[s.id],...describeRoll(draft,{kind:'skill',key:s.id})}));
  const saveSummary=abilityKeys.map(key=>({id:key,proficient:draft.saveProficiencies[key],...describeRoll(draft,{kind:'save',key})}));
  const weaponSummary=weapons.map(w=>{const result=weaponStats({...draft,equippedWeaponId:w.id});return{id:w.id,name:w.name,attack:result.attackBonus,damage:`${w.diceCount}d${w.dieSides}${result.damageBonus>=0?'+':''}${result.damageBonus}`,reach:w.reach,properties:w.profile.properties}});
  const coverage=[{section:'Abilità e salvezze',status:ext.blockedChecks.length?'partial':'converted',count:skillSummary.length+saveSummary.length},{section:'Armi e attacchi',status:weaponIssues.length?'partial':'converted',count:weapons.length},{section:'Magie e capacità',status:issues.some(t=>/magia|Magia|risorsa|formula|contatore/.test(t))?'partial':'converted',count:powers.length},{section:'Stato e risorse',status:'data-only',count:ext.conditions.length+ext.hitDice.length}];
  return {...report,version:3,checks:{...report.checks,conversion:'supported-extended'},conversion:{scope:'extended-v1',complete:false,sourceCounts:report.conversion.sourceCounts,blockers:[],evidence:[...report.conversion.evidence.filter(t=>!t.startsWith('Inventario:')),...evidence],omissions,coverage},draft,canImport:true,warnings:omissions,details:{skills:skillSummary,saves:saveSummary,weapons:weaponSummary,spells:powers.filter(p=>p.type==='spell').map(p=>({id:p.id,name:p.name,level:p.level,...spellStats(draft,p)})),facts:ext.facts}};
}
