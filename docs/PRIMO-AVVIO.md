# Primo avvio e sessione di collaudo — v0.57

Questa versione prepara il **blocco 3, prodotto fisico**. L'hardware è in
arrivo: tutte le prove fisiche sotto sono **da eseguire**, nessuna è già passata.
La grafica approvata è conservata; la v0.57 aggiunge circostanze e bonus
nel tiro morte e porta la copertura host a 102/102, senza attestare un esito fisico.

L'obiettivo della prima prova è trasferire una scheda nota, giocare offline,
conservare i progressi dopo un riavvio e recuperarli da un backup. Batteria,
sospensione e spegnimento controllato restano un lavoro successivo.

## 1. Identificare la scheda e conservare il firmware originale

Il firmware è compilato per **Waveshare ESP32-S3-Touch-AMOLED-1.75 standard,
senza suffisso C**, CO5300/CST9217, 466×466, flash 16 MB e PSRAM 8 MB.
Controllare sigla e revisione sulla scheda ricevuta. `board_target` nel rapporto
diagnostico indica la destinazione compilata, **non identifica la scheda fisica**.
Se la sigla differisce, verificare prima il BSP della revisione corretta.

Preparare un cavo USB dati, una cartella privata per i backup e il recovery
ufficiale della propria revisione. Per il primo avvio usare USB; il percorso
PMIC/batteria non è ancora collaudato. Non servono periferiche esterne.

Installare Python 3.10+ e gli strumenti sul PC. Gli esempi sono dalla radice
dell'archivio estratto; su Windows usare il comando Python della propria
installazione e `--port COM5`, su macOS `/dev/cu.usbmodem…`.

```sh
python3 -m pip install esptool==5.6.0 pyserial==3.5
mkdir device-backups
python3 -m esptool --chip esp32s3 --port /dev/ttyACM0 flash-id
python3 -m esptool --chip esp32s3 --port /dev/ttyACM0 read-flash 0 0x1000000 device-backups/factory-originale-16mb.bin
```

Confermare ESP32-S3 e flash da 16 MB prima di usare quell'intervallo. Scegliere
un nome nuovo per il backup e controllare lettura completa e file da
16.777.216 byte. Conservare privatamente il dump, che può contenere dati del
firmware di fabbrica. Questi comandi esptool possono riavviare la scheda per
entrare nel bootloader. Non sono stati eseguiti sull'hardware in questa fase.

## 2. Installare il pacchetto compilato

Usare insieme i file di **un solo archivio**. La procedura sostituisce il
firmware di fabbrica; eseguirla dopo il backup e la verifica del modello.
Dalla cartella `firmware-binaries/` dell'archivio:

```sh
python3 -m esptool --chip esp32s3 --port /dev/ttyACM0 write-flash @flash_args
```

`flash_args` include bootloader, tabella partizioni e app. Per la prima
installazione la sola app non basta. Non occorre `erase-flash`. Per BOOT/RESET
seguire la procedura Waveshare della propria revisione; la porta può cambiare
dopo il riavvio. Compilazione da sorgenti e compatibilità sono in
[FIRMWARE.md](FIRMWARE.md).

Atteso: compare l'eroe, poi menu e scheda sono raggiungibili. Un avvio senza
salvataggio personale mostra la demo; non deve cancellare salvataggi esistenti.
Se il display non parte, non procedere con prove di gioco o importazione.
La diagnostica di seguito diventa disponibile **dopo** l'inizializzazione del
display/touch: una mancata risposta non individua da sola il guasto. I log di
boot restano sulla console UART0 configurata, separata dalla USB di gioco.

## 3. Acquisire un riferimento diagnostico

Chiudere altri monitor della stessa porta. Dalla radice dell'archivio:

```sh
mkdir device-diagnostics
python3 scripts/device-check.py --port /dev/ttyACM0 --output device-diagnostics/01-avvio.json
```

Il comando legge un rapporto piccolo, senza nome, PF, inventario, magie,
storico, identificativi univoci, credenziali o dati Beyond. Non importa schede,
non scrive NVS, non estrae dadi e non comanda reset, flash o PMIC. Non accende
il display attenuato, non chiude conferme e non risolve un tiro pendente.
La raccolta PC non viene aperta.

