# Effetti collegati alla concentrazione — v0.30

> Aggiornamento v0.32: [danni misti dello stesso colpo](DANNI-MISTI.md), con
> raggruppamento per tipo e un solo esito. PDD13/USB v7/layout invariati.

> Aggiornamento v0.31: [difese temporanee, concentrazione e round comune](DIFESE-TEMPORANEE.md)
> con PDD13. I limiti/formati dell’incremento sotto sono storici.

Gli otto modificatori locali a CA/PF massimi possono essere associati
**esplicitamente alla magia attualmente concentrata**. Quando quella
concentrazione finisce, vengono disattivati e scollegati insieme. Il dispositivo
funziona offline; Foundry e PC non servono durante la sessione.

## Uso sul dispositivo

1. Lancia una magia con concentrazione da Magie e conferma il costo.
2. Apri **Usi → Stato → Effetti**, aggiungi o modifica un effetto numerico.
3. In **Durata → Vincolo** scegli **Concentrazione: sì**. La pagina mostra la
   magia corrente; senza una concentrazione non si può creare un collegamento.
4. **Anteprima → Conferma** salva valore, attivazione, durata e collegamento in
   una sola modifica. Uscire dalla bozza non cambia la scheda.

Il vincolo è facoltativo e inizialmente assente. Non viene creato leggendo il
nome o la descrizione di una magia. Una riga con **C** è collegata; l'anteprima
riporta **conc. sì/no**. Non aggiungere un bonus già incluso nella base importata.
Un effetto disattivato può essere collegato, ma non contribuisce ai totali.

Un timer a round può coesistere con il vincolo: scade al primo dei due eventi,
scadenza confermata del timer oppure fine concentrazione. Il tempo continua ad
avanzare solo con **1 round**, senza orologio o round impliciti. Disattivare un
effetto mette il timer in pausa. La scadenza non termina la magia concentrata.

## Fine, sostituzione e recuperi

La stessa operazione termina i collegamenti quando:

- si conferma **Concentrazione: termina** nelle risorse magiche;
- si conferma un'altra magia concentrata, **anche rilanciando la stessa voce**;
- si attiva Ira;
- un danno porta a zero PF o una verifica di concentrazione fallisce, con esito
  manuale oppure con tiro COS;
- si conferma il riposo lungo.

Una magia senza concentrazione, il riposo breve, un tiro riuscito e un danno
nullo conservano i vincoli. La scheda a zero PF li termina anche attraverso i
controlli manuali dei PF. Restano i blocchi già presenti per danni su personaggio
a zero PF e possibile morte istantanea: non sono stati automatizzati.

Alla fine, tutte le voci collegate diventano **inattive e senza vincolo**,
comprese quelle già in pausa o scadute. Rimangono nell'elenco per modificarle;
valore e round residui sono conservati. Rilanciare la magia non le riattiva e
non le riassocia. Occorre scegliere il nuovo vincolo e confermare nuovamente;
un timer scaduto richiede anche una durata positiva prima della riattivazione.

## Totali e PF

Si ricalcolano CA e PF massimi dopo aver disattivato **tutti** gli effetti
collegati. Si limita poi il numero di PF correnti una sola volta. Un massimo
maggiore non cura. Il riposo lungo termina prima i vincoli e cura fino al
massimo risultante. Il riepilogo del riposo può quindi mostrare una diminuzione
netta, senza trasformarla in un recupero numerico enorme.

Esempio: base PF massimi 32, effetto collegato +10, PF correnti 41. Alla fine
concentrazione diventano **32/32**. Anche se un danno di 1 viene assorbito dai
PF temporanei, un fallimento di concentrazione può comunque ridurre i correnti
da 41 a 32: è la perdita del bonus massimo, oltre all'assorbimento del colpo.

Il controllo manuale di fine mostra CA, massimo e PF correnti prima/dopo;
la conferma di sostituzione/Ira avvisa quanti effetti attivi terminano e mostra
l'eventuale riduzione dei correnti. Il flusso danni avvisa dei collegamenti,
include il limite al nuovo massimo nell'esito e ne espone il numero terminato.

**CA 1–40 e PF massimi 1–999 devono essere validi sia con gli effetti attivi,
sia senza tutti quelli collegati.** Ad esempio un bonus collegato non può
compensare una penalità indipendente che, alla fine concentrazione, lascerebbe
un massimo zero. Una modifica/rimozione/scadenza che viola questo criterio
viene rifiutata interamente. Correggere anche i valori senza concentrazione.
Questo evita una fine impossibile dopo aver già estratto un dado. Non si
correggono automaticamente bonus del giocatore o basi importate.

