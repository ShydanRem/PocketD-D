import {initialState} from './model.js';
import {emptyExtended} from './extended.js';
export function formulaDefinition(id=2619079){
  const heal=id===2619079,sides=heal||id===2618967?8:10,count=heal?2:1;
  const die=(n,fixedValue=null)=>({diceCount:n,diceValue:sides,diceMultiplier:null,fixedValue,diceString:`${n}d${sides}`});
  const empty=()=>({higherLevelDefinitions:[],additionalAttacks:[],additionalTargets:[],areaOfEffect:[],duration:[],creatures:[],special:[],points:[],range:[]});
  const higher=empty();higher.higherLevelDefinitions=(heal?[[1,2]]:[[5,2],[11,3],[17,4]]).map(([level,n])=>({level,typeId:15,dice:die(n,0),value:null,details:''}));
  return {id,name:'Synthetic anonymous spell',level:heal?1:0,isHomebrew:false,concentration:false,sources:[{sourceId:145,sourceType:1}],scaleType:heal?'spellscale':'characterlevel',duration:{durationType:'Instantaneous',durationInterval:0,durationUnit:null},atHigherLevels:empty(),modifiers:[{type:heal?'bonus':'damage',subType:heal?'hit-points':id===2618967?'radiant':'fire',restriction:'',statId:null,requiresAttunement:false,isGranted:false,entityId:null,entityTypeId:null,bonusTypes:[],duration:null,durationUnit:null,count:1,usePrimaryStat:heal,value:null,fixedValue:null,dice:null,die:die(count),atHigherLevels:higher}]};
}
export function formulaState(){const s=initialState();s.extended=emptyExtended();s.level=11;s.abilities.CAR=18;s.powers=[{id:'formula',name:'Synthetic formula',description:'Invented test',type:'spell',level:1,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:true,formula:{kind:2,count:2,sides:8,ability:1,scaling:1,step:2,source:0,bonus:0}}];return s;}
