import {GRID_SIZE,reachableCells,moveActive,placeToken,unplaceToken,toggleWall,viewportCells,centerViewport,panViewport,targetGeometry} from './grid.js';
import {participantView,turnOrder,upsertParticipant,removeParticipant,startEncounter,nextTurn,stopEncounter,adjustParticipantHp,queueDamage,applyPendingDamage} from './encounter.js';
import {upsertPower,deletePower,usePower,rest,validateMagic} from './powers.js';
import {LIBRARY_KEY,LEGACY_KEY,MAX_CHARACTERS,MAX_LIBRARY_BYTES,createLibrary,activeCharacter,updateCharacter,addCharacter,selectCharacter,removeCharacter,parseLibrary,serializeLibrary,loadLibrary,storeLibrary} from './library.js';
import {equippedWeapon,equipWeapon,adjustQuantity} from './inventory.js';
import {openInventoryManager} from './inventory-ui.js';
import './style.css';
import {initialState,changeHp,rollD20,attack,weaponDamage,useItem,weaponStats,serialize,parseBackup,abilityKeys,modifier,proficiency,skills,describeRoll,recordRoll} from './model.js';
let library,storedRaw=null,storageBlocked=false,storageWarning='',recoveryRaw=null;
try{const loaded=loadLibrary(localStorage);library=loaded.library;storedRaw=loaded.raw}
catch{
  library=createLibrary();storageBlocked=true;
  storageWarning='I dati salvati non sono leggibili. Questa sessione è temporanea: i dati precedenti restano conservati. Puoi scaricarli o importare una raccolta valida.';
  try{recoveryRaw=localStorage.getItem(LIBRARY_KEY)??localStorage.getItem(LEGACY_KEY)}catch{}
}
let state=activeCharacter(library);
let page='hero',result=null,rolling=false,undo=null,notice='',combat=null,damageResult=null,targetAc=14,packMessage='',sheetTab='abilities',rollContext={kind:'free'},undoRestoresHistory=false,actionTab='weapon',powerMessage='',targetId=null,encounterMessage='',gridMessage='',gridZoom=9,gridOrigin={x:0,y:0},dmOverride=false;
const escapeHtml=value=>String(value).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const skinColors=['#e5b181','#bb805c','#785044'];
const hairColors=['#704635','#d0b676','#222637'];
const cloakColors=[['#245a50','#398a72','#173f3a'],['#613748','#a05b70','#432739'],['#334d75','#5c7fac','#243655']];
const newId=()=>Array.from(crypto.getRandomValues(new Uint8Array(16)),byte=>byte.toString(16).padStart(2,'0')).join('');
const signed=n=>n>=0?`+${n}`:`${n}`;
function rememberUndo(restoreHistory=false){undo=structuredClone(state);undoRestoresHistory=restoreHistory}
function persistLibrary(){
  if(storageBlocked){notice='Sessione temporanea: esporta la raccolta per conservarla.';return}
  try{storedRaw=storeLibrary(localStorage,library,storedRaw);storageWarning='';notice='Raccolta salvata in questo browser'}
  catch(error){storageWarning=error.name==='StorageConflictError'?error.message:'Salvataggio non disponibile. Esporta la raccolta per conservare i progressi di questa sessione.';notice='Sessione temporanea: esporta la raccolta per conservarla.'}
}
function save(){library=updateCharacter(library,state);persistLibrary()}
function resetCharacterView(){
  page='hero';result=null;rolling=false;undo=null;undoRestoresHistory=false;combat=null;damageResult=null;
  targetId=null;targetAc=14;dmOverride=false;rollContext={kind:'free'};sheetTab='abilities';actionTab='weapon';
  packMessage='';powerMessage='';encounterMessage='';gridMessage='';gridZoom=9;gridOrigin={x:0,y:0};
}
function useLibrary(next){library=next;state=activeCharacter(library);resetCharacterView();persistLibrary()}
function downloadJson(raw,filename){
  const url=URL.createObjectURL(new Blob([raw],{type:'application/json'}));
  const link=document.createElement('a');link.href=url;link.download=filename;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
}
function characterBar(){return `<section class="character-bar" aria-label="Raccolta personaggi"><label for="character-select">PERSONAGGIO ATTIVO <span>${library.characters.length}/${MAX_CHARACTERS}</span></label><select id="character-select" ${rolling?'disabled':''}>${library.characters.map(item=>`<option value="${item.id}" ${item.id===library.activeId?'selected':''}>${escapeHtml(item.state.name)} · livello ${item.state.level}</option>`).join('')}</select><button data-action="manage-characters" ${rolling?'disabled':''}>Gestisci personaggi</button></section>${storageWarning?`<div class="storage-warning" role="alert">${escapeHtml(storageWarning)} ${recoveryRaw!==null?'<button data-action="recover-storage">Scarica dati da recuperare</button>':''}</div>`:''}`}
function sprite(avatar=state.avatar){const cloak=cloakColors[avatar.cloak];return `<svg class="sprite" viewBox="0 0 32 40" role="img" aria-label="Eroe originale in pixel art con mantello personalizzato"><g shape-rendering="crispEdges"><path fill="#182827" d="M8 37h17v2H8z"/><path fill="${cloak[0]}" d="M10 17h14v17H7v-9h3z"/><path fill="${cloak[1]}" d="M11 18h9v14H9V22z"/><path fill="${cloak[2]}" d="M20 18h4v16h-6z"/><path fill="#a36943" d="M12 29h5v8h-6v-3h1zm7 0h4v8h-5v-3h1z"/><path fill="${skinColors[avatar.skin]}" d="M12 9h10v10H12zM8 21h4v5H8zm14 0h4v5h-4z"/><path fill="${hairColors[avatar.hair]}" d="${['M11 7h12v5h-3V9h-5v3h-4zM11 12h2v4h-2z','M11 7h12v5h-3V9h-5v3h-4zM10 11h3v10h-3zM22 11h3v10h-3z','M11 8h12v2H11z'][avatar.style]}"/><path fill="#1b272a" d="M14 13h2v2h-2zm6 0h2v2h-2z"/><path fill="#bb7958" d="M16 17h4v1h-4z"/><path fill="#d8b770" d="M12 20h10v2H12zm0 7h11v2H12z"/><path fill="#b4d6d1" d="M27 13h2v15h-2z"/><path fill="#f4e6ba" d="M28 11h1v3h-1z"/><path fill="#d8b770" d="M25 27h6v2h-6z"/><path fill="#8a5c40" d="M27 29h2v4h-2z"/><path fill="#d8b770" d="M12 5h10v2H12z"/></g></svg>`}
function hero(){return `<div class="eyebrow">IL TUO PERSONAGGIO</div><h2>${escapeHtml(state.name)}<span>${escapeHtml(state.title)}</span></h2><div class="scene"><span class="moon"></span><i class="mountain one"></i><i class="mountain two"></i><div class="ruin left"></div><div class="ruin right"></div><div class="ground"></div>${sprite()}<span class="spark s1">✦</span><span class="spark s2">·</span></div><div class="hero-meta">EROE <span>·</span> LIVELLO ${state.level}</div><div class="health"><div><span>PUNTI FERITA</span><strong>${state.hp}<small> / ${state.maxHp}</small></strong></div><div class="health-track"><i style="width:${state.hp/state.maxHp*100}%"></i></div><div class="hp-actions"><button data-hp="-1" aria-label="Perdi un punto ferita">−</button><span>${state.hp===0?'A terra':state.hp<10?'Ferito':'Pronto all’avventura'}</span><button data-hp="1" aria-label="Recupera un punto ferita">+</button></div></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla</button><button data-action="condition" aria-pressed="${state.condition}">${state.condition?'● Avvelenato':'○ Condizioni'}</button></div>`}
function sheet(){
  const tabs=[['abilities','Caratt.'],['skills','Abilità'],['saves','Salvezza'],['history','Storico']];
  const header=`<div class="eyebrow">CONOSCI IL TUO EROE</div><h2>Scheda<span>Ogni tiro, il suo modificatore</span></h2><div class="sheet-tabs" role="group" aria-label="Viste della scheda">${tabs.map(([key,label])=>`<button data-sheet-tab="${key}" aria-pressed="${sheetTab===key}">${label}</button>`).join('')}</div>`;
  if(sheetTab==='abilities')return header+`<div class="stat-grid">${abilityKeys.map(key=>{const score=state.abilities[key];return `<button data-check="${key}"><span>${key}</span><strong>${signed(modifier(score))}</strong><small>${score}</small></button>`}).join('')}</div><div class="sheet-foot"><span>CLASSE ARMATURA <b>${state.armorClass}</b></span><span>COMPETENZA <b>${signed(proficiency(state.level))}</b></span></div>`;
  if(sheetTab==='history')return header+`<div class="sheet-scroll history-list">${state.history.length?state.history.map(entry=>`<article><div><strong>${escapeHtml(entry.label)}</strong><time>${new Date(entry.timestamp).toLocaleTimeString('it-IT',{hour:'2-digit',minute:'2-digit'})}</time></div><p>${entry.kind==='damage'?`<small>${entry.dice.length}d${entry.sides??8}</small> · `:''}${entry.dice.join(entry.kind==='damage'?' + ':' / ')} ${signed(entry.bonus)} = <b>${entry.total}</b> <small>${entry.mode==='advantage'?'vantaggio':entry.mode==='disadvantage'?'svantaggio':''}</small></p></article>`).join(''):'<p class="micro">Nessun tiro registrato. Inizia dalla scheda o dai dadi.</p>'}</div>`;
  const items=sheetTab==='skills'?skills.map(skill=>({key:skill.id,kind:'skill',detail:skill.ability,rank:state.skillRanks[skill.id]})):abilityKeys.map(key=>({key,kind:'save',detail:key,rank:state.saveProficiencies[key]?1:0}));
  return header+`<div class="sheet-scroll check-list">${items.map(item=>{const roll=describeRoll(state,item);return `<button data-roll-kind="${item.kind}" data-roll-key="${item.key}"><span>${roll.label}<small>${item.detail} · ${item.rank===2?'maestria':item.rank===1?'competente':'non competente'}</small></span><b>${signed(roll.bonus)}</b></button>`}).join('')}</div><p class="micro">Competenze modificabili in “Personalizza eroe”.</p>`;
}
function dice(){const info=describeRoll(state,rollContext);return `<div class="eyebrow">LA SORTE TI ATTENDE</div><h2>Un salto nel destino<span>${info.label} · d20</span></h2><div class="die ${rolling?'rolling':''}"><svg viewBox="0 0 120 120" aria-hidden="true"><path d="M60 4 110 34 104 91 60 117 10 89 5 34Z M60 4 34 42 5 34 M34 42 10 89 60 79 104 91 86 42 110 34 M34 42H86L60 79Z M60 79V117 M86 42 60 4"/></svg><strong>${rolling?'…':result?result.total:'20'}</strong></div><div class="roll-detail" aria-live="polite">${rolling?'Il destino si muove…':result?`${result.dice.join(' / ')} ${signed(result.bonus)} = ${result.total}${result.natural===20?' · 20 naturale!':result.natural===1?' · 1 naturale':''}`:'Il prossimo capitolo inizia qui.'}</div><p class="roll-source">${info.parts}${state.condition&&['check','skill'].includes(rollContext.kind)?' · Avvelenato: svantaggio':''}</p><div class="bonus"><button data-bonus="-1" ${rollContext.kind!=='free'?'disabled':''} aria-label="Riduci bonus">−</button><span>MODIFICATORE <b>${signed(info.bonus)}</b></span><button data-bonus="1" ${rollContext.kind!=='free'?'disabled':''} aria-label="Aumenta bonus">+</button></div><div class="modes">${[['disadvantage','Svant.'],['normal','Normale'],['advantage','Vant.']].map(([m,n])=>`<button data-mode="${m}" aria-pressed="${state.mode===m}">${n}</button>`).join('')}</div><div class="dice-context"><button data-action="free">Dado libero</button><span>Effettivo: ${info.mode==='normal'?'normale':info.mode==='advantage'?'vantaggio':'svantaggio'}</span></div><button class="roll-button" data-action="roll" ${rolling?'disabled':''}>${rolling?'LANCIO IN CORSO':'✧ LANCIA IL D20'}</button>`}
function actions(){const weapon=weaponStats(state);if(targetId&&!state.encounter.participants.some(item=>item.id===targetId))targetId=null;const target=targetId?participantView(state,targetId):null;const geometry=targetId?targetGeometry(state,targetId):null;const blocked=geometry?.known&&(!geometry.inRange||!geometry.visible);const tabs=`<div class="sheet-tabs"><button data-action-tab="weapon" aria-pressed="${actionTab==='weapon'}">Arma</button><button data-action-tab="powers" aria-pressed="${actionTab==='powers'}">Magie e capacità</button><button data-action-tab="encounter" aria-pressed="${actionTab==='encounter'}">Turni</button><button data-action-tab="grid" aria-pressed="${actionTab==='grid'}">Mappa</button></div>`;if(actionTab==='grid')return gridView(tabs);if(actionTab==='encounter')return encounterView(tabs);if(actionTab==='powers')return powersView(tabs);return `<div class="eyebrow">AZIONI AL TAVOLO</div><h2>${escapeHtml(weapon?.name??'Nessuna arma')}<span>${weapon?`${weapon.ability} ${signed(modifier(state.abilities[weapon.ability]))} · competenza ${weapon.proficient?signed(proficiency(state.level)):'non applicata'}`:'Equipaggia un’arma dallo Zaino'}</span></h2>${tabs}<div class="functional-content weapon-content"><label class="target-picker">Bersaglio<select id="attack-target"><option value="">CA manuale</option>${state.encounter.participants.filter(item=>item.id!=='hero').map(item=>{const view=participantView(state,item.id);return `<option value="${item.id}" ${targetId===item.id?'selected':''}>${escapeHtml(view.name)} · CA ${view.armorClass}</option>`}).join('')}</select></label><div class="geometry-info" role="status">${geometry?.known?`${geometry.distance} caselle · portata ${weapon?.reach??1} · ${geometry.visible?'linea libera':'muro sulla linea'}${geometry.inRange?'':' · fuori portata'}`:target?'Posizioni mancanti: verifica distanza e vista con il DM.':'CA manuale: distanza e vista gestite dal DM.'}</div>${blocked?`<button class="dm-override" data-action="override-attack" aria-pressed="${dmOverride}">${dmOverride?'✓ Override DM attivo per un attacco':'Il DM consente questo attacco'}</button>`:''}<div class="equipment-status">${weapon?`Attacco ${signed(weapon.attackBonus)} · danni ${weapon.diceCount}d${weapon.dieSides}${signed(weapon.damageBonus)} ${escapeHtml(weapon.damageType)}`:'Nessuna arma equipaggiata'}</div>${weapon?`<p class="weapon-breakdown">${weapon.ability} ${signed(modifier(state.abilities[weapon.ability]))} · competenza ${signed(weapon.proficient?proficiency(state.level):0)} · extra attacco ${signed(weapon.attackModifier)}<br>Danni: caratteristica ${signed(weapon.addAbilityToDamage?modifier(state.abilities[weapon.ability]):0)} · extra ${signed(weapon.damageModifier)}</p>`:''}<div class="bonus"><button data-ac="-1" ${target?'disabled':''} aria-label="Riduci CA bersaglio">−</button><span>CA BERSAGLIO <b>${target?.armorClass??targetAc}</b></span><button data-ac="1" ${target?'disabled':''} aria-label="Aumenta CA bersaglio">+</button></div><p class="micro">${state.condition?'Avvelenato: svantaggio; annulla il vantaggio.':`Modalità dadi: ${state.mode==='normal'?'normale':state.mode==='advantage'?'vantaggio':'svantaggio'}`}</p><button class="roll-button" data-action="attack" ${!weapon||(blocked&&!dmOverride)?'disabled':''}>TIRA PER COLPIRE</button><div class="combat-result" aria-live="polite">${combat?`<strong>${combat.hits?(combat.critical?'20 naturale · Critico':'Colpito'):'Mancato'}</strong><span>${combat.dice.join(' / ')} ${signed(combat.bonus)} = ${combat.total} contro CA ${combat.armorClass}${combat.natural===1?' · 1 naturale':''}${combat.override?' · Override DM':''}</span>`:'Scegli la CA indicata dal DM.'}</div>${combat?.hits?`<button class="roll-button" data-action="damage" ${damageResult?'disabled':''}>TIRA DANNI ${combat.weapon.diceCount*(combat.critical?2:1)}d${combat.weapon.dieSides}${signed(combat.weapon.damageBonus)}</button>`:''}<p class="damage-result" aria-live="polite">${damageResult?`${damageResult.dice.join(' + ')} ${signed(damageResult.bonus)} = ${damageResult.total} danni`:''}</p>${state.encounter.strike?`<div class="pending-damage"><span>${state.encounter.strike.amount} danni → ${escapeHtml(participantView(state,state.encounter.strike.targetId).name)}</span><button data-action="apply-damage" ${state.encounter.strike.applied?'disabled':''}>${state.encounter.strike.applied?'Danni applicati':'Applica danni'}</button></div>`:''}<p class="micro">Danni applicati solo su conferma. Resistenze gestite dal DM.</p></div>`}
function pack(){return `<div class="eyebrow">OGGETTI E RISORSE</div><h2>Pronto a partire<span>${state.inventory.length} oggetti · ${escapeHtml(equippedWeapon(state)?.name??'nessuna arma')}</span></h2><div class="pack-list"><button class="rest-button" data-action="manage-inventory">Gestisci inventario e armi</button>${state.inventory.map(item=>`<article class="inventory-card"><div><strong>${escapeHtml(item.name)} ×${item.quantity}</strong><small>${item.type==='weapon'?(state.equippedWeaponId===item.id?'Equipaggiata':'Arma nello zaino'):item.type==='healing'?`Cura ${item.diceCount}d${item.dieSides}+${item.healingModifier}`:item.type==='consumable'?'Consumabile · effetto manuale':'Oggetto'}</small></div>${item.notes?`<p>${escapeHtml(item.notes)}</p>`:''}<div class="inventory-controls"><button data-item-quantity="-1" data-item="${item.id}" aria-label="Riduci quantità ${escapeHtml(item.name)}" ${item.quantity===0?'disabled':''}>−</button><button data-item-quantity="1" data-item="${item.id}" aria-label="Aumenta quantità ${escapeHtml(item.name)}" ${item.quantity===999?'disabled':''}>+</button>${item.type==='weapon'?`<button data-equip-weapon="${item.id}" ${item.quantity===0?'disabled':''}>${state.equippedWeaponId===item.id?'Riponi':'Equipaggia'}</button>`:['consumable','healing'].includes(item.type)?`<button data-use-item="${item.id}" ${item.quantity===0||(item.type==='healing'&&state.hp===state.maxHp)?'disabled':''}>${item.type==='healing'?'Usa cura':'Usa'}</button>`:''}</div></article>`).join('')||'<p class="micro">Inventario vuoto. Aggiungi oggetti dalla gestione.</p>'}<div class="pack-row"><span>Slancio ${state.resolve}/1<small>Risorsa narrativa originale</small></span><button data-action="resolve" ${!state.resolve?'disabled':''}>Usa</button></div><button class="rest-button" data-action="rest-short">Riposo breve · capacità</button><button class="rest-button" data-action="rest">Riposo lungo · PF e risorse</button><p class="pack-message" aria-live="polite">${escapeHtml(packMessage)||'I riposi conservano le quantità degli oggetti.'}</p></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla ultima modifica</button></div>`}
function invalidateEquipment(){combat=null;damageResult=null;dmOverride=false;state={...state,encounter:{...state.encounter,strike:null}}}
function commitInventory(next){rememberUndo();state=next;invalidateEquipment();save();render()}
function manageInventory(){openInventoryManager({getState:()=>state,commit:commitInventory,newId,escapeHtml,abilityKeys})}
function render(){const scrolls=['.powers-scroll','.weapon-content','.encounter-scroll','.pack-list'].map(selector=>[selector,document.querySelector(selector)?.scrollTop??0]);const info={hero:['IL PERSONAGGIO','Un eroe,\nun mondo.','Ogni eroe conserva la propria storia, le risorse e il combattimento. Scegli chi portare al tavolo.','Tocca + o − per aggiornare i PF.'],sheet:['LA SCHEDA','Ogni scelta\nha un valore.','Scegli una caratteristica: prepariamo il dado con il suo modificatore.','Tocca una delle sei caratteristiche.'],dice:['IL DESTINO','Lascia parlare\ni dadi.','Un lancio, tutti i calcoli. Prova anche vantaggio e svantaggio.','Tocca Lancia o premi Spazio.'],actions:['LE AZIONI','Ogni colpo\nconta.','Gli attacchi usano l’arma equipaggiata: caratteristica, competenza, dadi e bonus configurati. Il critico raddoppia i dadi.','Scegli l’arma nello Zaino; personalizzala da Gestisci inventario.'],pack:['LO ZAINO','Porta ciò\nche serve.','Oggetti consumabili e risorse rimangono salvati. Puoi annullare l’ultima modifica e fare un backup.','Scorri lo zaino per il riposo.']}[page];document.querySelector('#app').innerHTML=`<header><a class="brand" href="./"><span class="brand-icon">✧</span> POCKET<span>D-D</span></a><span class="prototype">LABORATORIO / 001</span></header>${characterBar()}<main><section class="intro"><div class="edition"><span></span> UN PICCOLO OGGETTO. GRANDI AVVENTURE.</div><h1>Il tuo eroe. <br>Nel palmo <br><em>della mano.</em></h1><p>La tua scheda prende vita. Pixel, dadi e un pizzico di magia, ovunque ti porti la prossima sessione.</p><div class="intro-bottom"><span class="line"></span><span>UN COMPAGNO DI VIAGGIO<br><b>Progettato per il tuo tavolo.</b></span></div></section><section class="device-section" aria-label="Simulazione del dispositivo"><div class="device-top"><span>ESP32-S3 / AMOLED</span><span class="online">● STATO LOCALE</span></div><div class="device"><div class="screen"><div class="screen-status"><span>✦ POCKETD-D</span><span>DEMO</span></div><div class="page ${page}">${page==='hero'?hero():page==='sheet'?sheet():page==='actions'?actions():page==='pack'?pack():dice()}</div><nav aria-label="Sezioni del dispositivo">${[['hero','♙','Eroe'],['sheet','▤','Scheda'],['dice','◇','Dadi'],['actions','⚔︎','Azioni'],['pack','▣','Zaino']].map(([p,i,n])=>`<button data-page="${p}" aria-current="${page===p?'page':'false'}"><span>${i}</span>${n}</button>`).join('')}</nav></div></div><div class="device-bottom"><span>466 × 466 <b>·</b> ROUND TOUCH</span><span>PROTOTIPO INTERATTIVO</span></div><p class="device-notice" role="status">${escapeHtml(notice)}</p></section><aside><span class="aside-label">01 — ${info[0]}</span><h3>${info[1]}</h3><p>${info[2]}</p><div class="hint"><span>↗</span> ${info[3]}</div><p class="save-status">${escapeHtml(notice)||'Progressi salvati in questo browser.'}</p></aside></main><footer><span>FATTO DI PIXEL. MOSSO DALL’IMMAGINAZIONE.</span><div class="backup-tools"><button data-action="manage-inventory">Gestisci inventario e armi ▣</button><button data-action="manage-map">Prepara mappa ▦</button><button data-action="manage-encounter">Prepara combattimento ⚔︎</button><button data-action="manage-powers">Gestisci magie e capacità ✧</button><button data-action="edit">Personalizza eroe ✎</button><button data-action="new">Nuovo eroe +</button><button data-action="export">Esporta scheda ↓</button><button data-action="import">Aggiungi da backup ↑</button><button data-action="reset">Ricomincia ↗</button><input type="file" id="backup-file" accept=".json,application/json" hidden><input type="file" id="library-file" accept=".json,application/json" hidden></div><span>CONCEPT 0.10 <b>✦</b> NESSUN ACCOUNT RICHIESTO</span></footer>`;for(const [selector,top] of scrolls)document.querySelector(selector)?.scrollTo(0,top);
}
async function roll(){if(rolling)return;const info=describeRoll(state,rollContext);rolling=true;result=null;render();const outcome=rollD20(info.bonus,info.mode);await new Promise(r=>setTimeout(r,matchMedia('(prefers-reduced-motion: reduce)').matches?0:650));result=outcome;state=recordRoll(state,info.label,outcome,'d20',info.mode);save();rolling=false;render()}
document.addEventListener('click',e=>{const b=e.target.closest('button');if(!b||b.disabled)return;if(rolling)return;
if(b.dataset.page){page=b.dataset.page;render();return}
if(b.dataset.hp){rememberUndo();state=changeHp(state,Number(b.dataset.hp));save();if(Number(b.dataset.hp)<0&&state.concentration)notice='Danno subito: verifica con il DM il tiro di concentrazione. Nessun esito automatico.'}
if(b.dataset.sheetTab){sheetTab=b.dataset.sheetTab;render();return}
if(b.dataset.check||b.dataset.rollKind){rollContext=b.dataset.check?{kind:'check',key:b.dataset.check}:{kind:b.dataset.rollKind,key:b.dataset.rollKey};page='dice';result=null}
if(b.dataset.action==='free'){rollContext={kind:'free'};result=null}

if(b.dataset.bonus&&rollContext.kind==='free'){state.bonus=Math.max(-5,Math.min(15,state.bonus+Number(b.dataset.bonus)));result=null;save()}
if(b.dataset.mode){state.mode=b.dataset.mode;result=null;combat=null;damageResult=null;dmOverride=false;save()}
if(b.dataset.action==='condition'){rememberUndo();state.condition=!state.condition;combat=null;damageResult=null;dmOverride=false;save()}
if(b.dataset.action==='undo'&&undo!==null){state={...undo,history:undoRestoresHistory?undo.history:state.history};undo=null;gridMessage='';result=null;rollContext={kind:'free'};combat=null;damageResult=null;dmOverride=false;packMessage='Ultima modifica annullata.';save()}
if(b.dataset.action==='roll'){roll();return}
if(b.dataset.action==='reset'){
  if(!confirm('Ripristinare solo il personaggio attivo ai valori dimostrativi? Gli altri personaggi restano conservati. Esporta prima la sua scheda.'))return;
  state=initialState();gridMessage='';page='hero';rollContext={kind:'free'};result=null;undo=null;combat=null;damageResult=null;dmOverride=false;packMessage='';save();
}
if(b.dataset.action==='manage-inventory'){manageInventory();return}
if(b.dataset.equipWeapon){try{commitInventory(equipWeapon(state,state.equippedWeaponId===b.dataset.equipWeapon?null:b.dataset.equipWeapon))}catch(error){packMessage=error.message}}
if(b.dataset.itemQuantity){try{commitInventory(adjustQuantity(state,b.dataset.item,Number(b.dataset.itemQuantity)))}catch(error){packMessage=error.message}}
if(b.dataset.useItem){
  try{
    const item=state.inventory.find(entry=>entry.id===b.dataset.useItem),next=useItem(state,b.dataset.useItem);
    if(next.state!==state){commitInventory(next.state);packMessage=next.healing===null?`${item.name}: un oggetto consumato. Risolvi l’effetto con il DM.`:`${item.name}: ${next.healing} PF tirati, ${next.gained} recuperati.`}
  }catch(error){packMessage=error.message}
}
if(b.dataset.action==='resolve'&&state.resolve>0){rememberUndo();state.resolve--;packMessage='Slancio utilizzato. Annota l’effetto concordato con il DM.';save()}
if(b.dataset.action==='rest'||b.dataset.action==='rest-short'){
  const kind=b.dataset.action==='rest'?'long':'short';
  if(!confirm(kind==='long'?'Confermi il riposo lungo con il DM? Recupera PF, Slancio, slot e capacità previste. Termina la concentrazione; conserva condizioni e oggetti consumati.':'Confermi il riposo breve con il DM? Recupera solo le capacità impostate su riposo breve. Non recupera PF o slot.'))return;
  rememberUndo();state=rest(state,kind);packMessage=kind==='long'?'Riposo lungo completato.':'Capacità da riposo breve recuperate.';powerMessage=packMessage;save();
}
if(b.dataset.actionTab){actionTab=b.dataset.actionTab;render();return}
if(b.dataset.action==='manage-powers'){openPowerManager();return}
if(b.dataset.usePower){
  const power=state.powers.find(item=>item.id===b.dataset.usePower);
  if(power?.type==='spell'&&power.concentration&&state.concentration&&!confirm('Il nuovo lancio sostituisce la concentrazione attuale. Continuare?'))return;
  try{const level=Number(document.querySelector(`[data-slot-for="${power.id}"]`)?.value??0);const next=usePower(state,power.id,level);rememberUndo();state=next;powerMessage=`${power.name}: risorsa utilizzata. Risolvi l’effetto con il DM.`;save()}catch(error){powerMessage=error.message}
}
if(b.dataset.action==='end-concentration'){rememberUndo();state={...state,concentration:null};powerMessage='Concentrazione terminata.';save()}
if(b.dataset.ac){targetAc=Math.max(1,Math.min(30,targetAc+Number(b.dataset.ac)));combat=null;damageResult=null}
if(b.dataset.action==='override-attack'){dmOverride=!dmOverride}
if(b.dataset.action==='attack'){
  const geometry=targetId?targetGeometry(state,targetId):null;
  if(geometry?.known&&(!geometry.inRange||!geometry.visible)&&!dmOverride){notice='Attacco fuori portata o bloccato da un muro. Serve una decisione del DM.';render();return}
  const override=dmOverride;dmOverride=false;

  const armorClass=targetId?participantView(state,targetId).armorClass:targetAc;
  combat={...attack(state,armorClass),armorClass,targetId,override};damageResult=null;
  state={...state,encounter:{...state.encounter,strike:null}};state=recordRoll(state,`Arma: ${combat.weapon.name} · CA ${armorClass}${override?' · DM':''}`,combat,'d20',combat.mode);save();
}
if(b.dataset.action==='damage'&&combat?.hits&&!damageResult){damageResult=weaponDamage(combat);state=recordRoll(state,`${combat.critical?'Critico':'Danni'}: ${combat.weapon.name}`,damageResult,'damage');if(combat.targetId)state=queueDamage(state,combat.targetId,damageResult.total,newId());save()}
if(b.dataset.action==='manage-encounter'){openEncounterManager();return}
if(b.dataset.action==='start-encounter'||b.dataset.action==='next-turn'||b.dataset.action==='stop-encounter'){
  if(b.dataset.action==='stop-encounter'&&!confirm('Terminare il combattimento? Partecipanti e PF vengono conservati.'))return;
  try{const next=b.dataset.action==='start-encounter'?startEncounter(state):b.dataset.action==='next-turn'?nextTurn(state):stopEncounter(state);rememberUndo();state=next;combat=null;damageResult=null;dmOverride=false;gridMessage='';focusGrid();save()}catch(error){encounterMessage=error.message}
}
if(b.dataset.encounterHp){
  rememberUndo();state=adjustParticipantHp(state,b.dataset.participant,Number(b.dataset.encounterHp));save();
  if(b.dataset.participant==='hero'&&Number(b.dataset.encounterHp)<0&&state.concentration)notice='Danno subito: verifica la concentrazione con il DM.';
}
if(b.dataset.action==='apply-damage'){
  try{const next=applyPendingDamage(state);rememberUndo();state=next;save()}catch(error){notice=error.message}
}
if(b.dataset.action==='manage-map'){openMapEditor();return}
if(b.dataset.gridInspect){
  const participant=participantView(state,b.dataset.gridInspect);
  if(participant.id!=='hero'){targetId=participant.id;combat=null;damageResult=null;dmOverride=false;state={...state,encounter:{...state.encounter,strike:null}};save()}
  const geometry=participant.id!=='hero'?targetGeometry(state,participant.id):null;
  gridMessage=`${participant.name} · ${participant.hp}/${participant.maxHp} PF · CA ${participant.armorClass}${geometry?.known?` · ${geometry.distance} caselle · ${geometry.visible?'vista libera':'muro'}`:''}`;
}
if(b.dataset.action==='zoom-grid'){gridZoom=gridZoom===9?5:9;focusGrid()}
if(b.dataset.action==='center-grid')focusGrid();
if(b.dataset.pan){const [dx,dy]=b.dataset.pan.split(',').map(Number);gridOrigin=panViewport(gridOrigin,dx,dy,gridZoom)}
if(b.dataset.action==='aim-map'){actionTab='weapon'}
if(b.dataset.gridCell!==undefined){
  try{const next=moveActive(state,Number(b.dataset.gridCell));rememberUndo();state=next;combat=null;damageResult=null;dmOverride=false;gridMessage='Movimento completato.';focusGrid();save()}catch(error){gridMessage=error.message}
}
if(b.dataset.action==='export'){downloadJson(serialize(state),'pocketdd-eroe.json');notice='Scheda attiva esportata'}
if(b.dataset.action==='export-library'){downloadJson(serializeLibrary(library),'pocketdd-raccolta.json');notice='Raccolta esportata'}
if(b.dataset.action==='recover-storage'&&recoveryRaw!==null)downloadJson(recoveryRaw,'pocketdd-dati-da-recuperare.json');
if(b.dataset.action==='manage-characters'){openCharacterManager();return}
if(b.dataset.action==='import-library'){document.querySelector('#library-file').click();return}
if(b.dataset.action==='edit'||b.dataset.action==='new'){openEditor(b.dataset.action==='new');return}
if(b.dataset.action==='import'){document.querySelector('#backup-file').click();return}

render();});
document.addEventListener('keydown',e=>{if(e.code==='Space'&&page==='dice'&&e.target===document.body){e.preventDefault();roll()}});
if(storedRaw===null&&!storageBlocked)persistLibrary();render();

