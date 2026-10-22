# Borsello offline — v0.51

**Aggiornamento v0.53:** monete e storico sono inclusi nel
[backup nativo del dispositivo](BACKUP-DISPOSITIVO.md), con ripristino confermato.

**Aggiornamento v0.52:** lo [storico dei tiri](STORICO-TIRI.md) è ora persistente.
Le nuove scritture della scheda sono PDD25; USB9 e backup PC v11 restano gli stessi.
I dettagli PDD24 sotto descrivono il rilascio v0.51.

Il blocco grafica/UX resta concluso nel software alla v0.50. Si riprende il
blocco 1, scheda sul dispositivo, con le monete. Storico persistente, regole
speciali ed eccezioni restano aperti; il backup dei progressi dal dispositivo
al PC è ancora da implementare. Le prove hardware appartengono al blocco 3.

## Sul dispositivo

**Menu → Zaino → Monete** apre cinque saldi separati:

| Taglio | Sigla display | Campo PC/USB | Equivalente in rame |
| --- | --- | --- | --- |
| Rame | MR | cp | 1 |
| Argento | MA | sp | 10 |
| Electrum | ME | ep | 50 |
| Oro | MO | gp | 100 |
| Platino | MP | pp | 1000 |

Due pagine mostrano tutti i saldi. Toccando un taglio si prepara una bozza;
le frecce passano agli altri tagli e **Passo** cambia l'incremento da 1 fino a
100.000.000. I pulsanti ± rispettano l'intervallo **0–999.999.999** per taglio.
**Annulla** scarta tutta la bozza. **Anteprima** mostra i cinque prima/dopo;
**Indietro** conserva la bozza e **Conferma** salva tutti i tagli insieme.

La conferma non scrive se i saldi sono identici. Un errore di memoria lascia
lo stato precedente e la bozza disponibile per riprovare, senza ricevuta di
successo. Durante modifica e conferma il menu, le scorciatoie fisiche e una
nuova importazione non possono interrompere la bozza. Le assegnazioni PWR/BOOT
restano quelle configurate dall'utente.

**Annulla monete** ripristina e salva l'ultima modifica se era una modifica al
borsello. È un solo livello di annullamento della sessione, condiviso con le
altre operazioni; dopo un'altra modifica o riavvio non è uno storico. Anche
un errore di salvataggio dell'annullamento conserva saldo e possibilità di
riprovare. La reimportazione non è annullabile con questo comando.

Un vecchio salvataggio o una copia priva di monete mostra **Saldo non
disponibile**. **Configura saldo** permette di verificarlo e creare un saldo
locale, anche tutto zero. Sconosciuto e zero verificato restano distinti.

L'equivalente in rame è solo una somma informativa a 64 bit: fino a
1.160.999.998.839 MR. Non effettua cambi, non consente di spendere oro
automaticamente prendendo argento o platino, non compra oggetti e non calcola
peso, capacità dei contenitori o prezzi. Entrate e spese si registrano
correggendo esplicitamente i saldi, d'accordo con il DM.

## Preparazione PC e Beyond

In **Stato, risorse e note → Monete** i cinque importi hanno la casella
**Saldo verificato, anche se zero**. Il backup PC resta v11, con
`extended.currenciesAvailable` opzionale. Nei backup precedenti senza questa
proprietà, almeno un importo non zero rende il saldo disponibile; tutti zero
restano da verificare. Nessun importo viene cancellato da questa inferenza.
Usare l'app aggiornata per conservare anche la distinzione fra zero e ignoto.

La conversione Beyond marca verificati i cinque saldi quando l'oggetto
`currencies` è presente e valido. Se è assente, il saldo resta sconosciuto;
campi mancanti, negativi, frazionari o oltre limite non vengono corretti
silenziosamente. **In questo incremento non è stata fatta una lettura reale
Beyond**: la nuova distinzione è verificata su sorgenti sintetici. Le prove
reali precedenti restano separate in [BEYOND-PROVE.md](BEYOND-PROVE.md).

