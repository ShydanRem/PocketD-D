# Avatar illustrato del compagno — v0.49

La home del dispositivo usa ora l'avatar illustrato della nuova direzione
grafica, adattato alle **81 combinazioni già salvate**. Tre carnagioni, tre colori
di capelli, tre mantelli e i tagli corto/lungo/rasato mantengono gli indici
precedenti: non serve migrare o importare nuovamente il personaggio.

Per ogni taglio ci sono quattro pose disegnate: neutra, battito delle palpebre,
cura e danno. Il personaggio ha volto, silhouette, mantello e dettagli più ricchi
del precedente sprite procedurale. La scena, le icone e i percorsi della v0.48
rimangono la base dell'interfaccia. Nessuna arma, classe o regola deriva dal
disegno: il costume resta decorativo.

## Comportamento nel firmware

- A riposo il personaggio mantiene il movimento lieve della home e chiude gli
  occhi per 200 ms ogni 3,2 secondi. Il fotogramma chiuso comprende una piccola
  variazione del mantello; non è un ciclo di camminata o una simulazione fisica.
- Una cura **salvata** mostra la posa felice per un secondo; un danno salvato
  mostra la posa contratta per 600 ms, con aggiornamento cosmetico ogni 200 ms.
  Contano i delta effettivi dei PF o, in
  loro assenza, dei temporanei. La ricevuta numerica conserva i due secondi
  della v0.48. Non vengono alterati i tempi o i risultati dei dadi.
- Una ridisegnatura non prolunga la reazione. Annulla usa i delta effettivamente
  ripristinati. Un'operazione senza cambiamento o non salvata non produce una
  reazione di successo. Spese/recuperi di altre risorse non fingono una cura.
- Animazione disattivata conserva posa neutra e ricevuta statica. Attenuare lo
  schermo ripristina la posa neutra e cancella il feedback; il risveglio non
  ripete la reazione. Navigazione e conferme eliminano i timer della vista.
- PWR/BOOT mantengono tutte le associazioni esistenti. Le reazioni non fanno
  scritture NVS e non consumano numeri casuali.

L'anteprima della **copia USB sul PC** usa gli stessi pixel e le stesse palette
del firmware. Il vecchio avatar SVG delle altre schermate PC rimane separato:
il PC continua a essere lo strumento di preparazione. Il dialogo segnala che
questa grafica richiede v0.49; firmware precedenti leggono gli stessi indici
con il loro disegno precedente.

## Asset, memoria e riproduzione

Il foglio originale generato con OpenAI ImageGen è conservato senza modifiche
in `firmware/assets/avatar-source/poses.png`. `provenance.json` registra hash
del riferimento e del risultato, riepiloghi delle richieste e metodo di export.
Il riferimento è `firmware/ui/companion/source/hero.png`; la generazione segue
una griglia unica di dodici pose, poi corregge il taglio rasato.

`scripts/generate-avatar-art.py` estrae le righe sulle effettive separazioni
trasparenti: dividere semplicemente in tre parti uguali taglierebbe gli stivali.
Tutte le pose condividono scala **58/357**, griglia **48×60** e punto d'appoggio
degli stivali **(27,59)**. Il generatore assegna sedici gradazioni ciascuno a
pelle, capelli e mantello; cuoio, lino, oro, occhi e contorni restano separati.
La palette conserva i colori configurabili precedenti, con ombre e luci nuove.

Le dodici maschere I8 occupano **34560 byte**. Il buffer avatar statico passa da
5824 a **12544 byte** (+6720): include la palette BGRA da 1024 byte e l'immagine
96×120 della home. La vista Aspetto usa 72×90, preparata con campionamento al
vicino più prossimo; la home usa replica esatta 2×. Non c'è trasformazione di
immagini indicizzate affidata a LVGL, né un nuovo canvas applicativo. La memoria
del decoder/cache LVGL rimane distinta: questi valori non misurano l'heap libero.

```sh
# Verifica sorgente, maschere e palette C/JS versionate, senza ImageMagick:
python3 scripts/generate-avatar-art.py --check
# Solo per riesportare il foglio, richiede ImageMagick 7:
python3 scripts/generate-avatar-art.py --export
npm test
npm run build
npm run firmware:test
python3 scripts/preview-device.py
```

Firmware e app PC compilano dai dati versionati e non richiedono ImageMagick.
Il vecchio generatore `generate-device-art.py` continua a gestire scene, icone
procedurali e font delle altre viste; non riscrive più l'avatar.

## Verifiche effettuate

- **147 test JavaScript**; unicità visiva di tutti gli aspetti/pose, indipendenza
  dei tre materiali, input invalidi e build PC/HTML autonomo.
- Avatar C: guardie dei buffer, bordi trasparenti, palette, unicità, campionamento
  nelle tre dimensioni, input invalidi e scadenze delle pose.
- **324 confronti esatti C–JS** dell'intero buffer I8: palette, trasparenza e
  pixel per ogni combinazione. Non sono letture del personaggio Beyond.
- Suite C/NVS/UI completa e **41411 confronti sintetici PC → USB → C** già
  previsti per le funzioni della scheda. Le nuove catture attraversano vere
  azioni PF salvate nel banco di prova, compreso il cambio posa dopo ridisegno.
- Test UI di errori, temporanei, animazione disattivata, attenuazione/risveglio,
  scadenza al wrap del timer, importazione, tasti configurabili e dati massimi.
- Avatar e UI con **ASan/UBSan**, leak detector disattivato.
- Browser Chromium separato: anteprima copia USB, download sintetico e raccolta
  del test invariata; nessun errore JavaScript.
- Firmware ESP-IDF 5.5.5 / LVGL 9.4.0 compilato: **1624672 byte**, partizione app
  da 4 MiB **61% libera**. Nessun flash o collaudo sul dispositivo fisico.

Le anteprime `avatar-illustrated-preview.png` (tre aspetti × quattro pose),
`avatar-reactions.gif`, `avatar-presets.png` e `home-integration-preview.png`
provengono da `firmware/main/main.c` eseguito con LVGL su host e personaggi
sintetici. La GIF delle reazioni monta catture per illustrare i cambi di posa;
non misura il frame rate del dispositivo. `avatar-preview.gif` e
`feedback-preview.gif` conservano anche le sequenze temporali del banco UI.

PDD23, USB8, backup PC v11 e partizioni restano invariati. Nessun accesso Beyond
o modifica alla raccolta personale. Restano il restyling delle viste interne,
transizioni e animazioni più articolate, oltre ai collaudi hardware e ai punti
funzionali aperti della [roadmap](PROGETTO.md).
