# Asset esterni sulla SD — v0.83

Il caricatore SD ospita i [504 look delle classi](PRESET-CLASSI.md). Sfondi e
grafica originale restano incorporati; NVS conserva PG, progressi e preferenze.

## Uso

Il percorso consigliato è [Installa look sulla SD via USB](SD-WEB-USB.md),
dall’installer o dall’app PC, con SD FAT32 nel Pocket e firmware 0.83.
Il browser prepara il pacchetto e il dispositivo lo scrive/verifica, senza lettore.

Alternativa: scaricare `PocketDD-SD.zip` ed estrarre `pocketdd` nella radice della
SD FAT32 con un lettore PC. Percorso `/pocketdd/classes.pdc`, senza cartelle
intermedie. Inserire la scheda a dispositivo spento; scegliere il look in
Aspetto → Modifica → Classe. `Rileggi` rimonta e invalida la cache.

PocketD-D non formatta la SD. La nuova scrittura USB riguarda soltanto il catalogo
look e i suoi file temporaneo/recupero; PG, NVS e file personali restano separati.
Senza asset viene mostrato l’originale, conservando preset, specie e silhouette.

## Hardware e limiti

Montaggio `esp_vfs_fat_sdmmc_mount`, SDMMC a un bit: CLK GPIO2, CMD GPIO1,
D0 GPIO3; D1–D7, card detect e write protect non usati. Configurazione dal BSP
ufficiale della ESP32-S3-Touch-AMOLED-1.75 standard, commit
[e4344e70c2fa78a13e8a06566507f1ba8af6672a](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/tree/e4344e70c2fa78a13e8a06566507f1ba8af6672a).

Clock predefinito 20 MHz, due file massimi, formattazione all'errore disattivata.
Mount fallito non interrompe l'avvio. Il task UI possiede caricatore e LVGL;
nessun accesso concorrente dal task USB. Senza card detect, dopo sostituzione
serve `Rileggi`. Lo sprite verificato in cache può restare visibile dopo una
rimozione, fino al cambio di look o alla rilettura.

**Compilato e verificato su host, non ancora collaudato con SD sull'hardware.**
Capacità, FAT32, tempi, consumi e rimozione/reinserimento sono da misurare;
exFAT non fa parte del percorso verificato.

## Pacchetto PDCL v1

Percorso fisso `/sdcard/pocketdd/classes.pdc`; nessun percorso da Beyond,
backup o manifesto viene aperto dal firmware. Tutti i numeri sono little endian.

| Offset | Campo |
| --- | --- |
| 0 | magic `PDCL` |
| 4 | schema u16 = 1 |
| 6, 7 | larghezza/altezza u8 = 64 |
| 8 | specie u16 = 21 |
| 10, 11 | corpi u8 = 2, classi u8 = 12 |
| 12 | byte/frame u32 = 12288 |
| 16 | frame u32 = 504 |
| 20 | byte totali u32 = 6195200 |
| 24 | CRC32 della tabella u32 |
| 28 | CRC32 dei primi 28 byte u32 |
| 32 | tabella: 504 CRC32 dei frame |
| 2048 | 504 frame RGB565 + piano alpha A8 |

Indice `(specie*2+corpo)*12+classe-1`; ID in `src/avatar-classes.json`.
Originale=0 rimane nel firmware. Header, limiti, tabella e lunghezza esatta
sono verificati prima del frame; CRC del frame prima di modificare il buffer.
CRC rileva errori accidentali, non autentica l'autore. Il manifesto contiene SHA256.

Cache fissa di un frame, 12288 byte; buffer visibile massimo 49152 byte in
PSRAM, 128×128 RGB565A8. Lettura solo al cambio di combinazione o a `Rileggi`:
nessun file aperto, allocazione o accesso al disco per ogni fotogramma.
Anche gli errori sono memorizzati fino a cambio/rilettura.

Build normali: pacchetto già generato. `generate-class-art.py` verifica integrità
e sorgenti; `--export` richiede Pillow/numpy e normalizza le tavole ImageGen,
senza ridisegnarle. Prove C con file host: assenza, corruzione, troncamento,
cache e ripiego; non attestano il controller SD fisico.
