# Ponte Beyond v0.79: prove e limiti

28 ottobre 2026. Nessuna scrittura autenticata, nessun cookie o JSON completo
sorgente usato nelle prove. Raccolta personale conservata. I dati del ponte
nei test sono tutti sintetici e salvati in cartelle temporanee private.

| Controllo | Risultato |
| --- | --- |
| JavaScript | 283 test passati, inclusi protocollo rigoroso, TLS reale locale, SAN IPv4, coda persistente e worker con API Chrome simulate |
| Storico C → ponte | 32 stati / 31 assegnazioni singole, confronto facce/totali e 12 configurazioni malformate rifiutate |
| Servizio nativo simulato | Conferma obbligatoria, impronta, namespace NVS, errore/riprova, riavvio, epoch, sospensione e rimozione |
| UI del firmware su PC | 126/126 viste, conferme protette, errore di salvataggio visibile, PG immutato e tasti configurabili conservati |
| ASan / UBSan | UI e nuovi parser/servizi host passati; non una misura della RAM ESP32 |
| Chromium | Portale Wi-Fi, pannello ponte, formati mobile e adattatore PF/broker su pagina interamente sintetica |
| Compilazione ESP-IDF 5.5.5 | ESP32-S3, 0.79.0, partizioni invariate; non un flash o collaudo fisico |
| Distribuzione | ZIP PC estratto senza node_modules, ponte e app avviabili, file estensione/protocollo/guida confrontati, installer verificato nel browser |
| Estensione MV3 installata | **Bloccata**: la policy amministrativa di Chromium dell’ambiente vieta estensioni non pacchettizzate. Nessun tentativo di aggirarla |
| Account Beyond e dispositivo | **Non eseguito**: nessuna pubblicazione di dadi o modifica dei PF sul tuo account; radio, LAN, latenza e batteria da misurare |

## Come ripetere i controlli software

- `npm test` con Node.js 24 attuale.
- `IDF_PATH=… npm run firmware:test` con dipendenze host e OpenSSL di sviluppo.
- `python3 scripts/check-bridge-browser.py` e `python3 scripts/check-wifi-portal.py`.
- `python3 scripts/check-bridge-extension.py`: richiede Chromium con estensioni
  non pacchettizzate consentite. Codice uscita 2 indica il blocco amministrativo,
  non un test passato. Anche quando passa usa soltanto una pagina sintetica.
- Dopo la build: `python3 scripts/check-pc-package.py` e
  `python3 scripts/check-installer-ui.py`. La shell Windows non è eseguita qui.

## Cosa deve ancora dimostrare la prova reale

1. Abbinamento TLS e rete 2,4 GHz dal Pocket, fluidità e memoria durante il gioco.
2. Un solo tiro nel registro Beyond, facce/bonus/totale identici; persistenza
   dopo reload e destinazione personale/campagna. L’eco del broker resta
   «osservato», non una garanzia della persistenza del registro.
3. PF attuali e temporanei sulla scheda di prova: comandi realmente accettati,
   rilettura concordante dal connettore pubblico e conservazione dopo reload.
4. Wi-Fi interrotto, ponte chiuso, doppie finestre, ACK perso e conflitto con
   un cambiamento esterno. Nessun reinvio automatico di un tentativo incerto.
5. Ritest separato dell’importazione USB che mostrava flash nero e PG Elian.

Sincronizzazione inversa, animazione dei dadi Beyond, PF massimi, condizioni,
inventario, slot e utilizzi non sono implementati dal ponte. Il gioco offline
resta disponibile senza servizi PC. [Guida operativa](PONTE-BEYOND.md).
