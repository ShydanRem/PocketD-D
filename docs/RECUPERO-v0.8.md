# Recupero verificato di PocketD-D v0.8

Recupero completato il 16 ottobre 2026 dalla chat **Configura PocketD-D**, ID
`01a13ebf-f38c-76f5-be98-588d1cf39057`, host `durable`.

## Provenienza e metodo

- Letta l'intera cronologia in tre pagine, seguendo i cursori fino all'inizio con
  `includeOutputs: true`, `turnLimit: 10`, `maxOutputCharsPerItem: 20000`.
- Ripristinati i tre documenti iniziali da `fileChange`, poi applicate in ordine
  cronologico 56 scritture selezionate da 30 elementi `commandExecution`, fino
  all'incremento 0.8 e al formato backup v8.
- Esaminati i comandi prima dell'applicazione; estratti solo i blocchi che scrivono
  sorgenti, test, configurazione e documentazione. Non rieseguiti server,
  installazioni, richieste web o diagnostica della sessione storica.
- La sostituzione parzialmente oscurata nel comando dell'editor mappa è stata
  recuperata dall'output successivo dello stesso comando, che mostra integralmente
  `token:${item.id}` e la selezione della pedina. Nessun segreto da recuperare.
- Confrontate 89 righe di `src/main.js` con l'ultima lettura storica disponibile,
  tenendo conto della successiva correzione italiana: corrispondono.
- Prima del recupero il repository conteneva solo `.git`, senza commit o ref
  remoti. `.git` è stato conservato e verificato byte per byte durante la
  ricostruzione. Commit e push vengono eseguiti solo dopo le verifiche.

Il `package.json` storico conserva versione `0.1.0` e intervallo Vite `^7.1.0`:
la versione funzionale del prototipo è 0.8, indicata nell'interfaccia e nella
documentazione. Il lockfile storico non era presente nei dati recuperabili;
`package-lock.json` è stato generato nuovamente fissando Vite **7.3.7**, usato dalla
build storica. Non sono state introdotte nuove funzioni.

## Verifica attuale

Ambiente: Node.js **24.19.0**, npm **11.9.0**, Vite **7.3.7**.

- Installazione pulita con `npm ci`: riuscita.
- `npm test`: **43 test passati**, 0 falliti, 0 saltati.
- `npm run build`: riuscita; rigenerati `dist/` e `PocketDD-demo.html`.
- Gli artefatti compilati hanno gli stessi nomi con hash e le stesse dimensioni
  della build storica 0.8:
  - `index-BBIsHtZx.css`: 22,91 kB (gzip 6,05 kB).
  - `index-D-OhdHkd.js`: 60,92 kB (gzip 19,75 kB).
- Rieseguito il flusso Chromium storico della v0.8 sull'HTML autonomo, senza
  scaricare asset esterni: zoom, panoramica entro i bordi, ricentramento dopo
  movimento, coordinate globali, selezione del bersaglio, portata, muri, override
  DM per un singolo tiro e non persistente, portata personalizzata, backup v8,
  CA manuale e viewport desktop/mobile 390 e 320 px. Esito positivo, nessun errore
  JavaScript rilevato.
- `node --check src/main.js` e controllo whitespace: riusciti.

I dettagli delle operazioni e gli hash dei file recuperati sono in
[`recovery-manifest.json`](recovery-manifest.json). L'archivio scaricabile comprende
sorgenti, lockfile, documentazione, HTML autonomo, build, risultati delle verifiche
e un bundle Git del commit di recupero. Esclude `node_modules` e la configurazione
locale `.git`.

## Avvio locale

Aprire `PocketDD-demo.html` per provare subito il prototipo. Per sviluppare:

```sh
npm ci
npm test
npm run build
npm run dev -- --port 5173
```

Il firmware ESP32, la verifica sul dispositivo fisico, Beyond e il multiplayer
restano fuori da questo recupero.
