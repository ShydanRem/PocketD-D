import {initialState, parseBackup, serialize} from './model.js';

export const LIBRARY_KEY = 'pocketdd-library-v1';
export const LEGACY_KEY = 'pocketdd-demo-v1';
export const MAX_CHARACTERS = 20;
export const MAX_LIBRARY_BYTES = 5000000;
const validId = id => typeof id === 'string' && /^[a-zA-Z0-9_-]{1,64}$/.test(id);
const copyState = state => parseBackup(serialize(state));

export function createLibrary(state = initialState(), id = 'first-hero') {
  if (!validId(id)) throw new Error('Identità personaggio non valida');
  return {activeId:id, characters:[{id, state:copyState(state)}]};
}

export function activeCharacter(library) {
  const entry = library.characters.find(item => item.id === library.activeId);
  if (!entry) throw new Error('Personaggio attivo non trovato');
  return structuredClone(entry.state);
}

export function updateCharacter(library, state) {
  activeCharacter(library);
  const next = copyState(state);
  return {...library, characters:library.characters.map(item => item.id === library.activeId ? {...item,state:next} : item)};
}

export function addCharacter(library, state, id) {
  if (library.characters.length >= MAX_CHARACTERS) throw new Error(`La raccolta contiene già ${MAX_CHARACTERS} personaggi`);
  if (!validId(id) || library.characters.some(item => item.id === id)) throw new Error('Identità personaggio duplicata o non valida');
  return {activeId:id, characters:[...library.characters, {id,state:copyState(state)}]};
}

export function selectCharacter(library, id) {
  if (!library.characters.some(item => item.id === id)) throw new Error('Personaggio non trovato');
  return {...library, activeId:id};
}

export function removeCharacter(library, id) {
  if (!library.characters.some(item => item.id === id)) throw new Error('Personaggio non trovato');
  if (library.characters.length === 1) throw new Error('Conserva almeno un personaggio nella raccolta');
  const characters = library.characters.filter(item => item.id !== id);
  return {activeId:library.activeId === id ? characters[0].id : library.activeId, characters};
}

export function parseLibrary(raw) {
  if (typeof raw !== 'string' || new TextEncoder().encode(raw).length > MAX_LIBRARY_BYTES) throw new Error('Raccolta troppo grande');
  const input = JSON.parse(raw);
  if (!input || input.type !== 'pocketdd-library' || input.version !== 1 || !Array.isArray(input.characters) || input.characters.length < 1 || input.characters.length > MAX_CHARACTERS) throw new Error('Raccolta non valida');
  const ids = new Set();
  const characters = input.characters.map(item => {
    if (!item || !validId(item.id) || ids.has(item.id)) throw new Error('Identità personaggio duplicata o non valida');
    ids.add(item.id);
    return {id:item.id, state:parseBackup(JSON.stringify(item.state))};
  });
  if (!ids.has(input.activeId)) throw new Error('Personaggio attivo non valido');
  return {activeId:input.activeId, characters};
}

export function serializeLibrary(library) {
  const raw = JSON.stringify({type:'pocketdd-library',version:1,activeId:library.activeId,
    characters:library.characters.map(({id,state}) => ({id,state:JSON.parse(serialize(state))}))},null,2);
  parseLibrary(raw);
  return raw;
}

// Reading never writes: even an invalid existing collection stays available for recovery.
export function loadLibrary(storage) {
  const raw = storage.getItem(LIBRARY_KEY);
  if (raw !== null) return {library:parseLibrary(raw), raw};
  const legacy = storage.getItem(LEGACY_KEY);
  return {library:createLibrary(legacy === null ? initialState() : parseBackup(legacy)), raw:null};
}

// Reject stale tabs before they can overwrite another tab's newer collection.
export function storeLibrary(storage, library, expectedRaw) {
  if (storage.getItem(LIBRARY_KEY) !== expectedRaw) {
    const error = new Error('La raccolta è cambiata in un’altra scheda. Esporta i progressi di questa sessione e ricarica la pagina.');
    error.name = 'StorageConflictError';
    throw error;
  }
  const raw = serializeLibrary(library);
  storage.setItem(LIBRARY_KEY, raw);
  return raw;
}
