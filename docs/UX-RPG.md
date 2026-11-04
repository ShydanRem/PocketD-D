# Pocket come RPG tascabile — revisione UX in corso

3 novembre 2026. La priorità concordata è rivedere l’esperienza complessiva del
dispositivo. I nuovi blocchi funzionali della roadmap restano annotati in
[STATO-SOFTWARE.md](STATO-SOFTWARE.md); la priorità attuale è UI/UX fino a un
risultato soddisfacente per l’utente. Questa revisione non dichiara finito il lavoro.

## Direzione

Pixel art 2D-HD, avatar completo e ambienti già approvati. Toni notte/petrolio,
avorio, ottone e un accento per categoria. Cornici nette e pulsanti con rilievo
sobrio; icone condivise, ingrandite senza sfocatura. Il dispositivo fisico da
466×466 px resta il riferimento, insieme al touch e ai tasti configurabili.

L’azione e il risultato vengono prima della spiegazione. Le formule, le proprietà
e le note estese rimangono nei dettagli. Costi, motivo di un blocco e problemi di
salvataggio devono essere visibili: ridurre il testo non significa nascondere
informazioni necessarie a decidere.

## Prima iterazione sul renderer nativo

- **Home:** avatar e scena al centro, azioni rappresentate da icone più grandi;
  nomi disponibili con pressione prolungata. Niente istruzioni permanenti.
- **PF:** danni, cura, temporanei e stato come azioni principali. Le correzioni
  manuali −1/+1 passano in una vista dedicata; annullamento ancora accessibile.
- **Attacco:** apertura diretta del comando di tiro; arma e variante magica
  selezionabili nella stessa schermata. Dettagli numerici dietro il riepilogo,
  catalogo completo delle varianti ancora disponibile. Nessun tiro o consumo
  automatico entrando nella schermata.
- **Magie e inventario:** icone coerenti, righe con gerarchia fra nome e stato,
  comandi secondari più compatti. Testi lunghi nei dettagli.
- **Struttura comune:** il messaggio sano «Salvato offline» non occupa sempre una
  riga. Errori, memoria incerta, recuperi e avvisi batteria rimangono visibili.

## Criteri di riuscita

1. L’utente riconosce l’azione principale prima di leggere una spiegazione.
2. La home apre l’attacco ordinario in un tap; il tiro richiede il suo comando.
3. Una variante con arma si seleziona senza passare obbligatoriamente da un menu.
4. La vista PF non propone subito modifiche che saltano difese/concentrazione.
5. Nomi lunghi, saldi massimi, stati bloccati ed errori restano leggibili.
6. Nessun cambio a regole, dati, formati, SD o assegnazioni personali dei tasti.

## Iterazioni successive della stessa priorità

Rivedere risultati dei dadi e danni, lancio delle magie e costi, uso/equipaggiamento
degli oggetti, preparazione, riposi e stato. Poi raccolta/importazione, aspetto e
impostazioni. Le viste avanzate non diventano automaticamente una UX completa
perché condividono il nuovo tema: ogni percorso va semplificato e rivisto.

La valutazione estetica dell’utente, le prove del renderer e il riscontro sul
display fisico sono evidenze distinte. Le immagini del renderer non dimostrano
fluidità, touch o consumi sull’hardware.

## Verifica e aggiornamento

Prima iterazione distribuita come v0.96. Aggiornare da GitHub Pages con
**Conserva dati**; nessuna nuova importazione o installazione dei look richiesta.
Le funzioni del componente PC non cambiano e il precedente resta compatibile.

I percorsi vengono esercitati attraverso gli eventi pointer del renderer LVGL:
home → attacco → lancio in due tap, scelta della magia con tutte le sei
caratteristiche, preparazione con annullamento e salvataggio fallito/riprovato,
assenza di consumo di slot durante la scelta, correzioni PF separate e pressione
prolungata senza eseguire l’azione. Restano controllati i tasti configurabili,
i confini circolari e i bersagli di almeno 44 px delle 153 viste.

[Anteprima nativa con personaggio sintetico](previews/v096-rpg-ux.png).
Non è una fotografia dell’ESP32 e non dimostra touch o fluidità fisici.

