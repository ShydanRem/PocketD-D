import {damageNames,defenceFlags,defenceLabel} from './defences.js';
import {avatarClasses,avatarOutfit} from './avatar-classes.js';
import {loadClassLooks,classLooksAvailable} from './class-looks.js';
import {avatarIdentity,avatarSpecies,avatarBodies} from './avatar-species.js';
import {projectDevice,serializeDevice} from './device-export.js';
import {sendDeviceCopy} from './device-usb.js';
import {bindUsbDialog,usbStatusMarkup} from './device-usb-ui.js';
import {openDeviceBackup} from './device-backup-ui.js';
import {deviceAvatarSvg} from './device-avatar.js';
import {importedSummary,openExtendedState} from './extended-ui.js';
import {setCombatOption,emptyExtended} from './extended.js';
import {openBeyondImport,showBeyondLimitations} from './beyond-ui.js';
import './beyond.css';
import {GRID_SIZE,reachableCells,moveActive,placeToken,unplaceToken,toggleWall,viewportCells,centerViewport,panViewport,targetGeometry} from './grid.js';
import {participantView,turnOrder,upsertParticipant,removeParticipant,startEncounter,nextTurn,stopEncounter,adjustParticipantHp,queueDamage,applyPendingDamage} from './encounter.js';
import {upsertPower,deletePower,usePower,rest,validateMagic,spellStats,availableSlots} from './powers.js';
import {LIBRARY_KEY,LEGACY_KEY,MAX_CHARACTERS,MAX_LIBRARY_BYTES,createLibrary,activeCharacter,updateCharacter,addCharacter,selectCharacter,removeCharacter,parseLibrary,serializeLibrary,loadLibrary,storeLibrary} from './library.js';
import {equippedWeapon,equipWeapon,adjustQuantity} from './inventory.js';
import {openInventoryManager} from './inventory-ui.js';
import {avatarSvg,avatarDescription,avatarPresets,skinColors,hairColors,cloakColors} from './avatar.js';
import {createPartyUI} from './party-ui.js';
import './style.css';
import './interface.css';
import './party.css';
import {initialState,changeHp,rollD20,attack,weaponDamage,useItem,weaponStats,serialize,parseBackup,abilityKeys,modifier,proficiency,skills,skillAbility,describeRoll,recordRoll} from './model.js';
await loadClassLooks();
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
const newId=()=>Array.from(crypto.getRandomValues(new Uint8Array(16)),byte=>byte.toString(16).padStart(2,'0')).join('');
const party=createPartyUI({getHero:()=>state,newId,escapeHtml});
const signed=n=>n>=0?`+${n}`:`${n}`;
const sections=[['hero','Eroe'],['sheet','Scheda'],['dice','Dadi'],['actions','Azioni'],['pack','Zaino']];
const scrollSelectors=['.powers-scroll','.weapon-content','.encounter-scroll','.pack-list','.sheet-scroll'];
const scrollPositions=new Map();let lastRenderedView=null,focusMode=false;
const viewKey=()=>page==='sheet'?`sheet:${sheetTab}`:page==='actions'?`actions:${actionTab}`:page;
function navIcon(id){const paths={hero:'M8 21v-5a4 4 0 0 1 8 0v5M12 3a4 4 0 1 0 0 8 4 4 0 0 0 0-8Z',sheet:'M7 3h12v18H5V7h14M9 11h6M9 15h6',dice:'m12 2 9 5v10l-9 5-9-5V7l9-5Zm0 0L7 9l5 7 5-7-5-7ZM3 7l4 2h10l4-2M3 17l9-1 9 1M12 16v6',actions:'m4 3 12 12M3 3l1 6 5-5-6-1Zm17 0L8 15m13-12-1 6-5-5 6-1ZM4 15l5 5m6-5 5 5M3 21l4-4m10 0 4 4',pack:'M5 9h14l2 12H3L5 9Zm3 0V6a4 4 0 0 1 8 0v3M8 13h8v5H8z'};return `<svg viewBox="0 0 24 24" aria-hidden="true"><path d="${paths[id]}"/></svg>`}
function navigatePage(next,source='screen'){
  if(rolling||!sections.some(([id])=>id===next))return;
  page=next;render();
  const selector=source==='quick'?`[data-go-page="${next}"]`:source==='tabs'?`[data-page="${next}"]`:'.screen';
  document.querySelector(selector)?.focus({preventScroll:true});
}
function quickNavigation(){return `<div class="quick-navigation" role="group" aria-label="Navigazione rapida">${sections.map(([id,label])=>`<button data-go-page="${id}" aria-current="${page===id?'page':'false'}" ${rolling?'disabled':''}>${navIcon(id)}<span>${label}</span></button>`).join('')}</div>`}
function supportTools(){return `<section class="support-tools" aria-label="Strumenti del tavolo"><div class="tools-heading"><span class="eyebrow">IL TUO TAVOLO</span><h2>Prepara la prossima avventura.</h2><p>La gestione della scheda, a portata di mano.</p></div><div class="tool-group"><h3>Il personaggio</h3><button data-action="edit">Personalizza eroe</button><button data-action="new">Nuovo eroe +</button><button data-action="beyond-import">Importa da D&D Beyond</button></div><div class="tool-group"><h3>Al tavolo</h3><button data-action="party">Party locale simulato</button><button data-action="manage-inventory">Gestisci inventario e armi</button><button data-action="manage-powers">Gestisci magie e capacità</button><button data-action="manage-encounter">Prepara combattimento</button><button data-action="manage-map">Prepara mappa</button></div><div class="tool-group"><h3>I tuoi salvataggi</h3><button data-action="device-export">Prepara per dispositivo (USB)</button><button data-action="device-backup">Backup del dispositivo</button><button data-action="export">Esporta scheda</button><button data-action="import">Aggiungi da backup</button><button data-action="reset">Ripristina eroe dimostrativo</button></div></section><footer><span>FATTO DI PIXEL. MOSSO DALL’IMMAGINAZIONE.</span><span>SUPPORTO PC 0.74 <b>✦</b> IMPORTAZIONE E CONFIGURAZIONE</span><input type="file" id="backup-file" accept=".json,application/json" hidden><input type="file" id="library-file" accept=".json,application/json" hidden></footer>`}

