# Danni ricevuti e difese per tipo — v0.28

> Aggiornamento v0.33: [condizioni difensive e fonte esplicita](DIFESE-CONDIZIONALI.md),
> raggruppamento per applicabilità e PDD14. Le omissioni/formati sotto sono storici.

> Aggiornamento v0.32: [danni misti dello stesso colpo](DANNI-MISTI.md), con
> raggruppamento per tipo e un solo esito. PDD13/USB v7/layout invariati.

> Aggiornamento v0.31: [difese temporanee, concentrazione e round comune](DIFESE-TEMPORANEE.md)
> con PDD13. I limiti/formati dell’incremento sotto sono storici.

> Aggiornamento v0.30: [effetti collegati alla concentrazione](EFFETTI-CONCENTRAZIONE.md),
> fine/sostituzione congiunta e PDD12. I limiti e formati sotto sono storici.

> Documento della v0.28. La v0.29 aggiunge [salvezza COS e CD opzionale max 30](CONCENTRAZIONE.md).
> La risoluzione manuale resta disponibile; i limiti sul tiro riportati sotto
> descrivono l'incremento storico. Formati invariati.

Il dispositivo aggiunge **Usi → Stato → Danni**, con una componente di danno
in entrata e configurazione manuale delle difese per i 13 tipi comuni. Sono
funzioni offline, senza Foundry, servizi o PC al tavolo. Nessuna nuova lettura
personale Beyond e nessuna modifica della raccolta PC.

## Configurazione esplicita

**Danni → Difese per tipo** elenca Contundente, Perforante, Tagliente, Acido,
Freddo, Fuoco, Forza, Fulmine, Necrotico, Veleno, Psichico, Radiante e Tuono.
Toccare una riga apre Resistenza, Vulnerabilità, Immunità e Configurato.
Anteprima mostra i flag prima/dopo; solo Conferma salva il profilo locale.

I tipi iniziano **da configurare**, anche nelle schede già importate: i dati
USB non dichiarano ancora queste proprietà. Non configurato **non significa
normale**. Un danno di quel tipo rimane bloccato e offre Configura difesa.

Attivare un flag marca la bozza come configurata. Per dichiarare un tipo normale,
lasciare tutti i flag spenti e impostare esplicitamente **Configurato: sì**.
Disattivare Configurato cancella tutti i flag del tipo e torna da verificare.
Indietro abbandona la bozza. Confermare un profilo senza cambiamenti non scrive
flash e non applica danni.

Questi sono profili manuali del giocatore, **non resistenze dedotte dal testo**.
Non si attivano/terminano con Ira, oggetti, concentrazione o condizioni; non
scadono con i round e non si recuperano con i riposi. Aggiornarli quando cambia
il contesto. Non sono rappresentati attacchi magici/non magici, argento,
materiali, requisiti della fonte o altre eccezioni. Selezionare soltanto i flag
validi per la componente in questione, dopo verifica con il DM.

## Danno, arrotondamento e temporanei

Danni ricevuti permette di scegliere il tipo e **1–9999 danni**, con −10/−1/+1/+10.
L'importo è il totale della singola componente **prima** di queste tre difese:
considerare già nel valore dadi critici, altri bonus e riduzioni da gestire
manualmente. I flag del profilo non vanno applicati nuovamente prima di inserire
l'importo. Non è il tiro dei danni dell'arma contro un bersaglio.

Il calcolo esplicito è:

1. Immunità: danno effettivo zero, con precedenza sugli altri flag.
2. Altrimenti resistenza: metà, arrotondata per difetto.
3. Poi vulnerabilità: raddoppio del risultato precedente.
4. I PF temporanei assorbono per primi il danno effettivo; il resto riduce i PF,
   fino a zero, senza modificare il massimo.

Le resistenze non si sommano tra loro. Se resistenza e vulnerabilità sono entrambe
selezionate, l'arrotondamento intermedio rimane visibile: **11 → 5 → 10**, non 11.
Per esempio, 11 danni resistiti con 3 temporanei danno **5 effettivi**, consumano
3 temporanei e tolgono 2 PF. Immunità e 1 danno resistito producono zero.

**Anteprima** espone tipo/profilo, danno iniziale/effettivo, temporanei e PF
prima/dopo. Indietro non modifica nulla; Conferma applica una volta sola e mostra
il riepilogo. Nuovo riapre il modulo con importo 1 e lo stesso tipo, con una
nuova conferma richiesta. Conferme duplicate sul riepilogo non riapplicano danni.
Il calcolo non estrae dadi.

Danni di tipi diversi o con eccezioni differenti non vengono aggregati in un
singolo colpo dal dispositivo. L'ordine, le concentrazioni di un colpo misto e
le altre interazioni restano da gestire con il DM; non è ancora un motore di
pacchetti di danno. Una riduzione dei PF massimi da effetto/scadenza resta
separata: non viene trasformata in un colpo.

## Concentrazione e zero PF

Se c'è concentrazione, il danno effettivo è positivo e restano PF positivi,
l'anteprima mostra la **CD base max(10, danno effettivo / 2 per difetto)**.
Il danno assorbito dai temporanei conta comunque per questa verifica.

Conferma resta bloccata finché **Esito** non è impostato su **mantieni** o
**termina**, dopo aver risolto con il DM la salvezza e le eventuali eccezioni.
Il firmware non lancia automaticamente COS, non applica talenti, vantaggi o
successi garantiti. Usare dadi esterni oppure uscire dall'anteprima per usare
Dadi, poi rientrare. Cambiare importo/tipo o iniziare un nuovo colpo azzera la
scelta; non si riusa in silenzio l'esito precedente.

