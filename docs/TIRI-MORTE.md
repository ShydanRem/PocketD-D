# Tiri contro morte: circostanze e bonus espliciti — v0.57

Il dispositivo conserva **Lancia d20 base** e aggiunge **Modifica** nel registro
morte: vantaggio, svantaggio e bonus numerico per il singolo tiro. Funziona
offline, mantenendo grafica del compagno, animazioni dei dadi e tasti
configurabili. Il PC resta uno strumento di importazione, configurazione e backup.

## Preparazione e anteprima

Aprire **Usi → Stato → Salvezze contro morte → Modifica**. I due interruttori
dichiarano la presenza di almeno una fonte di vantaggio e/o svantaggio,
verificata dal giocatore con il DM. Fonti multiple dello stesso segno non si
sommano: con entrambi attivi si tira un solo d20. Con solo vantaggio si tirano
due d20 e si sceglie il maggiore; con solo svantaggio si sceglie il minore.

**Bonus totale**, da −30 a +30, è la somma numerica esplicita applicabile al
tiro. Non viene aggiunta una caratteristica, competenza o salvezza della
scheda. Avvelenato, Ira e gli effetti locali di Accecato/Spaventato/Prono/
Trattenuto non determinano la modalità di questo tiro. Non viene interpretato
il testo di oggetti, incantesimi, capacità o condizioni importate.

**Anteprima** riassume modalità effettiva, bonus e regola. Tornare indietro
prima del lancio permette di modificare la bozza senza estrarre dadi o scrivere
flash. Indietro dalla preparazione la scarta. Ogni nuova apertura di Modifica
riparte senza circostanze e con bonus zero; **Lancia d20 base** usa sempre
un solo dado e bonus zero, anche dopo un tiro modificato.

Le conferme richiedono il touch. Scorciatoie PWR/BOOT non lanciano, confermano
o escono dalla preparazione/conferma. Import e ripristino ricevono `busy`.
Durante l'animazione Lancia può accelerare il volo mantenendo il risultato.

## Naturale e totale sono distinti

La regola rappresentata resta il tiro morte base con soglia 10:

| Dado naturale scelto | Esito |
| --- | --- |
| 1, anche con bonus +30 e totale 31 | Due fallimenti, fino a tre |
| 20, anche con bonus −30 e totale −10 | Riprende 1 PF e azzera il registro |
| Da 2 a 19, totale almeno 10 | Una riuscita; la terza stabilizza e azzera i contatori |
| Da 2 a 19, totale sotto 10 | Un fallimento, fino a tre |

Per esempio, 9 naturale con bonus +1 è una riuscita; 10 naturale con bonus −1
è un fallimento. Con vantaggio e dadi 1/9, il naturale scelto è 9: il dado
scartato non aggiunge due fallimenti. Con svantaggio e dadi 20/10 il naturale
scelto è 10: il 20 scartato non cura.

Il risultato mostra naturale, bonus e totale separati dall'esito. I due dadi
rimangono visibili nell'animazione e nello storico, insieme alla modalità
effettiva e al totale. La ricevuta dell'esito non viene ricalcolata leggendo
uno stato della scheda modificato successivamente.

## Persistenza, errori e annullamento

Solo **Conferma** estrae il tiro, prepara PF/registro e salva la scheda prima
dell'animazione. Se la scrittura fallisce, lo stato mostrato resta quello
precedente, ma il tiro è fissato in RAM e viene mostrato nella conferma.
**Riprova** tenta di salvare la stessa candidata senza usare altro RNG;
Indietro e modifiche sono bloccati dopo l'estrazione, anche nel percorso base.
Un riavvio perde la candidata in RAM: non è un registro persistente dei tentativi
falliti. Un errore I/O reale può bloccare il controller fino al riavvio e
richiede diagnosi; il nuovo tentativo del test host non certifica il recupero
di una scrittura interrotta sull'hardware.

Lo storico viene scritto dopo la scheda con il meccanismo già presente: i
due salvataggi non sono una singola transazione. Se fallisce solo lo storico,
l'esito sulla scheda è già applicato; compare un tiro pendente e un nuovo
lancio viene bloccato prima di RNG finché non si risolve la copia pendente.
Riprova nello storico registra il tiro senza applicare nuovamente l'esito.
[Garanzie e limiti dello storico](STORICO-TIRI.md).

**Eroe → Annulla** ripristina l'intera scheda precedente, mentre il tiro già
avvenuto resta nello storico. Non è un comando di rilancio. Anche in caso
di canvas non disponibile l'esito è salvato e mostrato senza estrarre altri dadi.

Il registro deve essere disponibile, a zero PF, non stabilizzato e con entrambi
i contatori sotto tre. Schede precedenti senza registro, PF positivi, tre
successi non stabilizzati e tre fallimenti bloccano entrambe le vie prima di RNG.

## Compatibilità e limiti

PDD25, PDH2 con lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Le opzioni sono una bozza in RAM per
quel tiro; non sono nuovi effetti persistenti né proprietà trasferite dal PC.
PDH2 rappresenta già i due dadi, la modalità e il bonus firmato. Le API
preesistenti `pd_death_face` e `pd_death_roll` conservano il comportamento base.

Bonus a dado aggiuntivo, reroll selettivi, dadi sostitutivi, soglie alternative,
capacità che evitano morte/zero PF e tutti gli effetti dell'incoscienza restano
manuali. Questo incremento permette di dichiarare alcune circostanze, senza
ricostruire automaticamente le eccezioni di un personaggio o scegliere
l'edizione dell'intera scheda. Per un caso non rappresentabile usare la
correzione del registro con il DM. I danni ricevuti a zero PF seguono il
[percorso separato della v0.56](DANNI-ZERO-PF.md).

## Verifiche

Il nuovo test core confronta 102.020 casi con un oracolo indipendente:
tutte le coppie di d20, quattro combinazioni di circostanze, sette bonus
limite/soglia e nove registri, più tutti i bonus −30…+30 su ogni faccia.
Verifica priorità dei naturali, mancata modifica della sorgente, input
invalidi senza RNG, compatibilità delle API base e codec PDH2.

La UI verifica preparazione/annullamento, bonus estremi, import/backup e tasti
bloccati, vantaggio/svantaggio/annullamento, scrittura fallita con tiro fissato,
tentativi ripetuti, un solo aggiornamento di scheda/storico, annullamento,
ricevuta congelata, canvas assente e recupero del solo storico pendente.
Gli esiti della suite e della compilazione sono in [FIRMWARE.md](FIRMWARE.md).

Le catture `death-roll-*.png` sono schermate del firmware LVGL su host con
dati sintetici. Nessun flash, prova fisica, lettura Beyond o modifica della
raccolta personale è stato eseguito. Il futuro controllo fisico H24 è in
[COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).
