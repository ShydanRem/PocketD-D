# Dadi animati sul dispositivo — v0.17

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
e “Altri” sono disabilitati durante il breve volo e riattivati all'arresto.
Il task USB continua a essere servito; un'anteprima USB conclude la presentazione
del tiro esistente e sospende la pagina. Annullandola si ritrova lo stesso esito.

## Altri dadi e lanci multipli (v0.17)

Da Dadi premere **Altri**. Selezionare tipo (d4/d6/d8/d10/d12/d20), quantità
**1–6** e bonus **−15…+30**. Il tipo si sceglie da sei pulsanti grandi; quantità
e bonus hanno controlli separati +/−. **Somma** mantiene anche totali negativi;
**Danni** applica minimo zero alla somma più bonus. Il bonus è aggiunto una sola
volta al gruppo, non ad ogni dado. Esempio: 1,6,1,6,1,6 con +3 dà 24; 1d4−15
con faccia 1 dà −14 in Somma e 0 in Danni.

**Esci** torna al tiro precedente senza cambiarne tipo, facce, bonus o totale.
**Prova** apre il d20 libero con bonus zero; dalla scheda resta disponibile il
d20 con modificatore di caratteristica. **Lancia** conferma la preparazione e
estrae tutti i valori una volta sola. **Rilancia** ripete i parametri dell'ultimo
tiro confermato, anche se una preparazione successiva è stata annullata.

I dadi hanno forme originali distinte e palette coerenti: tetraedro, cubo,
ottaedro, trapezoedro pentagonale, dodecaedro e icosaedro. Da uno a tre dadi
sono mostrati su una riga; da quattro a sei su due righe, senza nascondere facce
in pagine successive. Il singolo dado occupa un riquadro da 96 px, ridotto a
64 px per gruppi da quattro a sei; cifre rispettivamente da 20/15 px di altezza.
La leggibilità sul display fisico deve ancora essere verificata.

I lanci Somma/Danni sommano **tutti** i dadi: nessuno viene attenuato o scartato,
nessun 20 attiva l'effetto del d20 di prova. Vantaggio e svantaggio restano
esclusivi delle prove d20 e non sono ereditati dai danni. Nessun consumo di
risorse, applicazione di PF, critico, resistenza, attacco o bonus di arma è
automatico. I parametri manuali e l'esito restano in RAM durante la sessione;
non vengono aggiunti al formato del personaggio o al salvataggio NVS.

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
  Gli altri cinque atlas sono in `poly_atlas.h`, altri **138.240 byte**, per un
  totale di **165.888 byte** in flash. Nessun asset remoto, texture o motore 3D
  da caricare sul dispositivo.
- Riproduzione su canvas RGB565 320×128: **81.920 byte dinamici** solo nella
  pagina Dadi. L'allocatore IDF configurato può usare PSRAM per questo blocco;
  il buffer DMA del display rimane interno. Uscire dalla pagina libera canvas,
  buffer e timer. In caso di memoria insufficiente, il numero e il totale
  restano disponibili senza estrarre un altro risultato.
- Timer LVGL ogni 33 ms (obiettivo circa 30 aggiornamenti/s). A ogni frame si
  invalida solo il canvas, senza ricreare tutta l'interfaccia. Volo singolo
  1.100 ms, doppio 1.240 ms; ogni dado aggiuntivo parte con 140 ms di ritardo
  (sei dadi: 1.800 ms). Effetto 20 naturale di prova +260 ms. Non sono misure di FPS
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
python3 scripts/preview-dice.py --pool-only
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
Dadi percentuali, gruppi con tipi diversi, critici e integrazione con le armi
restano incrementi successivi. Il simulatore browser conserva la sua precedente grafica:
il lavoro di questa versione riguarda il firmware del dispositivo.

Misura storica ESP32-S3 v0.16: **573.104 byte** (circa 560 KiB), **86% della partizione
applicativa da 4 MiB libera**, DIRAM statica 78.287 byte. Il canvas è dinamico e
non è incluso in questo valore statico. Sono dimensioni di compilazione, non
misure dell'heap o del consumo durante il lancio sulla scheda.

## Verifiche aggiuntive v0.17

- Core: sei tipi × quantità 1–6, tutte le facce, bonus estremi, somma negativa,
  minimo zero per danni, rifiuto di input non validi prima di leggere il RNG.
- Rejection sampling: coda distorta rifiutata per d6/d10/d12/d20 e UINT32_MAX
  correttamente accettato per d4/d8, senza troncare 2^32 a zero.
- Renderer: ogni forma/faccia/quantità, moto, wrap del tick, guardie del buffer,
  stabilità del risultato dopo accelerazione. Il precedente d20 resta verificato.
- UI LVGL su host: scelta per touch, 6d6+3, annulla e rilancia con parametri
  confermati, ritorno alla prova d20, danni a zero, 6d20+30=150, memoria
  insufficiente. Nessuna scrittura di PF/contatori o NVS durante questi tiri.
- Test ripetuti anche con AddressSanitizer/UndefinedBehaviorSanitizer. Il loro
  esito e la build non sostituiscono la prova del dispositivo fisico.

I backup USB/NVS mantengono le stesse versioni; nessuna migrazione o lettura
Beyond è necessaria per questo incremento. Le prove usano soltanto dati sintetici.

Build ESP32-S3 v0.17: **713.808 byte** (circa 697 KiB), partizione
app da 4 MiB **83% libera**, DIRAM statica **78.335 byte**. Il canvas rimane
da 81.920 byte dinamici; heap, prestazioni e consumo vanno misurati sulla scheda.
