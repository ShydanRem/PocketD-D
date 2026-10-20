# Difese condizionali e fonte del danno — v0.33

> Aggiornamento v0.34: [tre extra indipendenti per tipo](DIFESE-MULTIPLE.md),
> PDD15 con lettura PDD1–PDD14. I limiti sul singolo extra descritti qui sono storici.

Le proprietà resistenza, vulnerabilità e immunità possono dipendere dalla
fonte del danno. Le condizioni sono scelte esplicite, disponibili sul dispositivo
offline e senza Foundry. Le proprietà di un colpo non vengono ricavate da nomi,
descrizioni, bonus delle armi, importazione Beyond o note delle condizioni.

## Configurare le regole

Da **Danni → Difese per tipo → tipo**, abilita le proprietà desiderate della
base oppure del profilo **A tempo**. **Regole** apre i selettori separati per
resistenza, vulnerabilità e immunità. Una proprietà non selezionata non può
ricevere una condizione. Ciascuna delle tre, in ciascuno dei due profili,
può avere una delle sette condizioni:

| Scelta | Quando si applica |
| --- | --- |
| Sempre | Ogni fonte |
| Magico | Danno dichiarato magico |
| Non magico | Danno dichiarato non magico |
| Att. non magico | Attacco dichiarato non magico |
| Non argentato | Attacco non magico, con arma non argentata |
| Non adamantio | Attacco non magico, con arma non in adamantio |
| Nessuno dei due | Attacco non magico, né argentato né in adamantio |

Le ultime tre sono abbreviazioni: sotto il selettore e nell'anteprima viene
mostrato il requisito completo. **Anteprima** dal selettore apre **Prima/Dopo**
sulla proprietà e sul profilo scelti. Base/Extra e il selettore della proprietà
permettono di consultare le altre regole senza cambiare la bozza. **Conferma**
salva una sola modifica; Indietro torna al selettore senza applicarla.

L'anteprima generale mantiene valori base/extra, durata e concentrazione;
**Regole** espone i requisiti completi. Il titolo segnala **cond.** se la bozza
contiene selettori condizionali. Nell'elenco, un profilo con regole è indicato
come **condizionale**, senza presentarlo falsamente come una difesa sempre
applicabile o un profilo normale. Le basi ancora sconosciute restano da configurare.

Disabilitare una proprietà elimina la sua condizione; lasciare la proprietà
selezionata durante una modifica conserva la regola. Disattivare un extra,
farlo scadere o terminare il suo vincolo conserva invece i selettori insieme
alle proprietà, per un eventuale riuso esplicito.

## Dichiarare la fonte di ogni componente

In **Danni → Componenti → riga → Fonte**, quattro proprietà possono essere
**? / no / sì**: Magico, Attacco, Argentato, Adamantio. Ogni pressione cicla
in questo ordine. **? significa sconosciuto, non falso**.

**Usa fonte** nell'editor aggiorna la bozza della componente; serve poi **Usa
parte** per inserirla nel colpo. Indietro scarta le dichiarazioni della pagina.
Non si modificano PF, risorse o stato del personaggio. Le altre componenti
mantengono le proprie dichiarazioni, anche se hanno lo stesso tipo di danno.

Se l'anteprima non può risolvere una difesa per mancanza di informazioni, mostra
**Fonte da verificare**, blocca tutto il colpo e offre **Dichiara fonte** per la
prima componente che ne ha bisogno. Usa fonte torna direttamente all'anteprima;
se mancano ancora fatti pertinenti, il colpo resta bloccato. La configurazione
mancante di un profilo ha precedenza e mantiene il percorso Configura difesa.

Non occorre risolvere i fatti che non possono cambiare le proprietà applicate.
Ad esempio:

- «Attacco non magico non argentato» con **Attacco no** è esclusa anche se
  Magico/Argentato restano sconosciuti.
- Con **Magico no, Attacco sì, Argentato no**, la stessa difesa si applica senza
  conoscere Adamantio.
- Una resistenza di base ai danni magici e un extra attivo ai non magici insieme
  garantiscono quella resistenza senza conoscere Magico. Non si accumulano.
- Un'immunità certa rende irrilevanti eventuali incertezze su resistenza e
  vulnerabilità per quel colpo, che infligge zero.

Le proprietà non necessarie possono quindi restare **?**. Una difesa Sempre
continua a funzionare senza aggiungere passaggi al vecchio flusso. Una base
sconosciuta non diventa conosciuta tramite un extra o una dichiarazione di fonte.

**Nuovo** e la scorciatoia Danni azzerano le dichiarazioni. Aggiungi crea una
componente con fonte sconosciuta; rimuovere/promuovere righe conserva la fonte
associata alla riga corretta. Non si eredita accidentalmente l'argentatura o
la natura magica di un impatto precedente.

## Parti dello stesso tipo con applicabilità diversa

Nel [colpo misto](DANNI-MISTI.md) ora si sommano prima dell'arrotondamento le
componenti con **stesso tipo e stesse proprietà difensive applicate**. Le parti
con applicabilità diversa restano distinte. L'ordine delle righe non cambia
l'esito e non si arrotondano separatamente parti con lo stesso profilo risolto.

Esempio: resistenza al contundente non magico. Un solo impatto contiene
3 contundente non magico + 3 contundente non magico + 3 contundente magico:
le prime due righe producono `(3 + 3) / 2 = 3`, l'ultima 3. Totale **6**.
Se un'immunità condizionale copre la porzione magica, quella porzione diventa zero.

