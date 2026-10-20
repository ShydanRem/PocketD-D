# Durate e scadenze degli effetti — v0.27

Questo incremento estende gli otto [modificatori locali CA/PF massimi](EFFETTI-LOCALI.md)
con una durata esplicita. Funziona nel firmware offline; non serve Foundry, un
server o il PC durante il gioco. Il modello numerico e i valori base importati
rimangono quelli della v0.26.

## Durata manuale o a round

**Usi → Stato → Effetti → voce/Aggiungi → Durata** apre la bozza della durata:

- **Manuale**, predefinita anche per gli effetti già salvati: nessuna scadenza.
- **Round**, con **0–99 round residui**. Un effetto attivo deve averne almeno uno;
  zero è lo stato scaduto e può essere salvato soltanto come inattivo.

Cambiare Manuale → Round imposta un round; Round → Manuale azzera il conteggio
ed elimina la scadenza. I comandi −10/−1/+1/+10 operano soltanto sulla bozza.
Indietro torna al modificatore; Elenco abbandona tutta la bozza. Anteprima mostra
la durata scelta e i cambiamenti a CA/PF; solo Conferma scrive. Modificare la
destinazione CA/PF azzera il bonus come prima, ma conserva la durata della bozza.

Un effetto disattivato con round residui è **in pausa**: non contribuisce ai
valori e non perde round. Riattivarlo riprende dal conteggio conservato. Un effetto
**scaduto** conserva la voce e il suo valore, ma resta inattivo a zero round.
Per riattivarlo occorre impostare una nuova durata positiva oppure scegliere
esplicitamente Manuale. Nessuna riattivazione implicita o reinizializzazione
silenziosa del timer.

L'elenco distingue manuali attivi/inattivi, round residui, pausa e scaduto.
Non ci sono un cronometro, una conversione da minuti o una lettura della durata
nel testo di una magia. Lasciare aperta una schermata o spegnere il dispositivo
non fa trascorrere round.

## Avanzare un round

**Effetti → 1 round** apre l'anteprima. Il comando è disponibile quando esiste
almeno un effetto attivo a round e decrementa **tutti** questi effetti insieme.
Il giocatore concorda con il DM il momento in cui usarlo: non è una gestione
automatica dell'iniziativa né della distinzione inizio/fine turno.

La conferma mostra gli indici delle voci che avanzano, quelle che scadono e i
valori CA, PF massimi e PF correnti prima/dopo. **Indietro non consuma round**.
Conferma applica decrementi, disattivazioni e limite ai PF correnti in un unico
salvataggio. Dopo il successo torna all'elenco: un evento di conferma ripetuto
non avanza un secondo round.

Quando il massimo scende, i correnti sono limitati al nuovo massimo, con perdita
indicata prima di confermare. Quando il massimo cresce, anche per la scadenza
di una penalità, non vengono restituiti PF. I temporanei restano invariati.

Le scadenze vengono valutate **simultaneamente** prima di limitare i PF: due
effetti opposti che terminano insieme non producono una riduzione intermedia.
Se il totale finale uscirebbe da CA 1–40 o PF massimi 1–999, **l'intero round è
rifiutato**, senza avanzare nemmeno i timer che non scadono. Correggere gli
effetti o la loro durata e riprovare; non si scelgono arbitrariamente le scadenze
da ignorare. I contatori non vengono corretti dal nome di una condizione.

## Tasti, salvataggio e interazioni

**Eroe → Aspetto → Tasti** aggiunge l'assegnazione **Round** a PWR breve, BOOT breve
e BOOT lungo. Apre soltanto l'anteprima; **la conferma rimane touch**. Senza effetti
attivi a round il comando non modifica nulla. Durante un'animazione non interrompe
il tiro; durante una conferma i tasti sono assorbiti. PWR lungo rimane riservato
all'alimentazione. Le assegnazioni precedenti mantengono il loro significato.

Durante l'anteprima del round USB risponde busy e l'attenuazione è sospesa.
Il primo gesto di risveglio continua a riattivare soltanto lo schermo.

