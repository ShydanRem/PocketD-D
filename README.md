# PocketD-D

Un compagno tascabile per le sessioni di gioco di ruolo: il proprio eroe in pixel art, una scheda interattiva e dadi animati su un dispositivo AMOLED rotondo.

**Stato:** prototipo web interattivo **v0.10**, con raccolta di personaggi, inventario e armi personalizzabili. Il firmware ESP32 non è ancora implementato.

Hardware previsto: Waveshare **ESP32-S3-Touch-AMOLED-1.75**, display touch rotondo da 466 × 466 pixel. La descrizione del produttore fornita dall’utente indica 8 MB di PSRAM, 16 MB di flash e slot microSD integrato, con controller CO5300 e touch CST9217. Pin, driver e prestazioni restano da verificare sugli esempi ufficiali e sul dispositivo.

- [Proposta di prodotto e roadmap](docs/PROGETTO.md)
- [Architettura e verifiche tecniche](docs/ARCHITETTURA.md)

## Primo traguardo

Un personaggio dimostrativo interamente originale, un avatar animato, punti ferita modificabili e un tiro di d20 con modificatore. Prima validare l'interazione su una simulazione del display, poi sul dispositivo reale.

## Avvio del prototipo

Richiede Node.js 22.12+ (verificato con Node 24) e npm.

```sh
cd /workspace/PocketD-D
npm ci
npm run dev -- --port 5173
```

```sh
npm test
npm run build
```

La demo include cinque sezioni (Eroe, Scheda, Dadi, Azioni, Zaino), avatar personalizzabile, abilità e salvezze, armi personalizzabili, inventario estensibile, risorse manuali e griglia tattica. La raccolta conserva fino a 20 personaggi indipendenti, con selezione, creazione, duplicazione e backup. “Ricomincia” ripristina soltanto il personaggio attivo. I dati rimangono nel browser: esporta la raccolta per conservarla o trasferirla. Nessun account richiesto. I font remoti sono opzionali e hanno alternative locali. La demo non è una PWA installabile e non garantisce il riavvio offline del browser.

Il layout riproduce l'area circolare, ma non dimostra leggibilità o prestazioni sul display reale. Non include Beyond, regole complete o sincronizzazione. Avvelenato applica svantaggio ad attacchi, prove e abilità; le salvezze e il dado libero mantengono la modalità selezionata.

La scelta della toolchain firmware verrà confermata dopo la verifica degli esempi ufficiali della scheda.


## Incremento 0.2 — azioni e zaino

- Cinque sezioni: Eroe, Scheda, Dadi, Azioni e Zaino.
- Spada equipaggiabile: tiro +4 contro CA impostabile, 1 naturale manca e 20 naturale colpisce; danni 1d8+2 o 2d8+2 al critico. Il DM applica i danni al bersaglio.
- Pozioni 2d4+2 con consumo e limite ai PF massimi; torce e risorsa narrativa originale Slancio.
- Riposo esplicito con conferma: recupera PF e Slancio, conserva condizioni e consumi. È una scorciatoia dimostrativa, non un motore completo dei riposi D&D.
- Backup JSON esportabile/importabile con validazione, conferma e compatibilità con il precedente salvataggio v1. Nessuna password o dato Beyond.
- `npm run build` aggiorna anche `PocketDD-demo.html`, il file autonomo da scaricare e aprire nel browser. I font remoti sono opzionali.

Verificati otto test del modello e flussi Chromium: consumo/annullamento, equipaggiamento, riposo, backup valido/non valido, persistenza, attacco/danni, viewport desktop e mobile 390/320 px. Verificato anche il file autonomo servito senza accesso ad asset esterni. Display fisico ancora da verificare.

## Incremento 0.3 — creazione manuale e avatar

“Personalizza eroe” apre l'editor dell'app di supporto; “Nuovo eroe” prepara un nuovo profilo e richiede conferma prima di sostituire quello attivo. Nome, titolo, livello, sei caratteristiche, PF massimi e CA sono modificabili. Carnagione, colore e taglio dei capelli e mantello hanno tre varianti ciascuno, con anteprima. Non è ancora un editor che disegna singoli pixel né un creatore completo di classi, magie e capacità.

La scheda e la spada usano modificatori e competenza derivati dal profilo. I PF massimi e la CA restano manuali. Il [formato v3](docs/FORMATO-SCHEDA.md) conserva aspetto e profilo e importa i vecchi backup v1/v2.


## Incremento 0.4 — abilità, salvezze e storico

La sezione Scheda offre Caratteristiche, Abilità, Salvezza e Storico. L'editor permette di indicare competenza o maestria nelle 18 abilità e competenza nei sei tiri salvezza. Il selettore è manuale: non valida le scelte rispetto a una classe o a un regolamento specifico.

I dadi mostrano caratteristica, contributo della competenza e modalità effettiva; i bonus derivati non sono modificabili con i pulsanti del dado libero. Lo storico conserva gli ultimi 20 tiri nel browser e nel backup v4, compatibile in lettura con v1/v2/v3. Restano da implementare magie, capacità di classe, regole complete e firmware.

