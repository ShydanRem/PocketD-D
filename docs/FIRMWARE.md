# Firmware PocketD-D 0.66 — avatar e sfondi locali

Il prodotto principale è il dispositivo. La parte PC serve per importare,
configurare, trasferire e conservare backup. La simulazione browser rimane un
banco di prova; non stabilisce la priorità delle funzioni del prodotto.

[Novità v0.66: editor dell'avatar, tre sfondi, pulsanti e backup PDD29](PERSONALIZZAZIONE.md).

[Novità v0.65: quantità degli oggetti con tastiera e conferma](QUANTITA-OGGETTI.md).

[Scheda, acquisizione e ripristino dal browser via USB](USB-BROWSER.md).

[Installer USB da browser e pacchetto pronto, firmware v0.64](INSTALLAZIONE-USB.md).

[Novità v0.64: preparazione locale, conferma e risorse conservate](PREPARAZIONE-MAGIE.md).

[Novità v0.63: ricerca per nome nello zaino e nelle magie/capacità](RICERCA-LOCALE.md).

[Novità v0.62: telemetria batteria, pausa/risveglio e spegnimento](ALIMENTAZIONE.md).

[Novità v0.61: controllo pratico, cure/temporanei locali, importi e retry](USO-AL-TAVOLO.md).

[Novità v0.60: scelta esplicita di una spesa per colpo, PDD28 e backup](COSTI-ALTERNATIVI.md).

[Novità v0.59: editor dei nomi A/B/C, PDD27 e backup](NOMI-TURNI.md).

[Novità v0.58: durate per i riferimenti esterni A/B/C, PDD26 e compatibilità backup](TURNI-ESTERNI.md).

[Novità v0.57: vantaggio, svantaggio e bonus espliciti nel tiro morte](TIRI-MORTE.md).

[Novità v0.56: risoluzione esplicita di danni a zero PF e morte istantanea](DANNI-ZERO-PF.md).

[Novità v0.55: prova locale di tocco e tasti configurabili](COLLAUDO-COMANDI.md).
[Novità v0.54: primo avvio, diagnostica e sessione guidata](PRIMO-AVVIO.md).
[Novità v0.53: backup nativo, ripristino e garanzie](BACKUP-DISPOSITIVO.md).
[Storico v0.52: storico persistente, tasti e limiti](STORICO-TIRI.md).
[Borsello v0.51: monete, conferme e compatibilità](BORSELLO.md).
[Grafica v0.50: grafica e UX di tutte le viste, catalogo e verifiche](UI-COMPAGNO.md).
[Avatar illustrato v0.49: 81 aspetti e reazioni](AVATAR-ILLUSTRATO.md).
[Integrazione home, menu, PF e tasti v0.48](HOME-COMPAGNO.md).
PDD25 + registro NVS PDH2 (lettura PDH1), USB v9, backup PC v11; layout invariato e lettura dei salvataggi precedenti.

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
Il firmware tiene una sola scheda attiva. La copia nativa USB conserva i
progressi sul PC per il ripristino sul dispositivo; non sincronizza la raccolta PC.

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
sulla scheda insieme a heap e PSRAM; la v0.54 espone questi dati via USB.

La [v0.62](ALIMENTAZIONE.md) legge lo stato AXP2101 e integra indicatore/avvisi,
pausa con pannello spento e light sleep a intervalli a batteria, risveglio
protetto e Soft PWROFF esplicito dopo salvataggio. I registri di carica,
voltaggio, regolatori e parametri del gauge restano invariati: l'unica nuova
scrittura PMIC è il bit di arresto, su conferma. **Avvio a batteria, risveglio,
arresto, alimentazioni, orientamento touch e luminosità devono ancora essere
verificati sulla scheda reale.** Nessuna autonomia o calibrazione attestata.
Luminosità iniziale 50%, configurabile da Schermo. Attenuazione per inattività
e pausa automatica opzionale sono descritte nel documento alimentazione.
Audio, SD, RTC e IMU non vengono avviati. Nessun GPIO riutilizzato per
periferiche esterne.

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

## Diagnostica USB in sola lettura — v0.54

La [guida di primo avvio](PRIMO-AVVIO.md) distingue acquisizione, lettura dei
valori e collaudo fisico. `scripts/device-check.py --port PORTA` negozia prima
`PDD HELLO` → `PDD READY 9`, poi invia `PDD DIAGNOSTICS GET 1`. Il firmware
risponde con una sola riga `PDD DIAGNOSTICS {JSON}\n`, JSON massimo 2048 byte,
formato `pocketdd-diagnostics`, versione 1. Versioni/formati futuri, campi
sconosciuti/duplicati, tipi errati, contatori fuori limite e trasferimenti
incompleti vengono rifiutati. Un vecchio firmware riceve solo comandi testuali,
mai una scheda da importare. Una coda piena restituisce `PDD ERROR busy`.

Il task USB accoda una richiesta piccola; il task proprietario LVGL raccoglie
il rapporto senza modificare stato, pagina, conferme, timer di inattività,
tiro pendente o registro. Nessuna scrittura NVS, RNG, comando touch, reset,
controllo PMIC o lettura di dati personali. Il mutex di uscita mantiene integra
la riga e il framing dei backup. Un trasferimento nativo occupato va terminato
prima di aprire un secondo strumento sulla stessa porta.

La scheda espone versione app/SDK, destinazione compilata, uptime/reset,
heap interno/PSRAM, minimi stack UI/USB in byte, contatori touch/tasti, invii
al display, massimo handler LVGL e stati di disponibilità dei salvataggi.
I contatori PWR/BOOT sono fronti grezzi; le azioni configurabili continuano a
passare dal debounce e dai blocchi esistenti. Nessun campo contiene la scheda,
le facce dei dadi o identificativi univoci. La validità RAM non prova un commit
NVS; il contatore display non prova colori o fluidità, e non equivale a FPS.
Il rapporto non certifica l'esito delle prove hardware.

## Backup dal dispositivo al PC

La [procedura completa](BACKUP-DISPOSITIVO.md) acquisisce e ripristina uno stato
nativo, compresi effetti/difese locali, preferenze e storico. È distinta dalla
proiezione scheda USB9 sopra e dal backup PC v11. Richiede firmware e strumento
v0.53 o successivi, conferma sul display e `scripts/device-backup.py`. Nel PC,
**Backup del dispositivo** apre l'anteprima del file senza cambiare la raccolta.

```sh
mkdir device-backups
python3 scripts/device-backup.py read --port /dev/ttyACM0 --output device-backups/eroe-dispositivo.json
python3 scripts/device-backup.py restore device-backups/eroe-dispositivo.json --port /dev/ttyACM0
```

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

## Incremento v0.53 — backup e ripristino

[Formato, USB e verifiche](BACKUP-DISPOSITIVO.md). Backup nativo v1 con record
PDD25 e PDH1 canonici; nuovo registro fisico NVS PDH2, stesso contenuto con
recupero per identità. Vecchi registri PDH1 leggibili senza scritture al boot.
Acquisizione senza scritture NVS; file PC nuovo e completo, senza sovrascritture.
Ripristino dello storico nell'altro banco prima della scheda; errori incerti
impediscono altre scritture al registro fino al riavvio. Nuove conferme protette
e timeout, anteprima PC, raccolta conservata e 97/97 viste native esercitate.
Scheda PDD25, USB9, backup PC v11 e layout invariati. Nessun trasferimento USB
fisico, flash, lettura Beyond o modifica della raccolta personale.

## Incremento v0.54 — preparazione al collaudo fisico

[Primo avvio e sessione guidata](PRIMO-AVVIO.md),
[modello degli esiti ancora da compilare](COLLAUDO-HARDWARE.md).
Diagnostica USB v1 in sola lettura: memoria/stack, touch/tasti, handler LVGL,
reset e stato dei salvataggi, senza dati della scheda. Snapshot dal task UI,
nessuna risoluzione di conferme o tiri e nessuna scrittura NVS/RNG. Tasti
configurabili e grafica approvata conservati; formati PDD25/PDH2/USB9/backup e
partizioni invariati. La fixture USB9 originale per il collaudo non apre la
raccolta PC e non contiene materiale Beyond.

Verificati: 161 test JavaScript, suite firmware e 97/97 viste LVGL, codec
con buffer limite/contatori estremi, otto contesti UI senza effetti laterali,
12 test Python di trasporto/schema/file/CLI e fixture PC → parser C con
30 bonus confrontati. ASan/UBSan UI e codec, build PC/HTML autonomo ed
ESP32-S3 passati. Binario app 3.383.376 byte, partizione app 4 MiB 19% libera.
Misure dei driver, USB fisico, resa, precisione, memoria e alimentazione sono
ancora da acquisire sull'hardware in arrivo. Nessun flash, prova fisica,
lettura Beyond o modifica della raccolta personale in questo incremento.

## Incremento v0.55 — collaudo locale dei comandi

[Uso e garanzie](COLLAUDO-COMANDI.md). Da Aspetto → Tasti → Collaudo si
provano cinque bersagli touch ordinati e PWR breve/BOOT breve-lungo con le
scorciatoie attuali mostrate, senza eseguirle. Risultati solo in RAM, Azzera,
uscita e timeout assoluto di 120 secondi per vista, con assorbimento degli
input mantenuti fino al rilascio. Nessuna scrittura di scheda/registro,
estrazione RNG o modifica delle assegnazioni. Import/backup ricevono busy;
la diagnostica USB resta leggibile. PMIC e driver hardware invariati.

Passati: 161 test JavaScript, suite firmware completa, nuovo controller C
con soglia 799/800 ms e wrap, flusso puntatore LVGL con pressione/rilascio e
trascinamento, tre timeout con contatti mantenuti e ritorno alle scorciatoie.
100/100 viste esercitate, confronti di stato/preferenze/storico/copia pendente
e contatori NVS/RNG invariati. ASan/UBSan sul controller e UI, build PC/HTML
ed ESP32-S3 passati. Binario app 3.385.632 byte, partizione app 4 MiB 19% libera.
PDD25/PDH2/USB9, backup, diagnostica v1 e partizioni invariati. Nuove catture
`input-check-*.png` nel pacchetto, tutte native su host con periferiche sintetiche.
Nessun flash, collaudo fisico, lettura Beyond o modifica della raccolta personale.

## Incremento v0.56 — danni a zero PF e morte istantanea

[Regola rappresentata, percorso e limiti](DANNI-ZERO-PF.md). L'anteprima dei
danni apre una risoluzione separata per i casi prima bloccati: verifica Base
2014, dichiarazione esplicita del critico quando serve, PF/temporanei e
registro prima/dopo. Un colpo misto produce un solo evento del registro;
il critico non moltiplica l'importo già inserito. Il massimo è quello al
momento dell'impatto. Costi, reazione, effetti collegati e registro sono
salvati nella stessa candidata, con annullamento completo. Nessun RNG o
elemento aggiunto allo storico dei tiri. Eccezioni e cause persistenti
separate restano fuori da questo incremento.

Passati: 161 test JavaScript, suite firmware completa, 67.584 combinazioni
indipendenti del nuovo core e 101/101 viste LVGL host. Verificati critici,
temporanei anche a zero PF, stabilizzazione, soglie esatte, sei componenti,
tre costi, reazione, effetti collegati, codec PDD25, registri indisponibili
o incoerenti, scrittura fallita/nuovo tentativo, ricevuta e annullamento.
ASan/UBSan sul nuovo core e sulla UI, build PC/HTML autonomo ed ESP32-S3
passati. Binario app 3.388.624 byte, partizione app 4 MiB 19% libera.
PDD25/PDH2/USB9, backup, diagnostica v1 e partizioni invariati. Catture
`death-damage-*.png` native su host con dati sintetici, incluse nel pacchetto.
Grafica e tasti configurabili conservati. Nessun flash, collaudo fisico,
lettura Beyond o modifica della raccolta personale.

## Incremento v0.57 — circostanze e bonus nel tiro morte

[Uso, salvataggi e limiti](TIRI-MORTE.md). Modifica dal registro prepara
vantaggio/svantaggio separati e bonus da −30 a +30 per il singolo tiro,
con anteprima e conferma. Segni opposti si annullano; la faccia naturale
scelta conserva la priorità di 1/20 sul totale modificato. Il comando base
rimane un d20 senza bonus. Nessuna deduzione da testi, statistiche o condizioni.
La scheda viene salvata prima dell'animazione; un errore mantiene il tiro
fissato e blocca Indietro/modifiche fino al nuovo tentativo, anche nel flusso
base. Lo storico conserva dadi, bonus, totale e modalità effettiva con la
persistenza separata già presente e il blocco sui nuovi tiri pendenti.

Passati: 161 test JavaScript, suite firmware completa, 102.020 casi core
indipendenti e 102/102 viste LVGL host. Verificati tutte le coppie di d20,
segni opposti, bonus estremi e soglie, naturali speciali, registri bloccati,
compatibilità API base, codec PDH2, anteprima/annulla, scrittura fallita e
nuovo tentativo senza RNG aggiuntivo, storico pendente, ricevuta congelata,
annullamento completo e canvas assente. ASan/UBSan su core, stato e UI,
build PC/HTML autonomo ed ESP32-S3 passati. Binario app 3.389.920 byte
(partizione 4 MiB, 19% libera). PDD25/PDH2/USB9, backup,
diagnostica v1 e partizioni invariati. Le nuove catture `death-roll-*.png`
sono native su host con dati sintetici. Grafica e tasti conservati;
bonus a dado, reroll e altre eccezioni restano manuali. Nessun flash,
collaudo fisico, lettura Beyond o modifica della raccolta personale.

## Incremento v0.58 — turni esterni e PDD26

[Uso, formato e limiti](TURNI-ESTERNI.md). Le 50 durate di effetti numerici,
extra difensivi e profili possono riferirsi al proprio turno oppure ad A/B/C.
Gli editor dichiarano fase/soggetto e Altri turni apre selezione, anteprima,
dettagli e conferma. Solo l'inizio del proprio turno recupera la reazione.
Scadenze fuori limite rifiutano l'intero evento; nessun RNG o storico dei tiri.
Le associazioni A/B/C restano manuali, senza identità, nomi o iniziativa automatica.

Passati: 162 test JavaScript, suite firmware completa, 38.400 nuovi casi core
indipendenti e 103/103 viste LVGL host. Verificati i tre editor, fasi e soggetti
separati, pause, cap PF, rollback, conferma/annulla, nuovo tentativo dopo errore,
annullamento completo, eventi vuoti senza scrittura, 50 scadenze e ultima pagina.
ASan/UBSan su core e UI, build PC/HTML autonomo ed ESP32-S3 passati.
Binario app 3.391.872 byte (partizione 4 MiB, 19% libera).
Scritture PDD26/PDDQ con selettori compattati, dimensione record invariata e
lettura dei precedenti formati. Backup nativo v1, decoder PC e trasporto Python
accettano PDD25/PDD26; dieci prove Python del backup e codec/trasferimenti passati.
PDH2/lettura PDH1, USB9, backup PC v11, diagnostica v1 e partizioni invariati.
Nuove catture `external-turn-*.png` native su host con dati sintetici;
grafica e tasti conservati. Nessun flash, collaudo fisico, lettura Beyond
o modifica della raccolta personale.

## Incremento v0.59 — nomi locali dei riferimenti di turno

[Editor, formati e limiti](NOMI-TURNI.md). I riferimenti A/B/C hanno nomi locali
opzionali da 20 caratteri ASCII, modificabili sul dispositivo con tastiera a
sei pagine, anteprima precedente/nuovo e conteggio dei timer assegnati/attivi.
Conferma salva soltanto il nome, senza cambiare riferimenti, durate, PF,
reazione o risorse; nessun RNG o storico. Il nome compare nella selezione e
nell'anteprima del turno. Cancellazione, nessuna scrittura se invariato,
errore di salvataggio con bozza conservata e annullamento completo.

Passati: 165 test JavaScript, suite firmware completa, 28.160 nuovi casi core
indipendenti e 105/105 viste LVGL host. Codec PDD27 con 0/1/120 poteri,
migrazione PDD25/PDD26, nomi malformati con CRC valido, auto-copia e spazi,
tutte le pagine della tastiera, nomi lunghi a capo, preview/cancel, errore e
nuovo tentativo, cancellazione, salvataggio invariato, annullamento e ripristino
nel banco host. Backup reale prodotto dal C con nomi e 50 riferimenti letto
esattamente da PC e accettato dal validatore Python, senza aprire porte seriali;
11 test Python del trasporto backup e ASan/UBSan su core nomi, timer e UI
passati. Build PC/HTML autonomo ed ESP32-S3
passati. Binario app 3.394.576 byte (partizione 4 MiB, 19% libera).

Scritture PDD27/PDDR, fino a 75.182 byte, con lettura dei formati precedenti e
nomi inizialmente vuoti. Backup nativo v1, anteprima PC e strumento Python
aggiornati; USB9, PDH2/lettura PDH1, backup PC v11, diagnostica v1 e partizioni
invariati. Nuove catture `turn-name-*.png` native su host con dati sintetici.
Grafica e tasti conservati. Identità e associazioni restano manuali; niente
Unicode o iniziativa automatica. Nessun flash, collaudo fisico, lettura Beyond
o modifica della raccolta personale.

## Incremento v0.60 — costi alternativi dei profili

[Configurazione, applicazione e limiti](COSTI-ALTERNATIVI.md). Ogni profilo può
richiedere Tutte le spese oppure Una scelta esplicita per colpo, con almeno due
fonti configurate. Le altre fonti restano invariate anche se esaurite; nessun
pagamento sostitutivo automatico. La reazione è comune al profilo. Anteprima,
conferma, concentrazione, percorso morte e annullamento usano la stessa scelta;
le ricevute congelate distinguono le fonti non scelte. La modalità persiste,
la scelta del singolo colpo resta in RAM e viene azzerata per il nuovo impatto.

Passati: 167 test JavaScript, suite firmware completa, 25.920 nuovi casi core
indipendenti e 106/106 viste LVGL host. Verificati saldi e danni con stati attesi
completi, selezione obbligatoria, fonti indisponibili non scelte, buchi, input
malformati, reazione, danno zero, una/sei componenti, COS senza RNG sui blocchi,
danni a zero PF/morte istantanea, record massimo e migrazione PDD27.
UI: editor/anteprima, fonti disabilitate, errore di scrittura e retry,
annullamento, ricevute congelate, nomi lunghi e candidato COS con un solo tiro.
ASan/UBSan su core alternative e UI passati. Backup effettivamente prodotto
in C con 120 poteri sintetici e modalità [1,0,1] letto esattamente dal PC e
accettato dal validatore Python senza aprire porte seriali; 12 prove Python
backup passate. Build Vite/HTML autonomo ed ESP32-S3 passate.
Binario app 3.397.856 byte (partizione 4 MiB, 19% libera).

Scritture PDD28/PDDS: modalità e fase compattate nello stesso byte, dimensione
8942 + 552 × poteri, massimo 75.182 byte. Lettura dei precedenti formati con
modalità Tutte; bit riservati rifiutati. Backup nativo v1, anteprima PC e
trasporto Python aggiornati; USB9, PDH2/lettura PDH1, backup PC v11,
diagnostica v1 e partizioni invariati. Catture `alternative-*.png` e galleria
native LVGL host con dati inventati. Restano oltre tre fonti, gruppi di costi
alternativi, variabili/opzionali e requisiti automatici. Nessun flash,
collaudo fisico, lettura Beyond o modifica della raccolta personale.

## Incremento v0.61 — minimo pratico per la prima sessione

[Controllo dei percorsi, guida e limiti](USO-AL-TAVOLO.md). Il pannello PF
raggiungibile dalla home aggiunge cura di importo noto, temporanei e accessi
Danni/Stato. Cura e temporanei hanno tastiera numerica, anteprima, conferma,
salvataggio e annullamento completo. Cura limitata al massimo effettivo;
temporanei assegnati conservando il maggiore oppure corretti esplicitamente,
anche a zero, senza somma o cura dei PF. I comandi ±1 restano correzioni
manuali dichiarate. La tastiera è disponibile anche per l'importo della prima
e delle altre componenti del colpo, senza cambiare difese o risoluzione.

Il controllo ha trovato e corretto il rilancio della pozione dopo una scrittura
fallita e la possibilità di abbandonare il dado vita già tirato dopo un errore.
Candidato e risultato ora restano fissati; uscita e comandi in coda sono
bloccati e Riprova salva la stessa cura e lo stesso consumo. Animazione e
storico vengono presentati dopo il salvataggio; un errore dello storico usa
il percorso separato di nuovo tentativo già esistente. Il retry host non
certifica il recupero dopo un vero guasto I/O o perdita di alimentazione.

Passati: 167 test JavaScript, suite firmware completa, 6.720 nuovi casi core
indipendenti con confronto dell'intero stato, 108/108 viste LVGL host e harness
del compagno. Verificati limiti/cap e modificatori al massimo, temporanei
senza somma, registro morte, blocchi, tastiera/annullamento, nessuna scrittura
per modifiche invariate, retry/undo, import/tasti protetti, prima/altre componenti,
pozioni/dadi vita fissati, consumo unico, storico unico e fallback senza canvas.
ASan/UBSan su core PF/temporanei e UI passati. Build Vite/HTML autonomo ed
ESP32-S3 passate. Binario app 3.400.512 byte (partizione 4 MiB, 19% libera).

PDD28/PDDS, dimensione dei record, PDH2/lettura PDH1, USB9, backup nativo v1,
backup PC v11, diagnostica v1 e partizioni invariati. Grafica approvata e
assegnazioni dei tasti conservate. Catture `practical-*.png` e galleria native
LVGL host con dati sintetici. Nessuna lettura Beyond o modifica della raccolta
personale. Collaudi fisici ancora da eseguire; il prossimo traguardo pratico
è la sessione offline via USB, poi alimentazione autonoma e tenuta ai guasti.

## Incremento v0.62 — alimentazione, batteria e pausa

[Uso, fonti hardware e limiti](ALIMENTAZIONE.md). AXP2101 letto sul bus touch
con verifica dell'identità: batteria presente, USB valida, direzione di carica,
stima percentuale del gauge e tensione ADC già abilitato. Nessuna stima
inventata da una curva tensione/percentuale; campioni falliti, obsoleti o fuori
intervallo rimangono indisponibili. Indicatore nella home e pagina Alimentazione,
avvisi al 20% e 5%, senza taglio automatico dell'alimentazione.

Pausa manuale con pannello spento e RAM conservata; a batteria, light sleep
CPU a intervalli di 20 ms, touch/BOOT come wake GPIO e PWR controllato fra gli
intervalli. Con USB o dati sconosciuti resta la sola pausa display/rendering.
Ripresa protetta fino al rilascio del gesto, collegamento USB durante la pausa,
DMA drenato e ripristino della sorgente di entropia prima di altri tiri.
Pausa automatica dopo cinque minuti a batteria se Attenua è abilitata,
compreso il caso luminosità 10%; bloccata nelle operazioni protette e con
storico/memoria da risolvere. I campioni di tensione non ricreano la home o
riavviano l'animazione dell'avatar.

Spegnimento esplicito con batteria e USB scollegata: conferma, salvataggio di
scheda e storico, quindi nuova verifica PMIC e singolo comando Soft PWROFF.
Un salvataggio fallito impedisce il comando. Esito richiesto e non confermato
restano distinti; dopo il tentativo i comandi di gioco sono bloccati, senza
ripetere scritture. Nessuna modifica di ricarica, tensioni, regolatori, TS,
IRQ o parametri del gauge. PWR lungo resta hardware, tre tasti configurabili
conservati. Arresto, riaccensione e risparmio reali devono essere collaudati.

Passati: 167 test JavaScript e suite firmware completa. Nuovo core con
262.144 combinazioni indipendenti di stato/direzione/SOC, 16.384 valori ADC,
letture mancanti, gauge/ADC disabilitati, scadenza e wrap temporale, condizioni
di pausa e tutti i valori del registro di configurazione per l'unica scrittura
consentita. UI 110/110: dati sconosciuti/assenti/ricarica/bassi/obsoleti,
pausa/risveglio simulati, input mantenuti, USB, errori display/sleep, inattività,
nessuna modifica a scheda/storico/RNG, errori dei salvataggi e richiesta di
arresto unica. ASan/UBSan su core alimentazione/schermo e UI passati.
Build PC/HTML autonomo ed ESP32-S3 passate. Binario app 3.416.128 byte
(partizione 4 MiB, 19% libera).

PDD28/PDDS, PDH2/lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Nessun nuovo campo persistente.
Catture `power-*.png` e galleria LVGL host con dati inventati. Gli adapter
PMIC/sleep/pannello del test UI sono simulati; la build reale usa le API
ESP-IDF e il driver del pannello. Nessun flash, misura di autonomia o collaudo
fisico eseguiti, nessuna lettura Beyond o modifica della raccolta personale.

## Incremento v0.63 — ricerca per nome negli elenchi

[Uso, identità delle voci e limiti](RICERCA-LOCALE.md). Zaino e magie/capacità
hanno ricerca locale per una parte del nome, senza distinzione maiuscole/
minuscole, con query fino a 20 caratteri, tastiera in quattro pagine e conteggio
live. I filtri Magie/Capacità/Tutte si combinano con la query. Liste paginate,
zero risultati, nomi lunghi e duplicati mantengono l'indice originale per
dettagli, equipaggiamento e consumi. Due ricerche separate in RAM, nessun
salvataggio/RNG/storico o sostituzione dell'annullamento durante la ricerca.
Annullare conserva filtro/pagina precedenti; Mostra applica dalla prima pagina.
Pausa conserva i filtri, import/ripristino li azzerano solo su conferma.

Passati: 167 test JavaScript, suite firmware completa, 11.506 nuovi casi core
su sottostringhe/case/indici, dati mancanti, duplicati/voci esaurite, categorie,
limiti e normalizzazione, con 60 oggetti e 120 poteri. UI 112/112: entrambe
le tastiere, quattro pagine, trim/clear/cancel/limite, testo completo a capo,
nessun risultato, memoria non scrivibile senza nuove scritture, filtri incrociati,
selezione e consumo della pozione originale 59 e della capacità originale 119,
pausa e import/ripristino confermati/annullati. ASan/UBSan su core catalogo e
UI passati. Build PC/HTML autonomo ed ESP32-S3 passate.
Binario app 3.418.352 byte (partizione 4 MiB, 19% libera).

PDD28/PDDS, PDH2/lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Nessuna preferenza o query persistente.
Grafica, alimentazione e tasti configurabili conservati. Catture `search-*.png`
e galleria native LVGL su host con dati inventati. Nessuna lettura Beyond,
modifica della raccolta personale o prova fisica. Ricerca nelle descrizioni,
preferiti e ordinamento restano affinamenti successivi, insieme al collaudo
sul display reale.

## Incremento v0.64 — preparazione locale delle magie

[Uso e limiti di preparazione/disponibilità](PREPARAZIONE-MAGIE.md).
Il dettaglio di una magia offre Gestisci, con preparazione e correzione degli
utilizzi innati. La preparazione richiede anteprima e conferma e modifica
soltanto PD_PREPARED: nessun recupero di slot/usi, lancio, RNG o tiro nello
storico. Concentrazione attiva, effetti, difese e riduzioni collegati restano
invariati anche togliendo la preparazione. I requisiti di lancio già presenti
restano applicati. Capacità escluse, trucchetti e innate gestiti come
scelta esplicita di disponibilità; limiti di classe, lista conosciuta e momento
consentito restano manuali con il DM.

Salvataggio fallito conserva stato e scelta da riprovare; riuscito rende
annullabile la modifica di gioco. Tasti, import/ripristino e pausa automatica
non interrompono la conferma. Ricerca e indice originale sono conservati.
Backup e record includono già il flag; import/ripristino confermati adottano
la selezione della copia. Nessuna modifica della raccolta PC.

Passati: 167 test JavaScript, suite firmware completa e 3.840 nuovi casi core
su tutti i 120 indici, quattro tipi di costo, prima/dopo e vincoli attivi.
Il confronto dell'intera scheda prova che cambi solo il flag; coperti anche
idempotenza, rifiuti atomici, lancio, riposi, record massimo e backup nativo.
UI 114/114: anteprima/annulla, salvataggio fallito/riprova/Annulla, input protetti,
lancio preparato, slot ordinari/patto e utilizzi conservati, concentrazione e
legami mantenuti, innati/trucchetti, capacità escluse, nome lungo e voce 119
filtrata, import/ripristino confermati/annullati. ASan/UBSan su preparazione
e UI passati. Build PC/HTML autonomo ed ESP32-S3 passate.
Binario app 3.419.664 byte (partizione 4 MiB, 18% libera).

PDD28/PDDS, PDH2/lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Nessun campo persistente aggiunto.
Catture `prepare-*.png` e galleria LVGL host con dati inventati. Nessuna nuova
lettura Beyond, pubblicazione di dati personali o prova hardware. Preparazione
automatica per classe, apprendimento di nuove magie e regole speciali restano
successivi; la prova fisica H31 è ancora da eseguire.

## Distribuzione del firmware v0.64 — installer USB

Aggiunto installer statico Chrome/Edge, esptool-js 0.6.0 e crypto-js 4.2.0
con dipendenze fissate nel lockfile e licenze incluse. Tre immagini separate,
SHA-256 prima della connessione, controllo ESP32-S3/16 MB, confronto del layout
per aggiornamento, nessun eraseAll, MD5 per regione e rilascio della porta.
Riavvio riuscito e firmware verificato con reset manuale necessario sono distinti.
Pagina e ZIP ridotto funzionano anche tramite server locale, senza CDN.

Passati 173 test JavaScript e controlli Chromium dell'installer compilato:
caricamento/integrità, annullamento selettore, file corrotto/riprova, browser
non supportato e layout mobile. I test di scrittura sono simulazioni, nessuna
scheda fisica collegata. Il binario ESP32-S3 v0.64 da 3.419.664 byte è invariato,
come partizioni e formati; restano validi i controlli firmware sopra. H32 ancora
Non eseguito. [Uso, distribuzione e limiti](INSTALLAZIONE-USB.md).

## Supporto PC v0.64 — scheda e backup via Web Serial

Il PC usa ora i protocolli esistenti di copia USB9 e backup nativo v1 direttamente
dal browser. Anteprima congelata, negoziazione prima dell'invio, conferma sul
display e ACK finale obbligatorio; timeout/interruzione non diventano successo.
Acquisizione con lunghezza/CRC e record verificati, download dei byte originali,
ripristino sottoposto alla validazione semantica del dispositivo. Nessuna
modifica della raccolta PC o lettura remota.

Passati 182 test JavaScript e controlli Chromium sul codice compilato con
Web Serial simulata, inclusa la conservazione della raccolta sintetica.
Verificato anche il precedente flusso file/anteprima. Il firmware ESP32-S3,
formati, partizioni e tasti restano invariati; il binario è quello v0.64 già
compilato e verificato su host, senza nuova compilazione necessaria. Nessun
trasferimento fisico eseguito. [Uso, interruzioni e limiti](USB-BROWSER.md).

## Incremento v0.65 — quantità degli oggetti sul dispositivo

Il saldo 0–999 di una riga esistente si corregge dallo Zaino con tastiera,
anteprima e conferma. Nessun uso, cura, ricarica di cariche o effetto dedotto.
Zero sull'arma scelta ripone l'arma e azzera il lancio nello stesso salvataggio;
una quantità nuovamente positiva richiede una nuova scelta. Tasti/import/
ripristino/pausa sono protetti durante le bozze. Filtri e indici originali
restano validi. Salvataggio fallito conserva stato e scelta, Annulla ripristina
la modifica riuscita; le righe azzerate restano consultabili.

Passati 182 test JavaScript, suite firmware completa e 40.320 nuovi casi core
con confronto dell'intera scheda, 60 indici, quattro tipi, saldi, equipaggiamento
e cariche, idempotenza, fuori limite, rifiuti e codec nativi. UI 115/115: tastiera,
annullamento, conferma invariata senza scritture, retry/Annulla, cariche/PF/
concentrazione conservati, arma zero/ritorno, nome lungo/indice filtrato 59,
backup e import/ripristino. ASan/UBSan su quantità e UI passati. Build PC/HTML
ed ESP32-S3 passate. Binario app 3.420.784 byte, partizione 4 MiB, 18% libera.

PDD28/PDDS, PDH2/lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Catture quantity e galleria LVGL host
con dati inventati. Nessuna lettura Beyond, modifica della raccolta personale
o prova hardware. Creazione/rinomina delle voci, prezzi, peso e automatismi
restano successivi. [Uso, salvataggi e limiti](QUANTITA-OGGETTI.md).
