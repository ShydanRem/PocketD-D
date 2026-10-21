# Cariche degli oggetti — v0.41

Un oggetto può avere un saldo di cariche distinto dalla quantità. Il PC configura
il pool; il dispositivo lo consulta e lo aggiorna offline, senza Foundry.
Nessuna nuova lettura Beyond è stata eseguita per questo incremento.

## Configurazione e significato

In **Gestisci inventario e armi → Modifica → Gestisci cariche** scegli:

- saldo attuale 0–999 e massimo 1–999, con attuale ≤ massimo;
- recupero **Manuale**, **Riposo breve o lungo** oppure **Riposo lungo**.

Il pool appartiene alla **voce dell'inventario**, anche se quantità > 1. Per
copie con saldi separati crea voci distinte. Disattivare Gestisci cariche rimuove
il pool dopo Salva oggetto; annullare la modifica conserva la scheda.
I riposi ammessi riempiono il saldo fino al massimo. Alba, ricariche casuali,
recuperi parziali, distruzione all'ultima carica e altre eccezioni sono manuali.
La conversione Beyond non ricava ancora cariche da `limitedUse` o descrizioni:
la presenza di questo modello non rende completa l'importazione del personaggio.

## Sul dispositivo

**Usi → Zaino → oggetto → Cariche** mostra saldo, massimo e recupero. I comandi
−10/−1/+1/+10 preparano il nuovo saldo, limitato a 0–massimo. **Anteprima** mostra
prima/dopo; **Conferma** salva. Indietro scarta la bozza senza modifiche.
**Annulla** sull'Eroe ripristina l'ultima modifica salvata.

Quantità, PF ed effetti non cambiano quando correggi il saldo. Usare un
consumabile o una cura continua a consumare una unità, senza spendere cariche.
Con quantità zero il saldo resta consultabile, ma correzione, spesa e recupero
al riposo sono bloccati. Un oggetto assente non viene ricreato dal riposo.
Per gestire eccezioni o quantità occorre la configurazione PC; non c'è ritorno
automatico dei progressi dal dispositivo al PC.

I riposi mostrano le unità di carica recuperate nell'anteprima, separatamente
dalle voci con utilizzi recuperati. Conferma salva insieme tutte le modifiche;
Indietro e un errore di salvataggio conservano lo stato precedente.

## Costi dei profili di riduzione

In **Danni → Riduci → Profili → Modifica → Dettagli → Costi**, la fonte
**cariche N** seleziona una riga precisa dello zaino. Il nome dell'oggetto viene
mostrato sotto il selettore. Non occorre che l'oggetto sia l'arma equipaggiata;
equipaggiamento richiesto e altre condizioni di utilizzo restano decisioni del DM.
Il costo è 1–massimo. Nessuna sostituzione con un altro oggetto o pool.

Saldo insufficiente, quantità zero, reazione già usata o altre informazioni
necessarie mancanti bloccano l'intero colpo prima di RNG e scritture. Quantità
zero mostra **Oggetto non disponibile**, anche con saldo residuo sufficiente.
Un profilo configurato rimane definito quando l'oggetto si esaurisce, ma non è
utilizzabile finché non torna disponibile.

Se il profilo viene scelto e l'impatto confermato, cariche, reazione, PF,
temporanei e concentrazione sono un unico candidato. Come per gli altri costi,
la spesa scelta avviene anche con danno finale zero. Il resoconto conserva nome,
costo e saldi originali. Dopo un errore di salvataggio di un tiro COS, Riprova
salva lo stesso candidato senza rilanciare o spendere una seconda volta.
Questa funzione non lancia incantesimi dagli oggetti e non applica effetti
arbitrari: la spesa manuale è separata dalla risoluzione delle loro regole.

## Formati e compatibilità

| Formato | Nuova scrittura | Lettura precedente |
| --- | --- | --- |
| Backup PC | v11, per schede base ed estese | v1–v10 conservati; nessun pool inventato |
| USB | v8, ogni oggetto include `charges` | firmware legge v1–v7 senza cariche |
| NVS | PDD21, tag `PDDL` | PDD1–PDD20 letti senza cariche e senza scritture al boot |

PC: campo opzionale `charges: {current, max, recovery}`. USB8: campo obbligatorio
`charges: {current, maximum, recovery}`; recovery 0 manuale, 1 breve/lungo, 2 lungo.
Pool assente sul filo: tre zeri. Valori frazionari, fuori intervallo, membri
mancanti/duplicati/sconosciuti e saldo oltre il massimo vengono rifiutati.
Le vecchie versioni USB non accettano il nuovo campo. L'uploader attende
`PDD READY 8` per inviare v8 e blocca il payload verso firmware precedente,
prima dell'invio; nessuna rimozione silenziosa delle cariche.

PDD21 aggiunge 300 byte: 60 righe da saldo uint16 LE, massimo uint16 LE e
recupero uint8, prima del CRC. Dimensione **8829 + 552 × voci magia**, massimo
**75069 byte**. Le righe inutilizzate devono avere pool nullo. I tag PDD17–20
rifiutano il nuovo tipo costo 5. Layout partizioni invariato; le vecchie banche
restano conservate. Dopo una scrittura PDD21 usa firmware v0.41 o successivo:
un downgrade non deve essere aggirato cancellando NVS. Conserva i backup
precedenti per poter tornare a una versione PC precedente a v11.

## Verifiche eseguite e limiti

Tutte le sorgenti sono sintetiche; nessuna raccolta personale modificata.

- 144 test JavaScript; build Vite e HTML autonomo riuscite.
- 17.280 casi C su posizione, capacità, saldo, quantità, fase, reazione e danno;
  3.660 confronti PC → USB8 → parser C/riposo con 30 schede da 60 oggetti.
- Riposi con recupero fino a 59.940 cariche, oggetti assenti, clamp INT_MIN/MAX,
  candidato COS unico, rifiuto prima di RNG e tutti i 300 byte controllati con CRC valido.
- Fixture PDD20 prodotta dall'encoder originale v0.40 `6604bad`: lettura senza
  scritture al boot, primo salvataggio PDD21 e riavvio con cariche conservate.
- Intera suite host; AddressSanitizer/UndefinedBehaviorSanitizer su core/parser,
  storage, cariche, riposi, costi, tempo e UI (leak detector disattivato).
- LVGL host: conferma/annullamento, errore/retry, annulla Eroe, consumo quantità
  distinto, oggetto assente, nome di 40 caratteri, riga 60, saldi massimi,
  costi e ricevuta congelata, riposi, touch nel cerchio e scorciatoie protette.
- Chromium isolato: configurazione, bozza scartata, saldo invalido rifiutato,
  salvataggio/reload/rimozione pool, altra scheda identica, viewport 320/390 px;
  anteprima/download USB senza modifiche alla raccolta. Nessun errore JS.
- ESP32-S3 compilato con ESP-IDF 5.5.5 e LVGL 9.4.0; misure in [FIRMWARE.md](FIRMWARE.md).

Anteprime nell'archivio: `firmware-host-preview/item-charges-preview.png` e
schermate `charges-*.png`, ottenute dal firmware compilato per host.
Nessun flash o collaudo fisico di display, tasti, touch, USB, batteria o
alimentazione. Lettura reale Beyond e test sintetici restano evidenze distinte.