Dopo la prima iterazione restavano risultati e azioni successive al tiro,
selezione e lancio magie, equipaggiamento/uso degli oggetti, riposi/stato.
La seconda iterazione qui sotto copre i risultati e le schede di gioco;
equipaggiamento, conferme, riposi e stato restano da ripensare. Raccolta, aspetto,
importazione e impostazioni seguono nella stessa revisione, non vengono
considerati terminati dal solo cambiamento del tema.

Controlli della release: 356 test JavaScript, suite nativa e 153 viste LVGL,
ASan/UBSan e compilazione ESP32. Nessuna nuova lettura Beyond reale o modifica
alla raccolta personale; le nuove prove usano solo dati sintetici.

## Seconda iterazione · risultati e schede di gioco

Obiettivo: distinguere risultato, significato e prossima azione. Numero centrale
in carattere pixel originale a 48 px; critico oro, colpo mancato rosso, cure verde.
Il 20 naturale di una prova resta un 20 naturale: non viene chiamato critico o
successo automatico. Con bonus e due dadi si conservano i valori del tiro fissato.
Durante il volo il risultato rimane nascosto. Dadi animati sotto il numero,
comando successivo principale e navigazione secondaria per icone con nome su
pressione prolungata. Nessun nuovo tiro al termine dell’animazione o navigando.

Nelle schede magia/oggetto l’emblema identifica la voce prima della lettura.
Per le magie: nome, preparazione, valore utile, costo, azione; testo esteso
apribile dalla scheda. Costi e limiti manuali restano espliciti. Regole e roadmap
funzionale non cambiano. La home approvata della v0.96 rimane il riferimento.

### v0.97: comportamento e limiti

- Risultato numerico a 48 px, sopra i dadi. L’esito compare al termine del volo;
  toccare il dado o Accelera termina l’animazione dello stesso risultato.
- Dopo un colpo riuscito **Danni** è l’azione principale. Un mancato propone
  il nuovo attacco. Le componenti radianti, Hex/Marchio e Punizione mantengono
  il totale aggregato; cambiare componente non tira di nuovo e non registra
  copie nello storico.
- Cure e ricariche mostrano il guadagno effettivo dopo il limite massimo.
  Se non c’è memoria per l’animazione, il numero e l’esito restano visibili.
- Navigazione secondaria con le stesse icone di Eroe, Scheda, Storico e Usi.
  Pressione prolungata per il nome; tasti configurabili conservati.
- Scheda magia: emblema, nome completo, preparazione, attacco/CD o effetto,
  costo e comando. Tocca il nome per leggere il testo. **Effetti manuali**
  resta esplicito per le magie prive di automazione; l’icona non concede regole.
- Emblemi anche nelle schede oggetto. I nomi larghi adattano il carattere senza
  coprire i valori. Scudo ridisegnato nello stesso catalogo nativo/PC per renderlo
  più distinto dal teschio.

[Quattro schermate native](previews/v097-result-ux.png), PG e tiri sintetici.
La cura mostrata è un guadagno limitato dai PF massimi, non il valore grezzo dei
dadi. La nuova verifica attraversa gli eventi pointer del firmware, confronta
risultati e stato completo, conta RNG e scritture e verifica anche i nomi larghi
nei componenti figli. Nessun accesso Beyond reale o raccolta personale usati.

La v0.97 non introduce nuovi formati, risorse SD o regole e non chiude la revisione
UI. Non serve reimportare il PG né reinstallare il componente PC; aggiorna il
firmware conservando i dati. Nuova resa e interazioni da valutare sul dispositivo.

Per una magia senza modello d’effetto, **Profilo** indica il bonus di attacco
magico e la CD dell’origine da incantatore. Non implica che quella magia
richieda un attacco o un TS: **Effetti manuali** resta visibile e i dettagli
della magia stabiliscono quali valori siano pertinenti.

Verifica della v0.97: 356 test JavaScript, suite nativa, tutte le 153 viste
LVGL e nuovi scenari dei risultati, ASan/UBSan e compilazione ESP32. Le prove
software non attestano fluidità o risposta del touch sul display fisico.

## Terza iterazione · azioni e recuperi

