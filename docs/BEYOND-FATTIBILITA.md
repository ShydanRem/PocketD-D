# D&D Beyond: fattibilità per PocketD-D

Verifica del 16 ottobre 2026, sulla base di PocketD-D v0.12. Esito: **il codice di MrPrimate supporta la fattibilità tecnica di una lettura diretta. Il percorso richiesto è PocketD-D → Beyond, con un connettore nostro integrato nell’app di supporto. Autenticazione privata e scrittura su Beyond richiedono verifiche separate.** Nessuna integrazione è stata implementata o provata su un account in questa ricerca.

Requisito confermato dall'utente: importazione diretta dentro PocketD-D, senza Foundry, esportazioni intermedie o servizi MrPrimate obbligatori. MrPrimate è un riferimento tecnico per accesso e conversione. Il fatto che l'utente possieda Foundry non cambia questo requisito.

L'esperienza preferita è avviare l'importazione direttamente dal dispositivo. Se accesso, autenticazione o conversione risultassero troppo complessi sul firmware, è accettata l'importazione dall'app PocketD-D su PC e il successivo trasferimento della scheda al dispositivo. Il PC è quindi un'alternativa accettata, non un requisito definitivo del prodotto.

## Risultati verificati

Non è emersa una documentazione pubblica di API/OAuth Beyond per questo uso. Nel forum ufficiale una risposta dello staff del 2022, aggiornata nel 2023, escludeva una API pubblica e deroghe per progetti personali: è una posizione storica, non un annuncio del 2026. La documentazione di supporto attuale descrive l'esportazione della scheda in PDF, non un contratto di esportazione JSON per sviluppatori. [Risposta ufficiale](https://www.dndbeyond.com/forums/d-d-beyond-general/d-d-beyond-feedback/148262-can-i-use-the-api-for-free), [esportazione PDF](https://dndbeyond-support.wizards.com/hc/en-us/articles/7747193980820-Character-Header).

L'importatore di MrPrimate fornisce un percorso tecnico reale, distinto da un'API ufficiale. Importa personaggi pubblici o accessibili al DM della campagna, converte i dati e li inserisce in Foundry. Per dati riservati utilizza il cookie `CobaltSession` attraverso un proxy; il browser non può semplicemente riutilizzare quel flusso con un normale `fetch` da PocketD-D. [Personaggi](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/character), [configurazione e autenticazione](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/initial-setup).

Ho ispezionato, senza installarne dipendenze o avviarne server:

| Repository | Snapshot esaminato |
| --- | --- |
| [MrPrimate/ddb-importer](https://github.com/MrPrimate/ddb-importer/tree/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c) | `49f42d1f7ae64f27908ba727233b7f0abd0f3b7c`, package 7.5.8, commit dell'11 ottobre 2026 |
| [MrPrimate/ddb-proxy](https://github.com/MrPrimate/ddb-proxy/tree/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a) | `a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a`, commit del 14 ottobre 2026 |

Entrambi dichiarano MIT: eventuali porzioni di codice riutilizzate devono conservare avvisi e licenza. Gli asset non sono tutti coperti dalla stessa licenza: il README dell'importatore segnala, per esempio, risorse JB2A con licenza separata non commerciale. La licenza del codice non concede diritti sui contenuti Beyond né autorizza l'accesso ai suoi servizi. Nessun codice o asset dei due repository è stato copiato in PocketD-D. [Licenza importer](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/LICENSE.md), [licenza proxy](https://github.com/MrPrimate/ddb-proxy/blob/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a/LICENSE), [attribuzioni degli asset](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/README.md#attributions).

I termini generali Wizards, richiamati da Beyond, contengono nella sezione 2.2 restrizioni su raccolta automatizzata e connessioni di terzi non autorizzate. La diffusione di MrPrimate non dimostra un'autorizzazione applicabile a PocketD-D. Questo resta un vincolo da chiarire prima di distribuire un connettore diretto. La fattibilità tecnica e l'autorizzazione del servizio sono questioni distinte; la licenza MIT del proxy non risolve la seconda. [Termini Beyond, aggiornati il 25 giugno 2026](https://www.dndbeyond.com/en/terms-conditions), [termini Wizards §2.2](https://company.wizards.com/en/legal/terms).

## Percorso diretto e codice di riferimento

### Dispositivo e PC

| Modalità | Obiettivo e verifica necessaria |
| --- | --- |
| Dispositivo → Beyond tramite Wi-Fi | Esperienza preferita per una scheda pubblica, se praticabile. Verificare HTTPS/TLS, dimensione e parsing delle risposte, conversione dei valori, input del codice/URL e salvataggio recuperabile. Non ancora implementata o provata sull'hardware. |
| Dispositivo → backend PocketD-D → Beyond | Possibile importazione comandata dal dispositivo, con conversione svolta dal nostro servizio. Non equivale all'accesso autonomo: richiederebbe rete e backend disponibile. Da valutare se il firmware non può gestire tutto. |
| App PocketD-D su PC → Beyond → trasferimento al dispositivo | Alternativa accettata. Consente di verificare accesso e calcoli prima della disponibilità dell'hardware; il metodo di trasferimento USB/Wi-Fi resta da definire e provare. |

L'importazione autonoma dal dispositivo e quella comandata dal dispositivo attraverso un backend sono due possibilità distinte, entrambe ancora da verificare. Per le schede private rimane aperto il flusso di autenticazione. I vincoli CORS riguardano il browser; nel firmware occorre invece verificare trasporto HTTPS, risorse e gestione degli accessi.

Il primo prototipo di accesso e conversione può essere verificato sul PC. Si manterranno separati lettura remota, normalizzazione e formato locale, per valutare in seguito quali parti eseguire sul dispositivo. Una volta caricata, la scheda deve restare utilizzabile offline per il gioco individuale.

| Funzione in PocketD-D | Valutazione |
| --- | --- |
| URL di un personaggio pubblico → connettore interno → anteprima | Primo prototipo proposto. Il codice pubblico mostra il meccanismo di lettura; accesso dal vivo e conversione esatta rimangono da verificare. |
| Lettura di un personaggio privato | Richiede autenticazione Beyond: non basta l'URL e la sessione del sito non è automaticamente disponibile alla nostra app. |
| Aggiornamento da PocketD-D verso Beyond | Traguardo successivo, con autorizzazioni, identificativi degli oggetti, conflitti e limiti del servizio da verificare. |
| JSON locale → convertitore | Fixture di sviluppo per verifiche ripetibili, senza diventare un passaggio obbligatorio per l'utente. |

Dal proxy di MrPrimate possiamo studiare il trasporto HTTP e la forma delle risposte; dal parser possiamo studiare i calcoli e valutare riuso mirato nel rispetto della licenza. Il parser è legato a `foundry`, `game`, al sistema D&D5e e ai suoi documenti: occorre adattare o riscrivere la conversione per il nostro modello. Il prodotto non richiederà di installare questi componenti. [Proxy](https://github.com/MrPrimate/ddb-proxy/tree/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a), [parser](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/DDBCharacter.ts).

Il rischio dei valori derivati è osservabile nel codice: la generazione dei PF può lasciare `max: null`, delegando il calcolo a Foundry; la CA include armature, scudi ed effetti. Il nostro convertitore dovrà ricostruire i valori dai dati Beyond, gestendo modificatori, equipaggiamento e override, e confrontarli con la scheda originale. La sola rinomina delle chiavi JSON non è sufficiente. [Generazione PF](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/character/hp.ts), [generazione CA](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/character/ac.ts).

## Limiti attuali di PocketD-D

Il formato locale v9 è più semplice della scheda Beyond/Foundry. Un importatore deve dichiarare ciò che non rappresenta, senza scartare o indovinare dati in silenzio.

| Dati | Situazione nella v0.12 |
| --- | --- |
| Nome, livello, sei caratteristiche, PF, CA | Rappresentabili se risolti e nei limiti del formato. Nome massimo 24 caratteri, livello 1–20, caratteristiche 1–30, PF massimi 999, CA 1–30. |
| Abilità e salvezze | Competenze/maestrie standard presenti; mancano mezza competenza, bonus specifici e sostituzioni della caratteristica di un'abilità. Copiare solo il grado può produrre tiri diversi dall'origine. |
| Inventario e armi | Massimo 60 voci e una sola arma equipaggiata. Armature come oggetti non ricalcolano la CA; formule condizionali, proprietà avanzate e consumi non sono tutti rappresentabili. |
| Magie e capacità | Massimo 40 voci, descrizioni fino a 600 caratteri e slot ordinari. Mancano slot del patto separati, più caratteristiche da incantatore e il modello completo delle attività Foundry. |
| Stato della sessione | Mancano PF temporanei, condizioni generiche, tiri morte e dadi vita. Il solo flag Avvelenato non rappresenta tutte le condizioni. |
| Identità e regolamento | Mancano provenienza esterna persistente, identificativi di collegamento e scelta esplicita 2014/2024. Necessari prima di aggiornamenti riconciliati. |

Questi limiti risultano da `src/model.js`, `src/inventory.js` e `src/powers.js` di PocketD-D. L'import iniziale deve creare una nuova scheda neutra, senza oggetti dimostrativi, mostrare valori e omissioni e richiedere conferma prima di aggiungerla alla raccolta. Un aggiornamento futuro dovrà distinguere definizione importata e progressi locali, evitando di sovrascrivere PF, consumi o party senza una scelta esplicita.

## Sincronizzazione e primo traguardo consigliato

La documentazione MrPrimate riserva il ritorno verso Beyond ai sostenitori Patreon. La sincronizzazione dinamica richiede un livello dedicato, un GM con permessi sulle schede e collegato durante le modifiche; non copre tutte le operazioni. La creazione completa del personaggio e gli avanzamenti di livello non sono un obiettivo di quel ritorno. [Configurazione delle funzioni](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/initial-setup), [Dynamic Sync](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/dynamic-sync), [aggiornamento della scheda](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/character#update-dd-beyond).

Il proxy pubblico esaminato espone letture di personaggi e contenuti, non implementa le rotte di aggiornamento usate dal modulo. È dichiarato per uso individuale, con funzionalità ridotte. Installarlo non fornisce automaticamente sincronizzazione bidirezionale né sostituisce i servizi Patreon. [Proxy pubblico](https://github.com/MrPrimate/ddb-proxy/tree/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a), [chiamate di aggiornamento](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/updater/character.ts).

**Primo traguardo diretto proposto:** incollare l'URL di un proprio personaggio pubblico in PocketD-D, leggere i dati tramite il connettore interno, vedere anteprima e omissioni e aggiungerlo come nuova scheda. L'accesso a personaggi privati è un traguardo distinto: il codice di riferimento utilizza una sessione Beyond, ma non è stata trovata un'autenticazione OAuth pubblica da promettere come pulsante di accesso. L'app non può leggere automaticamente il cookie Beyond da un'altra origine.

Per il prototipo da PC il connettore HTTP farà parte dell'app di supporto, sul computer o su un backend PocketD-D; gestirà richieste e vincoli CORS senza imporre un servizio proxy di terzi. Il solo HTML statico attuale non copre questa architettura. Questo prototipo non decide ancora dove risiederà il connettore della modalità dispositivo. Il trasporto risolve il collegamento, non l'autenticazione o i diritti di accesso. La conversione resta separata dal trasporto per poter essere verificata con fixture locali.

Criteri di verifica del prototipo: URL accettati soltanto per i formati Beyond previsti, destinazione remota fissa, errori espliciti per scheda privata/inesistente o servizio non disponibile, limiti di tempo e dimensione, nessuna scrittura della raccolta prima dell'anteprima e della conferma. Testare almeno un personaggio marziale, un incantatore e un caso con bonus/effetti o regole miste. Confrontare valori, inventario, slot e omissioni con la scheda Beyond. Conservare la provenienza per i futuri aggiornamenti; dopo l'importazione il personaggio deve restare utilizzabile localmente. Nessun cookie o token deve entrare nei backup, nel repository o nel dispositivo.

Questa verifica ha esaminato fonti e codice. Non ha effettuato accessi a Beyond, importazioni reali, chiamate al proxy o prove con Foundry installato. Il funzionamento completo del connettore diretto resta da dimostrare su una scheda dell'utente; lo stato dell'app rimane v0.12.