document.addEventListener('change',async e=>{
  if(e.target.id==='attack-target'){targetId=e.target.value||null;combat=null;damageResult=null;dmOverride=false;state={...state,encounter:{...state.encounter,strike:null}};save();render();return}

  if(e.target.id==='character-select'){
    if(rolling)return;
    useLibrary(selectCharacter(updateCharacter(library,state),e.target.value));render();document.querySelector('#character-select').focus();return;
  }
  if(!['backup-file','library-file'].includes(e.target.id))return;
  const collection=e.target.id==='library-file',file=e.target.files?.[0];if(!file)return;
  e.target.value='';
  if(file.size>(collection?MAX_LIBRARY_BYTES:250000)){notice='Importazione rifiutata: file troppo grande.';render();return}
  const sourceId=library.activeId;
  try{
    const raw=await file.text();
    if(rolling||sourceId!==library.activeId){notice='Importazione annullata: completa il tiro o ripeti dopo il cambio personaggio.';render();return}
    if(collection){
      const imported=parseLibrary(raw);
      if(!confirm(`Sostituire tutta la raccolta con i ${imported.characters.length} personaggi del backup? Esporta prima la raccolta corrente per conservarla.`))return;
      // In recovery mode the user explicitly authorizes replacement of the invalid saved data.
      if(storageBlocked){try{storedRaw=localStorage.getItem(LIBRARY_KEY);storageBlocked=false}catch{}}
      useLibrary(imported);
    }else{
      const imported=parseBackup(raw);
      if(!confirm(`Aggiungere ${imported.name} come nuovo personaggio? Quelli esistenti restano conservati.`))return;
      useLibrary(addCharacter(updateCharacter(library,state),imported,newId()));
    }
    document.querySelectorAll('dialog').forEach(dialog=>dialog.close());
  }catch(error){notice=error.message.includes('20 personaggi')?error.message:'Importazione rifiutata: backup non valido. Raccolta conservata.'}
  render();
});

