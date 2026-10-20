# Salvezza di concentrazione nel flusso danni — v0.29

> Documento della v0.29. La v0.30 aggiunge [vincoli degli effetti alla concentrazione](EFFETTI-CONCENTRAZIONE.md)
> e PDD12; i formati e i limiti sui collegamenti sotto descrivono lo stato storico.

La v0.29 aggiunge **Tira COS** all'anteprima dei [danni ricevuti](DANNI-DIFESE.md).
Il dispositivo può estrarre il dado, confrontare il totale con la CD e salvare
insieme danni, temporanei e concentrazione prima dell'animazione. Funziona offline,
senza Foundry o PC al tavolo. La scelta manuale Esito rimane disponibile.

## Quando il tiro è disponibile

Serve una componente di danno già configurata, danno effettivo positivo,
concentrazione in corso e PF ancora positivi dopo il colpo. I danni assorbiti dai
temporanei richiedono comunque la verifica. Immunità e resistenza che arrotonda
il danno a zero non estraggono dadi. I casi già a zero PF, possibile morte
istantanea e discesa a zero non attivano questo tiro: conservano i blocchi o la
fine concentrazione del flusso v0.28.

**Tira COS** è abilitato soltanto se la salvezza COS è disponibile e non è
segnata da verificare. Si usano i dati espliciti della scheda, non il testo di
una capacità. Le vecchie copie senza formazione non diventano salvezze senza
competenza: l'anteprima indica **COS da verificare**, non estrae dadi e conserva
la risoluzione manuale con il DM. Un blocco su un'altra salvezza non blocca COS.

## Bonus, modalità e CD

Tira COS apre una conferma che espone COS, contributo della competenza, extra
importato e totale. Si può aggiungere un **extra locale da −30 a +30**, inizialmente
zero a ogni nuovo ingresso, per una variazione non già inclusa nella scheda.
Questo non modifica i bonus importati e non viene salvato come preferenza.

**Base/Vantaggio/Svantaggio** è una scelta esplicita. Avvelenato e il vantaggio
alle prove FOR non modificano questa salvezza COS. Talenti o condizioni che
concedono vantaggio alla concentrazione non vengono dedotti dai nomi: scegliere
la modalità dopo verifica col DM. Bonus a dadi, reroll, minimi garantiti, successi
automatici o altre eccezioni non vengono interpretati.

Il pulsante CD alterna due criteri, con il valore effettivo sempre visibile:

- **CD base**, iniziale: `max(10, danni effettivi / 2 per difetto)`, senza limite
  superiore, come nel criterio 2014 già usato dal flusso danni.
- **CD max 30**: la stessa formula limitata a 30, come nel criterio 2024.

La scelta riguarda solo questa verifica, non cambia il regolamento dell'intera
scheda. Non viene inferita da Beyond o dal nome della magia, né conservata tra
nuovi ingressi. Per altre CD/eccezioni usare la risoluzione manuale. La salvezza
usa il danno effettivo prima dell'assorbimento dei temporanei, non i soli PF persi.

Il totale è faccia scelta + COS + competenza + extra scheda + extra locale.
**Totale ≥ CD mantiene concentrazione; altrimenti la termina.** Naturale 1 e 20
non applicano fallimento/successo automatici: un 1 con bonus sufficienti può
riuscire e un 20 con penalità può fallire. L'effetto grafico del 20 è decorativo.

## Conferma, dado e applicazione

Prima dell'estrazione, Indietro torna all'anteprima danni senza consumare entropia,
modificare PF o salvare. L'esito manuale viene lasciato da risolvere quando si
entra nel percorso del tiro. Si può uscire e scegliere il risultato col DM.

**Tira e applica**:

1. Ricontrolla disponibilità della salvezza, piano, modalità e bonus prima del RNG.
2. Estrae uno o due d20 e sceglie la faccia secondo la modalità.
3. Prepara un solo candidato con danni, temporanei e risultato della concentrazione.
4. Salva quel candidato e soltanto dopo avvia i dadi pixel art.

