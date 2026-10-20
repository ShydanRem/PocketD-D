# Firmware PocketD-D 0.22 — magie, capacità e tasti configurabili

Il prodotto principale è il dispositivo. La parte PC serve per importare,
configurare, trasferire e conservare backup. La simulazione browser rimane un
banco di prova; non stabilisce la priorità delle funzioni del prodotto.

## Stato e perimetro

Firmware per **Waveshare ESP32-S3-Touch-AMOLED-1.75**, versione standard senza
suffisso C. Toolchain ESP-IDF **v5.5.5**, LVGL **9.4.0**. Nessuna dipendenza da
Foundry, server PC, Beyond, Wi-Fi o Bluetooth durante l'uso al tavolo.

La v0.16 aggiunge [d20 animato in pixel art](DADI-ANIMATI.md), lancio doppio,
risultato naturale sul dado, totale separato e accelerazione senza rilanci.

La v0.17 aggiunge d4/d6/d8/d10/d12, quantità fino a sei, bonus e modalità
Somma/Danni. Da Dadi → Altri si prepara il tiro. Nessun danno è applicato
automaticamente ai PF; nessun bonus di arma viene dedotto.

La v0.18 introduce la prima [revisione visiva del dispositivo](DIREZIONE-VISIVA.md):
scena con avatar base originale, icone pixel art, tema notturno, gerarchia dei
valori e barre PF/risorse. L'avatar è statico e decorativo; l'aspetto scelto sul
PC non viene ancora trasferito. Dati e formati USB/NVS restano invariati.

La v0.19 trasferisce anche l'aspetto scelto nell'editor PC: tre carnagioni,
colori capelli, mantelli e tagli, con animazione di attesa disattivabile da
Eroe → Aspetto. L'anteprima PC usa i disegni del firmware. La copia USB è v2;
lettura delle vecchie copie v1 ancora supportata, con aspetto base. I salvataggi
v1 restano leggibili; le nuove scritture sono v2 (vedere compatibilità sotto).

La v0.20 porta abilità e salvezze con competenza/maestria, mezza competenza,
extra e dettaglio dei contributi; [uso e limiti](ABILITA-SALVEZZE.md).
Copia USB v3, record NVS PDD3 da 512 byte. Le copie v1/v2 restano leggibili e
conservano PF/avatar/contatori, ma non hanno competenze: per quelle viene
richiesta una copia aggiornata, senza assegnare falsi gradi zero.

La v0.21 aggiunge [inventario, attacchi e cure](INVENTARIO-ATTACCHI.md):
Usi → Zaino, scelta/rimozione arma, CA bersaglio, dettaglio bonus, danni e critici
fino a 18 dadi, consumabili con conferma e annullamento. USB v4 e record PDD4,
con lettura delle copie precedenti; niente oggetti inventati per schede vecchie.

La v0.22 aggiunge [magie/capacità e tasti configurabili](MAGIE-TASTI.md),
slot ordinari/patto, preparazione, utilizzi e concentrazione con conferma.
USB v5, record PDD5 compatti in una nuova partizione NVS; vecchia area conservata.
I testi oltre 500 caratteri sono estratti dichiarati; effetti delle magie al DM.

Primo percorso: avvio con Elian originale o copia precedentemente salvata →
Scheda → caratteristica → d20 → risultato → Eroe → PF ±1 → annulla →
salvataggio locale. Anche PF temporanei e fino a otto contatori manuali.
Vantaggio e svantaggio selezionabili dal pulsante Base/Vant/Svant. Durante il volo
si può accelerare; modalità e navigazione attendono l'arresto del tiro.

Non è ancora una scheda completa: mancano riposi automatici,
gestione completa di effetti/condizioni, morte, dadi vita,
monete, storico persistente, mappa e party. Le prove applicano i contributi
riconosciuti e trasferiti; i contesti da verificare restano bloccati. Le istantanee per prove e attacchi includono Avvelenato, Ira, attacco potente e dadi critici extra: Ira riconosciuta usa ora il contatore della capacità; altre durate e interazioni restano manuali. Il dado libero
usa bonus zero. La CA è il valore esportato, non un ricalcolo dell'equipaggiamento.
I contatori sono manuali: usarli non attiva effetti né riposi automatici.
Il firmware tiene una sola scheda attiva; non sincronizza modifiche al PC.

