# Durate del proprio turno — v0.39

**Aggiornamento v0.40:** [minuti/ore e tempo di gioco](TEMPO-GIOCO.md), PDD20.
Le limitazioni sui minuti/ore e il formato PDD19 sotto descrivono la v0.39.

Effetti locali CA/PF massimi, extra difensivi e profili di riduzione possono
scadere a **inizio o fine del proprio turno**. Round, inizio turno e fine turno
sono tre avanzamenti distinti, scelti esplicitamente sul dispositivo.
Funziona offline e conserva i tasti configurabili.

## Configurare la scadenza

Negli editor **Effetti → Durata**, **Difese → A tempo → Durata** e
**Profili → Modifica → Dettagli**, Durata scorre:

1. **Manuale**: nessun conteggio.
2. **Round**: avanza soltanto con 1 round.
3. **Inizio turno**: avanza soltanto quando si conferma l'inizio del proprio turno.
4. **Fine turno**: avanza soltanto quando si conferma la fine del proprio turno.

Il conteggio è 1–99 per una voce attiva. Con 1, l'effetto termina al prossimo
passaggio corrispondente confermato; con 2, al secondo. **Cambiare modalità
riparte da 1**, oppure da zero scegliendo Manuale. Nelle viste compatte r/i/f
indicano round/inizio/fine; l'editor mostra sempre il nome completo. Le voci
inattive restano in pausa. A zero vengono disattivate; per riattivarle serve
una durata positiva. Quantità, regole, costi e altri dettagli restano disponibili.

La conferma della configurazione salva soltanto la definizione. Non avanza
durate, non spende utilizzi/slot e non modifica la reazione. Il vincolo alla
concentrazione rimane indipendente: la perdita della concentrazione disattiva
e scollega le voci vincolate anche se avevano altri passaggi da attendere.
La sola scadenza del timer non termina la magia concentrata.

## Inizio e fine turno

Da **Stato → Reazione** (anche con la scorciatoia Stato configurata):

- **Usa reazione** annota una spesa esterna; nessun timer viene avanzato.
- **Inizio turno** propone i soli timer di inizio e il recupero della reazione.
- **Fine turno** propone i soli timer di fine e conserva la reazione.

Il riepilogo mostra conteggi avanzati/scaduti, reazione, CA, PF massimi e PF
correnti prima/dopo. **Scadenze** elenca ogni effetto/difesa/riduzione coinvolto,
con conteggio prima/dopo e stato finale; fino a 50 voci su 17 pagine.
**Indietro** annulla la proposta. **Conferma** salva tutto in un unico candidato.
Eroe → Annulla ripristina l'intera modifica della sessione.

Se il nuovo massimo scende sotto i PF correnti, l'anteprima dichiara i PF persi.
Un massimo che cresce non cura. Se una scadenza rendesse CA/PF massimi fuori
limite, **nessun timer e nessuna reazione cambiano**: occorre correggere gli
effetti locali. Un turno senza cambiamenti non scrive inutilmente in memoria.
Ripetere una conferma con timer ancora attivi rappresenta invece un altro
passaggio: non esiste un identificatore di turno o un gestore di iniziativa.

I comandi non stabiliscono chi sta agendo, né controllano che un turno sia
iniziato prima di finirlo. Non riconoscono durate «del prossimo turno del
bersaglio/incantatore», iniziativa altrui, minuti, ore o testo delle capacità.
Quelle regole restano da rappresentare o gestire manualmente con il DM.

## Round, riposi, danni e salvataggio

**1 round** avanza soltanto i timer Round e conserva la reazione. Il relativo
pulsante non si abilita per la sola presenza di timer del turno. I riposi non
avanzano questi conteggi; restano validi recuperi e fine della concentrazione
del riposo lungo già previsti. Nessun turno recupera slot o usi limitati.

Gestisci reazione da un danno bloccato conserva il colpo preparato. Se Inizio
turno fa scadere il profilo scelto, tornando al colpo questo viene **bloccato
per profilo inattivo**, senza rimuoverlo silenziosamente o spendere il costo.
Occorre rivedere la scelta. I riepiloghi dei danni già applicati conservano
lo snapshot dell'impatto e non sono riscritti dalle scadenze successive.

Su errore di salvataggio lo stato visibile resta quello precedente; confermare
di nuovo riparte da quello stato, senza avanzamenti parziali in RAM. Il
controller NVS reale blocca ulteriori scritture dopo un errore I/O e un commit
segnalato fallito può avere già scritto il candidato: occorrono riavvio e
verifica. Non è uno storico degli eventi né una garanzia di deduplicazione
dopo un'interruzione dell'alimentazione.

Import USB, attenuazione e scorciatoie non interrompono le nuove conferme o
il dettaglio. PWR breve e BOOT breve/lungo conservati e configurabili; PWR lungo
hardware. Importare una nuova proiezione con conferma sostituisce i timer locali;
annullare conserva tutto. Raccolta PC conservata, nessuna nuova lettura Beyond.

## Persistenza e verifiche

Scritture **PDD19**, tag **PDDJ**, dimensione **8429 + 552 × numero voci**, massimo
**74669 byte**. Prima del CRC si aggiungono **50 selettori uint8**, nell'ordine:
8 effetti numerici, 39 extra difensivi (slot × 13 + tipo), 3 profili di riduzione.
Valori: 0 Round, 1 Inizio, 2 Fine; senza timer deve essere 0. I campi durata e
conteggio precedenti restano agli stessi offset. I conteggi interni continuano
a usare il campo storico rounds. Nessun dump di struct.

Legge **PDD1–PDD18 senza scrivere al boot**: timer precedenti ancora a Round,
manuali senza scadenza, costi/reazione e altri dati conservati. Fixture PDD18
sintetica dell'encoder v0.38 originale `807ccff`, prima delle modifiche;
prima scrittura nella banca alternata e riavvio verificati. Le nuove scritture
richiedono v0.39 o successivo; non cancellare NVS per aggirare un downgrade.
**USB v7, backup PC v10 e layout flash invariati.**

10800 casi indipendenti di famiglia/indice/fase/conteggio/pausa/reazione/vincolo;
50 scadenze simultanee con bit alti, rifiuto atomico, limite PF senza cura,
riposi separati, record massimo e tutti i 50 selettori corrotti con CRC valido.
UI di configurazione dei tre tipi, inizio/fine, confronto/dettaglio, errore/
riprova/annulla, 17 pagine, colpo preparato e import; tasti e confini circolari.
[Verifiche complete](FIRMWARE.md#verifiche-v039--durate-del-proprio-turno).
Catture host e compilazione non costituiscono collaudo sul dispositivo fisico.

Questo completa la prima gestione delle durate del proprio turno nel blocco
effetti. Altre durate, cariche degli oggetti, condizioni complete, requisiti
ed effetti automatici restano successivi; batteria, sospensione/spegnimento
e collaudo fisico rimangono aperti.
