# Un piccolo mondo — prototipo nativo della nuova UI

**Documento storico del commit `66625ff`.** La successiva
[v0.48](HOME-COMPAGNO.md) integra home/menu/PF nel firmware. Il renderer, i PNG
e il generatore sono stati spostati in `firmware/ui/companion/`; il programma di
prova resta in `firmware/prototypes/companion/`. Le descrizioni qui sotto
documentano lo stato della prima proposta, con un unico avatar dimostrativo.

Il riferimento del 21 ottobre 2026 è un **compagno fantasy simile a un Tamagotchi**:
avatar intero nella home, ambiente pixel art con profondità e luce, grandi icone
riconoscibili e reazioni visive. La nuova proposta ha ricevuto un riscontro
positivo come direzione; la qualità finale e le singole schermate sono ancora
da valutare. La revisione estetica v0.47 non è piaciuta e non è il riferimento
da estendere. Il codice di produzione resta v0.47: nessun rollback è implicito.

## Cosa esegue davvero questa prova

`firmware/prototypes/companion/` contiene un modulo C con **LVGL 9.4.0** e una
prova host separata, a **466×466**. Non è una tavola generata con i numeri impressi
nell'immagine: sfondo, avatar, pulsanti, icone, valori e feedback sono oggetti
separati del renderer. Il target CMake `companion` è opzionale e disattivato
per impostazione predefinita. Non è collegato a `firmware/main/main.c`.

- Home: avatar intero, nome, livello, PF effettivi/massimi, CA e PF temporanei.
- Accessi Dadi, Menu e Zaino; toccare i PF apre i relativi comandi.
- Menu: Attacco, Dadi, Magie, Zaino, Riposo e Scheda, con icone **64×64** e
  pulsanti **88×100**. La home usa due varianti **48×48** in pulsanti **72×76**.
- Pannello PF: -1, +1, annulla e ritorno. Il programma di prova usa il vero
  `pd_hp` del core con dati sintetici e salvataggio simulato in memoria.
- Le icone del menu emettono eventi di destinazione; le sei funzioni complete
  restano nella UI di produzione e non sono ancora collegate al prototipo.
- La selezione da tasto ha contorno visibile, scorre i controlli disponibili e
  attiva quello selezionato. Gli agganci pubblici sono pronti; il collegamento
  a PWR breve, BOOT breve e BOOT lungo configurabili è ancora da realizzare.
  PWR lungo resta riservato all'alimentazione.
- Piccolo movimento idle, salto alla cura, arretramento al danno e particelle.
  Sono trasformazioni dello sprite: nuove pose disegnate e vere espressioni
  non sono ancora implementate.
- Il feedback mostra le variazioni effettive: un danno assorbito dai PF
  temporanei riporta la loro diminuzione. Nessun successo se la scrittura fallisce.
  Il modulo riceve i delta dal chiamante; il futuro adattatore di produzione
  dovrà passarli soltanto dopo il salvataggio riuscito.
- Animazioni disattivabili; pausa, scadenza e distruzione esplicita dei timer.
  Il rendering non scrive NVS, non consuma risorse e non usa casualità.

La metafora Tamagotchi riguarda presenza e movimento, senza nuovi bisogni,
fame o decadimenti delle caratteristiche. Il personaggio, la scena e tutti
i valori della prova sono inventati. Nessun accesso a Beyond o alla raccolta.

## Asset e limiti artistici

I tre PNG in `source/` sono sorgenti generate con ImageGen per questa direzione.
`provenance.json` conserva i prompt e la provenienza. Sono diversi dai disegni
procedurali originali della UI v0.47. Gli originali restano inalterati.

`build_assets.py` esporta i pixel per LVGL tramite ImageMagick 7: separazione
esplicita dell'atlante 3×2, alpha binario, eliminazione dei margini trasparenti,
adattamento alla griglia logica e replica esatta 2×. Nessun ridimensionamento
dei raster a runtime. I file C generati e `assets.json` restano nel build ignorato.
I raster usano **561800 byte**, esclusi descrittori, font, heap LVGL e buffer di
disegno. Non è una misura della RAM libera o dei consumi dell'ESP32.

Il prototipo ha **un solo avatar**: non rappresenta le 81 combinazioni della
versione attuale e non deduce aspetto o equipaggiamento dall'importazione.
Servono rifinitura dei pixel, pose coerenti, personalizzazione e controllo del
contrasto sul pannello fisico prima di considerare conclusa l'estetica.

## Riproduzione e anteprime

Prerequisiti: C/C++, CMake, Ninja, Python 3, ImageMagick 7 e LVGL alla revisione
fissata in `firmware/dependencies.json`. `npm run firmware:deps` recupera le
dipendenze ufficiali mancanti; per questo target non serve una scheda collegata.

```sh
cmake -S firmware/tests -B firmware/tests/out/companion-build -G Ninja \
  -DPD_BUILD_COMPANION_PROTOTYPE=ON
cmake --build firmware/tests/out/companion-build --target companion --parallel 4
(cd firmware/tests/out/companion-build && ./companion)
python3 firmware/prototypes/companion/preview.py firmware/tests/out/companion-build
```

`companion-preview.png` raccoglie home, menu e cura. `companion-heal.gif`
contiene fotogrammi reali dell'animazione del prototipo. I PNG singoli includono
focus, pannello PF, nome lungo, valori massimi, PF zero, errore di salvataggio,
temporanei e animazione disattivata. Sono **catture del codice LVGL su host**,
con maschera del display circolare, senza modifiche illustrative alle schermate.
Non sono fotografie del dispositivo. Il test è automatico e non apre una finestra
interattiva; tutte le azioni descritte sono inviate al codice dei controlli LVGL.

## Verifiche eseguite

- Target nativo compilato con `-Wall -Wextra -Werror`; prova touch, eventi delle
  sei destinazioni, focus/attivazione e controlli disabilitati riuscita.
- Rettangoli touch dentro il cerchio, nessuna sovrapposizione tra pulsanti,
  etichette contenute; nome lungo, 999/999 PF, 999 temporanei e PF zero catturati.
- Core PF: cura, annulla, assorbimento temporanei e salvataggio fallito simulato;
  nessuna mutazione durante idle o pausa. Immagine immobile con animazioni
  disattivate, feedback che scade e timer puliti dopo 80 cambi di vista.
- ASan/UBSan sul prototipo riusciti, con leak detector disattivato.
- Suite UI LVGL di produzione rieseguita e passata. Le prove JavaScript e
  l'intera suite delle regole non sono state ripetute: quei sorgenti non cambiano.
- Modulo grafico e asset compilati come oggetti per Xtensa/ESP32-S3 con le
  opzioni del progetto ESP-IDF 5.5.5. Questo verifica la compilabilità dei file,
  **non** il collegamento del nuovo flusso al firmware né la fluidità fisica.

Nessun flash o collaudo hardware. Versione, modello dati, partizioni, USB e
archivio dei personaggi restano invariati. L'archivio di questa prova contiene
sorgenti e anteprime; non viene pubblicato un nuovo binario firmware.

## Passo successivo

Valutare questa resa nativa, rifinire avatar/icone e collegare home/menu/PF allo
stato, alle conferme e ai tasti configurati già esistenti. Poi estendere il
linguaggio a dadi, scheda, inventario, magie e riposi, verificando memoria e
fluidità sul dispositivo. L'aspetto finale deve essere coerente in ogni vista.
