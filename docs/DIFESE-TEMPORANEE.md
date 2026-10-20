# Difese temporanee sul dispositivo — v0.31

> Aggiornamento v0.33: [condizioni difensive e fonte esplicita](DIFESE-CONDIZIONALI.md),
> raggruppamento per applicabilità e PDD14. Le omissioni/formati sotto sono storici.

> Aggiornamento v0.32: [danni misti dello stesso colpo](DANNI-MISTI.md), con
> raggruppamento per tipo e un solo esito. PDD13/USB v7/layout invariati.

Resistenza, vulnerabilità e immunità possono avere una durata manuale o a round,
con un vincolo facoltativo alla concentrazione. Sono configurazioni locali
esplicite, disponibili offline e senza Foundry. Si aggiungono ai
[profili di base](DANNI-DIFESE.md), che restano separati e conservati.

## Configurazione

Da **Usi → Stato → Danni → Difese per tipo**, scegli un tipo di danno:

1. La prima pagina modifica solo il **profilo base**, compreso Configurato.
   Se la base è normale, conferma Configurato sì lasciando le tre proprietà no.
   Una base sconosciuta continua a bloccare i danni, anche con un extra attivo.
2. **A tempo** apre il profilo aggiuntivo: scegli le proprietà e **Attiva sì**.
   La pagina Base permette di tornare alle scelte precedenti della stessa bozza.
3. **Durata** seleziona manuale oppure 1–99 round. Disattivata significa timer
   in pausa; per riattivare un profilo scaduto serve anche una durata positiva.
4. **Vincolo** associa facoltativamente la difesa alla magia concentrata attuale.
   Senza concentrazione la scelta è disabilitata; nulla viene dedotto dai nomi.
5. **Anteprima → Conferma** salva base ed extra in una sola modifica.

L'anteprima mostra le tre proprietà effettive prima/dopo e, per ciascuna, i
contributi base/extra separati. Espone inoltre base verificata, attivazione,
round residui e vincolo. Un extra attivo senza proprietà o con timer zero
viene rifiutato. Indietro torna alla pagina da cui si è aperta l'anteprima,
senza perdere la bozza; abbandonarla non modifica lo stato salvato.

Le righe dell'elenco mostrano il profilo effettivo e **+T** se l'extra è attivo.
Riaprire/modificare solo la base non vi copia le proprietà temporanee.
Disattivare l'extra conserva le scelte per un successivo riuso; non serve
ricostruire la base. È disponibile **un solo profilo aggiuntivo per ciascuno
dei 13 tipi**, indipendente dagli otto modificatori CA/PF massimi.

## Combinazione e danni

Si uniscono le proprietà base e quelle dell'extra attivo, senza moltiplicare
più volte una proprietà uguale. Due resistenze applicano una sola divisione
per due. La risoluzione rimane quella del flusso danni esistente:

- immunità: zero danni;
- altrimenti resistenza: dimezza per difetto;
- poi vulnerabilità: raddoppia;
- quindi assorbimento nei PF temporanei, PF correnti e concentrazione.

Resistenza e vulnerabilità insieme non si cancellano aritmeticamente sui danni
dispari: 11 diventa 5, poi 10. Le difese non includono riduzioni numeriche o
eccezioni dedotte da testo, fonte, arma o condizione.

**Il colpo usa le difese presenti all'impatto.** Se poi si perde concentrazione,
l'extra collegato termina per i colpi successivi. Il danno corrente non viene
ricalcolato con le nuove difese. Anche la CD di concentrazione usa quel danno
effettivo, prima dei temporanei. Immunità e danno arrotondato a zero non
richiedono un tiro e non terminano concentrazione.

Esempio: resistenza temporanea al fuoco, danno 25, PF temporanei 20. Il colpo
infligge 12 e lascia 8 temporanei. Se il tiro fallisce, la resistenza collegata
si disattiva; il prossimo colpo userà la base. Se termina anche un bonus ai PF
massimi, il relativo limite viene applicato nello stesso candidato.

## Un solo comando per avanzare il round

**1 round**, disponibile da Effetti e da Difese, avanza insieme tutti i timer
attivi dei modificatori numerici e delle difese. Anche la scorciatoia fisica
**Round** apre questa anteprima; non applica direttamente la modifica.

Il riepilogo mostra conteggi, scadenze e CA/PF prima/dopo. **Difese** apre una
vista delle sole difese che avanzano, fino a tre per pagina: round residui e
profilo effettivo prima/dopo. Le pagine sono consultazione; Riepilogo riporta
alla conferma comune. Non occorre scorrere tipi senza timer attivo.

**Conferma** salva l'intero avanzamento una sola volta. Se una scadenza numerica
lascerebbe CA/PF fuori limite, non avanza alcun timer, comprese le difese.
Se la scrittura fallisce, lo stato visibile resta precedente. **Eroe → Annulla**
ripristina insieme timer, proprietà, vincoli, attivazioni e PF dell'ultima
modifica di sessione. Non c'è un orologio automatico: ridisegni, animazioni,
attesa, attenuazione e letture USB non fanno passare round.

Alla scadenza l'extra viene disattivato; proprietà e stato del vincolo restano
consultabili. La base non cambia: se era sconosciuta, resta da verificare.
Nessuna proprietà temporanea diventa permanente per effetto della scadenza.

