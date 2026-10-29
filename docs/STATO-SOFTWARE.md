# PocketD-D: stato e lavoro rimanente

Aggiornamento del **29 ottobre 2026 — v0.82**. Il dispositivo è il prodotto;
il PC serve per importare, configurare e conservare i backup.

Il software copre una prima sessione individuale offline. Il passaggio ancora
non verificato sul tuo hardware è **portare il PG personale sul dispositivo e
ritrovarlo dopo il riavvio**. L'avvio del firmware è già stato osservato da te;
non equivale al collaudo dell'importazione, della batteria o dei salvataggi.
I test USB rimandati restano aperti, senza essere considerati superati.

## Importazione da Beyond

| Area | Disponibile | Cosa manca o va controllato |
| --- | --- | --- |
| Accesso | Lettura pubblica diretta tramite connettore PocketD-D su PC | Personaggi privati/autenticazione, lettura autonoma dal dispositivo; nessuna scrittura verso Beyond |
| Creazione | Anteprima, problemi bloccanti distinti dai limiti, consenso e nuovo PG | Nessun aggiornamento automatico di una vecchia scheda con conservazione dei progressi |
| Caratteristiche | Base, modificatori/scelte riconciliati, bonus personalizzati, massimi espliciti, valori fissati e override supportati | Origini/capacità sostitutive, altre restrizioni/formule e override numerici non riconosciuti |
| PF e CA | Calcolati con le caratteristiche finali e gli effetti supportati; PF correnti e temporanei distinti | Sono fotografie all'importazione: cambiare equipaggiamento o salire di livello non ricalcola tutto |
| Abilità e salvezze | Competenze, maestrie, mezza competenza supportata, bonus, associazioni a caratteristiche e tiri da verificare; personalizzazioni standard | Abilità non standard, mezza competenza/maestria alle salvezze, bonus condizionali e formule |
| Armi | Configurazioni riconosciute: caratteristica, competenza, dadi, bonus, portata e alcune proprietà | Armi versatili/due armi, maestrie, munizioni e danni aggiuntivi speciali; arma attiva da scegliere se più armi sono equipaggiate |
| Inventario | Fino a 60 voci, quantità, custom, consumabili, cariche e cure esplicite; stati/peso/contenitore sorgente consultabili | Dotazioni come fotografie; niente gestione automatica di peso, contenitori o sintonia. Pile con saldi individuali e proprietà arbitrarie restano manuali |
| Magie e capacità | Fino a 120 voci, descrizioni, slot ordinari/patto, preparazione e contatori riconosciuti | Contatori manuali separati dagli utilizzi. Non tutti i recuperi o costi condivisi; magie/azioni da oggetto con usi restano note per evitare di duplicare le cariche; nessuna esecuzione generale degli effetti |
| Stato del PG | PF temporanei, annotazioni delle condizioni, dadi vita e contatori morte supportati | Le annotazioni non attivano automaticamente tutti gli effetti; concentrazione in corso e altri stati speciali da impostare/verificare |
| Difese ai danni | Concessioni incondizionate riconciliate, configurabili e trasferibili | Difese condizionali e immunità alle condizioni non sono ricostruite automaticamente; un tipo assente resta sconosciuto, non Normale |
| Monete | Cinque saldi distinti, compreso lo zero verificato | Nessun cambio, acquisto o peso automatico |

La conversione resta **parziale**. HTTP 200 prova l'accesso; JSON valido prova
il formato. Per dichiarare un dato convertito servono il controllo delle sue
origini/regole e il confronto del risultato. Una descrizione importata non
significa che il suo effetto sia automatizzato.

### Nuovo nella v0.82

[Sessione e preparazione del collaudo](SESSIONE-V082.md): una sequenza continua
con attacco, magia/concentrazione, danni, pozione, consumabile/capacità e riposi
ricarica tutti i campi a ogni salvataggio. Due interruzioni NVS simulate conservano
la copia precedente; la riprova della pozione non tira di nuovo.

Tasti configurabili con scelta diretta, icone e annullamento; identificativi
esistenti conservati. Controllo PC della scheda attuale con collegamenti agli
editor e resoconto Beyond originale conservato. Diagnosi firmware dal browser,
solo su richiesta, e report locale degli ultimi quattro tentativi senza JSON
PG/log integrali/seriali/credenziali. READY non viene confuso con la versione
firmware; diagnosi letta non equivale a importazione salvata.