Un errore di salvataggio mantiene lo stato visibile, compresi tutti i conteggi.
Riprovarlo nella stessa anteprima calcola lo stesso avanzamento a partire dallo
stato non modificato, senza dadi o entropia. Come prima, un commit NVS fallito
può essere ambiguo: verificare dopo il riavvio. **Eroe → Annulla** ripristina insieme
timer, effetti e PF dell'ultima modifica di sessione. Al riavvio si riprende dal
conteggio salvato; non si sottrae il tempo trascorso a dispositivo spento.

Riposi, zero PF, fine concentrazione e comandi ad altre risorse non avanzano i
round né terminano questi effetti. Restano indipendenti dalle magie e dalle
annotazioni delle condizioni. Il giocatore gestisce esplicitamente eventuali
legami con concentrazione, durata di una capacità o validità di un riposo.

Nuove copie USB sostituiscono effetti e durate locali dopo avviso/conferma;
annullare conserva tutto. Preferenze del dispositivo e raccolta PC rimangono
conservate. Il backup di ritorno dei progressi dal dispositivo al PC **non è
ancora disponibile**: il backup PC non contiene le nuove modifiche locali.

## Formato e compatibilità

USB **v7** e backup PC **v10** restano invariati. Le nuove scritture locali sono
**PDD10**, con tag binario a quattro byte **PDDA** (A = versione 10):
**7966 + 552 × numero magie/capacità**, massimo **74206 byte**.

Il payload di PDD9 rimane nello stesso ordine; prima del CRC si aggiungono otto
coppie di byte `timed` booleano stretto / `rounds` 0–99. Manuale richiede zero
round, attivo a round ne richiede almeno uno. Le voci non usate hanno durata
manuale/zero. La nuova scorciatoia locale 11 è Round; Effetti rimane 10.
Non si serializzano direttamente struct o padding.

PDD1–PDD9 rimangono leggibili senza riscrittura al boot. PDD9 conserva bonus,
attivazione e PF superiori alla base; le durate diventano manuali. Il fixture
PDD9 proviene dall'encoder reale del commit **6999e80**, prima delle modifiche,
con soli dati sintetici. La prima scrittura nuova conserva il record precedente
nell'altra banca. Layout flash e pdd_state da 256 KiB invariati.

Un downgrade alla v0.26 incontra un formato sconosciuto e blocca il caricamento
per conservare i dati: non cancellare NVS per aggirarlo. Nessun flash eseguito qui.

## Verifiche e limiti

- **40648 avanzamenti/sottoinsiemi indipendenti**: durate 1–99, entrambe le
  destinazioni, segni e attivazioni, tutti i 256 sottoinsiemi di otto scadenze.
- Pausa, scaduto, riattivazione esplicita, scadenze opposte simultanee, nessun
  limite PF intermedio, rifiuto atomico con maschere di output conservate.
- Massimo maggiore senza cura; temporanei invariati, morte/riposi isolati,
  120 magie, otto timer, campi malformati con CRC valido e output intatti.
- Migrazioni golden PDD1–PDD9 senza scritture al boot; NVS con bonus vecchi,
  nuovo timer, riavvio e scadenza persistita.
- UI LVGL su host: bozza, cancel/conferma, errori e tentativi ripetuti, annulla,
  scadenze, pausa, zero, import busy, scorciatoia e attenuazione protetta;
  pulsanti nel cerchio alti almeno 44 px.
- Regressioni: 138 JS, build PC, 37751 confronti PC→USB→C e 17266 casi degli
  effetti numerici; ASan/UBSan e compilazione ESP32-S3. Leak detector disattivato
  nel sandbox; immagini dal renderer host, non fotografie dell'AMOLED.

Questo completa il **conteggio manuale a round degli effetti numerici locali**,
non tutte le durate D&D. Mancano inizi/fini turno distinti, minuti/ore, legami con
concentrazione e condizioni, resistenze/immunità, proprietà speciali e regole
complete. I prossimi incrementi restano nel blocco effetti; grafica e comandi
fisici accompagnano le viste. Batteria/sospensione e collaudo fisico restano aperti.
Nessuna nuova lettura personale Beyond o modifica della raccolta personale.
