// Bounded observations only: no source character JSON, cookies or account credentials.
const int=(v,a,b)=>Number.isSafeInteger(v)&&v>=a&&v<=b;
const exact=(v,keys)=>v&&typeof v==='object'&&!Array.isArray(v)&&Object.keys(v).length===keys.length&&keys.every(k=>Object.hasOwn(v,k));
export function vitals(v){if(!exact(v,['hp','max','temp'])||!int(v.max,1,999)||!int(v.hp,0,v.max)||!int(v.temp,0,999))throw Error('PF non validi');return {hp:v.hp,max:v.max,temp:v.temp};}
export const sameVitals=(a,b)=>a&&b&&a.hp===b.hp&&a.max===b.max&&a.temp===b.temp;
export function syncRoll(v){
 if(!exact(v,['sequence','kind','label','sides','dice','mode','bonus','total','floor'])||!int(v.sequence,1,4294967295)||!int(v.kind,0,10)||typeof v.label!=='string'||!v.label.trim()||v.label.length>40||/[\x00-\x1f\x7f]/.test(v.label)||![4,6,8,10,12,20].includes(v.sides)||!Array.isArray(v.dice)||!int(v.dice.length,1,18)||v.dice.some(d=>!int(d,1,v.sides))||!int(v.mode,-1,1)||v.mode&&(v.sides!==20||v.dice.length!==2)||!int(v.bonus,-200,200)||typeof v.floor!=='boolean')throw Error('Tiro non valido');
 let total=(v.mode?v.mode>0?Math.max(...v.dice):Math.min(...v.dice):v.dice.reduce((a,b)=>a+b,0))+v.bonus;if(v.floor)total=Math.max(0,total);if(v.total!==total)throw Error('Risultato del tiro discordante');return structuredClone(v);
}
export function syncSnapshot(v){
 if(!exact(v,['version','boot','counter','epoch','name','vitals','sequence','rolls'])||v.version!==1||!/^([0-9a-f]{16})$/.test(v.boot)||!int(v.counter,1,4294967295)||!int(v.epoch,0,4294967295)||typeof v.name!=='string'||!v.name.trim()||v.name.length>48||/[\x00-\x1f\x7f]/.test(v.name)||!int(v.sequence,0,4294967295)||!Array.isArray(v.rolls)||v.rolls.length!==Math.min(20,v.sequence))throw Error('Osservazione Pocket non valida');
 const rolls=v.rolls.map(syncRoll);if(rolls.some((r,i)=>r.sequence!==v.sequence-i))throw Error('Storico incompleto o fuori ordine');return {...v,vitals:vitals(v.vitals),rolls};
}
export function brokerRoll(roll){
 const r=syncRoll(roll);
 const selected=r.mode?[r.mode>0?Math.max(...r.dice):Math.min(...r.dice)]:r.dice;
 if(r.floor&&selected.reduce((a,b)=>a+b,0)+r.bonus<0)throw Error('Minimo zero non rappresentabile dal registro Beyond');
 return {diceNotation:{constant:r.bonus,set:[{count:r.dice.length,dieType:`d${r.sides}`,operation:r.mode>0?2:r.mode<0?1:0,dice:r.dice.map(d=>({dieType:`d${r.sides}`,dieValue:d}))}]},diceNotationStr:`${r.dice.length}d${r.sides}${r.mode>0?'kh1':r.mode<0?'kl1':''}${r.bonus>=0?'+':''}${r.bonus}`,rollKind:r.mode>0?'advantage':r.mode<0?'disadvantage':'',rollType:[3,8,9].includes(r.kind)?'save':r.kind===4?'to hit':r.kind===5?'damage':[1,2].includes(r.kind)?'check':'roll',result:{constant:r.bonus,text:selected.join('+')+(r.bonus>=0?'+':'')+r.bonus,total:r.total,values:selected}};
}