Verifica dell'incremento: 16 test del modello, compilazione, flussi Chromium su desktop e mobile 390/320 px, competenze/maestria, effetti di avvelenato, storico dopo riavvio, export/import e HTML autonomo senza asset esterni.


## Incremento 0.5 — magie e capacità manuali

Da **Gestisci magie e capacità** nell'app di supporto puoi aggiungere, modificare ed eliminare fino a 40 voci con note personali, configurare i nove livelli di slot e scegliere la caratteristica da incantatore. Da **Azioni → Magie e capacità** puoi consultare le note e consumare slot/utilizzi.

La gestione include trucchetti, slot di livello superiore, concentrazione unica, annullamento e recuperi distinti per riposo breve/lungo/manuale. Il backup v5 comprende tutte le risorse e legge le versioni precedenti. L'app non fornisce un catalogo proprietario e non esegue automaticamente gli effetti delle magie o le regole delle classi. Le risorse del vecchio personaggio non vengono inventate durante la migrazione: configura quelle corrette manualmente.

`npm test` esegue anche i test del modulo risorse `src/powers.js`. `npm run build` rigenera il file HTML autonomo.

Verifica 0.5: 23 test del modello passati e build completata. In Chromium, sul file autonomo senza asset esterni, verificati creazione/modifica/eliminazione delle voci, consumi, rifiuto di slot non validi, sostituzione della concentrazione con conferma o annullamento, riposi breve/lungo, annullamento, riavvio, export/import e conservazione delle risorse quando cambia il profilo. Verificati viewport desktop e mobile 390/320 px, senza errori JavaScript.


## Incremento 0.6 — combattimento locale

**Prepara combattimento** aggiunge il tuo eroe collegato alla scheda e altri partecipanti manuali (massimo 12). **Azioni → Turni** gestisce iniziativa, turni, round e PF. Roster e iniziativa si modificano dopo aver terminato il combattimento, conservando i PF correnti.

**Azioni → Arma** permette di scegliere un bersaglio e usare la sua CA. Il tiro danni può essere applicato esplicitamente una sola volta, con stato conservato dopo riavvio e nel backup v6. Rimangono disponibili annullamento e CA manuale. Resistenze e regole speciali restano al DM. Non sono ancora presenti griglia o collegamento tra dispositivi.

Verifica 0.6: 30 test del modello passati, build completata e flusso browser sul file autonomo: preparazione, iniziativa, round, PF eroe sincronizzati, bersaglio/CA, danni, rifiuto di seconda applicazione dopo reload, annullamento, export/import v6, blocco roster durante il combattimento e mobile 390/320 px. Nessun errore JavaScript rilevato.


## Incremento 0.7 — prima griglia tattica

1. Aggiungi i partecipanti da **Prepara combattimento**, indicando anche il movimento per turno in caselle.
2. Apri **Prepara mappa**: scegli un partecipante e tocca una cella per piazzarlo, oppure usa lo strumento muri.
3. Avvia il combattimento da **Azioni → Turni**, poi passa a **Mappa**. Le celle evidenziate sono raggiungibili dal partecipante attivo; toccale per muovere. Tocca una pedina per identificarla.

La griglia locale 9 × 9 calcola percorsi e consumo di movimento, evitando muri, altre pedine e diagonali attraverso angoli bloccati. Il budget si rinnova a ogni turno. Posizioni, ostacoli e movimento sono salvati nel browser e nel backup v7, compatibile in lettura con v1–v6. L'annullamento recupera la posizione precedente.

Verifica 0.7: 38 test passati, build completata e flusso Chromium su desktop e mobile 390/320 px, incluso HTML autonomo: piazzamenti, muri, rifiuto sovrapposizioni, movimento, ispezione pedine, annullamento, cambio turno, persistenza, export/import e blocco dell'editor durante il combattimento. Nessun errore JavaScript rilevato.

Le caselle sono piccole: precisione touch e leggibilità sul display fisico non sono ancora verificate. Zoom e controlli geometrici sono aggiunti nell’incremento 0.8; multiplayer resta da implementare; la mappa non è ancora un motore completo di combattimento.


## Incremento 0.8 — zoom e assistenza agli attacchi

Nella mappa, il pulsante **＋** passa alla vista 5 × 5; le frecce spostano la finestra e **◎** ricentra. Il movimento continua a usare le coordinate della mappa completa. Tocca una pedina per selezionarla e usa il pulsante dell'arma per aprire l'attacco.

La vista Arma mostra distanza, portata e presenza di muri sulla linea tra l'eroe e il bersaglio. Un controllo negativo richiede l'override esplicito del DM per un singolo tiro, registrato nello storico. Se mancano le posizioni, il controllo resta manuale. **Personalizza eroe** permette di impostare la portata dell'arma; il backup v8 conserva il valore e legge v1–v7.

