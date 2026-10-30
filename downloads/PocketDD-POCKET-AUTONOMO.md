# Pocket autonomo: obiettivo e primo controllo

30 ottobre 2026 · v0.89.1. Il dispositivo è il prodotto. Il traguardo concordato
è usare il PC per installazione/aggiornamento e, se necessario, per il primo
collegamento Beyond; poi scegliere e importare un PG dal Pocket tramite Wi-Fi.
Questo documento non dichiara già disponibile tale importazione.

## Esperienza da ottenere

1. Al primo avvio: «Gioca», «I miei PG», «Impostazioni», con icone comuni e
   brevi etichette. Personaggio dimostrativo subito utilizzabile senza rete.
2. Prima associazione: login nel browser sul sito Beyond, verifica dell'account
   e conferma del Pocket riconoscibile. Nessuna password da digitare sul display.
   È un percorso da implementare, non un OAuth ufficiale già disponibile.
3. Sul Pocket: «I miei PG» → scegli avatar/nome → anteprima → «Importa e gioca».
   Non ripetere l'associazione per ogni PG; al cambio di sessione chiedere di
   ricollegare l'account, senza perdere le copie locali.
4. Avvii successivi: ultimo PG immediatamente giocabile offline. Wi-Fi per
   cercare/importare/aggiornare, non per poter aprire una scheda o tirare.
5. Personalizza dal PG, con anteprima sullo stesso avatar. Comandi fisici locali
   coerenti con il touch e con i tasti configurabili; indietro sempre prevedibile.

Le conferme proteggono associazione, sostituzione e salvataggi. Avvisi estesi
in un dettaglio, errori bloccanti subito visibili; niente nuova accettazione
generica delle omissioni a ogni azione. Un errore di rete non deve apparire come
un'importazione riuscita né sostituire il personaggio attivo.

## Cosa c'è in questa versione

Nell'estensione esistente compare **Verifica account Beyond**. Apre una pagina
separata: accesso opzionale ai soli servizi auth/character richiesto dal browser
quando si preme **Verifica accesso**. Il ponte PC può rimanere spento.

- Il browser invia la propria sessione all'URL HTTPS fisso di autenticazione.
  PocketD-D non estrae cookie e non usa l'API `chrome.cookies`.
- Il token breve rimane nelle variabili della verifica, senza cache persistente.
  L'identificativo account deriva dallo stesso token; non da un ID inserito
  dall'utente o dai personaggi di una campagna.
- Si legge solo l'elenco, con bearer e senza cookie verso character-service.
  Non si leggono schede complete, non si scrive su Beyond e non si invia niente
  al Pocket o al ponte locale. Nessun servizio cloud Pocket obbligatorio.
- Risultato: sei controlli separati, nomi/livello/razza/classe minimi proiettati
  con `textContent`. Nessun token, ID account, eccezione remota o corpo grezzo
  nei messaggi. Il risultato non viene salvato; «Cancella risultato» annulla
  anche un controllo in corso. I permessi opzionali sono rimovibili dalla pagina.

Gli endpoint sono **non documentati come contratto pubblico per PocketD-D**:
non ho trovato un OAuth pubblico utilizzabile dal prodotto. Le fonti di codice
sostengono la fattibilità, non dimostrano il funzionamento con questo account.
Le richieste sono limitate nel tempo (10 s complessivi), senza redirect o
ripetizioni automatiche, con JSON limitato a 32 KiB per auth e 512 KiB/300 righe
per l'elenco. Oltre limite si rifiuta il risultato, senza troncarlo di nascosto.
Un elenco paginato viene indicato come parziale. Anche un elenco formalmente
valido va confrontato con «I miei personaggi»; zero righe non prova completezza.

## Come eseguire la prima prova con il proprio account

Non serve aggiornare o collegare il firmware per questo controllo.

