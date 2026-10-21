# Riduzioni esplicite dei danni — v0.35

> Aggiornamento v0.37: [costi e reazione](COSTI-REAZIONE.md), PDD17.
> Base v0.36: [profili persistenti di riduzione e budget per colpo](PROFILI-RIDUZIONE.md).
> PDD16 con lettura PDD1–PDD15; i valori manuali del colpo restano solo RAM.

Il dispositivo permette di sottrarre quantità esplicite al colpo, distinguendo
**prima delle difese della parte** e **dopo le difese dell'intero colpo**. Sono
valori manuali del colpo corrente: non sono capacità permanenti del personaggio
e non vengono dedotti da un talento, un oggetto, Beyond o dalle descrizioni.

## Impostare e verificare

Da **Danni → Riduci**, il selettore scorre **Dopo: intero colpo** e tutte le
parti del colpo, identificate da numero e tipo di danno. Per ciascuna scegli
0–9999 con i quattro pulsanti. **Usa valori** aggiorna la preparazione del colpo;
non scrive PF, risorse o difese. **Indietro** scarta tutte le modifiche della
pagina. Le altre quantità della bozza si conservano passando da una parte all'altra.

Da **Componenti → parte → Riduci** si modifica soltanto la sottrazione prima
delle difese di quella parte. **Usa valori** torna alla bozza della parte:
serve poi **Usa parte** per inserirla nel colpo. Tornare all'elenco senza
Usa parte la scarta. Cambiare il danno grezzo non cancella la quantità scelta:
l'anteprima limita la sottrazione al danno disponibile.

L'**Anteprima** mostra il danno effettivo, PF/temporanei e le riduzioni realmente
sottratte nelle due fasi. Con riduzioni impostate, **Tipi** è disponibile anche
per un'unica parte. Ogni gruppo mostra quantità grezza, sottrazione prima delle
difese ed esito delle difese; **Conto** espone il percorso completo:

1. Danno iniziale dell'intero colpo.
2. Quantità sottratta prima delle difese e quantità complessivamente scelta.
3. Somma dopo resistenza, vulnerabilità e immunità.
4. Quantità sottratta dopo le difese e quantità scelta per quel passaggio.
5. Danno effettivo che raggiunge i temporanei e i PF.

**Conferma** applica quel colpo. Il riepilogo conserva lo stesso Conto e i gruppi
originari, anche dopo la fine di una difesa vincolata alla concentrazione.
**Eroe → Annulla** ripristina l'ultima modifica della sessione.

## Ordine del calcolo

Per ogni parte: `residuo = max(0, danno − riduzione prima)`. Si sommano i residui
con stesso tipo e stesse proprietà difensive applicate; si applicano le difese
e i loro arrotondamenti una volta per gruppo. Infine:

`danno effettivo = max(0, somma dopo difese − riduzione dopo)`

Solo questo danno effettivo passa a temporanei/PF, controlli di morte e salvezza
di concentrazione. La sottrazione **Dopo** avviene **una sola volta per colpo**,
anche con sei parti o più gruppi dello stesso tipo. Non si ripete per riga.
L'eccedenza di una sottrazione è ignorata: non cura, non diventa un bonus, non
si trasferisce a un'altra parte e non passa automaticamente alla fase Dopo.

Esempi:

| Danno e difese | Prima | Dopo | Conto |
| --- | --- | --- | --- |
| 11, resistenza | 3 | 3 | `(11 − 3) / 2 = 4`, poi `4 − 3 = 1` |
| 11, resistenza | 0 | 3 | `floor(11 / 2) = 5`, poi `5 − 3 = 2` |
| 11 contundente resistente + 11 fuoco vulnerabile | 3 sul contundente | 3 | `4 + 22 = 26`, poi `26 − 3 = 23` |
| 11, nessuna difesa | 99 | 99 | Prima sottrae 11, Dopo sottrae 0; danno 0 |

Se una regola concede una sola riduzione prima delle difese su un colpo misto,
**distribuisci quella quantità fra le parti ammesse**, senza inserirla intera
in ogni riga. Aggiungere una parte nuova inizia con riduzione zero; non duplica
le altre. Dividere una parte e mantenere la stessa riduzione totale assegnata
produce lo stesso risultato se le difese applicate sono le stesse. Fra gruppi
con difese diverse l'assegnazione può cambiare il risultato: è una scelta
esplicita, non un'allocazione automatica dell'app.

Un'immunità può già rendere nullo il danno di un gruppo. Le sottrazioni mostrate
sono quantità tolte **nella rispettiva fase**, non PF risparmiati rispetto a un
colpo ipotetico senza quella configurazione.

## Blocchi, concentrazione ed errori

