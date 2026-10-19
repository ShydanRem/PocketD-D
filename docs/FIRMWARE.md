# Firmware PocketD-D 0.16 — percorso offline e dadi animati

Il prodotto principale è il dispositivo. La parte PC serve per importare,
configurare, trasferire e conservare backup. La simulazione browser rimane un
banco di prova; non stabilisce la priorità delle funzioni del prodotto.

## Stato e perimetro

Firmware per **Waveshare ESP32-S3-Touch-AMOLED-1.75**, versione standard senza
suffisso C. Toolchain ESP-IDF **v5.5.5**, LVGL **9.4.0**. Nessuna dipendenza da
Foundry, server PC, Beyond, Wi-Fi o Bluetooth durante l'uso al tavolo.

La v0.16 aggiunge [d20 animato in pixel art](DADI-ANIMATI.md), lancio doppio,
risultato naturale sul dado, totale separato e accelerazione senza rilanci.

Primo percorso: avvio con Elian originale o copia precedentemente salvata →
Scheda → caratteristica → d20 → risultato → Eroe → PF ±1 → annulla →
salvataggio locale. Anche PF temporanei e fino a otto contatori manuali.
Vantaggio e svantaggio selezionabili dal pulsante Base/Vant/Svant. Durante il volo
si può accelerare; modalità e navigazione attendono l'arresto del tiro.

Non è ancora una scheda completa: mancano avatar, inventario, attacchi, magie,
abilità e salvezze con competenze, effetti/condizioni, morte, dadi vita,
monete, storico persistente, mappa e party. Le prove sono **solo di
caratteristica base**, senza competenza, bonus extra o effetti. Il dado libero
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

Il layout PocketD-D ha NVS a 0x9000 e app a 0x10000, dimensione app 4 MB.
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

Formato `pocketdd-device` v1, massimo 4096 byte, campi esatti: `format`, `version`,
`name`, `level`, `hp`, `maxHp`, `tempHp`, `armorClass`, `abilities` e `resources`.
Nome ASCII 1–48 byte, contatore 1–32; accenti traslitterati e altri simboli
sostituiti nell'anteprima PC. Livello 1–20, caratteristiche 1–30, CA 1–40,
PF 0–massimo (massimo 1–999), temporanei 0–999. Massimo otto contatori con
`name`, `current` 0–maximum, `maximum` 1–999. Priorità: Slancio solo per schede
originali/manuali, capacità con utilizzi, slot ordinari e slot patto. L'anteprima
nomina i contatori esclusi oltre il limite. Nessun valore numerico viene troncato
per farlo entrare. Nessun collegamento tra contatori con lo stesso nome.

Il firmware rifiuta chiavi sconosciute/duplicate, valori frazionari, Unicode
nel formato USB, strutture troppo annidate, payload troppo grandi e versioni
incompatibili. Prove sintetiche non equivalgono alla conversione del personaggio
reale Beyond: quella resta documentata separatamente in BEYOND-PROVE.md.

## Salvataggio e dadi

NVS nel namespace `pocketdd`, due chiavi alternate `state_a`/`state_b`.
Record binario esplicito di 384 byte, magic/versione `PDD1`, generazione uint32,
CRC32; niente dump di struct C con padding. La nuova copia è scritta, committata
e riletta prima di aggiornare lo stato visibile. Quella precedente rimane valida.
All'avvio si sceglie la generazione valida più recente, incluso wrap del contatore.
Una copia corrotta permette il recupero dall'altra; entrambe corrotte o versione
sconosciuta con CRC valido bloccano scritture. Nessun erase/reformat automatico.
Se un'operazione I/O fallisce, ulteriori scritture restano bloccate fino al riavvio;
un errore di commit può avere esito ambiguo e non va dichiarato annullamento certo.
La demo visualizzata con errore memoria non sostituisce dati salvati.

PF e contatori vengono persistiti ad ogni modifica; “Annulla modifica” ripristina
l'ultima modifica di sessione ai PF/contatori e la salva, non la vecchia scheda dopo
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
