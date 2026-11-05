# Magie in combattimento · v0.110

Dal Pocket: **Magie → scegli la magia → Lancia → Attacco oppure TS nemico**.
Restano disponibili Danni diretti, Cura me, Cura altri e Solo costo. I profili
speciali, fra cui True Strike, mantengono il proprio percorso.

## Attacco con una magia

1. Usa la formula salvata oppure configura i dadi per questo lancio.
2. Imposta la CA del bersaglio e, se serve, vantaggio o svantaggio. Se sei
   spaventato dichiara anche se la fonte della paura è visibile.
3. Rivedi formula, slot ordinario/patto, utilizzi e conseguenze sulla
   concentrazione; **Lancia e salva** conferma il costo e il d20 insieme.
4. Sul risultato, **Danni** tira la formula se hai colpito; **Riepilogo** mostra
   la ricevuta. La consultazione e le due parti di un tiro grande non ritirano.

Il bonus è quello della magia: caratteristica di lancio, competenza e bonus
agli attacchi con incantesimo rappresentati nella scheda. Non usa l'arma attiva,
FOR o il danno della rabbia. Le condizioni rappresentate e il vantaggio richiesto
concorrono al risultato; vantaggio e svantaggio si annullano. Distanza, copertura
e altre circostanze del bersaglio restano da dichiarare al tavolo.

1 naturale manca; 20 naturale colpisce ed è critico. Il critico raddoppia solo
i dadi, con il bonus applicato una volta: fino a 36 dadi, presentati in due
parti quando superano 18. Il totale potenziale comprende entrambe le parti.
Un mancato consuma comunque slot/utilizzi e non genera dadi di danno.

## Tiro salvezza del bersaglio

Seleziona esplicitamente la caratteristica del TS e cosa prescrive la magia su
un successo: **Zero danni** oppure **Metà danni**. Nessuna delle due scelte è
preselezionata. La CD visualizzata deriva dalla scheda del lanciatore.
Dopo **Lancia e salva**, il costo è già speso: dichiara **Fallito** o **Riuscito**
secondo il tiro effettivo del bersaglio, effettuato dal giocatore o dal DM.

- Fallito: danno pieno.
- Riuscito con metà: si tira una sola formula e si dimezza il totale completo,
  arrotondando per difetto. Nel registro restano i dadi originali e una nota.
- Riuscito con zero: nessun dado di danno viene generato o aggiunto al registro.

Il Pocket non inventa un d20, competenze o modificatori del bersaglio. Le
condizioni del lanciatore che influenzano i suoi attacchi non diventano
svantaggio al TS nemico.

## Costi, riprove e limiti

Il costo si salva una volta, anche sul mancato o sul TS riuscito. Un errore di
salvataggio dell'attacco conserva il medesimo d20 per la riprova. Un errore dello
storico conserva i risultati in attesa senza nuovi dadi o nuove spese; le parti
di un critico grande vengono registrate insieme. Cambi della scheda fra
anteprima e conferma bloccano la proposta ormai superata.

La risoluzione pagata è temporanea: chiudere o riavviare prima dei danni lascia
il costo speso e non rimborsa né riprende automaticamente il lancio. L'Annulla
standard della scheda può ripristinare lo stato precedente; i tiri già annotati
restano fatti del registro. Per riprendere un errore dello storico usa il
percorso esistente, prima di generare altri risultati.

**Attacco e TS sono scelte esplicite per ogni lancio.** Non sono dedotti dal
nome, dai dadi o dal testo Beyond. Questa versione non aggiunge identità di
magie al riconoscimento automatico della v0.109. La formula può essere salvata,
ma CA del bersaglio, modalità, caratteristica TS e regola sul successo non lo sono.

Si mostrano **danni potenziali**: PF, resistenze, immunità, vulnerabilità e altre
difese del bersaglio non vengono applicati. Più bersagli, attacchi multipli,
formule con più tipi di dado/danno, bonus di classe/oggetti non già rappresentati,
colpi critici speciali e proprietà del bersaglio restano manuali. Preparazione,
componenti, azioni e limiti specifici richiedono ancora il controllo del tavolo.

## Aggiornamento e verifiche

Aggiorna dal sito con **Conserva dati**. Nessun nuovo formato di scheda,
protocollo USB o backup rispetto alla v0.109: profili, PG, look SD e assegnazioni
dei tasti conservati. Nessuna reimportazione o nuova installazione dei look
necessaria. Non occorre aggiornare l'estensione per questo flusso.

Verifiche con dati sintetici: 11.520 combinazioni di attacco/costo; 2.592 casi di
danno pieno/metà/zero/critico; condizioni confrontate con il motore condiviso;
percorsi LVGL con riprova dello stesso tiro, errore dello storico, critico a 36
dadi, costo sul mancato, TS senza RNG, scelta obbligatoria delle regole,
annullamento, navigazione fisica e blocco di anteprime superate. Catture della
UI nativa eseguita su PC, non fotografie del dispositivo.

Compilazione ESP32, suite nativa/JavaScript e controlli sanitizzati sono riportati
nel [resoconto software](STATO-SOFTWARE.md). In questo aggiornamento nessuna
nuova lettura reale Beyond, scrittura verso Beyond o prova USB/hardware.
Le prove reali precedenti di importazione non certificano questi nuovi percorsi.
