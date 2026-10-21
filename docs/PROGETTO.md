# Proposta di prodotto

## Visione

PocketD-D deve sembrare un piccolo oggetto fantasy personale e risultare utile durante una sessione: consultare la scheda, effettuare tiri e aggiornare risorse senza interrompere il gioco.

Ipotesi iniziale da discutere: si gioca al tavolo con un Dungeon Master. Il dispositivo assiste il giocatore; il DM mantiene il controllo delle decisioni e delle eccezioni. Il gioco senza DM è un possibile progetto futuro e non è un requisito del primo prototipo.

## Principi

- Funzioni principali disponibili offline dopo il trasferimento del personaggio.
- Azioni frequenti raggiungibili con pochi tocchi; testo lungo in viste dedicate.
- Pixel art coerente, animazioni brevi e risultato dei dadi sempre leggibile.
- Valori calcolati spiegabili: mostrare dado, bonus e totale separatamente.
- Modifiche manuali consentite per gestire decisioni del DM ed eccezioni.
- Nessun account obbligatorio per utilizzare una scheda locale.

## Esperienza sul display rotondo

La zona centrale ospita contenuti e pulsanti; il bordo può mostrare indicatori, evitando testo e controlli importanti nelle parti tagliate dal cerchio. Dimensioni dei bersagli touch e leggibilità andranno provate sul dispositivo.

| Vista | Contenuto | Azione principale |
| --- | --- | --- |
| Eroe | Avatar animato, PF attuali/massimi, CA | Aprire i controlli PF |
| Scheda | Caratteristiche, abilità, tiri salvezza | Toccare una voce per tirare |
| Azioni | Attacchi e capacità selezionate | Risolvere un tiro con bonus visibili |
| Zaino | Oggetti, quantità, equipaggiamento | Consultare o cambiare una voce |
| Risorse | Slot e utilizzi limitati | Consumare o ripristinare con conferma |
| Dadi | Tipo di dado e modificatore | Tirare e vedere il dettaglio |

Navigazione proposta: scorrimento laterale tra sezioni, tocco per aprire, pulsante indietro visibile. Non affidare funzioni indispensabili a gesti nascosti. Ridurre i gesti in conflitto con lo scorrimento degli elenchi.

## Primo prototipo: una sequenza completa

1. Accendere e visualizzare un eroe dimostrativo originale.
2. Aprire una caratteristica ed effettuare un tiro di d20 con bonus.
3. Vedere animazione, risultato naturale, modificatore e totale.
4. Modificare i PF, annullare un errore e applicare una condizione annotata.
5. Riavviare e ritrovare lo stato salvato.

Il tiro deve essere calcolato indipendentemente dall'animazione. Un 20 naturale viene mostrato come tale; gli effetti dipendono dal tipo di tiro e dal regolamento scelto.

Non fanno parte di questo traguardo: importazione Beyond, editor completo, incantesimi automatizzati, griglia o multiplayer.

## Roadmap

| Fase | Risultato | Criterio di completamento |
| --- | --- | --- |
| 0 — Verifica hardware | Esempio ufficiale con display, touch e SD se supportata | Disegno e input funzionanti; lettura/scrittura verificata |
| 1 — Prova dell'esperienza | Simulazione interattiva circolare con dati originali | Sequenza del primo prototipo completabile con mouse/touch |
| 2 — Prototipo sul dispositivo | Avatar, dadi, PF e salvataggio | Sequenza completata sull'hardware, anche dopo riavvio |
| 3 — Scheda utilizzabile | Inventario, risorse, capacità e backup | Una sessione di prova senza perdere aggiornamenti |
| 4 — Personalizzazione | Editor avatar e trasferimento dall'app di supporto | Creazione, trasferimento e ripristino di un personaggio |
| 5 — Party locale | Associazione dispositivi e iniziativa condivisa | Riconnessione senza duplicare eventi o perdere stato |
| 6 — Griglia | Movimento e posizioni condivise | Piccolo incontro con visibilità e controlli leggibili |

L'integrazione Beyond procede come verifica separata. Un suo eventuale blocco non impedisce le altre fasi. L'ordine delle fasi 0 e 1 può sovrapporsi se l'hardware non è immediatamente disponibile.

## Punto attuale — v0.44