293 test JS, suite nativa completa/127 viste, UI e sessione con ASan/UBSan
(leak detection non supportata), browser di editor/quota/limiti/report e build
ESP32. Nuove prove esclusivamente sintetiche: nessuna rilettura Beyond reale,
nessuna raccolta personale modificata. USB fisico, avvio Windows e ponte
Beyond autenticato restano aperti. Formati e partizioni invariati.

### Nuovo nella v0.81

[Guida dall’installazione al tavolo](INIZIA-QUI.md) nella stessa app locale:
Pocket → Personaggio → Al tavolo. Il pacchetto Windows x64 include runtime
Node ufficiale non modificato, licenza, app compilata e installer/firmware.
Niente installazione separata di Node o Python. Lo stesso indirizzo/origine
conserva la raccolta; un secondo avvio riutilizza soltanto la stessa versione.

Chi ha già il Pocket pronto salta il firmware; chi non ha il dispositivo può
preparare il PG e rimandare. Nella guida: anteprima Beyond sintetizzata, resoconti
richiudibili, scelte estetiche, consenso ai limiti e singolo pulsante di creazione,
poi invio USB e conferma sul display. Nessun completamento dopo il solo flash,
un file scaricato o un controllo porta. L’editor del solo aspetto conserva gli
altri campi. Errori di memoria non creano duplicati o avanzamenti fittizi.

288 test JS, prove browser guidate e regressioni della UI/USB simulate,
pacchetto estratto, identità/architettura SHA256 del runtime Windows e build
ESP32 verificati. La shell/esecuzione Windows non è eseguita nell’host Linux;
USB fisico e ponte Beyond autenticato restano aperti. Nessuna lettura o scrittura
Beyond reale in queste prove. Formati USB17/PC20/PDDa e partizioni invariati.

### Nuovo nella v0.80

[Revisione dell’uso pratico](USO-PRATICO.md): spada e magie direttamente dalla
home, Danni/Cura in prima riga, menu e ricerca con icone coerenti e pressione
lunga per l’aiuto. Filtri Consumabili e Pronte insieme alla ricerca, paging
con swipe nelle liste, senza attivare righe o conferme al rilascio. Le azioni
usano ancora i controlli di regole, anteprima e persistenza esistenti.

Sul PC: importa/personalizza → verifica/trasferisci → copie/collegamenti;
i prototipi sono sotto una sezione richiudibile. 283 test JS, 126 viste native,
input pointer LVGL, verifiche di memoria e browser. È una prova software con
dati sintetici: nessuna raccolta personale modificata, nessuna prova USB,
touch o Beyond autenticato aggiunta. I problemi fisici e i limiti della v0.79
restano aperti. Nessun nuovo campo nel PG: USB17/PC20/PDDa invariati.

### Nuovo nella v0.79

[Ponte Beyond sperimentale](PONTE-BEYOND.md): ESP32 → HTTPS LAN → coda privata
sul PC → estensione Chrome/Edge → pagina Beyond autenticata. Abbinamento con
certificato e conferma sul display; chiavi separate dal PG e dai backup.
Tiri già salvati, con facce/bonus/totali invariati, e PF attuali/temporanei;
nessun secondo lancio, nessun requisito Foundry. Pausa con conservazione della
coda, riconciliazione esplicita senza reinvio, revoca e storico degli esiti.

| Livello | Esito / limite |
| --- | --- |
| HTTPS e protocollo | Certificato, SAN IPv4, autenticazione, formato e storico C verificati localmente |
| Conversione dei tiri | Confronto facce, modalità, bonus e totale; minimo zero non rappresentabile bloccato |
| Browser | UI e adattatore PF/broker su pagina sintetica; worker con API Chrome simulate. Installazione MV3 bloccata dalla policy amministrativa del browser di sviluppo |
| Ricezione dei dadi | «Osservato» richiede eco corrispondente dal broker; persistenza Beyond dopo reload non provata |
| PF | «Verificato» richiede rilettura concordante dal connettore pubblico; nel test il servizio è sintetico |
| Hardware e account | Nessuna scrittura sul tuo Beyond, nessun collaudo RF/ESP32 o estensione sulla tua sessione |

La sincronizzazione **non è dichiarata pronta per la produzione**. L’adattatore
usa interfacce interne del sito, può richiedere aggiornamenti e riconosce la
gestione PF inglese supportata. Non sono sincronizzati PF massimi, condizioni,
inventario, slot o utilizzi. La direzione inversa resta successiva. Restano
aperti il problema USB già segnalato, le prove Wi-Fi fisiche e il test controllato
su una scheda Beyond di prova. Raccolta personale e account non modificati.
USB17 / backup PC v20 / stato nativo PDD36-PDDa invariati.

