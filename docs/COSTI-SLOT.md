# Costi in slot sul dispositivo — v0.38

**Aggiornamento v0.42:** [spese multiple](COSTI-MULTIPLI.md) disponibili: fino a
tre fonti distinte per profilo più reazione, confermate insieme. Formato NVS
corrente PDD22; USB8 e backup PC v11 invariati. Le misure e le limitazioni
storiche sotto si riferiscono alla versione del titolo.

**Aggiornamento v0.41:** [cariche degli oggetti](CARICHE-OGGETTI.md) disponibili
come saldo distinto dalla quantità, con configurazione PC, gestione sul dispositivo,
costi e recuperi. Formati correnti: backup PC v11, USB8 e PDD21. Le verifiche
e le limitazioni storiche descritte sotto si riferiscono alla versione del titolo.

**Aggiornamento v0.39:** [durate del proprio turno](DURATE-TURNO.md), PDD19.
Inizio turno avanza ora anche i timer Inizio; Fine turno è separato dal Round.
I timer dei salvataggi precedenti mantengono la modalità Round.

I [profili di riduzione](PROFILI-RIDUZIONE.md) possono spendere anche slot
**ordinari oppure del patto**, oltre ai contatori e agli usi limitati della
[v0.37](COSTI-REAZIONE.md). Il pool e il livello sono espliciti. Funziona offline;
non richiede Foundry, Beyond o un PC durante il gioco.

## Configurare e usare

Da **Danni → Riduci → Profili → Profilo N → Modifica → Dettagli → Costi**,
Fonte scorre nell'ordine: Nessuna, contatori, utilizzi limitati, slot ordinari
e slot del patto. Si mostrano solo fonti con un massimo positivo; gli slot
richiedono i dati Magie importati. Un pool esistente ma esaurito resta
selezionabile: salvare la definizione non richiede di avere già il saldo.

- **Fonte: slot 1 >** indica gli slot ordinari di livello 1.
- **Fonte: patto 5 >** indica gli slot del patto di livello 5.
- Il nome completo sotto i controlli conferma sempre pool e livello.
- **Costo** è il numero di slot da spendere, da 1 al massimo del pool scelto
  (al massimo 9 nel formato attuale). Cambiare fonte riparte da costo 1.
- **Usa reazione** rimane indipendente e facoltativo.

La quarta pagina dell'anteprima confronta nome, costo e reazione prima/dopo.
**Confermare la definizione non spende slot.** Per un colpo si mantengono Usa
profilo e Usa valori; **Tipi → Conto → Profilo → Costi** mostra pool/livello,
saldo prima/dopo e reazione all'impatto. Solo la conferma del colpo spende,
insieme a PF/temporanei ed eventuale esito della concentrazione.

Esempio: due slot del patto di livello 5, costo 1. Confermare un colpo con quel
profilo porta il saldo a 1 e lascia invariati tutti gli slot ordinari e gli
altri livelli. Con saldo 0, il colpo è bloccato prima del RNG. **Non si sceglie
automaticamente un livello superiore, il pool ordinario o quello del patto.**
Occorre modificare esplicitamente la fonte oppure scegliere Nessuno.

Rimane la politica della v0.37: **un profilo scelto e confermato paga anche a
danno zero**, inclusi immunità o altre riduzioni che annullano il suo contributo.
A danno zero non si tira COS. L'anteprima dichiara la spesa; Eroe → Annulla
ripristina l'intera modifica, compresi slot e reazione.

Spendere uno slot in questo percorso non lancia automaticamente una magia:
non verifica preparazione, livello minimo, classe, equipaggiamento o talenti,
non attiva effetti e non aumenta la quantità di riduzione in base al livello.
Il profilo è una configurazione esplicita da verificare con il DM; il percorso
Magie per lanciare un incantesimo rimane distinto. Non consumare lo stesso
costo sia da Magie sia dal profilo.

## Recuperi, errori e tasti

I recuperi mantengono le politiche esistenti: il **riposo breve** restituisce
slot del patto; il **riposo lungo** restituisce ordinari e patto. L'anteprima
del riposo mostra i recuperi distinti. **1 round e Inizio turno non recuperano
slot**; i riposi non recuperano automaticamente la reazione. Inizio turno
continua a cambiare soltanto quest'ultima.

Un errore di salvataggio conserva lo stato visibile; nel percorso COS candidato,
dado e conto restano fissati, senza un secondo consumo o rilancio su Riprova.
Il controller NVS reale blocca ulteriori scritture dopo un errore I/O e un
commit segnalato fallito può avere già scritto il candidato: occorrono riavvio
e verifica. Non è uno storico persistente degli eventi.

Tasti PWR breve/BOOT breve/lungo conservati e configurabili, PWR lungo hardware.
Le nuove fonti usano le pagine protette già presenti: import USB e scorciatoie
non confermano né cambiano le bozze. L'import confermato sostituisce i profili
locali come prima; annullare conserva tutto. La raccolta PC resta conservata.

## Persistenza e compatibilità

Scritture **PDD18**, tag **PDDI**, dimensione invariata **8379 + 552 × numero
voci**, massimo **74619 byte**. Nessun campo o byte aggiunto: nel campo tipo
fonte dei profili, 0 Nessuna, 1 Contatore, 2 Usi, **3 Ordinari, 4 Patto**.
Per questi ultimi l'indice 0–8 rappresenta il livello 1–9, non l'indice di una
magia. Si validano pool importato, massimo positivo, costo e indice anche con
CRC valido. Un record PDD17 mantiene l'interpretazione precedente e rifiuta
le nuove fonti 3/4.

Legge **PDD1–PDD17 senza scrivere al boot**. La fixture PDD17 è sintetica,
prodotta dall'encoder v0.37 originale `1274857` prima delle modifiche: costi
precedenti, reazione usata, timer, concentrazione e saldi conservati. La prima
scrittura PDD18 usa la banca alternata senza toccare l'originale; rilettura dopo
riavvio verificata. Le nuove scritture richiedono v0.38 o successivo: non
cancellare NVS per aggirare un downgrade. **USB v7, backup PC v10 e layout flash
invariati.** Nessuna nuova lettura/scrittura Beyond o modifica della raccolta.

## Verifiche e lavoro ancora aperto

233280 combinazioni indipendenti di tre profili, due fasi, due pool, nove livelli,
costo 1–9, saldo 0–9, reazione, danno e immunità. Limiti di capacità 0–9 e costo
0–10, riferimenti assenti, dati Magie mancanti, saldi esatti a zero, nessuna
sostituzione automatica, candidato COS unico e recuperi separati. PDD18 massimo,
enum/indici/costi corrotti con CRC valido, semantica PDD17 e migrazione NVS.
UI LVGL di configurazione, confronto, pool esaurito, livello 9, ordinari/patto,
conferma/errore/riprova/annulla anche a danno zero, snapshot, riposi e import.
Catture host ispezionate; nessun collaudo sul dispositivo fisico.
[Verifiche complete](FIRMWARE.md#verifiche-v038--costi-in-slot).

**Cariche degli oggetti ancora non rappresentate.** La copia dispositivo contiene
la quantità degli oggetti, ma non un pool distinto di cariche con massimo e
recupero. Non si scala la quantità al posto delle cariche. Per supportarle
serviranno modello, configurazione/importazione, trasferimento e UI dedicati;
un contatore manuale resta un contatore, non una carica riconosciuta dall'oggetto.
Restano anche costi multipli, requisiti/effetti automatici e durate di turno/minuti.
Questo incremento completa gli slot nel blocco effetti; non chiude il blocco.
