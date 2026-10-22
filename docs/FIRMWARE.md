# Firmware PocketD-D 0.52 — storico dei tiri

Il prodotto principale è il dispositivo. La parte PC serve per importare,
configurare, trasferire e conservare backup. La simulazione browser rimane un
banco di prova; non stabilisce la priorità delle funzioni del prodotto.

[Novità v0.52: storico persistente, tasti e limiti](STORICO-TIRI.md).
[Borsello v0.51: monete, conferme e compatibilità](BORSELLO.md).
[Grafica v0.50: grafica e UX di tutte le viste, catalogo e verifiche](UI-COMPAGNO.md).
[Avatar illustrato v0.49: 81 aspetti e reazioni](AVATAR-ILLUSTRATO.md).
[Integrazione home, menu, PF e tasti v0.48](HOME-COMPAGNO.md).
PDD25 + registro PDH1, USB v9, backup PC v11; layout invariato e lettura dei salvataggi precedenti.

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

La v0.28 aggiunge [danni ricevuti e difese manuali per tipo](DANNI-DIFESE.md),
con anteprima dei temporanei/PF ed esito della concentrazione scelto col DM.
Non configurato distinto da normale; zero PF/morte istantanea rimangono bloccati.
PDD11 (PDDB), USB v7 e layout invariati.

La v0.29 aggiunge il [tiro COS di concentrazione](CONCENTRAZIONE.md), bonus e
CD espliciti, danni/esito salvati insieme prima dell'animazione e retry del
medesimo tiro. Formati e layout invariati; nessun giornale persistente della faccia.

Primo percorso: avvio con Elian originale o copia precedentemente salvata →
Scheda → caratteristica → d20 → risultato → Eroe → PF ±1 → annulla →
salvataggio locale. Anche PF temporanei e fino a otto contatori manuali.
Vantaggio e svantaggio selezionabili dal pulsante Base/Vant/Svant. Durante il volo
si può accelerare; modalità e navigazione attendono l'arresto del tiro.

Non è ancora una scheda completa: mancano gestione completa degli effetti, eccezioni ai tiri morte e morte istantanea,
registro completo delle modifiche, mappa e party. Le prove applicano i contributi
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

Formato corrente `pocketdd-device` v9, massimo 262144 byte, campi esatti di base: `format`, `version`,
`name`, `level`, `hp`, `maxHp`, `tempHp`, `armorClass`, `abilities` e `resources`.
Da v2 è obbligatorio anche `avatar`, con esattamente `skin`, `hair`, `cloak`,
`style`: indici interi 0–2 corrispondenti alle scelte PC. Da v3 è obbligatorio
`training`, con gradi, competenze, extra, mezze competenze, blocchi e le due
istantanee; [schema completo](ABILITA-SALVEZZE.md). Il firmware accetta anche
v1–v8: v1 usa l'aspetto base; v1/v2 segnalano le competenze mancanti.
Da v4 è obbligatorio `inventory`; [schema e omissioni](INVENTARIO-ATTACCHI.md).
Da v5 è obbligatorio `magic`; [schema, testi e tasti locali](MAGIE-TASTI.md).
Da v6 è obbligatorio `recovery`; [dadi vita e regole esplicite](RIPOSI-RECUPERI.md).
Da v7 è obbligatorio `status`; [note e registro morte](STATO-CONDIZIONI-MORTE.md).
Da v8 gli oggetti possono avere pool di cariche. Da v9 è obbligatorio `wallet`,
con saldo verificato e cinque tagli; [schema e limiti](BORSELLO.md).
La preferenza locale per
l'animazione non viene trasferita dal PC: la conferma conserva quella già attiva.
Il nuovo uploader negozia `PDD READY 1` … `PDD READY 9` e blocca una copia prima dell'invio
se il firmware non ne supporta la versione. Usare lo script di questa versione;
gli uploader precedenti non riconoscono `PDD READY 9`.
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
`state_a`/`state_b`. Record PDD25 (tag `PDDP`) da **8879 + 552 × numero voci** byte, massimo
75119. Generazione e CRC32 espliciti; commit e rilettura prima dello stato
visibile. Nessun dump di struct, erase o fallback distruttivo. Errori I/O o
allocazioni insufficienti bloccano scritture; CRC e versioni future conservati.

