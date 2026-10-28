// Runs in the authenticated page. Receives bounded events, never bridge credentials.
export async function pageAdapter(operation){
 const characterId=location.pathname.match(/\/characters\/([1-9]\d*)/)?.[1];
 const number=e=>{const t=e instanceof HTMLInputElement?e.value:e?.textContent?.trim();return typeof t==='string'&&/^\d{1,3}$/.test(t)?Number(t):null;};
 function panel(){const old=document.querySelector('.ct-health-manager');if(old)return old;const h=[...document.querySelectorAll('h1,h2')].find(e=>e.textContent.trim()==='HP Management');if(!h)return null;let p=h.parentElement;for(let i=0;p&&i<5;i++,p=p.parentElement)if(p.querySelectorAll('input').length>=2&&[...p.querySelectorAll('label,span')].some(e=>e.textContent.trim()==='Max'))return p;return null;}
 function fields(){const root=panel();if(!root)return null;
  const current=root.querySelector('.ct-health-manager__hp-current input, input[aria-label="Current Hit Points"]')??[...root.querySelectorAll('label')].filter(e=>['Current','Current Hit Points'].includes(e.textContent.trim())).map(e=>e.control??e.parentElement.querySelector('input')).find(e=>e&&root.contains(e));
  const temp=root.querySelector('.ct-health-manager__temp input, input[aria-label="Temporary Hit Points"], div[class*="styles_temp"] input')??[...root.querySelectorAll('label')].filter(e=>['Temp','Temporary Hit Points'].includes(e.textContent.trim())).map(e=>e.control??e.parentElement.querySelector('input')).find(e=>e&&root.contains(e));
  const max=root.querySelector('.ct-health-manager__hp-max, [data-pdd-test-max]')??[...root.querySelectorAll('label,span')].filter(e=>e.textContent.trim()==='Max').map(e=>e.parentElement.querySelector('div[class*="styles_number"],div[class*="styles_maxContainer"],input')).find(Boolean);
  const v={hp:number(current),max:number(max),temp:number(temp)};if(Object.values(v).some(n=>n===null)||v.max<1||v.hp>v.max)return null;return {current,temp,v};
 }
 const f=fields();const observation=f&&characterId?{characterId,vitals:f.v}:null;if(operation.type==='read')return observation;
 const e=operation.event;if(!e||e.characterId!==characterId)return {outcome:'conflict'};
 if(e.kind==='hp'){
  if(!f||!e.before||!e.after)return {outcome:'unsupported'};
  if(e.after.max!==e.before.max)return {outcome:'unsupported'};
  if(['hp','max','temp'].some(k=>f.v[k]!==e.before[k]))return {outcome:'conflict'};
  if(f.current.readOnly||f.current.disabled||f.temp.readOnly||f.temp.disabled)return {outcome:'unsupported'};
  const setter=Object.getOwnPropertyDescriptor(HTMLInputElement.prototype,'value').set;
  for(const [key,input] of [['temp',f.temp],['hp',f.current]]){if(e.before[key]===e.after[key])continue;const live=fields();if(!live||live.v.max!==e.before.max)return {outcome:'uncertain'};input.focus();setter.call(input,String(e.after[key]));input.dispatchEvent(new Event('input',{bubbles:true}));input.dispatchEvent(new Event('change',{bubbles:true}));input.blur();await new Promise(r=>setTimeout(r,400));}
  await new Promise(r=>setTimeout(r,700));const after=fields();return {outcome:after&&['hp','max','temp'].every(k=>after.v[k]===e.after[k])?'dom-updated':'uncertain'};
 }
 if(e.kind!=='roll'||!operation.roll)return {outcome:'unsupported'};
 const broker=window[Symbol.for('@dndbeyond/message-broker-lib')];if(!broker||typeof broker.dispatch!=='function'||typeof broker.subscribe!=='function'||!/^\d+$/.test(String(broker.userId)))return {outcome:'unsupported'};
 const game=String(broker.gameId??'0');if(e.scope==='game'&&(!/^\d+$/.test(game)||game==='0'))return {outcome:'unsupported'};
 const messageScope=e.scope==='game'?'gameId':'userId',messageTarget=e.scope==='game'?game:String(broker.userId),roll=operation.roll;
 const message={id:e.remoteId,dateTime:String(Date.now()),source:'PocketDD',persist:true,userId:String(broker.userId),entityId:characterId,entityType:'character',messageScope,messageTarget,...(e.scope==='game'?{gameId:game}:{}),eventType:'dice/roll/fulfilled',data:{rollId:e.remoteId,action:e.roll.label,context:{entityId:characterId,entityType:'character',messageScope,messageTarget},rolls:[roll]}};
 const key='__pocketdd_roll_receipts_v1__',cache=window[key]??(window[key]=new Map());if(cache.has(e.id))return {outcome:cache.get(e.id)};
 cache.set(e.id,'uncertain');if(cache.size>256)cache.delete(cache.keys().next().value);
 // Subscribe once per page and keep only bounded receipts. No dispatch interception.
 const registry='__pocketdd_roll_watch_v1__';if(!window[registry]){const watch={pending:new Map()};window[registry]=watch;broker.subscribe(m=>{const p=watch.pending.get(m?.data?.rollId);if(m?.eventType==='dice/roll/fulfilled'&&p&&JSON.stringify(m.data.rolls)===p.rolls){watch.pending.delete(m.data.rollId);p.resolve();}});}
 const received=await new Promise(resolve=>{const pending=window[registry].pending;const timer=setTimeout(()=>{pending.delete(e.remoteId);resolve(false)},4000);pending.set(e.remoteId,{rolls:JSON.stringify([roll]),resolve:()=>{clearTimeout(timer);resolve(true)}});try{broker.dispatch(message)}catch{clearTimeout(timer);pending.delete(e.remoteId);resolve(false)}});
 const outcome=received?'broker-observed':'uncertain';cache.set(e.id,outcome);return {outcome};
}