| Blocco | Stato software | Cosa manca |
| --- | --- | --- |
| Scheda individuale offline | PF/temporanei, prove, abilità/salvezze, zaino/cariche, armi, magie, risorse, riposi e dadi vita nel firmware | Monete, storico persistente, effetti completi e regole speciali; sessione reale |
| Effetti numerici | Otto modificatori CA/PF massimi, basi separate, cure integrate, durate manuali/a round/inizio-fine turno/minuti-ore e vincolo esplicito alla concentrazione con fine congiunta | Altre durate/regole, proprietà speciali e ricalcolo da equipaggiamento |
| Danni e difese | Profili manuali per 13 tipi, base/tre extra separati, durate/vincoli e scadenze comuni, PF/temporanei, salvezza COS, sei componenti per colpo, fonte esplicita e riduzioni manuali prima/dopo, tre profili locali con budget, conto completo, fino a tre spese da contatori/usi/slot ordinari-patto/cariche e reazione | Condizioni arbitrarie, più di tre extra per tipo, catalogo/requisiti automatici, oltre tre spese, costi alternativi ed eventi/formule di ricarica automatici, eccezioni alle salvezze e casi morte bloccati |
| Stato e morte | Note separate dagli effetti sui tiri di Avvelenato/Accecato/Spaventato/Prono/Trattenuto, d20 morte base e correzioni, reazione e inizio turno espliciti | Altri effetti/condizioni, durate e fonti multiple, eccezioni e morte istantanea |
| Grafica e comandi | Tema pixel art, avatar 81 combinazioni, dadi animati, PWR/BOOT configurabili | Leggibilità e precisione touch sulla scheda |
| Impostazioni schermo | Luminosità, inattività e risveglio protetto nella v0.25 | Consumi misurati, sospensione, batteria e alimentazione |
| Preparazione PC | Importazione Beyond, raccolta, configurazione, backup PC, copia USB con conferma | Trasferimento USB fisico e backup dei progressi aggiornati dal dispositivo al PC |
| Party e griglia | Prototipi nel browser | Trasporto e interfaccia sul dispositivo |

Le fasi hardware 0 e 2 restano aperte. Le funzioni individuali delle fasi 3–4
sono portate nel firmware, ma i criteri di completamento richiedono ancora una
sessione fisica, trasferimento/ripristino e misure. Non sono percentuali di
completamento né una dichiarazione di collaudo.

## Quattro blocchi concordati — v0.44

| Blocco | Punto attuale | Prossimo lavoro |
| --- | --- | --- |
| 1 — Scheda sul dispositivo | Percorsi base nel firmware: magie/slot/usi, concentrazione, riposi, dadi vita e morte base | Eccezioni e verifica nell'uso reale |
| 2 — Effetti | Modificatori, round, concentrazione, difese, danni e riduzioni con costi/reazione; v0.38 aggiunge slot ordinari/patto; v0.39 durate del proprio turno; v0.40 minuti/ore; v0.41 cariche distinte dalla quantità; v0.42 tre spese congiunte; v0.43 ricarica parziale a dado esplicita; v0.44 quattro effetti di condizione sui tiri | Durate di altri soggetti, costi alternativi o oltre tre fonti, eventi/formule di ricarica automatici e requisiti/effetti automatici |
| 3 — Prodotto fisico | Luminosità, attenuazione/risveglio, salvataggi, tasti configurabili e firmware compilato | Batteria, sospensione/spegnimento e collaudi reali touch/USB/memoria/alimentazione |
| 4 — Estetica | Tema pixel art, avatar e dadi animati, UI circolare | Illustrazioni, sprite, icone, transizioni/feedback e leggibilità sul display reale |

Il blocco 2 resta aperto. Le cariche hanno ora un modello distinto dalla quantità,
configurazione PC, trasferimento, correzione locale, costi e recuperi al riposo.
Non vengono dedotte automaticamente da Beyond. Tre spese distinte sono ora
configurabili e addebitate insieme. La ricarica parziale a dado è disponibile
come evento manuale. Restano costi alternativi o più di tre fonti, eventi/formule
automatici, durate di altri soggetti e altri effetti/regole ancora manuali.
Accecato, Spaventato, Prono e Trattenuto hanno ora effetti sui tiri espliciti,
con contesto e blocchi sui fatti necessari, senza dedurre regole dalle note. Hardware e qualità visiva restano
traguardi del dispositivo, senza espandere il PC in un prodotto di gioco separato.

## Priorità confermata: dispositivo prima del PC

