> v0.93: [Pocket PC](https://github.com/ShydanRem/PocketD-D/blob/main/docs/POCKET-PC.md) riunisce Beyond, SD e backup. Non serve più preparare l’accesso separatamente: prima autorizzazione, poi Collega Beyond e conferma sul display.

# Pocket autonomo: stato dell’integrazione Beyond

**v0.94, 3 novembre:** [connessione e recupero sul dispositivo](AUTONOMIA-RECUPERO.md).
Richiesta ripresa dopo il Wi-Fi, riprova per ID e accesso da ricollegare distinto
dagli errori di rete. L’estensione resta la scelta per la prima associazione;
nessuna lettura automatica sostituisce l’anteprima e la conferma d’importazione.
Nuovi percorsi verificati con rete/account sintetici, da collaudare sull’ESP32.

2 novembre 2026 · v0.92. Il dispositivo è il prodotto: il PC serve per
installazione/aggiornamenti e per la prima associazione dell’account; poi il
codice prevede scelta e importazione del PG dal Pocket tramite Wi-Fi.
**Questo nuovo percorso deve ancora essere collaudato con l’account reale e l’ESP32.**

[Guida al collegamento, alla memoria opzionale e ai limiti](https://github.com/ShydanRem/PocketD-D/blob/main/docs/COLLEGAMENTO-BEYOND.md).
[Pagina di installazione](https://shydanrem.github.io/PocketD-D/).

## Che cosa è implementato

1. **Account ed elenco browser:** verifica opzionale nell’estensione, già
   confermata dall’utente il 31 ottobre: i PG visibili coincidono tutti con
   «I miei personaggi». Nessun cookie o JSON reale acquisito dall’agente.
2. **Raccolta locale v0.90:** fino a 32 copie native su SD, progressi/aspetto/storico
   separati, checkpoint prima dei cambi, doppie copie verificate. Senza SD resta
   un PG attivo. USB/installer/asset SD precedenti confermati dall’utente; nuove
   operazioni della raccolta e interruzioni fisiche hanno collaudo separato.
3. **Conversione nativa v0.91:** regole PC eseguite localmente in QuickJS isolato
   e parser C finale. Intero record identico su sorgente pubblica reale e su 67
   casi sintetici accettati; altri 7 bloccati. Non basta un GET riuscito.
4. **Associazione e sessione v0.92:** preparazione esplicita nel browser, cookie
   opzionale, codice USB e conferma fisica prima del trasferimento, verifica
   indipendente dell’identità sul Pocket. Accesso ricordato solo se scelto;
   namespace separato da PG/SD/backup, NVS non cifrata dichiarata, cancellazione
   e ricollegamento. Nessuna password da digitare sul display.
5. **Scelta account v0.92:** elenco proprio sul Pocket, rilettura e schede
   autenticate, anteprima con valori/limiti e conferma. Non usa elenchi campagna
   né fallback anonimi. Se Beyond non risponde, i PG locali restano giocabili.

## Come partire quando si potrà provare

Aggiorna il firmware conservando i dati e ricarica l’estensione aggiornata;
collega il Pocket al Wi-Fi. Popup → **Apri Pocket PC → Collega Beyond**. Confronta il codice sul display, scegli se ricordare l’accesso
in memoria non cifrata e conferma. Solo la ricevuta finale conferma l’associazione.
Poi scegli il PG direttamente dal Pocket e verifica anteprima/limiti prima di importare.
La guida collegata sopra contiene recupero dagli errori e scollegamento.

## Distinzione fra prove

- **Reale riferita dall’utente:** login ed elenco completo nel browser, 31 ottobre.
- **Reale eseguita dall’agente:** lettura pubblica con connettore PC e parità
  completa del convertitore nativo nella v0.91; nessuna modifica alla raccolta.
- **Sintetiche v0.92:** cookie/token, permessi, seriale, risposte auth/lista/scheda,
  NVS e task. UI di produzione e controlli memoria; non sono accessi privati reali.
- **Da fare:** nuovo permesso cookie su MV3 reale, codice USB su hardware,
  autenticazione diretta ESP32, elenco/PG privati reali, rinnovo dopo riavvio,
  scadenza/revoca reali, memoria totale e fluidità. Il Chromium gestito non
  permette di caricare la MV3: qui non si aggira quella policy.

Gli endpoint non sono un OAuth pubblico per Pocket. Nessuna promessa di sessione
permanente: logout/revoca possono richiedere un nuovo collegamento. Il cookie
mantiene i privilegi del proprio account, anche se il software esegue solo
letture. Non pubblicare credenziali o JSON completi. L’estensione non salva
cookie/token, non li mostra e non li invia al ponte locale o a server Pocket.

La sincronizzazione di danni, tiri e altri cambi verso Beyond resta un lavoro
separato: questo percorso non scrive i personaggi sul sito.

## Fonti primarie di codice esaminate

- [MrPrimate, scambio sessione → bearer](https://github.com/MrPrimate/ddb-proxy/blob/main/auth.js).
- [iamjameslennon/ddb-mcp, token e claim account](https://github.com/iamjameslennon/ddb-mcp/blob/main/src/session-fetch.ts).
- [Stesso progetto, elenco e struttura `data.characters`](https://github.com/iamjameslennon/ddb-mcp/blob/main/src/tools/character.ts).
- [AlexWorland, definizione dell'endpoint](https://github.com/AlexWorland/dndbeyond-mcp/blob/main/src/api/endpoints.ts).
  Il suo strumento `listCharacters` invece usa campagne: non è la prova di una
  raccolta completa di PG propri, e non è il ripiego adottato da Pocket.

Nessuna dipendenza da questi progetti o da Foundry; nessun loro sorgente o asset
copiato. Implementazione Pocket specifica, limitata alla prova di sola lettura.
