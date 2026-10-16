# Banco di prova Beyond — 16 ottobre 2026

**Esito attuale: il percorso locale è verificato con risposte sintetiche; la lettura reale dal cloud è bloccata dalla configurazione di rete. La fattibilità completa dell'importazione non è ancora dimostrata.** L'app principale resta v0.12, con formato scheda v9 invariato.

## Prova reale effettuata

L'utente ha fornito il link di un proprio personaggio da usare per la prova. Il connettore PocketD-D ha tentato la lettura in sola lettura dal servizio `character-service.dndbeyond.com`, senza credenziali e senza passare da Foundry o MrPrimate.

Il controllo della rete dell'ambiente riporta una policy applicata con destinazioni limitate; il dominio del servizio non è incluso. Una richiesta HTTPS di verifica ha ricevuto **HTTP 403 durante CONNECT dal proxy cloud**, prima della connessione al servizio. Il comando di lettura sul personaggio ha restituito `NETWORK_ERROR`.

Non è una risposta HTTP del personaggio da parte di Beyond: non dimostra che la scheda sia privata, inesistente o non importabile. Non è stata ricevuta la sua risposta JSON e non è stato possibile confrontare caratteristiche, PF o CA con l'originale. Il link personale e dati del personaggio non sono inclusi nel repository.

Per proseguire qui occorre consentire **`character-service.dndbeyond.com` sulla porta HTTPS 443** nella configurazione di rete dell'ambiente. I permessi di rete del singolo comando non cambiano la lista di domini del proxy. Non sono stati cambiati proxy, controlli TLS o percorsi per aggirare il rifiuto. In alternativa il banco può essere eseguito sul PC dell'utente, con una rete che raggiunga il servizio.

## Eseguire la prova sul PC

Richiede **Node.js 24.19 o successivo**, versione verificata per i comandi con `--use-env-proxy`. I soli strumenti Beyond usano moduli Node nativi: non richiedono Foundry, credenziali, dipendenze npm installate o una build Vite. Usare la cartella del progetto estratta dall'archivio o clonata da GitHub.

```sh
npm run beyond:lab
```

Aprire **http://127.0.0.1:5174**, incollare il link del proprio personaggio pubblico e premere **Prova lettura**. Il server rimane accessibile soltanto dal computer locale. Il pulsante **Scarica resoconto JSON** conserva il risultato della prova. Terminare il server dal terminale con Ctrl+C.

È disponibile anche il comando da terminale:

```sh
npm run beyond:probe -- "https://www.dndbeyond.com/characters/ID"
```

Sostituire `ID` con il numero reale. L'opzione `--out percorso/report.json` salva il resoconto in un file nuovo; rifiuta di sovrascrivere un file esistente. Il resoconto contiene soltanto i campi selezionati dall'anteprima, non il JSON sorgente completo. Contiene comunque nome e ID del personaggio: conservarlo come un proprio dato personale.

Per usare una porta diversa del laboratorio impostare la variabile `POCKETDD_BEYOND_PORT`. Il connettore usa le impostazioni proxy dell'ambiente attraverso il supporto Node, mantenendo le verifiche TLS.

## Che cosa prova il laboratorio

| Parte | Stato |
| --- | --- |
| URL/ID → destinazione fissa del servizio | Verificato con test locali. Sono accettati URL HTTPS Beyond nei formati previsti o un ID positivo; gli altri indirizzi vengono rifiutati. |
| Richiesta HTTP senza credenziali, redirect non seguiti | Verificata con risposte controllate e server HTTP locali. |
| Timeout di header/corpo, limite 2 MiB, HTML/challenge, JSON non valido, errori HTTP/rete | Verificati localmente. |
| Nome, livello, caratteristiche base/bonus/override, anteprima PF e riepilogo inventario | Verificati su fixture completamente sintetiche. |
| Corrispondenza con la scheda Beyond dell'utente | **Non verificata:** il cloud non ha raggiunto il servizio. |
| Importazione come personaggio PocketD-D | **Non implementata in questa prova.** `canImport` è sempre `false`; nessuna scrittura della raccolta. |
| Autenticazione privata, sincronizzazione e firmware | Non implementati o provati dal laboratorio. |

Caratteristiche e PF sono esplicitamente un'**anteprima parziale**: non vengono riconciliati modificatori di classe, talenti, oggetti ed effetti. Anche un valore numerico plausibile può non corrispondere alla scheda finale. La CA rimane non determinata. Non vengono convertiti competenze, attacchi, magie, slot o condizioni. Il riepilogo dell'inventario conserva nomi, quantità e stato equipaggiato, fino a 60 voci, senza trasformarli in armi o capacità PocketD-D. Il resoconto non è un backup importabile.

## Verifiche ripetibili

```sh
npm test
npm run build
python scripts/check-beyond.py
```

Risultati: **89 test passati** (78 precedenti e 11 nuovi), build riuscita. La demo principale generata conserva gli stessi asset della v0.12. I nuovi test usano trasporti iniettati e server HTTP locali, senza contattare Beyond; includono dati incoerenti, ID diversi, limiti, timeout durante la lettura e gestione degli errori.

La verifica browser richiede Playwright Python e Chromium in `/usr/bin/chromium`, come gli altri script del progetto. Serve il vero laboratorio locale con un connettore di test e risposte sintetiche, indicate anche nella pagina come **simulate**. Verifica anteprima, download, testo ostile reso come testo, input/errori, stato di attesa e pulsanti, viewport desktop/mobile 390/320 px e conservazione dello storage locale. Nessun errore JavaScript rilevato. Non è una verifica di accesso remoto.

## Prossima verifica necessaria

Ripetere il comando reale con il link già fornito, quando la destinazione sarà raggiungibile, e confrontare il resoconto con la scheda visibile su Beyond. Solo dopo la ricezione effettiva si potrà stabilire se la risposta corrente rispetta il formato atteso, quali calcoli mancano per quel personaggio e se ulteriori restrizioni del servizio impediscono l'accesso. Il banco non avvia tentativi automatici ripetuti e non modifica Beyond.

I criteri e le opzioni per dispositivo e PC restano in [BEYOND-FATTIBILITA.md](BEYOND-FATTIBILITA.md).