Il dispositivo è il prodotto. Il PC è per importazione, configurazione,
trasferimento e backup; il simulatore rimane un banco di prova. Il lavoro sulle
funzioni di gioco procede nel firmware, senza attendere importazione autonoma
Beyond, Foundry, party o servizi remoti. La v0.15 porta il primo percorso offline
in ESP-IDF/LVGL: scheda essenziale, d20, PF/annulla, contatori e salvataggio.
L'USB trasferisce una copia limitata con conferma sul display, senza cambiare la
raccolta PC. La v0.16 aggiunge dadi d20 animati nel firmware con sprite originali, rimbalzi,
lancio doppio e risultato naturale separato dal totale. La v0.17 aggiunge
d4/d6/d8/d10/d12 e lanci manuali Somma/Danni fino a sei dadi con bonus,
senza applicare danni automaticamente o migrare salvataggi.
La v0.18 stabilisce la prima base grafica moderna in pixel art direttamente
nel firmware: scena originale, avatar base, icone e tema condiviso.
La v0.19 aggiunge avatar personale con 81 combinazioni dall'editor PC,
anteprima USB coerente, animazione sobria e preferenza locale per fermarla.
Il disegno pixel per pixel e nuove silhouette restano successivi.
La v0.20 porta 18 abilità e sei salvezze nel firmware, con contributi spiegati,
maestria, mezze competenze, extra e blocchi espliciti dei valori non verificati.
La v0.21 porta inventario, attacchi/danni e cure. La v0.22 porta magie/capacità, slot ordinari/patto, utilizzi e concentrazione, oltre a PWR/BOOT configurabili. La v0.23 porta riposi espliciti e dadi vita. La v0.24 porta registro morte base e note delle condizioni; restano effetti completi, eccezioni e storico.
Il porting della scheda estesa procede con questa direzione visiva; collaudo
hardware, alimentazione e autonomia restano attività necessarie in parallelo.
Vedere [FIRMWARE.md](FIRMWARE.md) e [DIREZIONE-VISIVA.md](DIREZIONE-VISIVA.md).

## D&D Beyond e contenuti

La verifica del 16 ottobre 2026 ha esaminato DDB Importer e ddb-proxy di MrPrimate come riferimenti tecnici. Il requisito è un'importazione diretta in PocketD-D tramite URL, con un connettore interno all'app di supporto, senza passaggi obbligatori da Foundry, file intermedi o servizi MrPrimate. Il primo prototipo proposto legge un personaggio pubblico e presenta anteprima e omissioni prima di creare la scheda locale. Non è emersa una API pubblica documentata per questo uso: accesso dal vivo e conversione dei dati principali sono ora verificati su un caso, mentre conversione completa e condizioni di distribuzione richiedono ulteriori verifiche. Autenticazione per schede private e scrittura verso Beyond sono traguardi separati; la v0.14 implementa l'importazione estesa da PC con limiti espliciti con anteprima e conferma, conservando le schede esistenti. Dettagli, fonti e limiti sono in [BEYOND-FATTIBILITA.md](BEYOND-FATTIBILITA.md).

L'utente preferisce poter importare direttamente dal dispositivo. Se troppo complesso, accetta l'importazione nell'app PocketD-D su PC e il trasferimento della scheda. Prima verificare accesso e conversione nel prototipo PC; poi valutare l'accesso autonomo via Wi-Fi o un comando dal dispositivo verso un backend PocketD-D, dichiarandone la dipendenza dal servizio. La v0.15 implementa il primo firmware e un trasferimento USB limitato; l'uso e il trasferimento reali richiedono ancora il collaudo hardware. Il personaggio caricato deve restare utilizzabile offline per il gioco individuale.

È disponibile un [banco di prova PC](BEYOND-PROVE.md) in sola lettura. Il blocco del primo tentativo cloud è superato: accesso HTTP, struttura della risposta e valori convertiti sono documentati separatamente su una scheda reale; i test sintetici coprono altri casi. La proiezione firmware ha un perimetro più ristretto e non eredita automaticamente la completezza dell'importatore PC.

L'acquisto di contenuti su Beyond non stabilisce da solo i diritti di utilizzo da parte di PocketD-D. Verificare termini, licenze e modalità di distribuzione. Non includere cataloghi proprietari, immagini, descrizioni o credenziali Beyond nel repository.

