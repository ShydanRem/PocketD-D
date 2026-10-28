# Backup dal dispositivo al PC — v0.53

> Aggiornamento v0.74: [associazioni personalizzate delle abilità](ABILITA-CARATTERISTICHE.md),
> Beyond tipo 27 ed editor PC, USB15 / backup PC v17, PDD34/PDDY. Lettura dei precedenti
> e partizioni invariate; nuova prova sintetica, verifica USB fisica ancora aperta.
> Le descrizioni e prove datate sotto conservano il proprio contesto.


> Aggiornamento v0.69: [21 famiglie e 42 silhouette](AVATAR-SELEZIONE-ESTESA.md),
> PDD32/PDDW; USB12 e backup PC v14 per gli ID nuovi.
> Aggiornamento v0.70: [preset di classe su SD](PRESET-CLASSI.md), PDD33/PDDX,
> USB13 e backup PC v15 per i look classe; Originale conserva i formati precedenti.

> Aggiornamento v0.68: [quattro nuove famiglie e compressione senza perdita](AVATAR-SPECIE-ANIMALI.md).
> Catalogo a 16 specie/32 silhouette, PDD31/PDDV; USB11 e backup PC v13
> per i nuovi ID. Formati precedenti conservati; collaudo fisico ancora aperto.

> Aggiornamento v0.67: [specie, silhouette e proposta da Beyond](AVATAR-SPECIE.md).
> Scrittura nativa PDD30/PDDU; USB10 e backup PC v12 per i nuovi avatar,
> con lettura dei formati precedenti. Le prove reali pregresse non attestano
> questa nuova selezione visiva; collaudo hardware ancora aperto.

> Aggiornamento v0.66: [avatar e sfondi locali](PERSONALIZZAZIONE.md).
> Il backup nativo v1 accetta ora anche PDD29/PDDT, con scena e assegnazione
> Aspetto ai tasti. Record massimo 75.183 byte; compatibilità con i record
> precedenti conservata. Usare l'app PC aggiornata per leggere i nuovi backup.

> Aggiornamento supporto PC v0.64: [ricezione e ripristino dal browser via USB](USB-BROWSER.md),
> con anteprima, controllo d'integrità e conferma sul dispositivo. Gli script
> Python descritti sotto restano disponibili come alternativa.


> Aggiornamento v0.60: [costi alternativi per colpo](COSTI-ALTERNATIVI.md).
> Ogni profilo supporta Tutte oppure Una scelta. PDD28/PDDS mantiene la dimensione
> di PDD27; backup nativo v1 e strumenti aggiornati accettano PDD25–PDD28.
> Descrizioni e limiti successivi si riferiscono alla versione storica del titolo.

> Aggiornamento v0.59: [nomi locali di A/B/C e PDD27](NOMI-TURNI.md).
> Il backup nativo v1 accetta PDD25/PDD26/PDD27; i riferimenti dei timer restano
> stabili. Formati e limiti sotto descrivono la versione del titolo.

> Aggiornamento v0.58: [turni esterni A/B/C e PDD26](TURNI-ESTERNI.md).
> Il backup nativo v1 accetta PDD25/PDD26; round e minuti/ore restano separati
> dai turni. Formati e limiti sotto descrivono la versione storica del titolo.

La v0.53 completa il percorso software di **acquisizione, anteprima e ripristino
del backup nativo**. Il dispositivo resta il prodotto principale; il PC
conserva i file e prepara il ripristino. Il trasferimento USB fisico, la tenuta
dopo interruzioni di alimentazione e le prestazioni reali restano da collaudare.

## Che cosa conserva

La copia contiene tutti i dati rappresentati nella scheda firmware: PF e basi,
effetti locali, durate, difese/profili/costi, reazione, condizioni esplicite,
inventario/cariche, arma, magie/slot/usi/concentrazione, dadi vita, riposi
rappresentati, monete, avatar, tasti, preferenze schermo e ultimi 20 tiri
registrati. Il contenuto viene mantenuto nei record nativi senza convertirlo
nel modello PC, che non rappresenta tutti gli effetti locali del dispositivo.

