# Storico dei tiri sul dispositivo — v0.52

La v0.52 continua il blocco 1 della roadmap. Dopo le monete, il dispositivo
conserva gli **ultimi 20 tiri registrati**, anche al riavvio. Il blocco 1 resta
aperto per regole speciali ed eccezioni. Il backup dei progressi dal dispositivo
al PC è ancora da implementare; le prove fisiche appartengono al blocco 3.

## Uso e contenuto

**Menu → Dadi → Storico**, oppure assegnare **Storico** a PWR breve, BOOT breve
o BOOT lungo da **Aspetto → Tasti**. Le 13 scelte precedenti conservano numero
e significato; Storico è la scelta aggiuntiva. PWR lungo resta riservato
all'alimentazione. Durante conferme protette e dadi in volo valgono i blocchi
precedenti; premere un tasto non duplica un tiro.

L'elenco mostra tre voci per pagina, dalla più recente. Toccando una voce si
leggono nome completo, tipo di tiro, bonus, totale, modalità effettiva,
singole facce e nota dell'esito. Le frecce nel dettaglio scorrono sei dadi per
pagina, fino a tutti i 18 dadi. Le voci lunghe sono abbreviate nell'elenco e
leggibili nel dettaglio. Il registro usa lo stesso ambiente, icone e controlli
del compagno; non richiede nuovi asset o un canvas aggiuntivo.

Sono registrati undici tipi:

| Tipo | Dati conservati |
| --- | --- |
| Libero | d20 o pool, somma/danni, bonus e facce |
| Prova, abilità, salvezza | Etichetta, bonus, modalità effettiva e contesto dichiarato per vista/paura ed esclusione degli effetti |
| Attacco | Arma, facce, bonus, modalità, CA dichiarata, colpito/mancato/critico |
| Danni | Arma, pool, bonus e totale; nessun danno applicato al bersaglio |
| Cura | Oggetto, pool e PF effettivamente recuperati dopo limite ai massimi |
| Dado vita | Pool, bonus e cura effettiva |
| Morte | d20 base e PF/contatori risultanti |
| Concentrazione | Facce, bonus, modalità, CD, mantenuta/persa e PF risultanti |
| Ricarica | Oggetto, dadi/bonus e cariche prima/dopo, compresi zero e limiti |

Le facce e il calcolo sono fotografie dell'esito: cambiare equipaggiamento,
caratteristiche o condizioni non riscrive i tiri precedenti. Per vantaggio o
svantaggio sono conservati entrambi i d20; il totale usa il dado scelto.
Per i pool con minimo zero si conserva il totale limitato, insieme a facce e
bonus. Le cure e ricariche distinguono il tiro dal recupero effettivo.

Il numero `#` stabilisce l'ordine locale, non è una data. Il firmware non
inizializza un orologio reale. Al ventunesimo tiro viene rimossa la voce più
vecchia. Consultazione, navigazione, ridisegno e accelerazione dell'animazione
non estraggono dadi né scrivono il registro.

## Errori e conferme

Un tiro libero viene registrato dopo l'estrazione e prima dell'animazione.
Un tiro che modifica PF/usi viene registrato dopo il salvataggio riuscito dello
stato di gioco. Le bozze non applicate o i tiri rifiutati non diventano esiti
registrati. La registrazione non decide un effetto e non applica nuovamente
PF, consumi o regole.

Se il registro non si salva, il risultato resta visibile con **Tiro non
registrato**, compare in cima allo storico e rimane in memoria. **Riprova**
salva lo stesso risultato: nessuna nuova estrazione, nessun nuovo consumo.
Anche un tentativo ripetuto dopo un errore di commit/rilettura non aggiunge
duplicati. Un errore del registro non viene presentato come errore dei PF già
salvati; gli errori della scheda conservano la precedenza.

Prima di un altro tiro o di una nuova importazione bisogna risolvere la voce
pendente. Si può riprovare oppure scegliere **Scarta**, con conferma esplicita.
Scartare rinuncia alla registrazione; **non annulla** PF o utilizzi già salvati.
Le modifiche manuali senza dadi rimangono possibili. Il risultato non
registrato va perso al riavvio: il display invita a salvarlo prima di spegnere.

**Scheda e registro hanno due commit distinti.** Un'interruzione esattamente
fra il salvataggio della scheda e quello del registro può lasciare PF/consumi
aggiornati senza l'ultima voce. Inoltre un errore di conferma della scrittura
può lasciare una voce realmente scritta, ritrovata al riavvio. Il registro non
è un diario transazionale completo, non ricostruisce eventi mancanti e non
certifica che un'azione non sia avvenuta perché non compare nell'elenco.

## Importazione, compatibilità e memoria