**Tipi** mostra quindi gruppi, non necessariamente tipi tutti diversi. Ogni
riga espone il tipo, la difesa effettivamente applicata e quantità prima/dopo;
con fonte ancora da verificare mostra `?`. Un gruppo bloccato non diventa una
somma parziale applicabile. Restano massimo sei componenti, un unico assorbimento
dei temporanei, controllo PF/morte, esito di concentrazione e salvataggio.

Tutte le condizioni sono risolte sullo stato **all'impatto**. Perdere poi la
concentrazione termina gli extra associati per i colpi successivi; il riepilogo
conserva i gruppi e le difese originali. Nel tiro, candidato e dettaglio restano
fissati insieme al dado su errore: non si cambiano fonte o regole durante il retry.

## Durate, conferme ed errori

I requisiti non cambiano i timer: **1 round** avanza insieme effetti e difese
attivi, con anteprima e una sola conferma. Fine manuale, stessa magia rilanciata,
Ira, zero PF, fallimento e riposo lungo disattivano/scollegano gli extra associati,
conservando i loro selettori. Non li rendono attivi o permanenti.

**Eroe → Annulla** ripristina l'ultima modifica di sessione, comprese le regole
persistenti o lo stato risultante di un colpo. Un errore di scrittura conserva
lo stato visibile precedente. Il controller NVS blocca le scritture dopo errore
I/O: occorre riavvio/verifica e un commit fallito può aver scritto il candidato.
Non c'è un giornale persistente delle facce o delle dichiarazioni del colpo.

PWR/BOOT mantengono le assegnazioni; nelle conferme, nel riepilogo delle regole,
in Fonte e nel dettaglio del colpo non confermano o abbandonano la scelta.
USB risponde busy e l'attenuazione è sospesa. PWR lungo rimane hardware.

## Salvataggi e migrazione

Le nuove scritture sono **PDD14**, tag **PDDE** (E = 14), dimensione
**8125 + 552 × numero magie/capacità**, massimo **74365 byte**. Si aggiungono
78 byte prima del CRC: 39 selettori della base e 39 dell'extra, tre per tipo.
Codici fuori 0–6 e condizioni su proprietà non selezionate vengono rifiutati
anche con CRC valido. Booleani, timer e riferimenti conservano i controlli esistenti.

Lettura **PDD1–PDD13** conservata, senza scritture al boot: ogni proprietà
precedente mantiene **Sempre**, senza reinterpretare vecchi profili o descrizioni.
La fixture sintetica PDD13 proviene dall'encoder v0.32 del commit `7ed65df`,
prima della modifica: comprende base, extra a round e vincoli di concentrazione.
Il test NVS verifica lettura senza scritture, prima modifica nella banca alternata,
copia originale intatta e riavvio con regole e timer riletti correttamente.

Firmware precedenti alla v0.33 non leggono PDD14 e conservano il blocco sui
record futuri. Non cancellare NVS per aggirare un downgrade.
**USB v7, backup PC v10 e layout flash invariati.** Fonte e composizione sono
solo RAM; le regole difensive sono locali e persistenti. L'importazione
confermata sostituisce lo stato di gioco e rimuove anche i selettori locali;
annullarla conserva tutto. Raccolta PC intatta, backup dispositivo→PC ancora aperto.

Per chi usa il core: `pd_defence_flags` restituisce solo proprietà esplicitamente
Sempre; per un impatto usare la preview con contesti. Le vecchie API a componente
singola non inventano fatti della fonte e restano bloccate se quei fatti servono.

## Verifiche e perimetro

- **404352 casi indipendenti**: tutti i 13 tipi e tre proprietà, sette regole o
  assenza per entrambe le fonti, extra attivo/inattivo, tutte le 81 descrizioni
  parziali della fonte. Aspettative da valutazioni booleane complete indipendenti.
- Proprietà complementari, immunità certa, fatti irrilevanti, raggruppamento per
  applicabilità, fonte malformata, rifiuti prima del RNG, fine/timer/riposi,
  proprietà all'impatto e candidato coerente di concentrazione.
- Record massimo, tutti i 78 selettori alterati con CRC valido, condizioni su
  proprietà assenti, fixture PDD1–PDD13 e NVS senza migrazione al boot.
- UI LVGL reale su host per regole base/extra, anteprima completa, testi lunghi,
  fonte parziale/cancel, proprietà necessarie, parti dello stesso tipo, errore e
  tiro fissato, risultato originario, annullamento, durata/vincolo, import e tasti.
  Pulsanti ≥44 px nel cerchio e nessuna sovrapposizione fra testi/controlli.
- Regressioni 138 JS/build PC, 37751 confronti PC→USB→C, 494312 casi misti,
  19968 suddivisioni/720 permutazioni, 139904 danni, 78840 salvezze, 4608 vincoli,
  32032 difese/64350 avanzamenti, 17266 effetti e 40648 durate. ASan/UBSan su
  condizioni, misti/danni/concentrazione, vincoli/difese, effetti/durate,
  magie/riposi/stato, core/parser, NVS, allocazioni e UI; leak detector disattivato.
  Compilazione ESP32-S3 riuscita.

Restano condizioni arbitrarie (specifiche creature, attaccanti, stati, scuole o
fonti non rappresentate), riduzioni fisse e più fonti aggiuntive con scadenze
diverse per tipo. C'è una condizione per proprietà nella base e una nell'unico
extra del tipo: non si inventano selettori per cataloghi o eccezioni non supportate.
Argentato/Adamantio sono fatti dichiarati sull'arma del colpo, non capacità
automatiche del personaggio. Nessuna nuova lettura Beyond, credenziale o JSON
personale incluso. Nessun flash/collaudo fisico; batteria, sospensione, touch,
USB e recupero dopo interruzione richiedono ancora il dispositivo reale.
