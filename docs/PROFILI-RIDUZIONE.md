# Profili locali di riduzione — v0.36

**Aggiornamento v0.37:** [costi espliciti e reazione](COSTI-REAZIONE.md), quarto
confronto Costi e formato corrente PDD17. Questa pagina documenta la base v0.36;
la frase sulla mancata spesa e il formato PDD16 descrivono quella versione.

Il dispositivo conserva **tre profili riutilizzabili**, ciascuno con quantità,
tipi ammessi, condizione della fonte, fase prima/dopo, attivazione, durata e
vincolo alla concentrazione. Per ogni colpo si sceglie **un solo profilo**.
La configurazione è esplicita: non è un riconoscimento automatico di talenti,
armature, reazioni o capacità importate da Beyond.

## Configurare un profilo

Da **Danni → Riduci → Profili → Profilo N → Modifica**:

- **Quantità**: 0–9999; un profilo attivo richiede una quantità positiva.
- **Tipi**: tutti, contundente/perforante/tagliente insieme oppure un singolo
  tipo, scorrendo i 13 tipi. Un profilo attivo richiede almeno un tipo.
- **Fase**: prima delle difese oppure dopo le difese dell'intero colpo.
- **Attivo**: abilita o mette in pausa quel profilo.

**Dettagli** sceglie una delle [sette condizioni della fonte](DIFESE-CONDIZIONALI.md),
durata manuale/a round, round residui e vincolo alla magia concentrata corrente.
Le abbreviazioni di argento/adamantio richiedono anche un attacco non magico:
il requisito completo è visibile nell'anteprima della regola. L'attivazione non
inventa quei fatti, né verifica equipaggiamento o requisiti non rappresentati.

**Anteprima** mostra tre pagine di confronto: Dati, Regola e Durata. Il selettore
scorre quantità/tipi/fase/attivazione, requisito completo prima/dopo e timer/vincolo.
**Conferma** salva tutta la definizione con un solo commit. Se attivo con quantità,
tipi o round non validi, la conferma è bloccata. Tornare dalla bozza senza
confermare non cambia il profilo salvato. **Eroe → Annulla** ripristina l'ultima
modifica della sessione, che può essere una configurazione o un colpo.

Il salvataggio della definizione è una modifica del personaggio locale, distinta
dalla selezione per un colpo. Annullare poi l'editor Riduci non annulla un profilo
che è già stato salvato con Conferma.

## Scegliere un profilo per il colpo

**Usa** apre l'assegnazione. Per la fase Prima, scegli una parte e distribuisci
una quantità; il selettore passa alle altre parti. La **somma delle assegnazioni
non può superare il budget del profilo**. Puoi lasciare budget inutilizzato,
ma non selezionare un profilo Prima con tutte le assegnazioni a zero.

Esempio: budget 3 e due parti ammesse. Sono valide 3+0, 2+1, 1+2 e 0+3;
**3+3 viene bloccato**. L'eccedenza rispetto al danno di una parte non viene
trasferita automaticamente alle altre. Aggiungere una parte inizia con
assegnazione zero; rimuovere/promuovere righe conserva l'assegnazione associata
alla riga corretta. Se la parte rimossa possedeva l'unica assegnazione, occorre
rivedere il budget o scegliere Nessuno.

Per la fase Dopo, la quantità è fissa e condivisa: niente assegnazioni per parte,
una sola sottrazione dal totale dell'impatto. Il profilo deve essere ammissibile
per **tutte** le parti del colpo. Un colpo misto con una parte esclusa non riceve
automaticamente una riduzione Dopo sulle altre: usare un diverso profilo o le
riduzioni manuali se la regola reale richiede altro.

**Usa profilo** aggiorna soltanto la bozza esterna di Riduci; serve ancora
**Usa valori** per inserirlo nel colpo. **Indietro** da Riduci annulla la selezione,
insieme alle modifiche manuali di quella pagina. **Profili → Nessuno** prepara
la rimozione del profilo dalla bozza; non cancella la sua definizione e non
cancella le altre riduzioni manuali.