Se la nuova area è vuota, legge PDD1/PDD2 da 384 byte, PDD3 da 512 o PDD4 da
5184 dalla vecchia `pdd_nvs`, aperta in sola lettura. Nessun record vecchio viene
riscritto al boot o al primo salvataggio nuovo. Magie assenti restano dichiarate
non importate. I record PDD5 della v0.22 sono leggibili dalla nuova partizione;
i record PDD6 sono leggibili senza creare registri morte. PDD7 mantiene stato
e contatori; le copie precedenti a PDD8 ricevono 50%/mai senza riscrittura al boot.
PDD8 conserva le preferenze; le copie precedenti iniziano senza effetti locali.
PDD9 conserva i modificatori con durata manuale, PDD10 conserva i timer; le
vecchie copie iniziano con difese da configurare. PDD11 conserva le difese;
le copie precedenti a PDD12 iniziano con effetti senza vincoli alla concentrazione.
PDD12 conserva i vincoli numerici; le copie precedenti a PDD13 iniziano senza
extra temporanei alle difese. Dopo gli otto riferimenti numerici si aggiungono
65 byte (proprietà/attivazione/timer/round/vincolo per 13 tipi), prima del CRC.
PDD13 conserva gli extra; le copie precedenti a PDD14 mantengono Sempre per
ogni proprietà. Al payload si aggiungono 78 byte di selettori base/extra, prima
del CRC. PDD15 aggiunge altri 208 byte per due extra indipendenti in più per
tipo; le copie precedenti mantengono il primo extra e iniziano gli altri vuoti.
PDD16 aggiunge 30 byte per tre profili di riduzione; copie precedenti con
profili vuoti, senza perdere gli extra. PDD17 aggiunge 16 byte: tre riferimenti
fonte/costo/reazione e lo stato globale della reazione. PDD1–PDD16 sono letti
senza spesa e con reazione pronta, conservando i profili già presenti.
PDD18 conserva la dimensione e aggiunge i tipi fonte 3 (slot ordinari) e 4
(patto), indice livello 0–8; i costi/reazione PDD17 restano conservati.
PDD19 aggiunge 50 selettori Round/Inizio/Fine per effetti, extra e profili.
Le copie precedenti mantengono Round senza scritture al boot.
PDD20 aggiunge 100 byte di residui in minuti e i selettori Minuti/Ore;
le copie PDD1–PDD19 mantengono i vecchi conteggi e iniziano senza timer di tempo.
PDD21 aggiunge 300 byte per i pool di cariche (saldo/massimo/recupero), tipo
costo 5 per gli oggetti; PDD1–20 letti senza pool e senza scritture al boot.
PDD22 aggiunge 24 byte: due spese aggiuntive per ciascuno dei tre profili;
PDD1–21 letti con la prima spesa conservata e le altre vuote, senza scritture al boot.
PDD23 aggiunge un byte di effetti locali sui tiri, senza inferenza dalle note;
PDD1–22 letti con i quattro nuovi effetti disattivati e Avvelenato conservato.
PDD24 aggiunge 21 byte: disponibilità e cinque saldi uint32; PDD1–23 iniziano
con borsello non disponibile, senza scritture al boot. PDD25 aggiunge quattro
byte di identità locale dello storico e il valore 13 per la scorciatoia Storico.
PDD1–24 ricevono identità zero, conservando monete e tasti precedenti. Le nuove
scritture richiedono firmware v0.52 o successivo; [registro e compatibilità](STORICO-TIRI.md).
Il nuovo uploader supporta USB v1–v9. Preferenze avatar/tasti
locali conservate durante l'importazione. [Dettagli](MAGIE-TASTI.md).

PF e contatori vengono persistiti ad ogni modifica; “Annulla modifica” ripristina
l'ultima modifica di sessione a PF/contatori/inventario/magie/recuperi/stato/effetti/durate/difese/monete e la salva, non la vecchia scheda dopo
un'importazione. I tiri liberi/attacco/danni non scrivono flash; cure e dadi vita salvano PF e consumo; le salvezze morte salvano PF e contatori prima dell’animazione. Lo storico dei tiri v0.52 è un registro separato, non un
giornale completo delle modifiche o un annullamento persistente.
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
effetti/durate, riposi, stato, core/parser, NVS, allocazioni e UI; leak detector disattivato.

Build v0.27: **855360 byte**, partizione app da 4 MiB **80% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0, layout flash invariato. Nessun flash o prova fisica.
Nessuna nuova lettura personale Beyond o modifica della raccolta personale.


## Verifiche v0.28 — danni e difese

139904 casi indipendenti di tipo/flag/importo/PF/temporanei, profili non
configurati, concentrazione con temporanei, arrotondamenti e immunità. Blocco di
zero PF e danni massicci sul massimo effettivo; errori senza mutazione, durata
indipendente e fine Ira/concentrazione a zero. Regressioni 138 JS, build PC,
37751 confronti PC→USB→C, 17266 casi degli effetti e 40648 delle durate.
Fixture PDD1–PDD10, NVS senza scritture al boot, tutti i profili/record massimo
e CRC valido con campi invalidi. UI LVGL con bozze, conferme, retry, esiti della
concentrazione, zero danni senza scritture, annullamento, limiti, import e tasti.
Controllo delle sovrapposizioni fra pulsanti oltre alle dimensioni e al cerchio.
ASan/UBSan su core/parser, storage, danni/difese, effetti/durate, riposi, stato,
core/parser, NVS, allocazioni e UI; leak detector disattivato nel sandbox.

Build v0.28: **860688 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0, layout invariato. Nessun flash, collaudo fisico,
nuova lettura personale Beyond o modifica della raccolta personale.


## Verifiche v0.29 — salvezza di concentrazione

78840 casi di COS/competenza/bonus/facce/modalità/CD, criterio max 30 e naturale
1/20 confrontati sul totale. Rifiuti prima del RNG per salvezze mancanti/bloccate
e piani non idonei; candidato danni/esito riletto in PDD11. UI LVGL per scelte,
conferma, successo/fallimento, save error con tiro fissato, opzioni bloccate,
nessun rilancio dal risultato, canvas mancante, annullamento e fallback manuale.
Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 139904 casi dei danni,
17266 degli effetti e 40648 delle durate. ASan/UBSan su concentrazione/danni,
core/parser, NVS, allocazioni e UI; leak detector disattivato. PDD11 e layout invariati.

Build v0.29: **863008 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. Il controller NVS rimane bloccato dopo errore I/O:
riavvio e verifica necessari; il tiro pendente non sopravvive al reboot. Nessun
flash, collaudo fisico, nuova lettura Beyond o modifica della raccolta personale.


## Verifiche v0.30 — effetti e concentrazione

