# Collega Beyond al Pocket una volta

2 novembre 2026 · v0.92 · **percorso sperimentale, da collaudare sull’hardware**.
Il codice ora comprende associazione dell’account, elenco sul Pocket e lettura
anche autenticata delle schede. Le nuove prove usano sessioni e risposte
sintetiche: non attestano un login reale ESP32 o la lettura reale di un PG privato.

![UI firmware con account e codice sintetici](previews/v092-beyond-account.png)

## Prima associazione

1. Dal [sito PocketD-D](https://shydanrem.github.io/PocketD-D/) aggiorna il firmware
   alla **0.92 conservando i dati**. Collega il Pocket al Wi-Fi dalle impostazioni.
   Non cancellare flash o SD; non occorre reinstallare i look.
2. Scarica l’[estensione aggiornata](https://shydanrem.github.io/PocketD-D/downloads/PocketDD-Beyond-Check.zip),
   estraila e ricarica la cartella `extension` nella gestione estensioni Chrome/Edge.
   Sostituisci la copia precedente, senza tenere due ponti attivi.
3. Accedi normalmente a Beyond nello stesso profilo browser. Nell’estensione
   apri **Account Beyond e Pocket**, quindi **Prepara accesso**. Consenti il
   permesso opzionale cookie e l’accesso ai due servizi Beyond.
4. Collega il Pocket via USB, premi **Collega Pocket** e scegli la sua porta.
   Confronta il codice di sei cifre sul PC e sul display. Il codice non è una
   credenziale Beyond; la richiesta scade se non viene confermata.
5. Sul Pocket scegli **Ricorda accesso: SI/NO**, poi **Collega**. Solo dopo
   questa conferma l’estensione invia l’accesso. Il dispositivo verifica via Wi-Fi
   identità dell’account ed elenco; non basta la ricezione USB per riuscire.
6. Attendi **Pocket collegato** nel browser e l’elenco sul display. Solo allora
   il nuovo collegamento è confermato. Puoi scollegare e chiudere il PC.

La preparazione nel browser dura al massimo tre minuti. Il confronto sul Pocket
scade dopo due minuti; dopo la conferma attende il trasferimento per 30 secondi.
**Annulla** interrompe la richiesta; se la conferma era già partita, controlla
il display prima di riprovare. Non vengono ripetuti automaticamente gli invii
contenenti l’accesso. Una cancellazione arrivata dopo il commit non finge di
aver rimosso un collegamento già salvato.

## Uso autonomo

Pocket → **I miei PG → lente** apre Beyond. Se l’accesso è ricordato ma l’elenco
non è in RAM, si prova a caricarlo quando il Wi-Fi è pronto. Altrimenti **Rileggi**
carica/aggiorna l’elenco. Scegli il nome, attendi lettura/conversione, controlla
caratteristiche, PF, CA e limiti, quindi **Importa**. Non serve lasciare il PC,
l’estensione o il ponte locale accesi.

Le righe mostrano nome abbreviato, livello e ultime cifre dell’ID per distinguere
omonimi. Un elenco che il servizio dichiara parziale viene indicato come tale;
non si ripiega sui personaggi delle campagne e non si inventa completezza.
Il PG selezionato viene letto con un bearer appena ottenuto per lo stesso account.
Non si ripiega sulla lettura anonima se l’autenticazione fallisce.

L’importazione riusa la [conversione nativa e la conferma](BEYOND-DISPOSITIVO.md).
Con SD, la raccolta salva la scheda uscente prima di cambiarla; senza SD rimane
un solo PG attivo. Non sincronizza automaticamente progressi, PF, slot o aspetto
con Beyond. Offline puoi aprire i PG locali e giocare senza il servizio.

## Ricordare o rimuovere l’accesso

- **SI:** cookie di sessione e identità attesa sono salvati nella NVS del Pocket,
  in una chiave e namespace dedicati (`pdd_nvs/pdd_beyond/access`), con rilettura
  di verifica. Il bearer resta temporaneo in RAM; viene rinnovato a ogni richiesta.
- **NO:** il collegamento resta in RAM fino allo spegnimento. Un vecchio accesso
  persistente viene rimosso prima di confermare quello temporaneo, così non
  ricompare un altro account al riavvio.
- La memoria flash/NVS attuale **non è cifrata**. Chi può leggerla fisicamente può
  recuperare l’accesso ricordato. Non vengono attivate cifratura o eFuse durante
  l’aggiornamento. Il codice e il trasporto USB non offrono protezione contro
  un PC o firmware compromesso.
- Il cookie **non ha privilegi ridotti**: è una sessione del proprio account.
  Pocket esegue solo login tecnico e letture, ma il segreto va protetto come
  l’accesso al sito. Non copiarlo in messaggi, file, screenshot o diagnostica.
- Schede, raccolta SD, backup PG USB, pacchetti e repository non contengono queste
  credenziali. Un dump integrale della flash comprende invece anche la NVS.
- In Beyond sul Pocket: **Esci → Scollega** rimuove l’accesso e l’elenco in RAM,
  senza cancellare PG, progressi, Wi-Fi o il login del browser. Se il salvataggio
  o la rimozione hanno esito incerto, le operazioni account si bloccano fino al riavvio.
- Il collegamento non è permanente: sessioni scadute/revocate, 401/403 o cambi di
  account possono richiedere una nuova associazione. Un errore di rete non
  cancella le copie locali e non viene chiamato importazione riuscita.

## Controlli e limiti

| Parte | Verifica effettuata |
| --- | --- |
| Login/elenco reale browser | L’utente ha confermato elenco completo il 31 ottobre con l’estensione precedente. Non è una prova del nuovo trasferimento cookie |
| Preparazione/USB browser | Test JS e pagina Chromium con API cookie/serial/permessi simulate: consenso, cookie fisso, identità, cambio login, scadenza, codice prima del segreto, conferma, ricevuta finale e rifiuti; nessun segreto in DOM/localStorage |
| Protocollo/archiviazione C | Grammatica stretta e 20.000 richieste malformate, JWT/identità/scadenza, duplicati/schema, salvataggio/rilettura/rimozione del solo namespace con NVS simulata |
| Servizio nativo | Codice reale con HTTP/task simulati: cookie solo verso auth, bearer solo verso character-service, account atteso, elenco, lettura privata sintetica, RAM/persistenza, errori, annullamento tardivo e commit incerto |
| UI | 148 viste LVGL, compresi confronto codice, scelta memoria, elenco e scollegamento. PG e progressi invariati dall’associazione |
| Conversione | Stesso convertitore della v0.91, parità su sorgente pubblica reale già verificata; nessuna nuova lettura privata reale in questa release |
| Hardware | Nuovi cookie MV3, handshake USB, POST autenticato ESP32, elenco e PG privati reali, persistenza dopo riavvio e prestazioni **ancora da provare** |

HTTPS con verifica certificato/nome host e senza redirect: cookie al solo
`auth-service.dndbeyond.com/v1/cobalt-token`, bearer agli endpoint account/scheda
su `character-service.dndbeyond.com`. Identità e scadenza vengono lette dal token
ricevuto da quell’origine HTTPS; non è una verifica indipendente della firma JWT.
Nessun servizio Pocket cloud o Foundry obbligatorio e nessuna scrittura dei PG su Beyond.

Limiti: cookie 4 KiB, risposta auth 32 KiB, bearer 16 KiB, elenco 512 KiB/300 PG,
nomi/dettagli abbreviati con UTF-8 valido, profondità JSON account 32. Oltre limite
si rifiuta il risultato; la conversione della scheda mantiene i budget v0.91.
Gli endpoint non sono un contratto OAuth pubblico: la fattibilità reale dipende
anche dalla risposta Beyond all’ESP32. Non si aggirano verifiche del servizio.