### Nuovo nella v0.78

[Profilo Beyond della prima versione](BEYOND-V1.md): consultazione sul dispositivo
con identità/classi, movimento base, sensi/passivi, lingue, strumenti e dotazioni;
tiro di iniziativa con bonus riconciliato e contesto attuale. Incertezza distinta
dallo zero. Duplicati di azioni/magie non creano riserve doppie. Resoconto per PG
con regole manuali, fotografie ed eccezioni specifiche; estratti accettati.
USB17 / backup PC v20 quando c’è il profilo, PDD36 senza crescita dello stato.

[Wi-Fi](WIFI.md): rete temporanea protetta, configurazione dal telefono o PC,
salvataggio dopo connessione verificata, spegni/dimentica e sospensione della radio.
Nessuna credenziale nel PG. Prove software del servizio, non della radio fisica.
[Tempo reale](BEYOND-TEMPO-REALE.md): ricerca e progetto del ponte opzionale;
ponte sperimentale implementato nella v0.79; animazione Beyond non implementata.

La nuova lettura reale conferma i valori v0.77 sotto riportati. Aggiunge 14 pagine
di profilo (64 voci complessive), tutte confrontate nel parser/codec C: passo/volo
30 ft con restrizione del volo, scurovisione 60 ft, passivi 14/10/14 e iniziativa +4.
I dati di dotazione non disponibili restano `?`; azioni custom ed effetti generici
restano manuali. Nessuna raccolta personale modificata e nessun trasferimento USB.

### Nuovo nella v0.77

[Creazione del cannone con slot](BEYOND-CANNONE.md) nella variante riconosciuta
Artillerist di Tasha’s: prima un utilizzo gratuito per riposo lungo, poi uno
slot ordinario o patto scelto e confermato. La spesa non ripristina il gratuito,
non modifica i saldi manuali e non termina la concentrazione. PC e dispositivo
conservano i valori precedenti se il salvataggio fallisce.

Nomi simili e varianti non riconciliate non ricevono questa automazione.
Scelta del cannone attivo, durata, PF, modalità e detonazione restano manuali.
Richiede firmware 0.77 / USB16, backup PC v19; PDD35/PDDZ a dimensioni invariate.

### Disponibile dalla v0.76

[Contatori manuali delle azioni](BEYOND-CONTATORI-MANUALI.md) separati dagli
utilizzi: residui conservati e modificabili sul PC e sul dispositivo, senza
consumo o recupero automatico. Nel PG reale cinque saldi prima presentati
come utilizzi vengono distinti dalla creazione del cannone con recupero lungo.
In questa versione storica non erano automatizzati collegamenti o spese di slot;
la v0.77 aggiunge il costo della creazione riconosciuta, mantenendo manuali gli altri casi.
Backup PC v18 solo con questi contatori; USB e stato nativo invariati.

**Testi estesi rimandati per scelta dell’utente:** i tagli attuali sono accettati
per questa fase e non richiedono espansione su SD per chiudere l’importazione.

### Disponibile dalla v0.75

[Verifica rapida delle difese](DIFESE-VERIFICA-RAPIDA.md) sul dispositivo e sul PC:
anteprima dei tipi mancanti e conferma manuale per dichiararli Normali di base.
Le difese note, regole ed effetti temporanei restano conservati. Annullamento,
errore di salvataggio e riprova non applicano modifiche parziali. Il collegamento
nell’anteprima USB apre direttamente la configurazione sul PC.
Nessun tipo viene completato automaticamente durante l’importazione.

### Disponibile dalla v0.74

[Caratteristica personalizzata per abilità](ABILITA-CARATTERISTICHE.md), importabile
da Beyond e configurabile sul PC: per esempio Intimidire con FOR. La scelta
arriva al dispositivo e segue punteggi ed effetti supportati senza alterare
competenza o extra. Richiede firmware 0.74 / USB15 quando presente.
Annullamento ed errore di salvataggio dell’editor conservano la scheda.

### Disponibile dalla v0.73

Personalizzazioni standard delle abilità e salvezze: bonus numerici separati,
competenze esplicite e mezza competenza delle abilità. Un valore esplicito
Nessuna può rimuovere la competenza derivata dalla classe; Vuoto/Automatico
la conserva. Duplicati identici non si sommano e duplicati discordanti bloccano.
La somma finale degli extra deve restare fra −30 e +30.

