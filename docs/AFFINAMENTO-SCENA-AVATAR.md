# Affinamento scena e avatar — v0.46

La **v0.45 `4afcf0b` resta la base estetica approvata**. La v0.46 ne conserva
layout, colori di navigazione, numeri pixel, pannelli e comandi e affina il
volto del dispositivo: Eroe e Aspetto.

## Cambiamenti visibili

- Paesaggio con creste meno regolari, acquedotto spezzato e fasce discontinue
  di foschia; distanza fredda e primo piano più leggibile.
- Portale con pietre scheggiate, giunti, edera e bordo irregolare; lastricato
  in prospettiva con ombra di contatto e riflessi delle lanterne.
- Alberi con rami e gruppi di foglie, più organici delle precedenti sagome.
- Lanterne con gabbia e **aloni trasparenti** dal decadimento disegnato su griglia.
  La loro intensità varia leggermente insieme alle scintille, anche in Aspetto.
- Avatar ridisegnato: volto arrotondato, sopracciglia/naso/mandibola, spalle e
  mani meno rigide, mantello sagomato e pieghe asimmetriche. I piedi restano
  piantati, mentre spalle e tessuto cambiano leggermente nelle pose.

Si mantengono tre carnagioni, tre capelli, tre mantelli e tre tagli: **81 aspetti**
con gli stessi indici. Nessuna classe, arma o statistica è dedotta dal disegno.
Il PC usa le stesse nuove maschere per l'anteprima di esportazione.

## Rendering e stato

Scene e maschere restano I4 a scale intere: 300×96 per il diorama, avatar
48×60 / 72×90 / 96×120, buffer statico **5824 byte** invariato.
Il nuovo alone condiviso è un asset I4 **40×40, 864 byte** palette inclusa,
con 16 livelli di alpha. Due oggetti immagine lo riusano; non viene creato
un filtro di sfocatura o un nuovo canvas applicativo.

Aloni e scintille condividono il timer cosmetico dell'avatar da 200 ms.
Disattivare Animazione mantiene il disegno statico; l'attenuazione sospende gli
aggiornamenti. La navigazione elimina il timer e pulisce i riferimenti agli
oggetti. Questo percorso non usa RNG, risorse o scritture NVS.

Il firmware collega **127682 byte** di immagini per cornice, scena, icone,
oggetti e alone, palette incluse, oltre a maschere/font/descrittori già descritti
nel [restyling v0.45](RESTYLING-PIXEL-HD.md). Decoder e oggetti LVGL richiedono
comunque heap: non si tratta di una misura della memoria libera sull'ESP32.

PDD23, USB8, backup PC v11 e partizioni invariati. PWR breve, BOOT breve e BOOT
lungo mantengono le associazioni configurabili; PWR lungo resta dell'alimentazione.
Raccolta personale e dati del personaggio non vengono modificati; nessuna lettura
Beyond. Le regole e i limiti funzionali delle versioni precedenti restano validi.

## Confronto riproducibile

```sh
python3 scripts/generate-device-art.py --check
npm run firmware:test
python3 scripts/preview-device.py
python3 scripts/compare-device-style.py /percorso/PocketD-D-v0.45.0-4afcf0b.zip
```

La comparazione richiede Pillow 10.1 o successivo. Verifica versione e commit
dell'archivio di riferimento, poi legge solo metadati e le immagini Eroe/Aspetto.
A sinistra sono le catture LVGL host della v0.45 approvata; a destra le catture
del firmware attuale, con lo stesso personaggio inventato. Nessun mockup.

L'archivio include `firmware-host-preview/pixel-polish-comparison.png`,
`device-preview.png`, `avatar-preview.gif` e le altre gallerie rigenerate.
L'archivio v0.45 originale viene conservato e non riscritto.

## Verifiche e lavoro successivo

- Suite host completa e 41411 confronti sintetici PC → USB → C.
- Avatar: 81 aspetti × quattro pose, palette, unicità, trasparenza, guardie,
  replica esatta 2×/3×/4× e input invalidi.
- UI: animazione senza RNG/salvataggi, aloni/luci in pausa nell'attenuazione,
  pulizia alla navigazione, animazione disattivata, valori/nomi massimi,
  conferme, import, errori e tasti; controlli nel cerchio e alti almeno 44 px.
- 144 test JavaScript e build PC/HTML autonomo; ASan/UBSan avatar/UI con leak
  detector disattivato; build ESP32-S3 riusciti.
- Firmware **988800 byte**, partizione app 4 MiB **76% libera**, ESP-IDF 5.5.5,
  LVGL 9.4.0. Nessun flash, collaudo fisico o misura di fluidità/consumi.

Il prossimo affinamento estetico riguarda pannelli più fantasy, icone specifiche,
transizioni e feedback per cure/danni/recuperi. Restano nel piano le illustrazioni
per altre sezioni e il collaudo sul display reale. Non si considera concluso
il blocco estetico né quello dell'alimentazione.