4608 casi indipendenti di vincoli/bersagli/fine, comprese stessa magia, Ira,
zero PF e danni con tiro; disattivazione simultanea, nessuna cura implicita,
costi e validità dei totali anche dopo la fine. Delta PF del riposo con segno,
timer/pausa, riferimenti non correnti e otto byte corrotti con CRC valido.
Fixture sintetica PDD11 dell'encoder v0.29 `a75afd1`; lettura PDD1–PDD11,
nessuna scrittura al boot e copia originale conservata al primo salvataggio.
UI LVGL di vincolo/anteprima/fine, sostituzione, riposo, danni, errore e tiro
fissato, annullamento e tasti protetti. Regressioni 138 JS/build PC,
37751 confronti PC→USB→C, 17266 effetti, 40648 durate, 139904 danni e 78840
salvezze. ASan/UBSan su vincoli/effetti/durate/danni/concentrazione,
magie/riposi/stato, core/parser, NVS, allocazioni e UI; leak detector disattivato nel sandbox.

Build v0.30: **865008 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD12, layout/USB v7 invariati. Nessun flash,
collaudo fisico, nuova lettura Beyond o modifica della raccolta personale.
[Uso e limiti](EFFETTI-CONCENTRAZIONE.md).


## Verifiche v0.31 — difese temporanee

32032 combinazioni indipendenti di proprietà base/extra, tipo, attivazione e
danno; 64350 avanzamenti delle durate 1–99. Proprietà uguali non moltiplicate,
base sconosciuta conservata, scadenze comuni e rifiuti atomici, difese all'impatto,
CD/temporanei corretti e fine concentrazione congiunta. Fixture sintetica PDD12
dall'encoder v0.30 `a9bcd6f`; lettura PDD1–PDD12 senza scritture al boot,
prima modifica nella banca alternata e 65 campi corrotti con CRC valido.
UI LVGL di base/extra, durata/vincolo, scadenze filtrate e paginazione dei 13 tipi,
21 voci collegate, errore/riprova, annullamento e import; tasti e conferme protetti.
Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 17266 effetti, 40648 durate,
139904 danni, 78840 salvezze e 4608 vincoli. ASan/UBSan su extra/effetti/durate,
danni/concentrazione/vincoli, magie/riposi/stato, core/parser, NVS, allocazioni e UI; leak detector
disattivato nel sandbox. Nuove scritture PDD13; USB v7 e layout invariati.

Build v0.31: **869200 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. Nessun flash, collaudo fisico, nuova lettura Beyond
o modifica della raccolta personale. [Uso e limiti](DIFESE-TEMPORANEE.md).


## Verifiche v0.32 — danni misti

494312 casi indipendenti di tipi/proprietà/quantità, 19968 suddivisioni e 720
permutazioni verificano raggruppamento prima degli arrotondamenti, totale,
temporanei e PF. Fino a 119988 senza troncamento, profilo mancante che blocca
tutto, soglie aggregate, difese presenti all'impatto, un solo tiro COS e scelta
CD max 30. Candidato danni/concentrazione riletto in PDD13. UI LVGL di righe,
bozze/rimozione/paginazione, dettaglio Tipi, conferma, annullamento, grandi
valori, immunità e save error con tiro/dettaglio fissati. Risultato con profili
originali anche dopo la fine concentrazione; tasti, import e schermate protetti.
Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 139904 danni, 78840
salvezze, 4608 vincoli, 32032 difese/64350 avanzamenti, 17266 effetti e 40648
durate. ASan/UBSan su calcolo misto/danni/concentrazione/vincoli/difese,
effetti/durate, magie/riposi/stato, allocazioni e UI; leak detector disattivato.

Build v0.32: **872416 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD13/USB v7/layout invariati: righe e dettaglio sono
solo in RAM, nessuna nuova migrazione o grande stato sullo stack. Nessun flash,
collaudo fisico, nuova lettura Beyond o modifica della raccolta personale.
[Uso e limiti](DANNI-MISTI.md).


## Verifiche v0.33 — condizioni difensive

404352 casi indipendenti di tipo/proprietà/regole base-extra/attivazione e tutte
le 81 fonti parziali, confrontati con la valutazione booleana delle descrizioni
complete. Fatti irrilevanti, fonti complementari, immunità certa, gruppi dello
stesso tipo con applicabilità diversa, rifiuti prima del RNG, vincoli/timer e
snapshot del colpo. Fixture sintetica PDD13 dell'encoder v0.32 `7ed65df`,
lettura PDD1–PDD13 senza scritture al boot e banca originale conservata; 78
selettori corrotti con CRC valido e campi attaccati a proprietà assenti.
UI LVGL di base/extra, regole complete prima/dopo, Fonte con sì/no/?, cancel,
profili necessari, gruppi, errore e tiro fissato, annullamento, import e testi
lunghi. Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 494312 misti,
19968 suddivisioni/720 permutazioni, 139904 danni, 78840 salvezze, 4608 vincoli,
32032 difese/64350 avanzamenti, 17266 effetti e 40648 durate. ASan/UBSan su
condizioni/misti/danni/concentrazione/vincoli/difese, effetti/durate,
magie/riposi/stato, core/parser, NVS, allocazioni e UI; leak detector disattivato.

Build v0.33: **877408 byte**, partizione app da 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD14, USB v7/layout invariati. Nessun flash,
collaudo fisico, nuova lettura Beyond o modifica della raccolta personale.
[Uso e limiti](DIFESE-CONDIZIONALI.md).


