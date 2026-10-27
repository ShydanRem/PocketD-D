import {avatarSpecies,suggestBeyondAvatar} from './avatar-species.js';
import {parseBackup,serialize} from './model.js';
export function addBeyondAppearance(report,source){
  const appearance=suggestBeyondAvatar(source?.race,source?.gender);
  report.appearance=appearance;
  if(!report.draft)return report;
  report.draft.avatar={...report.draft.avatar,lineage:appearance.lineage,body:appearance.body};
  report.draft=parseBackup(serialize(report.draft));
  const message=appearance.status==='matched'?`Aspetto proposto dalla specie sorgente: ${appearance.source} → ${avatarSpecies[appearance.lineage].name}. Personalizzabile; nessun valore di gioco modificato.`:'Specie senza corrispondenza visiva univoca: silhouette umana provvisoria, da scegliere nell’anteprima o in Aspetto.';
  report.conversion.evidence.push(message);
  if(appearance.bodySource==='manual')report.conversion.evidence.push('Aspetto maschile/femminile non risolto dai dati: scelta manuale disponibile, nessuna deduzione dal nome o dalla specie.');
  return report;
}
