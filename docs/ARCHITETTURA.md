# Architettura proposta e verifiche

## Stato implementazione al 19 ottobre 2026

Il dispositivo è il runtime principale; il PC prepara schede e backup.
La v0.15 introduce `firmware/` con ESP-IDF 5.5.5 e LVGL 9.4.0, display/touch
ricavati dai sorgenti della scheda esatta. Core C, salvataggio NVS su due copie,
import USB con conferma e proiezione PC sono separati. Il modello firmware v1
è un sottoinsieme esplicito del backup PC v9/v10: nessuna equivalenza di regole
implicita. Dettagli, pin, memoria, compilazione e gate hardware in
[FIRMWARE.md](FIRMWARE.md). La v0.16 aggiunge il renderer d20 pixel art, con atlas in flash e canvas
dinamico da 81.920 byte; calcolo del tiro e salvataggi restano separati dalla
presentazione ([dettagli](DADI-ANIMATI.md)). La v0.17 estende lo stesso canvas a sei tipi di
dado e gruppi fino a sei; aggiunge 138.240 byte di atlas in flash, senza
modificare i formati USB/NVS. La v0.18 aggiunge la palette comune in
`firmware/core/visual.h`, scena e quattro icone I4 originali in flash
(12.272 byte di dati), generate da `scripts/generate-device-art.py`.
Il tema è applicato alle viste LVGL reali e non modifica il modello di gioco.
Testi di una riga con altezza esplicita impediscono ai nomi lunghi di invadere
i valori. Dettagli e anteprime in [DIREZIONE-VISIVA.md](DIREZIONE-VISIVA.md).
La v0.19 separa ambiente e avatar: maschere originali, palette condivisa con il
PC, buffer I4 statico da 5.824 byte e timer attivo solo nelle viste Eroe/Aspetto.
L'ingrandimento è prodotto a dimensione nativa con replica dei pixel, senza
trasformare l'immagine indicizzata in LVGL. Il core di presentazione resta puro.
La copia USB v2 porta quattro indici estetici; il firmware legge v1/v2 e negozia
la capacità tramite `PDD READY 2`. NVS scrive `PDD2`, legge anche `PDD1` senza
riscrittura all'avvio e conserva la banca precedente alla prima modifica.
La v0.20 aggiunge `core/checks.c` per calcoli di prove/abilità/salvezze, copia USB
v3 e NVS PDD3 da 512 byte. Legge i record precedenti da 384 byte senza inventare
competenze: lo stato distingue dati non importati da gradi zero reali. Il parser
conserva blocchi di conversione, mezze competenze ed extra; gli effetti attivi
supportati sono due istantanee esplicite. UI e renderer restano separati dal
calcolo. Il confronto di integrazione passa dal JSON PC al parser C e confronta
1.890 risultati/modalità con il modello PC. [Dettagli](ABILITA-SALVEZZE.md).
Il resto di questo documento conserva le scelte
progettuali e le verifiche ancora necessarie.

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
| Input e USB | PWR sul PMIC, BOOT GPIO0 e connettore Type-C |
| Audio | Doppio microfono, codec a basso consumo e circuito di cancellazione eco |

Il 15 ottobre 2026 la pagina prodotto e la pagina wiki hanno risposto HTTP 403 dall'ambiente cloud. Il 19 ottobre il repository ufficiale esatto è stato letto al commit e4344e70: hardware reference, schema e BSP sono ora confrontati con il firmware. Le prove fisiche restano aperte; PWR passa dal PMIC/expander e non è un GPIO libero.

Gli 8 MB di PSRAM rendono plausibile mantenere buffer grafici e una cache di sprite, ma non garantiscono frame rate o compatibilità DMA. I 16 MB di flash possono ospitare firmware e risorse essenziali; la SD ospiterà cataloghi più ampi e backup. Il primo partizionamento usa app da 4 MB e NVS da 24 KB; non usa ancora la SD. La dimensione misurata della build è riportata in FIRMWARE.md.