## Conferme, tasti ed errori

Fine, costi di lancio, effetti e PF sono un solo candidato salvato prima di
cambiare lo stato visibile. **Eroe → Annulla** ripristina l'ultima modifica di
sessione, compresi concentrazione, vincoli, attivazioni, round e PF. Non è uno
storico persistente. I tasti PWR/BOOT mantengono le assegnazioni; non confermano
queste operazioni né abbandonano le conferme. PWR lungo resta hardware.

Un errore lascia visibile lo stato precedente. Per il tiro di concentrazione,
anche candidato ed effetti restano fissati insieme al dado in RAM: un tentativo
successivo riusa quel tiro. Il controller NVS blocca le scritture dopo un errore
I/O e richiede riavvio/verifica; un commit fallito può aver scritto il candidato.
Non c'è un giornale persistente delle facce o una ripresa garantita della conferma
al reboot. Vedere [concentrazione](CONCENTRAZIONE.md).

## Salvataggi e compatibilità

Le nuove scritture sono **PDD12**, tag **PDDC** (C = 12), con dimensione
**7982 + 552 × numero magie/capacità**, massimo **74222 byte**. Gli otto byte
prima del CRC salvano, per ogni effetto, zero oppure l'indice della magia + 1.
Un riferimento non corrente o in una posizione fuori elenco è rifiutato anche
con CRC valido. La fine di ogni concentrazione cancella i riferimenti, perciò
la successiva attivazione della stessa magia non recupera vecchi vincoli.

Lettura **PDD1–PDD11** conservata; effetti precedenti inizializzati **senza
vincolo**, senza migrazione o scritture al boot. La fixture PDD11 è sintetica,
generata con l'encoder v0.29 del commit `a75afd1`, prima della modifica al codice.
Il test NVS verifica lettura, nessuna scrittura al boot, prima modifica nella
banca alternata e conservazione della copia originale.

Firmware precedenti alla v0.30 non leggono PDD12: la protezione dei record
futuri blocca le scritture, senza cancellare i dati. Non effettuare un downgrade
aspettandosi la lettura dei progressi nuovi; non cancellare NVS per aggirarlo.
Il layout delle partizioni resta quello esistente. **USB v7 e backup PC v10
invariati**: il trasferimento PC→dispositivo sostituisce lo stato di gioco previa
anteprima/conferma, senza vincoli locali importati. Annullare conserva quelli
presenti. La raccolta personale PC non viene toccata. Il backup dei progressi
dispositivo→PC resta da completare.

## Verifiche e perimetro

- **4608 casi indipendenti**: 256 sottoinsiemi di otto vincoli, entrambi i bersagli,
  nove cause di fine; totali e PF attesi, costi, sostituzione della stessa voce,
  Ira, riposi e danni/tiro. Bonus opposti verificano il limite applicato una volta.
- Percorsi che conservano i vincoli, timer scaduto, effetto in pausa, penalità
  massima rimossa senza cura, riferimenti errati e valori senza concentrazione
  fuori limite; rifiuti prima del RNG e senza mutazioni.
- Record massimo e otto riferimenti alterati con CRC valido, fixture PDD1–PDD11
  e salvataggio alternato. UI LVGL reale su host: bozze, cancellazione, conferme,
  errore/riprova, annullamento, rilancio, danni, riposo e assenza concentrazione.
  Pulsanti ≥44 px dentro il cerchio; testi e controlli senza sovrapposizioni.
- Regressioni 138 JS, build PC, 37751 confronti PC→USB→C, 17266 casi effetti,
  40648 durate, 139904 danni e 78840 salvezze di concentrazione. ASan/UBSan su
  vincoli, effetti/durate, danni/concentrazione, magie/riposi/stato, allocazioni
  e UI; leak detector disattivato nel sandbox. Compilazione ESP32-S3 riuscita.

Non si automatizzano il testo delle magie, bersagli/alleati, condizioni
incapacitanti, altri tipi di bonus, regole di cumulo, difese con scadenza,
bonus a dadi, colpi misti o durate di turno/minuti. Le note delle condizioni
rimangono annotate; le resistenze sono profili manuali distinti dagli effetti.
Nessuna nuova lettura Beyond, credenziale o JSON personale incluso. Nessun
flash o collaudo fisico: batteria, sospensione, alimentazione, touch, USB e
resistenza dei salvataggi a interruzioni restano da provare sull'hardware.
