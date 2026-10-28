const hp=v=>v?`${v.hp}/${v.max} (+${v.temp} temporanei)`:'non disponibile';
const $=s=>document.querySelector(s),csrf=$('[name=pdd-csrf]').content;
const call=async(path,data={})=>{const r=await fetch(path,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({csrf,...data})});const b=await r.json();if(!r.ok)throw Error(b.error??'Ponte non disponibile');return b;};
const act=async fn=>{try{$('#error').textContent='';await fn()}catch(e){$('#error').textContent=e.message}};
$('#extension').onclick=()=>act(async()=>$('#code').textContent=(await call('/api/extension-code')).code);
$('#bind').onsubmit=e=>{e.preventDefault();act(async()=>{const f=new FormData(e.target);$('#bundle').value=(await call('/api/bind',Object.fromEntries(f))).bundle;await refresh()})};
$('#copy').onclick=()=>act(async()=>{await navigator.clipboard.writeText($('#bundle').value);$('#error').textContent='Configurazione copiata. Incollala solo nel portale del tuo Pocket.'});
$('#archive').onclick=()=>act(async()=>{await call('/api/archive');await refresh()});
async function refresh(){const s=await call('/api/status');$('#fingerprint').textContent=`Impronta ponte ${s.ip}:${s.devicePort}: ${s.fingerprint}`;
 $('#bindings').replaceChildren(...s.bindings.map(b=>{const root=document.createElement('article'),p=document.createElement('p');p.textContent=`Beyond ${b.characterId} · ${b.snapshot?.name??'Pocket non collegato'} · ${b.status.state} · ${b.status.queued} in coda. Pocket ${hp(b.snapshot?.vitals)} / Beyond ${hp(b.remote)}`;root.append(p);for(const [label,path] of [['Avvia / riprendi dopo verifica','approve'],['Pausa','pause'],['Ho verificato: riconcilia senza reinviare','reconcile'],['Revoca','revoke']]){const button=document.createElement('button');button.textContent=label;button.onclick=()=>act(async()=>{if(['reconcile','revoke'].includes(path)&&!confirm('Confermi? Gli eventi non saranno reinviati.'))return;await call('/api/'+path,{id:b.id});await refresh()});root.append(button)}return root}));
 $('#events').replaceChildren(...s.events.slice(-50).reverse().map(e=>{const li=document.createElement('li');li.textContent=`${e.kind==='roll'?`${e.label}: ${e.total}`:`PF ${hp(e.before)} → ${hp(e.after)}`} · ${e.status}`;return li}));}
setInterval(()=>act(refresh),2000);act(refresh);
