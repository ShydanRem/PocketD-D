# Avatar per specie e silhouette — v0.67

> Aggiornamento v0.69: [21 famiglie e 42 silhouette](AVATAR-SELEZIONE-ESTESA.md),
> PDD32/PDDW; USB12 e backup PC v14 per gli ID nuovi.
> [Preset di classe](PRESET-CLASSI.md): studio visivo, integrazione successiva.

> Aggiornamento v0.68: [quattro nuove famiglie e compressione senza perdita](AVATAR-SPECIE-ANIMALI.md).
> Catalogo a 16 specie/32 silhouette, PDD31/PDDV; USB11 e backup PC v13
> per i nuovi ID. Formati precedenti conservati; collaudo fisico ancora aperto.

Lo gnomo della tavola è stato ridisegnato con naso piccolo e proporzioni più
armoniose. Le altre famiglie approvate sono state usate come riferimento per
sprite trasparenti effettivi, integrati nel PC e nel firmware.

Sono disponibili **12 famiglie visive**: Umano, Elfo, Nano, Halfling, Gnomo,
Mezzelfo, Orco, Mezzorco, Tiefling, Dragonide, Goliath e Aasimar. Ognuna ha una
silhouette **maschile e femminile**, indipendenti da carnagione, capelli e mantello.
Le differenze fra i corpi sono volutamente sobrie e variano fra specie: non sono
24 costumi diversi. Non viene dedotto il genere da nome, classe o specie.

## Uso

Sul dispositivo: **avatar → Aspetto → Modifica**. Le frecce scorrono sei campi:
carnagione, capelli, mantello, taglio, specie e silhouette. Nel campo Specie,
**Altre** cambia il gruppo di tre alternative. **Anteprima → Conferma** salva
insieme aspetto e scena; Annulla conserva la scheda precedente. I tasti fisici
continuano a permettere selezione, attivazione e ritorno, senza cambiare le
assegnazioni personali. Per il Dragonide il colore dei capelli è escluso e i
tagli diventano cresta, corna e testa liscia.

Sul PC: **Personalizza eroe** offre gli stessi indici visivi e mostra gli sprite
del firmware per i nuovi avatar. I tre stili di colore non sovrascrivono specie
e silhouette. Ripristina aspetto torna alla bozza originale; il salvataggio
conserva inventario, dati estesi e cariche nel formato corrente.

Durante l'importazione da Beyond la specie viene confrontata con un elenco
esplicito di nomi inglesi/italiani, comprese alcune sottospecie. Sono corrispondenze
di **famiglia visiva**, non illustrazioni esatte per ogni sottospecie né una
validazione degli ID ufficiali. Il genere esplicito riconosciuto propone la
silhouette; rimane modificabile. Se manca o non è riconosciuto, l'anteprima
segnala che la silhouette maschile è provvisoria. Specie sconosciute, homebrew o
ambigue richiedono una scelta manuale: il ripiego umano è dichiarato.

L'anteprima consente di cambiare entrambe le scelte prima della conferma che
crea una nuova scheda. Le schede già presenti non vengono aggiornate da questa
proposta. Aspetto, colori e capelli non applicano bonus, regole di specie,
equipaggiamento o immagini personali di Beyond.

## Asset e risorse

Le sorgenti ImageGen originali sono conservate in
`firmware/assets/species-source/`, con hash e provenienza. Il generatore campiona
sulla griglia 48×60 senza ridisegnare gli sprite, mantiene le altezze relative e
condivide maschere e palette fra C e JavaScript. L'umano maschile conserva
esattamente l'atlante precedente. Le nuove 276 immagini coprono due corpi, tre
tagli e quattro stati; con le 12 immagini originali sono 288 frame. Le espressioni
sono più o meno marcate a seconda del disegno; il feedback di cura/danno del
runtime resta distinto.

I dati I8 usano PackBits e, quando conviene, differenze XOR rispetto alla posa
neutra: 396.617 byte di immagini compresse. La decodifica usa il buffer già
esistente, senza allocazioni per fotogramma. Le sorgenti ad alta risoluzione e i
metadati di normalizzazione non vengono caricati nella RAM del dispositivo.