Controlli iniziali: pin, driver e configurazione sono confrontati con il sorgente ufficiale; sulla scheda fisica completare:

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

Incremento 0.14: `src/beyond-details.js` amplia la normalizzazione dopo il controllo dei valori principali: scelte di competenza, profili arma, magie per incantatore, contatori e note/capacità per livello. `src/extended.js` valida lo schema v10 e centralizza i PF temporanei anche per il combattimento; `src/extended-ui.js` espone stato e risorse. `src/powers.js` distingue slot ordinari/patto e contatori innati; `src/model.js` gestisce i bonus riconosciuti e gli effetti espliciti in combattimento. `scripts/check-beyond-rich.py` prova importazione e gioco su dati sintetici estesi, oltre alle regressioni esistenti. Il connettore e il salvataggio atomico della raccolta restano separati dalla conversione. Nessuna dipendenza Foundry o modifica firmware.

Incremento 0.21: `firmware/core/combat.c` calcola attacchi, danni e cure,
`packet.c` valida USB v4 con inventario fino a 60 voci, `pocketdd.c` codifica
PDD4 da 5184 byte e legge PDD1–PDD3. Partizioni invariate. UI e salvataggio
applicano consumo/PF nella stessa transazione; i tiri di attacco/danno restano
transienti. `scripts/check-combat.js` confronta le regole C con il modello PC
attraverso il parser effettivo, senza leggere dati personali. Dettagli, omissioni
e verifiche in [INVENTARIO-ATTACCHI.md](INVENTARIO-ATTACCHI.md).

Incremento 0.22: `core/magic.c` modella costi, slot/patto, concentrazione e
capacità riconosciute; `core/buttons.c` filtra PWR/BOOT con azioni al rilascio.
`board.c` legge BOOT GPIO0 e PWR dal TCA9554 P4 senza scritture PMIC. La UI
conserva le preferenze locali agli import. USB v5, stati estesi in PSRAM,
PDD5 compatti e nuova NVS a 0x410000; la partizione legacy resta in sola lettura.
Selezione record senza allocazioni intermedie ambigue: OOM non diventa perdita
silenziosa della banca più recente. [Schema e verifiche](MAGIE-TASTI.md).


Incremento 0.23: `core/rest.c` valida piani di recupero prima della mutazione e
modella spesa/cura base dei dadi vita. `src/recovery.js` fornisce il riferimento
deterministico; il parser reale è attraversato dai confronti PC/USB/C.
`pd_recovery` conserva fino a 20 riserve, con disponibilità distinta da elenco
vuoto. USB v6 aggiunge `recovery`; PDD6 aggiunge 882 byte al record compatto
precedente senza cambiare partizioni. `hit_candidate` e gli stati della UI
restano in PSRAM. La cura è estratta dopo conferma, salvata insieme al consumo
e poi animata; un tentativo ripetuto nella stessa finestra non rilancia.
La nuova scorciatoia locale 7 apre Riposi, senza autorizzare conferme fisiche.
Fixture PDD5 generato dall'encoder `a6c525f`, migrazione senza scritture al boot,
CRC/allocazioni/versioni future verificati. [Dettagli](RIPOSI-RECUPERI.md).


Incremento 0.24: `core/status.c` gestisce contatori, d20 morte base e annotazioni;
`src/status.js` fornisce il riferimento deterministico. `pd_hp`, cure, capacità
e recuperi proteggono il caso tre fallimenti a zero PF. Le condizioni non sono
un motore di effetti: il flag Avvelenato è indipendente e l'editor PC lo espone
separatamente. `status.available` indica il registro morte; aggiungere note
locali non lo inizializza. USB v7/PDD7 aggiungono 1865 byte prima del CRC, senza
cambiare partizioni o backup PC. Il candidato della conferma è condiviso fra
cura con dado vita e tiro morte, con mutua esclusione dei dialoghi; rimane in
PSRAM. I tentativi di salvataggio ripetuti riusano il risultato estratto.
La scorciatoia 8 è Stato. 16013 confronti aggiuntivi attraversano il parser
reale; fixture PDD6 generato dall'encoder `da7b8e2` prima della modifica.
[Schema, uso e limiti](STATO-CONDIZIONI-MORTE.md).