function rememberUndo(restoreHistory=false){undo=structuredClone(state);undoRestoresHistory=restoreHistory}
function persistLibrary(){
  if(storageBlocked){notice='Sessione temporanea: esporta la raccolta per conservarla.';return}
  try{storedRaw=storeLibrary(localStorage,library,storedRaw);storageWarning='';notice='Raccolta salvata in questo browser'}
  catch(error){storageWarning=error.name==='StorageConflictError'?error.message:'Salvataggio non disponibile. Esporta la raccolta per conservare i progressi di questa sessione.';notice='Sessione temporanea: esporta la raccolta per conservarla.'}
}
function save(){library=updateCharacter(library,state);persistLibrary()}
function resetCharacterView(){
  scrollPositions.clear();lastRenderedView=null;
  page='hero';result=null;rolling=false;undo=null;undoRestoresHistory=false;combat=null;damageResult=null;
  targetId=null;targetAc=14;dmOverride=false;rollContext={kind:'free'};sheetTab='abilities';actionTab='weapon';
  packMessage='';powerMessage='';encounterMessage='';gridMessage='';gridZoom=9;gridOrigin={x:0,y:0};
}
function useLibrary(next){library=next;state=activeCharacter(library);resetCharacterView();persistLibrary()}
function commitBeyond(draft){
  if(storageBlocked)throw new Error('Risolvi prima il recupero dei dati salvati');
  const candidate=parseBackup(serialize(draft));
  if(!candidate.importInfo)throw new Error('Provenienza della scheda assente');
  const next=addCharacter(updateCharacter(library,state),candidate,newId());
  // Persist first: quota failures and stale tabs must not add a temporary duplicate.
  const raw=storeLibrary(localStorage,next,storedRaw);
  library=next;storedRaw=raw;state=activeCharacter(library);resetCharacterView();storageWarning='';
  notice='Nuovo personaggio Beyond salvato. Schede precedenti conservate; completa i dati omessi.';render();
}
function downloadJson(raw,filename){
  const url=URL.createObjectURL(new Blob([raw],{type:'application/json'}));
  const link=document.createElement('a');link.href=url;link.download=filename;link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
}
function characterBar(){return `<section class="character-bar" aria-label="Raccolta personaggi"><label for="character-select">PERSONAGGIO ATTIVO <span>${library.characters.length}/${MAX_CHARACTERS}</span></label><select id="character-select" ${rolling?'disabled':''}>${library.characters.map(item=>`<option value="${item.id}" ${item.id===library.activeId?'selected':''}>${escapeHtml(item.state.name)} · livello ${item.state.level}</option>`).join('')}</select><button data-action="manage-characters" ${rolling?'disabled':''}>Gestisci personaggi</button></section>${storageWarning?`<div class="storage-warning" role="alert">${escapeHtml(storageWarning)} ${recoveryRaw!==null?'<button data-action="recover-storage">Scarica dati da recuperare</button>':''}</div>`:''}`}
function sprite(avatar=state.avatar,armed=Boolean(equippedWeapon(state))){return avatarSvg(avatar,{armed})}
function hero(){return `<div class="eyebrow">IL TUO PERSONAGGIO</div><h2>${escapeHtml(state.name)}<span>${escapeHtml(state.title)}</span></h2><div class="scene"><span class="moon"></span><i class="mountain one"></i><i class="mountain two"></i><div class="ruin left"></div><div class="ruin right"></div><div class="ground"></div>${sprite()}<span class="spark s1">✦</span><span class="spark s2">·</span></div><div class="hero-meta"><span>LIVELLO <b>${state.level}</b></span><span class="hero-armor">CA <b>${state.armorClass}</b></span></div><div class="health"><div><span>PUNTI FERITA</span><strong>${state.hp}<small> / ${state.maxHp}</small></strong></div><div class="health-track" role="progressbar" aria-label="Punti ferita" aria-valuemin="0" aria-valuemax="${state.maxHp}" aria-valuenow="${state.hp}"><i style="width:${state.hp/state.maxHp*100}%"></i></div><div class="hp-actions"><button data-hp="-1" aria-label="Perdi un punto ferita">−</button><span>${state.hp===0?'A terra':state.hp<10?'Ferito':'Pronto all’avventura'}</span><button data-hp="1" aria-label="Recupera un punto ferita">+</button></div></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla</button><button data-action="condition" aria-pressed="${state.condition}">${state.condition?'● Avvelenato':'○ Condizioni'}</button></div>`}
function sheet(){
  const tabs=[['abilities','Caratt.'],['skills','Abilità'],['saves','Salvezza'],['history','Storico']];
  const header=`<div class="eyebrow">CONOSCI IL TUO EROE</div><h2>Scheda<span>Ogni tiro, il suo modificatore</span></h2><div class="sheet-tabs" role="group" aria-label="Viste della scheda">${tabs.map(([key,label])=>`<button data-sheet-tab="${key}" aria-pressed="${sheetTab===key}">${label}</button>`).join('')}</div>`;
  if(sheetTab==='abilities')return header+`<div class="stat-grid">${abilityKeys.map(key=>{const score=state.abilities[key];return `<button data-check="${key}"><span>${key}</span><strong>${signed(modifier(score))}</strong><small>${score}</small></button>`}).join('')}</div><div class="sheet-foot"><span>CLASSE ARMATURA <b>${state.armorClass}</b></span><span>COMPETENZA <b>${signed(proficiency(state.level))}</b></span></div>`;
  if(sheetTab==='history')return header+`<div class="sheet-scroll history-list">${state.history.length?state.history.map(entry=>`<article><div><strong>${escapeHtml(entry.label)}</strong><time>${new Date(entry.timestamp).toLocaleTimeString('it-IT',{hour:'2-digit',minute:'2-digit'})}</time></div><p>${entry.kind==='damage'?`<small>${entry.dice.length}d${entry.sides??8}</small> · `:''}${entry.dice.join(entry.kind==='damage'?' + ':' / ')} ${signed(entry.bonus)} = <b>${entry.total}</b> <small>${entry.mode==='advantage'?'vantaggio':entry.mode==='disadvantage'?'svantaggio':''}</small></p></article>`).join(''):'<p class="micro">Nessun tiro registrato. Inizia dalla scheda o dai dadi.</p>'}</div>`;
  const items=sheetTab==='skills'?skills.map(skill=>({key:skill.id,kind:'skill',detail:skillAbility(state,skill),rank:state.skillRanks[skill.id]})):abilityKeys.map(key=>({key,kind:'save',detail:key,rank:state.saveProficiencies[key]?1:0}));
  return header+`<div class="sheet-scroll check-list">${items.map(item=>{const roll=describeRoll(state,item);return `<button data-roll-kind="${item.kind}" data-roll-key="${item.key}" ${roll.blocked?'disabled':''}><span>${roll.label}<small>${item.detail} · ${item.rank===2?'maestria':item.rank===1?'competente':'non competente'}</small></span><b>${signed(roll.bonus)}</b></button>`}).join('')}</div><p class="micro">Competenze modificabili in “Personalizza eroe”.</p>`;
}
function dice(){const info=describeRoll(state,rollContext);return `<div class="eyebrow">LA SORTE TI ATTENDE</div><h2>Un salto nel destino<span>${info.label} · d20</span></h2><div class="die ${rolling?'rolling':''}"><svg viewBox="0 0 120 120" aria-hidden="true"><path d="M60 4 110 34 104 91 60 117 10 89 5 34Z M60 4 34 42 5 34 M34 42 10 89 60 79 104 91 86 42 110 34 M34 42H86L60 79Z M60 79V117 M86 42 60 4"/></svg><strong>${rolling?'…':result?result.total:'20'}</strong></div><div class="roll-detail" aria-live="polite">${rolling?'Il destino si muove…':result?`${result.dice.join(' / ')} ${signed(result.bonus)} = ${result.total}${result.natural===20?' · 20 naturale!':result.natural===1?' · 1 naturale':''}`:'Il prossimo capitolo inizia qui.'}</div><p class="roll-source">${info.parts}${state.condition&&['check','skill'].includes(rollContext.kind)?' · Avvelenato: svantaggio':''}</p><div class="bonus"><button data-bonus="-1" ${rollContext.kind!=='free'?'disabled':''} aria-label="Riduci bonus">−</button><span>MODIFICATORE <b>${signed(info.bonus)}</b></span><button data-bonus="1" ${rollContext.kind!=='free'?'disabled':''} aria-label="Aumenta bonus">+</button></div><div class="modes">${[['disadvantage','Svant.'],['normal','Normale'],['advantage','Vant.']].map(([m,n])=>`<button data-mode="${m}" aria-pressed="${state.mode===m}">${n}</button>`).join('')}</div><div class="dice-context"><button data-action="free">Dado libero</button><span>Effettivo: ${info.mode==='normal'?'normale':info.mode==='advantage'?'vantaggio':'svantaggio'}</span></div><button class="roll-button" data-action="roll" ${rolling||info.blocked?'disabled':''}>${rolling?'LANCIO IN CORSO':'✧ LANCIA IL D20'}</button>`}
function combatOptions(){
  if(!state.extended)return '';
  const w=equippedWeapon(state),c=state.extended.combat;
  return `<label class="target-picker">Arma attiva<select id="active-weapon"><option value="">Scegli arma</option>${state.inventory.filter(i=>i.type==='weapon'&&i.quantity>0).map(i=>`<option value="${i.id}" ${i.id===state.equippedWeaponId?'selected':''}>${escapeHtml(i.name)}</option>`).join('')}</select></label><div class="turn-controls">${c.raging?'<button data-combat-option="raging">Termina Ira</button>':''}${c.greatWeaponMasterAvailable&&w?.profile?.heavy?`<button data-combat-option="greatWeaponMaster" aria-pressed="${c.greatWeaponMaster}">GWM −5 / +10 ${c.greatWeaponMaster?'attivo':'spento'}</button>`:''}${w?.profile?.thrownRange?`<button data-combat-option="thrown" aria-pressed="${c.thrown}">${c.thrown?'Attacco a distanza':'Lancia arma'}</button>`:''}</div>`;
}
function actions(){const weapon=weaponStats(state);if(targetId&&!state.encounter.participants.some(item=>item.id===targetId))targetId=null;const target=targetId?participantView(state,targetId):null;const geometry=targetId?targetGeometry(state,targetId):null;const blocked=geometry?.known&&(!geometry.inRange||!geometry.visible);const tabs=`<div class="sheet-tabs"><button data-action-tab="weapon" aria-pressed="${actionTab==='weapon'}">Arma</button><button data-action-tab="powers" aria-pressed="${actionTab==='powers'}">Magie e capacità</button><button data-action-tab="encounter" aria-pressed="${actionTab==='encounter'}">Turni</button><button data-action-tab="grid" aria-pressed="${actionTab==='grid'}">Mappa</button></div>`;if(actionTab==='grid')return gridView(tabs);if(actionTab==='encounter')return encounterView(tabs);if(actionTab==='powers')return powersView(tabs);return `<div class="eyebrow">AZIONI AL TAVOLO</div><h2>${escapeHtml(weapon?.name??'Nessuna arma')}<span>${weapon?`${weapon.ability} ${signed(modifier(state.abilities[weapon.ability]))} · competenza ${weapon.proficient?signed(proficiency(state.level)):'non applicata'}`:'Equipaggia un’arma dallo Zaino'}</span></h2>${tabs}<div class="functional-content weapon-content">${combatOptions()}<label class="target-picker">Bersaglio<select id="attack-target"><option value="">CA manuale</option>${state.encounter.participants.filter(item=>item.id!=='hero').map(item=>{const view=participantView(state,item.id);return `<option value="${item.id}" ${targetId===item.id?'selected':''}>${escapeHtml(view.name)} · CA ${view.armorClass}</option>`}).join('')}</select></label><div class="geometry-info" role="status">${geometry?.known?`${geometry.distance} caselle · portata ${weapon?.reach??1} · ${geometry.visible?'linea libera':'muro sulla linea'}${geometry.inRange?'':' · fuori portata'}`:target?'Posizioni mancanti: verifica distanza e vista con il DM.':'CA manuale: distanza e vista gestite dal DM.'}</div>${blocked?`<button class="dm-override" data-action="override-attack" aria-pressed="${dmOverride}">${dmOverride?'✓ Override DM attivo per un attacco':'Il DM consente questo attacco'}</button>`:''}<div class="equipment-status">${weapon?`${weapon.rage?'Ira · ':''}${weapon.powerAttack?'GWM · ':''}Attacco ${signed(weapon.attackBonus)} · danni ${weapon.diceCount}d${weapon.dieSides}${signed(weapon.damageBonus)} ${escapeHtml(weapon.damageType)}`:'Nessuna arma equipaggiata'}</div>${weapon?`<p class="weapon-breakdown">${weapon.ability} ${signed(modifier(state.abilities[weapon.ability]))} · competenza ${signed(weapon.proficient?proficiency(state.level):0)} · extra attacco ${signed(weapon.attackModifier)}<br>Danni: caratteristica ${signed(weapon.addAbilityToDamage?modifier(state.abilities[weapon.ability]):0)} · extra ${signed(weapon.damageModifier)}</p>`:''}<div class="bonus"><button data-ac="-1" ${target?'disabled':''} aria-label="Riduci CA bersaglio">−</button><span>CA BERSAGLIO <b>${target?.armorClass??targetAc}</b></span><button data-ac="1" ${target?'disabled':''} aria-label="Aumenta CA bersaglio">+</button></div><p class="micro">${state.condition?'Avvelenato: svantaggio; annulla il vantaggio.':`Modalità dadi: ${state.mode==='normal'?'normale':state.mode==='advantage'?'vantaggio':'svantaggio'}`}</p><button class="roll-button" data-action="attack" ${!weapon||(blocked&&!dmOverride)?'disabled':''}>TIRA PER COLPIRE</button><div class="combat-result" aria-live="polite">${combat?`<strong>${combat.hits?(combat.critical?'20 naturale · Critico':'Colpito'):'Mancato'}</strong><span>${combat.dice.join(' / ')} ${signed(combat.bonus)} = ${combat.total} contro CA ${combat.armorClass}${combat.natural===1?' · 1 naturale':''}${combat.override?' · Override DM':''}</span>`:'Scegli la CA indicata dal DM.'}</div>${combat?.hits?`<button class="roll-button" data-action="damage" ${damageResult?'disabled':''}>TIRA DANNI ${combat.weapon.diceCount*(combat.critical?2:1)+(combat.critical?combat.weapon.extraMeleeCriticalDice??0:0)}d${combat.weapon.dieSides}${signed(combat.weapon.damageBonus)}</button>`:''}<p class="damage-result" aria-live="polite">${damageResult?`${damageResult.dice.join(' + ')} ${signed(damageResult.bonus)} = ${damageResult.total} danni`:''}</p>${state.encounter.strike?`<div class="pending-damage"><span>${state.encounter.strike.amount} danni → ${escapeHtml(participantView(state,state.encounter.strike.targetId).name)}</span><button data-action="apply-damage" ${state.encounter.strike.applied?'disabled':''}>${state.encounter.strike.applied?'Danni applicati':'Applica danni'}</button></div>`:''}<p class="micro">Danni applicati solo su conferma. Resistenze gestite dal DM.</p></div>`}
function pack(){return `<div class="eyebrow">OGGETTI E RISORSE</div><h2>Pronto a partire<span>${state.inventory.length} oggetti · ${escapeHtml(equippedWeapon(state)?.name??'nessuna arma')}</span></h2><div class="pack-list"><button class="rest-button" data-action="manage-inventory">Gestisci inventario e armi</button>${state.inventory.map(item=>`<article class="inventory-card"><div><strong>${escapeHtml(item.name)} ×${item.quantity}</strong><small>${item.type==='weapon'?(state.equippedWeaponId===item.id?'Equipaggiata':'Arma nello zaino'):item.type==='healing'?`Cura ${item.diceCount}d${item.dieSides}+${item.healingModifier}`:item.type==='consumable'?'Consumabile · effetto manuale':'Oggetto'}</small></div>${item.notes?`<p>${escapeHtml(item.notes)}</p>`:''}<div class="inventory-controls"><button data-item-quantity="-1" data-item="${item.id}" aria-label="Riduci quantità ${escapeHtml(item.name)}" ${item.quantity===0?'disabled':''}>−</button><button data-item-quantity="1" data-item="${item.id}" aria-label="Aumenta quantità ${escapeHtml(item.name)}" ${item.quantity===999?'disabled':''}>+</button>${item.type==='weapon'?`<button data-equip-weapon="${item.id}" ${item.quantity===0?'disabled':''}>${state.equippedWeaponId===item.id?'Riponi':'Equipaggia'}</button>`:['consumable','healing'].includes(item.type)?`<button data-use-item="${item.id}" ${item.quantity===0||(item.type==='healing'&&(state.hp===state.maxHp||(state.hp===0&&state.extended?.deathSaves.failures===3)))?'disabled':''}>${item.type==='healing'?'Usa cura':'Usa'}</button>`:''}</div></article>`).join('')||'<p class="micro">Inventario vuoto. Aggiungi oggetti dalla gestione.</p>'}<div class="pack-row"><span>Slancio ${state.resolve}/1<small>Risorsa narrativa originale</small></span><button data-action="resolve" ${!state.resolve?'disabled':''}>Usa</button></div><button class="rest-button" data-action="rest-short">Riposo breve · capacità</button><button class="rest-button" data-action="rest" ${state.hp===0&&state.extended?.deathSaves.failures===3?'disabled':''}>Riposo lungo · PF e risorse</button><p class="pack-message" aria-live="polite">${escapeHtml(packMessage)||'I riposi conservano le quantità degli oggetti.'}</p></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla ultima modifica</button></div>`}
function invalidateEquipment(){combat=null;damageResult=null;dmOverride=false;state={...state,encounter:{...state.encounter,strike:null}}}
function commitInventory(next){rememberUndo();state=next;invalidateEquipment();save();render()}
function manageInventory(){openInventoryManager({getState:()=>state,commit:commitInventory,newId,escapeHtml,abilityKeys})}
function render(){if(lastRenderedView)for(const selector of scrollSelectors){const element=document.querySelector(selector);if(element)scrollPositions.set(`${lastRenderedView}:${selector}`,element.scrollTop)}const currentView=viewKey();const info={hero:['IL PERSONAGGIO','Un eroe,\nun mondo.','Ogni eroe conserva la propria storia, le risorse e il combattimento. Scegli chi portare al tavolo.','Tocca + o − per aggiornare i PF.'],sheet:['LA SCHEDA','Ogni scelta\nha un valore.','Scegli una caratteristica: prepariamo il dado con il suo modificatore.','Tocca una delle sei caratteristiche.'],dice:['IL DESTINO','Lascia parlare\ni dadi.','Un lancio, tutti i calcoli. Prova anche vantaggio e svantaggio.','Tocca Lancia o premi Spazio.'],actions:['LE AZIONI','Ogni colpo\nconta.','Gli attacchi usano l’arma equipaggiata: caratteristica, competenza, dadi e bonus configurati. Il critico raddoppia i dadi.','Scegli l’arma nello Zaino; personalizzala da Gestisci inventario.'],pack:['LO ZAINO','Porta ciò\nche serve.','Oggetti consumabili e risorse rimangono salvati. Puoi annullare l’ultima modifica e fare un backup.','Scorri lo zaino per il riposo.']}[page];document.querySelector('#app').innerHTML=`<header><a class="brand" href="./"><span class="brand-icon">✧</span> POCKET<span>D-D</span></a><button class="presentation-toggle" data-action="focus-view" aria-pressed="${focusMode}">${focusMode?'Vista completa':'Solo dispositivo'} <span aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M4 9V4h5m6 0h5v5m0 6v5h-5M9 20H4v-5"/></svg></span></button></header>${characterBar()}${state.importInfo?'<div class="import-notice">Scheda importata da Beyond: dati compilati, effetti condizionali da verificare. <button data-action="beyond-info">Dati non importati</button></div>':''}${importedSummary(state)}<main><section class="intro"><div class="edition"><span></span> UN PICCOLO OGGETTO. GRANDI AVVENTURE.</div><h1>Il tuo eroe. <br>Nel palmo <br><em>della mano.</em></h1><p>La tua scheda prende vita. Pixel, dadi e un pizzico di magia, ovunque ti porti la prossima sessione.</p><div class="intro-bottom"><span class="line"></span><span>UN COMPAGNO DI VIAGGIO<br><b>Progettato per il tuo tavolo.</b></span></div></section><section class="device-section" aria-label="Simulazione del dispositivo"><div class="device-top"><span>ESP32-S3 / AMOLED</span><span class="online">● STATO LOCALE</span></div><div class="device"><div class="screen" tabindex="0" role="region" aria-label="Display circolare, usa le frecce per cambiare sezione"><div class="screen-status"><span>✦ POCKETD-D</span><span>DEMO</span></div><div class="page ${page}">${page==='hero'?hero():page==='sheet'?sheet():page==='actions'?actions():page==='pack'?pack():dice()}</div><nav aria-label="Sezioni del dispositivo">${sections.map(([id,label])=>`<button data-page="${id}" aria-current="${page===id?'page':'false'}" aria-label="${label}" ${rolling?'disabled':''}><span>${navIcon(id)}</span>${label}</button>`).join('')}</nav></div></div><div class="device-bottom"><span>466 × 466 <b>·</b> ROUND TOUCH</span><span>PROTOTIPO INTERATTIVO</span></div>${quickNavigation()}<p class="device-notice" role="status">${escapeHtml(notice)}</p></section><aside><span class="aside-label">01 — ${info[0]}</span><h3>${info[1]}</h3><p>${info[2]}</p><div class="hint"><span>↗</span> ${info[3]}</div><p class="save-status">${escapeHtml(notice)||'Progressi salvati in questo browser.'}</p></aside></main>${supportTools()}`;for(const selector of scrollSelectors)document.querySelector(selector)?.scrollTo(0,scrollPositions.get(`${currentView}:${selector}`)??0);lastRenderedView=currentView;document.body.classList.toggle('focus-mode',focusMode);party.refreshLauncher();
}
async function roll(){if(rolling)return;const info=describeRoll(state,rollContext);if(info.blocked){notice='Tiro da verificare nei dettagli della scheda';render();return}rolling=true;result=null;render();const outcome=rollD20(info.bonus,info.mode);await new Promise(r=>setTimeout(r,matchMedia('(prefers-reduced-motion: reduce)').matches?0:650));result=outcome;state=recordRoll(state,info.label,outcome,'d20',info.mode);save();rolling=false;render()}
document.addEventListener('click',e=>{const b=e.target.closest('button');if(!b||b.disabled)return;if(rolling)return;
if(b.dataset.page){navigatePage(b.dataset.page,'tabs');return}
if(b.dataset.goPage){navigatePage(b.dataset.goPage,'quick');return}
if(b.dataset.action==='focus-view'){focusMode=!focusMode;render();document.querySelector('[data-action="focus-view"]').focus({preventScroll:true});return}
if(b.dataset.hp){rememberUndo();state=changeHp(state,Number(b.dataset.hp));save();if(Number(b.dataset.hp)<0&&state.concentration)notice='Danno subito: verifica con il DM il tiro di concentrazione. Nessun esito automatico.'}
if(b.dataset.sheetTab){sheetTab=b.dataset.sheetTab;render();document.querySelector(`[data-sheet-tab="${sheetTab}"]`).focus({preventScroll:true});return}
if(b.dataset.check||b.dataset.rollKind){rollContext=b.dataset.check?{kind:'check',key:b.dataset.check}:{kind:b.dataset.rollKind,key:b.dataset.rollKey};page='dice';result=null}
if(b.dataset.action==='free'){rollContext={kind:'free'};result=null}

if(b.dataset.bonus&&rollContext.kind==='free'){state.bonus=Math.max(-5,Math.min(15,state.bonus+Number(b.dataset.bonus)));result=null;save()}
if(b.dataset.mode){state.mode=b.dataset.mode;result=null;combat=null;damageResult=null;dmOverride=false;save()}
if(b.dataset.action==='condition'){rememberUndo();state.condition=!state.condition;if(state.extended){state.extended={...state.extended,conditions:state.condition?[...state.extended.conditions.filter(c=>!['poisoned','avvelenato'].includes(c.name.toLowerCase())),{name:'Poisoned',level:0}]:state.extended.conditions.filter(c=>!['poisoned','avvelenato'].includes(c.name.toLowerCase()))}}combat=null;damageResult=null;dmOverride=false;save()}
if(b.dataset.action==='undo'&&undo!==null){state={...undo,history:undoRestoresHistory?undo.history:state.history};undo=null;gridMessage='';result=null;rollContext={kind:'free'};combat=null;damageResult=null;dmOverride=false;packMessage='Ultima modifica annullata.';save()}
if(b.dataset.action==='roll'){roll();return}
if(b.dataset.action==='reset'){
  if(!confirm('Ripristinare solo il personaggio attivo ai valori dimostrativi? Gli altri personaggi restano conservati. Esporta prima la sua scheda.'))return;
  state=initialState();gridMessage='';page='hero';rollContext={kind:'free'};result=null;undo=null;combat=null;damageResult=null;dmOverride=false;packMessage='';save();
}
if(b.dataset.action==='extended-state'){openExtendedState({getState:()=>state,commit:commitExtended,escapeHtml});return}
if(b.dataset.combatOption&&state.extended){try{rememberUndo();state=setCombatOption(state,b.dataset.combatOption,!state.extended.combat[b.dataset.combatOption]);combat=null;damageResult=null;save()}catch(error){notice=error.message}render();return}
if(b.dataset.action==='beyond-import'){openBeyondImport({commit:commitBeyond});return}
if(b.dataset.action==='beyond-info'&&state.importInfo){showBeyondLimitations(state.importInfo);return}
if(b.dataset.action==='manage-inventory'){manageInventory();return}
if(b.dataset.action==='party'){party.open();return}
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
  if(!confirm(kind==='long'?'Confermi il riposo lungo con il DM? Recupera PF, Slancio, slot e capacità previste. Termina la concentrazione; conserva condizioni e oggetti consumati.':'Confermi il riposo breve con il DM? Recupera solo le capacità impostate su riposo breve. Non recupera PF o slot ordinari; recupera gli slot del patto.'))return;
  try{const next=rest(state,kind);rememberUndo();state=next;packMessage=kind==='long'?'Riposo lungo completato.':'Capacità da riposo breve recuperate.';powerMessage=packMessage;save()}catch(error){packMessage=error.message}
}
if(b.dataset.actionTab){actionTab=b.dataset.actionTab;render();document.querySelector(`[data-action-tab="${actionTab}"]`).focus({preventScroll:true});return}
if(b.dataset.action==='manage-powers'){openPowerManager();return}
if(b.dataset.usePower){
  const power=state.powers.find(item=>item.id===b.dataset.usePower);
  if(power?.type==='spell'&&power.concentration&&state.concentration&&!confirm('Il nuovo lancio sostituisce la concentrazione attuale. Continuare?'))return;
  try{const level=document.querySelector(`[data-slot-for="${power.id}"]`)?.value??0;const next=usePower(state,power.id,level);rememberUndo();state=next;powerMessage=`${power.name}: risorsa utilizzata. Risolvi l’effetto con il DM.`;save()}catch(error){powerMessage=error.message}
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
if(b.dataset.action==='device-export')openDeviceExport();
if(b.dataset.action==='device-backup')openDeviceBackup(escapeHtml);
if(b.dataset.action==='export'){downloadJson(serialize(state),'pocketdd-eroe.json');notice='Scheda attiva esportata'}
if(b.dataset.action==='export-library'){downloadJson(serializeLibrary(library),'pocketdd-raccolta.json');notice='Raccolta esportata'}
if(b.dataset.action==='recover-storage'&&recoveryRaw!==null)downloadJson(recoveryRaw,'pocketdd-dati-da-recuperare.json');
if(b.dataset.action==='manage-characters'){openCharacterManager();return}
if(b.dataset.action==='import-library'){document.querySelector('#library-file').click();return}
if(b.dataset.action==='edit'||b.dataset.action==='new'){openEditor(b.dataset.action==='new');return}
if(b.dataset.action==='import'){document.querySelector('#backup-file').click();return}

render();});
document.addEventListener('keydown',e=>{
  if(document.querySelector('dialog[open]'))return;
  const navigation=e.target.closest('[data-page],[data-go-page]');
  if(['ArrowLeft','ArrowRight','Home'].includes(e.key)&&(navigation||e.target===document.body||e.target.classList.contains('screen'))){
    e.preventDefault();const index=sections.findIndex(([id])=>id===page),offset=e.key==='ArrowLeft'?-1:1;
    navigatePage(e.key==='Home'?'hero':sections[(index+offset+sections.length)%sections.length][0],navigation?.dataset.goPage?'quick':navigation?'tabs':'screen');return;
  }
  if(e.code==='Space'&&page==='dice'&&(e.target===document.body||e.target.classList.contains('screen'))){e.preventDefault();roll()}
});
let swipeStart=null;
document.addEventListener('pointerdown',e=>{
  swipeStart=!rolling&&!document.querySelector('dialog[open]')&&e.target.closest('.screen')&&!e.target.closest('button,input,select,textarea,.grid-play-area')?{x:e.clientX,y:e.clientY,id:e.pointerId}:null;
});
document.addEventListener('pointercancel',()=>swipeStart=null);
document.addEventListener('pointerup',e=>{
  const start=swipeStart;swipeStart=null;if(!start||start.id!==e.pointerId||document.querySelector('dialog[open]'))return;
  const dx=e.clientX-start.x,dy=e.clientY-start.y;if(Math.abs(dx)<65||Math.abs(dx)<Math.abs(dy)*2)return;
  const index=sections.findIndex(([id])=>id===page);navigatePage(sections[(index+(dx<0?1:-1)+sections.length)%sections.length][0]);
});
if(storedRaw===null&&!storageBlocked)persistLibrary();render();

