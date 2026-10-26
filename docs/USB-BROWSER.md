# Scheda e backup via USB dal browser — supporto PC v0.64

Dopo aver installato il firmware, l'app PC può inviare il personaggio,
ricevere il backup nativo e ripristinarlo direttamente da **Chrome o Edge sul
PC**. Non occorrono gli script Python per questi tre passaggi. Il dispositivo
rimane il prodotto di gioco; il PC prepara la scheda e conserva le copie.

Usare l'app compilata tramite `npm start` e aprire `http://127.0.0.1:5173/`,
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
Non si comandano reset, flash, erase o cambi DTR/RTS. Il comportamento fisico
all'apertura della porta dipende anche dal sistema e resta da collaudare.

## Verifiche

Nove test del trasporto usano vere ReadableStream/WritableStream con periferica
**simulata**: frammentazione fino al singolo byte, negoziazione, copia/versione
congelate, ACK obbligatori, vecchi/futuri protocolli, cancellazione/errori/EOF,
CRC/record corrotti, limiti di riga/buffer, aborto, esclusione delle operazioni
contemporanee, riapertura e apertura tardiva dopo timeout. Nessuna porta fisica.

La suite JavaScript passa **182 test**. Chromium prova il codice PC compilato
con Web Serial simulata: invio/anteprima, conferma/guardie, aborto, acquisizione
con download esatto, ripristino, errore d'integrità, browser non supportato e
raccolta sintetica invariata. Il precedente flusso di esportazione su file e
anteprima backup resta verificato dal controllo PC esistente.

Firmware e installer rimangono v0.64; il binario ESP32-S3, le 114 viste LVGL,
partizioni, PDD28/PDDS, USB9, backup nativo v1 e backup PC v11 sono invariati.
Non sono stati ripetuti i collaudi fisici né effettuate letture Beyond o
modifiche della raccolta personale. H33 in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md)
resta **Non eseguito**. Gli script Python rimangono disponibili come alternativa.