Per iniziare: personaggi e grafica originali, inserimento manuale e formato locale documentato. Un eventuale regolamento aperto deve indicare fonte, versione, licenza e attribuzioni richieste. Distinguere le regole 2014 da quelle 2024: la versione sarà un dato esplicito della scheda.

## Decisioni da discutere

- Confermare il tavolo con DM come esperienza iniziale.
- Affinare avatar e ambienti nella direzione richiesta: pixel art moderna e
  curata sul dispositivo. La prima base propone fondi scuri e accenti menta/oro,
  con testi leggibili.
- Scegliere il primo regolamento e quali automatismi offrire.
- Definire alimentazione, autonomia desiderata e modalità d'uso durante la sessione.

La grafica iniziale deve restare originale e sostituibile: non serve acquistare o generare un intero catalogo di asset prima di provare il flusso.


## Stato della simulazione browser

Gli incrementi 0.1–0.6 hanno aggiunto scheda, avatar personalizzabile, inventario e risorse manuali, abilità e salvezze, magie/capacità personali e tracker di combattimento locale con bersagli. Il tracker anticipa una parte dell'esperienza di combattimento senza completare le fasi hardware, party o griglia. La verifica sul dispositivo fisico rimane necessaria prima di dichiarare praticabile l'interfaccia sul display da 1,75 pollici.


L'incremento 0.7 aggiunge una prima griglia locale 9 × 9 con muri, pedine e movimento per turno. Anticipa la prova dell'interfaccia di mappa; non completa la fase 6 della roadmap, che richiede ancora visibilità e controlli verificati sull'hardware e posizioni condivise tra dispositivi.

L'incremento 0.8 aggiunge zoom 5 × 5, panoramica e assistenza geometrica agli attacchi. L'incremento 0.9 aggiunge una raccolta locale di personaggi indipendenti, con migrazione del salvataggio, selezione, duplicazione e backup completo. Questa è una parte dell'app di supporto della fase 4; il trasferimento al dispositivo resta da sviluppare.

## Prosecuzione senza hardware

L'hardware non è ancora disponibile. Lo sviluppo procede sulla simulazione e sull'app di supporto, mantenendo aperta la verifica fisica delle fasi 0 e 2:

1. **Completato nella 0.9:** raccolta di più personaggi con progressi separati e backup.
2. **Completato nella 0.10:** inventario estensibile, armi e cure personalizzabili, mantenendo le regole dimostrative esplicite e la compatibilità dei backup.
3. **Completato nella 0.11:** prima revisione di grafica, avatar e navigazione del display circolare con prove browser desktop/mobile. La verifica fisica resta aperta.
4. **Completato nella 0.12:** primo protocollo di party tra finestre dello stesso browser, con iniziativa, turni, PF e mappa condivisi, riconnessione e deduplicazione. Restano collegamento tra dispositivi, associazione, controlli del party nel cerchio e gestione completa della sessione.
5. Base firmware v0.15 preparata dagli esempi ufficiali. Completare collaudo della scheda: display/touch, USB, riavvio e perdita di alimentazione, PMIC e autonomia. Inventario/attacchi portati nella v0.21 e magie/capacità nella v0.22; riposi e dadi vita portati nella v0.23; stato/registro morte base portati nella v0.24; portare poi gli altri flussi, con le stesse verifiche del modello PC; SD e party successivi.

Codice e stato della roadmap vengono salvati su GitHub a ogni incremento verificato. Il recupero della v0.8 è documentato separatamente in [RECUPERO-v0.8.md](RECUPERO-v0.8.md).


L'incremento 0.10 porta inventario e armi oltre le tre voci dimostrative: ogni personaggio possiede una lista manuale di oggetti, consumabili, cure e armi. Gli attacchi e la portata seguono l'arma equipaggiata. La CA resta manuale; non sono ancora automatizzati peso, monete, munizioni, proprietà speciali o un catalogo di regole.

L'incremento 0.11 rivede la presentazione fantasy, le icone e i comandi del dispositivo, mette la simulazione in primo piano su mobile e aggiunge navigazione rapida, tastiera e gesti. La personalizzazione dell'avatar usa anteprima, scelte visive e preset compatibili con le schede esistenti. Questa è una revisione verificata nel browser; non completa la verifica hardware né l'editor per disegnare asset pixel per pixel.