Confronti prima/dopo a tessere con icona per equipaggiamento, costi e riposi.
Il comando nello zaino apre una bozza: nessuna applicazione senza conferma.
I dettagli completi restano disponibili nella stessa anteprima. Riposo lungo:
scelta diretta fra manuale, 2014 e 2024; nessuna edizione dedotta dal PG.
Perdita di PF, concentrazione, bonus senza sintonia ed errori restano visibili.
Questa iterazione mantiene regole, tasti configurabili e protezioni delle bozze.

### v0.98: equipaggiamento, conferme e riposi

- **Zaino:** l’icona della tunica apre direttamente la bozza Equipaggia/Riponi
  per armi e profili di oggetto noti. Il bordo oro segnala la dotazione attiva.
  Dal proprio elenco filtrato: un tap sull’icona, uno su Conferma, ritorno
  all’elenco. Quantità zero e profili assenti non diventano equipaggiabili.
- **Confronto dotazione:** CA, PF massimi/attuali, attacco e caratteristica o
  bonus danni; tutti gli altri effetti sono in Dettagli. La perdita di PF/CA
  o bonus attacco appare in rosso. Nessuna cura implicita aumentando i PF massimi.
  Sintonia e configurazione rimangono esplicite; nessun effetto dedotto dal disegno.
- **Magia:** prima/dopo di slot, utilizzi finiti, PF e concentrazione. Dettagli
  mostra anche CA e PF massimi quando termina un effetto vincolato. Slot e uso
  innato possono essere entrambi costi, senza sommarli o spendere in anticipo.
- **Consumabile:** quantità che verrà spesa, dadi o effetti manuali, PF attuali;
  nessun tiro di anteprima. Un salvataggio fallito conserva il risultato fissato.
- **Riposo lungo:** tre scelte dirette Manuale/2014/2024; cambiare regola scarta
  soltanto la selezione dei dadi da recuperare nella bozza. Il PG non cambia.
- **Riposo:** riepilogo PF, dadi vita e slot separati ordinari/patto. Utilizzi,
  cariche ed effetti terminati rimangono nei dettagli. Un massimo temporaneo che
  finisce può ridurre i PF: si mostra il numero reale, anche quando diminuisce.
  Dati non importati restano «Non disponibili», senza falsi zeri.

[Anteprima del renderer](previews/v098-transaction-ux.png), solo dati sintetici.
Le verifiche percorrono i controlli pointer del firmware, controllano l’identità
originale della riga filtrata, i due tap, annullamento, retry di salvataggio,
dettagli senza scritture/RNG, protezione dei tasti e dell’importazione, edizioni
esplicite, costi combinati e variazioni reali di PF/CA/concentrazione. I tasti
personali e i formati restano conservati. Nuove interazioni da provare fisicamente.

Questa iterazione non chiude l’intera revisione: restano le viste avanzate di
riposo/dadi vita, condizioni ed effetti, poi raccolta, importazione, aspetto e
impostazioni. La roadmap funzionale rimane separata e conservata.

Verifiche v0.98: 356 test JavaScript, suite nativa, tutte le 153 viste LVGL,
ASan/UBSan e build ESP32. Nuove prove con dati sintetici; nessuna nuova
lettura Beyond reale o modifica della raccolta personale.

## Quarta iterazione · riposo e stati

Riserve dei dadi vita selezionabili direttamente dal riposo breve, con pagine
per tutte le classi. Stato come griglia di azioni, note delle condizioni con
icone bilingui condivise, stati sui tiri scelti esplicitamente in una bozza.
Effetti locali: valore, attività, durata e vincolo distinti. Anteprime di modifica
e scadenza mostrano CA, massimi e PF reali prima/dopo. Le icone non attribuiscono
regole e il round avanza solo dopo conferma. Regole, tasti e dati rimangono compatibili.

### v0.99: riserve, condizioni ed effetti locali

- **Riposo breve:** pagine da tre riserve, per tutte le 20 classi/riserve del
  modello. Toccare la riserva apre direttamente la conferma; consultazione e
  paginazione non tirano dadi né scrivono. Il comando classico rimane disponibile.
  Esaurimento, PF massimi, zero PF e dati assenti restano protetti.
- **Dado vita:** quantità da spendere e PF attuali in evidenza; nessuna cura
  anticipata. Il dado viene speso anche con cura zero. Se il salvataggio fallisce,
  Riprova salva lo stesso risultato, senza nuova casualità.
