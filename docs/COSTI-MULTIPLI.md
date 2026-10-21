# Spese multiple sul dispositivo — v0.42

**Aggiornamento v0.43:** le [ricariche a dado](RICARICHE-OGGETTI.md) sono disponibili
come evento esplicito, senza rilevare automaticamente alba o frequenza.
PDD22/USB8/backup v11 invariati. Le limitazioni storiche sotto si riferiscono
alla versione del titolo.

Un profilo di riduzione può richiedere fino a **tre spese distinte**, oltre
all'eventuale reazione. Esempio: due cariche dello scudo e un utilizzo di una
capacità. Il dispositivo verifica e salva l'intera operazione offline.

## Configurazione e conferma

Apri **Danni → Riduci → Profili → Modifica → Dettagli → Costi**. Il pulsante
**Spesa 1/3 → 2/3 → 3/3** sfoglia le tre posizioni. Per ogni posizione scegli
una fonte precisa e un costo positivo, fino alla capacità della fonte:

- un contatore;
- gli utilizzi di una magia innata o capacità con limite;
- slot ordinari o del patto di un livello preciso;
- le cariche di una voce dello zaino.

**Fonte: nessuna** lascia vuota quella posizione. Sono valide anche una prima
posizione vuota o una posizione intermedia vuota. Non sono tre alternative:
tutte le spese configurate sono obbligatorie per quell'impatto.

Il selettore salta le fonti già utilizzate nelle altre due posizioni. Per
pagare più volte dalla stessa fonte, somma il costo in una sola posizione.
Il core rifiuta anche dati duplicati costruiti fuori dall'interfaccia.
Due livelli diversi, slot ordinari/patto o due oggetti distinti restano fonti
diverse. L'identità è il tipo e l'indice, non il nome visualizzato.

**Usa reazione** è una proprietà dell'intero profilo e cambia per tutte le
posizioni. Modificare la bozza non scala risorse. L'anteprima del profilo mostra
prima/dopo per ciascuna spesa nella scheda **Costi**, con il selettore Spesa.
Conferma salva la definizione completa; Indietro consente di scartare la bozza.
La consultazione di un profilo salvato permette di sfogliare, senza modificarlo.

## Applicazione e resoconto

Scegli il profilo per l'impatto come prima. **Tipi → Conto → Profilo → Costi**
mostra una spesa alla volta, con nome, tipo/indice della fonte, costo e saldi.
Se manca un solo saldo, un oggetto ha quantità zero, la reazione è già usata o
manca un fatto necessario sul danno, l'intero impatto è bloccato. Tutti i saldi
restano invariati; il riepilogo non propone un addebito parziale.
Sfoglia le tre posizioni per vedere quale risorsa è insufficiente.

Confermare un impatto valido salva tutte le spese, reazione, PF/temporanei ed
esito della concentrazione insieme. Come per i costi singoli, le spese scelte
avvengono anche se il danno finale è zero. Le quantità degli oggetti non sono
costi: spendere cariche non consuma un oggetto. Nessuna sostituzione automatica
con un'altra fonte o livello e nessuna spesa opzionale se il profilo è selezionato.

Il resoconto conserva nomi, costi e saldi dell'impatto originale per tutte le
posizioni. Un tiro COS con errore di salvataggio conserva un solo candidato;
**Riprova** non rilancia e non addebita di nuovo. **Annulla** sull'Eroe ripristina
l'intera ultima modifica, comprese tutte le risorse spese.

## Salvataggi e trasferimento

PDD22, tag `PDDM`, aggiunge **24 byte** prima del CRC: tre profili × due spese
aggiuntive × (tipo uint8, indice uint8, costo uint16 LE). La prima spesa conserva
la posizione storica. Dimensione **8853 + 552 × voci magia**, massimo **75093 byte**.
Valori fuori intervallo, riferimenti inesistenti, costi nulli su fonti attive,
fonti vuote con dati residui e duplicati vengono rifiutati senza modificare
stato o output.

PDD1–PDD21 restano leggibili: prima spesa conservata, due aggiuntive vuote,
nessuna scrittura al boot. Fixture PDD21 sintetica dall'encoder originale
v0.41 `048ef28`; prima scrittura PDD22 in un'altra banca, vecchia copia conservata.
Dopo il salvataggio PDD22 serve firmware v0.42 o successivo: non cancellare NVS
per aggirare un downgrade. Layout partizioni invariato.

USB8 e backup PC v11 non cambiano. I profili sono configurazioni locali del
dispositivo; il PC non li importa né ne recupera i progressi. Importare una
nuova copia richiede conferma e azzera i profili locali, comprese le spese
aggiuntive. Annullare l'importazione li conserva. La raccolta sul PC resta intatta.

## Verifiche e limiti

Tutte le prove nuove usano dati sintetici. Nessuna nuova lettura Beyond,
credenziale o JSON personale pubblicati, né modifica di raccolte personali.

- 48.000 casi C indipendenti: tutte le terne dei cinque tipi di fonte, ogni
  profilo/fase, combinazioni di saldi insufficienti, reazione e danno zero/nonzero.
- Addebiti confrontati con stati attesi completi; duplicati in ogni coppia di
  posizioni, buchi, rifiuto atomico, nessun RNG al blocco e candidato COS unico.
- Indici estremi: utilizzo 119, oggetto 59, livello 9; costi fino a 99/999/9;
  record massimo, tutti i 24 byte corrotti con CRC valido e duplicati serializzati.
- Migrazione PDD1–21 e NVS senza scritture al boot; nuovo salvataggio e riavvio
  con tutte le spese conservate; byte della vecchia banca identici.
- LVGL host: configurazione di tre fonti, confronto e consultazione, scorciatoie
  e import bloccati nelle viste protette, errore/retry, ricevuta congelata,
  risorsa secondaria mancante, Annulla, nomi lunghi e import confermato/annullato.
- Suite host completa e 41.411 confronti PC → USB → C; 144 test JavaScript e
  build Vite/HTML autonomo. Sanitizzatori e misure firmware in [FIRMWARE.md](FIRMWARE.md).

Anteprime nell'archivio: `firmware-host-preview/multiple-costs-preview.png` e
`multi-cost-*.png`, render del vero codice LVGL compilato per host, con dati
inventati. Non sono fotografie del dispositivo né prove di prestazioni fisiche.

Restano oltre tre spese, costi alternativi/opzionali o variabili al tiro,
requisiti automatici e uso di oggetti/incantesimi con effetti arbitrari.
Le regole devono essere configurate e valutate con il DM. Restano aperti
ricariche speciali, condizioni complete, durate di altri soggetti, lavoro
estetico e collaudi fisici di display/touch/tasti/USB/memoria/alimentazione.