L'anteprima mostra anche bonus su tiri senza competenza, gradi rimossi e mezza
competenza. Le personalizzazioni non eliminano blocchi causati da altri effetti
irrisolti. Oggetti a quantità zero o inattivi non forniscono bonus ai tiri;
origini o requisiti ambigui mantengono i tiri interessati da verificare.
[Formato, casi supportati e prove](BEYOND-PERSONALIZZAZIONI-TIRI.md).

### Ultima lettura reale del tuo PG

28 ottobre 2026, personaggio **144777811**, con codice v0.77 e connettore esistente:

- HTTP **200**, **426.718 byte**, struttura JSON valida.
- FOR/DES/COS/INT/SAG/CAR: **8/18/14/18/10/10**; PF **32/73**, CA **17**.
- **41 voci** d'inventario; le quantità della proiezione coincidono nel parser C.
- Quattro pozioni di cura con formula esplicita **2d4+2**.
- I **30 tiri** di caratteristica/abilità/salvezza della proiezione coincidono
  fra modello PC e calcolo C; nessuno risulta bloccato in questo sorgente.
- Cinque contatori manuali coincidono nel parser/codec C: **4/4, 1/4, 50/50,
  35/50, 50/50**. Create Eldritch Cannon è **0/1**, recupero lungo; la nuova
  creazione con uno slot ordinario coincide nel modello PC e nel calcolo C.
  La prova usa una copia temporanea: nessun saldo personale salvato.
- Nessun bersaglio con le personalizzazioni delle abilità/salvezze: per quei casi
  la prova rimane **sintetica**, non dimostrata da questo PG.

È una verifica dei dati ricevuti e della conversione, non un confronto con una
schermata autenticata Beyond. Il parser e il codec C sono stati eseguiti sul PC:
non provano il trasferimento fisico. Nessuna raccolta personale aperta o
modificata, nessun invio USB; il JSON completo non è pubblicato né negli archivi.

## Funzionamento sul dispositivo

| Funzione | Stato del software | Limite pratico |
| --- | --- | --- |
| Home, avatar, menu e dadi | Implementati; dadi animati, risultati e storico | Leggibilità, fluidità e consumo sulla scheda da collaudare con il PG reale |
| Scheda e tiri | Caratteristiche, 18 abilità, sei salvezze, bonus e modalità | Le regole non rappresentate richiedono verifica o dado libero |
| PF, cure e temporanei | Inserimento importi, anteprima/conferma, salvataggio e annullamento | Per un colpo con difese e concentrazione usare Danni, non la correzione diretta dei PF |
| Danni, effetti e turni | Difese/configurazione locale, durate, riduzioni e costi espliciti | Non è un motore completo D&D; contesto, fonti e regole speciali possono richiedere decisioni del giocatore/DM |
| Inventario e risorse | Ricerca, quantità, arma attiva, consumi, cariche e ricariche | Nuove voci/configurazioni strutturali si preparano dal PC; i testi degli oggetti non diventano effetti |
| Magie, concentrazione e riposi | Slot ordinari/patto, preparazione, utilizzi, dadi vita e recuperi supportati | Effetti, danni delle magie, eccezioni e recuperi speciali restano da gestire |
| Condizioni e morte | Annotazioni, effetti espliciti sui tiri, contatori e regole base | I nomi delle condizioni non attivano automaticamente tutti gli effetti |
| Tasti | PWR breve, BOOT breve/lungo configurabili; protezione delle conferme | Debounce e comportamento fisico da provare; le preferenze locali sono conservate all'importazione |
| Salvataggi e backup | Persistenza NVS, storico, copia/backup/ripristino USB | Mancano prove fisiche dopo riavvio, perdita di alimentazione e ripristino; backup PC e nativo sono archivi distinti |
| Alimentazione | Lettura PMIC/batteria, luminosità, pausa/risveglio e spegnimento con salvataggio implementati | Autonomia, calibrazione, ricarica e risveglio non misurati sul tuo esemplare |

Attenzione pratica: il PG attuale non dichiara un profilo completo di difese
verificate. Prima di usare il calcolo Danni vanno verificati i tipi richiesti
ed eventualmente dichiarati Normali. Questa verifica si può fare dal PC o
sul dispositivo; non viene inventata dall'assenza di modificatori Beyond.

Si gestisce **una scheda attiva sul dispositivo**. La raccolta di più PG resta
sul PC. Inviare un nuovo PG e confermarlo sostituisce la copia attiva e il suo
stato locale; le schede PC restano conservate. Prima di sostituire una copia
con progressi importanti va acquisito il backup nativo.

Party/griglia/sincronizzazione fra dispositivi non fanno parte dell'attuale
uso offline; gli strumenti PC relativi sono prototipi locali.

## Grafica e personalizzazione

