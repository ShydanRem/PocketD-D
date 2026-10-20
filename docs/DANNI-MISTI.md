# Danni misti di un solo colpo — v0.32

> Aggiornamento v0.33: [condizioni difensive e fonte esplicita](DIFESE-CONDIZIONALI.md),
> raggruppamento per applicabilità e PDD14. Le omissioni/formati sotto sono storici.

Il dispositivo può preparare **da una a sei componenti dello stesso colpo**,
raggrupparle per tipo e applicare una sola modifica a temporanei, PF e
concentrazione. Funziona offline, senza Foundry e senza PC al tavolo.
Le [difese di base e temporanee](DIFESE-TEMPORANEE.md) rimangono esplicite.

## Composizione sul dispositivo

**Usi → Stato → Danni** mantiene i controlli della prima componente. Con più
righe il valore centrale indica **1/N**, non il totale del colpo.
**Componenti** apre l'elenco, fino a tre righe per pagina:

- tocca una riga per modificarne tipo e quantità;
- **Aggiungi** apre una nuova bozza, inizialmente di un tipo non ancora presente;
- **Usa parte** inserisce la bozza nell'elenco senza applicare danni alla scheda;
- **Indietro** nell'editor scarta quelle modifiche; **Rimuovi** elimina la riga
  dall'elenco del colpo, lasciandone sempre almeno una;
- **Difese** nell'editor apre il profilo del tipo selezionato, con la normale
  conferma della configurazione; poi torna all'editor della componente.

Si possono scegliere anche tipi già presenti. Ogni riga ammette 1–9999 danni
nell'interfaccia; il core conserva la possibilità di zero della vecchia API.
Dalla sesta riga Aggiungi è disabilitato. Eliminare la prima promuove la
successiva. Le righe sono una bozza in RAM: prepararle, modificarle o eliminarle
non consuma risorse, non estrae dadi e non scrive lo stato del personaggio.

**Anteprima** apre il totale del colpo. **Tipi** espone il dettaglio raggruppato:
quantità prima/dopo e difese effettive per ogni tipo, in ordine fisso. Riepilogo
torna alla conferma senza applicare nulla. Lo stesso dettaglio resta disponibile
dopo l'applicazione e conserva le difese usate in quel colpo.

**Nuovo** svuota le componenti aggiuntive e prepara la prima con quantità 1.
La scorciatoia fisica **Danni** apre sempre un nuovo colpo semplice. Allontanarsi
dall'editor non scrive le righe nella scheda; aprire nuovamente Danni le azzera.
Non sono una coda di colpi, uno storico o un nuovo dato da importare da Beyond.

## Raggruppamento e arrotondamento

Prima si sommano le componenti **dello stesso tipo**; poi si applica a ciascun
tipo il suo profilo effettivo al momento dell'impatto:

1. immunità azzera il gruppo;
2. altrimenti resistenza dimezza per difetto;
3. poi vulnerabilità raddoppia;
4. i risultati dei tipi vengono sommati per il totale effettivo del colpo.

Una stessa proprietà non si applica più volte. L'ordine delle righe non cambia
il risultato, né cambia dividere una quantità in più righe dello stesso tipo.

| Componenti di un colpo | Difese | Risultato |
| --- | --- | --- |
| 3 fuoco + 3 fuoco | Resistenza al fuoco | (3 + 3) / 2 = 3 |
| 3 contundente + 4 contundente + 7 fuoco | Resistenza contundente, vulnerabilità fuoco | 7 / 2 per difetto + 7 × 2 = 17 |
| 11 freddo | Resistenza e vulnerabilità | 5 × 2 = 10 |
| 25 contundente + 80 fuoco | Resistenza contundente, immunità fuoco | 12 + 0 = 12 |

Il raggruppamento assume **lo stesso profilo per tutte le componenti dello
stesso tipo**. Condizioni diverse, ad esempio una porzione magica e una non
magica soggette a difese differenti, non sono rappresentate separatamente.

## Un solo impatto, un solo esito

Dal totale effettivo si applicano una volta l'assorbimento dei temporanei,
la riduzione dei PF e i controlli di zero PF/possibile morte istantanea.
Due componenti singolarmente piccole possono raggiungere insieme una soglia
che blocca il flusso: non vengono applicate parzialmente.

Se serve concentrazione, si sceglie un unico esito manuale oppure **Tira COS**:
la CD base è `max(10, totale effettivo / 2 per difetto)`, prima dei temporanei.
Resta la scelta esplicita **CD max 30**, con modalità ed extra locali già presenti.
I casi con salvezza COS assente/bloccata mantengono l'esito manuale, senza RNG.

Tutte le difese del colpo vengono lette **prima** dell'eventuale fine della
concentrazione. Non si applica una componente per poi ricalcolare le successive
senza le difese appena terminate. Nell'ultimo esempio della tabella, un
fallimento termina le difese collegate, ma il colpo resta da 12 danni; il
prossimo userà il profilo aggiornato. Anche il dettaglio del risultato conserva
resistenza/immunità e calcoli originali.

Se termina un bonus collegato ai PF massimi, il limite ai correnti viene
applicato nello stesso candidato, dopo la disattivazione di tutti i vincoli.
Un massimo maggiore non cura. Un colpo interamente immune o arrotondato a zero
non richiede una verifica, non termina concentrazione e non scrive uno stato
identico su NVS.