L'anteprima PC per il dispositivo mostra disponibilità e tutti i cinque
importi. Un saldo non verificato resta conservato sul PC, ma non viene
presentato come denaro disponibile sul dispositivo. Download e trasferimento
non modificano la raccolta personale. L'importazione sul display segnala se
il saldo è verificato e sostituisce, solo dopo conferma, l'intera copia attiva
locale, comprese le monete. Prima di reimportare considerare i progressi locali:
la sincronizzazione dispositivo → PC non è ancora disponibile.

## Compatibilità

- Copia USB **v9**: oggetto obbligatorio `wallet` con esattamente `available`
  booleano e `cp`, `sp`, `ep`, `gp`, `pp` interi nell'intervallo indicato.
  Con `available:false` tutti i campi USB devono essere zero. Chiavi duplicate,
  dati incoerenti o versioni future vengono rifiutati prima di modificare stato.
- Il firmware legge ancora USB v1–v8, con borsello non disponibile. L'uploader
  aggiornato riconosce `PDD READY 1` … `9`; impedisce l'invio di USB9 a firmware
  precedenti. Aggiornare firmware e script insieme.
- Nuovi record NVS **PDD24**, tag `PDDO`, aggiungono 21 byte prima del CRC:
  disponibilità (1 byte, 0/1), poi cinque uint32 little-endian nell'ordine sopra.
  Dimensione `8875 + 552 × numero magie/capacità`, massimo **75115 byte**.
  Generazione, CRC32, doppio banco e layout delle partizioni restano invariati.
- PDD1–23 sono letti senza scritture al boot e senza inventare un saldo.
  La prova v23 usa un record generato dal vero encoder v0.50, commit `b2b1537`.
  Dopo una nuova scrittura PDD24 usare firmware v0.51 o successivo: i precedenti
  non conoscono il formato e devono conservare il record, senza fallback o erase.

## Verifiche software

Tutti i dati delle prove sono originali o sintetici; nessuna scheda personale
è stata modificata e nessun JSON completo Beyond è incluso nella distribuzione.

- **153 test JavaScript**, inclusi importazione sintetica, backup, flag
  opzionale precedente, saldo zero/ignoto e immutabilità della proiezione.
- **105 casi PC → USB9 → parser C reale**: cinque saldi esatti, equivalente
  confrontato con aritmetica BigInt indipendente, limiti e input malformati.
- **245 combinazioni C** di taglio/importo/delta, oltre a totale a 64 bit,
  rifiuto atomico, corruzioni con CRC valido, PDD23 reale e record massimo.
- Regressioni core/NVS complete: fixture PDD1–23, nessuna scrittura al boot,
  nuovo salvataggio e riavvio con saldi esatti, banchi precedenti conservati,
  errori di memoria/I/O e versioni future.
- **91/91 viste** esercitate in LVGL host. Percorsi reali di tocco per bozza,
  annullamento, zero verificato, cinque saldi, errore/retry, annullamento con
  errore/retry, conferma senza modifica, importazione e tasti conservati.
- Core/parser, NVS, UI e borsello eseguiti anche con AddressSanitizer/UndefinedBehaviorSanitizer;
  flusso PC verificato in Chromium isolato, senza accesso al browser personale.
- Build PC Vite/HTML autonomo ed ESP32-S3 con ESP-IDF 5.5.5/LVGL 9.4.0.
  Binario app 3.368.016 byte (`0x336450`), 20% della partizione app libero.

Le catture in `firmware-host-preview/wallet-preview.png`, le singole schermate
`wallet-*.png` e il catalogo `ui-catalog.html` nell'archivio provengono dal
firmware C/LVGL eseguito su PC. **Non sono fotografie del dispositivo.** Non
sono stati eseguiti flash, trasferimenti USB fisici o collaudi di alimentazione,
touch, leggibilità, consumi o durata della memoria sul dispositivo reale.