Rimangono esclusi pagina aperta, animazione in volo, bozza non applicata,
annullamento della sessione, risultati non registrati e altre strutture
transitorie. Descrizioni, note o dati Beyond non arrivati sul dispositivo non
possono essere ricostruiti dal backup. Non è il JSON originale di Beyond, un
dump della flash, un backup di credenziali o una sincronizzazione remota.

| File | Scopo |
| --- | --- |
| Backup scheda/raccolta PC | Conserva i dati dell'app PC, descrizioni e provenienza incluse |
| `pocketdd-device` USB9 | Copia ridotta PC → dispositivo, con impostazioni locali conservate |
| `pocketdd-device-backup` v1 | Copia nativa dispositivo → PC → dispositivo, compresi storico e preferenze |

La raccolta PC resta identica durante acquisizione, anteprima, download e
ripristino nativo. Non si cerca una scheda omonima e non si sovrappongono
progressi alle schede PC esistenti. Un'eventuale riconciliazione richiederà
identità e regole esplicite, oltre il nome del personaggio.

## Acquisizione dal dispositivo

Richiede firmware e strumento **v0.53 o successivi**, Python 3 e `pyserial==3.5`.
Il firmware va già installato: lo strumento non esegue flash, reset, erase o
scritture verso Beyond. Chiudere altri monitor della stessa porta seriale.

```sh
python3 -m pip install pyserial==3.5
mkdir device-backups
python3 scripts/device-backup.py read --port /dev/ttyACM0 --output device-backups/eroe-sessione.json
```

Su Windows usare, ad esempio, `--port COM5`; su macOS la porta `/dev/cu.usbmodem…`.
La cartella `device-backups/` è esclusa da Git; i file del personaggio si
conservano privatamente. La destinazione deve essere un **nome nuovo**. Sul display appaiono nome, PF/CA,
quantità di oggetti/magie, disponibilità delle monete e conteggio dello storico.
**Invia** autorizza la copia al PC; **Annulla** o 120 secondi di attesa conservano
lo stato. Da **Aspetto → Schermo → Backup** si consulta lo stato del percorso.

L'acquisizione congela una copia logica coerente e **non scrive NVS**, non
consuma risorse e non estrae dadi. Non parte durante una conferma protetta o
con un tiro non registrato da risolvere. Se un tiro già registrato è in
animazione, la sua presentazione viene completata senza cambiare il risultato.

Il PC controlla lunghezza, CRC del trasferimento, record, identità coincidenti
e conferma di fine invio. Crea il file solo dopo tutti questi passaggi, tramite
temporaneo privato, sincronizzazione e creazione atomica senza sovrascrittura.
Un file o collegamento già presente viene conservato anche in caso di gara
fra controlli e scrittura. La pubblicazione usa un hard link nella stessa
cartella: serve un filesystem che lo supporti, altrimenti si sceglie un'altra
cartella. Non compare un nuovo file finale con un trasferimento incompleto.

**Copia inviata** sul dispositivo indica l'invio dei byte. Il messaggio finale
del programma PC conferma il file verificato e salvato; un solo HTTP/handshake
o l'apparizione della richiesta sul display non sono un backup riuscito.

## Anteprima nell'app PC

Aprire l'app PC o l'HTML autonomo → **Backup del dispositivo** → scegliere il
file ricevuto. Sono visibili nome, livello, PF effettivi/temporanei, CA effettiva,
numero di oggetti/magie/risorse/effetti, cinque saldi, tasti, luminosità e tutti
i tiri conservati, con facce, bonus e note. Il nuovo download conserva il file
originale, senza perdere campi che il PC non espone.

L'anteprima verifica **integrità e dati selezionati**, non tutte le regole
native: il validatore completo C sul dispositivo controlla ogni campo prima
di proporre un ripristino. Le prove includono un record con CRC valido e un
campo non rappresentato dall'anteprima non valido: il PC non lo certifica
come applicabile, mentre il firmware lo rifiuta prima di qualunque scrittura.
Il messaggio di questa distinzione è visibile nell'anteprima.