- **Stato:** PF, temporanei e accessi visivi a condizioni, effetti, morte e danni;
  Tempo, Reazione e Usi rimangono accessibili. Avvelenato conserva il suo comando
  esplicito separato dalle note.
- **Condizioni:** catalogo condiviso PC/nativo per tutte le 15 condizioni standard,
  in italiano e inglese, con nove nuovi glifi. Icone, nomi e livelli sono note:
  aggiungerli non attiva penalità. L’attivazione dei quattro stati supportati sui
  tiri resta una bozza con anteprima e conferma; le altre regole restano manuali.
- **Effetti locali:** valore, attività/pausa, durata e vincolo C distinti. Ore e
  minuti mostrati nell’unità corretta: 180 minuti nella vista ore sono 3h 0min.
  Nessun timer diventa un conto alla rovescia reale: round e tempo avanzano
  attraverso i comandi già presenti e le loro conferme.
- **Modifica e scadenza:** CA, PF massimi e PF effettivi prima/dopo. Pause,
  scadenze, vincoli e difese mantengono il comportamento del motore. In rosso le
  perdite; aumentare un massimo non cura. Nessuna applicazione durante la vista.

[Anteprima nativa](previews/v099-session-states.png), dati e input sintetici.
Nuovi scenari: ultima riserva delle 20, cancel/retry senza nuovo RNG, confronto
completo dello stato, note senza effetti automatici, classificazione bilingue,
bozza dei tiri, timer in pausa, ore corrette e scadenza con perdita di PF/CA.

La revisione continua con raccolta/importazione, personalizzazione e impostazioni.
Gli editor avanzati di durate, difese e altri timer restano ulteriormente
rifinibili. Nessun nuovo formato o asset SD; le nuove interazioni richiedono
riscontro sul dispositivo fisico. La roadmap funzionale rimane conservata.

Verifiche v0.99: 357 test JavaScript, suite nativa, 153 viste LVGL, nuovi
scenari di sessione/stati, ASan/UBSan e compilazione ESP32. Dati sintetici;
nessuna nuova lettura reale Beyond o modifica della raccolta personale.

## v0.100: raccolta, aspetto e impostazioni

Raccolta e catalogo Beyond con schede coerenti, identità e stato visibili. Il
cambio PG mostra il suo avatar completo e valori prima della conferma; regole
di conservazione, copie e controlli di accesso non cambiano. Personalizzazione:
scelta diretta del campo o dello sfondo, con bozza e conferma conservate.
Impostazioni con luminosità immediata e accessi visivi a rete, alimentazione,
tasti e backup. Privacy e consensi non vengono rimossi per ridurre il testo.


- **I miei PG:** nome, livello e stato della copia separati; la copia attuale ha
  il bordo dorato. Il catalogo usa i metadati già presenti, senza aprire tutti i
  salvataggi per disegnare le righe. Le copie non leggibili rimangono conservate.
- **Apri e gioca:** avatar completo della copia selezionata, PF, temporanei,
  CA e tiri salvati prima della conferma. La selezione non cambia PG; alla
  conferma restano il checkpoint e la rilettura che rifiuta una copia SD cambiata.
- **Beyond:** righe con nome, livello e ultime cifre dell’ID; anteprima con
  avatar, PF, CA, temporanei e tutte le sei caratteristiche. Limiti e dati
  manuali rimangono consultabili e la conservazione su SD è esplicita. Accesso,
  blocchi della conversione, approvazioni e gestione dell’account non cambiano.
- **Aspetto:** `Modifica > Campi` apre la scelta diretta di carnagione, capelli,
  mantello, taglio, specie, silhouette, classe e sfondo. Le frecce continuano a
  scorrere i campi. I colori fissi dei look di classe restano disabilitati;
  PWR li salta, BOOT conferma, BOOT lungo torna dall’elenco all’editor senza
  buttare la bozza. `Anteprima > Conferma` salva solo le scelte estetiche.
- **Impostazioni:** luminosità in percentuale con due comandi immediati e
  attenuazione separata; Wi-Fi, energia, tasti e backup in una griglia coerente.
  Icone originali dedicate a rete, tasti e alimentazione. La vista non avvia
  connessioni e non inventa una percentuale di batteria se il dato è assente.