Incremento 0.25: core/screen.c separa tempo d'inattività, quarantena degli input
e applicazione/riprova della luminosità. Il read callback touch della board usa
lo stesso controller del ciclo principale: nessun input di risveglio raggiunge
LVGL prima del rilascio. screen_service esercita tasti, blocchi e driver nel task
proprietario. Il setter usa l'API CO5300 pubblica dopo il drain DMA. PDD8 aggiunge
due byte locali prima del CRC; USB v7 invariato, prefs conservate da import/undo.
Fixture PDD7 generato dall'encoder a39413c. [Dettagli](SCHERMO-IMPOSTAZIONI.md).


Incremento 0.26: pd_state mantiene CA/max_hp come basi e un elenco di otto
pd_effect locali. pd_armor/pd_max_hp sommano solo i delta attivi; pd_effect_set
valida il totale prima di applicare il limite ai correnti, senza allocare un
intero stato sullo stack. Cure, pozioni, dadi vita e riposi usano lo stesso
massimo effettivo. Le conferme riusano scratch in PSRAM; nessun tiro RNG.
PDD9 aggiunge 33 byte a PDD8; USB v7 e backup PC v10 non cambiano. Fixture PDD8
prodotto dall'encoder 9a5c946 prima della modifica, con dati sintetici. Scorciatoia
10 Effetti, conferme/import protetti. [Uso e limiti](EFFETTI-LOCALI.md).


Incremento 0.27: pd_effect aggiunge timed/rounds, con validazione di pausa,
scaduto e riattivazione. pd_effect_advance decrementa tutte le voci attive a
round, valida il totale finale e soltanto allora limita i PF; rollback di tutte
le voci e maschere intatte su errore, senza grande stato sullo stack. La UI
riusa scratch in PSRAM per la conferma, ricalcolando dal medesimo stato su retry.
Nessun orologio o RNG; riposi, zero PF e concentrazione indipendenti. PDD10 con
tag PDDA aggiunge 16 byte al payload PDD9; fixture v9 dall'encoder 6999e80 prima
della modifica. Scorciatoia 11 Round apre l'anteprima protetta. USB v7 e layout
flash invariati. [Schema e limiti](DURATE-EFFETTI.md).


Incremento 0.28: core/damage.c separa il piano di danno, anteprima e applicazione
senza RNG. La preview distingue configured/allowed, temporanei, massimo effettivo,
zero/massive e concentrazione. apply rifiuta senza mutazione e scrive PF/esito
insieme tramite il candidato scratch in PSRAM. pd_defences usa quattro maschere
uint16, con flag sottoinsieme dei tipi configurati; le copie precedenti partono
da sconosciuto, non da normale. PDD11 (PDDB) aggiunge 8 byte prima del CRC;
fixture PDD10 generato dall'encoder 066e3d8 prima della modifica. Scorciatoia 12
Danni; conferme/import/input protetti. USB v7, backup PC v10 e layout invariati.
[Uso e limiti](DANNI-DIFESE.md).


Incremento 0.29: pd_concentration_info valida il piano e la salvezza COS prima
del RNG; pd_concentration_roll usa il totale contro la CD scelta e produce un
solo candidato con danni/esito. UI riusa hit_candidate in PSRAM, con esclusione
reciproca delle conferme. Dopo estrazione un save error fissa dado e opzioni fino
al retry/riavvio; lo stato non salvato non è un giornale persistente. L'animazione
combat_roll=6 mostra il risultato impegnato senza percorso di rilancio. Nessuna
migrazione o nuovo campo in PDD11/USB v7. [Uso e limiti](CONCENTRAZIONE.md).