## Ripristino sul dispositivo

```sh
python3 scripts/device-backup.py restore device-backups/eroe-sessione.json --port /dev/ttyACM0
```

Prima dell'invio si negozia la capacità backup v1. Un firmware precedente o
un protocollo futuro non riceve il contenuto della copia. Il firmware verifica
entrambi i record e la loro identità, poi mostra **Ripristina copia?** con i
valori principali. **Ripristina** sostituisce la sola scheda attiva e lo
storico sul dispositivo, **comprese le preferenze tasti/schermo del backup**.
Questo è diverso dall'importazione USB9, che conserva le preferenze locali.
Annulla o il timeout non applicano il ripristino.

L'identità locale viene rinnovata su entrambi i record; le generazioni NVS sono
quelle del dispositivo ricevente. I dati di gioco e le voci dello storico sono
preservati. L'annullamento di sessione viene disattivato: non ripristina la
scheda precedente dopo un import. La raccolta PC resta conservata.

Il programma dichiara successo solo dopo **PDD RESTORED**, inviato dopo le
scritture e riletture riuscite. Un timeout o una risposta persa lasciano esito
non confermato: controllare il dispositivo prima di riprovare. Non si esegue
un nuovo ripristino automaticamente.

## Scritture, errori e interruzioni

1. Preparare lo storico nel banco inattivo con una nuova identità, conservando
   il banco della scheda ancora attiva.
2. Commettere e rileggere la scheda nel suo banco inattivo.
3. Solo dopo il successo attivare in RAM scheda e storico insieme e rispondere
   al PC. Non servono ulteriori scritture per attivare il registro preparato.

Le identità presenti in entrambi i banchi vengono saltate sia dal ripristino
sia dall'importazione USB9 successiva. Un ripristino preparato e poi abbandonato
non può così prestare il suo storico a un altro personaggio. Dopo un errore
della scheda un nuovo ripristino non parte prima del riavvio.

Al riavvio il registro sceglie il banco valido corrispondente all'identità
della scheda recuperata. Un registro preparato per un'altra identità non
nasconde quello precedente. La generazione più recente rimane il riferimento
per le scritture successive. Nessuna scrittura automatica al boot.

Se la preparazione dello storico fallisce, la scheda resta precedente e si può
riprovare. Se il salvataggio della scheda fallisce, potrebbe essere già
persistito: le scritture della scheda e del registro restano bloccate fino al
riavvio, **anche annullando la schermata**. Non si deve sovrascrivere il registro
preparato continuando a scrivere sul vecchio. Il messaggio indica di riavviare.
Il recupero può trovare la coppia precedente oppure quella nuova completa.

Le prove simulate coprono tutti i sei punti set/commit/rilettura dei due
registri, inclusi record parziali e scritture persistite ma non confermate.
Non equivalgono a togliere realmente alimentazione alla scheda.

Questo meccanismo riguarda il **ripristino**. Restano due commit distinti per
le normali azioni di gioco introdotte nella v0.52: un'interruzione fra scheda
e storico può ancora perdere l'ultima voce del tiro. Il backup conserva quanto
effettivamente disponibile; non ricostruisce un evento mai registrato.

## Formati e protocollo

- File JSON ASCII limitato a **110000 byte**, con quattro chiavi esatte:
  `format:"pocketdd-device-backup"`, `version:1`, `state` e `history`.
  I due valori sono base64 canonici, senza escape. Chiavi duplicate, campi
  extra, formati futuri e codifiche non canoniche vengono rifiutati.
- Record file: scheda **PDD25** (`PDDP`, massimo 75119 byte) e storico **PDH1**
  canonico (massimo 2481 byte), ciascuno con CRC32 e identità coincidente.
  Sono serializzazioni esplicite, non struct o dump di memoria. Le generazioni
  nel file sono zero e non sostituiscono quelle del dispositivo ricevente.