Verifica 0.8: 43 test passati (incluse linee geometriche in tutte le direzioni), compilazione e flusso browser dell'HTML autonomo su desktop e mobile 390/320 px. Verificati zoom, bordi, ricentramento dopo movimento, coordinate globali, selezione bersaglio, portata, muri, override consumato/non persistente, modifica della portata e backup. Nessun errore JavaScript rilevato. La precisione touch sul dispositivo fisico resta da verificare.

## Incremento 0.9 — raccolta personaggi

Il selettore sopra il dispositivo cambia il personaggio attivo. **Gestisci personaggi** apre la raccolta con anteprime, livello, PF e combattimento in corso.

- **Nuovo personaggio** aggiunge un eroe senza sostituire quelli esistenti.
- **Duplica** crea una copia indipendente dell'intera scheda e sessione, utile anche prima di fare esperimenti.
- **Elimina** richiede conferma; l'ultimo personaggio resta protetto. L'eliminazione non è annullabile.
- **Aggiungi da backup** legge le schede v1–v8 e le aggiunge come nuovi personaggi, anche se il nome è già presente.
- **Esporta scheda** conserva il formato individuale v8; **Esporta raccolta** include tutti i personaggi e la selezione attiva. **Ripristina raccolta** sostituisce tutta la raccolta dopo validazione e conferma.

Ogni personaggio conserva PF, avatar, competenze, inventario, magie/capacità, concentrazione, storico, partecipanti, turni e mappa del proprio combattimento locale. Un cambio personaggio azzera risultati temporanei, bersaglio selezionato, override DM e annullamento. Non permette di spostare un tiro in corso su un altro eroe e non costituisce un party condiviso.

Il salvataggio v0.8 viene migrato automaticamente nella nuova raccolta senza cancellare la vecchia chiave. Dati corrotti non vengono sovrascritti: la sessione resta temporanea e permette di scaricare i dati originali. Anche con memoria piena o salvataggio negato si può esportare la raccolta in memoria. Una scheda del browser già non aggiornata non può sovrascrivere il salvataggio più recente di un'altra scheda: viene chiesto di esportare e ricaricare.

Verifica 0.9: **54 test passati**, build riuscita, flussi Chromium desktop/mobile 390/320 px e regressione del combattimento v0.8 passati, senza errori JavaScript. Lo script ripetibile `python scripts/check-library.py` richiede Playwright Python, Chromium in `/usr/bin/chromium` e una build aggiornata; non richiede un server o asset esterni.


## Incremento 0.10 — inventario e armi personalizzabili

Da **Gestisci inventario e armi**, nel footer o nello **Zaino**, puoi creare, modificare ed eliminare fino a 60 oggetti per personaggio. Ogni voce ha nome, note e quantità (0–999). Sono disponibili quattro tipi:

- **Oggetto:** quantità e note per l'equipaggiamento generico.
- **Consumabile:** il comando Usa consuma una unità; gli effetti restano al DM.
- **Cura:** configura 1–6 dadi d4/d6/d8/d10/d12 e un bonus 0–30. L'uso consuma una unità e recupera PF entro il massimo; a PF pieni non consuma nulla.
- **Arma:** scegli caratteristica, competenza al tiro per colpire, dadi di danno, bonus extra separati, contributo della caratteristica ai danni, portata e tipo di danno come nota. Si equipaggia una sola arma disponibile alla volta.

**Azioni → Arma** usa l'arma equipaggiata e mostra la provenienza dei bonus. Il critico raddoppia tutti i dadi dell'arma, lasciando singoli i bonus. I danni non scendono sotto zero e restano applicabili al bersaglio solo con conferma. Distanza, muri e override DM usano la portata dell'arma. Modificare inventario/equipaggiamento invalida i risultati e i danni in attesa; quantità zero, eliminazione o cambio tipo ripongono automaticamente l'arma. L'annullamento ripristina l'ultima modifica.

Il backup individuale passa al **formato v9**, letto dalla v0.10 e successive. Le schede v1–v8, comprese quelle nelle raccolte della v0.9, vengono migrate: spada, quantità delle pozioni e torce e portata precedente sono conservate. Il formato della raccolta rimane v1 e contiene ora schede v9. Il vecchio salvataggio individuale resta intatto. I riposi conservano inventario e consumi.

Questo incremento non automatizza armature/CA, peso, monete, munizioni, proprietà speciali delle armi o resistenze ai tipi di danno. Le definizioni sono manuali e non includono un catalogo esterno.

Verifica 0.10: **66 test passati**, build e controlli Chromium dell'inventario e della raccolta su desktop/mobile 390/320 px riusciti, senza errori JavaScript. Verificati migrazione, CRUD, quantità, consumi/cure, critici deterministici, portata/muri, override DM, danni applicati una volta, annullamento, backup v9, isolamento tra personaggi e inventario vuoto. `python scripts/check-inventory.py` ripete il flusso; richiede gli stessi strumenti di `check-library.py`.
