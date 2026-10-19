# Architettura proposta e verifiche

## Hardware

Prodotto indicato dall'utente: [Waveshare ESP32-S3-Touch-AMOLED-1.75](https://www.waveshare.com/esp32-s3-touch-amoled-1.75.htm), variante senza case e GPS.

Specifiche dalla descrizione del produttore condivisa dall'utente:

| Componente | Specifica |
| --- | --- |
| MCU | ESP32-S3R8, dual core LX7 fino a 240 MHz |
| Memoria | 512 KB SRAM, 8 MB PSRAM, 16 MB flash esterna |
| Display | AMOLED 1,75 pollici, 466 × 466, QSPI, controller CO5300 |
| Touch | Capacitivo, CST9217 su I2C |
| Archiviazione | Slot TF/microSD integrato |
| Rete | Wi-Fi 2,4 GHz e Bluetooth LE |
| Movimento | QMI8658, accelerometro e giroscopio |
| Ora | RTC PCF85063 |
| Alimentazione | AXP2101, connettore batteria al litio 3,7 V MX1.25 |
| Input e USB | Pulsanti PWR/BOOT programmabili e connettore Type-C |
| Audio | Doppio microfono, codec a basso consumo e circuito di cancellazione eco |

Il 15 ottobre 2026 la pagina prodotto e la pagina wiki hanno risposto HTTP 403 dall'ambiente cloud. Le specifiche sopra provengono dal testo fornito dall'utente; non sono ancora state confrontate con schemi o provate sull'hardware.

Gli 8 MB di PSRAM rendono plausibile mantenere buffer grafici e una cache di sprite, ma non garantiscono frame rate o compatibilità DMA. I 16 MB di flash possono ospitare firmware e risorse essenziali; la SD ospiterà cataloghi più ampi e backup. Partizionamento e dimensioni effettive saranno decisi dopo la prima build.

Prima di scrivere firmware verificare:

- Revisione della scheda, pin e configurazione PSRAM/flash negli esempi ufficiali.
- Alimentazioni e sequenza di inizializzazione CO5300/CST9217.
- Bus, pin, capacità e filesystem supportati per la microSD.
- API AXP2101 per ricarica, misura della batteria e sospensione.
- Versioni compatibili degli esempi ufficiali, framework e driver.
- Modalità di flash USB, recupero e debug seriale.

Microfoni, RTC e IMU non servono al primo prototipo. Un gesto con l'IMU per tirare i dadi è una possibile funzione successiva, sempre affiancata dal comando touch. La descrizione non conferma altoparlante o uscita audio utilizzabile: non prevedere effetti sonori senza una verifica dedicata.

## Componenti

**Firmware:** interfaccia, input, dadi, stato della scheda, salvataggio e caricamento degli asset. Funzionamento offline per il gioco individuale.

**App web di supporto:** modifica comoda della scheda, editor avatar, backup e trasferimento. Nel primo prototipo può limitarsi alla simulazione del display. Trasporto verso l'hardware da scegliere dopo le prove USB/Wi-Fi; non assumere che un browser possa accedere direttamente a qualunque periferica.

**Importazione Beyond:** esperienza preferita dal dispositivo; importazione dall'app PocketD-D su PC e trasferimento accettati come alternativa. Verificare inizialmente lettura e normalizzazione sul PC, mantenendo il formato locale indipendente dal trasporto. Valutare separatamente accesso autonomo dal firmware e importazione comandata dal dispositivo attraverso un backend nostro: la seconda dipende dalla disponibilità del servizio. Nessuna delle due modalità è ancora implementata o verificata sull'hardware. Dettagli in [BEYOND-FATTIBILITA.md](BEYOND-FATTIBILITA.md).

**Formato dati condiviso:** versione dello schema, identità locale, versione del regolamento, valori base, risorse correnti, inventario e riferimenti agli asset. Separare definizione del personaggio e stato della sessione. Un importatore esterno converte in questo formato senza imporre dipendenze al firmware.

