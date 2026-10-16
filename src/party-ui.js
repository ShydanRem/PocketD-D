import {heroProfile} from './party.js';
import {PartyConnection} from './party-transport.js';
import {turnOrder} from './encounter.js';
import {reachableCells} from './grid.js';

export function createPartyUI({getHero,newId,escapeHtml:esc}){
  let dialog=null,selection='wall',formError='';
  const connection=new PartyConnection({newId:()=>newId().slice(0,24),onChange:()=>{refreshLauncher();draw()}});
  const disabled=value=>value?'disabled':'';
  function refreshLauncher(){const button=document.querySelector('[data-action="party"]');if(button)button.textContent=connection.sessionId?'Apri party · '+(connection.connected?'collegato':'in attesa'):'Party locale simulato'}
  function draw(){
    if(!dialog?.open)return;
    const values=Object.fromEntries([...dialog.querySelectorAll('input')].map(input=>[input.name,input.value]));
    const detailsOpen=dialog.querySelector('details')?.open??false;
    const focus=document.activeElement?.name,scroll=dialog.scrollTop,mapScroll=dialog.querySelector('.party-map-scroll')?.scrollLeft??0;
    const c=connection,s=c.session,dm=c.isCoordinator,ready=c.connected&&!c.pending;
    const member=s?.members.find(m=>m.clientId===c.clientId),actor=s?.encounter.participants.find(p=>p.id===member?.actorId);
    const encounter=s?.encounter,planning=!encounter?.round;
    const canMove=ready&&encounter?.round&&(dm||encounter.activeId===actor?.id);
    const reachable=canMove?reachableCells({encounter}):new Map();
    const roster=encounter?turnOrder({encounter}):[];
    const stateLabel={idle:'Nessun party aperto',connecting:'Ricerca coordinatore…',connected:'Collegato',offline:'Collegamento sospeso'}[c.status];
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">IL TAVOLO CONDIVISO · SIMULAZIONE</div><h2>Party locale</h2></div><button data-party="close" aria-label="Chiudi party">×</button></div><p class="editor-intro">Finestre dello stesso browser e dello stesso indirizzo HTTP simulano più dispositivi. Iniziativa, PF e mappa del party sono separati dalle schede personali. Il coordinatore gestisce il combattimento.</p><p class="party-status" role="status">${stateLabel}${s?` · revisione <b data-party-revision>${s.revision}</b>`:''}${c.sessionId?` · ${dm?'Coordinatore':'Giocatore'}`:''}</p>
    ${!c.sessionId?`<div class="party-entry"><form data-party-form="create"><h3>Prepara il tavolo</h3><p>Entra con ${esc(getHero().name)} e diventa coordinatore.</p><button>Crea party</button></form><form data-party-form="join"><h3>Raggiungi gli altri</h3><label>Codice del party<input name="code" required pattern="[a-f0-9]{24}" minlength="24" maxlength="24" autocomplete="off" spellcheck="false"></label><button>Entra con ${esc(getHero().name)}</button></form></div>`:`<div class="party-code"><span>CODICE DEL PARTY</span><code data-party-code>${esc(c.sessionId)}</code><p>Apri un'altra finestra della demo e inserisci questo codice. Mantieni aperta la finestra del coordinatore. Ricaricandola, il party riprende dal suo stato salvato in questa scheda del browser.</p></div><div class="party-toolbar"><button data-party="${c.channel?'pause':'resume'}">${c.channel?'Sospendi collegamento':'Riconnetti'}</button><button data-party="leave">Esci dal party</button>${dm?'<button data-party="export">Scarica stato del party</button>':''}</div>
    ${!c.checkpointSaved?'<p class="storage-warning" role="alert">Salvataggio del party non disponibile: questa sessione è temporanea. Il coordinatore può scaricarne lo stato; il riavvio non è garantito.</p>':''}
    ${s?`<p>${planning?'Preparazione: iniziativa e piazzamenti sono modificabili.':`Round <b data-party-round>${encounter.round}</b> · turno di <b data-party-active>${esc(roster.find(p=>p.id===encounter.activeId)?.name??'')}</b> · movimento usato ${encounter.grid.spent}`}</p>
    ${!member?`<button data-party="retry-join" ${disabled(!ready)}>Riprova ingresso con ${esc(c.profile?.name??'eroe')}</button>`:''}
    ${actor?`<form data-party-form="initiative" class="party-inline"><label>La tua iniziativa<input name="initiative" type="number" min="-10" max="50" value="${actor.initiative}" required ${disabled(!planning)}></label><button ${disabled(!ready||!planning)}>Conferma iniziativa</button></form>`:''}
    ${dm?`<div class="party-toolbar"><button data-party="start" ${disabled(!ready||!planning||roster.length<2)}>Avvia combattimento</button><button data-party="next" ${disabled(!ready||planning)}>Turno successivo</button><button data-party="stop" ${disabled(!ready||planning)}>Termina combattimento</button></div>`:''}
    <div class="party-layout"><section><h3>Iniziativa condivisa</h3><ol class="party-roster">${roster.map(p=>`<li data-party-actor="${p.id}" class="${p.id===encounter.activeId?'party-active':''}"><div><strong>${esc(p.name)} ${p.id===actor?.id?'· tu':''}</strong><span>${p.side==='enemy'?'Avversario':'Eroe'} · iniziativa ${p.initiative} · CA ${p.armorClass}</span><b data-party-hp>${p.hp}/${p.maxHp} PF</b></div>${dm?`<div class="party-hp"><button data-party-hp="-1" data-actor="${p.id}" aria-label="Riduci PF ${esc(p.name)}" ${disabled(!ready)}>−</button><button data-party-hp="1" data-actor="${p.id}" aria-label="Aumenta PF ${esc(p.name)}" ${disabled(!ready)}>+</button></div>`:''}</li>`).join('')}</ol>
    ${dm?`<details class="party-npc"><summary>Aggiungi avversario manuale</summary><form data-party-form="npc"><label>Nome<input name="npcName" maxlength="40" required></label><div class="party-numbers"><label>PF<input name="npcHp" type="number" min="1" max="9999" value="15" required></label><label>CA<input name="npcAc" type="number" min="1" max="30" value="12" required></label><label>Iniziativa<input name="npcInitiative" type="number" min="-10" max="50" value="5" required></label></div><button ${disabled(!ready||!planning||roster.length>=12)}>Aggiungi avversario</button></form></details>`:''}</section>
    <section class="party-map-section"><h3>Mappa condivisa · 9 × 9</h3>${dm&&planning?`<label>Strumento di preparazione<select name="placement"><option value="wall">Muri: aggiungi / rimuovi</option>${roster.map(p=>`<option value="${p.id}">${esc(p.name)}</option>`).join('')}</select></label>`:''}<p class="party-map-help">${planning?(dm?'Scegli una pedina o i muri, poi tocca una casella.':'Il coordinatore prepara le posizioni.'):(canMove?'Tocca una casella evidenziata per muovere.':'Attendi il tuo turno o il coordinatore.')}</p><div class="party-map-scroll" tabindex="0" role="region" aria-label="Mappa del party, scorri per vedere tutte le colonne"><div class="party-map">${Array.from({length:81},(_,cell)=>{
      const p=roster.find(a=>encounter.grid.positions[a.id]===cell),wall=encounter.grid.walls.includes(cell);
      return `<button data-party-cell="${cell}" class="${wall?'wall':''} ${p?'token':''} ${reachable.has(cell)&&!p?'reachable':''} ${p?.id===encounter.activeId?'party-active':''}" aria-label="Colonna ${cell%9+1}, riga ${Math.floor(cell/9)+1}${p?`, ${esc(p.name)}`:wall?', muro':''}" ${disabled(!ready||!(dm&&planning)&&!(canMove&&reachable.has(cell)&&!p))}>${p?esc(p.name.slice(0,2)):wall?'▦':'·'}</button>`;
    }).join('')}</div></div></section></div>`:'<p>In attesa dello stato del coordinatore. Controlla codice, browser e indirizzo della demo; usa Riconnetti per riprovare.</p>'}`}
    <p class="party-message" role="status">${esc(formError||c.message)}</p><p class="party-footnote">Massimo 12 partecipanti, avversari inclusi. Chiudere il pannello mantiene il collegamento. Chiudere la scheda del coordinatore interrompe il party; non c'è subentro automatico. Puoi conservare lo stato scaricato come resoconto JSON. Il collegamento tra dispositivi fisici arriverà in seguito.</p>`;
    for(const input of dialog.querySelectorAll('input'))if(values[input.name]!==undefined)input.value=values[input.name];
    const details=dialog.querySelector('details');if(details)details.open=detailsOpen;
    const select=dialog.querySelector('[name="placement"]');if(select){if(![...select.options].some(o=>o.value===selection))selection='wall';select.value=selection}
    dialog.scrollTop=scroll;const map=dialog.querySelector('.party-map-scroll');if(map)map.scrollLeft=mapScroll;
    if(focus)dialog.querySelector(`[name="${focus}"]`)?.focus({preventScroll:true});
  }
  function open(){
    if(dialog?.open)return;
    dialog=document.createElement('dialog');dialog.className='editor-dialog party-dialog';
    dialog.addEventListener('close',()=>{dialog.remove();dialog=null});
    dialog.addEventListener('change',event=>{if(event.target.name==='placement')selection=event.target.value});
    dialog.addEventListener('submit',event=>{
      event.preventDefault();formError='';const form=event.target,data=new FormData(form),kind=form.dataset.partyForm;
      try{
        if(kind==='create')connection.create(heroProfile(getHero()));
        if(kind==='join')connection.join(String(data.get('code')).trim(),heroProfile(getHero()));
        if(kind==='initiative')connection.send('initiative',{initiative:Number(data.get('initiative'))});
        if(kind==='npc')connection.send('npc',{id:`n_${newId().slice(0,24)}`,name:data.get('npcName'),maxHp:Number(data.get('npcHp')),hp:Number(data.get('npcHp')),armorClass:Number(data.get('npcAc')),initiative:Number(data.get('npcInitiative')),speed:6});
      }catch(e){formError=e.message}draw();
    });
    dialog.addEventListener('click',event=>{
      event.stopPropagation();
      const button=event.target.closest('button');if(!button||button.disabled)return;
      const action=button.dataset.party;formError='';
      try{
        if(action==='close'){dialog.close();return}
        if(action==='pause')connection.pause();
        if(action==='resume')connection.resume();
        if(action==='leave'){
          if(connection.isCoordinator&&!confirm('Uscire come coordinatore termina il party. Scarica prima lo stato se vuoi conservarne un resoconto.'))return;
          connection.leave();
        }
        if(action==='retry-join')connection.send('join',connection.profile);
        if(['start','next','stop'].includes(action))connection.send(action);
        if(action==='export'){
          const url=URL.createObjectURL(new Blob([JSON.stringify(connection.session,null,2)],{type:'application/json'}));
          const link=document.createElement('a');link.href=url;link.download='PocketDD-party.json';link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
        }
        if(button.dataset.partyHp)connection.send('hp',{actorId:button.dataset.actor,delta:Number(button.dataset.partyHp)});
        if(button.dataset.partyCell!==undefined){
          const cell=Number(button.dataset.partyCell);
          if(connection.session.encounter.round)connection.send('move',{cell});
          else connection.send(selection==='wall'?'wall':'place',{cell,actorId:selection});
        }
      }catch(e){formError=e.message}if(action||button.dataset.partyCell!==undefined||button.dataset.partyHp)draw();
    });
    document.body.append(dialog);dialog.showModal();draw();
  }
  connection.restore();
  return {open,refreshLauncher};
}
