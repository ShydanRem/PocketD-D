# Importazione Beyond e conferma — v0.82.1

29 ottobre 2026. Correzione PC; compatibile con il firmware 0.82 già installato.
Non occorre riflashare per correggere questa importazione. Il firmware incluso
nel pacchetto ha soltanto l’identificativo aggiornato, con USB17 e gli stessi
formati/partizioni.

## Causa riprodotta sul personaggio reale

La lettura del personaggio 167361077 risponde HTTP 200, 630.759 byte iniziali,
JSON valido. La v0.82 blocca la conversione per due motivi distinti:

- una regola `replace-weapon-ability / charisma-score` viene classificata come
  modificatore del punteggio del PG e richiede erroneamente l’origine di un ASI;
- personalizzazioni di un’arma nella lista `characterValues` bloccano tutta la
  scheda come override non riconciliati.

La casella era disabilitata perché mancava una scheda convertibile, non perché
la spunta richiedesse un clic particolare. Il motivo era troppo lontano dalla
conferma. Ora i problemi bloccanti sono immediatamente sotto i valori del PG;
in presenza di blocchi la conferma viene sostituita da una spiegazione, senza
mostrare una casella disabilitata apparentemente guasta.

## Comportamento corretto e limite esplicito

La sostituzione della caratteristica dell’arma non aumenta Carisma o altri
punteggi. Le personalizzazioni d’arma 10/12/20 vengono separate soltanto quando
il bersaglio è un oggetto Weapon univoco, il namespace è quello degli oggetti
e i valori sono del tipo ammesso. Bersagli ambigui, namespace sbagliati, valori
malformati e override sconosciuti rimangono bloccanti.

Le armi interessate sono conservate **come oggetti, senza tiri automatici**.
Non viene indovinato se una personalizzazione sostituisca o aggiunga un bonus,
né quale arma sia legata al patto. In presenza della regola di sostituzione
della caratteristica, tutte le armi sono da verificare. Configurale in Gestisci
inventario e armi, confrontando caratteristica, competenza, dadi e bonus con
Beyond; poi scegli l’arma attiva. Le armi ordinarie non interessate mantengono
la conversione esistente.

La creazione richiede ancora una bozza valida, `canImport` vero, assenza di
blocchi e consenso esplicito. Una risposta incoerente senza bozza non abilita
il comando. La casella e la sua etichetta rimangono cliccabili nelle viste
classica e guidata; il pulsante guidato non si sovrappone più ai controlli.

## Prove reali e sintetiche

**Lettura reale**, ripetuta con il connettore PocketD-D in sola lettura; sorgente
completo mantenuto solo in memoria. Nessuna modifica a Beyond o alla raccolta.
Nel replay dopo la correzione:

- FOR/DES/COS/INT/SAG/CAR: 15/10/16/8/8/17; PF 35/53; CA 16, confermati
  dall’utente sulla sua scheda Beyond.
- PF: base 35 + contributo COS 18 = massimo 53; 18 rimossi = attuali 35.
- Undici voci d’inventario conservate nel parser e nel codec C; entrambe le
  armi rimangono oggetti, nessun attacco inventato o arma attiva automatica.
- I trenta bonus di prova/abilità/salvezza e i relativi blocchi coincidono
  tra modello PC e parser/calcolo C.
- Anteprima del sorgente reale riprodotta in browser a 1280×700 e 320×600:
  consenso cliccabile e creazione abilitata. Il comando Crea non viene premuto
  in questa prova reale; non è un’importazione USB fisica.

**Prove sintetiche:** 297 test JavaScript, casi di sostituzione caratteristica,
personalizzazioni invalide/ambigue/sconosciute, campi non impostati e protezione
del marcatore interno. Browser su risposte valide, bloccate e incoerenti:
clic reale su etichetta e casella, rilettura, creazione unica, schede precedenti
conservate, entrambe le modalità a 1280/390/320 pixel. Build ESP32 verificata;
nessuna modifica al codice nativo delle regole o dell’USB.

I limiti estesi di Beyond restano quelli del resoconto della scheda. La correzione
consente una creazione parziale consapevole; non dichiara l’automazione delle
armi speciali né la riuscita del trasferimento sul dispositivo.

## Aggiornare solo l’app PC

Chiudi la vecchia finestra del programma PocketD-D, scarica il pacchetto PC
aggiornato, estrailo in una nuova cartella e apri Avvia-PocketDD.cmd. Usa lo
stesso browser e indirizzo locale per conservare la raccolta. Nella guida,
Mostra già l’eroe → continua salta il firmware. Rileggi il link Beyond,
controlla le omissioni, accetta e crea la nuova scheda. L’invio USB viene dopo.