Incremento 0.30: ogni pd_effect conserva un riferimento facoltativo alla magia
attualmente concentrata. effects_valid impone totali validi con e senza tutti
quei riferimenti; pd_concentration_end disattiva/scollega tutte le voci prima
di limitare i PF una sola volta. Sostituzione, stessa magia rilanciata, Ira,
danni/zero PF e riposo lungo usano questa operazione senza allocare stati
sullo stack. Nessun fallimento di validità possibile dopo il RNG per uno stato
accettato. La preview danni espone massimo e PF finali, compreso il limite
indotto dalla perdita di concentrazione; il delta PF del riposo è con segno.
PDD12 (PDDC) aggiunge otto byte prima del CRC, massimo 74222; PDD1–PDD11
letti senza vincoli e senza scritture al boot. USB v7/backup PC v10 invariati.
[Uso, vincoli e limiti](EFFETTI-CONCENTRAZIONE.md).


Incremento 0.31: pd_defences conserva 13 extra temporanei, ciascuno con flags,
active/timed, rounds e concentrazione facoltativa. pd_defence_base_flags conserva
la lettura delle sole basi; pd_defence_flags restituisce base OR extra attivo,
senza marcare un profilo sconosciuto come configurato. L'editor inizializza la
base dalla prima funzione, evitando di rendere permanenti i contributi temporanei.
pd_round_advance valida e avanza prima gli effetti numerici; solo in caso valido
avanza le difese, senza ulteriori punti di fallimento. Un unico candidato, output
con maschere separate e nessuna copia di pd_state nello stack. Fine concentrazione
scollega/disattiva anche tutti gli extra associati. Il colpo usa la fotografia
delle difese precedente a questa fine. PDD13 aggiunge 65 byte prima del CRC,
massimo 74287; PDD1–PDD12 letti senza extra e senza scritture al boot. USB v7 e
backup PC v10 invariati. [Uso e limiti](DIFESE-TEMPORANEE.md).


Incremento 0.32: pd_damage_event contiene 1–6 componenti di un solo impatto.
La preview raggruppa per tipo, applica le difese presenti all'impatto e ricava
un totale unico per temporanei/PF/concentrazione. L'ordine delle righe e la
suddivisione per uno stesso tipo non cambiano arrotondamenti o soglie. Un profilo
sconosciuto blocca l'intero evento. pd_damage_event_summary conserva gruppi,
flags, valori grezzi/effettivi e risultato complessivo; i vecchi ingressi singoli
sono wrapper dello stesso motore. Candidati e dettaglio restano fissi insieme
al dado sul retry; il risultato non usa i profili successivi alla fine dei vincoli.
PDD13/USB v7/backup PC v10 invariati; composizione e dettaglio solo RAM.
[Uso e perimetro](DANNI-MISTI.md).


Incremento 0.33: 78 selettori persistenti, uno per proprietà base/extra e tipo;
validità vincolata alla presenza del flag. pd_defence_flags espone i soli grant
Sempre; la risoluzione dell'impatto considera i 16 contesti completi compatibili
con i fatti noti, unisce le due fonti per proprietà e richiede solo fatti che
possono ancora cambiarla. Immunità certa elimina incertezze irrilevanti su R/V.
L'evento contiene contesti paralleli alle sei componenti; la preview espone
maschere distinte di profili mancanti, righe/fatti necessari e proprietà non
risolte. Raggruppamento canonico per tipo/flags applicati, con separazione delle
applicabilità diverse. Candidato e gruppi restano fissi sul retry. PDD14 aggiunge
78 byte, massimo 74365, e legge PDD1–PDD13 con Sempre senza scritture al boot.
Fonte solo RAM, USB v7/backup PC v10/layout invariati. [Dettagli](DIFESE-CONDIZIONALI.md).
