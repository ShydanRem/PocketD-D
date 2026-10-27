# Scheda e backup via USB dal browser — supporto PC

> Aggiornamento v0.68: [quattro nuove famiglie e compressione senza perdita](AVATAR-SPECIE-ANIMALI.md).
> Catalogo a 16 specie/32 silhouette, PDD31/PDDV; USB11 e backup PC v13
> per i nuovi ID. Formati precedenti conservati; collaudo fisico ancora aperto.

> Aggiornamento v0.67: [specie, silhouette e proposta da Beyond](AVATAR-SPECIE.md).
> Scrittura nativa PDD30/PDDU; USB10 e backup PC v12 per i nuovi avatar,
> con lettura dei formati precedenti. Le prove reali pregresse non attestano
> questa nuova selezione visiva; collaudo hardware ancora aperto.

**Aggiornamento v0.66 del 27 ottobre 2026:** [avatar e sfondi sul dispositivo](PERSONALIZZAZIONE.md),
backup PDD29 e nuova assegnazione Aspetto accettati dal PC. Il trasferimento
della scheda resta USB9; la scena locale viene conservata. Questo incremento
non risolve né certifica il collegamento fisico segnalato: la prova è rimandata
al rientro dell'utente. Le correzioni USB datate sotto mantengono il proprio contesto.

**Correzione PC del 26 ottobre 2026:** gestiti avvio/riavvio della porta e
messaggi ROM/ESP-IDF riconosciuti. Il firmware distribuito rimane 0.65.0;
non occorre reinstallarlo per aggiornare questo collegamento.

**Diagnosi successiva:** il limite iniziale di 64 righe poteva interrompere
anche un singolo avvio con molti log. Non dimostra un ciclo di reset.
È ora disponibile **Verifica collegamento USB**, senza invio del PG, con
un resoconto selettivo della porta e delle risposte. La causa fisica segnalata
dall'utente rimane da individuare; non è una dichiarazione di trasferimento riuscito.

Dopo aver installato il firmware, l'app PC può inviare il personaggio,
ricevere il backup nativo e ripristinarlo direttamente da **Chrome o Edge sul
PC**. Non occorrono gli script Python per questi tre passaggi. Il dispositivo
rimane il prodotto di gioco; il PC prepara la scheda e conserva le copie.

