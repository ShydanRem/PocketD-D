# D&D Beyond: fattibilità per PocketD-D

Verifica del 16 ottobre 2026, sulla base di PocketD-D v0.12. Esito: **importazione in sola lettura tecnicamente fattibile; prima scelta un ponte da Foundry con valori già calcolati. Accesso diretto e scrittura su Beyond richiedono una valutazione separata.** Nessuna integrazione è stata implementata o provata su un account in questa ricerca.

L'utente conferma di utilizzare già Foundry con DDB Importer su un altro PC. Il percorso consigliato usa quell'installazione per esportare il file e trasferirlo all'app di supporto; non richiede che Foundry sia presente nell'ambiente cloud di sviluppo. Le versioni effettive di Foundry, sistema D&D5e e modulo restano da rilevare prima della prova del bridge.

## Risultati verificati

Non è emersa una documentazione pubblica di API/OAuth Beyond per questo uso. Nel forum ufficiale una risposta dello staff del 2022, aggiornata nel 2023, escludeva una API pubblica e deroghe per progetti personali: è una posizione storica, non un annuncio del 2026. La documentazione di supporto attuale descrive l'esportazione della scheda in PDF, non un contratto di esportazione JSON per sviluppatori. [Risposta ufficiale](https://www.dndbeyond.com/forums/d-d-beyond-general/d-d-beyond-feedback/148262-can-i-use-the-api-for-free), [esportazione PDF](https://dndbeyond-support.wizards.com/hc/en-us/articles/7747193980820-Character-Header).