- Registro NVS: nuove scritture **PDH2**, con lo stesso payload e dimensione
  di PDH1, ma recupero consapevole dell'identità. Il firmware v0.53 legge i
  PDH1 senza riscriverli al boot. Le versioni precedenti devono bloccare la
  scrittura del registro davanti a PDH2, conservando i banchi; usare v0.53 o successivo per
  continuare a usare il dispositivo dopo nuove scritture PDH2. Verificato anche
  con il codice v0.52 (`a07bf77`) su host con NVS simulata: PDH2 viene rifiutato
  senza scritture. Gli altri record e il layout restano
  invariati. Il formato del file è indipendente dal marcatore del banco NVS.
- USB: `PDD BACKUP HELLO` → `PDD BACKUP_READY 1`; `PDD BACKUP GET 1` →
  `PDD WAIT_BACKUP`; dopo conferma, `PDD BACKUP_DATA N CRC32`, esattamente N byte
  JSON e `PDD BACKUP_DONE`. Invio protetto da mutex rispetto alle altre risposte,
  frammenti e scritture parziali gestiti; limite di invio 30 secondi.
- Ripristino: invio del JSON nativo dopo negoziazione → `PDD WAIT_RESTORE` →
  `PDD RESTORED` soltanto dopo conferma e salvataggi. Errori/cancellazioni non
  sono successo. Timeout di conferma 120 secondi, verificato anche sul wrap.
- Proiezione ordinaria **USB9**, scheda **PDD25**, backup PC **v11**, partizioni
  e raccolta personale non cambiano. Nessuna dipendenza Foundry o lettura Beyond.

## Verifiche e immagini

- **161 test JavaScript**, inclusa una fixture originale generata dal vero
  encoder C: PF/CA con effetti, monete, preferenze, venti tiri, CRC, codifica,
  identità, input malformati e preservazione dei record completi.
- **36 roundtrip nativi C** fra 0 e 120 poteri, 0 e 20 tiri, identità limite,
  effetti, monete e impostazioni: tutti i campi confrontati mediante codec.
  Framing e CRC di invio, errori di scrittura e rifiuto senza modifiche parziali.
- Nuovo confronto C → anteprima PC e **9 casi PC → validatore C**, compresi
  record con CRC valido ma dati nativi non validi, duplicati e versioni future.
- Ripristino con entrambi gli store NVS di produzione su host e periferica simulata: sei interruzioni,
  coppie coerenti precedente/nuova, banchi precedenti conservati, blocco dopo
  errore incerto, annullamento della preparazione, tiri successivi e identità non riutilizzate
  da una nuova importazione.
- **9 test Python USB/file** con frammentazione, scritture parziali, rifiuti,
  timeout, file corrotto e destinazioni esistenti, senza porte seriali reali.
- **97/97 viste LVGL**: nuove anteprime, conferme, timeout, errori, ritorno
  all'eroe, stato immutato all'annullamento, nessun RNG o scrittura all'export.
- Chromium isolato: anteprima/venti voci, download identico, file errato e
  raccolta invariata. Nessun accesso al browser personale dell'utente.
- Suite firmware precedente, ASan/UBSan per codec/UI/store/ripristino e build
  PC Vite/HTML autonomo ed ESP32-S3 con ESP-IDF 5.5.5/LVGL 9.4.0.
  Binario app 3.380.608 byte, partizione app 4 MiB ancora 19% libera.

Nell'archivio: `firmware-host-preview/backup-preview.png`, schermate
`backup-*.png` e catalogo `ui-catalog.html`. Sono catture del firmware C/LVGL
su PC con personaggi sintetici, **non fotografie dell'hardware**. Nessun flash,
backup di un personaggio reale, collaudo USB fisico o pubblicazione di credenziali
e JSON completo Beyond. Fluidità durante il trasferimento, memoria disponibile,
alimentazione e affidabilità fisica dei salvataggi restano da misurare.
