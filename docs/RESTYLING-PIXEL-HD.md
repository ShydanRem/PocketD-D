# Restyling pixel art 2D — v0.45

La direzione concordata è un piccolo RPG fantasy personale, più videogiocoso e
ispirato alla profondità e alla luce HD-2D. Questa prima revisione è implementata
nel firmware LVGL: le anteprime provengono dal codice dell'app sul dispositivo
eseguito su host, con personaggi inventati.

## Intervento attuale

- **Eroe:** diorama da 300×96, con montagne distanti, luna, acquedotto, portale
  in rovina, foschia con dithering, lastricato, ombre, vegetazione e lanterne.
  Palette di 16 colori con distanza fredda e accenti caldi. Avatar da 72×90,
  più grande del precedente 48×60, con ritocchi a pieghe, bordi e dettagli.
- **HUD:** PF e contatori con cifre pixel originali, barra segmentata,
  temporanei nella scena, controlli PF separati e sempre raggiungibili.
  Restano leggibili PF 999/999, temporanei 999, nomi lunghi e PF zero.
- **Menu:** cornice illustrata, marchio pixel, pannelli con gradiente verticale,
  contorni netti, selezione oro e stati premuto/disabilitato distinti.
  Le aree touch rimangono nel cerchio e alte almeno 44 px.
- **Sezioni:** libro, bisaccia e falò originali per magie, inventario e riposi;
  cristallo per il contatore. Aspetto riusa il paesaggio dietro il ritratto.
- **Dadi:** tavolo intarsiato a gradini e riepilogo evidenziato; il 20 naturale
  ha accento oro e l'1 accento di avviso, senza attribuire automaticamente
  successo/fallimento alle prove. Numeri, modificatori e totale restano testo.
- **Atmosfera:** due luci di lanterna e tre scintille usano il timer cosmetico
  dell'avatar. Animazione disattivata o altra sezione: niente aggiornamenti
  ambientali. In attenuazione il callback non aggiorna né avatar né luci.

La profondità è costruita con livelli 2D e luce disegnata nella palette. Non
sono stati introdotti un motore 3D, shader o post-processing. Nomi e descrizioni
mantengono Montserrat: il font pixel limitato a marchio/numeri non sostituisce
caratteri importati o descrizioni lunghe.

## Comandi e stato

PWR breve, BOOT breve e BOOT lungo conservano le associazioni configurabili;
PWR lungo resta riservato all'alimentazione. Non vengono aggiunti gesti obbligatori.
Le schermate di conferma e il lancio proteggono ancora gli input incompatibili.

Scena, luci, pannelli e cifre non chiamano RNG e non scrivono NVS. L'avatar resta
una scelta estetica a 81 combinazioni: nessuna classe, arma o proprietà dedotta
dall'aspetto. Le nuove pose non cambiano gli indici salvati.

PDD23, USB8, backup PC v11 e partizioni sono invariati. Nessuna conversione dei
salvataggi, lettura Beyond o modifica della raccolta personale. Il PC conserva
il suo ruolo di preparazione; aggiorna solo le maschere dell'anteprima avatar
condivise con il firmware e l'HTML autonomo generato.

## Asset, memoria e riproducibilità

Tutto il disegno aggiunto è originale e generato da
`scripts/generate-device-art.py`, senza font o illustrazioni esterni.
`--check` confronta gli asset con la sorgente. `pixel_font.h` contiene lettere
per il solo marchio fisso e cifre/segni necessari per i numeri.

Asset I4 effettivamente collegati: **126818 byte** tra cornice, diorama,
quattro icone di navigazione e quattro oggetti, palette incluse; **1795 byte**
di bitmap per i due font pixel, oltre ai descrittori. Le maschere avatar restano
17280 byte e la palette base 60 byte. Il buffer avatar statico rimane **5824
byte**, sufficiente per 48×60, 72×90 o 96×120: espansione esatta dei pixel,
nessuna trasformazione LVGL di un'immagine indicizzata.

Le immagini sono costanti in flash. Oggetti, decoder, font e rendering LVGL
richiedono comunque memoria dinamica. Non è una misura della RAM disponibile,
dei consumi o della fluidità sull'hardware.

```sh
python3 scripts/generate-device-art.py --check
npm run firmware:test
python3 scripts/preview-device.py
python3 scripts/preview-dice.py
python3 scripts/preview-dice.py --pool-only
```

Nell'archivio, `firmware-host-preview/device-preview.png` raccoglie Eroe, Dadi,
Scheda e Usi. `avatar-preview.gif` mostra avatar/luci; `dice-preview.gif` e
`pool-preview.gif` mostrano i lanci. Le altre gallerie sono rigenerate nello
stesso tema, incluse importazione, condizioni, errori e schermate dense.

## Verifiche

- Suite host completa, compresi i 41411 confronti PC → USB → C e i percorsi di
  conferma, annullamento, salvataggi, errori, import e tasti.
- 81 aspetti × quattro pose: trasparenza, guardie ai limiti dei buffer, replica
  esatta 2×/3×/4× e rifiuto degli input invalidi senza modificare l'output.
- UI LVGL: timer/luci senza RNG o salvataggi; pausa in attenuazione e pulizia
  alla navigazione, animazione disabilitabile, nomi/valori massimi, testo e
  controlli contenuti nel cerchio. Dadi singoli e pool fino a 18: guardie,
  risultato indipendente dall'animazione e accelerazione senza rilancio.
- 144 test JavaScript, build PC e HTML autonomo, ASan/UBSan per avatar, dadi,
  pool e UI passati (leak detector disattivato).
- ESP32-S3: **987840 byte**, partizione app 4 MiB **76% libera**, ESP-IDF 5.5.5
  e LVGL 9.4.0. Nessun flash o collaudo fisico.

## Cosa resta della revisione estetica

Questa versione stabilisce la direzione; non chiude il blocco estetico.
Restano scene e illustrazioni più ampie per altre sezioni, ulteriori sprite e
personalizzazioni, icone specifiche per oggetti/magie/condizioni, transizioni
tra pagine e feedback più ricchi per le azioni. Leggibilità, precisione touch,
memoria, tempi di disegno e consumi vanno osservati sul dispositivo reale.
La sospensione e l'alimentazione restano nel blocco del prodotto fisico.