L'incremento 0.12 anticipa le fasi 5 e 6 con un trasporto browser sostituibile. Il coordinatore conserva lo stato autorevole del party; ogni finestra mantiene la propria identità e riprende lo snapshot corrente dopo un'interruzione. I dati del party sono separati dalla raccolta personale. La prova con più finestre non completa le fasi hardware o la comunicazione tra dispositivi. Uso e limiti sono descritti in [PARTY.md](PARTY.md).

Il percorso principale rimane articolato nei quattro blocchi concordati:

1. **Scheda sul dispositivo:** magie/slot/capacità/concentrazione, riposi, dadi
   vita e tiro morte base implementati; eccezioni e sessione fisica aperte.
2. **Effetti:** priorità software attuale. La v0.26 aggiunge modificatori locali
   espliciti a CA/PF massimi; la v0.27 aggiunge round/pausa/scadenze simultanee
   confermate. La v0.28 aggiunge difese manuali per tipo e danni ricevuti con
   esito esplicito della concentrazione; la v0.29 aggiunge il tiro COS con
   bonus/CD verificabili e salvataggio prima dell'animazione. La v0.30 collega
   esplicitamente gli effetti alla concentrazione, con disattivazione e PF
   coerenti alla fine/sostituzione. La v0.31 aggiunge difese temporanee separate
   dalla base, con durata/vincolo e round comune agli effetti numerici. La v0.32
   compone fino a sei componenti per colpo con raggruppamento e un solo esito.
   La v0.33 aggiunge condizioni difensive e fonte esplicita per componente.
   La v0.34 porta a tre gli extra indipendenti per tipo; la v0.35 aggiunge
   riduzioni manuali prima/dopo e conto completo del colpo. La v0.36 aggiunge
   tre profili persistenti con budget, regole, durate e vincoli. Seguono
   condizioni arbitrarie, proprietà speciali e
   durate di turno/minuti. Nessuna
   regola dedotta dai nomi delle note.
3. **Prodotto fisico:** luminosità/attenuazione implementate; batteria,
   sospensione/spegnimento e collaudo di touch, tasti, USB, memoria, fluidità e
   perdita d'alimentazione restano aperti.
4. **Estetica:** base pixel art, avatar e dadi animati implementati; illustrazioni,
   icone specifiche, transizioni e feedback proseguono insieme alle viste.

Il backup/ripristino dei progressi dal dispositivo verso PC rimane supporto
necessario alla conservazione dei dati e ancora da completare. Non sostituisce
il blocco effetti né sposta il prodotto sul PC. Importazione autonoma,
party/griglia fra dispositivi e scrittura Beyond rimangono successivi.


L'incremento 0.23 completa il blocco di riposi espliciti e dadi vita: anteprima,
conferma touch, criteri di recupero e cure base animate, con salvataggio atomico
ed effetti limitati al modello riconosciuto. I tasti configurabili includono
Riposi e non saltano le conferme. Import PC e raccolta rimangono strumenti di
preparazione; nessun bisogno di Foundry. [Uso e limiti](RIPOSI-RECUPERI.md).


L'incremento 0.24 porta il registro morte sul dispositivo, con d20 base animato,
stabilizzazione e correzioni confermate. Le condizioni hanno annotazioni
consultabili/modificabili; Avvelenato conserva un comando meccanico separato,
senza far passare la visualizzazione di un nome per l'applicazione delle regole.
La scorciatoia Stato si aggiunge alle assegnazioni fisiche esistenti.
[Uso e limiti](STATO-CONDIZIONI-MORTE.md).


La v0.25 aggiunge impostazioni locali e prima gestione dell'inattività del
pannello. Non completa la fase hardware/energia: niente percentuali batteria
inventate o promesse d'autonomia. Il backup dei progressi dal dispositivo al PC rimane aperto, senza sovrascrivere
in silenzio la raccolta originale. Importazione autonoma e comunicazione fra dispositivi
rimangono successivi. [Dettagli](SCHERMO-IMPOSTAZIONI.md).


La v0.26 avvia il blocco effetti con CA/PF massimi locali e valori base separati.
Conferma touch, annullamento, cure e riposi rimangono coerenti; i tasti possono
aprire Effetti senza confermare al posto del giocatore. È un modello numerico
esplicito, non un ricalcolo generale delle regole D&D. [Dettagli](EFFETTI-LOCALI.md).