La build ESP32-S3 è di 3.956.352 byte nella partizione applicazione da 4 MiB:
rimangono 237.952 byte, circa il 6%. Non sono cambiate le partizioni NVS. Prima
di espandere molto il catalogo servirà ridurre il peso degli asset o prevedere
pacchetti grafici; non basta aggiungere nomi alla lista.

## Formati e compatibilità

| Percorso | Comportamento |
| --- | --- |
| Backup PC | v12 per specie/corpo diversi dall'umano maschile; v11 per l'aspetto precedente. Lettura dei formati precedenti conservata. |
| Copia PC → dispositivo | USB10 con sei campi avatar per le nuove silhouette; USB9 per il precedente umano maschile. |
| Firmware | Risponde READY 10, accetta USB1–10; rifiuta copie con indici fuori catalogo. Il PC blocca USB10 su firmware precedente prima di inviare la scheda. |
| Salvataggio dispositivo | PDD30/PDDU, 8.945 + 552 × capacità byte, massimo 75.185. Scena seguita da specie/corpo prima del CRC. |
| Migrazione | PDD29 mantiene la scena e assume umano maschile; PDD28 e precedenti conservano i dati e usano anche la scena predefinita. Nessuna migrazione scrive durante la lettura. |
| Backup nativo | Involucro v1 invariato; validatori C/PC/Python aggiornati a PDDU, con anteprima della silhouette. Vecchi strumenti non leggono il nuovo record: aggiornare anche il pacchetto PC. |

La copia USB importa l'avatar scelto sul PC e conserva scena, luminosità,
animazione e tasti locali. Il ripristino nativo recupera anche l'aspetto salvato.
Annullare una variazione dei PF non annulla una personalizzazione già salvata.

## Verifiche e limiti

- **200 test JavaScript passati**. Test PC su alias, sorgenti ambigue/homebrew, genere esplicito/assente, backup
  v12, USB10 e rifiuto su vecchio firmware; nessuna modifica degli stati sorgente.
- 7.776 confronti byte per byte fra renderer C e anteprima JS: 12 famiglie,
  due corpi, 81 combinazioni di palette/taglio e quattro stati. I 324 confronti
  del precedente avatar continuano a passare.
- Codec nativo: tutte le 72 combinazioni specie/corpo/scena, migrazioni PDD29 e
  PDD28, valori invalidi con CRC valido e conservazione della destinazione in
  caso di errore. Backup C → PC → C con tutte le identità visive: 39 casi di integrità e semantica.
- UI LVGL host: bozza, conferma, salvataggio fallito, annullamento, isolamento
  dall'undo PF, navigazione fisica, importazione e backup; catture delle 24 home. Suite nativa completa passata, 118/118 viste; UI, renderer e codec aspetto verificati anche con AddressSanitizer/UBSan.
- Browser con connettore sintetico: riconoscimento Gnomo/Femminile, modifica
  manuale prima della creazione, annullamento, salvataggio nell'editor,
  conservazione delle altre schede, ricaricamento e backup; larghezze 320/390/1440. Test del pacchetto PC estratto, installer e trasporto USB simulato passati.

Queste sono **prove sintetiche ed esecuzioni del firmware su PC**. In questa
modifica non è stata eseguita una nuova lettura reale di Beyond né una prova USB
sul dispositivo dell'utente. Il problema di riavvio/importazione USB osservato
in precedenza rimane aperto per il collaudo fisico.

## Estensione del catalogo

Il catalogo non copre tutte le specie di D&D. Ordine proposto dei prossimi gruppi:

1. **Tabaxi, Kenku, Aarakocra, Tortle**: musi, becchi, piume e guscio riconoscibili.
2. **Goblin, Hobgoblin, Bugbear, Kobold, Lizardfolk**: proporzioni e teste proprie.
3. **Genasi** delle quattro origini, **Firbolg, Tritone, Yuan-ti, Warforged**.
4. Specie ulteriori e aspetti specifici delle sottospecie, dopo la verifica della
   leggibilità, delle varianti e del budget del dispositivo.

Ogni famiglia necessita di disegni dedicati, entrambi i corpi, tagli applicabili,
reazioni, controllo visivo a dimensione reale e confronto C/JS. Alias provvisori
non vengono presentati come nuove razze disegnate.