L'importatore di MrPrimate fornisce un percorso tecnico reale, distinto da un'API ufficiale. Importa personaggi pubblici o accessibili al DM della campagna, converte i dati e li inserisce in Foundry. Per dati riservati utilizza il cookie `CobaltSession` attraverso un proxy; il browser non può semplicemente riutilizzare quel flusso con un normale `fetch` da PocketD-D. [Personaggi](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/character), [configurazione e autenticazione](https://docs.ddb.mrprimate.co.uk/docs/ddb-importer/initial-setup).

Ho ispezionato, senza installarne dipendenze o avviarne server:

| Repository | Snapshot esaminato |
| --- | --- |
| [MrPrimate/ddb-importer](https://github.com/MrPrimate/ddb-importer/tree/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c) | `49f42d1f7ae64f27908ba727233b7f0abd0f3b7c`, package 7.5.8, commit dell'11 ottobre 2026 |
| [MrPrimate/ddb-proxy](https://github.com/MrPrimate/ddb-proxy/tree/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a) | `a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a`, commit del 14 ottobre 2026 |

Entrambi dichiarano MIT: eventuali porzioni di codice riutilizzate devono conservare avvisi e licenza. Gli asset non sono tutti coperti dalla stessa licenza: il README dell'importatore segnala, per esempio, risorse JB2A con licenza separata non commerciale. La licenza del codice non concede diritti sui contenuti Beyond né autorizza l'accesso ai suoi servizi. Nessun codice o asset dei due repository è stato copiato in PocketD-D. [Licenza importer](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/LICENSE.md), [licenza proxy](https://github.com/MrPrimate/ddb-proxy/blob/a181c5433df9f7a6765e38bc7fc1b4b7d9e0c17a/LICENSE), [attribuzioni degli asset](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/README.md#attributions).

I termini generali Wizards, richiamati da Beyond, contengono nella sezione 2.2 restrizioni su raccolta automatizzata e connessioni di terzi non autorizzate. La diffusione di MrPrimate non dimostra un'autorizzazione applicabile a PocketD-D. Questo resta un vincolo da chiarire prima di distribuire un connettore diretto; non impedisce di progettare il formato locale e un convertitore di file forniti dall'utente. Un ponte Foundry riduce le dipendenze tecniche di PocketD-D da Beyond, ma non sana automaticamente i diritti sul contenuto esportato. [Termini Beyond, aggiornati il 25 giugno 2026](https://www.dndbeyond.com/en/terms-conditions), [termini Wizards §2.2](https://company.wizards.com/en/legal/terms).

## Confronto dei percorsi

| Percorso | Valutazione |
| --- | --- |
| Beyond → MrPrimate/Foundry → snapshot locale → PocketD-D | **Preferito se l'utente usa Foundry.** Riutilizza la conversione esistente; PocketD-D riceve un file senza credenziali e funziona poi offline. Richiede un piccolo esportatore e un importatore nostro. |
| Esportazione standard Actor JSON di Foundry → PocketD-D | Possibile per un sottoinsieme. Serve verificare campi e versioni: il file può contenere dati base che Foundry completa durante l'esecuzione. Non promettere equivalenza alla scheda visualizzata. |
| JSON Beyond già ottenuto dall'utente → PocketD-D | Possibile senza Foundry, ma bisogna implementare calcoli e conversione. Il canale con cui si ottiene quel file resta da definire; non è stato trovato un export JSON ufficiale supportato. |
| URL del personaggio → proxy → PocketD-D | Tecnicamente plausibile dal codice pubblico; non verificato dal vivo, dipende da endpoint interni, accessibilità della scheda, autenticazione e condizioni d'uso. Non è il percorso iniziale consigliato. |
| PocketD-D → Beyond | Da rinviare: scritture, identificativi degli oggetti, autorizzazioni, conflitti e limiti del servizio richiedono lavoro aggiuntivo. |

Foundry documenta l'esportazione Actor JSON. L'importatore espone inoltre l'hook `ddb-importer.characterProcessDataComplete`, con l'Actor aggiornato: un punto utile per un futuro esportatore, senza modificare MrPrimate. Il parser è legato a `foundry`, `game`, al sistema D&D5e e ai suoi documenti; non è una libreria pronta da inserire nella nostra app Vite. [Export Foundry](https://foundryvtt.com/article/actors/), [hook e API del modulo](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/README.md#ddb-importer-hook-events), [parser](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/DDBCharacter.ts).

Il rischio dei valori derivati è osservabile nel codice: la generazione dei PF può lasciare `max: null`, delegando il calcolo a Foundry; la CA include armature, scudi ed effetti. Il bridge dovrà leggere esplicitamente i valori preparati dell'Actor e confrontarli con la scheda visualizzata. Non basta rinominare alcune chiavi JSON. [Generazione PF](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/character/hp.ts), [generazione CA](https://github.com/MrPrimate/ddb-importer/blob/49f42d1f7ae64f27908ba727233b7f0abd0f3b7c/src/parser/character/ac.ts).

La serie del modulo va scelta insieme alle versioni Foundry/D&D5e: il catalogo distingue, fra le altre, 7.1.x per Foundry 13/D&D5e 5.3.x e 7.5.x per Foundry 14/D&D5e 6.0.x. Il `module-template.json` dello snapshot esaminato indica minimo Foundry 14.367 e D&D5e 6.0.3; non è una verifica di funzionamento sulle altre combinazioni. [Matrice di compatibilità](https://foundryvtt.com/packages/ddb-importer).

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

**Prossimo traguardo proposto:** esportazione locale in sola lettura, anteprima e importazione come nuovo personaggio. Prima definire uno snapshot versionato con valori risolti e segnalazioni; poi costruire l'adattatore per la versione Foundry dell'utente, se disponibile. Testare almeno un personaggio marziale, un incantatore e un caso con bonus/effetti o regole miste. Confermare valori, inventario, slot, omissioni, isolamento della raccolta e uso offline del file. Nessun cookie o token deve entrare nello snapshot, nel repository o nel dispositivo.

Questa verifica ha esaminato fonti e codice. Non ha effettuato accessi a Beyond, importazioni reali, chiamate al proxy o prove con Foundry installato. Il funzionamento completo del futuro ponte resta da dimostrare su una scheda dell'utente; lo stato dell'app rimane v0.12.