La v0.27 aggiunge durata manuale/a round, pausa e scadenza degli effetti locali.
Avanzare un round è esplicito, con anteprima dei PF, salvataggio congiunto e
annullamento. La nuova assegnazione Round apre soltanto l'anteprima. Non è un
orologio delle regole o iniziativa automatica. [Dettagli](DURATE-EFFETTI.md).


La v0.28 aggiunge profili manuali di resistenza/vulnerabilità/immunità per 13 tipi,
con non configurato distinto da normale, anteprima danni/PF/temporanei e risultato
concentrazione esplicito prima del salvataggio. I casi già a zero e possibile
morte istantanea sono bloccati, non dichiarati gestiti. I tasti aprono Danni senza
confermare. Il percorso rimane dispositivo prima del PC. [Dettagli](DANNI-DIFESE.md).


La v0.29 aggiunge il tiro di concentrazione nel flusso danni, senza inventare
competenze per copie incomplete. Formula della CD e modalità esplicite, candidato
congiunto danni/esito, animazione dopo salvataggio e tiro fissato sul retry.
Il risultato pendente resta in RAM: recupero dopo perdita d'alimentazione e
storico persistente rimangono aperti. [Dettagli](CONCENTRAZIONE.md).


La v0.30 chiude il legame esplicito fra concentrazione e modificatori locali:
fine, nuova magia (anche la stessa voce), Ira, danni/zero PF e riposo lungo
terminano i vincoli in un'unica modifica. Controlli dei totali anche senza
concentrazione, limiti PF applicati una volta e annullamento completo. PDD12,
lettura PDD1–PDD11; le funzioni restano sul dispositivo. Il lavoro successivo a questo incremento riguarda
difese condizionali/con durata e componenti di danno miste,
con scelte esplicite e anteprima. [Dettagli](EFFETTI-CONCENTRAZIONE.md).


La v0.31 aggiunge un profilo temporaneo per tipo di danno: proprietà aggiuntive
esplicite, durata a round e vincolo facoltativo alla concentrazione. 1 round
avanza insieme effetti numerici e difese con una sola conferma/annullamento.
Le basi restano separate e i colpi usano le difese presenti all'impatto.
PDD13 legge PDD1–PDD12 senza riscriverli al boot. L’incremento successivo porta
le componenti di danno miste; restano difese condizionali e più fonti con
scadenze distinte per tipo. [Dettagli](DIFESE-TEMPORANEE.md).


La v0.32 aggiunge i danni misti: sei righe di un solo impatto, raggruppamento
per tipo prima delle difese, un totale per temporanei/PF/concentrazione e un
candidato salvato prima dei dadi. Dettaglio originale conservato dopo la fine
dei vincoli, profilo mancante che blocca tutto e annullamento congiunto.
PDD13/USB v7/layout invariati. Nel blocco effetti restano difese condizionali,
più fonti temporanee per tipo, riduzioni fisse e regole particolari; il prodotto
fisico conserva priorità e richiede ancora i collaudi della scheda.
[Dettagli](DANNI-MISTI.md).


La v0.33 copre le condizioni difensive di magico/non magico, attacco e alcune
eccezioni per argento/adamantio, separatamente per resistenza/vulnerabilità/immunità.
Fonte rimane esplicita e sconosciuto non significa falso. Il colpo è bloccato
solo se mancano fatti pertinenti; le parti si raggruppano per tipo e difese
applicate. PDD14 conserva le copie precedenti con Sempre. Restano più fonti con
scadenze distinte, riduzioni fisse, condizioni arbitrarie e ricalcoli completi;
il blocco hardware resta aperto. [Dettagli](DIFESE-CONDIZIONALI.md).


La v0.34 completa il supporto a tre extra per tipo, con regole/timer/vincoli
indipendenti, bozze comuni e confronti prima/dopo; 39 timer nel round atomico e
nessun accumulo delle proprietà uguali. PDD15 migra PDD1–PDD14 senza scritture al
boot. Restano riduzioni fisse, condizioni arbitrarie, durate di turno/minuti e
ricalcoli da equipaggiamento. Il blocco fisico resta aperto.
[Uso e limiti](DIFESE-MULTIPLE.md).


La v0.35 aggiunge riduzioni esplicite prima delle difese per componente e dopo
le difese una volta per colpo. Conto distingue quantità scelte/effettive, gruppi
e totale con cui si gestiscono PF e CD; errori e annullamento restano congiunti.
PDD15/USB v7/layout invariati. Restano applicabilità automatica, budget e costi
di riduzioni da talenti/oggetti, condizioni arbitrarie, altre durate e ricalcolo
da equipaggiamento; il blocco fisico resta aperto.
[Dettagli](RIDUZIONI-DANNI.md).


