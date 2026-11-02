// Same conversion and device projection as the PC. No browser, OS, network or RNG.
import {convertBeyondFull} from '../src/beyond-conversion.js';
import {projectDevice} from '../src/device-export.js';

// Conversion is deterministic and must never consume randomness, including
// the interpreter's default Math generator. Device dice RNG is never bound.
delete Math.random;

// JSON source/drafts contain ordinary objects and arrays. Preserve own undefined
// properties and __proto__ as data, as structuredClone does; never evaluate input.
globalThis.structuredClone = function clone(value, seen = new Map()) {
  if (value === null || typeof value !== 'object') return value;
  if (seen.has(value)) return seen.get(value);
  const copy = Array.isArray(value) ? new Array(value.length) : {};
  seen.set(value, copy);
  for (const key of Object.keys(value)) Object.defineProperty(copy, key, {value:clone(value[key], seen), enumerable:true, configurable:true, writable:true});
  return copy;
};

globalThis.pocketBeyondConvert = (payload, id) => {
  const report = convertBeyondFull(payload, id);
  const blockers = report.conversion?.blockers ?? ['Rapporto non disponibile'];
  const limits = [...new Set([...(report.readiness?.manual ?? []), ...(report.readiness?.snapshots ?? []), ...(report.readiness?.unresolved ?? [])])];
  if (!report.canImport) return {version:1, ready:false, blockers, limits, packet:null};
  const {packet} = projectDevice(report.draft);
  return {version:1, ready:true, blockers:[], limits, packet};
};
