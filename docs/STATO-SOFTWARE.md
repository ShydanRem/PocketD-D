# PocketD-D: stato e lavoro rimanente

Aggiornamento del **28 ottobre 2026 — v0.74**. Il dispositivo è il prodotto;
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
| Inventario | Fino a 60 voci, quantità distinte, custom riconciliati, consumabili, cariche e pozioni con cura esplicita | Peso, contenitori, gestione automatica della sintonia, pile con cariche individuali, proprietà arbitrarie; note degli oggetti conservate sul PC |
| Magie e capacità | Fino a 120 voci, descrizioni, slot ordinari/patto, preparazione e contatori riconosciuti | Non tutti i recuperi o costi condivisi; magie/azioni da oggetto con usi restano note per evitare di duplicare le cariche; nessuna esecuzione generale degli effetti |
| Stato del PG | PF temporanei, annotazioni delle condizioni, dadi vita e contatori morte supportati | Le annotazioni non attivano automaticamente tutti gli effetti; concentrazione in corso e altri stati speciali da impostare/verificare |
| Difese ai danni | Concessioni incondizionate riconciliate, configurabili e trasferibili | Difese condizionali e immunità alle condizioni non sono ricostruite automaticamente; un tipo assente resta sconosciuto, non Normale |
| Monete | Cinque saldi distinti, compreso lo zero verificato | Nessun cambio, acquisto o peso automatico |

La conversione resta **parziale**. HTTP 200 prova l'accesso; JSON valido prova
il formato. Per dichiarare un dato convertito servono il controllo delle sue
origini/regole e il confronto del risultato. Una descrizione importata non
significa che il suo effetto sia automatizzato.

### Nuovo nella v0.74

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

28 ottobre 2026, personaggio **144777811**, con codice v0.73 e connettore esistente
(nessuna nuova lettura reale nella v0.74):

- HTTP **200**, **426.718 byte**, struttura JSON valida.
- FOR/DES/COS/INT/SAG/CAR: **8/18/14/18/10/10**; PF **32/73**, CA **17**.
- **41 voci** d'inventario; le quantità della proiezione coincidono nel parser C.
- Quattro pozioni di cura con formula esplicita **2d4+2**.
- I **30 tiri** di caratteristica/abilità/salvezza della proiezione coincidono
  fra modello PC e calcolo C; nessuno risulta bloccato in questo sorgente.
- Nessun bersaglio con le nuove personalizzazioni dei tiri: per quei casi la
  prova di questa versione è **sintetica**, non dimostrata da questo PG.

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
5. **Ampliare l'import** sui casi concreti rimasti e completare l'informazione
   consultabile sul dispositivo, poi continuare espressioni e rifiniture grafiche.

Non serve attendere tutte le regole D&D per questa prima sessione. Servono un
trasferimento verificato, valori confrontati e progressi recuperabili.

## Verifiche di questa versione e compatibilità

**246 test JS**, browser di produzione con profili isolati, **324 schede
sintetiche** nel parser/codec C per le nuove associazioni, undici pacchetti
incompatibili respinti e regressioni dell'intera scheda. Parser/roundtrip e
UI host anche con ASan/UBSan. Compilazione ESP32-S3 riuscita. Questi risultati
non certificano touch, trasporto USB o memoria fisica.

Nuovi dati opzionali: **USB15 / backup PC v17** con associazioni personalizzate;
senza di esse si conserva il minimo precedente pertinente. Stato nativo
**PDD34/PDDY**, lettura dei formati precedenti e partizioni invariate.
Per trasferire la nuova informazione e leggere i nuovi backup nativi aggiornare
app e firmware a 0.74. Il profilo difese richiede almeno 0.71 / USB14; i look
classe almeno 0.70 e il relativo pacchetto SD. L'installer distribuisce 0.74.
