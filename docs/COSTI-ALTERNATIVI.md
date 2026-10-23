# Costi alternativi delle riduzioni — v0.60

Un profilo locale di riduzione può richiedere **tutte le spese configurate**
oppure **una sola spesa scelta per ogni colpo**. Il dispositivo calcola e salva
insieme danni, pagamento ed eventuale reazione, offline. Restano la grafica del
compagno e i tasti configurabili; il PC serve per preparazione e backup.

## Configurare il profilo

Aprire **Danni → Riduci → Profili → Modifica → Dettagli → Costi**.
Il pulsante **Tutte → Una scelta** cambia la modalità della bozza.
**Spesa 1/3 → 2/3 → 3/3** permette di configurare le fonti:
contatori, utilizzi limitati di capacità/magie, slot ordinari o patto di un
livello preciso, cariche di un oggetto. Quantità e cariche restano separate.

**Una scelta** richiede almeno due posizioni non vuote, con fonti distinte e
costi positivi entro la capacità della fonte. Le posizioni possono avere
buchi. Due nomi uguali non rendono uguali le fonti: contano tipo e indice.
Con una sola fonte l'anteprima chiede **Configura 2 spese** e blocca il salvataggio.
I saldi possono essere insufficienti al momento della configurazione.

L'anteprima del profilo confronta modalità e definizioni precedenti/nuove nella
scheda **Costi**. Conferma salva la definizione senza consumare risorse.
La modalità **Tutte**, predefinita per i profili precedenti, conserva il
comportamento esistente: tutte le fonti configurate devono essere disponibili.

## Scegliere il pagamento del colpo

Scegliere il profilo e aprire l'anteprima del danno. **Spesa: da scegliere**
apre tre righe con nome, costo e saldo. Le fonti insufficienti o gli oggetti
con quantità zero sono disabilitati. Toccare una fonte valida torna alla
conferma, con danno effettivo, PF/temporanei e spesa selezionata.
La scelta è esplicita anche quando una sola alternativa ha saldo sufficiente.

Una fonte non scelta esaurita o non disponibile non blocca il pagamento
valido. Una fonte scelta insufficiente blocca l'intero colpo: nessuna
sostituzione automatica con un'altra fonte o livello. Senza scelta, con
proprietà del danno irrisolte o con profilo non applicabile non vengono
addebitati costi né applicati danni. **Usa reazione** resta un requisito
dell'intero profilo e si applica una sola volta, oltre alla fonte scelta.

Confermare spende soltanto la fonte scelta e applica l'impatto come un'unica
modifica di stato. Come per i profili esistenti, il costo esplicito si paga
anche se la riduzione porta il danno finale a zero: verificare con il DM che
questa semantica rappresenti la capacità configurata. La scelta vale per il
colpo corrente e viene azzerata iniziando un nuovo impatto o applicando una
nuova configurazione del profilo/riduzione.

**Tipi → Conto → Profilo → Costi** mostra i saldi dell'impatto originale.
Le fonti escluse sono marcate **Non scelta**, con saldo invariato. Nomi,
definizione, fonte scelta e saldi restano nella ricevuta in RAM anche dopo
modifiche successive; non sono un nuovo registro persistente delle spese.

## Concentrazione, morte e recupero

La stessa scelta viene usata dal tiro COS e dalla risoluzione esplicita dei
danni a zero PF/morte istantanea. Un blocco precedente al tiro non usa RNG.
Dopo un errore di salvataggio del candidato COS, **Riprova** usa risultato e
pagamento già fissati, senza rilanciare. Le restrizioni complete delle azioni
a zero PF e le eccezioni alle regole base restano fuori da questo incremento.

**Eroe → Annulla** ripristina l'intera ultima modifica, comprese risorsa,
reazione, PF ed effetti terminati. Un salvataggio fallito lascia invariata la
scheda visibile in RAM. Un vero errore I/O può bloccare le scritture fino al
riavvio e avere già scritto una candidata: i test host di retry non attestano
il recupero fisico dopo un'interruzione di alimentazione.

## Persistenza e compatibilità

Scritture **PDD28 / PDDS**. Dimensione invariata rispetto a PDD27:
`8942 + 552 × numero poteri`, massimo **75.182 byte**. Il byte della fase
di ciascun profilo, all'offset `8333 + 552 × poteri + 10 × indice profilo`,
contiene `fase | (modalità << 1)`:

| Byte | Fase | Pagamento |
| --- | --- | --- |
| 0 | Prima delle difese | Tutte |
| 1 | Dopo le difese | Tutte |
| 2 | Prima delle difese | Una scelta |
| 3 | Dopo le difese | Una scelta |

Gli altri bit sono riservati e rifiutati. Soltanto PDDS interpreta il bit
della modalità. PDD27 e precedenti restano leggibili e iniziano con **Tutte**;
non si reinterpretano fasi malformate dei vecchi record come alternative.
Il firmware precedente non legge PDD28; usare gli strumenti aggiornati e
conservare il backup prima dell'aggiornamento. Nessuna scrittura al solo boot.

Il backup nativo v1 accetta PDD25/PDD26/PDD27/PDD28. Conserva la modalità del
profilo, mentre la selezione del colpo è transitoria. L'anteprima PC espone
le tre modalità e mantiene i record originali; il validatore Python controlla
integrità e trasporto, il dispositivo verifica tutte le regole native prima
del ripristino. Una nuova importazione USB confermata sostituisce i profili
locali come il resto della copia dispositivo; annullarla conserva tutto.
La raccolta PC resta invariata.

USB9, PDH2 con lettura PDH1, backup PC v11, diagnostica v1 e partizioni non
cambiano. Nessuna dipendenza da Foundry, accesso Beyond o servizio remoto.

## Verifiche e lavoro residuo

Le prove sono sintetiche: **25.920 casi indipendenti** di fonti, saldi,
scelte, reazione, fasi, danno zero/nonzero e una/sei componenti, confrontati
con stati attesi completi. Casi aggiuntivi coprono buchi, selezioni malformate,
nessuna sostituzione, fonte non scelta assente, blocchi senza RNG, COS,
danni a zero PF/morte istantanea, record massimo e migrazione PDD27.

La UI host verifica configurazione/anteprima, scelta, blocchi, salvataggio
fallito e retry, annullamento, ricevute congelate, reazione, nomi lunghi,
candidato COS e percorso morte. Copertura **106/106 viste**. Un backup
effettivamente generato dal core C con 120 poteri sintetici e modalità
`[Una scelta, Tutte, Una scelta]` è letto dal PC e dal validatore Python
conservando i record. Nessuna porta seriale aperta in questa prova.
Suite completa, sanitizzatori e build sono riportati in [FIRMWARE.md](FIRMWARE.md).

L'archivio include `firmware-host-preview/alternative-costs-gallery.png` e
`alternative-*.png`: render del codice LVGL su host, non fotografie del pannello.
Il futuro controllo fisico H27 è in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).

Restano oltre tre fonti, combinazioni di gruppi come «due costi insieme oppure
un terzo costo», costi variabili/opzionali e requisiti automatici. Nessuna
regola viene dedotta dalle descrizioni Beyond. Il blocco effetti resta aperto;
hardware, alimentazione e sessione reale restano da verificare.
