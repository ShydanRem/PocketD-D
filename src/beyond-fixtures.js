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
