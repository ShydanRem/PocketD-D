# SD come archivio principale degli asset

La microSD è la destinazione prevista per cataloghi grafici, sfondi e preset di
classe. La capienza della partizione applicazione non deve limitare la selezione
artistica: la flash conserva programma e risorse essenziali di ripiego. Restano
da misurare RAM, lettura e decodifica degli asset effettivamente usati.

## Stato reale e riferimento hardware

Nella v0.69 gli asset sono ancora incorporati nel firmware. `pd_board_init()`
inizializza display, touch, input e alimentazione; non monta una SD e non esiste
ancora un caricatore di pacchetti grafici esterni. La scheda non è quindi già
utilizzabile come destinazione degli asset semplicemente copiandovi immagini.

È stato riletto il BSP della scheda **ESP32-S3-Touch-AMOLED-1.75 standard**, al
commit `e4344e70c2fa78a13e8a06566507f1ba8af6672a` del
[repository ufficiale](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/tree/e4344e70c2fa78a13e8a06566507f1ba8af6672a).
Le fonti sono
`firmware/brookesia/components/waveshare__esp32_s3_touch_amoled_1_75/include/bsp/esp32_s3_touch_amoled_1_75.h`
e `esp32_s3_touch_amoled_1_75.c` nella stessa cartella del componente.

| Voce | BSP verificato nel sorgente |
| --- | --- |
| Interfaccia | SDMMC a 1 bit, FAT tramite `esp_vfs_fat_sdmmc_mount` |
| CLK | GPIO2 |
| CMD | GPIO1 |
| D0 | GPIO3 |
| D1–D7 | Non collegati nella configurazione del BSP |
| Card detect / write protect | Non configurati dal BSP |
| Formattazione all'errore | Opzione del BSP; PocketD-D dovrà mantenerla disattivata |

Questi pin sono distinti dal QSPI del display (38/4–7/12), dall'I2C del touch
(14/15) e dall'USB nativo (19/20) usati dal firmware. La verifica del sorgente
non è una prova fisica di montaggio, alimentazione, compatibilità della carta o
lettura simultanea con display e touch. Capacità supportate e FAT32/exFAT non
sono ancora collaudati in PocketD-D.

## Distribuzione delle risorse

| Supporto | Ruolo previsto |
| --- | --- |
| SD | Pacchetti di razze, corpi, classi, abiti, armature, armi, animazioni e sfondi |
| Flash applicazione | Programma, interfaccia essenziale, avatar/scena di ripiego e lettore dei pacchetti |
| PSRAM/RAM | Strati del personaggio attivo, fotogrammi imminenti e cache limitata |
| NVS | Scheda e progressi già salvati, preferenze e riferimenti alle scelte estetiche |

Non caricare tutta la SD in RAM. Preparare gli asset sul PC in un formato adatto
al renderer, caricare prima di mostrare una scena e riusare i dati durante
l'animazione. Evitare aperture e letture per ogni fotogramma. La compressione
senza perdita rimane utile per tempi di lettura e cache; non deve imporre una
riduzione della varietà dei preset per farli entrare tutti nella flash.

La modularità di testa/corpo/abiti/accessori resta utile per coerenza artistica,
animazioni e combinazioni; non è un obbligo derivato dalla sola capacità della
flash. Eventuali sprite dedicati di qualità migliore possono stare nei pacchetti.

## Flusso da implementare

1. **Montaggio non distruttivo.** Portare la configurazione SDMMC del BSP nel
   proprietario dei bus della scheda, senza formattazione automatica. Verificare
   mount/unmount, assenza della carta ed errori di lettura sull'hardware.
2. **Pacchetto grafico verificabile.** Definire un manifesto con versione del
   formato, compatibilità col renderer, dimensioni e impronte dei file. I nomi
   dei pacchetti non devono permettere accesso fuori dalla propria cartella.
3. **Caricamento e cache.** Leggere soltanto gli asset necessari, verificare
   dimensioni e integrità prima di renderli attivi, poi passare i dati al task
   che possiede LVGL. Mostrare la nuova scelta quando è pronta.
4. **Selezione persistente.** Conservare identificativo di pacchetto e preset
   separati dai valori di gioco. Se manca il pacchetto, mostrare il ripiego senza
   cancellare la scelta, cambiare inventario o riscrivere la scheda personale.
5. **Preparazione pratica da PC.** Esportare una cartella di pacchetto da copiare
   nella SD con un lettore. Il trasferimento della scheda via USB rimane
   indipendente; aggiornare un pacchetto non deve richiedere riflashare il firmware.
6. **Collaudo.** Misurare caricamento iniziale, cambio di avatar/scena, fluidità,
   RAM e comportamento senza SD o con file mancanti/corrotti. Gestire anche una
   rimozione inattesa senza interrompere l'uso della scheda.

I pacchetti installati non devono sovrascrivere file personali. I backup su SD
possono essere un flusso aggiuntivo esplicito; non sostituiscono automaticamente
il salvataggio NVS o l'attuale backup USB. Non è prevista alcuna formattazione,
scrittura sulla SD o migrazione dei personaggi in questa modifica documentale.

## Preset delle classi

Il caricatore SD precede l'integrazione dei dodici look della
[tavola di classe](PRESET-CLASSI.md). Il primo pacchetto deve rendere distinguibili
vestiti, armature e armi, conservando razza e silhouette. Si mantiene un look
incorporato per l'uso senza SD. Il preset rimane cosmetico e non assegna oggetti,
CA o capacità automaticamente.