L'editor di una singola componente non seleziona profili per l'intero impatto:
la scelta si fa dall'editor Riduci generale.

## Ammissibilità e ordine delle sottrazioni

Per Prima, il profilo verifica tipo e condizione soltanto sulle parti con
assegnazione positiva. Per Dopo, li verifica su tutte le parti, anche a danno
zero. **Sconosciuto non significa falso**: i fatti pertinenti mancanti bloccano
il colpo e aprono la normale dichiarazione Fonte. Fatti non necessari possono
restare sconosciuti. Una parte esclusa, un profilo inattivo/scaduto o un budget
non valido bloccano l'applicazione con un messaggio specifico.

I profili non rendono note le difese ancora da configurare. Anche a danno
azzerabile, il dispositivo non dichiara riuscito un calcolo con profilo/fonte
non verificati.

Le [riduzioni manuali](RIDUZIONI-DANNI.md) possono coesistere e rimangono separate.
L'ordine esplicito è:

1. Su ciascuna parte, sottrazione manuale Prima, con minimo zero.
2. Assegnazione del profilo Prima, limitata al danno rimasto di quella parte.
3. Raggruppamento per tipo/proprietà applicate e difese/arrotondamenti.
4. Sottrazione manuale Dopo una sola volta, con minimo zero.
5. Profilo Dopo una sola volta sul totale rimasto, con minimo zero.
6. Temporanei/PF, controlli di morte e una salvezza di concentrazione sul finale.

Non inserire manualmente una riduzione già rappresentata dal profilo. L'app
valida il budget del profilo scelto; non può riconoscere due descrizioni dello
stesso beneficio né certificare che più regole speciali siano cumulabili.

**Tipi → Conto** mostra le sottrazioni complessive scelte/effettive e il totale;
**Profilo** mostra la definizione usata all'impatto e le quantità richieste e
sottratte per riga, oppure per l'intero colpo nella fase Dopo. Le quantità
sottratte sono valori della rispettiva fase, non un confronto ipotetico dei PF
con una configurazione diversa. A totale ancora bloccato non sono presentate
come un risultato applicabile.

A zero danni effettivi non si consumano temporanei, non si tira COS e non finisce
la concentrazione. Nessuna risorsa, reazione, carica o uso viene consumato
automaticamente dal profilo.

## Durata, concentrazione, errori e tasti

Il round comune avanza effetti numerici, 39 extra difensivi e tre profili:
**42 timer difensivi**, con 14 pagine di dettaglio quando tutti sono a round.
Un profilo disattivato resta in pausa. Scadenza a zero lo disattiva, senza
riattivarlo nei colpi successivi. Se una scadenza numerica rendesse i totali
invalidi, nessun timer del round viene avanzato.

La fine della concentrazione disattiva e scollega tutti i profili vincolati,
anche inattivi per la rimozione del vincolo. Rilancio della stessa magia, Ira,
zero PF, fallimento e riposo lungo seguono lo stesso percorso congiunto degli
altri effetti. Quantità, tipi, regola e timer restano disponibili per un riuso
esplicito. Un profilo selezionato e poi scaduto non è ignorato silenziosamente:
il colpo va rivisto prima della conferma.

Il riepilogo del colpo conserva la definizione **all'impatto**, anche quando il
profilo è terminato in quel colpo. Candidato, dado e conto restano fissati su
un errore di salvataggio: Riprova non consuma nuovo RNG. Il controller NVS blocca
scritture successive a un errore I/O; riavvio/verifica restano necessari e un
commit fallito può avere scritto il candidato.

Tutte le pagine dei profili proteggono la bozza da import USB, attenuazione e
scorciatoie PWR/BOOT; si torna con i controlli del display. Le assegnazioni dei
tasti restano configurabili e PWR lungo resta hardware. Le nuove pagine non
confermano danni tramite i tasti fisici.