PF, temporanei ed eventuale fine concentrazione vengono salvati insieme.
Danno effettivo zero non richiede l'esito e non termina concentrazione. Un colpo
che porta da PF positivi a zero segue il comportamento esistente: termina
concentrazione e Ira e avvia/azzera il registro morte se era disponibile.
Non viene inventato un registro per copie vecchie che non lo hanno.

Due situazioni **bloccano questo flusso**, senza applicare danni:

- Danno effettivo positivo con personaggio già a zero PF, anche con temporanei:
  fallimenti, critici e stabilizzazione si risolvono dal registro con il DM.
- Un possibile caso di **morte istantanea**, quando il danno oltre temporanei e
  PF raggiunge il massimo effettivo. Non viene convertito in un normale zero PF
  o in una morte certificata: usare le correzioni/manualità appropriate col DM.

Il pulsante PF −1 rimane una correzione diretta, senza resistenze e senza queste
nuove verifiche; l'anteprima Danni non ne cambia il significato. Eccezioni di
classe, riduzioni particolari, morte istantanea e salvezze a zero PF non sono
considerate automaticamente risolte da questo incremento.

## Tasti, salvataggio e conservazione

**Eroe → Aspetto → Tasti** aggiunge **Danni** per PWR breve, BOOT breve e BOOT lungo.
Apre il modulo, senza consumare PF. Le conferme di danno/profilo restano touch;
i tasti non le saltano. PWR lungo rimane riservato all'alimentazione.
Durante le conferme USB risponde busy e lo schermo non si attenua. Il filtro del
primo gesto di risveglio rimane applicato.

Errori di scrittura mantengono lo stato visibile precedente, compreso lo stato
di concentrazione della scheda. Si può riprovare nella stessa anteprima
senza sottrarre nuovamente i valori; un commit NVS fallito può essere ambiguo e
richiede controllo al riavvio, come negli altri flussi. Zero danno senza altre
modifiche evita scritture. **Eroe → Annulla** ripristina insieme PF, temporanei,
concentrazione, registro e profili dell'ultima modifica di sessione.

I profili sono salvati localmente e riletti al riavvio. **Una nuova copia USB
sostituisce effetti, durate e difese locali**, dopo avviso e conferma; Annulla
conserva tutto. Preferenze locali del dispositivo e raccolta PC rimangono
conservate. Il backup dei progressi/difese dal dispositivo verso PC è ancora da
completare: il backup PC non contiene i nuovi aggiornamenti locali.

## Formati e compatibilità

USB **v7** e backup PC **v10** invariati: nessun campo difese importato o dedotto
nuovamente da Beyond. Le vecchie copie mantengono i dati esistenti, ma tutte le
difese iniziano da configurare.

Nuove scritture **PDD11**, tag binario **PDDB** (B = 11):
**7974 + 552 × numero magie/capacità**, massimo **74214 byte**. Prima del CRC,
dopo il payload PDD10, si aggiungono quattro uint16 little-endian: maschere
configurato/resistenza/vulnerabilità/immunità. Sono ammessi soltanto i 13 bit
noti; i flag meccanici devono essere un sottoinsieme dei tipi configurati.
La nuova scorciatoia 12 è Danni. Nessun dump di struct o nuova partizione.

PDD1–PDD10 restano leggibili senza scritture al boot. Il fixture PDD10 è prodotto
dall'encoder reale del commit **066e3d8**, prima della modifica, con soli dati
sintetici; conserva timer, effetti e PF superiori alla base. La prima scrittura
nuova lascia il record precedente nell'altra banca. Layout flash/pdd_state
invariati. Un downgrade alla v0.27 incontra un formato sconosciuto e blocca il
caricamento per conservare i dati; non cancellare NVS per aggirarlo.

## Verifiche e limiti

- **139904 casi indipendenti** su 13 tipi, otto combinazioni di flag, importi,
  PF/temporanei e tutti gli importi 0–9999 per gli otto profili.
- Profili non configurati distinti dai normali, arrotondamenti, immunità,
  assorbimento, salvezza concentrazione con temporanei e scelte mantieni/termina.
- Rifiuti senza mutazione/output modificati, zero PF, danni massicci sul massimo
  effettivo, fine Ira/concentrazione, isolamento delle durate e input fuori limite.
- Record massimo con 120 magie/tutti i profili, campi malformati con CRC valido,
  fixture PDD1–PDD10, NVS senza scritture al boot e profili riletti al riavvio.
- UI LVGL reale su host: bozze, profili, anteprime, cancel, fallimenti/riprova,
  concentrazione, nessuna doppia applicazione, zero danno senza scritture,
  annullamento, ultimo tipo, limiti, import busy e tasti protetti. Pulsanti nel
  cerchio ≥44 px e controllo di sovrapposizioni tra pulsanti.
- Regressioni: 138 JS, build PC, 37751 confronti PC→USB→C, 17266 casi degli effetti
  numerici e 40648 delle durate; ASan/UBSan e compilazione ESP32-S3. Leak detector
  disattivato nel sandbox. Immagini dal renderer host, non foto dell'AMOLED.

Il blocco effetti resta parziale: difese condizionali, fonti multiple e scadenze
delle difese, colpi misti, salvezza concentrazione automatica, proprietà speciali,
regole complete delle condizioni e ricalcolo da equipaggiamento rimangono aperti.
Nessun flash, collaudo fisico o promessa di autonomia.