## Verifiche v0.34 — tre extra indipendenti

212992 casi indipendenti di tipo/proprietà/scadenze e 83349 contesti parziali
per tre regole; regressioni 404352 condizioni, 494312 misti/19968 suddivisioni/
720 permutazioni, 139904 danni, 78840 salvezze, 4608 vincoli, 32032 difese e
64350 avanzamenti, 17266 effetti/40648 durate. Timer fino al bit 38, conteggio
massimo di 47 vincoli, round numerico respinto atomicamente, fine/rilancio/Ira/
zero PF/riposo/salvezza e protezioni non vincolate ancora attive. Tutti i 208
nuovi campi corrotti con CRC valido; record massimo, fixture PDD1–PDD14 e
migrazione NVS senza scritture al boot, con banca originale conservata.
UI LVGL nativa per tre bozze, confronto, regole complete, singolo commit e
sibling invalido, errore/retry/annullamento, danno all'impatto, scadenze diverse,
importazione con soli extra aggiuntivi, tasti protetti e 13 pagine dei 39 timer.
ASan/UBSan su suite difese/danni/effetti, magie/riposi/stato, core/parser, NVS,
allocazioni e UI; leak detector disattivato. 138 test JS e build PC passati;
37751 confronti PC→USB→C conservati.

Firmware v0.34: **880096 byte**, partizione app 4 MiB **79% libera**,
ESP-IDF 5.5.5/LVGL 9.4.0. PDD15, USB v7/layout invariati. Nessun flash o
collaudo fisico, nessuna lettura Beyond o modifica della raccolta personale.
[Uso e limiti](DIFESE-MULTIPLE.md).


## Verifiche v0.35 — riduzioni esplicite prima/dopo

66560 casi indipendenti di tipo/proprietà/danno/riduzioni prima-dopo, con
confronto del danno e dei PF/temporanei; 95728 assegnazioni/suddivisioni e tutte
le 720 permutazioni di sei parti, una sola sottrazione finale. Eccedenze,
immunità/zero, profili/fonti sconosciuti ancora bloccanti, campi malformati,
rifiuti prima del RNG, CD sul totale finale e candidato PDD15. UI LVGL di editor
generale/per parte, annulla/usa, promozione e aggiunta, conto completo e quantità
scelte/effettive, retry fissato, esito originale dopo fine concentrazione,
annullamento, import e tasti protetti. Anche sei parti, somme a cinque/sei cifre,
totale massimo, controlli ≥44 px e testi senza sovrapposizioni.

Suite firmware completa passata: migrazioni PDD1–PDD14 e record PDD15, parser,
NVS/allocazioni, 37751 confronti PC→USB→C, dadi/tasti/schermo e regressioni
404352 condizioni, 212992 extra/83349 fonti, 494312 misti/19968 suddivisioni,
139904 danni, 78840 salvezze, 4608 vincoli, 32032 difese/64350 avanzamenti,
17266 effetti e 40648 durate. ASan/UBSan su riduzioni, extra/condizioni/misti,
danni/concentrazione/vincoli/difese, effetti/durate, magie/riposi/stato,
allocazioni e UI; leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.35: **882288 byte**, partizione app 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD15/USB v7/layout invariati, quantità del colpo solo
RAM. Nessun flash/collaudo fisico, nuova lettura Beyond o modifica della raccolta
personale. [Uso e limiti](RIDUZIONI-DANNI.md).


## Verifiche v0.36 — profili persistenti di riduzione

176904 combinazioni indipendenti di slot/tipo/regola/fonte parziale/fase/attivo/
inclusione, 2178 assegnazioni del budget e 1440 permutazioni nelle due fasi.
Parti ammesse, intero colpo Dopo, manuali precedenti al profilo, budget condiviso,
clamp, rifiuti prima del RNG, uno snapshot all'impatto e candidato fissato;
scadenze, rilancio/Ira/zero PF/riposo/fallimento, vincoli inattivi, round numerico
respinto atomicamente, 42 timer e massimo 50 effetti/difese collegati.
Record massimo, tutti i 30 byte aggiunti corrotti con CRC valido, fixture
PDD1–PDD15 e controller NVS senza scritture al boot e banca originale conservata.
UI LVGL di configurazione completa e confronti, invalidi/testi lunghi/limiti,
selezione interna/esterna con annullamento, budget/esclusioni/fonte, errore/retry,
conto originario, round/scadenza, import con soli profili locali, tre gesti
protetti e 14 pagine dei 42 timer. Controlli ≥44 px nel cerchio e nessuna
sovrapposizione fra testi/controlli.

Suite firmware e regressioni complete passate, inclusi 37751 confronti PC→USB→C,
66560 riduzioni/95728 suddivisioni/720 permutazioni, 404352 condizioni,
212992 extra/83349 fonti, 494312 misti/19968 suddivisioni, 139904 danni,
78840 salvezze, 4608 vincoli, 32032 difese/64350 avanzamenti, 17266 effetti e
40648 durate. ASan/UBSan su profili/riduzioni, extra/condizioni/misti, danni/COS/
vincoli/difese, effetti/durate, magie/riposi/stato, allocazioni, core/parser,
NVS e UI; leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.36: **891008 byte**, partizione app 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD16, USB v7/layout invariati. Nessun flash/collaudo
fisico, lettura Beyond o modifica della raccolta personale.
[Uso e limiti](PROFILI-RIDUZIONE.md).


## Verifiche v0.37 — costi e reazione

