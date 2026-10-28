// Entirely synthetic source data. Never replace this with a user's full sheet.
export function beyondFixture(){
  const scores=values=>values.map((value,index)=>({id:index+1,value}));
  return {success:true,data:{id:123456,name:'Eroe sonda',classes:[{level:3}],stats:scores([15,14,14,12,10,8]),bonusStats:scores([0,0,0,0,0,0]),overrideStats:scores([null,null,null,null,null,null]),baseHitPoints:18,bonusHitPoints:0,overrideHitPoints:null,removedHitPoints:5,temporaryHitPoints:0,inventory:[{definition:{name:'Lama di prova'},quantity:1,equipped:true}],modifiers:{class:[],race:[],background:[],feat:[],item:[]}}};
}

// Synthetic mechanics case; neither numbers nor inventory come from a user.
export function beyondCoreFixture(){
  const payload=beyondFixture(), c=payload.data;
  c.name='Sonda convertita';
  c.stats.forEach((s,i)=>{s.value=[13,12,15,10,11,8][i]});
  c.baseHitPoints=20;c.removedHitPoints=4;
  c.classes=[{level:3,classFeatures:[{definition:{id:500,requiredLevel:1}},{definition:{id:501,requiredLevel:3}},{definition:{id:502,requiredLevel:20}}]}];
  c.modifiers.race=[{id:'race-a',type:'bonus',subType:'strength-score',value:1,isGranted:true},{id:'race-b',type:'bonus',subType:'constitution-score',value:2,isGranted:true}];
  c.modifiers.class=[{id:'defense',componentId:500,type:'set',subType:'unarmored-armor-class',statId:3,value:null,isGranted:true},
    {id:'choice',componentId:501,type:'bonus',subType:'choose-an-ability-score',value:1,isGranted:true},
    {id:'future',componentId:502,type:'bonus',subType:'strength-score',value:8,isGranted:true}];
  c.choices={class:[{componentId:501,type:1,optionValue:696}]};
  c.inventory=[{definition:{name:'Lama sonda',filterType:'Weapon',grantedModifiers:[]},quantity:2,equipped:true},
    {definition:{name:'Lama sonda',filterType:'Weapon',grantedModifiers:[]},quantity:1,equipped:false}];
  return payload;
}