## Hardware e fonti verificate

Fonte ufficiale esatta, fissata per la verifica:
[repository Waveshare, e4344e70](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/tree/e4344e70c2fa78a13e8a06566507f1ba8af6672a).
Consultati HARDWARE_REFERENCE.md, schema e inizializzazione del BSP CO5300/CST9217.
Il dettaglio delle dipendenze compilate è in `firmware/dependencies.json`;
licenze e adattamenti in `firmware/THIRD_PARTY.md`.

| Funzione | Collegamenti usati |
| --- | --- |
| CO5300 QSPI 466×466 RGB565 | CLK38, D0–D3 GPIO4–7, CS12, reset39, gap X=6 |
| CST9217 I2C 0x5A | SCL14, SDA15, reset40, INT11; mirror X/Y dal BSP |
| USB nativo Serial/JTAG | GPIO19/20, console di gioco separata dai log UART0 |
| Flash / PSRAM | 16 MB / 8 MB octal, configurazione da sorgente ufficiale |

Un buffer parziale DMA **interno** di 466×40×2 = **37.280 byte**, senza assumere
DMA diretto da PSRAM. Un solo task possiede LVGL; il task USB passa copie
validate attraverso una coda. Il trasferimento SPI termina prima di riutilizzare
il buffer. Rettangoli allineati a coordinate pari come richiesto dal pannello.
Le aree touch sono alte almeno 44 pixel e restano dentro il cerchio.
Stati estesi/linea USB in PSRAM e temporanei nel heap; stack principale da
16384 byte, USB da 12288 byte. Il margine effettivo degli stack resta da misurare
sulla scheda insieme a heap e PSRAM.

Non vengono scritti registri di carica/voltaggio del PMIC AXP2101: si segue il
percorso display/touch dell'esempio ufficiale, che non riprogramma il PMIC.
**Avvio a batteria, alimentazioni, orientamento touch e luminosità devono ancora
essere verificati sulla scheda reale.** Non sono implementati telemetria
batteria, sospensione, spegnimento controllato o stime d'autonomia. Luminosità
iniziale circa 50%, senza valori di batteria inventati. Audio, SD, RTC e IMU
non vengono avviati. Nessun GPIO riutilizzato per periferiche esterne.

## Compilazione

Installare e attivare ESP-IDF **v5.5.5** con la sua procedura ufficiale per
ESP32-S3 (toolchain Xtensa GCC 14.2.0, esp-14.2.0_20260121). Non occorre Arduino.
Dalla radice del repository:

```sh
npm run firmware:deps
idf.py -C firmware set-target esp32s3
idf.py -C firmware build
npm run firmware:test
```

`firmware:deps` scarica soltanto sorgenti Git pubblici ai commit fissati.
Preparazione delle dipendenze verificata anche da directory vuota. La prima preparazione richiede rete; il runtime non la usa. Il component
manager è disabilitato nel progetto: niente versioni flottanti dal registry.
Il test nativo richiede anche un compilatore C/C++, CMake e Ninja per il PC;
cJSON viene preso dallo stesso checkout ESP-IDF. Nessun account o credenziale
va inserito nel firmware.

I binari prodotti sono in `firmware/build/`: `pocketdd.bin`,
`bootloader/bootloader.bin`, `partition_table/partition-table.bin`,
`flash_args` e `flasher_args.json`. La sola app non basta per una scheda con
layout di fabbrica. L'archivio di questa versione include una cartella
`firmware-binaries/` con i tre binari, argomenti e licenze.

### Prima accensione e flash: prova hardware ancora da fare

Il layout conserva NVS legacy a 0x9000 e app a 0x10000 da 4 MiB.
La v0.22 aggiunge `pdd_state` da 256 KiB a 0x410000, senza spostare quelle aree.
Aggiornare solo l’app con layout v0.21 lascia il dispositivo in sola lettura:
serve anche il nuovo layout, dopo backup. [Migrazione e downgrade](MAGIE-TASTI.md).
**Scrivere bootloader/layout/app sostituisce il firmware di fabbrica.** Prima
conservare una copia completa della flash e il recovery ufficiale della propria
revisione; non mescolare immagini di versioni diverse. Il backup flash può
contenere configurazioni personali: conservarlo privatamente.