1. Scarica [solo l'estensione di verifica](https://shydanrem.github.io/PocketD-D/downloads/PocketDD-Beyond-Check.zip)
   ed estraila. È inclusa anche nel pacchetto PC 0.89.1, cartella `extension`.
2. Chrome: `chrome://extensions`, Edge: `edge://extensions`. Abilita modalità
   sviluppatore e «Carica estensione non pacchettizzata», scegliendo `extension`.
   Se usi già l'estensione Pocket, sostituisci i file nella cartella già caricata
   e premi «Ricarica» nella gestione estensioni: non caricare due copie del ponte
   in contemporanea.
3. Apri il popup Pocket → **Verifica account Beyond**. Apri Beyond, accedi
   normalmente nel medesimo profilo browser, poi **Verifica accesso**.
4. Controlla i sei esiti e confronta l'elenco con quello Beyond, includendo
   eventuali PG privati e fuori campagna. Basta riferire esiti, conteggio e se
   manca un PG: **non condividere cookie, token, dump di rete o JSON completi**.

Se il browser non trasmette la sessione nel contesto dell'estensione, il
controllo fallisce esplicitamente: non estrae altre credenziali come ripiego.
401/403, indisponibilità del servizio, rete/timeout e formato non riconosciuto
hanno esiti diversi. Non tentare di aggirare il login o le verifiche del sito.

## Stato delle prove

| Livello | Esito / limite |
| --- | --- |
| Ricerca del protocollo | Codice pubblico: scambio Cobalt → bearer, claim identità, endpoint account e `data.characters`; non una verifica autenticata |
| Test automatici del client | Risposte e token sintetici: HTTP/JSON/schema separati, rifiuti, scadenza, timeout, limiti, niente fallback anonimo/campagne, niente segreti nei risultati |
| Interfaccia | Pagina reale in Chromium a 900/390/320 px, API permessi simulate e risposte HTTPS sintetiche: consenso rifiutato, elenco, testo sicuro, parziale/vuoto, HTTP/formato, cancellazione e revoca verificati |
| Estensione MV3 caricata | **Bloccata dalla policy del Chromium gestito**: nessuna prova di caricamento/permessi/cookie reali superata qui. Il worker esistente è verificato con API simulate; non aggirata la policy |
| Account Beyond reale | **Da eseguire nel browser dell'utente**: login, formato effettivo, elenco completo/privati e comportamento della sessione |
| Lettura HTTP pubblica del PG | Già provata nelle versioni precedenti con il connettore PC; non ripetuta o spacciata per prova account in questa versione |
| Conversione/USB esistenti | Percorso PC esistente conservato; nessuna nuova importazione o modifica alla raccolta personale |
| ESP32 → Beyond | **Non implementato in questa versione**: questa verifica non dimostra TLS, RAM o conversione sul Pocket |

Controlli della release: **339 test JS superati** (12 dedicati alla verifica
account), compilazione ESP32-S3 riuscita, pacchetto e sito verificati separatamente.
Nessuna modifica alla logica firmware o ai formati PG/USB: nel firmware cambia
solo la versione. Le prove native dettagliate della v0.89 restano documentate
nelle rispettive guide, non dichiarate rieseguite da questa verifica account.

## Prossimi blocchi, in ordine

1. **Validare account ed elenco reali.** Sessione scaduta/rifiutata, PG privati,
   account senza PG, confronto dei risultati e loro completezza. Nessuna promessa
   di login permanente: logout/revoca possono richiedere una nuova associazione.
2. **Libreria locale sul dispositivo.** Copie per PG su SD, progressi/aspetto e
   storico separati, backup prima del cambio, salvataggio recuperabile dopo
   interruzione. Senza SD, dichiarare il limite di un PG attivo. Oggi la raccolta
   multipla è sul PC; il dispositivo conserva un PG attivo.
3. **Lettura e conversione native.** HTTPS con verifica certificato/orario e
   limiti espliciti; parsing selettivo/streaming e conversione C confrontata con
   quella JS. PF, CA, caratteristiche, equipaggiamento, magie e risorse devono
   coincidere per i profili supportati. La SD aumenta lo spazio ma non elimina
   i limiti di RAM. L'attuale parser C riceve pacchetti Pocket già convertiti,
   non il JSON Beyond: sostituirlo con un GET non completa l'importazione.
4. **Associazione protetta e UI sul Pocket.** Trasferimento con conferma del
   dispositivo, sessione fuori da PG/backup/SD, cancellazione e ricollegamento.
   L'attuale NVS non è cifrata: non depositarvi una sessione Beyond fingendo che
   sia protetta. Non modificare eFuse/cifratura flash durante un aggiornamento
   ordinario. Questa versione non trasferisce alcuna sessione al dispositivo.
5. **Importazione integrata.** Galleria, anteprima con copertura reale e conferma,
   nuova copia senza cancellare altri PG; aggiornamento sorgente separato dai
   progressi al tavolo. Nessuna sovrascrittura automatica di PF/slot/aspetto.

La sincronizzazione in tempo reale verso Beyond rimane un lavoro separato:
una lettura diretta non prova scritture, ricevute, conflitti o tiri sul sito.

## Fonti primarie di codice esaminate

- [MrPrimate, scambio sessione → bearer](https://github.com/MrPrimate/ddb-proxy/blob/main/auth.js).
- [iamjameslennon/ddb-mcp, token e claim account](https://github.com/iamjameslennon/ddb-mcp/blob/main/src/session-fetch.ts).
- [Stesso progetto, elenco e struttura `data.characters`](https://github.com/iamjameslennon/ddb-mcp/blob/main/src/tools/character.ts).
- [AlexWorland, definizione dell'endpoint](https://github.com/AlexWorland/dndbeyond-mcp/blob/main/src/api/endpoints.ts).
  Il suo strumento `listCharacters` invece usa campagne: non è la prova di una
  raccolta completa di PG propri, e non è il ripiego adottato da Pocket.

Nessuna dipendenza da questi progetti o da Foundry; nessun loro sorgente o asset
copiato. Implementazione Pocket specifica, limitata alla prova di sola lettura.
