import {rest} from './powers.js';

const policies=['manual','2014','2024'];
export function hitDiceBudget(state,policy){
  if(!policies.includes(policy))throw new Error('Recupero dadi vita non valido');
  const pools=state.extended?.hitDice??[],used=pools.reduce((n,h)=>n+h.used,0),maximum=pools.reduce((n,h)=>n+h.max,0);
  return policy==='manual'?0:policy==='2024'?used:Math.min(used,maximum?Math.max(1,Math.floor(maximum/2)):0);
}
// The device chooses the policy explicitly. Existing PC rests remain manual for HD.
export function recoverRest(state,kind,policy='manual',regain=[]){
  if(!['short','long'].includes(kind)||!policies.includes(policy)||(kind==='short'&&policy!=='manual')||!Array.isArray(regain)||regain.length>20)throw new Error('Riposo non valido');
  const pools=state.extended?.hitDice??[];
  regain.forEach((n,i)=>{if(!Number.isInteger(n)||n<0||n>(pools[i]?.used??0)||(n&&(kind!=='long'||policy!=='2014')))throw new Error('Selezione dadi vita non valida')});
  if(regain.reduce((a,b)=>a+b,0)>hitDiceBudget(state,policy))throw new Error('Troppi dadi vita selezionati');
  const next=rest(state,kind);
  if(next.extended)next.extended={...next.extended,hitDice:pools.map((h,i)=>({...h,used:kind==='long'?(policy==='2024'?0:policy==='2014'?h.used-(regain[i]??0):h.used):h.used}))};
  return next;
}
// A supplied face makes this deterministic; entropy and animation belong to callers.
export function spendHitDie(state,index,face){
  const h=state.extended?.hitDice[index];
  if(!h||!Number.isInteger(index)||index<0||!Number.isInteger(face)||face<1||face>h.die||h.used>=h.max||state.hp<=0||state.hp>=state.maxHp)throw new Error('Dado vita non disponibile');
  const healing=Math.max(0,face+Math.floor((state.abilities.COS-10)/2));
  return {...state,hp:Math.min(state.maxHp,state.hp+healing),extended:{...state.extended,hitDice:state.extended.hitDice.map((v,i)=>i===index?{...v,used:v.used+1}:v)}};
}