Una riduzione non rende verificata una base sconosciuta né inventa fatti della
fonte: **difese o fatti pertinenti mancanti bloccano tutto il colpo**, anche
quando una sottrazione sarebbe sufficiente ad azzerarlo. Conto segnala il blocco
e non presenta una somma parziale come applicabile. I casi di morte già bloccati
restano da risolvere attraverso il registro e le regole pertinenti.

A danno effettivo zero, PF, temporanei e concentrazione restano invariati e non
si tira COS. A danno positivo, una sola CD usa il totale finale, anche se tutti
i danni sono assorbiti dai temporanei. La fine della concentrazione disattiva
insieme gli effetti/extra vincolati, conservando il dettaglio all'impatto.

Se un salvataggio del tiro fallisce, candidato, dado, gruppi e sottrazioni
restano fissati; **Riprova** usa lo stesso risultato senza consumare altro RNG.
Il controller NVS blocca nuove scritture dopo un errore I/O: un commit fallito
può avere scritto il candidato e richiede riavvio/verifica. Il fake NVS e le
prove UI non equivalgono a un'interruzione di alimentazione fisica.

Editor Riduci, Tipi e Conto proteggono le bozze da scorciatoie e importazioni USB;
PWR/BOOT conservano le assegnazioni, PWR lungo resta hardware. Nessuna nuova
pagina conferma danni tramite un tasto fisico. L'attenuazione è sospesa nei modali.

## Dati e compatibilità

**Nuovo**, la scorciatoia Danni e una nuova importazione confermata azzerano
le riduzioni. Annullare l'importazione le conserva. Le quantità e la composizione
del colpo sono solo RAM: riavviare non le ripristina e non esiste un giornale
persistente dei tiri o delle riduzioni. I PF/risorse/effetti risultanti sono
salvati insieme come in precedenza.

**PDD15, USB v7, backup PC v10 e layout flash invariati**. Nessuna nuova
migrazione: il core legge le fixture PDD1–PDD14 e i record PDD15. Le vecchie API
singole mantengono riduzioni zero. Per il core, `pd_damage_event.reductions[i]`
si riferisce alla stessa riga di `components[i]` e `contexts[i]`;
`reduction_after` riguarda l'intero evento. Solo i campi delle righe attive sono
letti. Valori oltre 9999 vengono respinti prima di mutazioni o RNG.

Nel report, `groups[].amount` è il grezzo, `groups[].reduction` la sottrazione
prima effettiva e `groups[].adjusted` il risultato delle difese, **prima della
sottrazione finale**. `mitigated` è la loro somma solo con profili/fonti verificati;
`total.adjusted` è il finale. `requested_before/after` e `reduction_before/after`
distinguono la scelta dalla quantità sottratta. Nessun record cambia formato.

## Verifiche e limiti

- **66560 casi indipendenti**: 13 tipi, tutte le combinazioni R/V/I, dieci danni,
  otto riduzioni prima e otto dopo; PF/temporanei, limiti e rifiuti atomici.
- **95728 suddivisioni/assegnazioni** con quantità conservata e tutte le
  **720 permutazioni** di sei parti; riduzione finale applicata una volta.
- Valori massimi, eccedenze, immunità/zero, profilo/fonte sconosciuti, campi
  malformati, rifiuti prima del RNG, CD sul totale ridotto, una sola salvezza,
  difese all'impatto e candidato PDD15 rileggibile.
- UI LVGL nativa: annulla/usa delle due bozze, editor generale/per parte,
  promozione e aggiunta di righe, Conto prima/dopo, salvataggio fallito e retry,
  azzeramento, annullamento, importazione e tasti protetti. Anche sei parti,
  somme a cinque/sei cifre e totale massimo; controlli ≥44 px nel cerchio e
  testi senza sovrapposizioni.
- Suite firmware completa e regressioni passate, 138 test JS/build PC;
  ASan/UBSan su calcoli difensivi, effetti, magie/riposi/stato, allocazioni e UI
  con leak detector disattivato. Firmware ESP32-S3 compilato;
  [dettagli](FIRMWARE.md).

Restano **regole automatiche e persistenti** di riduzione legate a specifici
talenti, armature o risorse, budget condivisi convalidati secondo una regola,
eventuali costi di reazione e durate. Non sono dedotti dai nomi o dal testo.
Questo incremento offre sottrazioni esplicite verificabili per un colpo; non
certifica l'applicabilità di ogni regola speciale del personaggio.

Funziona offline senza Foundry. Raccolta personale PC conservata; nessuna nuova
lettura Beyond, credenziale o JSON personale pubblicato. Restano da collaudare
sulla scheda alimentazione, touch, USB, memoria e prestazioni; batteria e
sospensione rimangono nel blocco del prodotto fisico.
