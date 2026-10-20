# Firmware PocketD-D 0.25 — schermo e impostazioni locali

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

La v0.23 aggiunge [riposi e dadi vita](RIPOSI-RECUPERI.md), anteprima e conferma,
recuperi breve/lungo, cure animate e scelta manuale/2014/2024 dei dadi da
restituire. Riposi è assegnabile ai tasti. USB v6 e PDD6; layout v0.22 invariato.

La v0.24 aggiunge [stato, condizioni e registro morte](STATO-CONDIZIONI-MORTE.md):
30 note, Avvelenato separato, d20 base animato, stabilizzazione e correzioni
confermate. Stato è assegnabile ai tasti. USB v7 e PDD7; layout invariato.

La v0.25 aggiunge [impostazioni schermo](SCHERMO-IMPOSTAZIONI.md): luminosità,
attenuazione per inattività e primo input di risveglio assorbito. PDD8; copie
USB v7 e layout invariati. Non è sospensione o telemetria batteria.

La v0.26 aggiunge [effetti locali CA/PF massimi](EFFETTI-LOCALI.md): otto
modificatori espliciti, basi conservate, anteprima e conferma, cure/riposi
integrati e scorciatoia Effetti. PDD9; USB v7 e layout invariati.

La v0.27 aggiunge [durate e scadenze](DURATE-EFFETTI.md) dei modificatori locali,
con avanzamento manuale dei round, pausa e scorciatoia Round. PDD10 (PDDA);
USB v7/layout invariati, niente scadenze da orologio o interpretazione dei testi.

Primo percorso: avvio con Elian originale o copia precedentemente salvata →
Scheda → caratteristica → d20 → risultato → Eroe → PF ±1 → annulla →
salvataggio locale. Anche PF temporanei e fino a otto contatori manuali.
Vantaggio e svantaggio selezionabili dal pulsante Base/Vant/Svant. Durante il volo
si può accelerare; modalità e navigazione attendono l'arresto del tiro.

Non è ancora una scheda completa: mancano gestione completa degli effetti, eccezioni ai tiri morte e morte istantanea,
monete, storico persistente, mappa e party. Le prove applicano i contributi
riconosciuti e trasferiti; i contesti da verificare restano bloccati. Le istantanee per prove e attacchi includono Avvelenato, Ira, attacco potente e dadi critici extra: Ira riconosciuta usa ora il contatore della capacità; altre durate e interazioni restano manuali. Il dado libero
usa bonus zero. CA/PF massimi effettivi sono basi esportate più modificatori
locali espliciti; non si ricostruiscono automaticamente dall'equipaggiamento.
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
iniziale 50%, configurabile da Schermo. Attenuazione per inattività disponibile;
nessun valore di batteria inventato. Audio, SD, RTC e IMU
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

Formato `pocketdd-device` v7, massimo 262144 byte, campi esatti di base: `format`, `version`,
`name`, `level`, `hp`, `maxHp`, `tempHp`, `armorClass`, `abilities` e `resources`.
Da v2 è obbligatorio anche `avatar`, con esattamente `skin`, `hair`, `cloak`,
`style`: indici interi 0–2 corrispondenti alle scelte PC. Da v3 è obbligatorio
`training`, con gradi, competenze, extra, mezze competenze, blocchi e le due
istantanee; [schema completo](ABILITA-SALVEZZE.md). Il firmware accetta anche
v1/v2/v3/v4/v5/v6: v1 usa l'aspetto base; v1/v2 segnalano le competenze mancanti.
Da v4 è obbligatorio `inventory`; [schema e omissioni](INVENTARIO-ATTACCHI.md).
Da v5 è obbligatorio `magic`; [schema, testi e tasti locali](MAGIE-TASTI.md).
Da v6 è obbligatorio `recovery`; [dadi vita e regole esplicite](RIPOSI-RECUPERI.md).
Da v7 è obbligatorio `status`; [note e registro morte](STATO-CONDIZIONI-MORTE.md).
La preferenza locale per
l'animazione non viene trasferita dal PC: la conferma conserva quella già attiva.
Il nuovo uploader negozia `PDD READY 1/2/3/4/5/6/7` e blocca una copia prima dell'invio
se il firmware non ne supporta la versione. Usare lo script di questa versione;
i precedenti uploader non riconoscono `PDD READY 7`.
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
`state_a`/`state_b`. Record PDD10 (tag `PDDA`) da **7966 + 552 × numero voci** byte, massimo
74206. Generazione e CRC32 espliciti; commit e rilettura prima dello stato
visibile. Nessun dump di struct, erase o fallback distruttivo. Errori I/O o
allocazioni insufficienti bloccano scritture; CRC e versioni future conservati.

Se la nuova area è vuota, legge PDD1/PDD2 da 384 byte, PDD3 da 512 o PDD4 da
5184 dalla vecchia `pdd_nvs`, aperta in sola lettura. Nessun record vecchio viene
riscritto al boot o al primo salvataggio nuovo. Magie assenti restano dichiarate
non importate. I record PDD5 della v0.22 sono leggibili dalla nuova partizione;
i record PDD6 sono leggibili senza creare registri morte. PDD7 mantiene stato
e contatori; le copie precedenti a PDD8 ricevono 50%/mai senza riscrittura al boot.
PDD8 conserva le preferenze; le copie precedenti iniziano senza effetti locali.
PDD9 conserva i modificatori con durata manuale. Le nuove scritture PDD10
richiedono firmware v0.27 o successivo:
[compatibilità e downgrade](DURATE-EFFETTI.md).
Il nuovo uploader supporta USB v1–v7. Preferenze avatar/tasti
locali conservate durante l'importazione. [Dettagli](MAGIE-TASTI.md).

