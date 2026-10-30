// Experimental, read-only account check. No cookie extraction, storage, bridge or PG writes.
// The browser supplies its existing session only to the fixed auth-service URL.
const AUTH = 'https://auth-service.dndbeyond.com/v1/cobalt-token';
const LIST = 'https://character-service.dndbeyond.com/character/v5/characters/list';
export const ACCOUNT_ORIGINS = ['https://auth-service.dndbeyond.com/*', 'https://character-service.dndbeyond.com/*'];
const CLAIM = 'http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier';
const LIMIT = 512 * 1024;
const integer = (n, min, max) => Number.isSafeInteger(n) && n >= min && n <= max;
class CheckError extends Error { constructor(code) { super(code); this.code = code; } }
const fail = code => { throw new CheckError(code); };
const messages = {
  network: 'Servizio non raggiungibile. Riprova; non significa che la sessione sia scaduta.',
  timeout: 'Tempo di attesa terminato. Riprova.',
  cancelled: 'Verifica annullata.',
  rejected: 'Accesso rifiutato. Apri Beyond, verifica il login e riprova.',
  http: 'Il servizio ha risposto con un errore. Riprova più tardi.',
  format: 'Formato della risposta non riconosciuto. Nessun risultato considerato valido.',
  size: 'Risposta oltre il limite della verifica. Elenco non accettato né tagliato.',
  session: 'Risposta ricevuta, ma sessione personale non verificabile. Apri Beyond e riprova.',
  expired: 'Il token ricevuto è scaduto o troppo vicino alla scadenza. Riprova.',
  catalog: 'Elenco non riconosciuto o incoerente. Nessun personaggio importato.'
};

async function readJSON(response, limit, signal) {
  if (!/^application\/(?:json|[\w.-]+\+json)(?:;|$)/i.test(response.headers.get('content-type') ?? '')) {
    await response.body?.cancel().catch(() => {}); fail('format');
  }
  const length = response.headers.get('content-length');
  if (length !== null && (!/^\d+$/.test(length) || Number(length) > limit)) {
    await response.body?.cancel().catch(() => {}); fail('size');
  }
  if (!response.body) fail('format');
  const reader = response.body.getReader(), decoder = new TextDecoder('utf-8', {fatal:true});
  let size = 0, text = '';
  try {
    while (true) {
      signal.throwIfAborted();
      const {done, value} = await reader.read();
      if (done) break;
      size += value.byteLength;
      if (size > limit) fail('size');
      try { text += decoder.decode(value, {stream:true}); } catch { fail('format'); }
    }
    try { text += decoder.decode(); } catch { fail('format'); }
    try { return JSON.parse(text); } catch { fail('format'); }
  } finally { await reader.cancel().catch(() => {}); reader.releaseLock(); }
}

function session(data, now) {
  const token = data?.token;
  if (typeof token !== 'string' || token.length > 16384 || !/^[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+\.[A-Za-z0-9_-]+$/.test(token)) fail('session');
  let payload;
  try {
    const encoded = token.split('.')[1].replace(/-/g, '+').replace(/_/g, '/');
    payload = JSON.parse(atob(encoded.padEnd(Math.ceil(encoded.length/4)*4, '=')));
  } catch { fail('session'); }
  const id = payload?.[CLAIM];
  if (typeof id !== 'string' || !/^[1-9]\d{0,14}$/.test(id) || !Number.isSafeInteger(Number(id))) fail('session');
  if (!integer(payload.exp, 1, Number.MAX_SAFE_INTEGER) || payload.exp * 1000 <= now + 15000) fail('expired');
  // Claims are read only from the fixed HTTPS auth response; no arbitrary JWT is accepted.
  // This is not an independent verification of the JWT signature.
  return {token, id, expires:payload.exp * 1000};
}

function label(value, required = false) {
  if (value === undefined || value === null) { if (required) fail('catalog'); return ''; }
  if (typeof value !== 'string' || value.length > 160 || /[\x00-\x1f\x7f]/.test(value) || required && !value.trim()) fail('catalog');
  return value.trim();
}

function catalog(data) {
  const rows = data?.data?.characters;
  if (data?.success !== true || !Array.isArray(rows)) fail('catalog');
  if (rows.length > 300) fail('size');
  const ids = new Set();
  const characters = rows.map(row => {
    if (!integer(row?.id, 1, Number.MAX_SAFE_INTEGER) || ids.has(row.id)) fail('catalog');
    ids.add(row.id);
    if (row.level != null && !integer(row.level, 0, 20)) fail('catalog');
    return {id:String(row.id), name:label(row.name, true), level:row.level ?? null,
      race:label(row.raceName), class:label(row.classDescription)};
  });
  // Even a plausible count is not evidence that every owned PG is present.
  // Do not fall back to campaign rosters (they include other people's PGs).
  const page = data.pagination;
  const partial = integer(page?.total, 0, Number.MAX_SAFE_INTEGER) && page.total > characters.length
    || integer(page?.pages, 1, Number.MAX_SAFE_INTEGER) && page.pages > 1;
  return {characters, completeness:partial ? 'partial' : 'unverified'};
}

export async function checkAccount({fetcher = fetch, now = Date.now, signal:externalSignal, timeoutMs = 10000} = {}) {
  const checks = {authHTTP:null, authJSON:null, session:null, listHTTP:null, listJSON:null, catalog:null};
  const status = {auth:null, list:null};
  let stage = 'auth', credential = null;
  const signal = externalSignal ? AbortSignal.any([externalSignal, AbortSignal.timeout(timeoutMs)]) : AbortSignal.timeout(timeoutMs);
  async function request(url, options, limit, name) {
    signal.throwIfAborted();
    const response = await fetcher(url, {...options, signal, redirect:'error', cache:'no-store', referrerPolicy:'no-referrer'});
    signal.throwIfAborted();
    status[name] = response.status;
    checks[name+'HTTP'] = response.ok;
    if (!response.ok) { await response.body?.cancel().catch(() => {}); fail([401,403].includes(response.status) ? 'rejected' : 'http'); }
    checks[name+'JSON'] = false;
    const data = await readJSON(response, limit, signal);
    checks[name+'JSON'] = true;
    return data;
  }
  try {
    const auth = await request(AUTH, {method:'POST', credentials:'include', headers:{Accept:'application/json','Content-Type':'application/json'}, body:'{}'}, 32768, 'auth');
    checks.session = false;
    credential = session(auth, now());
    checks.session = true;
    stage = 'list';
    const data = await request(LIST+'?userId='+credential.id, {method:'GET', credentials:'omit', headers:{Accept:'application/json',Authorization:'Bearer '+credential.token}}, LIMIT, 'list');
    checks.catalog = false;
    const result = catalog(data);
    if (now() >= credential.expires) fail('expired');
    checks.catalog = true;
    return {ok:true, stage, checks, status, ...result};
  } catch (error) {
    const code = externalSignal?.aborted ? 'cancelled' : signal.aborted ? 'timeout' : error instanceof CheckError ? error.code : 'network';
    // Never relay exception text, headers, raw response bodies, JWTs or account identifiers.
    return {ok:false, stage, checks, status, code, message:messages[code], characters:[]};
  } finally { credential = null; }
}