Codici di uscita: **0** rapporto valido senza gli stati problematici elencati
dallo strumento; **2** rapporto acquisito con stati da risolvere; **1** errore
di acquisizione, formato o file. Nessuno di questi codici certifica il collaudo
fisico. Un rapporto con codice 2 viene comunque conservato, se richiesto.
`--output` richiede un nome nuovo e un filesystem che supporti hard link;
senza questa opzione il riepilogo viene soltanto stampato.

| Dato | Che cosa misura | Che cosa resta da verificare |
| --- | --- | --- |
| Firmware/SDK/formati | Versioni dichiarate dall'app in esecuzione | Modello fisico e revisione |
| Heap interno/PSRAM | Byte liberi, minimo storico, maggiore blocco libero; totale heap PSRAM | Stabilità nelle operazioni più impegnative; non sono una prova della RAM intera |
| Stack UI/USB | Minimo spazio libero storico in **byte** secondo ESP-IDF | Margine dopo import, dadi, riposi e backup |
| Touch | Letture, errori del driver e inizi di contatto | Posizione, orientamento, precisione e tocchi involontari |
| PWR/BOOT | Fronti premuto dopo un rilascio osservato; letture PWR indisponibili | Debounce, pressione breve/lunga e azione assegnata; i contatori sono grezzi |
| Display/LVGL | Invii di rettangoli e massimo tempo di un handler in microsecondi | Colori, resa, FPS e latenza percepita; gli invii non provano la resa del pannello |
| Scheda/registro | Disponibilità di scrittura, validità RAM e identità coincidenti | Persistenza effettiva dopo riavvio e perdita di alimentazione |
| Schermo | Attenuazione, errore corrente di luminosità e operazione protetta | Luminosità reale e consumi; non viene letto uno stato batteria |

Contatori e massimi ripartono al boot, saturano a UINT32_MAX e non sono salvati.
Il tempo di funzionamento è a 64 bit. I valori heap sono letture ravvicinate,
non una transazione fra task. Il massimo LVGL include attese driver, non il
tempo totale del ciclo USB/interfaccia. Una lettura della diagnostica può
influire brevemente sulla latenza e sui margini osservati; usarla ai punti di
controllo, senza un polling continuo. Non assegniamo soglie di prestazione o
autonomia prima delle misure.

## 4. Provare display, touch e tasti configurabili

In **Aspetto → Tasti → Collaudo**, aprire prima **Prova tocco** e poi
**Prova tasti**. Il [percorso locale](COLLAUDO-COMANDI.md) offre cinque bersagli
ordinati e tre contatori dei gesti, senza eseguire le scorciatoie configurate.
Controllare rilascio/posizione e registrare l'esito osservato. Azzera riavvia la
prova; Esci o la scadenza di due minuti conservano scheda e preferenze.
Una pressione mantenuta all'uscita deve essere rilasciata prima di poter agire
sui comandi normali. Il collaudo non salva risultati, non scrive NVS e non estrae dadi.

Aprire Eroe, Menu, Scheda, Zaino e Magie. Controllare testi, icone, colori,
assenza di ritagli/lampeggiamenti e precisione dei comandi in tutte le zone
usate del cerchio. Annotare nome della schermata e punto del tocco per ogni
errore; una foto/video del pannello aiuta a confrontarlo con il catalogo host.

In **Aspetto → Tasti**, assegnare temporaneamente PWR breve a Scheda, BOOT
breve a Zaino e BOOT lungo a Storico. Provare ciascuna azione dopo il rilascio;
deve partire una volta sola. PWR lungo resta riservato all'hardware e non è
una quarta scorciatoia. Non attribuire al firmware uno spegnimento controllato.

Aprire una conferma di gioco e premere i tre comandi: non devono confermare,
consumare, lanciare o abbandonare la finestra. Riavviare in modo ordinario e
verificare le assegnazioni. La copia scheda USB9 conserva questi tasti; il
ripristino nativo ripristina quelli del suo archivio.

