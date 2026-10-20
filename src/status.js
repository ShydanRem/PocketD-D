// Base death-save rules only. Modifiers, optional rules and damage at zero need DM input.
export function deathReady(state){const d=state.extended?.deathSaves;return Boolean(d&&state.hp===0&&!d.stabilized&&d.successes<3&&d.failures<3)}
export function deathChange(state,action){
  const old=state.extended?.deathSaves;if(!old||!Number.isInteger(action)||action<0||action>6)throw new Error('Registro morte non disponibile');
  let d={...old};
  if(action===0)d={successes:0,failures:0,stabilized:false};
  else if(action===5||action===6){const key=action===5?'successes':'failures';if(!d[key])throw new Error('Contatore a zero');d[key]--;d.stabilized=false}
  else {
    if(state.hp!==0||d.failures===3)throw new Error('Correzione DM necessaria');
    if(action===1)d={successes:0,failures:0,stabilized:true};
    else if(action===2){if(!deathReady(state))throw new Error('Tiro non disponibile');if(++d.successes===3)d={successes:0,failures:0,stabilized:true}}
    else {d.stabilized=false;d.failures=Math.min(3,d.failures+(action===4?2:1))}
  }
  return {...state,extended:{...state.extended,deathSaves:d}};
}
export function deathFace(state,face){
  if(!deathReady(state)||!Number.isInteger(face)||face<1||face>20)throw new Error('Tiro morte non disponibile');
  if(face===20)return {...state,hp:1,extended:{...state.extended,deathSaves:{successes:0,failures:0,stabilized:false}}};
  return deathChange(state,face===1?4:face>=10?2:3);
}
export function conditionChange(state,index,delta){
  if(!state.extended||!Number.isInteger(index)||index<0||index>=state.extended.conditions.length||!Number.isInteger(delta))throw new Error('Condizione non disponibile');
  return {...state,extended:{...state.extended,conditions:state.extended.conditions.map((c,i)=>i===index?{...c,level:Math.max(0,Math.min(6,c.level+delta))}:c)}};
}