Su Windows il [pacchetto PC pronto](https://pocketdd-installer.shydan.chatgpt.site/downloads/PocketDD-PC.zip)
si estrae e si avvia con **Avvia-PocketDD.cmd** (Node.js 24 attuale).
In alternativa usare l'app compilata tramite `npm start` e aprire `http://127.0.0.1:5173/`,
oppure un'origine HTTPS compatibile con Web Serial. Un HTML aperto direttamente
non abilita questi comandi: resta disponibile il download dei file. Chiudere
l'installer firmware, monitor seriali e altre applicazioni che usano la porta.
Il dispositivo deve già eseguire PocketD-D: questo collegamento non installa
firmware e non è un comando del bootloader.

## Inviare il personaggio

1. Scegliere il personaggio nell'app PC e aprire **Prepara per dispositivo (USB)**.
2. Controllare nome, caratteristiche, PF, CA, inventario, magie e limiti riportati
   nell'anteprima. La copia mostrata resta quella inviata, anche durante la
   scelta della porta.
3. Premere **Invia al dispositivo via USB** e selezionare la porta della scheda.
4. Controllare nome e valori sul display e confermare entro 120 secondi.
5. Il browser mostra il salvataggio riuscito soltanto dopo **PDD IMPORTED**.

Si sostituisce la copia attiva sul dispositivo. Le altre schede della raccolta
PC restano conservate; non vengono sincronizzati i progressi al contrario.
La copia è sempre la proiezione USB limitata esistente: fino a 60 oggetti e
120 magie/capacità, nomi ASCII, testi lunghi in estratti segnalati, effetti
rappresentati secondo i limiti già documentati. Il dispositivo conserva le
preferenze locali all'importazione e valida tutti i dati prima della conferma.
Un firmware con versione USB insufficiente non riceve la copia.

## Ricevere e conservare il backup

Aprire **Backup del dispositivo → Ricevi backup via USB**, scegliere la porta
e confermare l'invio sul display. Il browser controlla lunghezza, CRC del
trasporto e integrità dei record nativi, poi mostra nome, PF/CA, risorse,
impostazioni e ultimi 20 tiri. **Scarica copia verificata** conserva il file
ricevuto integralmente, senza convertirlo nella raccolta PC.

Ricevere il backup non significa averlo già salvato in un file: premere Scarica
e verificare il download. Il contenuto ricevuto rimane soltanto nella memoria
della finestra finché non lo si scarica. Chiudere la finestra o ricaricare la
pagina lo perde. Se una nuova ricezione fallisce, la copia precedentemente
verificata rimane disponibile nella finestra.

## Ripristinare

Aprire un backup nativo già salvato oppure usare quello appena ricevuto.
Controllare l'anteprima e premere **Ripristina questa copia via USB**. Il comando
sostituisce scheda, storico e preferenze del dispositivo; la raccolta PC non
cambia. Il firmware esegue la validazione semantica completa e richiede una
nuova conferma sul display. Il browser attende **PDD RESTORED** prima di mostrare
successo. Non basta che il file sia stato inviato.

La verifica PC resta una verifica d'integrità con anteprima, non sostituisce
quella nativa di tutte le regole. File con envelope/record/CRC non validi non
aprono il selettore USB per il ripristino. Le copie della raccolta PC e quelle
native del dispositivo restano formati diversi.

## Errori, interruzione e comandi

Durante un'operazione, chiusura/Escape, selezione del file e altri comandi della
finestra sono bloccati. **Interrompi collegamento USB** interrompe l'attesa e
rilascia la porta; non invia un comando di annullamento al firmware. Controllare
il display: la richiesta può essere ancora aperta o il salvataggio può essere
già avvenuto. Scartare l'eventuale conferma prima di una nuova operazione.

Annullare il selettore prima dell'apertura non invia nulla. Annullamento sul
dispositivo, errori NVS, porta occupata, scollegamento e timeout non diventano
un falso successo. Un ACK finale perso lascia l'esito **non confermato**, anche
se il dispositivo avesse già salvato. Non c'è ritrasmissione automatica della
copia. Errori e messaggi non includono il JSON del personaggio.

La negoziazione dura al massimo 10 secondi, l'invio 30, la conferma 125 e la
ricezione del corpo 30. Scritture a blocchi fino a 1024 byte, righe di controllo
fino a 256 caratteri e backup fino a 110.000 byte. Una sola operazione nella
pagina alla volta; altre pagine/programmi possono comunque occupare la porta.
All'apertura si rilasciano DTR/RTS insieme (`false/false`), senza sequenze per
resettare o entrare nel bootloader. Non si comandano flash o erase. Il comportamento
fisico all'apertura dipende anche dal sistema e resta da verificare sulla scheda.

## Lampo nero, Elian ancora presente o risposta inattesa

L'utente ha segnalato un lampo nero seguito dal PG dimostrativo e l'errore
«Risposta USB inattesa o protocollo non supportato». Questo è compatibile con
un riavvio, ma non è una diagnosi fisica confermata. Il client precedente
rifiutava ogni riga estranea, compresi eventuali messaggi d'avvio della scheda;
non ripeteva il saluto se questo veniva perso durante la partenza.

Il client aggiornato riconosce soltanto righe ROM ESP32-S3/ESP-IDF note, con
limiti di 512 byte per riga, 512 righe e 32.768 byte per sessione. Questi sono
limiti di risorse, non una classificazione automatica di riavvio continuo.
La colorazione ANSI è
accettata soltanto su questi log. Testo estraneo, protocolli futuri, risposte
fuori fase e ACK precedenti continuano a essere rifiutati; l'errore indica la
fase e riporta soltanto risposte di controllo riconosciute, mai il JSON del PG.
Il messaggio del bootloader dà istruzioni per tornare all'app.

Durante i 10 secondi di collegamento si può ripetere soltanto il saluto HELLO
(intervalli di circa un secondo). Le righe parziali sono conservate tra i tentativi;
le eventuali risposte duplicate concordanti vengono consumate in numero limitato
prima dell'ACK della prima richiesta. **Non si ripetono automaticamente il PG,
la richiesta di backup o il ripristino**, e non si interpreta il riavvio come
un salvataggio. Rimangono obbligatori conferma sul display e ACK finale.

Per riprovare: chiudere l'app PC precedente e l'installer, estrarre il nuovo ZIP,
avviare `Avvia-PocketDD.cmd`, scollegare/ricollegare il dispositivo senza BOOT e
attendere la schermata PocketD-D. Con lo stesso browser e indirizzo locale il
PG già importato sul PC resta disponibile. Aprire **Prepara per dispositivo
(USB)** e, prima di ripetere l'invio, premere **Verifica collegamento USB**.

La verifica invia esclusivamente `PDD HELLO`: nessun JSON, richiesta di backup
o ripristino. Richiede `PDD READY` supportato per dichiarare il collegamento
verificato, e non dichiara importato alcun personaggio. Lo stesso comando è
disponibile nella finestra del backup. Al termine aprire **Dettagli collegamento
USB** e copiare il testo in caso di errore.

Il resoconto comprende solo VID/PID numerici (senza numero di serie), fase,
esito, versione di protocollo accettata, tentativi, conteggi log, annunci ROM,
codici/motivi di reset riconosciuti, ultimo tag del log e categorie di errore
firmware. Se presenti, conserva indirizzi esadecimali della riga Backtrace;
non conserva registri, dump di memoria, log integrali, nome o JSON del PG.
Gli annunci ROM possono provenire da dati accumulati: il loro conteggio da
solo non prova riavvii attuali. Nessun resoconto viene inviato a un server.
L'esito della verifica e quello del salvataggio rimangono separati.

## Verifiche

Quindici test del trasporto usano vere ReadableStream/WritableStream con periferica
**simulata**: frammentazione fino al singolo byte, negoziazione, copia/versione
congelate, ACK obbligatori, vecchi/futuri protocolli, cancellazione/errori/EOF,
CRC/record corrotti, limiti di riga/buffer, aborto, esclusione delle operazioni
contemporanee, riapertura e apertura tardiva dopo timeout. Aggiunti log d'avvio
frammentati/colorati, HELLO perso, READY parziale/tardivo/duplicato, rilascio
segnali, bootloader, eccesso di log e timeout; un solo invio effettivo per operazione.
Inclusi 120 log in un solo avvio, verifica con solo HELLO, metadati selettivi,
errori di collegamento/salvataggio distinti e limiti per byte/righe.
Nessuna porta fisica.

La suite JavaScript passa **195 test**. Chromium prova il codice PC compilato
con Web Serial simulata: invio/anteprima, conferma/guardie, aborto, acquisizione
con download esatto, ripristino, errore d'integrità, browser non supportato e
raccolta sintetica invariata, anche con 123 righe d'avvio e primo HELLO perso.
Verifica anche il pulsante di sola diagnosi, successo/errore senza invio del PG,
resoconto visualizzato come testo e assenza dei contenuti arbitrari dei log.
Il precedente flusso di esportazione su file e
anteprima backup resta verificato dal controllo PC esistente.

Questa correzione cambia soltanto il client PC. Binario ESP32-S3 0.65.0,
partizioni, PDD28/PDDS, USB9, backup nativo v1 e backup PC v11 sono invariati;
non è stata eseguita una nuova build firmware. Nessuna nuova lettura Beyond
o modifica della raccolta personale. Il trasferimento di questo PG sul dispositivo
dell'utente resta **da confermare**; H33 in
[COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md) resta **Non eseguito** come prova
strutturata completa. Gli script Python rimangono disponibili come alternativa.

Riferimenti di hardware/trasporto consultati: [Waveshare, USB nativa della scheda](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/main/HARDWARE_REFERENCE.md)
e [Espressif, USB Serial/JTAG su ESP32-S3](https://docs.espressif.com/projects/esp-idf/en/release-v5.5/esp32s3/api-guides/usb-serial-jtag-console.html).
Non si è introdotto un percorso UART o cambiato firmware senza identificare
prima la porta e il guasto osservato.