function openEditor(fresh=false){
  if(fresh&&library.characters.length>=MAX_CHARACTERS){notice=`La raccolta contiene già ${MAX_CHARACTERS} personaggi`;render();return}
  const sourceId=library.activeId;
  const draft=structuredClone(fresh?initialState():state);
  if(fresh){draft.name='Nuovo eroe';draft.title='Una storia da scrivere';draft.hp=draft.maxHp}
  const dialog=document.createElement('dialog');dialog.className='editor-dialog';
  const number=(label,key,value,min,max)=>`<label>${label}<input name="${key}" type="number" min="${min}" max="${max}" value="${value}" required></label>`;
  const choice=(label,key,names)=>`<label>${label}<select name="${key}">${names.map((name,i)=>`<option value="${i}" ${draft.avatar[key]===i?'selected':''}>${name}</option>`).join('')}</select></label>`;
  const training=`<fieldset><legend>Tiri salvezza competenti</legend><div class="save-training">${abilityKeys.map(key=>`<label><input type="checkbox" name="save-${key}" ${draft.saveProficiencies[key]?'checked':''}>${key}</label>`).join('')}</div></fieldset><fieldset><legend>Abilità · verifica le scelte con il DM</legend><div class="skill-training">${skills.map(skill=>`<label>${skill.name} (${skill.ability})<select name="skill-${skill.id}">${['Non competente','Competente','Maestria'].map((label,rank)=>`<option value="${rank}" ${draft.skillRanks[skill.id]===rank?'selected':''}>${label}</option>`).join('')}</select></label>`).join('')}</div></fieldset>`;
  dialog.innerHTML=`<form id="hero-editor"><div class="editor-heading"><div><div class="eyebrow">IL TUO PROSSIMO CAPITOLO</div><h2>${fresh?'Crea il tuo eroe':'Personalizza il tuo eroe'}</h2></div><button type="button" data-close aria-label="Chiudi editor">×</button></div><p class="editor-intro">Inserimento manuale · verifica i valori con il DM. Modificare Costituzione o livello non ricalcola i PF massimi.</p><div class="editor-body"><div class="editor-preview">${sprite(draft.avatar)}<span>AVATAR PIXEL ART</span></div><div class="editor-fields"><label>Nome<input name="name" value="${escapeHtml(draft.name)}" maxlength="24" required></label><label>Titolo<input name="title" value="${escapeHtml(draft.title)}" maxlength="40"></label><div class="editor-numbers">${number('Livello','level',draft.level,1,20)}${number('PF massimi','maxHp',draft.maxHp,1,999)}${number('Classe armatura','armorClass',draft.armorClass,1,30)}</div><fieldset><legend>Caratteristiche · punteggi 1–30</legend><div class="editor-stats">${abilityKeys.map(key=>number(key,key,draft.abilities[key],1,30)).join('')}</div></fieldset>${training}<fieldset><legend>Aspetto</legend><div class="editor-appearance">${choice('Carnagione','skin',['Chiara','Media','Scura'])}${choice('Capelli','hair',['Castani','Biondi','Corvini'])}${choice('Mantello','cloak',['Bosco','Vino','Notte'])}${choice('Taglio','style',['Corto','Lungo','Rasato'])}</div></fieldset></div></div><p class="editor-error" role="alert"></p><div class="editor-controls"><button type="button" data-close>Annulla</button><button type="submit" class="roll-button">${fresh?'CREA PERSONAGGIO':'SALVA PERSONAGGIO'}</button></div></form>`;
  document.body.append(dialog);
  dialog.querySelectorAll('[data-close]').forEach(button=>button.addEventListener('click',()=>dialog.close()));
  dialog.addEventListener('close',()=>dialog.remove());
  dialog.addEventListener('input',()=>{
    for(const key of ['skin','hair','cloak','style'])draft.avatar[key]=Number(dialog.querySelector(`[name="${key}"]`).value);
    dialog.querySelector('.editor-preview .sprite').outerHTML=sprite(draft.avatar);
  });
  dialog.querySelector('form').addEventListener('submit',event=>{
    event.preventDefault();
    const data=new FormData(event.currentTarget);
    try{
      const next=parseBackup(JSON.stringify({...draft,version:9,name:data.get('name').trim(),title:data.get('title').trim(),level:Number(data.get('level')),maxHp:Number(data.get('maxHp')),hp:fresh?Number(data.get('maxHp')):Math.min(draft.hp,Number(data.get('maxHp'))),armorClass:Number(data.get('armorClass')),abilities:Object.fromEntries(abilityKeys.map(key=>[key,Number(data.get(key))])),skillRanks:Object.fromEntries(skills.map(skill=>[skill.id,Number(data.get(`skill-${skill.id}`))])),saveProficiencies:Object.fromEntries(abilityKeys.map(key=>[key,data.has(`save-${key}`)]))}));
      if(sourceId!==library.activeId)throw new Error('Il personaggio attivo è cambiato. Riapri l’editor.');
      if(fresh)useLibrary(addCharacter(updateCharacter(library,state),next,newId()));
      else{rememberUndo();state=next;resetCombatAfterEdit();save()}
      dialog.close();render();
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message.includes('20 personaggi')?error.message:'Controlla nome, caratteristiche e valori numerici.'}
  });
  dialog.showModal();
}