354816 combinazioni indipendenti di slot/fase/fonte/costo/saldo/reazione/danno/
difese, tutti gli 8 contatori e 119 riferimenti a usi testati, pool di magie
limitate e capacità. Spesa esatta fino a zero, gratis/solo reazione, blocco prima
del RNG per saldo/reazione e tutti i vincoli del colpo; nessuna spesa parziale.
Un candidato COS con costo, reazione, PF e fine congiunta; snapshot del nome e
dei saldi, riprova fissata e annullamento completo. Reset del solo turno,
reazione invariata al round e ai riposi, recuperi delle capacità conservati.
PDD17 massimo, tutti i 16 nuovi byte corrotti con CRC valido e fixture PDD16
originale; migrazione NVS senza scritture al boot, banca originale conservata.

UI LVGL completa: configurazione, quarto confronto, consultazione, saldi e costi
a danno zero, blocchi, ritorno allo stesso impatto dopo recupero reazione,
conferma/errore/riprova/annulla, tasti/import protetti, assenza di pool,
zero/massimo e nomi da 40 caratteri anche con glifi larghi. Verifica dell'altezza
del testo avvolto, nessuna sovrapposizione e controlli ≥44 px dentro il cerchio.
Catture host visionate; anteprima `firmware-host-preview/profile-costs-preview.png`
nell'archivio. Non sono immagini del display fisico.

Suite firmware completa e 37751 confronti PC→USB→C passati; ASan/UBSan su
costi/profili/riduzioni, extra/condizioni/misti, danni/COS/vincoli/difese,
effetti/durate, magie/riposi/stato, allocazioni, core/parser, NVS e UI finale;
leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.37: **894832 byte**, partizione app 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD17 legge PDD1–PDD16, USB v7/layout invariati.
Nessun flash/collaudo fisico, nuova lettura Beyond o modifica della raccolta
personale. Slot/cariche oggetto, costi multipli, requisiti e conseguenze
automatiche delle capacità restano successivi. [Uso e limiti](COSTI-REAZIONE.md).


## Verifiche v0.38 — costi in slot

233280 combinazioni indipendenti di tre profili/due fasi/pool ordinari-patto/
nove livelli/costo/saldo/reazione/danno/immunità. Massimi 0–9, costi 0–10,
dati Magie mancanti, riferimenti fuori range, saldo esatto a zero, blocco prima
del RNG e nessuna sostituzione di livello/pool. Candidato COS unico con spesa,
reazione e fine dei vincoli; round/turno senza recupero slot e riposi distinti.
PDD18 massimo, corruzioni con CRC valido di tipo/indice/costo, nuove fonti
rifiutate nei record PDD17. Fixture PDD17 dell'encoder v0.37 originale, lettura
senza scritture al boot, prima banca alternata e riavvio con costi conservati.

UI LVGL di editor e confronto, consultazione, pool esaurito e livello 9,
slot ordinari/patto, costo a danno zero, errore/riprova con e senza COS,
annullamento completo, snapshot all'impatto, riposi e import. Tasti configurabili
conservati e bozze protette; controlli ≥44 px nel cerchio e testi senza
sovrapposizioni. Anteprima host visionata, inclusa nell'archivio come
`firmware-host-preview/slot-costs-preview.png`; non è una cattura fisica.

Suite firmware completa e 37751 confronti PC→USB→C passati; regressioni
354816 costi contatori/usi, 176904 profili e suite difese/danni/effetti precedenti.
ASan/UBSan su slot/costi/profili/riduzioni, extra/condizioni/misti, danni/COS/
vincoli/difese, effetti/durate, magie/riposi/stato, allocazioni, core/parser,
NVS e UI finale; leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.38: **895744 byte**, partizione app 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD18 legge PDD1–PDD17 senza aumentare la dimensione;
USB v7/layout invariati. Nessun flash/collaudo fisico, nuova lettura Beyond o
modifica della raccolta personale. Cariche oggetto distinte dalla quantità,
costi multipli, requisiti/effetti automatici e durate di turno/minuti ancora
successivi. [Uso e limiti](COSTI-SLOT.md).


## Verifiche v0.39 — durate del proprio turno

10800 combinazioni indipendenti di 50 posizioni/famiglie, fase Round/Inizio/Fine,
conteggio 1/2/99, attivazione/pausa, reazione e concentrazione. Cinquanta
scadenze simultanee con 8 bit numerici e 42 difensivi, limite PF senza cura,
nessun avanzamento nelle fasi diverse, riposi senza tick e fine dei vincoli.
Scadenza numerica fuori limite respinta prima di difese/profili/reazione;
stato e report immutati. Massimo PDD19, tutti i 50 selettori corrotti con CRC
valido, combinazioni senza timer e selettori numerici inutilizzati respinti.
Fixture PDD18 dell'encoder v0.38 originale, nessuna scrittura al boot,
prima banca alternata e riavvio con timer/costi/reazione conservati.

UI LVGL dei tre editor, modalità e conteggio, anteprime di inizio/fine con
reazione/CA/PF prima/dopo, dettaglio, errore/riprova/annulla, rifiuti atomici,
17 pagine per 50 voci e profilo scaduto nel colpo ancora preparato. Import e
tasti protetti, scorciatoia Stato conservata, PWR lungo hardware; controlli
≥44 px nel cerchio e testi senza sovrapposizioni. Catture host visionate:
`firmware-host-preview/turn-durations-preview.png` nell'archivio, nessuna
cattura del display fisico.