Disponibili tre ambienti, avatar originale configurabile, **21 famiglie visive**,
due silhouette e **504 look di classe** su SD. Beyond propone specie riconosciuta
e look di classe singola supportata; nei casi ambigui la scelta è manuale.

Restano espressioni/fotogrammi aggiuntivi dei look SD, Artificiere/sottoclassi,
altre illustrazioni/scene e rifiniture di icone e transizioni. I preset SD hanno
colori/taglio propri; lo stile originale conserva le proprie scelte. Il pacchetto
SD è opzionale e la lettura del lettore fisico resta da verificare: lo spazio
SD non sostituisce il controllo di RAM, fluidità e salvataggi sul dispositivo.

## La roadmap in quattro blocchi

| Blocco originale | Stato al 28 ottobre | Lavoro residuo |
| --- | --- | --- |
| 1. Completare la scheda sul dispositivo | Coperto per il primo uso: magie/slot, utilizzi, concentrazione, riposi, dadi vita e morte sono presenti | Eccezioni, completezza dell'import e verifica dei percorsi con il PG reale |
| 2. Gestire effetti e regole | Parziale: diversi effetti sono configurabili localmente; l'import sa ricostruirne solo un sottoinsieme | Regole condizionali, proprietà speciali, ricalcolo e collegamenti automatici alle fonti |
| 3. Prodotto fisico | Funzioni implementate e compilate; avvio osservato dall'utente | Import USB/riavvii, touch/tasti/SD, batteria, prestazioni e tenuta dei salvataggi |
| 4. Estetica | Base del compagno e selezione di specie/classi disponibili | Espressioni dei nuovi look, ulteriori sprite/scene e rifiniture dopo visione sul display |

Non assegno una percentuale unica: conterebbe allo stesso modo una schermata
compilata e un trasferimento fisico che oggi non è ancora confermato.

## Ordine consigliato per arrivare a una prima sessione

1. **Sbloccare/verificare USB sul dispositivo.** Aggiornare l'app PC, eseguire
   prima la diagnostica in sola lettura; poi un invio con anteprima/conferma
   e risposta finale. I precedenti messaggi di avvio e il ritorno a Elian non
   sono una conferma di salvataggio. Quando avrai tempo ripartiremo da qui.
2. **Confrontare il PG sul display:** caratteristiche, PF, CA, arma attiva,
   quantità, pozioni, slot e tiri principali; configurare difese e tasti.
3. **Verificare i progressi:** modificare un saldo, riavviare, ritrovarlo,
   acquisire un backup e provarne il ripristino con un caso controllato.
4. **Provare una breve sessione:** un attacco, danni, cura, magia/concentrazione,
   spesa/recupero di una risorsa e un riposo. Poi correggere gli attriti osservati.
5. **Ampliare l'import** sui casi concreti rimasti e completare gli altri dati
   consultabili sul dispositivo, poi continuare espressioni e rifiniture grafiche.
   I testi possono restare negli estratti attuali, come concordato.

Non serve attendere tutte le regole D&D per questa prima sessione. Servono un
trasferimento verificato, valori confrontati e progressi recuperabili.

## Verifiche di questa versione e compatibilità

**258 test JS**, browser di produzione con profili isolati, **324 schede
sintetiche** nel parser/codec C per le nuove associazioni, undici pacchetti
incompatibili respinti e regressioni dell'intera scheda. Parser/codec e UI host verificati anche
con ASan/UBSan. Compilazione ESP32-S3 riuscita. Tutti gli **8.192 sottoinsiemi**
delle difese verificati in JS e C; **120 schede sintetiche** per i nuovi contatori,
**108 sorgenti / 2.052 scelte di pagamento** per la nuova creazione e **119 viste**
del renderer. Questi risultati
non certificano touch, trasporto USB o memoria fisica.

Compatibilità: **USB16 / backup PC v19** per il costo della creazione del cannone,
con app e firmware 0.77. Le associazioni delle abilità richiedono almeno USB15 /
v17; i contatori manuali almeno app PC 0.76 / v18. Senza i campi nuovi si conserva
il minimo formato precedente pertinente.
Stato nativo **PDD35/PDDZ**, dimensioni e partizioni invariate, lettura dei formati
precedenti. Per leggere/ripristinare i backup nativi nuovi usare app e firmware 0.77.
Il profilo difese richiede almeno 0.71 / USB14; i look classe almeno 0.70 e il
pacchetto SD. Completa sul dispositivo richiede 0.75. L’installer distribuisce 0.77.
I testi rimangono negli estratti concordati.
