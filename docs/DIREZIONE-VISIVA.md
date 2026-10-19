# Direzione visiva del dispositivo — v0.18

PocketD-D deve essere bello da tenere sul tavolo e da guardare tra un turno e
l'altro: un piccolo RPG fantasy contemporaneo sul display AMOLED circolare.
La qualità visiva è un requisito del prodotto insieme alle funzioni di scheda.
Il PC rimane lo strumento per importazione, configurazione e backup.

## Linguaggio grafico

- Fondi blu quasi neri, superfici sobrie, contorni sottili e pochi accenti luminosi.
- Pixel art originale su griglia intera per personaggi, ambienti, icone e dadi.
  Ingrandimenti per replica dei pixel; niente sfocatura o filtri fotografici.
- Menta per azioni principali e PF, oro per selezione e identità, viola per
  risorse. Errori accompagnati da testo esplicito oltre al colore.
- Testi Montserrat leggibili, con dimensioni e spazi coerenti. La pixel art
  caratterizza l'oggetto; nomi e valori della scheda devono restare chiari.
- Bordi del cerchio dedicati a decorazioni discrete. Contenuti e controlli
  nell'area sicura; pulsanti con etichetta visibile, senza gesti indispensabili nascosti.
- Animazioni brevi, con esito dei dadi indipendente dalla presentazione.
  Nessuna animazione deve consumare risorse, inventare valori o cambiare un tiro.

La palette condivisa è in `firmware/core/visual.h`: inchiostro `#0b101c`,
superficie `#162333`, testo `#f0eee5`, menta `#76dfc5`, oro `#f2cc81`,
viola `#bc9bdb`, avviso `#ffa999`. Il vassoio dei dadi usa lo stesso fondo
dell'interfaccia, senza un rettangolo di colore diverso.

## Implementato nel firmware

- Cornice circolare, marchio discreto, stato del salvataggio sempre visibile.
- Eroe con scena notturna originale, avventuriero, luna, pini e rovina;
  livello/CA separati, PF grandi, PF temporanei, barra proporzionale e controlli.
- Navigazione con quattro icone pixel art e selezione evidenziata.
- Caratteristiche, contatori, preparazione dei dadi, scelta del tipo e conferma
  USB con lo stesso linguaggio grafico. Feedback premuto e disabilitato distinto.
- Nomi lunghi limitati a una riga con ellissi: non invadono i valori sottostanti.

L'avventuriero è un **avatar base originale e statico**: non è l'aspetto del
personaggio Beyond e non dipende dalla sua classe o dall'equipaggiamento.
Non viene trasferita o sovrascritta alcuna preferenza dell'avatar PC.
Livello, CA, PF, caratteristiche e contatori visualizzati restano quelli dello
stato attivo; le illustrazioni decorative non aggiungono regole.

## Asset e riproducibilità

`scripts/generate-device-art.py` genera `firmware/assets/device_art.h`:
scena 150×36 disegnata su griglia, replicata a 300×72; quattro icone 12×12
replicate a 24×24. Formato LVGL I4, palette BGRA a 16 colori, costanti in flash.
**12.272 byte** complessivi di dati immagine, oltre ai descrittori. Nessun file
esterno, servizio online o nuova allocazione canvas applicativa per la scena.
Decoder e oggetti LVGL usano comunque heap: non è una misura di memoria a runtime.

```sh
python3 scripts/generate-device-art.py
python3 scripts/generate-device-art.py --check
npm run firmware:test
python3 scripts/preview-device.py
python3 scripts/preview-dice.py
python3 scripts/preview-dice.py --pool-only
```

Le anteprime richiedono Pillow e si trovano in
`firmware/tests/out/ui-build/`. `device-preview.png` riunisce Eroe, Dadi,
Scheda e Usi. Sono fotogrammi del **codice firmware LVGL eseguito su host**,
con dati sintetici; non concept generati e non fotografie del dispositivo.
L'archivio include PNG, GIF e altri fotogrammi se prodotti prima del pacchetto.

## Verifiche v0.18

125 test JavaScript e build PC riusciti. Core C/NVS, dadi, pool e uploader finto
confermano i percorsi precedenti. La UI LVGL verifica navigazione, PF/annulla,
contatori, conferma USB e lancio; rettangoli touch nel cerchio, testi dei pulsanti
contenuti e campi di testo senza sovrapposizioni. Verificati nome di 48 caratteri,
PF 999/999, temporanei 999, PF zero ed errore memoria. Il controllo dei nomi lunghi
ha evidenziato e risolto un ritorno a capo che invadeva i PF.

Cross-compilazione per ESP32-S3 riuscita. Non è stata eseguita alcuna scrittura
sulla scheda fisica. Nitidezza, luminosità, touch, memoria disponibile e fluidità
devono ancora essere osservati sul dispositivo. Nessuna lettura personale Beyond
o modifica alla raccolta PC per questa revisione. Formati USB/NVS invariati.

## Da sviluppare con questa direzione

1. Avatar personalizzabile trasferibile dal PC, con animazione di attesa sobria
   e aspetto coerente con le scelte esplicite del giocatore.
2. Componenti per elenchi, dettagli e testi lunghi mentre abilità, inventario,
   attacchi e magie vengono portati sul dispositivo. Icone specifiche e pochi
   tocchi per le azioni frequenti, evitando di stipare tutto nella schermata Eroe.
3. Illustrazioni coordinate per nuove sezioni, stati vuoti e feedback delle azioni.
4. Transizioni leggere e preferenza per ridurre le animazioni. Le animazioni
   ambientali restano subordinate alla gestione energetica da implementare.

Questa versione stabilisce la prima base visiva reale; non completa il
trasferimento della scheda estesa, l'editor avatar o le funzioni di alimentazione.