**Party futuro:** eventi identificabili e versione dello stato, dispositivo coordinatore per iniziativa e griglia, riconnessione e riconciliazione esplicite. Non trasmettere continuamente framebuffer o interi cataloghi di asset.

## Stack candidato

- Firmware C/C++ con ESP-IDF e LVGL, se compatibili con gli esempi ufficiali; valutare Arduino se offre un percorso più affidabile per questa scheda.
- Simulazione e app di supporto con TypeScript e Vite; Canvas 2D è sufficiente per provare pixel art e layout circolare.
- Asset preparati sul computer e distribuiti in formati semplici, con manifesti e versioni. Non richiedere decodifiche costose durante ogni frame.

Sono proposte, non scelte già validate. Non installare due toolchain concorrenti prima di verificare il percorso hardware supportato. Il codice della simulazione non sarà automaticamente riutilizzabile come UI embedded; condividere invece asset, formato dati e casi di verifica.

## Memoria e prestazioni

La SD amplia l'archiviazione, non la RAM o la velocità di rendering. Un framebuffer completo RGB565 richiede 466 × 466 × 2 = **434.312 byte**, circa 424 KiB; due richiedono circa 848 KiB, prima di UI, asset, rete e stack.

Valutare buffer parziali, aggiornamenti delle sole regioni modificate e cache degli sprite. La disponibilità di PSRAM non garantisce che ogni buffer possa essere usato direttamente dal DMA: verificare i vincoli dei driver.

Puntare inizialmente a un'animazione breve e fluida a 30 fps nelle schermate semplici, come obiettivo da misurare. Nessuna promessa di frame rate prima della prova con display, SD e touch contemporaneamente attivi.

## Stato e affidabilità

- Utilizzare il generatore casuale appropriato di ESP-IDF secondo i suoi requisiti di entropia; non usare il frame dell'animazione come sorgente casuale.
- Salvare cambiamenti significativi, evitando scritture per ogni frame.
- Progettare salvataggi recuperabili dopo interruzione dell'alimentazione, con versione e copia valida precedente; provare il comportamento sul filesystem scelto.
- Gestire SD assente, piena o corrotta con messaggi comprensibili e senza sovrascrivere silenziosamente dati validi.
- Non conservare password Beyond sul dispositivo. Definire il flusso di autorizzazione solo dopo aver verificato un metodo supportato.
- Non esporre comandi di modifica della scheda su rete senza associazione e autorizzazione.

## Validazione progressiva

Nel cloud: simulazione, build dell'app, test significativi di modificatori, limiti PF e serializzazione quando tali moduli esisteranno.

Sul dispositivo: compilazione e flash, input ai bordi del cerchio, leggibilità, tempi di caricamento SD, fluidità, salvataggio/riavvio e comportamento in assenza della SD. Autonomia misurata sull'alimentazione reale.

Il cloud da solo non può validare display, consumo o touch senza hardware collegato e accessibile. Il primo prototipo web è stato compilato con Vite. Nella prima versione sono passati quattro test sul modello (limiti PF, vantaggio/svantaggio, estrazione uniforme d20, ripristino dati) e una verifica browser con Chromium su desktop e viewport mobile: PF, annullamento, persistenza condizione, selezione caratteristica e tiro con vantaggio. Nessun errore JavaScript rilevato. La verifica hardware resta da eseguire.


Incremento 0.3: editor manuale del profilo e avatar a palette, backup v3 con migrazione v1/v2, modificatori e attacco derivati dalle caratteristiche. Lo schema sperimentale è documentato in [FORMATO-SCHEDA.md](FORMATO-SCHEDA.md). Il prototipo usa attualmente JavaScript con moduli ES e Vite; TypeScript resta una possibile evoluzione.

Incremento 0.5: `src/powers.js` contiene il modello delle risorse manuali separato dalla UI: validazione, uso, modifica, eliminazione e riposi. Backup v5 con migrazione dei precedenti formati, nessun contenuto Beyond o catalogo incorporato. L'interfaccia distingue gestione nell'app di supporto e consumo nella vista circolare. Nessuna di queste verifiche dimostra ancora prestazioni o compatibilità del firmware ESP32.