Il risultato mostra bonus, CD e concentrazione mantenuta/persa. Accelerare il
volo, ridisegnare o premere Lancia non estrae altri dadi e non riapplica il danno.
Riepilogo apre i valori del colpo; per un altro colpo servono Nuovo e una nuova
conferma. Se il canvas non è disponibile, i valori già salvati restano visibili
senza animazione. **Eroe → Annulla** ripristina insieme PF, temporanei e
concentrazione dell'ultima modifica di sessione.

Il campo esito manuale rimane per le eccezioni: non è necessario usare il tiro
quando il modello non rappresenta la regola applicabile.

## Errori e interruzioni

Un errore di scrittura mantiene lo stato visibile precedente. Il candidato e
il risultato già estratto restano **fissati in RAM**; la stessa conferma può
soltanto riprovare il salvataggio. Indietro, modalità, CD, extra, navigazione,
nuovi tiri e scelte manuali vengono bloccati dopo l'estrazione, evitando un
rilancio tramite annullamento e rientro. Nessuna animazione prima di un successo.

Il controller NVS esistente blocca le scritture dopo un errore I/O/allocazione e
mostra **Errore memoria: riavvia**. In quel caso riprovare non aggira il blocco:
occorre riavviare e verificare lo stato riletto. Il test host esercita anche una
scrittura nuovamente disponibile, verificando il riuso del medesimo tiro.

**Il risultato pendente non è un giornale persistente.** Un riavvio perde il tiro
che era solo in RAM; un commit fallito può aver scritto il candidato. Non viene
garantita una ripresa esatta dell'anteprima dopo perdita d'alimentazione. Se il
salvataggio era riuscito ma l'animazione non era ancora terminata, al boot si
rileggono danni e concentrazione già applicati, senza storico NVS della faccia.
Il recupero fisico dopo interruzione va ancora collaudato.

## Tasti, importazione e formati

Le assegnazioni fisiche esistenti sono conservate. **Danni** apre il modulo;
**Lancia** può accelerare il dado in volo e non rilancia il risultato impegnato.
Durante entrambe le conferme i tasti non confermano o saltano le scelte; PWR
lungo rimane riservato all'alimentazione. USB risponde busy e l'attenuazione è
sospesa; il primo gesto di risveglio continua a essere assorbito.

**PDD11/PDDB, USB v7, backup PC v10 e layout flash restano invariati.** Non ci
sono nuovi campi persistenti o scritture di migrazione al boot. Il candidato
riusa la memoria PSRAM delle altre conferme, che non possono aprirsi insieme;
nessun grande stato viene aggiunto allo stack. I dati e le preferenze precedenti
rimangono leggibili. Un firmware v0.28 può leggere lo stesso stato, senza questa
nuova interfaccia. Il backup di ritorno dal dispositivo al PC resta da completare.

## Verifiche e limiti

- **78840 casi indipendenti**: livelli 1–20, COS 1–30, competenza sì/no, extra
  importati/locali, tutte le facce, tutte le coppie con vantaggio/svantaggio,
  profili di danno, diverse CD e limite 30 ai confini 60/61/62 e oltre.
- Confronto del totale anche su 1/20, salvezze mancanti/bloccate, input fuori
  limite e rifiuti prima del RNG senza cambiare gli output.
- Candidato con danni/concentrazione salvato e riletto in PDD11; nessuna nuova
  interpretazione di condizioni o descrizioni.
- UI LVGL reale su host: componenti del bonus, modalità/extra, cancel prima del
  tiro, successo/fallimento, tentativi su errore senza rilancio né cambi di CD,
  risultato impegnato, canvas mancante, annullamento, criterio max 30, fallback
  manuale, import busy e tasti protetti. Pulsanti ≥44 px nel cerchio, senza overlap.
- Regressioni: 138 JS e build PC, 37751 confronti PC→USB→C, 139904 casi dei danni,
  17266 degli effetti e 40648 delle durate. ASan/UBSan su concentrazione/danni,
  allocazioni e UI; leak detector disattivato nel sandbox. Build ESP32-S3.

Restano bonus a dadi, reroll, minimi garantiti, proprietà condizionali e colpi
misti. La fine della concentrazione non disattiva automaticamente i modificatori
numerici locali: non sono ancora legati a quella magia. La scheda non diventa
un motore completo di regole. Nessun flash, collaudo fisico, nuova lettura
personale Beyond o modifica della raccolta personale.