Suite firmware completa e 37751 confronti PC→USB→C passati, incluse le regressioni
233280 slot, 354816 costi contatori/usi e 176904 profili. ASan/UBSan su turni/
slot/costi/profili/riduzioni, extra/condizioni/misti, danni/COS/vincoli/difese,
effetti/durate, magie/riposi/stato, allocazioni, core/parser, NVS e UI finale;
leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.39: **899008 byte**, partizione app 4 MiB **79% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD19 legge PDD1–PDD18; USB v7/layout invariati.
Nessun flash/collaudo fisico, nuova lettura Beyond o modifica della raccolta
personale. Altri soggetti/minuti/ore, cariche oggetto e automatismi ancora
successivi. [Uso e limiti](DURATE-TURNO.md).


## Verifiche v0.40 — tempo di gioco

57600 combinazioni indipendenti di 50 posizioni/famiglie, Minuti/Ore, residuo,
intervallo, pausa, reazione e concentrazione. 13122 sequenze di otto effetti
confrontate con un riferimento separato minuto per minuto: scadenze simultanee,
ordine, perdite PF non restituite e rifiuti intermedi anche con finale valido.
Rollback dopo un cap PF già proposto; 50 scadenze, ore parziali serializzate,
round/turni/riposi separati e limiti di avanzamento. PDD20 massimo, tutti i 100
nuovi byte corrotti con CRC valido e selettori temporali respinti nei record
PDD19. Fixture PDD19 dell'encoder v0.39 originale, nessuna scrittura al boot,
prima banca alternata e riavvio con minuti residui conservati.

UI LVGL dei tre editor, minuti/ore e precisione, comando Tempo, anteprima/dettaglio,
PF e rifiuti cronologici, errore/riprova/annulla, nessuna spesa di risorse o
reazione, limiti/no-op, 17 pagine e import. Tasti configurabili conservati e
modali protette; controlli ≥44 px dentro il cerchio, testi senza sovrapposizioni.
Catture host visionate, incluse nell'archivio come
`firmware-host-preview/elapsed-time-preview.png`; nessuna cattura fisica.

Suite firmware completa e 37751 confronti PC→USB→C passati; regressioni di
turni/slot/costi/profili/difese/danni/effetti conservate. ASan/UBSan su tempo/
turni/slot/costi/profili/riduzioni, extra/condizioni/misti, danni/COS/vincoli/
difese, effetti/durate, magie/riposi/stato, allocazioni, core/parser, NVS e UI;
leak detector disattivato. 138 test JS e build PC passati.

Firmware v0.40: **903600 byte**, partizione app 4 MiB **78% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD20 legge PDD1–PDD19; USB v7/layout invariati.
Nessun flash/collaudo fisico, nuova lettura Beyond o modifica della raccolta
personale. Durate di altri soggetti, cariche oggetto, condizioni complete e
automatismi ancora successivi. [Uso e limiti](TEMPO-GIOCO.md).

## Verifiche v0.41 — cariche degli oggetti

Modello PC con pool opzionale per riga, configurazione e backup v11; copia USB8
con saldi esatti e parser rigoroso. Nel firmware: consultazione/correzione,
anteprima/conferma, Annulla, quantità indipendente, costi di riduzione atomici e
recuperi al riposo. Il pool non viene dedotto automaticamente da Beyond.
[Uso, contratti, compatibilità e limiti](CARICHE-OGGETTI.md).

144 test JavaScript e build Vite/HTML autonomo passati. Suite host completa e
41.411 confronti PC → USB → C, di cui 3.660 nuovi sulle cariche. 17.280 casi
indipendenti nel core cariche; 60 righe, saldi fino a 999, oggetti assenti,
recupero fino a 59.940 unità, riferimenti precisi, spesa a danno zero,
concentrazione/reazione/PF congiunti, clamp e rifiuti senza RNG/modifiche.
Tutti i 300 byte aggiunti sottoposti a corruzione con CRC valido; record massimo,
righe inutilizzate e significati vietati nei tag vecchi controllati.
Fixture PDD20 autentica dell'encoder v0.40 `6604bad` sintetica, migrazione senza
scritture al boot e riavvio dopo il salvataggio PDD21 con cariche conservate.

LVGL host: correzione, bozza scartata, conferma, errore/retry senza rilancio o
secondo addebito, Annulla, nome lungo, indice 59, quantità zero, costo esatto,
snapshot, riposi e protezione da tasti/import. Chromium isolato: form cariche,
validazione, salvataggio/reload/rimozione, altra scheda identica, viewport
320/390 px; anteprima/download USB senza mutazioni e importazione estesa
sintetica con schede precedenti conservate. Nessun errore JavaScript.

AddressSanitizer/UndefinedBehaviorSanitizer su core/parser, storage, cariche,
riposi, costi slot/profili, tempo e UI: passati, leak detector disattivato.
Build ESP32-S3 con ESP-IDF 5.5.5/LVGL 9.4.0: **906320 byte**, partizione app
4 MiB **78% libera**. Layout identico alla v0.40; PDD21 legge PDD1–PDD20,
USB8 legge v1–v7 senza cariche. L'uploader blocca v8 verso firmware precedente
prima di inviare il payload. Nessun flash/collaudo fisico, nuova lettura Beyond,
credenziale pubblicata o modifica di raccolte personali.

## Verifiche v0.42 — tre spese per profilo