La v0.36 copre tre profili di riduzione locali persistenti, un budget condiviso
per colpo e scelta esplicita; condizioni/tipi, durate e vincoli verificati, con
snapshot del conto e round fino a 42 timer difensivi. PDD16 conserva PDD1–PDD15
senza scritture al boot. Restano cataloghi/requisiti/costi di capacità reali,
reazioni, cumulo di profili, condizioni arbitrarie, altre durate e ricalcolo
da equipaggiamento; il blocco fisico resta aperto.
[Dettagli](PROFILI-RIDUZIONE.md).


La v0.37 collega i profili a un contatore o agli usi limitati di magie/capacità,
con reazione facoltativa. Spesa congiunta con PF/concentrazione, saldi in anteprima,
blocco prima del RNG e ripristino completo. Inizio turno recupera soltanto la
reazione: round e riposi restano distinti. PDD17 legge PDD1–PDD16; tasti/USB/layout
conservati. Restano slot/cariche e costi multipli, requisiti/effetti automatici,
altre durate e blocco fisico. [Dettagli](COSTI-REAZIONE.md).


La v0.38 aggiunge costi da slot ordinari o del patto, con livello fissato nella
definizione, saldi in anteprima, spesa congiunta e nessuna sostituzione automatica.
Recuperi nei riposi distinti; nessun consumo al salvataggio della configurazione.
PDD18 legge PDD1–PDD17, dimensione/USB/layout invariati. Cariche oggetto,
costi multipli e altre durate restano successivi. [Dettagli](COSTI-SLOT.md).


La v0.39 distingue Manuale/Round/Inizio/Fine per i timer dei tre tipi locali.
Inizio/Fine offrono scadenze e CA/PF/reazione prima/dopo, un solo candidato,
rifiuto atomico e annullamento; fino a 50 scadenze su 17 pagine. PDD19 legge
PDD1–PDD18, tasti/USB/layout conservati. Il blocco effetti resta aperto per
altre durate, condizioni, cariche e automatismi. [Dettagli](DURATE-TURNO.md).


La v0.40 aggiunge minuti/ore e Stato → Tempo, intervalli espliciti con residui
precisi, scadenze cronologiche, limite PF a ogni passaggio e rifiuto atomico
anche degli intermedi invalidi. Round/turni e risorse restano separati. PDD20
legge PDD1–PDD19, tasti/USB/layout conservati; durate altrui, cariche e automatismi
restano successivi. [Dettagli](TEMPO-GIOCO.md).


La v0.41 porta le [cariche oggetto](CARICHE-OGGETTI.md) lungo l'intero percorso
PC → USB → dispositivo, con anteprima/conferma/annulla, spesa dei profili e riposi.
Backup v11, USB8 e PDD21; lettura delle copie precedenti senza pool inventati.
Nessun flash, collaudo fisico, lettura Beyond o modifica di raccolte personali.


La v0.42 aggiunge [spese multiple](COSTI-MULTIPLI.md) ai profili: fino a tre
fonti esatte più reazione, verifica/addebito atomici, confronto e ricevuta
congelata di tutte le spese. PDD22; USB8, backup PC v11 e layout invariati.
Nessun flash, collaudo fisico, nuova lettura Beyond o modifica di raccolte personali.


La v0.43 aggiunge [ricariche parziali a dado](RICARICHE-OGGETTI.md): formula
esplicita sul dispositivo, candidato limitato al massimo, salvataggio prima
dell'animazione, retry congelato, resoconto e Annulla. Zero senza scritture.
PDD22/USB8/backup v11/layout invariati. Eventi come l'alba restano manuali;
nessun calendario, lettura Beyond o registro persistente delle ricariche.


La v0.44 porta quattro [effetti di condizione sui tiri](CONDIZIONI-TIRI.md),
con contesto per vista/paura, fallimento automatico senza RNG, cancellazione
booleana, salvezze DES e configurazione/annullamento. PDD23, USB8/backup v11 e
layout invariati. Altre parti delle condizioni restano al DM. Il prossimo
blocco software concordato è una revisione estetica organica delle schermate
principali; il collaudo fisico rimane separato e necessario.