[Quattro schermate native](previews/v100-collection-setup.png), esclusivamente
PG e periferiche sintetici. Nuovi scenari coprono puntatore, campi diretti,
focus fisico con opzioni disabilitate, ritorno locale, annullamento, fallimento
salvataggio e riprova, conservazione delle regole e dello storico, impostazioni
senza chiamate radio, luminosità e nomi lunghi nelle anteprime. La raccolta
continua a usare il filesystem reale dei test e il codec nativo; account e
importazione usano i doppi di prova dei servizi, senza accesso reale a Beyond.

La revisione UX non è dichiarata conclusa: rimangono editor avanzati di regole,
difese, durate e configurazioni, coerenza delle schermate secondarie e riscontro
sul dispositivo fisico. La roadmap funzionale rimane in STATO-SOFTWARE.md.
Formati di PG, salvataggi, backup, USB e look SD compatibili.


Verifiche v0.100: 357 test JavaScript, suite nativa completa, 153 viste LVGL,
scenari con puntatore e tasti simulati, ASan/UBSan e compilazione ESP32 passati.
Il controllo memoria usa il renderer host; non certifica memoria o fluidità
sull’ESP32. Nessun account o dato reale del personaggio usato in queste prove.


## v0.101: editor di effetti, difese e timer

- **Effetti locali:** CA e PF massimi sono due scelte dirette con icona, valore
  della bozza e confronto con i valori attuali. Riselezionare lo stesso bersaglio
  conserva il modificatore; cambiare bersaglio lo azzera. L’anteprima e la
  conferma continuano ad applicare il motore esistente, senza cure anticipate.
- **Difese:** resistenza, vulnerabilità, immunità e stato della base/extra in una
  griglia con tre nuovi glifi originali. I flag possono coesistere; non diventano
  scelte esclusive. Le proprietà condizionali mostrano `Con regola`, non una
  promessa di dimezzamento o immunità in ogni situazione.
- **Conferma difese:** quattro confronti prima/dopo per le proprietà potenziali e
  la verifica della base. `Dettagli` conserva il precedente confronto completo
  di base, extra selezionato, timer e concentrazione; `Regole` resta disponibile.
  La base ignota non viene dichiarata normale e la conferma rifiuta dati invalidi.
- **Regole:** selettore condiviso di tutte le sette condizioni già supportate,
  tre per pagina. Accessibile dalle difese e dai profili di riduzione. Argento,
  adamantio, origine magica e natura di attacco mantengono le distinzioni del
  motore: non si deducono dal nome o dall’icona.
- **Durate:** scelta diretta fra Manuale, Round, Inizio turno, Fine turno, Minuti
  e Ore per effetti, difese e profili. Riselezionare lo stesso tipo conserva
  quantità e soggetto del turno; cambiarlo riparte dai valori iniziali espliciti
  del tipo e rimuove il soggetto esterno. La concentrazione resta separata.
- **Valori rapidi:** 1/5/10 round o passaggi, 1/10/60 minuti, 1/8/24 ore. Le
  regolazioni fini restano disponibili, incluse ore con precisione al minuto.
  I valori rapidi modificano solo la bozza, anche nel terzo profilo/extra.
- **Tasti:** PWR seleziona il comando disponibile, BOOT lo attiva e BOOT lungo
  torna di un livello; le opzioni disabilitate sono saltate. Funziona nei nuovi
  selettori, negli editor e nelle relative conferme. La prima pressione BOOT
  attiva il focus, senza salvare. Le assegnazioni personali restano conservate.

Le bozze degli editor non ricevono importazioni USB o nuove associazioni al loro
posto. Nei selettori sono accettate solo le scelte locali e Indietro; consultarli
non salva, non lancia dadi e non avvia collegamenti. Le scelte applicate passano
ancora dall’anteprima e dalla conferma esistenti; una riprova dopo errore non
aggiunge spese o casualità.