document.addEventListener('change',async e=>{
  if(e.target.id==='active-weapon'){try{commitInventory(equipWeapon(state,e.target.value||null))}catch(error){notice=error.message;render()}return}
  if(e.target.id==='attack-target'){targetId=e.target.value||null;combat=null;damageResult=null;dmOverride=false;state={...state,encounter:{...state.encounter,strike:null}};save();render();return}

  if(e.target.id==='character-select'){
    if(rolling)return;
    useLibrary(selectCharacter(updateCharacter(library,state),e.target.value));render();document.querySelector('#character-select').focus();return;
  }
  if(!['backup-file','library-file'].includes(e.target.id))return;
  const collection=e.target.id==='library-file',file=e.target.files?.[0];if(!file)return;
  e.target.value='';
  if(file.size>(collection?MAX_LIBRARY_BYTES:1000000)){notice='Importazione rifiutata: file troppo grande.';render();return}
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
  const draft=structuredClone(fresh?initialState():state),originalAvatar=structuredClone(draft.avatar);
  if(fresh){draft.name='Nuovo eroe';draft.title='Una storia da scrivere';draft.hp=draft.maxHp}
  const dialog=document.createElement('dialog');dialog.className='editor-dialog hero-editor-dialog';
  Object.assign(draft.avatar,avatarIdentity(draft.avatar),{outfit:avatarOutfit(draft.avatar)});
  const identitySelect=(label,key,names)=>`<label>${label}<select name="${key}">${names.map((name,i)=>`<option value="${i}" ${draft.avatar[key]===i?'selected':''}>${name}</option>`).join('')}</select></label>`;
  const appearanceChoices=()=>{const species=avatarSpecies[draft.avatar.lineage];return choice(species.materialName??'Carnagione','skin',species.skinNames??['Chiara','Media','Scura'])+choice(species.hair===false?'Colore capelli · non applicabile':'Capelli','hair',species.hairNames??['Castani','Biondi','Scuri'])+choice('Mantello','cloak',['Bosco','Vino','Notte'])+choice(species.hair===false?'Forma della testa':'Taglio','style',species.styles??['Corti','Lunghi','Rasati'])};
  const number=(label,key,value,min,max)=>`<label>${label}<input name="${key}" type="number" min="${min}" max="${max}" value="${value}" required></label>`;
  const choice=(label,key,names)=>`<fieldset class="appearance-choice" ${draft.avatar.outfit||key==='hair'&&avatarSpecies[draft.avatar.lineage].hair===false?'disabled':''}><legend>${label}</legend><div>${names.map((name,i)=>`<label><input type="radio" name="${key}" value="${i}" ${draft.avatar[key]===i?'checked':''}><span class="appearance-swatch" style="--swatch:${key==='skin'?avatarSpecies[draft.avatar.lineage].skin[i]:key==='hair'?(avatarSpecies[draft.avatar.lineage].hairColors??hairColors)[i]:key==='cloak'?cloakColors[i][0]:'#bda574'}">${key==='style'?['▰','▤','▪'][i]:'●'}</span><span>${name}</span></label>`).join('')}</div></fieldset>`;
  const training=`<fieldset><legend>Tiri salvezza competenti</legend><div class="save-training">${abilityKeys.map(key=>`<label><input type="checkbox" name="save-${key}" ${draft.saveProficiencies[key]?'checked':''}>${key}</label>`).join('')}</div></fieldset><fieldset><legend>Abilità · verifica le scelte con il DM</legend><div class="skill-training">${skills.map(skill=>`<label>${skill.name} (${skill.ability})<select name="skill-${skill.id}">${['Non competente','Competente','Maestria'].map((label,rank)=>`<option value="${rank}" ${draft.skillRanks[skill.id]===rank?'selected':''}>${label}</option>`).join('')}</select></label><label>${skill.name} · caratteristica<select name="skill-ability-${skill.id}"><option value="">Predefinita (${skill.ability})</option>${abilityKeys.map(key=>`<option value="${key}" ${draft.extended?.skillAbilities?.[skill.id]===key?'selected':''}>${key}</option>`).join('')}</select></label>`).join('')}</div></fieldset>`;
  dialog.innerHTML=`<form id="hero-editor"><div class="editor-heading"><div><div class="eyebrow">IL TUO PROSSIMO CAPITOLO</div><h2>${fresh?'Crea il tuo eroe':'Personalizza il tuo eroe'}</h2></div><button type="button" data-close aria-label="Chiudi editor">×</button></div><p class="editor-intro">Inserimento manuale · verifica i valori con il DM. Modificare Costituzione o livello non ricalcola i PF massimi.</p><div class="editor-body"><div class="editor-preview"><div class="avatar-stage">${sprite(draft.avatar,Boolean(equippedWeapon(draft)))}</div><strong class="avatar-caption">${escapeHtml(draft.name)}</strong><span>UN EROE, LA TUA STORIA</span><p>Le scelte si applicano quando salvi.</p></div><div class="editor-fields"><label>Nome<input name="name" value="${escapeHtml(draft.name)}" maxlength="24" required></label><label>Titolo<input name="title" value="${escapeHtml(draft.title)}" maxlength="40"></label><div class="editor-numbers">${number('Livello','level',draft.level,1,20)}${number('PF massimi','maxHp',draft.maxHp,1,999)}${number('Classe armatura','armorClass',draft.armorClass,1,30)}</div><fieldset class="appearance-editor"><legend>Il tuo aspetto</legend><div class="avatar-presets" role="group" aria-label="Stili avatar">${avatarPresets.map((preset,index)=>`<button type="button" data-avatar-preset="${index}">${preset.name}</button>`).join('')}<button type="button" data-avatar-reset>Ripristina aspetto</button></div>${identitySelect('Specie · famiglia visiva','lineage',avatarSpecies.map(s=>s.name))}${identitySelect('Silhouette','body',avatarBodies)}${identitySelect('Look classe · solo estetica','outfit',avatarClasses.map(c=>c.name))}<p data-class-status></p><p><a href="assets/PocketDD-SD.zip" download>Scarica il pacchetto SD</a> · Estrai la cartella pocketdd nella radice della SD.</p><p>Scelte estetiche indipendenti dalle regole e dal genere del personaggio.</p><div class="editor-appearance">${appearanceChoices()}</div></fieldset><fieldset><legend>Caratteristiche · punteggi 1–30</legend><div class="editor-stats">${abilityKeys.map(key=>number(key,key,draft.abilities[key],1,30)).join('')}</div></fieldset>${training}</div></div><p class="editor-error" role="alert"></p><div class="editor-controls"><button type="button" data-close>Annulla</button><button type="submit" class="roll-button">${fresh?'CREA PERSONAGGIO':'SALVA PERSONAGGIO'}</button></div></form>`;
  document.body.append(dialog);paintAvatar();
  dialog.querySelectorAll('[data-close]').forEach(button=>button.addEventListener('click',()=>dialog.close()));
  dialog.addEventListener('close',()=>dialog.remove());
  function paintAvatar(){
    dialog.querySelector('.editor-preview .sprite').outerHTML=sprite(draft.avatar,Boolean(equippedWeapon(draft)));
    dialog.querySelector('[data-class-status]').textContent=draft.avatar.outfit?(classLooksAvailable()?'Preset con colori e taglio propri; le scelte originali sono conservate. Sul dispositivo serve il pacchetto SD.':'Asset classi assenti: anteprima originale. La scelta del preset viene conservata; installa il pacchetto SD sul dispositivo.'):'Stile originale: colori e capelli personalizzabili.';
    dialog.querySelectorAll('[data-avatar-preset]').forEach(b=>b.disabled=Boolean(draft.avatar.outfit));
    dialog.querySelector('.avatar-caption').textContent=dialog.querySelector('[name="name"]').value;
  }
  dialog.addEventListener('input',()=>{
    for(const key of ['skin','hair','cloak','style'])draft.avatar[key]=Number(dialog.querySelector(`[name="${key}"]:checked`).value);
    const previousLineage=draft.avatar.lineage,previousOutfit=draft.avatar.outfit;
    for(const key of ['lineage','body','outfit'])draft.avatar[key]=Number(dialog.querySelector(`[name="${key}"]`).value);
    if(previousLineage!==draft.avatar.lineage||previousOutfit!==draft.avatar.outfit)dialog.querySelector('.editor-appearance').innerHTML=appearanceChoices();
    paintAvatar();
  });
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button)return;
    if(button.dataset.avatarPreset!==undefined)draft.avatar={...draft.avatar,...structuredClone(avatarPresets[Number(button.dataset.avatarPreset)].avatar)};
    else if(button.hasAttribute('data-avatar-reset'))draft.avatar=structuredClone(originalAvatar);
    else return;
    Object.assign(draft.avatar,avatarIdentity(draft.avatar),{outfit:avatarOutfit(draft.avatar)});
    for(const key of ['lineage','body','outfit'])dialog.querySelector(`[name="${key}"]`).value=String(draft.avatar[key]);
    dialog.querySelector('.editor-appearance').innerHTML=appearanceChoices();
    for(const key of ['skin','hair','cloak','style'])dialog.querySelector(`[name="${key}"][value="${draft.avatar[key]}"]`).checked=true;
    paintAvatar();
  });
  dialog.querySelector('form').addEventListener('submit',event=>{
    event.preventDefault();
    const data=new FormData(event.currentTarget);
    try{
      const skillAbilities=Object.fromEntries(skills.map(skill=>[skill.id,String(data.get(`skill-ability-${skill.id}`))]).filter(([,value])=>value));
      let extended=draft.extended?{...draft.extended}:undefined;
      if(Object.keys(skillAbilities).length)extended={...(extended??emptyExtended()),skillAbilities};else if(extended)delete extended.skillAbilities;
      const next=parseBackup(serialize({...draft,...(extended?{extended}:{}),name:data.get('name').trim(),title:data.get('title').trim(),level:Number(data.get('level')),maxHp:Number(data.get('maxHp')),hp:fresh?Number(data.get('maxHp')):Math.min(draft.hp,Number(data.get('maxHp'))),armorClass:Number(data.get('armorClass')),abilities:Object.fromEntries(abilityKeys.map(key=>[key,Number(data.get(key))])),skillRanks:Object.fromEntries(skills.map(skill=>[skill.id,Number(data.get(`skill-${skill.id}`))])),saveProficiencies:Object.fromEntries(abilityKeys.map(key=>[key,data.has(`save-${key}`)]))}));
      if(sourceId!==library.activeId)throw new Error('Il personaggio attivo è cambiato. Riapri l’editor.');
      if(storageBlocked)throw new Error('Risolvi prima il recupero dei dati salvati');
      const nextLibrary=fresh?addCharacter(updateCharacter(library,state),next,newId()):updateCharacter(library,next);
      const raw=storeLibrary(localStorage,nextLibrary,storedRaw);
      if(!fresh)rememberUndo();library=nextLibrary;storedRaw=raw;state=activeCharacter(library);storageWarning='';
      if(fresh)resetCharacterView();else resetCombatAfterEdit();notice='Scheda salvata';
      dialog.close();render();
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.showModal();
}

function powersView(tabs){
  const concentration=state.powers.find(p=>p.id===state.concentration);
  const card=power=>{
    const spell=power.type==='spell',stats=spell?spellStats(state,power):null,slots=spell?availableSlots(state,power):[];
    const slotNeeded=spell&&power.level>0&&power.usesSpellSlot!==false;
    const disabled=power.passive||(!spell&&(power.remaining===0||power.effect==='rage'&&(state.hp===0||state.extended?.combat.raging)||power.effect==='relentless-endurance'&&state.hp!==0))||(spell&&(power.prepared===false||power.limited?.remaining===0||state.extended?.combat.raging||power.concentration&&state.hp===0||slotNeeded&&!slots.length));
    const detail=spell?`${power.level===0?'Trucchetto':`Magia ${power.level}°`} · ${stats.ability} · attacco ${signed(stats.attack)} · CD ${stats.dc}${power.prepared===false?' · non preparata':''}${power.limited?` · usi ${power.limited.remaining}/${power.limited.maxUses}`:''}`:power.passive?'Nota · effetti gestiti al tavolo':`Capacità · ${power.remaining}/${power.maxUses} · recupero ${power.recovery==='short'?'breve':power.recovery==='long'?'lungo':'manuale'}`;
    return `<article class="power-card"><strong>${escapeHtml(power.name)}</strong><p>${escapeHtml(power.description)||'Nessuna nota.'}</p><small>${detail}</small>${power.passive?'':`<div class="power-controls">${slotNeeded?`<label>Slot<select data-slot-for="${power.id}" aria-label="Slot per ${escapeHtml(power.name)}">${slots.map(s=>`<option value="${s.value}">${s.label}</option>`).join('')}</select></label>`:''}<button data-use-power="${power.id}" ${disabled?'disabled':''}>${spell?'Lancia':'Usa'}</button></div>`}</article>`;
  };
  const ordinary=state.slots.map((s,i)=>s.max?`<span>${i+1}° <b>${s.current}/${s.max}</b></span>`:'').join('');
  const pact=state.extended?.pactSlots.map((s,i)=>s.max?`<span>Patto ${i+1}° <b>${s.current}/${s.max}</b></span>`:'').join('')??'';
  return `<div class="eyebrow">RISORSE PERSONALI</div><h2>Magie e capacità<span>Liste, contatori e note del personaggio</span></h2>${tabs}<div class="powers-scroll"><p class="micro">Effetti condizionali da verificare al tavolo</p><div class="slot-summary">${ordinary+pact||'Nessuno slot configurato'}</div>${concentration?`<div class="concentration">Concentrazione: ${escapeHtml(concentration.name)}<button data-action="end-concentration">Termina</button></div>`:''}${state.extended?.combat.raging?'<div class="concentration">Ira attiva: bonus alle armi in mischia con FOR; effetti difensivi al DM.<button data-combat-option="raging">Termina Ira</button></div>':''}${state.powers.map(card).join('')||'<p class="micro">La tua lista è vuota.</p>'}<p class="power-message" role="status">${escapeHtml(powerMessage)}</p><button class="rest-button" data-action="manage-powers">Gestisci lista e slot</button></div><div class="minor-actions"><button data-action="undo" ${undo===null?'disabled':''}>↶ Annulla ultima modifica</button></div>`;
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
    dialog.querySelector('#power-form-container').innerHTML=`<form id="power-editor"><fieldset><legend>${state.powers.some(item=>item.id===power.id)?'Modifica voce':'Nuova voce'}</legend><label>Tipo<select name="type"><option value="spell" ${power.type==='spell'?'selected':''}>Magia</option><option value="feature" ${power.type==='feature'?'selected':''}>Capacità</option></select></label><label>Nome<input name="power-name" maxlength="40" value="${escapeHtml(power.name)}" required></label><label>Note personali<textarea name="description" maxlength="${state.extended?6000:600}" rows="3">${escapeHtml(power.description)}</textarea></label><div data-spell-fields ${power.type==='spell'?'':'hidden'}>${state.extended?`<label>Caratteristica di lancio<select name="power-casting">${abilityKeys.map(k=>`<option ${k===(power.castingAbility??state.spellcastingAbility)?'selected':''}>${k}</option>`).join('')}</select></label><label class="checkbox-label"><input name="prepared" type="checkbox" ${power.prepared!==false?'checked':''}>Preparata / disponibile</label><label class="checkbox-label"><input name="uses-slot" type="checkbox" ${power.usesSpellSlot!==false?'checked':''}>Consuma slot per magie di livello 1+</label>${power.limited?`<label>Utilizzi rimasti (massimo ${power.limited.maxUses})<input name="spell-remaining" type="number" min="0" max="${power.limited.maxUses}" value="${power.limited.remaining}" required></label>`:''}`:''}<label>Livello (0 = trucchetto)<input name="spell-level" type="number" min="0" max="9" value="${power.level??0}" required></label><label class="checkbox-label"><input name="concentration" type="checkbox" ${power.concentration?'checked':''}>Richiede concentrazione</label></div><div data-feature-fields ${power.type==='feature'?'':'hidden'}><div class="editor-numbers"><label>Utilizzi massimi<input name="max-uses" type="number" min="${state.extended?0:1}" max="99" value="${power.maxUses??1}" required></label><label>Utilizzi rimasti<input name="remaining" type="number" min="0" max="99" value="${power.remaining??1}" required></label></div><label>Recupero<select name="recovery">${[['short','Riposo breve'],['long','Riposo lungo'],['manual','Solo manuale']].map(([value,label])=>`<option value="${value}" ${power.recovery===value?'selected':''}>${label}</option>`).join('')}</select></label></div><div class="editor-controls"><button type="button" data-cancel-power>Annulla modifica</button><button class="roll-button" type="submit">Salva voce</button></div></fieldset></form>`;
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
        next={...state,spellcastingAbility:data.get('casting'),slots:state.slots.map((_,i)=>({current:Number(data.get(`current-${i}`)),max:Number(data.get(`max-${i}`))}))};validateMagic(next,Boolean(state.extended));
      }else if(form.id==='power-editor'){
        const power={...editing,id:editing.id,type:data.get('type'),name:data.get('power-name').trim(),description:data.get('description'),level:Number(data.get('spell-level')),concentration:data.has('concentration'),maxUses:Number(data.get('max-uses')),remaining:Number(data.get('remaining')),recovery:data.get('recovery')};
        if(state.extended&&power.type==='spell'){power.castingAbility=data.get('power-casting');power.attackModifier=editing.attackModifier??0;power.saveModifier=editing.saveModifier??0;power.prepared=data.has('prepared');power.usesSpellSlot=data.has('uses-slot');if(editing.limited)power.limited={...editing.limited,remaining:Number(data.get('spell-remaining'))};}
        if(state.extended&&power.type==='feature'){power.passive=power.maxUses===0;}
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
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">LE TUE AVVENTURE · ${library.characters.length}/${MAX_CHARACTERS}</div><h2>I tuoi personaggi</h2></div><button data-library-close aria-label="Chiudi personaggi">×</button></div><p class="editor-intro">Ogni eroe conserva PF, risorse, storico e combattimento. I personaggi sono salvati in questo browser: esporta la raccolta per trasferirli o conservarne una copia.</p><div class="character-list">${library.characters.map(({id,state:hero})=>`<article class="character-card ${id===library.activeId?'current-character':''}"><div class="character-portrait">${sprite(hero.avatar,Boolean(equippedWeapon(hero)))}</div><div class="character-details"><strong>${escapeHtml(hero.name)}</strong><p>Livello ${hero.level} · ${hero.hp}/${hero.maxHp} PF${hero.encounter.round?` · combattimento, round ${hero.encounter.round}`:''}</p><small>${id===library.activeId?'Personaggio attivo':escapeHtml(hero.title)}</small><div class="character-controls"><button data-select-character="${id}" ${id===library.activeId?'disabled':''}>Usa</button><button data-copy-character="${id}" ${library.characters.length>=MAX_CHARACTERS?'disabled':''}>Duplica</button><button data-delete-character="${id}" ${library.characters.length===1?'disabled':''}>Elimina</button></div></div></article>`).join('')}</div><div class="library-tools"><button data-library-new ${library.characters.length>=MAX_CHARACTERS?'disabled':''}>+ Nuovo personaggio</button><button data-action="import">Aggiungi da backup</button><button data-action="export-library">Esporta raccolta</button><button data-action="import-library">Ripristina raccolta</button></div><p class="editor-error" role="alert"></p>`;
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

function deviceTrainingPreview(s){
  const contexts=[...abilityKeys.map(key=>({kind:'check',key})),...skills.map(({id})=>({kind:'skill',key:id})),...abilityKeys.map(key=>({kind:'save',key}))];
  const rows=contexts.map(context=>{const r=describeRoll(s,context);return `<tr><td>${escapeHtml(r.label)}</td><td>${r.blocked?'Da verificare':(r.bonus>=0?'+':'')+r.bonus}</td><td>${escapeHtml(r.parts)}</td></tr>`}).join('');
  return `<details data-device-training><summary>Competenze e bonus · 30 prove</summary><p>Competenza +${proficiency(s.level)}. Le voci da verificare restano bloccate sul dispositivo.</p><table class="device-training-table"><thead><tr><th>Prova</th><th>Bonus</th><th>Composizione</th></tr></thead><tbody>${rows}</tbody></table></details>`;
}
function deviceInventoryPreview(s,packet){
  const w=weaponStats(s);
  return `<details data-device-inventory><summary>Zaino · ${packet.inventory.items.length} voci</summary><p>${w?`Arma scelta: ${escapeHtml(w.name)} · attacco ${(w.attackBonus>=0?'+':'')+w.attackBonus} · danni ${w.diceCount}d${w.dieSides} ${(w.damageBonus>=0?'+':'')+w.damageBonus}.`:'Nessuna arma scelta.'} Note e proprietà speciali restano sul PC.</p><table class="device-training-table"><thead><tr><th>Oggetto</th><th>Quantità</th><th>Tipo</th></tr></thead><tbody>${packet.inventory.items.map(v=>`<tr><td>${escapeHtml(v.name)}</td><td>${v.quantity}</td><td>${['Oggetto','Consumabile','Cura','Arma'][v.kind]}</td></tr>`).join('')}</tbody></table></details>`;
}
function deviceMagicPreview(s,packet){
  const rows=packet.magic.powers.map((v,i)=>{const p=s.powers[i],stats=p.type==='spell'?spellStats(s,p):null;return `<tr><td>${escapeHtml(v.name)}</td><td>${stats?`Attacco ${(stats.attack>=0?'+':'')+stats.attack} / CD ${stats.dc}`:`${v.remaining}/${v.maximum} usi`}</td><td>${v.flags&32?'Estratto 500 caratteri':`${v.description.length} caratteri`}</td></tr>`});
  return `<details data-device-magic><summary>Magie e capacità · ${packet.magic.powers.length} voci</summary><p>Slot ordinari e patto separati. Utilizzi e concentrazione richiedono conferma sul dispositivo; effetti da risolvere con il DM. Tasti configurabili da Eroe → Aspetto → Tasti.</p><table class="device-training-table"><thead><tr><th>Voce</th><th>Valori</th><th>Testo</th></tr></thead><tbody>${rows.join('')}</tbody></table></details>`;
}
function deviceStatusPreview(packet){
  const x=packet.status;
  return `<details data-device-status><summary>Stato · ${x.conditions.length} condizioni annotate</summary><p>${x.available?`Morte: ${x.successes}/3 riuscite, ${x.failures}/3 fallite; stabilizzato ${x.stabilized?'sì':'no'}.`:'Registro non disponibile: inizializzabile esplicitamente sul dispositivo.'}</p><p>Avvelenato: ${packet.training.poisoned?'attivo':'non attivo'}. Effetto separato dalle note; le altre condizioni non applicano regole automatiche.</p><ul>${x.conditions.map(c=>`<li>${escapeHtml(c.name)} · livello ${c.level}</li>`).join('')}</ul></details>`;
}
function deviceWalletPreview(packet){
  const x=packet.wallet,units={cp:'Rame (MR)',sp:'Argento (MA)',ep:'Electrum (ME)',gp:'Oro (MO)',pp:'Platino (MP)'};
  return `<details data-device-wallet><summary>Monete · ${x.available?'saldo verificato':'non disponibili'}</summary><p>${x.available?'Cinque saldi separati, modificabili sul dispositivo con anteprima e conferma.':'Saldo non verificato: nessuna ricchezza inventata. Configuralo sul dispositivo o verifica le monete in Stato e risorse.'}</p>${x.available?`<ul>${Object.entries(units).map(([k,name])=>`<li>${name}: ${x[k]}</li>`).join('')}</ul>`:''}<p>Richiede USB9 / firmware v0.51. Nessun cambio o acquisto automatico.</p></details>`;
}
function openDeviceExport(){
  const dialog=document.createElement('dialog');dialog.className='editor';
  try{
    const {packet,warnings}=projectDevice(state);
    dialog.innerHTML=`<div class="editor-heading"><h2>Prepara la copia per PocketD-D</h2><button data-device-close aria-label="Chiudi">×</button></div><p>Collega via USB il dispositivo con firmware PocketD-D. Invia questa copia direttamente da Chrome o Edge su PC, oppure scarica il file per lo strumento Python. Controlla l’anteprima: il dispositivo richiede un’ulteriore conferma sul display e sostituisce solo la propria copia. La raccolta PC resta conservata.</p><p><strong>${escapeHtml(packet.name)}</strong> · livello ${packet.level} · PF ${packet.hp}/${packet.maxHp} (+${packet.tempHp} temporanei) · CA ${packet.armorClass}</p><div class="device-export-avatar" data-device-avatar>${deviceAvatarSvg(packet.avatar)}<div><strong>Avatar da trasferire</strong><p>${escapeHtml(avatarDescription(packet.avatar))}.</p><small>I look classe richiedono firmware 0.70 e il pacchetto sulla SD. Senza asset viene mostrato lo stile originale. Classe e specie sono solo scelte estetiche; non equipaggiano armi o armature.</small></div></div><p>${abilityKeys.map(k=>`${k} ${packet.abilities[k]}`).join(' · ')}</p>${deviceTrainingPreview(state)}${deviceInventoryPreview(state,packet)}${deviceMagicPreview(state,packet)}${deviceStatusPreview(packet)}${deviceWalletPreview(packet)}${deviceDefencesPreview(packet)}<details data-device-recovery><summary>Dadi vita · ${packet.recovery.pools.length} riserve</summary><p>${packet.recovery.available?'Spesa e recuperi sul dispositivo con anteprima e conferma.':'Dati non disponibili: nessuna riserva inventata.'}</p><ul>${packet.recovery.pools.map(h=>`<li>${escapeHtml(h.name)}: ${h.maximum-h.used}/${h.maximum} d${h.die}</li>`).join('')}</ul></details><p>Contatori: ${packet.resources.map(r=>`${escapeHtml(r.name)} ${r.current}/${r.maximum}`).join(' · ')||'nessuno'}</p><ul>${warnings.map(w=>`<li>${escapeHtml(w)}</li>`).join('')}</ul><button data-device-send>Invia al dispositivo via USB</button>${usbStatusMarkup}<button data-device-download>Scarica copia limitata per dispositivo</button>`;
    const configure=dialog.querySelector('[data-device-configure-defences]');
    if(configure){configure.hidden=!state.extended;configure.onclick=()=>{dialog.close();openExtendedState({getState:()=>state,commit:commitExtended,escapeHtml,focusDefences:true});};}
    const usb=bindUsbDialog(dialog);dialog.querySelector('[data-device-send]').disabled=!usb.supported;
    dialog.querySelector('[data-device-send]').onclick=()=>usb.run(options=>sendDeviceCopy(packet,options));
    dialog.querySelector('[data-device-download]').onclick=()=>{downloadJson(serializeDevice(packet),'pocketdd-device.json');notice='Copia per dispositivo esportata. Raccolta PC conservata.';dialog.close();render()};
  }catch(error){dialog.innerHTML=`<h2>Esportazione non disponibile</h2><p>${escapeHtml(error.message)}</p><button data-device-close>Chiudi</button>`}
  dialog.querySelector('[data-device-close]').onclick=()=>dialog.close();
  dialog.addEventListener('close',()=>dialog.remove(),{once:true});document.body.append(dialog);dialog.showModal();
}

function deviceDefencesPreview(packet){return `<details data-device-defences><summary>Difese ai danni · ${packet.defences?damageNames.filter((_,i)=>defenceFlags(packet.defences,i)!==null).length:0}/13 tipi verificati</summary><p>Applicate nel calcolo Danni del dispositivo. Tipi non verificati da configurare; effetti temporanei, condizioni e variazioni dell’equipaggiamento richiedono un controllo.</p><ul>${damageNames.map((name,i)=>`<li>${name}: ${defenceLabel(packet.defences?defenceFlags(packet.defences,i):null)}</li>`).join('')}</ul><button type="button" data-device-configure-defences>Configura difese sul PC</button></details>`;}

function commitExtended(candidate){
 if(storageBlocked)throw new Error('Risolvi prima il recupero dei dati salvati');
 const next=updateCharacter(library,parseBackup(serialize(candidate))),raw=storeLibrary(localStorage,next,storedRaw);
 rememberUndo();library=next;storedRaw=raw;state=activeCharacter(library);storageWarning='';notice='Stato e risorse salvati';render();
}