## Persistenza e compatibilità

Scritture **PDD16**, tag **PDDG**, dimensione **8363 + 552 × numero voci**, massimo
**74603 byte**. Il tag continua la sequenza dei formati; G indica il sedicesimo.
Si aggiungono **30 byte** prima del CRC: tre profili da 10 byte, ciascuno con
quantità uint16, maschera tipi uint16, fase, regola, attivo, timer, round e vincolo.
Nessun dump di struct. Range, booleani, combinazioni attive e riferimenti sono
validati anche quando il CRC è valido.

Lettura **PDD1–PDD15** conservata senza scritture al boot: i tre profili iniziano
vuoti e le difese precedenti restano inalterate. Fixture sintetica PDD15 prodotta
con l'encoder v0.35 originale `c99bdf0`, prima delle modifiche, con tre extra
condizionali, timer e concentrazione. Il controller NVS verifica prima scrittura
nella banca alternata, record originale intatto e rilettura dopo riavvio.
Firmware precedenti alla v0.36 non leggono PDD16; non cancellare NVS per aggirarlo.

**USB v7, backup PC v10 e layout flash invariati.** Profili salvati localmente;
selezione, assegnazioni e riduzioni manuali del colpo solo RAM. Nuovo e scorciatoia
Danni azzerano la selezione. Importare una nuova proiezione con conferma rimuove
anche le definizioni locali; annullare conserva tutto. Raccolta PC conservata;
backup dei progressi dispositivo→PC ancora successivo.

Per il core: `pd_reduction_profile_set` modifica uno slot 0–2; `event.profile`
è 0 oppure indice+1, `allocation[i]` si riferisce alla stessa riga di tipo/fonte.
Il report conserva snapshot, richieste e quantità per riga. I bit 39–41 delle
maschere round uint64 individuano i profili; i precedenti 39 restano gli extra.
Le API singole precedenti non selezionano profili automaticamente.

## Verifiche e limiti

176904 combinazioni indipendenti di slot, tipo, condizione, fonte parziale,
fase, attivazione e inclusione del tipo; 2178 assegnazioni di budget e 1440
permutazioni nelle due fasi. Ammissibilità parziale/intero impatto, rifiuti prima
del RNG, somma condivisa e clamp, vincoli/fine/rilancio/Ira/riposo/zero PF/COS,
round atomico, 42 timer e massimo 50 effetti/difese vincolati. Record massimo,
tutti i 30 byte alterati con CRC valido, campi illegali e fixture PDD1–PDD15/NVS.

UI LVGL nativa di configurazione completa e testi lunghi, invalidi e limiti,
conferma/errore/annulla, selezione interna/esterna, budget, esclusioni e fonte,
conto originale dopo fine del profilo, round/scadenze, import, tasti e 14 pagine.
Controlli nel cerchio ≥44 px e testi senza sovrapposizioni. Suite firmware e
regressioni, 138 test JS/build PC passati; ASan/UBSan su calcoli, lifecycle,
allocazioni, core/parser, NVS e UI con leak detector disattivato. Build ESP32-S3
riuscita; [dettagli](FIRMWARE.md).

Limiti: tre profili numerati, **uno selezionabile per colpo**; un importo fisso,
sette condizioni e i preset di tipi descritti. Il core accetta maschere più
generali, ma il dispositivo non offre un editor di combinazioni arbitrarie.
Non ci sono nomi liberi, catalogo di talenti, ricalcolo del valore da livello/
competenza, verifica automatica dell'armatura, costi/risorse/reazioni, profili
cumulati o durate in minuti/turni. Configurare un profilo non importa né certifica
una specifica capacità del personaggio.

Funziona offline senza Foundry. Nessuna nuova lettura Beyond, credenziale o JSON
personale pubblicato. Compilazione e prove su host non sono collaudo fisico:
alimentazione, batteria, sospensione, touch, USB e recupero dopo interruzione
richiedono ancora il dispositivo reale.