[Quattro viste del renderer nativo](previews/v101-rule-editors.png), con PG,
input e periferiche sintetici. Scenari nuovi: selezioni dirette e idempotenti,
conservazione/reset del soggetto del turno, ore con minuti residui, scelte rapide,
coesistenza di difese condizionali, riepilogo/dettagli senza scritture, fallimento
salvataggio e riprova, indice originale dell’extra/profilo 3 e navigazione fisica
locale senza attivare le scorciatoie di gioco.

Questa iterazione non conclude la revisione UX. Restano flussi di danni misti,
riduzioni e relativi costi, schermate secondarie e messaggi da semplificare;
servono riscontri sul dispositivo. Timer ancora avanzati da comandi espliciti,
nessuna scadenza in tempo reale o nuova regola automatica. Formati, PG, backup,
USB e asset SD compatibili; roadmap funzionale conservata.


Verifiche v0.101, 4 novembre 2026: 357 test JavaScript, suite nativa completa,
153 viste LVGL, nuovi scenari con puntatore e tasti simulati, ASan/UBSan e
compilazione ESP32 passati. Dati sintetici; nuove interazioni e fluidità da
valutare sul dispositivo. Nessuna nuova lettura reale Beyond o modifica della
raccolta personale.


## v0.102: danni misti, riduzioni e spese

- **Tipo di danno:** selettore di tutti i 13 tipi del motore, sei per pagina,
  senza attraversare i tipi intermedi. Icone dedicate e coerenti nel selettore,
  nelle parti e nelle riduzioni; quattro nuovi glifi originali. Le indicazioni
  della difesa distinguono normale, non verificata e condizionale. Non concedono
  proprietà o dichiarano l’origine del colpo.
- **Parti:** nome, icona e quantità leggibili separatamente. L’aggiunta e modifica
  conservano gli indici originali e la conferma del singolo componente. L’intero
  evento rimane un solo colpo; arrotondamenti per tipo e una sola prova di
  concentrazione mantengono le regole esistenti.
- **Fonte del danno:** griglia per magico, attacco, argento e adamantio. `?` resta
  sconosciuto e non diventa `no`; due nuovi glifi per i materiali. Annullare non
  modifica la dichiarazione già usata dal colpo.
- **Riduzioni:** scegli direttamente la parte prima delle sue difese oppure
  l’intero colpo dopo le difese. La stessa scelta diretta serve ad assegnare il
  budget dei profili: niente scorrimento obbligato fra le parti. Nessuna spesa
  o applicazione entrando nei selettori o premendo `Usa valori`.
- **Fonte della spesa:** scelta per famiglia (nessuna, contatori, poteri, slot
  ordinari, patto, cariche), poi voce con nome e saldo. Restano gli indici
  originali, anche potere 120 e oggetto 60. Non si propongono doppioni delle
  altre due righe. Una fonte esaurita può essere configurata, ma resta bloccata
  all’applicazione; il selettore non ripristina risorse. Riselezionare la stessa
  fonte conserva la quantità; una fonte diversa parte da costo 1.
- **Anteprima del colpo:** quattro riquadri per danno effettivo, PF, temporanei e
  costi. `Tipi` conserva raggruppamento, conto completo, profilo e ricevute. I
  dati mancanti continuano a bloccare l’applicazione senza mostrare un totale
  parziale come valido. Morte e concentrazione restano passaggi espliciti.
- **Spese nello stesso percorso:** saldo prima/dopo per le tre fonti e reazione
  separata; per costi alternativi si può cambiare fonte prima del colpo. Il
  riepilogo segnala che un costo può essere dovuto anche a danno azzerato.
  Applicazione del danno e pagamento restano un’unica conferma atomica.
- **PWR/BOOT:** navigazione locale estesa a colpo, parti, fonte, riduzioni,
  selezione dei costi e relative anteprime. Le bozze non vengono rimpiazzate da
  nuove importazioni o scorciatoie di gioco. Saldi, tasti personali e storico
  non cambiano mentre si consulta o si annulla. La scelta dei tasti viene raccolta
  dopo il calcolo del layout, così anche i selettori appena aperti sono
  immediatamente navigabili. Menu resta disponibile nelle viste principali
  del colpo per uscire esplicitamente dalla bozza.

