# Beyond: punteggi fissati e massimi aumentati — v0.72

> Aggiornamento v0.73, 28 ottobre: [personalizzazioni standard dei tiri](BEYOND-PERSONALIZZAZIONI-TIRI.md)
> e [report generale con roadmap e limiti](STATO-SOFTWARE.md). Nessun nuovo formato;
> nuova lettura reale distinta dalle prove sintetiche; verifica USB fisica ancora aperta.


27 ottobre 2026. Un nuovo blocco dell'importazione PC: caratteristiche fissate
(per esempio da un oggetto), aumenti espliciti del massimo e ordine corretto
dei bonus personalizzati. Nessuna dipendenza Foundry, scrittura verso Beyond
o modifica alla raccolta personale. Il PC prepara la scheda per il dispositivo.

## Regole supportate

Per ciascuna delle sei caratteristiche, in questo ordine:

1. Sommare al valore base i modificatori numerici `bonus / <ability>-score`
   attivi e riconciliati.
2. Applicare il massimo ordinario 20, aumentato dai modificatori espliciti
   `bonus / ability-score-maximum` con `statId` da 1 a 6. Massimo locale 30.
3. Aggiungere `bonusStats`, cioè il bonus personalizzato Beyond, **dopo** il
   limite. La versione precedente lo sommava prima del limite e poteva
   troncarlo, oppure bloccarlo quando il totale superava 20.
4. Applicare il maggiore dei punteggi fissati `set / <ability>-score`, solo
   se supera il risultato precedente. Un valore fissato a 19 non abbassa
   un punteggio già pari a 20. Non si sommano più oggetti che fissano il valore.
5. Applicare `overrideStats` per ultimo, se presente: sostituisce il risultato.

Esempio sintetico: DES base 18 + bonus 4, massimo aumentato a 22, bonus
personalizzato 2 → DES 24. Un punteggio fissato a 19 resta inferiore e non
cambia il totale; un override esplicito a 8 prevale su entrambi.
I valori finali fuori da 1–30 e massimi fuori da 20–30 bloccano l'importazione,
senza arrotondamenti o tagli per farla sembrare riuscita. Un valore base già
oltre il massimo riconciliato richiede verifica, salvo override finale esplicito.

Si accettano concessioni incondizionate; per i punteggi fissati è supportata
anche l'esatta restrizione sorgente `if not already higher`, che descrive il
confronto precedente. Non si analizzano descrizioni, nomi degli oggetti o
restrizioni arbitrarie per ricostruire regole.

## Origini, requisiti e valori derivati

Si controllano livello della capacità e unicità dell'origine di classe. Per gli
oggetti servono una sola origine, quantità positiva, equipaggiamento e sintonia
quando necessaria. Quantità zero, non equipaggiato o non in sintonia escludono
la concessione. Origine ambigua o stato non verificabile bloccano il calcolo.
Il riepilogo e la concessione originale devono concordare anche su bersaglio,
valore e restrizione; durata o requisiti nascosti nell'oggetto restano problemi,
anche se omessi dal riepilogo. I normali aumenti selezionati mantengono la
riconciliazione esplicita delle scelte introdotta precedentemente.

La COS finale alimenta il contributo ai PF massimi; la DES finale alimenta la
CA. Le difese senza armatura supportate usano le caratteristiche finali e non
abbassano la CA base quando la formula alternativa è inferiore. Abilità,
salvezze, attacchi e lancio usano poi i punteggi convertiti.

Anche i PF sono calcolati dai campi sorgente validati **dopo** la riconciliazione
della COS. Un massimo preliminare non positivo non scarta più campi validi
che un oggetto attivo rende utilizzabili. Un massimo finale non positivo o
campi mancanti/malformati continuano a bloccare. L'override dei PF sostituisce
il massimo calcolato, senza aggiungervi nuovamente i bonus.

L'anteprima espone il calcolo: base, modificatori, massimo, bonus successivo,
minimo fissato, eventuale override e risultato finale. La conferma aggiunge un
nuovo PG e conserva gli altri. Cambiare equipaggiamento dopo l'importazione
non ricalcola questi valori: rimangono una fotografia dei dati letti.

## Prove e loro limiti

**Lettura reale del personaggio 144777811**, con il connettore PocketD-D:
HTTP 200, JSON valido, 426.718 byte. Restano FOR/DES/COS/INT/SAG/CAR
8/18/14/18/10/10, PF 32/73, CA 17, 41 voci d'inventario con quantità
conservate e quattro Potion of Healing da 2d4+2. La proiezione è stata
accettata dal parser C e dal codec nativo eseguiti sul PC. Nessuna raccolta
personale aperta o modificata, nessun trasferimento al dispositivo fisico,
nessun JSON completo del personaggio pubblicato.

Questo sorgente non dimostra i nuovi casi di massimi aumentati e punteggi
fissati: i risultati seguenti usano **dati inventati**, separati dalla prova reale.

- **230 test JS passati**, inclusi 432 combinazioni di caratteristiche, massimi,
  bonus, valori fissati e override; non mutazione della fonte, backup e limiti,
  oggetti inattivi, quantità zero, origini/requisiti discordanti e PF preliminari.
- **432 stati indipendenti PC → USB → parser C → codec nativo**: confronto con
  punteggi, PF e CA attesi, più **12.960 confronti** su prove, abilità e salvezze
  C. Parser e roundtrip di questi pacchetti eseguiti anche con ASan/UBSan.
- Browser di produzione: lettura/anteprima senza scritture, casi bloccati,
  override finale, oggetto inattivo, annullamento/conferma, nuovo PG, altre
  schede conservate, reload ed esportazione. Layout desktop, 390 e 320 px.
  Ripetute anche le regressioni Beyond di base, estese e di riconciliazione.
- Build ESP32-S3 riuscita. Il percorso usa i formati già esistenti: nessuna
  nuova regola numerica dedotta dal firmware.

Il risultato resta `conversion.complete: false`. Accesso HTTP, validità del
formato e fedeltà della conversione sono controlli distinti.

## Compatibilità e cosa manca

Nessun nuovo formato: backup PC v11–v16 e USB9–USB14 secondo i dati presenti;
stato nativo PDD33/PDDX, partizioni e NVS invariati. Il firmware distribuito è
0.72.0; il miglioramento del calcolo avviene nell'importatore PC. Non serve
aggiornare il firmware solo per ricevere i punteggi numerici già rappresentabili;
il profilo difese della v0.71 continua a richiedere almeno USB14 / firmware 0.71.

Restano da riconciliare restrizioni diverse, formule/dadi/durate, massimi con
sottotipi non riconosciuti (come `<ability>-score-maximum`), aumenti di massimo
ancora da scegliere, origini/capacità sostitutive e override numerici in
`characterValues`. Non sono equivalenti ai campi espliciti `bonusStats` e
`overrideStats`, che sono supportati. Difese condizionali, effetti speciali,
contenitori/peso, ricalcolo dopo modifiche, autenticazione privata, lettura dal
dispositivo e scrittura verso Beyond restano successivi. La diagnosi dei
riavvii USB segnalati dall'utente resta da fare sull'hardware.

Riferimento tecnico, non dipendenza esecutiva: il calcolo dei campi è stato
confrontato con [abilities.ts di MrPrimate/ddb-importer, commit
44bb451175da3e96dbfcb71547de59995655609e](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/abilities.ts).