// Synthetic extended sheet: invented text and values, including a spellcaster.
export function beyondRichFixture(){
  const p=beyondCoreFixture(),c=p.data;
  c.name='Sonda estesa';
  c.classes[0]={...c.classes[0],id:777,definition:{id:9,name:'Classe sonda',hitDice:12,spellCastingAbilityId:4},hitDiceUsed:1};
  c.classes[0].classFeatures.push({definition:{id:51,requiredLevel:1,name:'Rage',description:'Vigore di prova.',levelScales:[]},levelScale:{fixedValue:2}});
  c.modifiers.class.push({id:'pick',componentId:500,type:'proficiency',subType:'perception',isGranted:false},
    {componentId:500,type:'proficiency',subType:'strength-saving-throws',isGranted:true},
    {componentId:500,type:'proficiency',subType:'constitution-saving-throws',isGranted:true},
    {componentId:500,type:'proficiency',subType:'simple-weapons',isGranted:true},
    {componentId:500,type:'proficiency',subType:'martial-weapons',isGranted:true});
  c.choices.class.push({id:'2-pick',componentId:500,optionValue:81});
  c.choices.choiceDefinitions=[{options:[{id:81,label:'Perception'}]}];
  c.race={fullName:'Specie sonda',racialTraits:[{definition:{id:58,name:'Relentless Endurance',description:'Torna a un punto ferita, dopo verifica.'}},{definition:{id:59,name:'Savage Attacks',description:'Un dado aggiuntivo nei critici in mischia.'}}]};
  c.feats=[{definition:{id:21,name:'Great Weapon Master',description:'Opzione condizionale di prova.'}}];
  c.actions={class:[{id:'action-rage',componentId:51,name:'Rage',snippet:'Vigore {{scalevalue#signed}}.',limitedUse:{maxUses:3,numberUsed:1,resetType:2}}],race:[{id:'action-endurance',componentId:58,name:'Relentless Endurance',limitedUse:{maxUses:1,numberUsed:0,resetType:2}}]};
  c.inventory=[{quantity:2,equipped:true,definition:{name:'Lancia sonda',type:'Javelin',filterType:'Weapon',categoryId:1,attackType:1,damage:{diceCount:1,diceValue:6},damageType:'Piercing',range:30,longRange:120,properties:[{name:'Thrown'}],grantedModifiers:[]}},
    {quantity:1,equipped:true,definition:{name:'Ascia sonda',type:'Greataxe',filterType:'Weapon',categoryId:2,attackType:1,damage:{diceCount:1,diceValue:12},damageType:'Slashing',range:5,longRange:5,properties:[{name:'Heavy'},{name:'Two-Handed'}],grantedModifiers:[]}},
    {quantity:1,equipped:false,definition:{name:'Arco sonda',type:'Shortbow',filterType:'Weapon',categoryId:1,attackType:2,damage:{diceCount:1,diceValue:6},damageType:'Piercing',range:80,longRange:320,properties:[],grantedModifiers:[]}}];
  c.classSpells=[{characterClassId:777,spells:[{id:101,prepared:true,usesSpellSlot:true,definition:{id:201,name:'Luce di prova',level:1,concentration:true,description:'<p>Bagliore <b>inventato</b>.</p>'}},{id:102,prepared:false,definition:{id:202,name:'Eco di prova',level:1,concentration:false,description:'Nota inventata.'}}]}];
  c.spells={race:[{id:103,spellCastingAbilityId:6,usesSpellSlot:false,limitedUse:{maxUses:2,numberUsed:1,resetType:2},definition:{id:203,name:'Scintilla innata',level:1,concentration:false,description:'Effetto originale.'}}]};
  c.spellSlots=[{level:1,available:3,used:1}];c.pactMagic=[{level:2,available:2,used:1}];
  c.temporaryHitPoints=4;c.conditions=[{id:11,level:0},{id:4,level:1}];c.deathSaves={successCount:1,failCount:2,isStabilized:false};c.currencies={cp:1,sp:2,ep:0,gp:12,pp:0};
  return p;
}

// Invented manual action counters; no real sheet or rule text.
export function beyondManualFixture(count=5){
 const p=beyondRichFixture();
 for(let i=0;i<count;i++)p.data.actions.class.push({id:`manual-${i}`,componentId:500,name:`Riserva manuale ${i+1}`,description:'Testo sintetico.',limitedUse:{maxUses:i===2?50:4,numberUsed:i===2?15:i%4,resetType:4}});
 return p;
}

// Recognized source identities plus invented values/text; never a real character JSON.
export function beyondCannonFixture(remaining=0){
 const p=beyondManualFixture(),c=p.data,book=[{sourceId:67,sourceType:1}],cl=c.classes[0];
 cl.level=10;Object.assign(cl.definition,{id:252717,name:'Artificer',isHomebrew:false,sources:book});
 cl.subclassDefinition={id:254188,name:'Artillerist',isHomebrew:false,sources:book};
 cl.classFeatures.push({definition:{id:1138963,name:'Eldritch Cannon',requiredLevel:3,sources:book,description:'Capacità sintetica con pagamento alternativo verificato.'}});
 c.actions.class.push({id:'1038966',componentId:1138963,componentTypeId:12168134,name:'Create Eldritch Cannon',activation:{activationType:1,activationTime:null},limitedUse:{maxUses:1,numberUsed:1-remaining,resetType:2,minNumberConsumed:1,maxNumberConsumed:1,statModifierUsesId:null,useProficiencyBonus:false,resetDice:null}});
 return p;
}

// Synthetic consultation fixture, with explicit zero/unknown distinctions.
export function beyondProfileFixture(){const p=beyondCoreFixture(),c=p.data;c.modifiers.race=[];c.classes[0].definition={name:'Classe sonda',hitDice:8};c.race={fullName:'Specie sonda',weightSpeeds:{normal:{walk:30,fly:0,swim:0,climb:0,burrow:0}}};c.background={definition:{name:'Origine sonda'}};c.inventory=[{id:55,quantity:2,equipped:false,isAttuned:false,containerEntityId:c.id,definition:{name:'Corda sonda',filterType:'Adventuring Gear',grantedModifiers:[],canAttune:false,weight:5}}];return p;}