function powersView(tabs){
  const casting=modifier(state.abilities[state.spellcastingAbility])+proficiency(state.level);
  const concentration=state.powers.find(power=>power.id===state.concentration);
  return `<div class="eyebrow">RISORSE PERSONALI</div><h2>Magie e capacità<span>${state.spellcastingAbility} · attacco ${signed(casting)} · CD ${8+casting}</span></h2>${tabs}<div class="powers-scroll"><p class="micro">Gestione manuale · effetti e requisiti verificati dal DM</p><div class="slot-summary">${state.slots.map((slot,i)=>slot.max?`<span>${i+1}° <b>${slot.current}/${slot.max}</b></span>`:'').join('')||'Nessuno slot configurato'}</div>${concentration?`<div class="concentration">Concentrazione: ${escapeHtml(concentration.name)}<button data-action="end-concentration">Termina</button></div>`:''}${state.powers.length?state.powers.map(power=>`<article class="power-card"><strong>${escapeHtml(power.name)}</strong><p>${escapeHtml(power.description)||'Nessuna nota.'}</p><small>${power.type==='spell'?(power.level===0?'Trucchetto':`Magia di ${power.level}°`)+(power.concentration?' · Concentrazione':''):`Capacità · ${power.remaining}/${power.maxUses} · recupero ${power.recovery==='short'?'breve':power.recovery==='long'?'lungo':'manuale'}`}</small><div class="power-controls">${power.type==='spell'&&power.level>0?`<label>Slot<select data-slot-for="${power.id}" aria-label="Slot per ${escapeHtml(power.name)}">${state.slots.map((slot,i)=>i+1>=power.level&&slot.current>0?`<option value="${i+1}">${i+1}° · ${slot.current} disponibili</option>`:'').join('')}</select></label>`:''}<button data-use-power="${power.id}" ${power.type==='feature'?power.remaining===0?'disabled':'':power.concentration&&state.hp===0?'disabled':power.level>0&&!state.slots.some((slot,i)=>i+1>=power.level&&slot.current>0)?'disabled':''}>${power.type==='spell'?'Lancia':'Usa'}</button></div></article>`).join(''):'<p class="micro">La tua lista è vuota. Aggiungi voci dall’app di supporto.</p>'}<p class="power-message" role="status">${escapeHtml(powerMessage)}</p><button class="rest-button" data-action="manage-powers">Gestisci lista e slot</button></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla ultima modifica</button></div>`;
}
function openPowerManager(){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog power-manager';
  let editing=null;
  function draw(){
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">APP DI SUPPORTO</div><h2>Magie e capacità</h2></div><button data-manager-close aria-label="Chiudi gestione risorse">×</button></div><p class="editor-intro">Inserisci le tue note. Slot e recuperi sono manuali; nessun catalogo esterno. Ogni salvataggio si applica subito alla scheda.</p><form id="slots-editor"><fieldset><legend>Slot e caratteristica da incantatore</legend><label>Caratteristica<select name="casting">${abilityKeys.map(key=>`<option ${state.spellcastingAbility===key?'selected':''}>${key}</option>`).join('')}</select></label><div class="slot-editor-grid">${state.slots.map((slot,i)=>`<label>${i+1}° livello · attuali / massimi<div><input aria-label="Slot attuali livello ${i+1}" name="current-${i}" type="number" min="0" max="9" value="${slot.current}" required><input aria-label="Slot massimi livello ${i+1}" name="max-${i}" type="number" min="0" max="9" value="${slot.max}" required></div></label>`).join('')}</div><button type="submit" class="rest-button">Salva slot</button></fieldset></form><div class="manager-list">${state.powers.map(power=>`<div><span>${escapeHtml(power.name)}</span><button data-edit-power="${power.id}">Modifica</button><button data-delete-power="${power.id}">Elimina</button></div>`).join('')}</div><button data-new-power class="rest-button">+ Aggiungi magia o capacità</button><div id="power-form-container"></div><p class="editor-error" role="alert"></p><p class="manager-status" role="status"></p>`;
    if(editing)drawForm();
  }
  function drawForm(){
    const power=editing;
    dialog.querySelector('#power-form-container').innerHTML=`<form id="power-editor"><fieldset><legend>${state.powers.some(item=>item.id===power.id)?'Modifica voce':'Nuova voce'}</legend><label>Tipo<select name="type"><option value="spell" ${power.type==='spell'?'selected':''}>Magia</option><option value="feature" ${power.type==='feature'?'selected':''}>Capacità</option></select></label><label>Nome<input name="power-name" maxlength="40" value="${escapeHtml(power.name)}" required></label><label>Note personali<textarea name="description" maxlength="600" rows="3">${escapeHtml(power.description)}</textarea></label><div data-spell-fields ${power.type==='spell'?'':'hidden'}><label>Livello (0 = trucchetto)<input name="spell-level" type="number" min="0" max="9" value="${power.level??0}" required></label><label class="checkbox-label"><input name="concentration" type="checkbox" ${power.concentration?'checked':''}>Richiede concentrazione</label></div><div data-feature-fields ${power.type==='feature'?'':'hidden'}><div class="editor-numbers"><label>Utilizzi massimi<input name="max-uses" type="number" min="1" max="99" value="${power.maxUses??1}" required></label><label>Utilizzi rimasti<input name="remaining" type="number" min="0" max="99" value="${power.remaining??1}" required></label></div><label>Recupero<select name="recovery">${[['short','Riposo breve'],['long','Riposo lungo'],['manual','Solo manuale']].map(([value,label])=>`<option value="${value}" ${power.recovery===value?'selected':''}>${label}</option>`).join('')}</select></label></div><div class="editor-controls"><button type="button" data-cancel-power>Annulla modifica</button><button class="roll-button" type="submit">Salva voce</button></div></fieldset></form>`;
    toggleFields();
  }
  function toggleFields(){
    const form=dialog.querySelector('#power-editor');if(!form)return;
    const spell=form.elements.type.value==='spell';
    for(const [selector,active] of [['[data-spell-fields]',spell],['[data-feature-fields]',!spell]]){
      const section=form.querySelector(selector);section.hidden=!active;section.querySelectorAll('input,select').forEach(input=>input.disabled=!active);
    }
  }
  function message(text){dialog.querySelector('.manager-status').textContent=text}
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button)return;
    if(button.hasAttribute('data-manager-close'))dialog.close();
    if(button.hasAttribute('data-new-power')){editing={id:newId(),type:'spell',name:'',description:'',level:0,concentration:false};drawForm();dialog.querySelector('[name="power-name"]').focus()}
    if(button.dataset.editPower){editing=structuredClone(state.powers.find(power=>power.id===button.dataset.editPower));drawForm()}
    if(button.dataset.deletePower&&confirm('Eliminare questa voce? Se attiva, termina anche la concentrazione.')){rememberUndo();state=deletePower(state,button.dataset.deletePower);editing=null;save();draw();message('Voce eliminata');render()}
    if(button.hasAttribute('data-cancel-power')){editing=null;dialog.querySelector('#power-form-container').innerHTML=''}
  });
  dialog.addEventListener('change',event=>{if(event.target.name==='type')toggleFields()});
  dialog.addEventListener('submit',event=>{
    event.preventDefault();const form=event.target;const data=new FormData(form);
    try{
      let next;
      if(form.id==='slots-editor'){
        next={...state,spellcastingAbility:data.get('casting'),slots:state.slots.map((_,i)=>({current:Number(data.get(`current-${i}`)),max:Number(data.get(`max-${i}`))}))};validateMagic(next);
      }else if(form.id==='power-editor'){
        const power={id:editing.id,type:data.get('type'),name:data.get('power-name').trim(),description:data.get('description'),level:Number(data.get('spell-level')),concentration:data.has('concentration'),maxUses:Number(data.get('max-uses')),remaining:Number(data.get('remaining')),recovery:data.get('recovery')};
        next=upsertPower(state,power);
      }else return;
      rememberUndo();state=next;editing=null;save();draw();message('Risorse salvate');render();
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('close',()=>dialog.remove());draw();document.body.append(dialog);dialog.showModal();
}

function encounterView(tabs){
  const encounter=state.encounter;const order=turnOrder(state);
  const active=order.find(item=>item.id===encounter.activeId);
  return `<div class="eyebrow">COMBATTIMENTO LOCALE</div><h2>Ordine di iniziativa<span>${active?`Round ${encounter.round} · ${escapeHtml(active.name)}`:'In preparazione'}</span></h2>${tabs}<div class="encounter-scroll"><div class="turn-controls">${encounter.round?'<button data-action="next-turn">Turno successivo →</button><button data-action="stop-encounter">Termina</button>':`<button data-action="start-encounter" ${order.length<2?'disabled':''}>Inizia combattimento</button><button data-action="manage-encounter">Prepara</button>`}</div>${order.map(item=>`<article class="combatant ${item.id===encounter.activeId?'active-turn':''}"><div><strong>${escapeHtml(item.name)}${item.id==='hero'?' · tu':''}</strong><span>INI ${signed(item.initiative)} · CA ${item.armorClass}</span></div><div class="combatant-hp"><button data-encounter-hp="-1" data-participant="${item.id}" aria-label="Ferita a ${escapeHtml(item.name)}">−</button><span>${item.hp}/${item.maxHp} PF${item.hp===0?' · a terra':''}</span><button data-encounter-hp="1" data-participant="${item.id}" aria-label="Cura ${escapeHtml(item.name)}">+</button></div></article>`).join('')||'<p class="micro">Aggiungi il tuo eroe e almeno un altro partecipante dall’app di supporto.</p>'}<p class="micro">Parità: ordine di inserimento. A 0 PF il turno resta al DM.</p><p class="power-message" role="status">${escapeHtml(encounterMessage)}</p></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla ultima modifica</button></div>`;
}
function openEncounterManager(){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog encounter-manager';let editing=null;
  function draw(){
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">APP DI SUPPORTO · MASSIMO 12</div><h2>Prepara combattimento</h2></div><button data-encounter-close aria-label="Chiudi preparazione">×</button></div><p class="editor-intro">Partecipanti manuali. L’eroe è collegato alla scheda: nome, CA e PF rimangono sincronizzati. Termina il combattimento prima di modificare l’iniziativa.</p><div class="manager-list">${turnOrder(state).map(item=>`<div><span>${escapeHtml(item.name)} · INI ${signed(item.initiative)}</span><button data-edit-participant="${item.id}" ${state.encounter.round?'disabled':''}>Modifica</button><button data-remove-participant="${item.id}" ${state.encounter.round?'disabled':''}>Rimuovi</button></div>`).join('')}</div><div class="encounter-add"><button data-add-hero ${state.encounter.round||state.encounter.participants.some(item=>item.id==='hero')?'disabled':''}>+ Il mio eroe</button><button data-add-participant ${state.encounter.round?'disabled':''}>+ Partecipante</button></div><div id="participant-container"></div><p class="editor-error" role="alert"></p>`;
    if(editing)drawForm();
  }
  function drawForm(){const hero=editing.id==='hero';
    dialog.querySelector('#participant-container').innerHTML=`<form id="participant-form"><fieldset><legend>${hero?'Il tuo eroe':'Partecipante'}</legend>${hero?'':`<label>Nome<input name="participant-name" value="${escapeHtml(editing.name)}" maxlength="40" required></label><label>Schieramento<select name="side"><option value="enemy" ${editing.side==='enemy'?'selected':''}>Avversario</option><option value="ally" ${editing.side==='ally'?'selected':''}>Alleato</option></select></label>`}<label>Iniziativa<input name="initiative" type="number" min="-10" max="50" value="${editing.initiative}" required></label><label>Movimento per turno (caselle)<input name="speed" type="number" min="0" max="18" value="${editing.speed??6}" required></label>${hero?'':`<div class="editor-numbers"><label>PF attuali<input name="hp" type="number" min="0" max="9999" value="${editing.hp}" required></label><label>PF massimi<input name="maxHp" type="number" min="1" max="9999" value="${editing.maxHp}" required></label><label>CA<input name="armorClass" type="number" min="1" max="30" value="${editing.armorClass}" required></label></div>`}<div class="editor-controls"><button type="button" data-cancel-participant>Annulla</button><button class="roll-button" type="submit">Salva partecipante</button></div></fieldset></form>`;
  }
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button||button.disabled)return;
    if(button.hasAttribute('data-encounter-close'))dialog.close();
    if(button.hasAttribute('data-add-hero')){editing={id:'hero',initiative:0};drawForm()}
    if(button.hasAttribute('data-add-participant')){editing={id:newId(),name:'',initiative:0,hp:10,maxHp:10,armorClass:10,side:'enemy'};drawForm();dialog.querySelector('[name="participant-name"]').focus()}
    if(button.dataset.editParticipant){editing=structuredClone(state.encounter.participants.find(item=>item.id===button.dataset.editParticipant));drawForm()}
    if(button.hasAttribute('data-cancel-participant')){editing=null;dialog.querySelector('#participant-container').innerHTML=''}
    if(button.dataset.removeParticipant&&confirm('Rimuovere il partecipante dal combattimento?')){rememberUndo();state=removeParticipant(state,button.dataset.removeParticipant);editing=null;combat=null;damageResult=null;dmOverride=false;save();draw();render()}
  });
  dialog.addEventListener('submit',event=>{
    event.preventDefault();const data=new FormData(event.target);
    try{
      const next=upsertParticipant(state,{...editing,name:data.get('participant-name'),initiative:Number(data.get('initiative')),speed:Number(data.get('speed')),hp:Number(data.get('hp')),maxHp:Number(data.get('maxHp')),armorClass:Number(data.get('armorClass')),side:data.get('side')});
      rememberUndo();state=next;editing=null;combat=null;damageResult=null;dmOverride=false;save();draw();render();
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('close',()=>dialog.remove());draw();document.body.append(dialog);dialog.showModal();
}

function mapBoard(state,interactive=false,viewport=null){
  const encounter=state.encounter,grid=encounter.grid;
  const reachable=interactive?reachableCells(state):new Map();
  const numbered=encounter.participants.map((item,index)=>({id:item.id,number:index+1,...participantView(state,item.id)}));
  const size=viewport?.size??9,cells=viewportCells(size,viewport??{x:0,y:0});
  const sight=interactive&&targetId&&state.encounter.participants.some(item=>item.id===targetId)?targetGeometry(state,targetId).cells:[];
  return `<div class="tactical-grid" role="group" style="grid-template-columns:repeat(${size},minmax(0,1fr))" aria-label="Mappa tattica ${size} per ${size}">${cells.map(cell=>{
    const occupant=numbered.find(item=>grid.positions[item.id]===cell),wall=grid.walls.includes(cell);
    const label=`Riga ${Math.floor(cell/GRID_SIZE)+1}, colonna ${cell%GRID_SIZE+1}${wall?', muro':''}${occupant?`, ${occupant.name}`:''}`;
    return `<button type="button" ${interactive?(occupant?`data-grid-inspect="${occupant.id}"`:`data-grid-cell="${cell}" ${!reachable.has(cell)?'disabled':''}`):`data-map-cell="${cell}"`} class="map-cell ${wall?'wall':''} ${occupant?'token '+occupant.side:''} ${occupant?.id===encounter.activeId?'active-token':''} ${reachable.has(cell)&&!occupant?'reachable':''} ${sight.includes(cell)?'sight-line':''} ${occupant?.id===targetId?'selected-target':''}" aria-label="${escapeHtml(label)}" title="${escapeHtml(label)}">${occupant?occupant.number:wall?'▪':''}</button>`;
  }).join('')}</div>`;
}
function gridView(tabs){
  const active=state.encounter.activeId?participantView(state,state.encounter.activeId):null;
  const placed=active&&Object.hasOwn(state.encounter.grid.positions,active.id);
  return `<div class="eyebrow">GRIGLIA LOCALE · 9 × 9</div><h2>Il campo di battaglia<span>${active?`${escapeHtml(active.name)} · ${active.speed-state.encounter.grid.spent}/${active.speed} caselle`:'Posiziona le pedine prima di iniziare'}</span></h2>${tabs}<div class="grid-play-area">${mapBoard(state,true,{size:gridZoom,...gridOrigin})}<div class="map-camera left"><button data-action="zoom-grid" aria-label="${gridZoom===9?'Ingrandisci':'Mostra tutta la mappa'}">${gridZoom===9?'＋':'−'}</button><button data-action="center-grid" aria-label="Centra sul partecipante attivo">◎</button>${targetId&&state.encounter.participants.some(item=>item.id===targetId)?'<button data-action="aim-map" aria-label="Apri attacco al bersaglio">⚔︎</button>':''}</div>${gridZoom===5?'<div class="map-camera right"><button data-pan="0,-1" aria-label="Sposta vista in alto">↑</button><button data-pan="-1,0" aria-label="Sposta vista a sinistra">←</button><button data-pan="1,0" aria-label="Sposta vista a destra">→</button><button data-pan="0,1" aria-label="Sposta vista in basso">↓</button></div>':''}</div><div class="map-help" role="status">${escapeHtml(gridMessage)||(!active?'Prepara la mappa, poi avvia dai Turni.':!placed?'Pedina non posizionata: termina il combattimento per preparare la mappa.':active.hp===0?'A zero PF: movimento sospeso. Gestisci il turno con il DM.':'Tocca una casella evidenziata per muovere.')}</div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla</button><button data-action="manage-map">Prepara mappa</button>${active?'<button data-action="next-turn">Turno →</button>':''}</div>`;
}
function openMapEditor(){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog map-editor';let tool='wall';
  function draw(){
    const locked=state.encounter.round>0;
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">APP DI SUPPORTO</div><h2>Prepara la mappa</h2></div><button data-map-close aria-label="Chiudi mappa">×</button></div><p class="editor-intro">${locked?'Combattimento in corso: termina dai Turni per modificare la mappa.':'Scegli un partecipante e tocca una casella per posizionarlo, oppure aggiungi e rimuovi muri. Ogni modifica viene salvata subito.'}</p><div class="map-editor-body"><div class="map-editor-board">${mapBoard(state)}</div><div><label class="map-tool-label">Strumento<select id="map-tool" ${locked?'disabled':''}><option value="wall">Aggiungi/rimuovi muro</option>${state.encounter.participants.map(item=>`<option value="token:${item.id}" ${tool===`token:${item.id}`?'selected':''}>${escapeHtml(participantView(state,item.id).name)}</option>`).join('')}</select></label><div class="map-legend">${state.encounter.participants.map((item,index)=>`<div><span>${index+1}. ${escapeHtml(participantView(state,item.id).name)} · ${Object.hasOwn(state.encounter.grid.positions,item.id)?'posizionato':'da posizionare'}</span><button data-unplace="${item.id}" ${locked||!Object.hasOwn(state.encounter.grid.positions,item.id)?'disabled':''}>Rimuovi dalla mappa</button></div>`).join('')||'<p>Aggiungi prima i partecipanti in Prepara combattimento.</p>'}</div></div></div><p class="editor-error" role="alert"></p><p class="manager-status" role="status"></p><p class="editor-intro">Un passo, anche diagonale, costa una casella. Non si attraversano muri, altre pedine o angoli bloccati. Velocità configurabile nella preparazione del combattimento.</p>`;
    if(locked)dialog.querySelectorAll('[data-map-cell]').forEach(button=>button.disabled=true);
  }
  dialog.addEventListener('change',event=>{if(event.target.id==='map-tool')tool=event.target.value});
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button||button.disabled)return;
    if(button.hasAttribute('data-map-close')){dialog.close();return}
    try{
      let next;
      if(button.dataset.mapCell!==undefined)next=tool==='wall'?toggleWall(state,Number(button.dataset.mapCell)):placeToken(state,tool.slice(6),Number(button.dataset.mapCell));
      else if(button.dataset.unplace)next=unplaceToken(state,button.dataset.unplace);
      else return;
      rememberUndo();state=next;combat=null;damageResult=null;dmOverride=false;save();draw();dialog.querySelector('.manager-status').textContent='Mappa salvata';render();
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('close',()=>dialog.remove());draw();document.body.append(dialog);dialog.showModal();
}

function focusGrid(){
  const positions=state.encounter.grid.positions;
  const cell=positions[state.encounter.activeId]??positions.hero??40;
  gridOrigin=centerViewport(cell,gridZoom);
}

function resetCombatAfterEdit(){invalidateEquipment();result=null;rollContext={kind:'free'};packMessage='';page='hero'}

function openCharacterManager(){
  const dialog=document.createElement('dialog');dialog.className='editor-dialog character-manager';
  function draw(){
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">LE TUE AVVENTURE · ${library.characters.length}/${MAX_CHARACTERS}</div><h2>I tuoi personaggi</h2></div><button data-library-close aria-label="Chiudi personaggi">×</button></div><p class="editor-intro">Ogni eroe conserva PF, risorse, storico e combattimento. I personaggi sono salvati in questo browser: esporta la raccolta per trasferirli o conservarne una copia.</p><div class="character-list">${library.characters.map(({id,state:hero})=>`<article class="character-card ${id===library.activeId?'current-character':''}"><div class="character-portrait">${sprite(hero.avatar)}</div><div class="character-details"><strong>${escapeHtml(hero.name)}</strong><p>Livello ${hero.level} · ${hero.hp}/${hero.maxHp} PF${hero.encounter.round?` · combattimento, round ${hero.encounter.round}`:''}</p><small>${id===library.activeId?'Personaggio attivo':escapeHtml(hero.title)}</small><div class="character-controls"><button data-select-character="${id}" ${id===library.activeId?'disabled':''}>Usa</button><button data-copy-character="${id}" ${library.characters.length>=MAX_CHARACTERS?'disabled':''}>Duplica</button><button data-delete-character="${id}" ${library.characters.length===1?'disabled':''}>Elimina</button></div></div></article>`).join('')}</div><div class="library-tools"><button data-library-new ${library.characters.length>=MAX_CHARACTERS?'disabled':''}>+ Nuovo personaggio</button><button data-action="import">Aggiungi da backup</button><button data-action="export-library">Esporta raccolta</button><button data-action="import-library">Ripristina raccolta</button></div><p class="editor-error" role="alert"></p>`;
  }
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button||button.disabled||rolling)return;
    try{
      if(button.hasAttribute('data-library-close')){dialog.close();return}
      if(button.hasAttribute('data-library-new')){dialog.close();openEditor(true);return}
      if(button.dataset.selectCharacter){useLibrary(selectCharacter(updateCharacter(library,state),button.dataset.selectCharacter));dialog.close();render();return}
      if(button.dataset.copyCharacter){
        const original=library.characters.find(item=>item.id===button.dataset.copyCharacter).state;
        const copy={...original,name:(original.name.slice(0,16)+' · copia').slice(0,24)};
        useLibrary(addCharacter(updateCharacter(library,state),copy,newId()));draw();render();
      }
      if(button.dataset.deleteCharacter){
        const entry=library.characters.find(item=>item.id===button.dataset.deleteCharacter);
        if(!confirm(`Eliminare ${entry.state.name} e tutti i suoi progressi? Esporta prima una copia: l’eliminazione non si può annullare.`))return;
        const next=removeCharacter(updateCharacter(library,state),entry.id);
        if(next.activeId===library.activeId){library=next;persistLibrary()}else useLibrary(next);
        draw();render();
      }
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('close',()=>dialog.remove());draw();document.body.append(dialog);dialog.showModal();
}
