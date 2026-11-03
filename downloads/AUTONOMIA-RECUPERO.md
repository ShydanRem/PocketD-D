# Beyond sul Pocket: connessione e ripresa — v0.94

3 novembre 2026. L’estensione rimane il percorso scelto per la prima associazione
dell’account. Dopo il collegamento, il dispositivo gestisce lettura e conversione
via Wi-Fi. Questa versione completa il recupero dagli errori senza richiedere di
ripartire dal PC per un normale problema di rete.

## Meno passaggi sul dispositivo

- Aprendo Beyond con un account collegato e senza elenco in memoria, Pocket
  prepara una sola lettura. Se necessario prova a collegarsi alla rete salvata;
  dopo la connessione riprende l’elenco. La schermata consente di scegliere
  un’altra rete o annullare. Il tentativo sulla rete salvata dura al massimo 30 s.
- Scegliendo un PG dell’elenco mentre si è offline, conserva il suo **ID**, non
  la posizione nell’elenco. Dopo il Wi-Fi richiede proprio quel PG e ricontrolla
  che appartenga ancora al catalogo dell’account. Anche **Collega e leggi** per
  l’ID pubblico riprende la richiesta dopo la configurazione della rete.
- Se la rete non è configurata, si apre la schermata Wi-Fi. Ricerca, password,
  revisione e conferma restano sul Pocket. La lettura riprende solo dopo il ritorno
  alla schermata Wi-Fi collegata; non interrompe un editor, un tocco in corso o
  un errore di salvataggio della rete. Una nuova connessione già accodata è
  indicata come in corso prima che il task radio la esegua: la vecchia rete
  ancora collegata non viene confusa con la nuova. Una configurazione fallita
  conserva visibile l’errore invece di avviare Beyond sulla rete di recupero.
- **Indietro/Annulla** cancella la richiesta in attesa: un successivo collegamento
  della rete non avvia letture nascoste. Lasciare accesa la radio non mantiene
  un’importazione in coda. La normale configurazione Wi-Fi aperta senza una
  richiesta Beyond non avvia letture.
- Una richiesta fallita offre **Riprova** con gli stessi dati oppure **Wi-Fi**.
  Non rilancia automaticamente le richieste fallite, comprese le risposte 429.
  PG inesistente/accesso negato richiedono controllo del PG; la revoca dell’account
  richiede il nuovo collegamento dal PC. Non si ritrasmette automaticamente il
  segreto di una precedente associazione USB.

## Account, elenco e progressi

La pagina account indica **Accesso ricordato** oppure **Accesso fino allo
spegnimento**, con numero di pagina. Un elenco valido ma vuoto è distinto da un
elenco mai caricato e non provoca una nuova richiesta a ogni apertura. L’elenco
rimane in RAM: dopo un riavvio viene riletto quando si apre Beyond.

Un guasto di rete o un elenco/scheda in formato inatteso non revoca l’account
locale. Un rifiuto della sessione
da parte del servizio di autenticazione/elenco, o un’identità non verificabile,
richiede di ricollegare l’account. Un rifiuto della sola scheda non viene confuso
con la revoca dell’intero account. La mancata associazione di un nuovo account
non invalida quello precedente; il nuovo accesso diventa attivo solo al termine
del percorso già verificato nella v0.92. Lo stato di ricollegamento è in RAM:
al prossimo avvio un nuovo tentativo può verificare di nuovo l’accesso salvato.

Nessun nuovo campo di credenziali viene salvato. Token e cookie restano esclusi
da PG, SD, diagnostica e backup PG. Rimane il limite della NVS non cifrata,
descritto nella [guida all’associazione](COLLEGAMENTO-BEYOND.md).

Le operazioni automatiche si fermano sempre all’**anteprima**: limiti, consenso e
**Importa** sul display restano necessari. Nessun aggiornamento silenzioso del PG
attivo, nessuna scrittura verso Beyond. La SD conserva la scheda uscente secondo
la raccolta locale; senza SD resta una scheda attiva. La nuova importazione non
unisce automaticamente progressi/aspetto e dati freschi di una scheda precedente.

Nel percorso Wi-Fi aperto da Beyond, i tasti usano il fuoco locale: PWR breve
sposta la selezione, BOOT breve la attiva, BOOT lungo torna indietro. Le tre
assegnazioni personali restano conservate. Import/backup USB, ponte e navigazione
generale non possono interrompere la richiesta mentre si configura la rete.

## Compatibilità e prove

Basta aggiornare il firmware alla **0.94** conservando i dati. L’estensione 0.93
rimane compatibile; non è necessario reinstallare look o formattare la SD.
Formati PG/USB/backup, raccolta PDL1, partizioni e bundle di conversione invariati.

Verifiche software: suite JS, worker nativo con HTTP/task/rete simulati e
interfaccia LVGL di produzione con **150 viste**. Nuovi casi: tentativo Wi-Fi unico,
ripresa per ID, annullamento prima/dopo un evento radio, riprova esplicita,
rete configurata e da configurare, editor protetti, attesa limitata, blocco su
salvataggio incerto, tasti e assegnazioni conservate, elenco vuoto e distinzione
fra guasto di rete e accesso da ricollegare. Controlli memoria e compilazione ESP32.

[Schermate del renderer reale con rete/account sintetici](previews/v094-beyond-recovery.png).
Queste prove non dimostrano la radio, il nuovo handshake USB o un accesso privato
reale. Il collaudo con account ed ESP32 resta aperto, distinto dalle precedenti
conferme dell’utente su USB, installer, SD ed elenco nell’estensione.
