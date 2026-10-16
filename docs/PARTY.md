# Party simulato — protocollo 1, v0.12

## Prova con più finestre

1. Avvia `npm run dev -- --port 5173` e apri la demo. Le finestre devono usare lo stesso indirizzo, porta e profilo del browser.
2. Scegli l'eroe, poi **Al tavolo → Party locale simulato → Crea party**. Questa finestra è il coordinatore.
3. Apri una nuova finestra vuota e raggiungi lo stesso indirizzo. Scegli un altro eroe della raccolta, apri il party e inserisci il codice. Ripeti per altri giocatori. Non usare “Duplica scheda”: alcuni browser copiano anche l'identità di sessione.
4. Ogni giocatore imposta la propria iniziativa. Il coordinatore può aggiungere avversari manuali e piazzare pedine/muri sulla mappa. Il limite è di 12 partecipanti, avversari inclusi; nuovi ingressi e iniziativa si gestiscono in preparazione.
5. Il coordinatore avvia il combattimento, passa i turni e modifica i PF. Il giocatore può muovere la propria pedina durante il suo turno; il coordinatore può muovere quella attiva. Percorsi, collisioni, muri e consumo del movimento seguono il modello locale esistente.

Il pannello può essere chiuso mantenendo il collegamento. **Sospendi collegamento / Riconnetti** prova un'interruzione senza abbandonare l'identità del giocatore. Ricaricare la stessa scheda del browser ripristina il collegamento e il checkpoint, se `sessionStorage` è disponibile. **Esci dal party** dimentica l'identità di quella finestra: la sua pedina resta nel roster fino alla creazione di un nuovo party. Questa prima versione non rimuove membri e non riassegna pedine.

Il coordinatore deve tenere aperta la sua scheda del browser. Dopo circa 7–9 secondi senza risposta, i giocatori vedono il collegamento sospeso e non possono inviare modifiche. Il suo ritorno recupera lo stato; non si elegge automaticamente un nuovo coordinatore. Chiudere definitivamente quella scheda può perdere la sessione. **Scarica stato del party** produce un resoconto JSON; non c'è ancora un comando di importazione del resoconto.

La mappa nel pannello ha celle da almeno 44 px. Su schermi stretti scorre orizzontalmente nel proprio riquadro. La griglia nel dispositivo circolare rimane il combattimento personale della scheda: l'integrazione dei controlli del party nel cerchio è un lavoro successivo.

## Stato e isolamento

`src/party.js` definisce il modello puro e il riduttore dei comandi, riutilizzando `encounter.js` e `grid.js`. I partecipanti del party hanno dati espliciti: nessuna pedina speciale `hero` e nessun riferimento vivo alla scheda personale.

All'ingresso si copiano nome, PF, CA e movimento dell'eroe selezionato. I cambiamenti successivi nel party non modificano la raccolta; cambiare il personaggio attivo non cambia la pedina già associata. Inventario, tiri, magie, concentrazione e risorse personali non sono trasmessi. Attacchi automatici, richieste di danno dei giocatori e riconciliazione con la scheda personale restano da sviluppare.

La raccolta usa ancora `pocketdd-library-v1`, formato raccolta v1 con schede v9. Il party usa esclusivamente la chiave `pocketdd-party-v1` di `sessionStorage`, separata per finestra. Non introduce migrazioni o dipendenze npm.

## Messaggi e autorità

`src/party-transport.js` usa `BroadcastChannel` sul canale `pocketdd-party-v1-<sessionId>`. Sono simulazioni nello stesso browser/origine: nessun server, collegamento LAN, ESP-NOW, BLE o trasferimento tra dispositivi è implementato. Per la prova condivisa usa HTTP/HTTPS; il comportamento delle origini `file://` dipende dal browser e non è supportato.

Ogni sessione ha identificativi casuali a 96 bit e un solo coordinatore. I messaggi portano versione di protocollo e sessione. Un client scopre il coordinatore dal primo snapshot valido e poi accetta soltanto la sua identità. Questo trasporto presuppone client fidati nella stessa origine e non offre autenticazione crittografica: associazione, autorizzazione e protezioni per una rete reale sono ancora da progettare.

| Messaggio | Effetto |
| --- | --- |
| `hello` | Richiede lo stato corrente, anche dopo riconnessione. |
| `command` | Chiede al coordinatore una modifica: `clientId`, `seq`, `baseRevision`, `action`, `payload`. |
| `snapshot` | Stato completo validato, identità del coordinatore e conferma facoltativa destinata al mittente. |

Il client invia al massimo un comando in attesa e lo ritrasmette ogni 2 secondi fino alla conferma. Non esegue modifiche ottimistiche. Ogni client ha una sequenza crescente; il coordinatore conserva l'ultima sequenza e il suo eventuale errore nel checkpoint. Una ritrasmissione già elaborata restituisce lo stato senza ripetere l'azione. I salti di sequenza sono rifiutati.

Un comando accettato incrementa la revisione. Tranne l'ingresso iniziale, ogni comando deve basarsi sulla revisione corrente: due scelte concorrenti non aggiornate non si applicano entrambe. Il client riceve lo stato aggiornato e ripete esplicitamente la scelta. Un comando rifiutato di un membro consuma la sua sequenza, ma non incrementa la revisione; ritrasmetterlo non può renderlo valido in un momento successivo. Un primo ingresso rifiutato non riserva una voce nel registro; l'utente può riprovarlo.

Gli snapshot validano roster, proprietari, PF, round, muri, posizioni e ricevute. Una revisione precedente o un registro delle ricevute regressivo non sostituiscono lo stato già visto. I checkpoint comprendono identità, stato e comando eventualmente pendente; il coordinatore salva prima della conferma. Questi vincoli impediscono di ripetere un comando anche dopo una normale ricarica.

Se la memoria è negata o piena, il party funziona in memoria e mostra la condizione temporanea. In quel caso ripresa e deduplicazione dopo riavvio non sono garantite; l'eventuale checkpoint precedente può essere meno recente. Il resoconto è una copia esportabile dello stato corrente. I salvataggi personali restano indipendenti.

## Verifica ripetibile

`npm test` include casi del protocollo e del trasporto: permessi, limiti, input invalidi, comandi duplicati/obsoleti, movimento, snapshot regressivi, conferme perse, comando pendente, riconnessione, ricarica del coordinatore e fallimenti della memoria.

`npm run build` rigenera anche `PocketDD-demo.html`. Dopo la build, `python scripts/check-party.py` apre tre pagine Chromium nello stesso contesto e usa veri `BroadcastChannel`. Verifica iniziativa, turni, PF, mappa, controlli del giocatore, sospensione, recupero, reload di giocatore/coordinatore, isolamento della raccolta, esportazione e viewport desktop/mobile 390/320 px. Richiede Playwright Python e `/usr/bin/chromium`, come gli altri script di verifica; la pagina è servita dal routing del test senza asset esterni.
