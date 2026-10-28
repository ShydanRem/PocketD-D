# Beyond: personalizzazioni di abilità e salvezze — v0.73

28 ottobre 2026. Supporto ai campi standard riconciliati di `characterValues`,
senza Foundry, autenticazione privata, scritture a Beyond o modifica della
raccolta dell'utente. [Resoconto generale e roadmap](STATO-SOFTWARE.md).

## Campi e semantica

| Tipo sorgente | Bersaglio | Conversione |
| --- | --- | --- |
| 24 / 25 | Abilità standard identificata da `valueId` | Due extra distinti, sommati ai modificatori supportati della fonte |
| 26 | Abilità standard | 1 Nessuna, 2 Mezza, 3 Competenza, 4 Maestria; sostituisce il grado precedente |
| 39 / 40 | Salvezza, ID caratteristica 1–6 | Due extra distinti, sommati ai modificatori supportati della fonte |
| 41 | Salvezza | 1 Nessuna, 3 Competenza; mezza competenza e maestria non approssimate |

Si usano gli ID Beyond: per esempio Atletica è 2 e Percezione è 14, non gli
indici della lista locale. Il namespace numerico deve essere assente/null/zero;
un identificativo di entità oggetto non viene riutilizzato come bersaglio.
Altri namespace rimangono bloccati fino a riconciliazione.

Bonus: interi, anche negativi e zero, o stringhe di interi espliciti; nessuna
conversione di vuoto, booleani, decimali, formule o dadi. Il totale degli extra,
dopo i modificatori della fonte, deve stare in −30..30. Non si taglia un totale
per far riuscire la conversione. Duplicati dello stesso tipo/bersaglio e stesso
valore sono applicati una volta; valori discordanti bloccano l'importazione.

Un bonus `null` e una competenza `null`/0 sono non impostati. **Nessuna** (1)
è invece una scelta esplicita: rimuove competenza/maestria e mezza competenza
precedenti nell'abilità interessata. La scelta Mezza si arrotonda per difetto.
Mezza competenza sulle prove di caratteristica resta distinta e non si somma
nuovamente a una competenza piena. Un campo non impostato non cancella la
competenza ricavata dalla classe o da un altro effetto supportato.

Le personalizzazioni vengono applicate dopo la riconciliazione dei modificatori
originali. Non cancellano `blockedChecks`: un grado/bonus noto non risolve
un diverso effetto dinamico o un'origine ambigua.

## Provenienza e anteprima

Controllati anche origine e livello della capacità e, per gli oggetti, quantità,
equipaggiamento, sintonia, unicità del grant e corrispondenza del riepilogo.
Oggetti con quantità zero o inattivi non danno bonus ai tiri. Durate, formule o
requisiti non risolti marcano il tiro Da verificare, mantenendo il blocco anche
quando c'è una personalizzazione riconosciuta.

L'anteprima mostra totale e composizione anche per una rimozione esplicita di
competenza, mezza competenza o un extra senza competenza. Lettura e anteprima
non scrivono nella raccolta. Consenso e conferma creano una nuova scheda;
annullamento, errori di salvataggio e altri profili conservano le protezioni
esistenti. La conversione solo di base non dichiara supportate queste
personalizzazioni: richiedono il percorso esteso effettivamente usato dall'app.

Nessun nuovo campo nel salvataggio: gradi, bonus e maschere già presenti vengono
trasferiti nei formati precedenti pertinenti (USB9 senza altri dati più recenti,
backup PC v11). Stato nativo PDD33 e partizioni invariati. Non occorre aggiornare
il firmware solo per ricevere questi valori numerici già rappresentabili.

## Prove

- **240 test JS** complessivi: 18 ID, gradi, livelli, extra separati, somme,
  stringhe numeriche esatte, null/zero/nessuna, duplicati, errori, limiti,
  bersagli/namespace sconosciuti, oggetti inattivi, blocchi preservati,
  backup e raccolta; nessuna mutazione della fonte.
- **160 stati sorgente** attraverso convertitore, USB reale come formato,
  parser/codec C e calcolo nativo dei tiri: livelli 1–20, tutti i gradi delle
  abilità e i due gradi supportati delle salvezze. **14.400 confronti** di
  bonus/modalità, includendo Avvelenato e tiri da verificare; roundtrip nativo.
  Parser/codec su questi dati anche con ASan/UBSan.
- Browser di produzione: anteprima senza scritture, gradi rimossi e mezzi,
  extra, campi non impostati, casi bloccanti, annullamento/conferma, nuovo PG,
  altre schede conservate, reload, esportazione USB9 e layout 390/320 px.
  Ripetute le regressioni di import base/esteso e riconciliazione.
- Build ESP32-S3 riuscita; nessuna prova USB fisica in questo incremento.

**Prova reale separata**, 28 ottobre: rilettura 144777811 tramite connettore
PocketD-D, HTTP 200 e 426.718 byte, struttura valida. Rimangono caratteristiche
8/18/14/18/10/10, PF 32/73, CA 17, 41 voci e quattro cure 2d4+2. Le 30 prove
convertite coincidono nel calcolo C e nessuna è bloccata. Non ci sono bersagli
con le nuove personalizzazioni: il loro funzionamento è dimostrato dai test
sintetici, non da questo PG. Raccolta personale non aperta, nessun trasferimento
fisico e nessun JSON completo pubblicato.

## Limiti e riferimenti

Cambio di caratteristica associata (tipo 27), altri override numerici, abilità
non standard, namespace diversi e salvezze con gradi non rappresentabili
rimangono bloccati/non automatizzati secondo il caso. Non si trasformano in
bonus fissi per nasconderne i limiti. Condizioni/requisiti arbitrari, origini
opzionali e regole generali restano fuori dal perimetro. `conversion.complete`
rimane `false`.

Riferimenti di formato, non dipendenze esecutive, nel commit
`44bb451175da3e96dbfcb71547de59995655609e` di MrPrimate/ddb-importer:
[ID e gradi](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/config/dictionary/actor/actor.ts),
[abilità](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/skills.ts),
[salvezze](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/abilities.ts).