Esempio con porta scelta dall'utente, non eseguito in questa sessione:

```sh
python -m esptool --chip esp32s3 --port /dev/ttyACM0 read-flash 0 0x1000000 factory-backup.bin
idf.py -C firmware -p /dev/ttyACM0 flash
```

Oppure, nella cartella dei binari distribuiti:

```sh
python -m esptool --chip esp32s3 --port /dev/ttyACM0 write-flash @flash_args
```

Su Windows sostituire con COMn, su macOS con la porta `/dev/cu.usbmodem…`.
Se non entra in download, seguire BOOT/RESET del produttore. Nessuno script
PocketD-D esegue automaticamente `erase-flash`, flash o recovery.

## Trasferimento della scheda dal PC

1. Aprire l'app PC, selezionare la scheda e premere **Prepara per dispositivo
   (USB)**. L'anteprima elenca valori, contatori inclusi e funzioni escluse.
2. Scaricare `pocketdd-device.json`. È una proiezione limitata, non il backup
   completo e non il JSON originale di Beyond. La raccolta PC resta identica.
3. Sul PC installare `pyserial==3.5`, collegare la scheda col firmware PocketD-D,
   chiudere altri monitor seriali e inviare:

```sh
python3 -m pip install pyserial==3.5
python3 scripts/device-upload.py pocketdd-device.json --port /dev/ttyACM0
```

4. Controllare nome, PF e CA sul display. **Conferma** sostituisce la sola copia
   attiva sul dispositivo e salva; **Annulla** conserva la precedente. Timeout
   dopo 120 secondi. Il PC dichiara riuscita solo dopo `PDD IMPORTED`, inviato
   dal firmware dopo scrittura e rilettura. Mancata risposta significa esito non
   confermato: controllare il display prima di reinviare.

Nessun reset/flash, scrittura verso Beyond o modifica della raccolta da questo
strumento. Il trasferimento fisico USB resta da collaudare, benché parser,
protocollo host e conferma grafica siano testati separatamente.

Formato `pocketdd-device` v5, massimo 262144 byte, campi esatti di base: `format`, `version`,
`name`, `level`, `hp`, `maxHp`, `tempHp`, `armorClass`, `abilities` e `resources`.
Da v2 è obbligatorio anche `avatar`, con esattamente `skin`, `hair`, `cloak`,
`style`: indici interi 0–2 corrispondenti alle scelte PC. Da v3 è obbligatorio
`training`, con gradi, competenze, extra, mezze competenze, blocchi e le due
istantanee; [schema completo](ABILITA-SALVEZZE.md). Il firmware accetta anche
v1/v2/v3/v4: v1 usa l'aspetto base; v1/v2 segnalano le competenze mancanti.
Da v4 è obbligatorio `inventory`; [schema e omissioni](INVENTARIO-ATTACCHI.md).
Da v5 è obbligatorio `magic`; [schema, testi e tasti locali](MAGIE-TASTI.md).
La preferenza locale per
l'animazione non viene trasferita dal PC: la conferma conserva quella già attiva.
Il nuovo uploader negozia `PDD READY 1/2/3/4/5` e blocca una copia prima dell'invio
se il firmware non ne supporta la versione. Usare lo script di questa versione;
i precedenti uploader non riconoscono `PDD READY 5`.
Nome ASCII 1–48 byte, contatore 1–32; accenti traslitterati e altri simboli
sostituiti nell'anteprima PC. Livello 1–20, caratteristiche 1–30, CA 1–40,
PF 0–massimo (massimo 1–999), temporanei 0–999. Massimo otto contatori con
`name`, `current` 0–maximum, `maximum` 1–999. La v5 trasferisce Slancio per le
schede originali/manuali; capacità e slot hanno riserve dedicate in `magic`,
senza duplicazione nei contatori. Le vecchie copie conservano i loro contatori
manuali senza trasformarli in magie. Nessun numero viene troncato per entrare.

