# Goblin, Hobgoblin, Bugbear, Kobold e Lizardfolk — v0.69

> Cataloghi futuri: [asset e preset sulla SD](ASSET-SD.md), con risorse essenziali
> in flash e cache limitata in RAM. La v0.69 usa ancora asset incorporati.

La selezione raggiunge **21 famiglie visive e 42 silhouette**. Il nuovo gruppo
aggiunge Goblin piccolo con grandi orecchie, Hobgoblin color terracotta con tratti
più robusti, Bugbear peloso, Kobold piccolo con corna e coda, e Lizardfolk con
muso allungato, cresta/collare e coda. Ogni famiglia ha due corpi, tre forme della
testa e quattro stati: **120 frame nuovi**, 504 compreso l'atlante umano originale.

Restano disponibili Umano, Elfo, Nano, Halfling, Gnomo, Mezzelfo, Orco, Mezzorco,
Tiefling, Dragonide, Goliath, Aasimar, Tabaxi, Kenku, Aarakocra e Tortle. I loro
pixel e colori usati sono conservati; il riferimento di regressione ora copre
anche il gruppo della v0.68.

## Uso

Sul dispositivo: avatar → Aspetto → Modifica → Specie. **Altre** scorre sette
gruppi di tre; l'ultimo mostra Bugbear, Kobold e Lizardfolk. Goblin e Hobgoblin
hanno capelli configurabili; Bugbear usa pelliccia, Kobold e Lizardfolk squame e
forme della testa. Silhouette, colori, mantello e scena rimangono indipendenti.
Anteprima, conferma, annullamento e tasti configurabili conservano la scheda.

Sul PC gli stessi nomi sono selezionabili. Beyond riconosce le corrispondenze
esplicite, compresi Coboldo/Lucertoloide e i nomi inglesi. Genere assente, homebrew,
nomi ambigui e specie non ancora disegnate restano dichiarati e modificabili.
Il riconoscimento dell'aspetto non evita i blocchi della conversione meccanica.

La raccolta personale non viene modificata durante la lettura o la scelta
nell'anteprima. Confermare crea una nuova scheda; la proposta non cambia CA,
PF, capacità, inventario o equipaggiamento delle altre schede.

## Memoria e compatibilità

I 492 frame aggiuntivi codificati in Huffman/PackBits/XOR occupano **583.525 byte**
(696.828 prima della codifica Huffman). Si riusa il buffer del renderer, senza
allocazioni per fotogramma.

I cinque sfondi RGB565 (accampamento e quattro stanze) erano già formati da
blocchi di pixel 2×2 identici. Ora conservano una sola copia di ciascun blocco:
233×233 pixel al posto di 466×466. L'esportatore verifica che ricostruire i quattro
pixel produca **gli stessi byte RGB565 originali**, senza quantizzare o ridisegnare.
Il risparmio complessivo verificato è **1.628.670 byte di flash**.

Il renderer espande lo sfondo nella cache prima del disegno, conservando dimensioni
e stride nativi 466×466. Non introduce trasformazioni dell'immagine durante il
disegno né allocazioni per fotogramma. Due slot in PSRAM occupano **868.624 byte**;
la cache I8 delle scene e le miniature restano separate. L'API richiede al massimo
due sfondi espansi vivi su una schermata; il prototipo continua a usare il suo
accampamento tramite la stessa cache. Nessuno sfondo è stato rimosso.

Il confronto comprende **160 catture identiche pixel per pixel** prima/dopo il
cambio di memorizzazione: 118 viste e 42 home. Anche le 32 home della v0.68 sono
rimaste identiche prima di questa ottimizzazione. Il test nativo controlla copia,
cache hit, convivenza dei due sfondi, espulsione e input invalidi.

| Percorso | Nuova versione |
| --- | --- |
| Queste cinque famiglie nel backup PC | v14 |
| Copia al dispositivo | USB12, firmware 0.69 o successivo |
| Salvataggio nativo | PDD32/PDDW, dimensioni invariate |
| Backup nativo | Involucro v1; lettori C/JS/Python aggiornati |

I precedenti backup PC v11/v12/v13, USB9/10/11 e PDD30/PDD31 conservano i rispettivi
limiti. Il nuovo gruppo non viene inserito con un ID aggiuntivo in un formato
vecchio. I file nativi mantengono 8.945 + 552 × capacità byte, massimo 75.185;
scene, luminosità, animazione e tasti locali restano conservati durante l'import.
La build ESP32-S3 occupa **2.526.256 byte**: restano **1.668.048 byte**, circa il 40% della partizione applicazione da 4 MiB. Non sono cambiate le partizioni NVS.

## Verifica e prossimo blocco

**205 test JavaScript passati**, 13.608 confronti di buffer C/JS, 126 combinazioni specie/corpo/scena e 57 casi di ripristino C/JS. Cache e UI sono verificate anche con AddressSanitizer/UBSan; il prototipo LVGL opzionale passa con gli stessi sfondi ricostruiti.

I controlli coprono tutte le identità, la conservazione delle sedici famiglie
precedenti, i buffer C/JS, le migrazioni, l'importazione simulata Goblin →
Lizardfolk con salvataggio v14/USB12, l'annullamento, i backup e le altre schede.
Le catture della galleria provengono dal renderer LVGL eseguito su PC. La build
ESP32-S3 verifica dimensioni e compilabilità, non le prestazioni fisiche.

In questa modifica **non è stata eseguita una nuova lettura reale di Beyond né
una prova USB sul dispositivo dell'utente**. Il problema di riavvio/importazione
fisica rimane aperto.

Con questa selezione il prossimo blocco è [quello dei preset di classe](PRESET-CLASSI.md),
con vestiti, armature e armi distinti. La tavola di stile pubblicata separatamente
è una proposta per quel lavoro: i preset non sono ancora presenti nel firmware.