In **Aspetto → Schermo**, provare 10/50/100%, poi 50% con attenuazione a 30 s.
Il primo tocco/tasto deve solo risvegliare; dopo il rilascio il gesto successivo
può agire. Una conferma o un dado in volo devono impedire l'attenuazione.
Annotare gli errori reali: l'attenuazione non è sospensione del dispositivo.

## 5. Sessione guidata con scheda sintetica

Prima di sostituire una scheda già usata, acquisirne un
[backup nativo](BACKUP-DISPOSITIVO.md). Il pacchetto include una scheda
**originale sintetica**, riproducibile con `scripts/hardware-session.js`:

```sh
python3 scripts/device-upload.py firmware/tests/fixtures/hardware-session.usb9.synthetic.json --port /dev/ttyACM0
```

Annullare una prima richiesta e controllare che nulla cambi. Inviare di nuovo,
verificare nome/PF/CA, confermare e attendere `PDD IMPORTED`. Il file non entra
nella raccolta PC: sostituisce solo la scheda attiva del dispositivo.

| Valore iniziale | Atteso |
| --- | --- |
| Eroe | Elian Collaudo, livello 3, PF 24/32, temporanei 3, CA 16 |
| Caratteristiche | FOR 14, DES 16, COS 14, INT 10, SAG 12, CAR 8 |
| Prove | FOR +2, Atletica +4, salvezza COS +4 |
| Zaino | Spada ×1 equipaggiata, Pozione ×2 (2d4+2), Torcia ×3 |
| Attacco | Spada +4, danni 1d8+2; 20 naturale critico base 2d8+2 |
| Magia | Luce di prova, livello 1, concentrazione; ordinari 2/3, patto 1/1 al primo livello |
| Capacità | Passo di prova, 1/2 usi, recupero breve |
| Dadi vita | Classe di prova, 3d8 totali, 1 speso, 2 disponibili |
| Monete | 12 MO, gli altri quattro saldi zero verificati |

Le due capacità sono inventate per il collaudo: nessun testo di catalogo
Beyond e nessun effetto numerico è dedotto dalla descrizione.

Dopo l'importazione chiudere uploader, app e server PC, mantenendo
l'alimentazione USB. La sessione seguente deve funzionare senza servizi PC,
Beyond, Foundry o rete; non richiede ancora l'avvio a batteria.

Eseguire nell'ordine, annotando facce e risultati casuali effettivi:

1. **PF e annullamento:** −1 assorbe un temporaneo (PF 24, temporanei 2);
   Annulla riporta a 24 e 3. Il riavvio conserva lo stato salvato; annullamento
   e bozze di sessione non sono persistenti.
2. **Dadi/prove:** FOR, Atletica, COS e un tiro libero 3d6+2. Il totale deve
   coincidere con facce più bonus. Provare Base/Vant/Svant e accelerazione:
   accelerare mostra lo stesso tiro; Storico contiene l'esito una sola volta.
3. **Attacco:** scegliere la Spada e una CA bersaglio esplicita. Verificare
   +4 e risultato; tirare i danni se l'attacco colpisce. Non modificare PF del
   bersaglio automaticamente. Non occorre ottenere un critico per proseguire.
4. **Magie/usi:** Luce con uno slot ordinario porta a 1/3 e avvia
   concentrazione; il patto resta 1/1. Passo porta a 0/2. Lanciare Luce con il
   patto, confermando la sostituzione: patto 0/1, ordinari ancora 1/3.
5. **Riposo breve:** anteprima, Indietro senza modifiche, poi conferma.
   Passo torna 2/2, patto 1/1, ordinari restano 1/3; PF, temporanei e
   concentrazione restano quelli precedenti. Spendere un dado vita se ferito:
   un solo d8+2, cura limitata a 32, dadi spesi da 1 a 2. Annotare i PF effettivi.
6. **Zaino/monete:** consumare una Torcia (3→2), registrare una spesa di
   1 MO (12→11) con anteprima. La Pozione consuma una dose con cura 2d4+2
   limitata ai PF massimi; verificare prima che il caso sia applicabile.