PF e contatori vengono persistiti ad ogni modifica; “Annulla modifica” ripristina
l'ultima modifica di sessione a PF/contatori/inventario/magie/recuperi/stato/effetti/durate e la salva, non la vecchia scheda dopo
un'importazione. I tiri liberi/attacco/danni non scrivono flash; cure e dadi vita salvano PF e consumo; le salvezze morte salvano PF e contatori prima dell’animazione. Non c’è storico persistente.
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


## Verifiche v0.23 — riposi e dadi vita

134 test JS; 7840 nuovi confronti PC/USB/C per riposi/dadi vita, 21738 totali
inclusi prove/combattimento/magie. Schema USB v6, fixture indipendente PDD5,
round trip PDD6 con 20 riserve e dati malformati a CRC valido. UI LVGL con
selezione multiclass, recuperi espliciti, cure/cap/annulla, errori di memoria,
riuso della cura estratta su errore, import busy e tasti protetti. Chromium
verifica anteprima/copia v6 e raccolta effimera invariata. Sanitizzatori e build
ESP32-S3 passati; nessun flash, test elettrico o JSON personale pubblicato.
[Dettagli](RIPOSI-RECUPERI.md).

Build v0.23: **839.696 byte** (80% della partizione app da 4 MiB libera);
layout flash invariato rispetto alla v0.22.


## Verifiche v0.24 — stato, condizioni e morte

138 test JS; 16013 nuovi confronti PC/USB/C, 37751 totali. Schema USB v7,
fixture indipendente PDD6 e migrazione senza scritture al boot. UI LVGL con
correzioni, anteprima/annullamento, tre fallimenti, tiro 1/10/20, tasti protetti,
import busy, errore di scrittura senza rilanci e 30 note con nomi da 60 caratteri.
Chromium verifica configurazione Avvelenato indipendente, copia v7 e raccolta
invariata; regressione di importazione Beyond estesa su dati sintetici, anche
su mobile 390/320 px. Sanitizzatori e build ESP32-S3 passati. Nessuna prova
fisica o lettura personale Beyond in questo incremento.
[Dettagli](STATO-CONDIZIONI-MORTE.md).

Build v0.24: **846.944 byte**; partizione app da 4 MiB **80% libera**;
layout v0.22/v0.23 conservato.


## Verifiche v0.25 — schermo e risveglio

138 test JS e 37751 confronti PC/USB/C mantenuti. Controller C e ciclo principale
esercitati su host per scadenze/wrap, contatti mantenuti, risveglio, entrambe le
sorgenti di input, conferme/import/dadi, errore di comando e riprova. PDD8:
preferenze persistite, valori malformati rifiutati e fixture PDD7 indipendente.
UI LVGL, errori di salvataggio, import e annullamento verificati. Sanitizzatori
e build ESP32-S3; nessuna prova fisica o nuova lettura Beyond.
[Uso, compatibilità e limiti energia](SCHERMO-IMPOSTAZIONI.md).

Build v0.25: **849024 byte**, partizione app da 4 MiB **80% libera**;
layout flash invariato.


## Verifiche v0.26 — effetti locali

17266 casi indipendenti di basi/delta, effetti inattivi, limiti e rifiuti senza
mutazione; cure sopra/sotto la base, riposi, morte, annotazioni e otto voci.
Regressioni 138 JS, build PC, 37751 confronti PC→USB→C, migrazioni golden PDD1–PDD8,
I/O/allocazioni/CRC e record massimo. UI LVGL reale su host: bozze, conferme,
errori, limite PF, annullamento, pagine, import e tasti protetti. ASan/UBSan per
core/parser, storage, effetti, combattimento, riposi, stato, allocazioni e UI.

Build v0.26: **852688 byte**, partizione app da 4 MiB **80% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0, layout flash invariato. Nessun flash, prova fisica,
nuova lettura personale Beyond o modifica della raccolta personale.


## Verifiche v0.27 — durate e scadenze

40648 nuovi avanzamenti/sottoinsiemi verificano durate 1–99, pausa, zero/scaduto,
riattivazione esplicita e scadenze simultanee di otto effetti. Rifiuti senza
mutazione su totali finali invalidi, nessun limite PF intermedio e nessuna cura
implicita. Regressioni 138 JS, build PC, 37751 confronti PC→USB→C e 17266 casi dei
modificatori; fixture PDD1–PDD9 e NVS con durata salvata/riavvio/scadenza.
UI LVGL host per bozze, conferme, errori/riprova, annullamento, import busy,
scorciatoia Round e attenuazione sospesa. ASan/UBSan su core/parser, storage,
effetti/durate, riposi, stato, allocazioni e UI; leak detector disattivato.

Build v0.27: **855360 byte**, partizione app da 4 MiB **80% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0, layout flash invariato. Nessun flash o prova fisica.
Nessuna nuova lettura personale Beyond o modifica della raccolta personale.