Il firmware rifiuta chiavi sconosciute/duplicate, valori frazionari, Unicode
nel formato USB, strutture troppo annidate, payload troppo grandi e versioni
incompatibili. Prove sintetiche non equivalgono alla conversione del personaggio
reale Beyond: quella resta documentata separatamente in BEYOND-PROVE.md.

## Salvataggio e dadi

NVS nel namespace `pocketdd`, nuova partizione `pdd_state`, due chiavi alternate
`state_a`/`state_b`. Record PDD5 da **5168 + 552 × numero voci** byte, massimo
71408. Generazione e CRC32 espliciti; commit e rilettura prima dello stato
visibile. Nessun dump di struct, erase o fallback distruttivo. Errori I/O o
allocazioni insufficienti bloccano scritture; CRC e versioni future conservati.

Se la nuova area è vuota, legge PDD1/PDD2 da 384 byte, PDD3 da 512 o PDD4 da
5184 dalla vecchia `pdd_nvs`, aperta in sola lettura. Nessun record vecchio viene
riscritto al boot o al primo salvataggio nuovo. Magie assenti restano dichiarate
non importate. Il nuovo uploader supporta USB v1–v5. Preferenze avatar/tasti
locali conservate durante l'importazione. [Dettagli](MAGIE-TASTI.md).

PF e contatori vengono persistiti ad ogni modifica; “Annulla modifica” ripristina
l'ultima modifica di sessione a PF/contatori/inventario e la salva, non la vecchia scheda dopo
un'importazione. I tiri non scrivono flash e non hanno storico persistente.
L'uso prolungato della flash va misurato prima di cambiare strategia di scrittura.

Dadi con rejection sampling, senza modulo distorto. Wi-Fi/Bluetooth spenti;
`bootloader_random_enable()` mantiene la sorgente di entropia hardware SAR-ADC
per `esp_random()`. Non attivare in futuro ADC/radio senza coordinare questa
sorgente. Test deterministici verificano il calcolo, non certificano l'entropia
fisica del dispositivo.

## Esito verifiche della base v0.15, 19 ottobre 2026

- Test JavaScript: 125 passati; esportazione include valori corretti e non muta
  la sorgente. Regressione browser desktop/mobile e anteprima/download USB passati
  con scheda sintetica e profilo browser effimero.
- Core C nativo: PF temporanei, danni/cura e limiti, contatori, modificatori,
  d20 normale/vantaggio/svantaggio e rigetto, CRC noto, 3072 corruzioni singolo bit,
  384 troncamenti, generazioni con wrap e pacchetto generato dal JavaScript.
- Adapter NVS reale su backend finto: riavvio, recupero copia, errore I/O, blocco
  scritture senza cancellazione. Non è un power-cut test su flash fisica.
- UI LVGL reale renderizzata su host: navigazione, tiro, PF/annulla, contatori,
  conferma/annullamento importazione, errore di salvataggio, rettangoli touch nel
  cerchio e alti almeno 44 px. Un overflow del testo della navigazione corretto.
- Protocollo uploader su seriale finta: attesa conferma, successo, rifiuto, errore
  memoria, risposte frammentate e rifiuto backup/versione errati. Nessuna porta fisica usata.
- Cross-compilazione ESP32-S3 riuscita con ESP-IDF v5.5.5 (b774170ff46c393eeb5e495ea37936038d3f4f4f) e toolchain ufficiale Xtensa GCC 14.2.0 (esp-14.2.0_20260121, archivio verificato con SHA256 da tools.json). App `pocketdd.bin`: **542.432 byte**, partizione da 4.194.304 byte, **87% libera**. DIRAM statica 78.247 byte; buffer DMA, heap, task e memoria realmente disponibile vanno misurati a runtime. Nessun flash eseguito qui. Il build locale usa i sottomoduli necessari del tag ufficiale; non è stato eseguito in container. Le altre API deprecate di esptool 5 nel tooling IDF non impediscono la build.

### Cancello hardware prima di dichiarare usabile il dispositivo

- Verificare SKU/revisione e conservare recovery/flash originale.
- Flash controllato e avvio da USB; controllare PSRAM, colori, gap e quattro bordi.
- Verificare touch ai centri e bordi dei pulsanti, orientamento e trascinamenti.
- Scheda → DES → d20 → PF ±1 → annulla; niente tocchi involontari.
- Copia USB con conferma, annulla, scollegamento a metà, payload errato, timeout;
  raccolta PC immutata e precedente stato locale conservato se rifiutato.
