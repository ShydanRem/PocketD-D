# Costi dei profili e reazione — v0.37

**Aggiornamento v0.38:** [costi in slot ordinari/patto](COSTI-SLOT.md), PDD18
con lettura PDD1–PDD17. I limiti sugli slot e il formato PDD17 qui descritti
si riferiscono alla v0.37; cariche oggetto ancora successive.

Il dispositivo può collegare ciascuno dei tre [profili di riduzione](PROFILI-RIDUZIONE.md)
a **un contatore oppure agli usi limitati di una magia/capacità**, con quantità
esplicita e reazione facoltativa. Risorsa, reazione, PF/temporanei e fine della
concentrazione vengono salvati nello stesso candidato. Funziona offline.

## Configurazione e applicazione

Da **Danni → Riduci → Profili → Profilo N → Modifica → Dettagli → Costi**:

- **Fonte** scorre Nessuna, contatori disponibili e magie/capacità con un pool
  di usi valido. Le voci passive e le magie senza utilizzi limitati sono escluse.
  Il nome completo è sotto i controlli; il numero identifica la voce.
- **Costo** va da 1 al massimo della fonte. Zero è valido soltanto con Nessuna.
  Cambiare fonte reimposta il costo a 1 (oppure 0 per Nessuna).
- **Usa reazione** è indipendente: è possibile richiedere soltanto la reazione.

Anteprima del profilo offre quattro confronti: Dati, Regola, Durata e **Costi**.
Confermare la definizione non spende nulla. È consentito salvare un costo maggiore
della disponibilità corrente, purché non superi il massimo della fonte.
Da Profilo N → Costi si consulta la definizione salvata senza modificarla.

La scelta per il colpo conserva i passaggi Usa profilo e Usa valori.
**Tipi → Conto → Profilo → Costi** mostra nome, saldo prima/dopo, costo e
reazione all'impatto. Una risorsa insufficiente o una reazione già usata
bloccano l'applicazione prima del tiro, insieme ai controlli su budget,
ammissibilità, fonte e difese già presenti. Nessuna spesa parziale.

**Se il profilo è scelto e si conferma un colpo valido, il costo e la reazione
vengono consumati anche quando il danno finale è zero**, oppure una riduzione
manuale/immunità rende nulla la sottrazione effettiva del profilo. L'anteprima
lo dichiara. Non si tira COS a danno zero. Per evitare la spesa, scegliere
Nessuno e confermare Usa valori prima di applicare il colpo.

Esempio: contatore 3, costo 2, reazione pronta, colpo 31 e profilo Dopo 3.
Il danno finale è 28; confermare salva contatore 1 e reazione usata insieme
all'esito PF/temporanei/COS. Annulla dall'Eroe ripristina l'intera modifica.
Una fonte con disponibilità 1 blocca lo stesso evento senza spendere né tirare.

## Reazione e inizio del proprio turno

**Stato → Reazione** mostra Disponibile/Già utilizzata:

- **Usa reazione** annota una spesa esterna, con conferma. Non consuma altri pool
  e non applica effetti di capacità.
- **Inizio turno** propone il recupero della sola reazione. Non modifica risorse,
  timer, concentrazione o PF. Se è già pronta, non esegue una scrittura inutile.

**1 round non equivale all'inizio del proprio turno.** Né l'avanzamento comune
né i riposi recuperano automaticamente la reazione. I riposi continuano a
recuperare gli utilizzi secondo le politiche già configurate delle capacità.
Non esiste ancora un gestore di iniziativa che determini il turno.

Una conferma del danno bloccata dalla reazione offre **Gestisci reazione**:
si può annotare Inizio turno e tornare allo stesso impatto, mantenendo importo,
profilo e assegnazioni. Recuperare la reazione non risolve una risorsa insufficiente.
Le risorse si correggono nei normali percorsi Risorse/Magie.