Fino a tre fonti distinte più reazione, configurabili sul dispositivo. Se una
spesa è insufficiente o il colpo è bloccato, tutti i saldi restano invariati.
Conferma salva insieme ogni spesa, PF e concentrazione; confronto e ricevuta
per posizione, retry del candidato senza rilancio e Annulla dell'intero stato.
[Uso, compatibilità e limiti](COSTI-MULTIPLI.md).

48.000 casi C indipendenti su tutte le terne di tipi, tre profili, due fasi,
saldi insufficienti, reazione e danno. Controllati stati attesi completi,
rifiuto senza mutazioni/RNG, buchi, duplicati, quantità zero, riferimenti
estremi 119/59/8, costi massimi e candidato COS con spesa congiunta.
Record PDD22 massimo e tutti i 24 byte nuovi corrotti con CRC valido;
fixture PDD21 autentica dell'encoder v0.41 `048ef28`, senza dati personali.
Migrazione PDD1–21 senza scritture al boot; vecchia banca intatta e riavvio
con tre spese conservate dopo il primo salvataggio PDD22.

Suite host completa; 41.411 confronti PC → USB → C, 144 test JavaScript e build
Vite/HTML autonomo passati. Nessuna modifica al modello PC o al protocollo USB8.
LVGL host: tre spese, confronto, consultazione protetta, mancata disponibilità
secondaria, errore/retry senza doppio addebito o rilancio, ricevuta congelata,
Annulla, nomi lunghi, limiti nel cerchio, tasti/import protetti, import
annullato/confermato e azzeramento delle nuove configurazioni locali.
AddressSanitizer/UndefinedBehaviorSanitizer su core/parser, storage, spese
multiple, cariche, costi profili/slot, tempo, riposi e UI: passati con leak
detector disattivato. Anteprime generate dal firmware compilato per host.

Build ESP32-S3 **907872 byte**, partizione app 4 MiB **78% libera**;
ESP-IDF 5.5.5/LVGL 9.4.0. PDD22, USB8/backup PC v11/layout invariati.
Nessun flash, nuova lettura Beyond, modifica di raccolte personali o collaudo
fisico di display/touch/tasti/USB/batteria/memoria/alimentazione.


## Verifiche v0.43 — ricarica parziale delle cariche

Core `recharge.c`: piano esplicito 1–6 dadi d4/d6/d8/d10/d12/d20, bonus −15..+60,
recupero minimo zero e massimo limitato al saldo disponibile. Requisiti ed evento
restano manuali. Quantità, PF ed effetti invariati. Nessun RNG a saldo pieno,
oggetto assente, pool mancante o input invalido. [Uso e limiti](RICARICHE-OGGETTI.md).

103.680 casi C indipendenti: 60 righe, dadi/conteggi/capacità/saldi/bonus/facce;
candidati completi, rifiuti con tutti gli output intatti e persistenza PDD22.
Suite host completa, 41.411 confronti PC → USB → C e 144 test JavaScript passati;
build Vite/HTML autonomo riuscite. Nessuna modifica al modello PC o ai formati.
LVGL host: bozza/anteprima, annullamento, errore/retry senza rilancio, salvataggio
prima dell'animazione, recupero cap/pieno/zero/oggetto assente, nome lungo e riga
60, sei d20 +60, Annulla, tasti/import protetti e fallback senza canvas.
AddressSanitizer/UndefinedBehaviorSanitizer su ricariche e UI passati con leak
detector disattivato. Zero recuperato non scrive e conserva l'Undo precedente.

ESP32-S3: **910800 byte**, partizione app 4 MiB **78% libera**, ESP-IDF 5.5.5
e LVGL 9.4.0. PDD22/USB8/backup PC v11/layout invariati; nessuna nuova migrazione.
Nessun flash, nuova lettura Beyond, modifica di raccolte personali o collaudo
fisico di display/touch/tasti/USB/memoria/batteria/alimentazione.

La riprova del nuovo percorso è verificata con errore temporaneo del backend
host. Il backend NVS resta fail-closed: un errore I/O blocca nuove scritture
fino al riavvio; non si promette il recupero del tiro in RAM dopo spegnimento.
Al riavvio vale la banca valida più recente. Nessuna modifica a questa politica.


## Verifiche v0.44 — condizioni esplicite sui tiri

Accecato, Spaventato, Prono e Trattenuto hanno effetti locali separati dalle
annotazioni. Vista richiesta e fonte di paura in linea di vista sono fatti
dichiarati per il tiro. Si chiedono solo i dati influenti: un fallimento
automatico non genera dadi, uno svantaggio già certo non richiede anche la
fonte di paura. Esclusione DM per il singolo tiro, senza aggirare bonus non
verificati. Movimento, avversari, altre condizioni e durate restano manuali.
[Uso e contratti](CONDIZIONI-TIRI.md).

107.136 casi C con oracolo indipendente su tutte le combinazioni di quattro
effetti, tipi di tiro/caratteristiche, modi manuali, Avvelenato, Ira, esclusione
e contesti parziali. Controllati modi, output e RNG prima dei blocchi; note
separate, recuperi non dedotti e byte di persistenza. Tutti i 16 valori validi
e 240 invalidi con CRC valido, record massimo; migrazione PDD1–22 con fixture
PDD22 dell'encoder originale v0.43 `b340a8c`, vecchia banca intatta e riavvio.

Suite host completa, 41.411 confronti PC → USB → C, 144 test JavaScript e build
Vite/HTML autonomo passati. LVGL host: configurazione/confronto/errore/Annulla,
vista, fallimento senza dado, contesto rilevante, paura, cancellazione booleana,
attacchi, salvezze DES/COS, dado libero, tasti/import protetti e importazione
annullata/confermata. Sanitizzatori core/parser, NVS, condizioni, prove,
combattimento, concentrazione e UI passati con leak detector disattivato.

