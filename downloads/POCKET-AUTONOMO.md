# Pocket PC · prima associazione Beyond

**v0.115 — inizia da qui:**
[Apri Associa Beyond sul PC](https://shydanrem.github.io/PocketD-D/#beyond).

1. **Sul Pocket:** collega il Wi-Fi da Menu → ingranaggio → Wi-Fi → Cerca reti.
   Poi lascialo sulla schermata del personaggio e collega il cavo USB dati.
2. **Sul PC, Chrome/Edge:** accedi a Beyond nello stesso browser. Se stai leggendo
   questo file dallo ZIP, estrailo tutto e carica la cartella `extension` in
   `chrome://extensions` o `edge://extensions`, con Modalità sviluppatore attiva.
3. Torna al sito Pocket e premi **Associa Beyond**. Autorizza l’accesso se richiesto,
   poi **Scegli Pocket USB e collega**. Seleziona la porta del dispositivo.
4. **Il codice si apre da solo sul Pocket.** Confronta le sei cifre, scegli se
   ricordare l’accesso e premi **Collega sul display**. Attendi **Beyond collegato**
   nella pagina PC prima di scollegare il cavo.
5. Sul Pocket: **Menu → I miei PG → Beyond → scegli PG → anteprima → Importa**.
   Con accesso ricordato e ancora valido, i successivi import richiedono solo Wi-Fi.

Anche il pulsante dell’estensione **Associa Beyond** apre questo percorso.
**Backup e look SD** apre gli altri strumenti. Il ponte sperimentale e il suo
codice a nove cifre non servono per questa associazione.

[Guida completa e recupero](https://github.com/ShydanRem/PocketD-D/blob/main/docs/COLLEGAMENTO-BEYOND.md).
L’accesso ricordato è opzionale e la memoria attuale non è cifrata: chi legge la
flash può recuperarlo. Nessuna credenziale viene salvata nell’estensione,
nelle schede, sulla SD o nei backup PG. Le nuove prove browser/firmware usano
risposte simulate; **nessuna nuova associazione reale ESP32 è stata confermata**.

## Stato dell’integrazione Beyond

Il dispositivo è il prodotto. Prima associazione da PC, poi lettura e conversione
sul Pocket via Wi-Fi. L’anteprima e la conferma d’importazione restano necessarie;
account, lettura HTTP, formato e correttezza dei valori sono controlli distinti.

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