Incremento 0.8: `src/grid.js` gestisce percorsi, finestre di visualizzazione e geometria delle linee, indipendentemente dal DOM. La vista controlla l'attacco usando gli stessi dati globali anche con zoom attivo. Nessuna acquisizione di dati dal sensore touch fisico o comunicazione ESP32 è ancora implementata.

Incremento 0.9: `src/library.js` contiene il modello della raccolta, separato dalla scheda e dall'interfaccia. Il formato individuale v8 viene riutilizzato senza modificarlo. Ogni personaggio conserva una sessione locale completa; non c'è ancora stato condiviso tra giocatori. La persistenza migra la chiave precedente senza cancellarla e impedisce il fallback distruttivo da dati corrotti. Gli errori di scrittura mantengono esportabile la raccolta in memoria; le schede browser già non aggiornate sono rifiutate prima della scrittura. `src/library.test.js` verifica isolamento, migrazione, validazione e fallimenti; `scripts/check-library.py` verifica i flussi dell'interfaccia e del salvataggio nell'HTML autonomo.


Incremento 0.10: `src/inventory.js` contiene validazione, equipaggiamento, quantità e modifiche dell'inventario; `src/inventory-ui.js` gestisce l'editor dell'app di supporto. `src/model.js` calcola attacchi, danni e cure sulla configurazione manuale e migra le schede v1–v8 nel formato v9. La geometria usa la portata dell'arma equipaggiata. Il formato raccolta e la chiave persistente non cambiano. `src/inventory.test.js` verifica migrazioni, isolamento, consumi, critici, riferimenti ed errori; `scripts/check-inventory.py` esercita la UI con estrazioni deterministiche dei dadi. Nessuna dipendenza o toolchain firmware aggiunta.

Incremento 0.11: `src/avatar.js` rende l'avatar SVG originale e definisce palette/preset basati sugli indici esistenti. `src/interface.css` contiene il livello di presentazione rivisto sopra lo stile storico. Navigazione, focus, gesti e memoria dello scorrimento vivono nella UI; non scrivono le preferenze nei backup. I menu non modificano il modello e sono bloccati durante un lancio asincrono. La memoria delle viste viene svuotata al cambio personaggio. `scripts/check-interface.py` verifica anche la geometria: punti dei pulsanti raggiungibili dentro la maschera circolare, controlli separati dal menu, pulsanti esterni di almeno 44 px e assenza di overflow sui viewport mobili. Le misure sono verifiche browser, non misure di leggibilità o precisione sul display fisico.


Incremento 0.12: `src/party.js` contiene modello, validazione e riduttore autorevole dei comandi. `src/party-transport.js` isola il collegamento `BroadcastChannel`, le conferme, i tentativi e i checkpoint in `sessionStorage`. `src/party-ui.js` e `src/party.css` forniscono il pannello dell'app di supporto. Incontro e griglia riusano i modelli esistenti con partecipanti espliciti; nessuna modifica automatica ai dati delle schede personali. Sequenze per client, revisioni e ricevute persistenti impediscono ritrasmissioni duplicate durante il normale recupero. Il protocollo completo e i limiti di questa simulazione sono in [PARTY.md](PARTY.md). Il trasporto reale e l'autenticazione dei dispositivi non sono ancora implementati.

Incremento 0.13: `scripts/beyond-client.js` mantiene il trasporto HTTPS limitato alla destinazione Beyond prevista; `src/beyond-conversion.js` produce il resoconto v2 e una bozza locale solo per regole riconosciute. `scripts/app-server.js` riusa i controlli host/origine e serve build e API su loopback, senza archiviare schede. `src/beyond-ui.js` gestisce anteprima, invalidazione su cambio input e conferma; `src/main.js` valida e salva la raccolta prima di attivare la nuova scheda. Il modello conserva `importInfo` opzionale nello schema v9. Le omissioni persistenti dichiarano il sottoinsieme importato; non ci sono aggiornamenti di schede esistenti o scritture verso Beyond. Nessuna dipendenza Foundry o toolchain firmware aggiunta.
