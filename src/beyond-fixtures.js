// Entirely synthetic source data. Never replace this with a user's full sheet.
export function beyondFixture(){
  const scores=values=>values.map((value,index)=>({id:index+1,value}));
  return {success:true,data:{id:123456,name:'Eroe sonda',classes:[{level:3}],stats:scores([15,14,14,12,10,8]),bonusStats:scores([0,0,0,0,0,0]),overrideStats:scores([null,null,null,null,null,null]),baseHitPoints:18,bonusHitPoints:0,overrideHitPoints:null,removedHitPoints:5,temporaryHitPoints:0,inventory:[{definition:{name:'Lama di prova'},quantity:1,equipped:true}],modifiers:{class:[],race:[],background:[],feat:[],item:[]}}};
}