La conferma di una nuova copia USB avvia uno storico distinto, anche per un
personaggio con lo stesso nome. L'anteprima segnala il nuovo storico; Annulla
o un salvataggio della scheda fallito conservano quello precedente. La raccolta
PC, le monete importate e le assegnazioni locali dei tasti restano conservate.
Non si trasferisce il registro PC al dispositivo né quello locale al PC.

- **USB9 e backup PC v11 invariati.** L'identità dello storico è locale e non
  viene accettata dal pacchetto USB. Nessuna lettura o scrittura Beyond in
  questo incremento; i dati delle prove sono originali o sintetici.
- Nuova scheda **PDD25**, tag `PDDP`: aggiunge quattro byte little-endian di
  identità locale prima del CRC. Dimensione `8879 + 552 × numero magie/capacità`,
  massimo **75119 byte**. PDD1–24 letti con identità zero, senza scritture al
  boot, conservando i saldi presenti e le vecchie assegnazioni dei tasti.
  Il valore scorciatoia 13 è ammesso solo nel nuovo schema.
- Registro **PDH1** nel namespace separato `pdd_history` della partizione
  esistente `pdd_state`, chiavi alternate `hist_a` e `hist_b`. Generazione,
  identità, sequenza, conteggio, voci esplicite e CRC32. Ogni voce occupa 123
  byte; record `21 + 123 × numero voci`, massimo **2481 byte**. Nessun dump
  della struct e nessun nuovo layout/erase.
- Un tiro libero scrive solo questo record piccolo. Un'azione con consumi
  mantiene il salvataggio della scheda e aggiunge quello del registro.
  Conferma tramite commit e confronto della rilettura prima del successo UI.
- Lettura dei due banchi e recupero della copia valida senza scritture al boot.
  Errori di lettura, formati futuri o record troppo grandi impediscono scritture
  al registro, senza cancellare dati. La scheda e il registro hanno stati di
  errore distinti. Ogni nuova importazione chiude il vecchio handle del registro.
- Un'importazione cambia identità solo con il salvataggio riuscito della scheda.
  Il primo nuovo tiro usa l'altro banco; il vecchio storico non è un archivio
  multi-personaggio e verrà sovrascritto dalle scritture successive.
- Dopo nuove scritture PDD25 serve firmware v0.52 o successivo. I precedenti
  devono conservare il formato sconosciuto e bloccare la scrittura. Le sequenze
  non vengono riutilizzate in caso di overflow: si rifiuta una nuova voce;
  un'identità importazione esaurita blocca un nuovo import invece di mescolare dati.

Lo storico non registra correzioni manuali di PF/monete, riposi senza dadi,
annullamenti o ogni evento di gioco. I fallimenti automatici senza estrazione
non sono tiri di dado e non entrano nel registro. Non fornisce un annullamento
persistente, ricostruzione della scheda, esportazione, orari reali o tutte le
regole D&D. Questi limiti non sono risolti dall'esito HTTP di Beyond.

## Verifiche software e anteprime

- **4364 casi indipendenti C** per facce, dadi, quantità, bonus e modalità;
  rotazione delle 20 voci, record massimo, corruzione di ogni byte, tutte le
  troncature, campi incoerenti e rifiuto senza modifiche parziali.
- NVS reale compilato su host con periferica finta: riavvio, zero scritture al
  boot, banchi alternati, errori set/commit/rilettura, tentativi idempotenti,
  recupero, formati futuri, overflow della generazione e identità distinte.
- Fixture precedenti **PDD1–24**, comprese monete v24 generate dal vero encoder
  v0.51 (`7e56973`), con saldi conservati e PDD25/rilettura/nuova scorciatoia.
- Suite UI reale: tutti gli undici tipi collegati, **94/94 viste**, consultazione
  senza RNG/scritture, errori e retry senza duplicati, scarto confermato,
  18 facce, nomi lunghi, 20 voci, nuova importazione e tasti conservati.
- **153 test JavaScript**, confronti sintetici PC → USB → C e regressioni delle
  funzioni precedenti; build PC Vite/HTML autonomo ed ESP32-S3. Binario app
  3.375.376 byte. partizione app da 4 MiB ancora 20% libera.
- Core/parser, scheda NVS, registro NVS, storico e UI con
  AddressSanitizer/UndefinedBehaviorSanitizer (leak detector disattivato).

L'archivio include `firmware-host-preview/history-preview.png`, le singole
catture `history-*.png` e il catalogo `ui-catalog.html`. Sono catture del
firmware C/LVGL eseguito su PC con dati sintetici, **non fotografie del
dispositivo**. Nessun accesso alla raccolta personale, JSON completo Beyond o
credenziale pubblicata. Nessun flash o collaudo fisico di touch, USB, memoria,
alimentazione, consumi o fluidità. L'usura dovuta alle scritture e la tenuta
dopo un'interruzione di alimentazione vanno misurate sul dispositivo reale.
