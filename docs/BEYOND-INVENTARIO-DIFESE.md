# Beyond: inventario operativo e difese — v0.71

> Aggiornamento v0.75: [verifica rapida delle difese](DIFESE-VERIFICA-RAPIDA.md)
> sul dispositivo e sul PC. Completamento manuale dei tipi mancanti con anteprima
> e conferma; l’importazione continua a lasciarli sconosciuti. Formati invariati.


> Aggiornamento v0.72: [punteggi fissati, massimi e bonus delle caratteristiche](BEYOND-CARATTERISTICHE.md).
> Supportate concessioni esplicite riconciliate; valori derivati verificati anche nel parser C.
> Nessun nuovo formato. Restano restrizioni, origini e override non supportati; le prove datate sotto mantengono il proprio contesto.


27 ottobre 2026. Importazione dal PC con il connettore PocketD-D esistente,
senza Foundry, autenticazione privata o scritture verso Beyond. Il dispositivo
rimane il prodotto principale; il PC prepara e configura la scheda.

## Cosa cambia

- Le cariche esplicite di un singolo oggetto diventano massimo, saldo corrente
  e politica di recupero. La quantità resta indipendente dalle cariche.
- I consumabili dichiarati dal sorgente diventano utilizzabili. Le pozioni con
  una sola cura strutturata e supportata ricevono dadi e bonus, senza dedurli
  dal nome o dal testo. Usarle consuma una unità, dopo conferma.
- Resistenze, vulnerabilità e immunità ai danni incondizionate e riconciliate
  arrivano al profilo del dispositivo. L'assenza di una concessione non prova
  che un tipo sia normale: rimane **da verificare**.
- Anteprima Beyond ed esportazione mostrano i limiti. Nell'editor della scheda
  estesa sul PC si possono verificare i tredici tipi, indicare esplicitamente
  Normale, mantenere Da verificare o disattivare il trasferimento del profilo.
  Annullare non modifica nulla; un errore di salvataggio conserva la scheda
  precedente anche in memoria.

## Regole di riconciliazione

Cariche: servono `limitedUse.maxUses` e usi spesi espliciti e coerenti
(`numberUsed` / `chargesUsed`); massimo 999. Non si inventano saldi per pile
con più oggetti o formule non supportate. Solo `Short Rest` / `Long Rest`
senza altri dettagli ricevono un recupero automatico pieno. Alba, recuperi
variabili, descrizioni aggiuntive e codici sconosciuti restano manuali.
`minNumberConsumed` / `maxNumberConsumed` non generano costi di azioni:
il costo va configurato esplicitamente. Magie e azioni da oggetto con usi
restano annotazioni, evitando di duplicare il saldo fisico dell'oggetto.

Pozioni: la cura deve essere l'unica concessione `bonus / hit-points`, senza
condizioni, durata, sintonia o altri effetti. Accettati 1–6 dadi d4/d6/d8/d10/d12
e bonus 0–30, con campi concordanti; nessuna somma doppia dei bonus ripetuti.
Homebrew, formule più grandi e pozioni con effetti aggiuntivi restano
consumabili con cura manuale. Stati di consumo discordanti conservano
l'oggetto generico. Il valore dei PF non cambia durante lettura/importazione.

Difese: si controllano origine, scelta, livello della capacità, equipaggiamento
e sintonia. Per gli oggetti si riconciliano riepilogo e concessione originale,
compresi eventuali requisiti omessi dal riepilogo. Concessioni condizionali,
temporanee, duplicate discordanti o non risolte lasciano il tipo sconosciuto.
Immunità a una condizione (per esempio Avvelenato) non diventa immunità al
danno da veleno. Cambiare equipaggiamento o capacità richiede ricontrollare
il profilo: non è un motore completo delle regole Beyond.

Le difese si applicano nel percorso **Danni** del dispositivo. I pulsanti PF
del PC restano correzioni dirette. Sul dispositivo un tipo non verificato
richiede configurazione; immunità prevale, resistenza dimezza arrotondando
per difetto e vulnerabilità raddoppia. Resistenze uguali non si accumulano.

## Formati e conservazione dei dati

Il profilo opzionale usa backup PC **v16** e trasferimento **USB14**, richiesto
solo quando il profilo è presente, anche se tutti i tipi sono sconosciuti.
Le quattro maschere `configured`, `resistance`, `vulnerability`, `immunity`
hanno tredici bit; nessun flag può appartenere a un tipo non configurato.
I vecchi backup restano leggibili e, senza profilo, mantengono il formato
minimo precedente. Cariche e cure usano campi già supportati.

