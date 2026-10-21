# Durate in minuti e ore — v0.40

**Aggiornamento v0.41:** [cariche degli oggetti](CARICHE-OGGETTI.md) disponibili
come saldo distinto dalla quantità, con configurazione PC, gestione sul dispositivo,
costi e recuperi. Formati correnti: backup PC v11, USB8 e PDD21. Le verifiche
e le limitazioni storiche descritte sotto si riferiscono alla versione del titolo.

Effetti locali CA/PF massimi, extra difensivi e profili di riduzione possono
usare durate in **minuti o ore**, con minuti residui conservati anche dopo
salvataggio e riavvio. Il tempo viene dichiarato sul dispositivo: non avanza
in base all'orologio reale, alla sospensione o al tempo passato a leggere una
schermata. Il gioco individuale resta offline.

## Configurazione

Nei tre editor Durata, dopo Manuale/Round/Inizio/Fine turno si trovano:

- **Minuti**: durata iniziale 1 minuto, regolabile fino a 99 minuti.
- **Ore**: durata iniziale 1 ora, regolabile fino a 99 ore con precisione al minuto.
  I controlli −1h/−1m/+1m/+1h consentono anche durate come 1h 59min.

Cambiare modalità riparte dal valore iniziale. Per una voce attiva la durata
deve essere positiva; inattiva resta in pausa e a zero è scaduta. Il campo
Round/Passaggi rimane distinto: nella modalità Tempo è zero e non viene usato
per salvare i minuti. I riepiloghi mostrano unità e residuo effettivo; nelle
viste compatte **m/min indica sempre minuti**, anche per una durata scelta in ore.

Salvare la definizione non fa trascorrere tempo, non spende costi e non cambia
la reazione. La concentrazione resta un vincolo indipendente: la sua fine
disattiva e scollega le voci legate; una scadenza temporale non termina la magia.

## Dichiarare il tempo trascorso

**Stato → Tempo**, accessibile anche tramite la scorciatoia Stato già configurata:

1. Scegli l'intervallo con −1h/−1m/+1m/+1h, da **1 minuto a 99 ore**.
2. **Anteprima** mostra tempo scelto, voci coinvolte/scadute, CA, PF massimi e
   PF correnti prima/dopo. **Scadenze** elenca i residui prima/dopo in minuti,
   fino a 50 voci su 17 pagine.
3. **Indietro** annulla la proposta; **Conferma** salva tutti i conteggi e i PF
   insieme. **Eroe → Annulla** ripristina l'intera modifica della sessione.

Esempio: un effetto di 2 minuti, una difesa di 1h 59min e una riduzione di
1 minuto. Dopo 1 minuto rimangono rispettivamente 1, 118 e 0 minuti: la riduzione
si disattiva. Dopo un altro minuto scade l'effetto e la difesa ha 117 minuti.
La quantità o le cariche degli oggetti non sono coinvolte.

**Round, inizio/fine turno, reazione, risorse e slot rimangono invariati.**
I relativi comandi non decrementano i minuti. Non esiste una conversione
implicita da un minuto a dieci round. I riposi mantengono i loro recuperi e la
fine della concentrazione, ma non dichiarano automaticamente minuti trascorsi:
l'intervallo si applica da Tempo, nell'ordine concordato con il DM.

Non viene conservato un calendario globale. Senza timer pertinenti non c'è
una scrittura; il comando aggiorna durate esistenti. Una nuova conferma dopo
quella riuscita dichiara un ulteriore intervallo, non corregge quello precedente.
Durate di altri personaggi e iniziativa restano successive.

## Scadenze in ordine e PF

Gli effetti numerici vengono risolti **nell'ordine in cui scadono**. Voci che
terminano allo stesso minuto sono applicate insieme. Ogni passaggio verifica
CA/PF massimi e limita i PF correnti al massimo di quel momento; un aumento
successivo del massimo non restituisce PF.

Esempio: base 32 PF massimi, +10 in scadenza al minuto 1 e −10 in scadenza al
minuto 2, 32 PF correnti iniziali. Dichiarare 2 minuti porta i PF prima a 22 e
poi conserva 22, anche se il massimo finale torna a 32. Far scadere entrambi
allo stesso minuto lascia 32 PF.

Se un passaggio intermedio rendesse CA/PF massimi fuori limite, l'operazione
è interamente bloccata **anche quando il totale finale sarebbe valido**:
effetti, difese, profili e PF restano invariati. Occorre correggere le definizioni.
Nessun dado viene lanciato e nessun costo viene consumato avanzando il tempo.

Su errore di salvataggio lo stato visibile resta precedente. Nel controller
NVS reale gli errori I/O bloccano scritture successive; un commit segnalato
fallito può aver già scritto il candidato. Occorrono riavvio e verifica:
non è un registro persistente né una garanzia di deduplicazione dopo blackout.

Tasti PWR breve/BOOT breve/lungo conservati e configurabili, PWR lungo hardware.
Le pagine Tempo proteggono proposta e dettaglio da import USB, attenuazione e
scorciatoie. Importare una nuova proiezione con conferma sostituisce i timer
locali; annullare conserva tutto. Raccolta PC conservata; nessuna lettura Beyond.

## Persistenza e verifiche

Scritture **PDD20**, tag **PDDK**, dimensione **8529 + 552 × numero voci**, massimo
**74769 byte**. Si aggiungono **100 byte**, 50 contatori uint16 little endian
prima del CRC, dopo i selettori PDD19: 8 effetti, 39 extra (slot × 13 + tipo),
3 profili. Nuovi selettori **3 Minuti, 4 Ore**; timer temporale con Round zero,
residuo ≤99 o ≤5940 minuti, positivo se attivo. Per le altre modalità i minuti
devono essere zero; anche i contatori numerici inutilizzati devono essere zero.
Nessun dump di struct. I record PDD19 rifiutano le nuove modalità 3/4.

Legge **PDD1–PDD19 senza scrivere al boot**, conservando modalità precedenti,
conteggi, costi e reazione. Fixture PDD19 sintetica dell'encoder v0.39 originale
`70a194b`, generata prima delle modifiche; prima scrittura nella banca alternata
e riavvio con residui come 119 minuti verificati. Le nuove scritture richiedono
v0.40 o successivo: non cancellare NVS per aggirare un downgrade.
**USB v7, backup PC v10 e layout flash invariati.**

57600 combinazioni indipendenti di famiglia/indice/unità/residuo/intervallo/
pausa/reazione/vincolo; 13122 sequenze confrontate con un riferimento separato
che avanza minuto per minuto. Scadenze simultanee e intermedie, rifiuto dopo
una perdita PF già proposta, ore parziali serializzate, 50 timer e corruzione
di tutti i 100 byte con CRC valido. UI dei tre editor e del comando Tempo,
anteprima, dettaglio, errore/riprova/annulla, limiti, 17 pagine e import.
[Verifiche complete](FIRMWARE.md#verifiche-v040--tempo-di-gioco).
Catture host e compilazione non sostituiscono il collaudo fisico.

Restano durate legate a soggetti diversi, cariche oggetto, condizioni complete,
requisiti/effetti automatici e il blocco fisico con batteria, sospensione e
spegnimento. Il blocco effetti della roadmap rimane aperto.