## Concentrazione, riposi e tasti

La fine della concentrazione disattiva e scollega tutti gli extra associati,
insieme agli effetti numerici: fine manuale, nuovo lancio anche della stessa
magia, Ira, zero PF, fallimento manuale/con dado e riposo lungo. Sono inclusi
anche i profili già in pausa o scaduti. Proprietà e round residui si conservano;
nessun nuovo lancio li riattiva o ricollega automaticamente.

Il conteggio «effetti associati/terminati» comprende **profili temporanei attivi
più modificatori numerici**, fino a 21; le tre proprietà dello stesso profilo
contano come una sola voce. Il tiro salva insieme danni, concentrazione, effetti
e difese prima dell'animazione. Dopo errore, il tiro fissato e il candidato
sono riusati senza rilancio. Restano i [limiti del salvataggio](CONCENTRAZIONE.md):
il controller NVS blocca le scritture dopo errore I/O; serve riavvio e verifica,
non c'è un giornale persistente della faccia e un commit fallito può aver scritto.

Il riposo breve conserva gli extra; il lungo termina quelli collegati alla
concentrazione. Extra indipendenti e timer non avanzano automaticamente coi
riposi. Requisiti, condizioni particolari e altre scadenze restano al giocatore.
Le assegnazioni PWR/BOOT sono conservate; nelle conferme e nella consultazione
delle scadenze i tasti non applicano o abbandonano la modifica. USB risponde busy,
l'attenuazione è sospesa, PWR lungo rimane riservato all'alimentazione.

## Dati e compatibilità

Nuove scritture **PDD13**, tag **PDDD** (D = 13):
**8047 + 552 × numero magie/capacità** byte, massimo **74287 byte**.
Dopo i collegamenti numerici e prima del CRC si aggiungono 65 byte: per ciascuno
dei 13 tipi, proprietà, attivazione, modalità a round, round e concentrazione.
Booleani non canonici, proprietà/timer fuori limite, riferimenti non correnti
e combinazioni attive senza proprietà/durata vengono rifiutati anche con CRC
valido. Generazione, CRC, conferma e rilettura della banca NVS restano invariati.

Lettura **PDD1–PDD12** conservata. I vecchi profili restano di base, senza extra
inventati e senza scritture di migrazione al boot. La fixture PDD12 è sintetica,
generata con l'encoder v0.30 del commit `a9bcd6f` prima della modifica: comprende
profili base, concentrazione e modificatori collegati. Il test NVS verifica
lettura, nessuna scrittura al boot, modifica nella banca alternata, copia
originale conservata e riavvio con extra/durata/vincolo persistiti.

Firmware precedenti alla v0.31 non leggono PDD13 e conservano il blocco di
sicurezza sui record futuri. Non cancellare NVS per effettuare un downgrade.
**USB v7, backup PC v10 e layout flash restano invariati.** L'importazione
PC→dispositivo continua a sostituire lo stato di gioco con anteprima/conferma:
rimuove gli extra locali, anche con base sconosciuta, e lo segnala. Annullare
conserva tutto; la raccolta PC rimane intatta. Il backup dei progressi dal
dispositivo al PC è ancora da completare.

## Verifiche e limiti

- **32032 casi indipendenti** di tipi, proprietà base/extra, attivazione e danni
  pari/dispari; base sconosciuta e proprietà uguali senza accumulo.
- **64350 avanzamenti**: tutte le durate 1–99 per tutti i 13 tipi, scadenze,
  pausa e conservazione della base. Avanzamento comune, rifiuto atomico e vincoli.
- Fine manuale, stessa magia, Ira, riposi, zero PF, fallimento manuale/con dado;
  proprietà all'impatto, danno/CD/temporanei coerenti e nessuna permanenza implicita.
- Record massimo, alterazioni di tutti i 65 byte con CRC valido, fixture
  PDD1–PDD12 e scritture NVS alternate. UI LVGL reale su host per bozze, fonti,
  durata/vincolo, scadenze e paginazione di 13 tipi, 21 collegamenti, retry,
  annullamento, import, tasti e conferme. Controlli ≥44 px nel cerchio, senza
  sovrapposizioni tra testi e pulsanti.
- Regressioni 138 JS e build PC, 37751 confronti PC→USB→C, 17266 casi effetti,
  40648 durate, 139904 danni, 78840 concentrazione e 4608 vincoli. ASan/UBSan
  su extra, effetti/durate, danni/concentrazione, vincoli, magie/riposi/stato,
  core/parser, NVS, allocazioni e UI; leak detector disattivato nel sandbox. Build ESP32-S3.

Resta un limite per tipo: più fonti aggiuntive con scadenze diverse non sono
rappresentabili separatamente; occorre gestirle manualmente col DM. Non sono
automatizzate difese contro «non magico», specifiche armi/attaccanti, condizioni
incapacitanti, durate di turno/minuti, riduzioni fisse o danni misti. Nessun
nuovo accesso Beyond, credenziale o JSON personale incluso. Nessun flash o
collaudo fisico: alimentazione, batteria, sospensione, touch, USB e salvataggi
dopo interruzione richiedono ancora il dispositivo reale.