[Quattro viste native](previews/v102-incoming-hits.png), con dati e input
sintetici. Scenari nuovi: 13 tipi e glifi distinti, colpo con resistenza e
vulnerabilità e temporanei, confronti con il motore, annullamento, salvataggio
fallito/riprovato, ultima parte del colpo, ultimi indici di poteri/oggetti,
doppioni rifiutati, quantità della stessa fonte conservata, tre spese e reazione
anche con danno zero, una sola applicazione e comandi fisici locali.

Non è la conclusione dell’intera revisione UX: restano turni e tempo, messaggi e
schermate secondarie da uniformare e riscontro sul dispositivo. La selezione
fra molti costi è paginata; non introduce ancora ricerca testuale. Nessun nuovo
formato, asset SD richiesto o automatismo di regole. Roadmap funzionale conservata.


Verifiche v0.102, 4 novembre 2026: 357 test JavaScript, suite nativa completa,
153 viste LVGL, nuovi scenari con puntatore e tasti, ASan/UBSan e compilazione
ESP32 passati. Archivio e GitHub Pages verificati prima della pubblicazione.
Prove sintetiche: non dimostrano touch, fluidità o radio del dispositivo fisico.
Nessuna nuova lettura Beyond o modifica della raccolta personale.


## v0.103: turni, tempo e riferimenti

- **Turni e reazione:** griglia con reazione, inizio e fine del proprio turno,
  riferimenti A/B/C. Quattro nuovi glifi originali; stato della reazione visibile
  prima di scegliere. Usarla manualmente annota solo la reazione, senza lanciare
  una magia, avanzare timer o spendere altre risorse.
- **Altri turni:** riferimento selezionato riconoscibile, nome completo con
  altezza adattata e comandi Inizio/Fine distinti. Ogni comando mostra quanti
  timer avanzerebbe in quella fase, dal motore esistente. A/B/C restano identità
  stabili, i nomi sono soltanto alias locali e non riassegnano i timer.
- **Tempo trascorso:** valori diretti 1/10 minuti, 1/4/8/24 ore e regolazione
  fine. Le scelte cambiano solo la quantità da proporre. Il tempo è dichiarato
  dall’utente: non è un orologio reale e avanzare otto ore non esegue un riposo.
- **Anteprime coerenti:** CA, PF massimi, PF effettivi e reazione prima/dopo;
  numero di timer che avanzano e scadono. Le perdite di PF oltre il nuovo massimo
  sono esplicite. Timer, reazione, recuperi e pause mantengono il motore esistente.
  Nessun effetto o conteggio viene applicato durante la consultazione.
- **Scadenze:** righe con icona e fonte per effetti, difese e riduzioni. I timer
  espressi in ore mostrano ore e minuti in entrambi i valori, anche quando il
  residuo scende sotto un’ora; le scadenze sono evidenziate. I timer in pausa
  non avanzano e non sono inseriti fra quelli modificati.
- **Tasti:** navigazione locale estesa a turni, tempo, scadenze e nomi dei
  riferimenti. Anche il tastierino da 17 comandi conserva tutti i pulsanti di
  fondo; opzioni disabilitate saltate, BOOT lungo torna indietro. Le assegnazioni
  personali restano conservate. Menu ed Eroe permettono di uscire esplicitamente
  dalle viste principali; anteprime e modifica del nome restano protette.

Il proprio inizio turno ripristina la reazione. Fine turno, A/B/C e tempo
trascorso non la ripristinano. Round, proprio turno, turno altrui e minuti/ore
restano quattro percorsi distinti; non si deducono l’uno dall’altro. I riferimenti
non costituiscono una gestione automatica dell’iniziativa o dei bersagli.

[Quattro viste native](previews/v103-turn-time.png), con PG e input sintetici.
Scenari nuovi: comandi touch, tutti i valori rapidi, conteggi di inizio/fine,
ambito proprio/A/B/C su effetti/difese/riduzioni, annullamento, riprova del
salvataggio, ore con minuti residui, timer in pausa, perdita di PF alla scadenza,
nessuna scrittura per operazioni senza variazioni, nome massimo e PWR/BOOT fino
ai comandi di fondo; stato, risorse, storico e casualità confrontati.

La revisione continua con consultazione, storico, messaggi e altre viste
secondarie; serve anche il riscontro sul dispositivo. Nessun nuovo formato,
asset SD richiesto o automatismo di regole. La roadmap funzionale resta in
STATO-SOFTWARE.md e non viene considerata conclusa dalla revisione grafica.


