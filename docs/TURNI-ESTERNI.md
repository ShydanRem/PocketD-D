# Durate legate a soggetti esterni — v0.58

Effetti numerici, extra difensivi e profili di riduzione possono ora attendere
l'inizio o la fine del turno di un riferimento esterno **A, B o C**. Il proprio
turno rimane distinto. Tutto funziona sul dispositivo offline; grafica del
compagno e assegnazioni PWR/BOOT sono conservate.

## Associare e configurare

A/B/C sono tre riferimenti locali, da associare esplicitamente ai partecipanti
con il DM. **Non sono nomi o identità rilevati dal dispositivo**: non esistono
ancora nomi personalizzati, collegamento a un party o lista di iniziativa.
Conservare la stessa associazione finché esistono timer che la usano. Cambiare
chi rappresenta A senza rivedere quei timer li farebbe avanzare al turno sbagliato.

Negli editor Durata degli effetti e delle difese, scegliere **inizio turno**
o **fine turno**, poi il pulsante **Turno: mio → A → B → C → mio**. Nei profili
di riduzione il pulsante è nella modifica dei **Passaggi residui**. Il riepilogo
mostra, per esempio, `inizio A`, `fine B` o i marcatori compatti `iA`/`fB`.

Un timer attivo richiede 1–99 passaggi. Cambiare soggetto riparte da 1;
cambiare modalità della durata riparte da 1 e dal proprio turno, oppure dai
valori già previsti per manuale/minuti/ore. Round, minuti, ore e durata manuale
non possono avere un riferimento esterno. La configurazione resta una bozza
fino ad Anteprima e Conferma; non avanza timer e non recupera risorse.

## Avanzare l'evento giusto

Da **Usi → Stato → Reazione → Altri turni**, selezionare A/B/C e poi
**Inizio turno** o **Fine turno** del riferimento scelto. L'anteprima indica
il soggetto e mostra timer coinvolti/scaduti, reazione, CA, PF massimi e PF
correnti. **Scadenze** elenca ogni voce coinvolta, fino a 50 su 17 pagine.
Indietro lascia tutto invariato; Conferma salva l'intero candidato.

| Evento confermato | Timer che avanzano | Reazione del personaggio |
| --- | --- | --- |
| Inizio del proprio turno | Inizio, soggetto mio | Torna disponibile |
| Fine del proprio turno | Fine, soggetto mio | Conservata |
| Inizio A/B/C | Inizio, solo quel riferimento | Conservata |
| Fine A/B/C | Fine, solo quel riferimento | Conservata |
| Round o tempo trascorso | Solo le rispettive modalità, senza soggetto esterno | Conservata |

Voci inattive restano in pausa. Una scadenza disattiva la voce, senza cancellare
regola, costi o riferimento. La fine della concentrazione continua a disattivare
e scollegare insieme tutte le voci vincolate, qualunque sia il riferimento;
la sola scadenza del timer non termina la magia concentrata. Nessun turno
recupera slot, usi o cariche automaticamente.

Esempio sintetico: PF 42, massimo base 32 e bonus +10 fino a fine A. Confermare
inizio A o fine B non cambia quel bonus. Confermare fine A lo termina e limita
i PF a 32. Un bonus CA fino a inizio B resta indipendente; l'inizio di B non
recupera la reazione del personaggio.

## Salvataggi e limiti

Se una scadenza rendesse CA o massimo PF fuori limite, nessun timer, PF o
reazione cambia. Su errore di scrittura la RAM visibile conserva lo stato
precedente; il nuovo tentativo ricostruisce lo stesso avanzamento da quello
stato. Il controller reale può bloccarsi dopo errore I/O e un commit fallito
può avere già scritto il candidato: riavvio e verifica restano necessari.
**Eroe → Annulla** ripristina tutta la modifica della sessione.

Non c'è RNG o scrittura nello storico dei tiri. Un evento senza cambiamenti
non scrive flash. Una conferma accodata dopo l'uscita dall'anteprima non
applica nuovamente l'evento; aprire e confermare un nuovo passaggio rappresenta
invece un altro turno. Non esiste deduplicazione persistente degli eventi,
né controllo dell'ordine dell'iniziativa o del rapporto tra inizio e fine.

Scorciatoie, import e ripristino non interrompono selezione/anteprima/dettagli.
La diagnostica USB resta leggibile. Le assegnazioni dei tasti restano invariate.
Una nuova importazione confermata sostituisce le definizioni locali come prima;
annullarla conserva tutto. La raccolta personale PC non viene modificata.

Restano manuali l'associazione dei tre riferimenti, più di tre soggetti esterni,
la scelta se contare il turno corrente o il prossimo, iniziativa, identità del
bersaglio/incantatore e interpretazione del testo delle capacità. Questo chiude
un percorso esplicito del blocco effetti, non un motore completo delle durate.

## Formato e compatibilità

Le nuove scritture sono **PDD26 / PDDQ**, con la stessa dimensione della v0.57:
`8879 + 552 × numero poteri`, massimo 75.119 byte. I 50 byte già dedicati ai
selettori delle durate contengono ora `tick | (owner << 3)`: tick 0–4 nei tre
bit bassi, owner 0–3 nei due successivi; i bit restanti sono invalidi. Owner
non zero richiede una durata temporizzata di inizio/fine turno. Ordine:
8 effetti, 39 extra difensivi (slot × 13 + tipo), 3 profili. I validatori
rifiutano anche proprietari su posizioni numeriche non utilizzate.

PDD25 e tutti i salvataggi precedenti supportati restano leggibili: owner
inizia a zero, senza inventare riferimenti esterni. PDD25 con bit riservati
non validi continua a essere rifiutato. I vecchi firmware non leggono PDD26:
conservare backup e usare firmware/app di supporto aggiornati per il nuovo formato.

Il backup nativo v1 accetta PDD25 e PDD26, conserva i record completi e li
valida prima del ripristino. L'anteprima PC e lo strumento Python accettano
entrambi. La verifica semantica completa rimane sul dispositivo. PDH2/lettura
PDH1, USB9, backup della raccolta PC v11, diagnostica v1 e partizioni invariati.

## Verifiche

Il nuovo core confronta 38.400 casi indipendenti per le 50 posizioni, quattro
soggetti, due fasi, pause, conteggi, reazione e vincoli alla concentrazione.
Verifica isolamento tra gli orologi, cap PF, rifiuto atomico di scadenze non
valide, tutti i selettori serializzati, lettura PDD25 e backup nativo.
Il backup PDD26 generato realmente dal C viene letto dal decoder PC con
50 selettori conservati (A=13, B=13, C=12) e accettato dal validatore Python,
con dati sintetici e senza aprire una porta seriale.
La UI esercita i tre editor, azzeramento del riferimento cambiando modalità,
selezione A/B/C, anteprima/dettagli/annullamento, errore di scrittura e nuovo
tentativo, annullamento completo, evento vuoto senza scrittura, turno proprio
distinto dagli esterni e tutte le 50 scadenze fino all'ultima pagina.

Gli esiti di regressione e compilazione sono in [FIRMWARE.md](FIRMWARE.md).
Le catture `external-turn-*.png` sono native LVGL su host con dati sintetici.
Nessun flash, collaudo fisico, lettura Beyond o modifica della raccolta personale.
Il controllo fisico futuro H25 è in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).