ESP32-S3: **915360 byte**, partizione app 4 MiB **78% libera**, ESP-IDF 5.5.5
e LVGL 9.4.0. PDD23, USB8/backup v11/layout invariati. Nessun flash, lettura
Beyond, modifica di raccolte personali o collaudo fisico di display, touch,
tasti, USB, memoria, batteria o alimentazione.


## Verifiche v0.45 — restyling RPG 2D

Diorama e cornice I4 in flash, avatar 72×90 con buffer esistente, cifre pixel,
pannelli con rilievo, icone di sezione, luci cosmetiche e vassoio intarsiato.
Nomi e descrizioni conservano il font leggibile. Nessun nuovo campo di stato,
RNG, scrittura o associazione dei tasti. PDD23/USB8/backup v11/layout invariati.
[Contratti e anteprime](RESTYLING-PIXEL-HD.md).

Suite host completa, 41411 confronti PC → USB → C, 144 test JavaScript e build
PC/HTML autonomo passati. Replica esatta 2×/3×/4× per 81 aspetti × quattro pose,
guardie e input invalidi; UI con timer/luci, attenuazione, nomi/valori massimi,
conferme/import/errori/tasti; dado singolo, doppio e pool fino a 18. ASan/UBSan
per avatar/dadi/pool/UI passati con leak detector disattivato.

ESP32-S3: **987840 byte**, partizione app 4 MiB **76% libera**, ESP-IDF 5.5.5 e
LVGL 9.4.0. Nessun flash o collaudo fisico, lettura Beyond o modifica della
raccolta personale. Memoria, fluidità e consumi sul dispositivo restano da misurare.


## Verifiche v0.46 — scena/avatar sulla base approvata

Maschere avatar e diorama ridisegnati, stesso layout e personalizzazioni della
v0.45 approvata. Alone I4 40×40 (864 byte) in flash, riusato dalle due lanterne
in Eroe/Aspetto; timer cosmetico esistente, pausa in attenuazione e pulizia
alla navigazione. [Confronto e contratti](AFFINAMENTO-SCENA-AVATAR.md).

Suite host completa, 41411 confronti PC → USB → C, 144 test JavaScript e build
PC/HTML autonomo passati. Avatar 81 aspetti × quattro pose con guardie e
replica 2×/3×/4×; UI con aloni/luci, input, conferme/import/errori e limiti
del cerchio. ASan/UBSan avatar/UI passati con leak detector disattivato.

ESP32-S3: **988800 byte**, partizione app 4 MiB **76% libera**, ESP-IDF 5.5.5 e
LVGL 9.4.0. PDD23/USB8/backup v11/layout e tasti configurabili invariati.
Nessun flash, collaudo fisico, lettura Beyond o modifica della raccolta personale.


## Verifiche v0.47 — pannelli, icone e feedback

Pannelli con bordi bronzo, icone di lista e messaggi transitori dei PF/risorse
applicati. Osservazione dei saldi prima/dopo sulle azioni coperte, senza
chiamate RNG/salvataggi aggiuntive. Scadenza a due secondi, ornamenti o modalità
statica, rimozione alla navigazione/importazione/attenuazione. Errori di memoria
con precedenza; pannello non cliccabile. [Uso e limiti](PANNELLI-FEEDBACK.md).

Suite host completa e 41411 confronti PC → USB → C; 144 test JavaScript, build
PC/HTML autonomo e ASan/UBSan UI passati (leak detector disattivato). Coperti
PF/temporanei, errore/no variazioni, risorse/slot/capacità, timer/movimento
statico/wrap/pulizia, liste con nomi lunghi e controlli nel cerchio.
ESP32-S3: **994112 byte**, app 4 MiB **76% libera**, ESP-IDF 5.5.5/LVGL 9.4.0.
PDD23/USB8/backup v11/layout e tasti invariati. Nessun flash, collaudo fisico,
lettura Beyond o modifica della raccolta personale.

## Incremento v0.51 — borsello

[Uso, schema e verifiche](BORSELLO.md). Cinque saldi distinti, zero verificato
diverso da dato assente, bozze locali senza scritture, anteprima prima/dopo,
conferma atomica e annullamento dell'ultima modifica alle monete.
Trasferimento PC USB9 e record PDD24; raccolta PC e tasti locali conservati.
Nessun cambio, acquisto o peso automatico. 91/91 viste native esercitate;
nessuna nuova lettura Beyond e nessuna prova fisica in questo incremento.

## Incremento v0.52 — storico dei tiri

[Uso, salvataggi, errori e verifiche](STORICO-TIRI.md). Ultimi 20 esiti,
11 tipi di tiro, dadi/bonus/modalità e note immutabili rispetto alle modifiche
successive della scheda. Record PDH1 piccoli in due banchi, separati dalla
scheda; nuova importazione con identità PDD25 e storico distinto. Errori del
registro riprovabili sullo stesso esito, senza RNG aggiuntivo. Storico è la
quattordicesima scelta dei tasti (incluso Spento). USB9, backup PC v11 e layout
invariati; 94/94 viste native esercitate. Scheda e registro non costituiscono
un'unica transazione resistente a ogni interruzione di alimentazione. Nessun
flash o collaudo fisico, lettura Beyond o modifica della raccolta personale.