Verifiche v0.103, 4 novembre 2026: 357 test JavaScript, suite nativa completa,
153 viste LVGL, nuovi scenari con puntatore e tasti, ASan/UBSan e compilazione
ESP32 passati. Immagini native con PG sintetici; nuove interazioni, touch e
fluidità restano da valutare sul dispositivo. Nessuna nuova lettura Beyond o
modifica della raccolta personale.


## v0.104: consultazione e registro

- **Profilo importato:** tre schede per pagina, icone coerenti con il gruppo
  sorgente: profilo, movimento, sensi/passivi, lingue, strumenti, dotazioni e
  iniziativa. Movimento, occhio e fumetto sono tre nuovi glifi originali.
  Le voci da leggere indicano «Consultazione»; l'iniziativa indica «tiro» e
  richiede comunque il comando Lancia e le verifiche di contesto esistenti.
- **Ricerca:** Cerca filtra il nome o parte del nome con lo stesso tastierino
  dello zaino. Mostra applica; Indietro annulla la bozza. Il filtro del profilo
  è indipendente da quelli di magie e oggetti, conserva gli indici originali
  e si azzera quando viene adottata una nuova scheda. Nomi completi e testo
  paginato rimangono consultabili nel dettaglio.
- **Registro:** filtri diretti Tutti, d20, Danni e Recuperi. d20 comprende i
  record con dado a venti facce, anche liberi; Recuperi comprende cure, dadi
  vita e ricariche. Le righe separano fonte, totale e numero del record. La
  cronologia conserva il limite esistente di venti voci e non combina i valori
  delle componenti di un colpo in un totale inventato.
- **Dettaglio:** risultato numerico in pixel, nome completo fino a quaranta
  caratteri, formula, tipo, modalità e sei facce effettive per pagina. Nuovo
  tiro apre la preparazione: consultare o sfogliare non lancia nuovi dadi.
  Un venti archiviato non viene etichettato automaticamente come critico.
- **Tiro in attesa:** resta fissato in cima, anche se escluso dal filtro.
  Fino a cinque parti possono essere sfogliate senza rilancio. «Registro: tiro
  da salvare» è un avviso giallo distinto dagli errori della scheda, che
  mantengono la precedenza. Riprova registra tutte le parti insieme; Scarta
  richiede conferma e rinuncia soltanto alla voce, senza annullare PF, spese o
  altre azioni già salvate. Prima di spegnere occorre risolvere l'attesa: il
  tiro non ancora registrato risiede in memoria e non sopravvive alla perdita
  di alimentazione.
- **Tasti:** PWR seleziona, BOOT attiva e BOOT lungo torna indietro anche in
  profilo, testo, registro, filtri e ricerca. La consultazione lascia disponibili
  Menu ed Eroe; la bozza di ricerca e la conferma Scarta restano protette.
  Le assegnazioni personali dei tasti vengono conservate.

[Quattro viste native](previews/v104-reference-history.png), con dati e input
sintetici. Il profilo resta una fotografia dell'importazione: movimento, sensi,
lingue e dotazioni descrittive non diventano nuovi automatismi di regole.
Nessun formato di scheda o registro è cambiato; i look SD esistenti restano
compatibili. Restano da rivedere altre viste secondarie, messaggi di
installazione/collegamento e coerenza dell'intero percorso: la revisione UX
non è conclusa. La roadmap funzionale rimane in STATO-SOFTWARE.md.


Verifiche v0.104, 4 novembre 2026: 357 test JavaScript, suite nativa completa,
153 viste LVGL, scenari con puntatore e tasti, ASan/UBSan e compilazione ESP32
passati. Ricerca e filtri confrontati con gli indici originali, nomi da quaranta
caratteri e note da quarantotto, tutte le diciotto facce, riprova fallita/riuscita
con cinque parti, rinuncia senza annullamento e assenza di scritture alla scheda
o nuovi numeri casuali durante la consultazione. Le immagini sono catture del
renderer reale con dati sintetici. Nessuna nuova prova fisica USB/Wi-Fi, lettura
reale Beyond o modifica della raccolta personale.
