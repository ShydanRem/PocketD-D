# Nomi dei riferimenti di turno — v0.59

Sul dispositivo si possono nominare i tre riferimenti esterni **A/B/C**,
senza cambiare il riferimento usato dai timer. Il nome è un'etichetta locale:
non rileva identità, iniziativa, bersagli o partecipanti del party. Grafica del
compagno e assegnazioni dei tasti restano quelle approvate.

## Editor sul display

Aprire **Usi → Stato → Reazione → Altri turni**, scegliere A/B/C e toccare
il pulsante del nome. L'editor offre lettere maiuscole/minuscole, numeri,
spazio, apostrofo, trattino e punto in sei pagine di dodici posizioni.
Le frecce cambiano pagina; un carattere viene aggiunto in fondo, **Cancella**
rimuove l'ultimo. Il limite è **20 caratteri ASCII**. Al limite, i caratteri
sono disabilitati e resta possibile cancellare o cambiare pagina.

**Anteprima** rimuove gli spazi iniziali/finali e mostra nome precedente,
nuovo nome, numero di timer assegnati e numero di quelli attivi. Un nome
vuoto torna a **senza nome**. Indietro dall'anteprima conserva la bozza;
Indietro dall'editor la scarta. **Conferma** salva il nome nel personaggio
locale. Salvare lo stesso nome non scrive flash e conserva l'annullamento
precedente. Se la scrittura fallisce, la scheda resta invariata in RAM e la
bozza rimane disponibile per un nuovo tentativo o per essere scartata.

Il nome completo è visibile nelle viste con testo a capo dell'editor e della
conferma; il pulsante compatto della selezione può abbreviarlo con puntini.
L'anteprima di inizio/fine del turno esterno mostra anche il nome su una riga
dedicata, a capo quando serve. Restano sempre visibili A/B/C per distinguere
anche etichette uguali.

## Nomi e riferimenti sono separati

Rinominare **A** non sposta timer verso B, non modifica conteggi, non avanza
turni e non cambia PF, CA, concentrazione, reazione, slot, cariche o risorse.
Le definizioni inattive restano assegnate allo stesso riferimento. Il conteggio
in anteprima serve a verificare l'associazione prima di rinominare.

Se il nome indica un partecipante diverso da quello rappresentato finora,
rivedere i timer con il DM: rinominare non è un comando per riassegnarli.
La scelta di chi rappresentano A/B/C resta esplicita e manuale. Un'etichetta
uguale su due riferimenti non li unisce.

**Eroe → Annulla** ripristina l'intera scheda precedente, incluso il nome.
Annullare successivamente una scadenza ripristina timer/PF conservando il nome
che era già salvato prima dell'evento. Nominare non usa RNG e non aggiunge
elementi allo storico dei tiri. Tasti, import e ripristino non possono
interrompere editor o conferma. PWR lungo resta riservato all'hardware.

Un errore I/O reale può bloccare il controller fino al riavvio e può avere già
scritto una candidata prima di segnalare il problema: la prova host di nuovo
tentativo non certifica il recupero dopo un'interruzione dell'alimentazione.
Riavvio e verifica della banca valida restano necessari in quel caso.

## Persistenza e backup

Scritture **PDD27 / PDDR**, dimensione `8942 + 552 × numero poteri`, massimo
**75.182 byte**. Dopo `history_epoch` e prima del CRC si aggiungono 63 byte:
tre campi da 21 byte, A/B/C, con terminatore e riempimento zero. Un nome
valido ha 0–20 byte ASCII stampabili, senza spazi esterni; controlli,
byte non ASCII, terminatore mancante e dati non nulli dopo il terminatore
sono rifiutati. L'API locale normalizza gli spazi prima di salvare. Il formato
può conservare altri segni ASCII stampabili già presenti in un backup; la
tastiera sul display offre il sottoinsieme elencato sopra.

I selettori dei timer introdotti nella v0.58 conservano posizione e significato.
PDD25/PDD26 e tutti i precedenti formati supportati restano leggibili: i nomi
iniziano vuoti, senza inventare un'identità. I vecchi firmware non leggono
PDD27; conservare i backup e usare l'app di supporto aggiornata per il formato.

Il backup nativo v1 conserva i nomi e accetta PDD25/PDD26/PDD27. Anteprima PC
e trasporto Python riconoscono il nuovo campo finale e l'epoca dello storico
nella posizione corretta. L'anteprima PC mostra le tre etichette e conserva
i record originali; la validazione semantica completa resta sul dispositivo.
Un ripristino di una copia precedente senza nomi li lascia vuoti.

USB9, PDH2 con lettura PDH1, backup della raccolta PC v11, diagnostica v1 e
partizioni restano invariati. Una nuova importazione confermata sostituisce
le definizioni locali e i nomi come il resto della copia dispositivo;
annullarla conserva tutto. La raccolta PC non viene modificata.

## Limiti e verifiche

Restano tre riferimenti, associazione manuale, nessuna gestione dell'ordine
dei turni e nessun collegamento a identità remote. Accenti/Unicode, inserimento
nel mezzo del nome e nomi più lunghi non sono rappresentati da questo editor.
Il blocco effetti resta aperto per altre regole e automazioni.

Il nuovo core verifica **28.160** combinazioni indipendenti di riferimento,
lunghezza e byte, normalizzazione, input non validi, auto-copia e mancata
modifica degli altri campi. Codec verificato con 0/1/120 poteri, migrazione
PDD25/PDD26, nomi malformati con CRC valido e backup nativo. La UI esercita
le sei pagine, nomi lunghi a capo, modifica/annullamento, errori di scrittura,
salvataggio invariato senza scrittura, cancellazione, annullamento completo,
anteprima del turno nominato e ripristino effettivo nel banco host.

Un backup PDD27 generato dal C con 120 poteri sintetici, tre nomi e tutti i
50 riferimenti è letto da PC e validatore Python conservando record, nomi,
conteggi ed epoca. Non vengono aperte porte seriali in questa prova.
Gli esiti complessivi sono in [FIRMWARE.md](FIRMWARE.md).

Le catture `turn-name-*.png` sono native LVGL su host con dati sintetici.
Nessun flash, prova fisica, lettura Beyond o modifica della raccolta personale.
Il controllo futuro H26 è in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).