- Riavvio normale e tagli di alimentazione durante salvataggio: riprendere sempre
  una copia valida o mostrare recupero, senza formattare.
- Avvio a batteria/PMIC, riscaldamento, consumi, autonomia e luminosità; poi sleep
  e spegnimento controllato. Nessuna autonomia dichiarata prima delle misure.

## Pacchetto scaricabile

Dopo build, test e commit, `python3 scripts/package-release.py /percorso/PocketD-D.zip`
produce sorgenti tracciati, app PC compilata, demo HTML, binari firmware,
licenze, metadati con commit e SHA256, e anteprime LVGL ottenute su host.
Richiede albero Git pulito e build della stessa versione. Esclude dipendenze
scaricate, toolchain, node_modules, impostazioni browser, credenziali e dati del
personaggio. Il pacchetto non esegue flash o importazioni automaticamente.

## Verifiche v0.16 — dadi animati

La build ESP32-S3, i test del core C/NVS e i 125 test JavaScript sono ripetuti
per questa versione. Le nuove verifiche del renderer, dei timer LVGL e dei
controlli sono descritte in [DADI-ANIMATI.md](DADI-ANIMATI.md). Formati USB,
salvataggio NVS e raccolta PC conservano le versioni precedenti. Nessuna scheda
personale viene letta o modificata per questi test: si usano dati sintetici.
Build v0.16: binario **573.104 byte**, partizione app **86% libera**, DIRAM statica
78.287 byte, canvas Dadi dinamico aggiuntivo da 81.920 byte. Il collaudo hardware
resta aperto.

## Verifiche v0.17 — lanci multipli

Il calcolo `dice_pool.c` valida i parametri prima di usare il RNG, somma tutte
le facce e aggiunge il bonus una sola volta; minimo zero solo in Danni. I test
C, LVGL su host, ASan/UBSan e build ESP32-S3 coprono forme, limiti e nuovi
controlli. Dettaglio in [DADI-ANIMATI.md](DADI-ANIMATI.md). La raccolta personale,
il protocollo USB e i record NVS non sono modificati. Collaudo fisico aperto.

Build ESP32-S3 v0.17: **713.808 byte** (circa 697 KiB), partizione
app da 4 MiB **83% libera**, DIRAM statica **78.335 byte**. Il canvas rimane
da 81.920 byte dinamici; heap, prestazioni e consumo vanno misurati sulla scheda.

## Verifiche v0.18 — prima base visiva del dispositivo

Tema applicato a tutte le viste firmware, scena e icone originali, font da 16 e
32 px aggiunti ai 20/26 px. Nomi lunghi con ellissi e altezza esplicita, PF zero
e massimi, stato errore memoria, separazione di testi/controlli verificati sul
renderer LVGL host. Controlli C/NVS/dadi/pool/uploader e 125 test JavaScript
passati, build PC ed ESP32-S3 riuscite. UI nativa ripetuta con ASan/UBSan senza
errori nei percorsi verificati; leak detector disabilitato per l'istanza globale
LVGL mantenuta fino alla fine del processo.

Build ESP32-S3 v0.18: **789.584 byte**, partizione app da 4 MiB **81% libera**.
La scena non introduce un canvas applicativo aggiuntivo; i dati immagine statici
occupano 12.272 byte in flash. Il canvas Dadi rimane da 81.920 byte dinamici.
Le nuove dimensioni non stabiliscono heap disponibile o prestazioni reali.
Nessun flash o lettura Beyond eseguiti per questa revisione. Anteprime e limiti
in [DIREZIONE-VISIVA.md](DIREZIONE-VISIVA.md).

## Verifiche v0.19 — avatar personale e animazione

126 test JavaScript e build PC passati. Core C/NVS verifica record v1 golden
prodotto dall'encoder della v0.18, lettura senza scritture, prima scrittura v2
con banca precedente conservata, riavvio e recupero. Parser USB v1/v2 e uploader
finto verificano aspetto, rifiuti e capacità del firmware prima di inviare v2.
La UI PC è verificata in un profilo browser effimero con avatar sintetico:
anteprima dai disegni firmware, download corretto e raccolta invariata.