7. **Effetti:** configurare un modificatore CA +2 per un round; CA 16→18.
   Avanzare il round esplicitamente: CA torna 16. Non dedurre il bonus dal testo.
8. **Riposo lungo:** anteprima con recupero dadi vita 2024, poi conferma.
   PF 32, temporanei 0, concentrazione terminata, ordinari 3/3, patto 1/1,
   Passo 2/2 e dadi vita spesi 0. Torce, pozioni e monete non si ricreano.

Se un esito differisce, fermare quel percorso e conservare stato osservato e
diagnostica; non reimportare la scheda per nascondere il problema.

## 6. Riavvio, backup e recupero

Acquisire `device-diagnostics/02-dopo-sessione.json`. Confrontare minimi heap e
stack, errori e contatti/tasti con il primo rapporto. Ripetere i percorsi più
usati e controllare che i margini si stabilizzino: un solo campione non misura
una perdita di memoria.

Acquisire un nuovo backup nativo, aprirlo nell'anteprima PC, confrontare i
valori reali e lo storico. Modificare una moneta sul dispositivo, avviare il
ripristino e annullare: la modifica deve restare. Ripetere e confermare:
devono tornare scheda, storico e preferenze della copia. Riavviare e confrontare.
Il download riuscito non è già una prova del ripristino.

Solo dopo questi passaggi, con scheda sintetica e copie conservate, pianificare
insieme le interruzioni durante salvataggio/import/ripristino. Un normale
riavvio dopo un salvataggio **non** certifica la resistenza al taglio durante
la scrittura. Scheda e registro dei normali tiri restano due commit distinti:
[garanzie e limiti](BACKUP-DISPOSITIVO.md). Non cancellare NVS per superare un errore.

## 7. Prova opzionale dei danni a zero PF — dopo il backup

Su una scheda sintetica con registro disponibile, massimo 32, PF 0 e
3 temporanei, applicare un danno effettivo normale di 1: l'anteprima deve
mostrare temporanei 2 e un fallimento aggiunto. Verificare Base 2014,
confermare e controllare Stato. Annullare deve ripristinare tutta la scheda.
Ripetere dichiarando critico: due fallimenti, senza raddoppiare il danno.
Provare separatamente PF 24, temporanei 3 e danno effettivo 59: eccedenza 32,
morte istantanea e tre fallimenti. Conservare la copia precedente e verificare
riavvio/ripristino; non usare una scheda personale per queste prove.
I dettagli e i limiti sono in [DANNI-ZERO-PF.md](DANNI-ZERO-PF.md).
Registrare H23 senza dedurre un esito fisico dai test host.

## 8. Prova opzionale dei tiri contro morte — scheda sintetica

Dopo il backup, con registro disponibile a zero PF, aprire Modifica: attivare
entrambi i segni deve mostrare che si annullano. Impostare un bonus, controllare
l'anteprima e tornare indietro senza lancio: scheda e storico restano invariati.
Confermare poi un tiro valido e confrontare naturale, bonus, totale ed esito,
con la priorità di 1/20 descritta in [TIRI-MORTE.md](TIRI-MORTE.md).
Controllare nello storico facce e modalità. Annullare dall'Eroe deve recuperare
la scheda precedente, conservando il tiro avvenuto nello storico.
La successiva apertura di Modifica deve ripartire da bonus zero e segni spenti;
Lancia d20 base deve restare un solo dado senza bonus. Non cercare guasti NVS
per forzare il caso di nuovo tentativo: resta una prova host e il taglio fisico
richiede il protocollo separato già indicato. Registrare H24.

## Registrazione dell'esito

Copiare [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md) fuori dal repository e
compilarlo con versione, revisione e risultati osservati. `Non eseguito`,
`Passato`, `Fallito` e `Bloccato` restano distinti. Rimuovere dati personali da
foto e log prima di condividerli. Le cartelle `device-backups/` e
`device-diagnostics/` sono escluse da Git.

La prima versione utilizzabile richiede esito favorevole di avvio, comandi,
sessione, riavvio e backup/ripristino. Alimentazione autonoma e tenuta ai guasti
restano criteri separati: nessun esito viene dedotto dalla build o dalla diagnosi.
