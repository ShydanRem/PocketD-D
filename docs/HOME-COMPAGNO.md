# Home del compagno integrata — v0.48

**Stato storico della v0.48.** La [v0.49](AVATAR-ILLUSTRATO.md) sostituisce lo sprite
procedurale con le varianti illustrate e introduce pose di cura/danno. Dimensioni
dei buffer, asset e verifiche aggiornati sono nel documento successivo.

La home ambientata, il menu a icone e il pannello PF sono ora collegati al
**firmware ESP32-S3**, attraverso le stesse azioni e gli stessi salvataggi usati
dalle altre schermate. La prova separata del commit `66625ff` resta un riferimento
storico; le anteprime di questa versione vengono da `firmware/main/main.c`
eseguito con LVGL su host e dati sintetici.

## Percorsi disponibili

- **Home:** nome, livello, PF effettivi/massimi, temporanei e CA. CA e massimo
  includono i modificatori locali supportati. Lo sfondo occupa il display.
- **PF:** toccare il valore apre -1, +1 e annulla. Una modifica salvata torna
  alla home con il delta effettivo. Un errore conserva lo stato e resta visibile.
  Un danno assorbito dai temporanei viene indicato come variazione dei temporanei.
- **Menu:** Attacco, Dadi, Magie, Zaino, Riposo e Scheda aprono i percorsi già
  implementati. Attacco apre lo zaino se manca un'arma selezionata.
- **Usi**, in fondo al menu, raggiunge contatori, stato e le relative funzioni.
  **Eroe** torna alla home. La home conserva anche Dadi e Zaino diretti.
- **Avatar:** toccarlo apre Aspetto, con animazione, Schermo e Tasti.
- **Importazione USB:** anteprima, annulla e conferma restano obbligatori.
  Annullare torna alla vista di partenza. Confermare aggiorna la copia locale
  e apre la home; la raccolta PC resta indipendente. Anche un errore dopo una
  conferma dal menu rimane visibile.

I dadi, la scheda estesa, l'inventario, le magie e i riposi mantengono ancora
le loro strutture precedenti. Sono stati rimossi gli angoli decorativi bronzo
v0.47 e ripristinati i toni blu dei pannelli precedenti. Il restyling dell'intera
applicazione non è concluso.

## Avatar: distinzione importante rispetto alla proposta

Il firmware usa **lo sprite personalizzabile già esistente**, a 96×120, dentro
la nuova scena. Sono conservate tutte le 81 combinazioni, le pose attuali e la
corrispondenza con l'anteprima PC. Il personaggio illustrato del prototipo resta
nel programma dimostrativo: non viene sostituito all'aspetto scelto dall'utente.

La qualità artistica del nuovo avatar, le sue varianti e le nuove pose espressive
sono il prossimo lavoro grafico. Per ora cura e danno muovono lo sprite e mostrano
particelle; non sono state aggiunte espressioni disegnate. Nessuna classe, arma,
nuova necessità o regola di gioco viene dedotta dall'illustrazione.

## Tasti, errori e ciclo di vita

PWR breve, BOOT breve e BOOT lungo mantengono tutte le **13 associazioni esistenti**
(compreso Spento) e le rispettive condizioni. Per esempio Round richiede effetti
temporizzati; Lancia durante un tiro accelera l'animazione senza consumare altri
numeri casuali. PWR lungo resta riservato all'alimentazione.

La nuova home non sovrascrive le associazioni, neppure dopo un'importazione.
Non sono aggiunte nuove associazioni per scorrere/attivare il focus del menu:
gli agganci del renderer restano disponibili, ma i tasti fisici continuano a
eseguire le scorciatoie configurate. Le conferme e i dadi in volo conservano
i blocchi già previsti; gli eventi delle nuove viste non possono aggirarli.

Il renderer riceve uno snapshot dei dati e gli effettivi delta salvati. Non
scrive NVS e non usa RNG. Il feedback scade dopo due secondi e una ridisegnatura
non ne prolunga la durata. I timer vengono eliminati prima di cambiare vista;
l'attenuazione ferma il movimento e cancella il feedback, senza riproporlo al
risveglio. Disattivare Animazione conserva valori e ricevuta statica.

## Asset e riproduzione

Il modulo e i sorgenti grafici sono in `firmware/ui/companion/`. I PNG generati
restano originali e hanno prompt e hash in `source/provenance.json`. I raster C
sono ora versionati: **la compilazione del firmware non richiede ImageMagick**.
Le dieci immagini codificate occupano **561800 byte**, esclusi descrittori,
font, buffer e heap del renderer. Il firmware mantiene il buffer avatar statico
da 5824 byte e il buffer di trasferimento dello schermo già esistente.

```sh
# Verifica hash dei PNG e dei raster codificati, senza strumenti grafici:
python3 firmware/ui/companion/build_assets.py --verify
# Ricostruzione esatta e confronto, richiede ImageMagick 7:
python3 firmware/ui/companion/build_assets.py --check
npm run firmware:test
python3 scripts/preview-device.py
python3 scripts/preview-dice.py
python3 scripts/preview-dice.py --pool-only
```

`home-integration-preview.png` riunisce home, menu, pannello PF e cura.
`avatar-presets.png` mostra tre aspetti effettivi; `avatar-preview.gif` e
`feedback-preview.gif` mostrano fotogrammi del firmware su host. Le immagini
del programma dimostrativo `companion` rimangono distinte e non vengono
presentate come catture del nuovo flusso di produzione.

## Verifiche e limiti

- Suite C/NVS e **41411 confronti sintetici PC → USB → C** passati.
- Suite UI completa: percorsi precedenti attraverso i nuovi controlli,
  sette destinazioni del menu, arma assente, import annullato/confermato/fallito,
  PF e temporanei, modificatori, annulla, zero PF e controlli disabilitati.
- Tutte le associazioni di ognuno dei tre gesti provate dalle tre nuove viste;
  nessuna scrittura o estrazione casuale per la sola navigazione.
- Test di ingombro touch, testi lunghi/valori massimi, timer, preferenza statica,
  attenuazione/risveglio e scadenza a cavallo del wrap del contatore temporale.
- UI e programma dimostrativo verificati con ASan/UBSan, leak detector disattivato.
- **144 test JavaScript**, build PC e HTML autonomo passati.
- Firmware ESP-IDF 5.5.5 / LVGL 9.4.0 compilato e collegato: **1605840 byte**,
  partizione app da 4 MiB **62% libera**. Questi numeri non misurano fluidità,
  memoria disponibile a runtime o autonomia.

PDD23, USB8, backup PC v11 e partizioni invariati. Nessun accesso Beyond, modifica
alla raccolta personale, flash o collaudo fisico in questa revisione.

Il prossimo blocco comprende sprite personalizzabili all'altezza della nuova
scena, nuove pose e progressiva revisione delle viste interne. Restano aperti
il collaudo hardware e i punti funzionali già elencati nella roadmap.