Renderer avatar: 81 aspetti × quattro pose, trasparenza, ingrandimento esatto e
guardie del buffer. UI LVGL: animazione senza RNG/salvataggi, arresto nelle altre
sezioni, preferenza persistente, annulla PF conservato, errore memoria e
anteprima/conferma/annulla importazione. ASan/UBSan passati dopo la correzione
di una lettura oltre il buffer nello scaler di immagini indicizzate: la vista
grande ora usa pixel replicati alla dimensione nativa. Leak detector disabilitato
per l'istanza globale LVGL mantenuta fino alla fine del processo.

Cross-compilazione finale ESP32-S3 con ESP-IDF v5.5.5 e toolchain Xtensa ufficiale
esp-14.2.0_20260121, archivio verificato con SHA256 dal manifest ufficiale.
Binario **808.736 byte**, partizione app da 4 MiB **81% libera**. Buffer avatar
statico aggiuntivo da **5.824 byte**, canvas Dadi invariato a 81.920 byte dinamici.
Sono misure di compilazione: heap, resa, fluidità e consumi fisici restano da
misurare. Nessun flash o accesso al personaggio Beyond per questo incremento.

## Verifiche v0.20 — abilità e salvezze

127 test JavaScript; 1.890 confronti di integrazione modello PC → JSON USB →
parser e calcolo C per bonus e modalità, inclusa una conversione Beyond sintetica.
Unit test per livelli 1–20, caratteristiche 1–30, gradi 0–2, estremi −35/+52,
mezze competenze, annullamento vantaggio/svantaggio e contesti bloccati.

Record golden PDD1/PDD2 da 384 byte letti senza scritture; nuova banca PDD3 da
512 byte e recupero precedente verificati. CRC, 4096 corruzioni singolo bit,
512 troncamenti, wrap delle generazioni e blocco di versioni future. Core/parser,
adapter NVS, calcoli e UI passati anche con ASan/UBSan; leak detector disabilitato
per l'istanza globale LVGL mantenuta fino a fine processo.

UI LVGL: sei pagine di abilità, griglia salvezze, dettaglio contributi, risultati,
override degli effetti fotografati, protezione da eventi forzati e avviso per
copie vecchie senza competenze. Browser con profilo effimero: anteprima tabellare
di 30 prove, esportazione e raccolta sintetica invariata. Dettagli in
[ABILITA-SALVEZZE.md](ABILITA-SALVEZZE.md).

Build ESP32-S3 v0.20: **813.856 byte**, partizione app da 4 MiB **81% libera**.
Stesso canvas Dadi da 81.920 byte e buffer avatar da 5.824 byte. Stack principale
configurato a 8192 byte; sono dimensioni di compilazione e configurazione,
senza misure di heap o margine stack sul dispositivo. Nessun flash o accesso
al personaggio personale Beyond per questa revisione.

## Verifiche v0.21 — inventario e attacchi

129 test JS, 1608 confronti di combattimento PC/USB/C più i 1890 delle prove,
UI LVGL host, protocollo simulato e Chromium passanti. Migrazione golden PDD3,
CRC/troncamenti del nuovo record e consumo atomico verificati; controlli
ASan/UBSan inclusi. [Dettaglio e limiti](INVENTARIO-ATTACCHI.md).
La build ESP32-S3 è riuscita; nessun flash o lettura personale Beyond.
App `822.368` byte, partizione da 4 MiB **80% libera**.

## Verifiche v0.22 — magie e pulsanti

131 test JavaScript; 10400 confronti aggiuntivi PC/USB/C, oltre ai 3498 precedenti.
Costi, concentrazione/Ira, record compatti, migrazione golden v4, errori di
allocazione, tasti e UI LVGL host verificati, anche con ASan/UBSan. Chromium
verifica anteprima/download senza cambiare la raccolta effimera.
Nessun nuovo accesso Beyond, modifica personale, flash o collaudo fisico.
[Uso, sorgenti dei tasti e limiti](MAGIE-TASTI.md).
Build v0.22: **833.072 byte**, partizione app da 4 MiB **80% libera**.