**Non unire impatti separati**: più attacchi, raggi distinti o fonti che richiedono
salvezze separate vanno risolti con eventi separati, anche se avvengono nello
stesso turno. Il giocatore/DM stabilisce quali componenti appartengono allo
stesso impatto; il dispositivo non interpreta la descrizione di una magia.

## Profili sconosciuti, conferme e errori

Se manca anche un solo profilo, **tutto il colpo è bloccato**. Tipi mostra i
valori noti e `?` per quelli da verificare; il totale non diventa una somma
parziale applicabile. Anche una riga zero con profilo sconosciuto rimane da
verificare nella API. Un extra temporaneo non risolve una base sconosciuta.

**Configura difesa** seleziona il primo tipo mancante, senza applicare danni;
alla conferma del profilo torna all'anteprima del colpo misto. Ripetere se
mancano altri tipi. Annullare la configurazione lascia il colpo bloccato.
Il blocco per danni su personaggio già a zero PF o possibile morte istantanea
rimane esplicito: questi casi richiedono la gestione con il DM.

**Conferma** applica un solo candidato e un solo salvataggio; **Eroe → Annulla**
ripristina insieme temporanei, PF, concentrazione, effetti e difese dell'ultima
modifica di sessione. Non sono aggiunte transazioni separate per le righe.

Nel percorso del tiro, candidato e dettaglio vengono preparati insieme alla
faccia estratta, salvati prima dell'animazione e mantenuti fissi su errore.
Riprova non ricalcola con difese nuove e non estrae un altro dado. Dopo
l'estrazione non si possono cambiare componenti, CD, esito o modalità, né uscire
dalla conferma. Canvas assente e accelerazione conservano lo stesso risultato.
Restano i [limiti NVS del tiro](CONCENTRAZIONE.md): blocco scritture dopo errore
I/O, riavvio/verifica necessari e nessun giornale persistente della faccia.
Un commit fallito può aver scritto il candidato; il riavvio non ripristina
necessariamente la schermata precedente.

Le assegnazioni PWR/BOOT sono conservate. Conferma e dettaglio Tipi bloccano
scorciatoie, importazione USB e attenuazione; le azioni si confermano col touch.
PWR lungo rimane riservato all'alimentazione.

## Dati e compatibilità

**PDD13/PDDD, USB v7, backup PC v10 e layout flash invariati.** Le componenti e
il dettaglio del colpo sono solo in RAM; persistono i normali valori risultanti
del personaggio. Nessuna nuova migrazione o scrittura al boot. Le vecchie API
C a componente singola restano disponibili e usano lo stesso motore.

Il totale grezzo massimo è 59994; con vulnerabilità può arrivare a 119988.
I calcoli non troncano il valore a 16 bit; il risultato passa comunque per i
blocchi di PF/morte esistenti prima dell'applicazione. Il core rifiuta conteggi
fuori 1–6, tipi sconosciuti, quantità fuori 0–9999 e scelte non valide prima di
RNG o mutazioni. Nessun grande stato viene aggiunto allo stack: i candidati
riusano la memoria PSRAM già prevista per le conferme.

La raccolta PC resta conservata; nessuna nuova lettura personale Beyond o
scrittura verso Beyond. Il trasferimento confermato PC→dispositivo sostituisce
la copia di gioco come prima. Il backup dei progressi dispositivo→PC resta aperto.

## Verifiche e perimetro

- **494312 casi indipendenti**: coppie di tipi, tutte le proprietà difensive,
  quantità pari/dispari, totale, temporanei e PF.
- **19968 suddivisioni**: 1–6 righe dello stesso tipo, inclusi zeri e arrotondamenti;
  **720 permutazioni** dello stesso colpo con dettaglio canonico identico.
- Massimo 119988, profili sconosciuti, rifiuti senza mutazioni/RNG, soglie
  aggregate, una sola verifica COS, CD max 30, difese all'impatto e candidato
  riletto in PDD13. Tutti gli esiti conservano i limiti dei singoli danni.
- UI LVGL reale su host: composizione, aggiunta/cancel/rimozione della prima,
  paginazione di sei righe, dettaglio prima/dopo, profili mancanti, conferme,
  annullamento completo, errore/riprova con tiro fissato, risultato storico
  delle difese, grandi totali, immunità senza scrittura e tasti protetti.
- Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 139904 danni singoli,
  78840 concentrazione, 4608 vincoli, 32032 difese temporanee, 64350 avanzamenti,
  17266 effetti e 40648 durate; lettura dei vecchi record e NVS. ASan/UBSan sui
  nuovi calcoli, danni/concentrazione, vincoli/difese, effetti/durate, magie,
  riposi/stato, allocazioni e UI; leak detector disattivato nel sandbox.
  Compilazione ESP32-S3 riuscita.

Restano difese condizionali, più fonti temporanee con scadenze diverse per tipo,
riduzioni fisse, interazioni complesse delle condizioni e regole specifiche
sui danni. La v0.32 gestisce componenti dello stesso impatto con gli stessi
profili per tipo; non è un motore completo di regole. Nessun flash o collaudo
fisico: batteria, sospensione, touch, USB e recupero dopo interruzione richiedono
ancora il dispositivo reale. Le anteprime allegate sono render host del firmware.