Le assegnazioni PWR breve, BOOT breve/lungo restano configurabili e conservate.
La scorciatoia esistente **Stato** apre anche la nuova gestione della reazione;
non vengono aggiunti valori ai tasti. PWR lungo resta hardware. Le nuove pagine
proteggono le bozze da scorciatoie, attenuazione e import USB; le conferme
avvengono sul display.

## Persistenza, errori e importazione

Scritture **PDD17**, tag **PDDH**, dimensione **8379 + 552 × numero voci**, massimo
**74619 byte**. Dopo i 30 byte dei profili PDD16 e prima del CRC si aggiungono
**16 byte**: per ciascuno dei tre profili, tipo fonte uint8, indice uint8,
costo uint16 little endian e richiede-reazione booleano; infine reazione-usata
booleana globale. Nessun dump di struct. Booleani, riferimenti, quantità e
combinazioni sono verificati anche con CRC valido.

Legge **PDD1–PDD16 senza scrivere al boot**. I profili PDD16 mantengono la loro
definizione, iniziano senza costi/reazione e con reazione disponibile. Fixture
PDD16 sintetica prodotta dall'encoder originale v0.36 `885d9d2`: verifica lettura,
primo salvataggio nella banca alternata, banca originale intatta e riavvio.
Le nuove scritture richiedono v0.37 o successivo. Un firmware precedente non
legge PDD17: non cancellare NVS per aggirare un downgrade.

Su errore di salvataggio, il candidato COS, il dado e il conto delle spese
rimangono fissati: Riprova non ricalcola né scala un'altra volta. Il controller
NVS blocca ulteriori scritture dopo un errore I/O; un commit segnalato fallito
può avere già scritto il candidato. Occorrono riavvio e verifica: non è uno
storico transazionale che garantisce il ripristino di ogni evento dopo blackout.
Il riepilogo conserva nome/costo/saldi e reazione all'impatto, anche se il
profilo finisce con la concentrazione. Non è uno storico persistente.

**USB v7, backup PC v10 e layout flash invariati.** Importare una nuova proiezione
con conferma sostituisce anche costi e reazione locali; annullare conserva tutto.
L'avviso di sostituzione comprende profili con soli costi/reazione. Preferenze
locali e tasti conservati; raccolta PC conservata. Backup progressi dispositivo→PC
ancora successivo. Nessuna nuova lettura/scrittura Beyond.

## Verifiche e limiti

354816 combinazioni indipendenti di slot, fase, fonte, costo, saldo, reazione,
danno e difese; tutti i riferimenti ai pool, saldo esatto a zero, costi gratuiti
e solo reazione. Controlli prima del RNG per risorsa/reazione, requisiti,
budget, profilo spento, zero PF e morte massiva. Candidato COS unico, reset
esplicito e riposi/round separati; record massimo, corruzione dei 16 byte con
CRC valido, migrazione PDD16 e regressioni precedenti.

UI LVGL nativa: modifica/confronto/consultazione, assenza di fonti, zero/massimo,
nomi da 40 caratteri, costo anche a danno zero, saldi e snapshot, ritorno allo
stesso impatto, annullamento, errore/retry, import e tasti protetti. Controlli
≥44 px nel cerchio e verifica visiva delle catture host. Suite firmware completa,
ASan/UBSan (leak detector disattivato), 138 test JS e build PC; compilazione
ESP32-S3. [Dettagli di verifica](FIRMWARE.md#verifiche-v037--costi-e-reazione).
Le catture host e la compilazione non costituiscono collaudo fisico.

Restano successivi: costi in slot/cariche oggetto o più risorse insieme,
riconoscimento automatico dei requisiti di talenti/armature/capacità, altre
azioni/reazioni e recuperi, cumulo di più profili, automazione degli effetti
quando si spende un uso, condizioni arbitrarie e altre durate. Non si certifica
l'applicabilità di una regola D&D soltanto perché il saldo è sufficiente.
