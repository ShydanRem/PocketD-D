# Dadi animati sul dispositivo — v0.16

Il firmware mostra un d20 originale in pixel art che entra nel vassoio, ruota,
rimbalza tre volte e si arresta con il **valore naturale** sulla faccia. Il
modificatore e il totale rimangono sotto: un risultato 20 con bonus +3 mostra
20 sul dado e 23 nel totale. Nessun successo automatico viene aggiunto alle
regole delle prove di caratteristica.

Da **Scheda**, scegliere una caratteristica, poi **Lancia**. Il pulsante **Base**
cicla Base → Vant → Svant → Base. Vantaggio e svantaggio lanciano due dadi da
lati opposti, con partenze sfalsate. Alla fine quello scelto resta dorato e
ha un indicatore sotto, quello scartato diventa verde/grigio. In caso di parità
si evidenzia il primo. Il 20 naturale selezionato riceve un breve effetto di
scintille. Un 20 scartato non attiva l'effetto.

Durante il volo il pulsante diventa **Accelera**; anche toccare i dadi conclude
l'animazione immediatamente. Il risultato è lo stesso. Un dado già fermo non
rilancia se toccato: per un nuovo tiro usare **Rilancia**. Modalità, navigazione
e “Libero” sono disabilitati durante il breve volo e riattivati all'arresto.
Il task USB continua a essere servito; un'anteprima USB conclude la presentazione
del tiro esistente e sospende la pagina. Annullandola si ritrova lo stesso esito.

## Implementazione e memoria

- `pd_d20` estrae l'esito una sola volta, con lo stesso RNG hardware/rejection
  sampling della v0.15. L'animazione non chiama RNG, non ricalcola l'esito e non
  scrive NVS o altre risorse del personaggio.
- `firmware/core/dice_animation.c` separa la presentazione dal calcolo del tiro.
  Il moto usa parabole decrescenti ed easing interi. Le collisioni visualizzate
  non determinano il valore casuale. Il tempo deriva dal tick monotono, con wrap
  uint32 gestito; un frame saltato non prolunga il volo.
- Atlas originale `firmware/assets/d20_atlas.h`: 24 orientamenti da 48×48 pixel,
  palette indicizzata a 4 bit, **27.648 byte in flash**. Icosaedro rasterizzato
  offline, ombre e facce a colori piatti; cifre pixel disegnate dal firmware.
  Nessun asset remoto, texture o motore 3D da caricare sul dispositivo.
- Riproduzione su canvas RGB565 320×128: **81.920 byte dinamici** solo nella
  pagina Dadi. L'allocatore IDF configurato può usare PSRAM per questo blocco;
  il buffer DMA del display rimane interno. Uscire dalla pagina libera canvas,
  buffer e timer. In caso di memoria insufficiente, il numero e il totale
  restano disponibili senza estrarre un altro risultato.
- Timer LVGL ogni 33 ms (obiettivo circa 30 aggiornamenti/s). A ogni frame si
  invalida solo il canvas, senza ricreare tutta l'interfaccia. Volo singolo
  1.100 ms, doppio 1.240 ms; effetto 20 naturale +260 ms. Non sono misure di FPS
  su hardware: display, fluidità e consumo vanno ancora provati sulla scheda.

Rigenerare gli asset, senza dipendenze Python aggiuntive:

```sh
python3 scripts/generate-dice.py
```

La rigenerazione deve lasciare l'header invariato. Build e test:

```sh
npm run firmware:test
idf.py -C firmware build
```

Il test UI salva fotogrammi del **codice LVGL reale eseguito su host** in
`firmware/tests/out/ui-build/`. Per ottenere una GIF (Pillow opzionale, escluso
dal runtime):

```sh
python3 -m pip install Pillow
python3 scripts/preview-dice.py
```

L'archivio di rilascio include questa anteprima, se generata. È un rendering
su PC, non una ripresa della scheda fisica né una prova delle prestazioni.

## Verifiche

Test del renderer per tutte le facce 1–20, uno/due dadi, fasi di volo, effetto 20,
wrap del timer, limiti del buffer, frame finale stabile e accelerazione con esito
identico. Verificati evidenziazione del secondo dado e gestione della parità.
I test LVGL esercitano il timer reale, risultato naturale/totale, vantaggio e
svantaggio, blocco degli eventi durante il volo, rilancio esplicito, accelerazione
via pulsante e canvas, memoria insufficiente, liberazione ripetuta e raccolta
invariata di PF/contatori (nessuna scrittura durante i tiri).

Le stesse prove native vengono eseguite anche con AddressSanitizer e
UndefinedBehaviorSanitizer: nessun errore di accesso o comportamento indefinito
nei percorsi verificati. La verifica delle perdite non è abilitata per l'istanza
LVGL globale del test, che rimane viva fino all'uscita del processo.

Ancora da provare: flash e touch sulla Waveshare reale, FPS e latenza percepita,
heap/PSRAM a runtime, eventuali tearing o colori scambiati, consumo a batteria.
D4, d6, d8, d10, d12, dadi percentuali e lanci di danno multipli restano un
incremento successivo. Il simulatore browser conserva la sua precedente grafica:
il lavoro di questa versione riguarda il firmware del dispositivo.

Build ESP32-S3 v0.16: **573.104 byte** (circa 560 KiB), **86% della partizione
applicativa da 4 MiB libera**, DIRAM statica 78.287 byte. Il canvas è dinamico e
non è incluso in questo valore statico. Sono dimensioni di compilazione, non
misure dell'heap o del consumo durante il lancio sulla scheda.