Firmware **0.71**, stato nativo **PDD33/PDDX** invariato: nessuna modifica alle
partizioni o migrazione NVS. USB14 viene rifiutato dai firmware precedenti
prima di inviare la scheda, senza perdere silenziosamente le difese.
Il trasferimento resta una copia confermata sul display; il salvataggio
riuscito richiede la risposta finale del dispositivo.

## Prova reale: personaggio 144777811

Lettura pubblica tramite `scripts/beyond-client.js`: HTTP **200**, **426.718 byte**,
struttura valida. Questi sono controlli diversi dalla correttezza della conversione.
Verifica dei valori rispetto ai dati letti:

| Dato | Risultato |
| --- | --- |
| FOR / DES / COS / INT / SAG / CAR | 8 / 18 / 14 / 18 / 10 / 10 |
| PF | 32 / 73, temporanei 0 |
| CA | 17 |
| Inventario riconciliato | 41 voci; quantità conservate |
| Potion of Healing | una voce ×4, cura esplicita 2d4+2 |
| Oggetti ricaricabili | nessuno nel sorgente della prova |
| Difese ai danni riconciliabili | nessuna nel sorgente della prova |

PF massimi: base 53 + contributo COS 20 = 73; 41 rimossi → 32 correnti.
Il pacchetto convertito è stato passato al **parser C reale sul PC**, poi al
codec nativo: caratteristiche, PF, CA, tutte le quantità e formula della
pozione coincidono. Questo PG usa USB9, non avendo un profilo difese o un
nuovo look impostato. Nessuna raccolta personale modificata, nessun invio
USB fisico e nessun JSON completo pubblicato.

**Conversione ancora parziale** (`conversionComplete: false`). La prova reale
non dimostra cariche o resistenze assenti dal personaggio.

## Prove sintetiche separate

- 221 test JS passati: saldi, zero, limiti, pile, formule, recuperi manuali, cure,
  concessioni condizionali/discordanti, equipaggiamento/sintonia/livelli,
  versioni e conservazione della raccolta.
- Parser e codec C: 91 combinazioni (13 tipi × 7 combinazioni R/V/I), danni
  dispari, precedenze, tipi sconosciuti, quantità e cariche; 12 pacchetti
  malformati o con versione incompatibile respinti. Anche con ASan/UBSan.
- Browser di produzione: anteprima senza scritture, consenso e nuovo PG,
  altre schede conservate, backup/reload, configurazione difese, annullamento,
  quota esaurita, esportazione USB14; desktop, 390 e 320 px.
- Firmware LVGL host: importazione in attesa, annullamento, mancato
  salvataggio e conferma con vecchia scheda preservata fino al commit;
  tutti i 118 pannelli esercitati. Anche con ASan/UBSan.
- Build ESP32-S3 riuscita. Sono prove sul PC, non del touch, del lettore SD,
  dell'alimentazione o del trasporto USB fisico.

## Aggiornare e usare

Scaricare il nuovo pacchetto PC dal solito installer, estrarlo e avviare
`Avvia-PocketDD.cmd`. Usare lo stesso indirizzo `http://127.0.0.1:5173/` e lo
stesso profilo browser per ritrovare la raccolta; prima esportarne un backup.
Non eliminare le schede esistenti: un nuovo import crea un altro personaggio,
non riconcilia automaticamente i progressi di quello precedente.

Per un profilo difese aggiornare il firmware scegliendo **Aggiorna PocketD-D**,
poi leggere Beyond → controllare anteprima/omissioni → confermare il nuovo PG
sul PC → Prepara per dispositivo → inviare via USB → confermare sul display.
Non dichiarare riuscito il trasferimento se manca la conferma finale.
Il problema fisico dei messaggi di avvio/riavvio USB segnalato dall'utente
rimane da diagnosticare insieme sul dispositivo: questo rilascio non lo
considera risolto.

## Cosa resta

Difese condizionali e immunità alle condizioni richiedono configurazione;
durate, proprietà speciali e costi condivisi non vengono ricostruiti da
semplice testo. Restano casi di origini opzionali, override personalizzati,
massimi delle caratteristiche, regole/azioni non riconciliate, pile con
cariche individuali, peso/contenitori e gestione automatica della sintonia.
CA e PF massimi sono valori riconciliati all'importazione, non si ricalcolano
integralmente a ogni modifica successiva. Importazione dal dispositivo,
autenticazione privata, aggiornamento dei progressi da Beyond e scrittura
verso Beyond rimangono successivi. Firmware e interfaccia supportano già
magie, slot, capacità e altri dati: non significa che ogni regola Beyond
venga convertita o automatizzata.
