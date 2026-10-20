# Direzione visiva del dispositivo — v0.22

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
- Eroe con scena notturna originale, avatar personalizzabile, luna, pini e rovina;
  livello/CA separati, PF grandi, PF temporanei, barra proporzionale e controlli.
- Navigazione con quattro icone pixel art e selezione evidenziata.
- Caratteristiche, contatori, preparazione dei dadi, scelta del tipo e conferma
  USB con lo stesso linguaggio grafico. Feedback premuto e disabilitato distinto.
- Nomi lunghi limitati a una riga con ellissi: non invadono i valori sottostanti.
- Nella v0.20, abilità a tre righe per pagina, salvezze su griglia e vista
  dedicata alla composizione dei bonus. Il punto interrogativo e il testo
  Da verificare distinguono i valori incompleti anche senza affidarsi al colore.

L'avventuriero è un **avatar originale personalizzabile**: carnagione, colore dei
capelli, mantello e taglio hanno tre scelte ciascuno, per 81 combinazioni. Si
configura nell'editor PC e si trasferisce con la copia USB v2. Non rappresenta
ritratti di Beyond, classe o equipaggiamento. La raccolta PC rimane conservata.
Livello, CA, PF, caratteristiche e contatori visualizzati restano quelli dello
stato attivo; le illustrazioni decorative non aggiungono regole.

Toccare **Aspetto** nella scena Eroe apre l'avatar ingrandito e il comando
**Animazione: si/no**. La preferenza viene salvata sul dispositivo; un nuovo
trasferimento conserva la scelta locale. Il comando non sostituisce l'ultima
modifica PF/contatori annullabile.

Quattro pose muovono leggermente il mantello e chiudono gli occhi una volta in
un ciclo da 3,2 secondi. Un timer da 200 ms aggiorna solo la figura e soltanto
quando cambia posa. Si ferma nelle altre sezioni e nella conferma USB, che mostra
un'anteprima statica del nuovo avatar. L'animazione non usa RNG o scritture NVS.
Non vengono dichiarati risparmi energetici o prestazioni hardware misurati.

## Asset e riproducibilità

`scripts/generate-device-art.py` genera `firmware/assets/device_art.h`:
scena 150×36 disegnata su griglia, replicata a 300×72; quattro icone 12×12
replicate a 24×24. Formato LVGL I4, palette BGRA a 16 colori, costanti in flash.
**12.272 byte** complessivi di dati immagine, oltre ai descrittori. Nessun file
esterno, servizio online o nuova allocazione canvas applicativa per la scena.
Decoder e oggetti LVGL usano comunque heap: non è una misura di memoria a runtime.

Lo stesso generatore produce `firmware/assets/avatar_atlas.h` e
`src/device-avatar-masks.json`: tre tagli × quattro pose, disegnati a 24×30 e
replicati a 48×60; 17.280 byte di maschere I4 e 60 byte di colori base in flash.
La palette canonica è `src/avatar-palette.json`, condivisa dal vecchio avatar
SVG del PC e dal generatore firmware. L'anteprima di esportazione usa le stesse
maschere e gli stessi colori del dispositivo attraverso `src/device-avatar.js`.

Il buffer avatar statico è **5.824 byte**, palette inclusa: ospita la figura
48×60 oppure l'ingrandimento 96×120 prodotto per replica esatta dei pixel.
Non si trasforma a runtime un'immagine indicizzata con lo scaler LVGL. Questo
evita una lettura oltre il buffer riscontrata dal sanitizer nella vista grande.
La cache immagine viene invalidata prima di aggiornare palette o pixel e quando
si esce dalla vista. Non occorre un secondo canvas completo del display.

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
La v0.19 aggiunge `avatar-preview.gif`, `avatar-presets.png` e le viste Aspetto.

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

## Verifiche v0.19

126 test JavaScript, core C/NVS e test LVGL nativi passati. Il renderer verifica
81 aspetti × quattro pose, palette trasparenti, guardie del buffer e ingrandimento
esatto. UI verificata anche con ASan/UBSan dopo la correzione della lettura oltre
il buffer: timer, abilitazione/disabilitazione, annulla PF, errore memoria,
anteprima/conferma/annulla importazione e preferenza locale conservata.
Migrazione da un record v1 prodotto dall'encoder effettivo della v0.18, senza
riscrittura all'avvio; prima scrittura v2 conserva la vecchia banca. L'uploader
non invia una copia v2 a un firmware che dichiara solo v1. La prova browser usa
un profilo effimero e un avatar sintetico, verificando anteprima, download e
raccolta invariata. Nessun personaggio personale Beyond letto o modificato.

## Da sviluppare con questa direzione

1. Ulteriori silhouette e personalizzazioni, mantenendo esplicite le scelte del
   giocatore; eventuale aspetto dell'equipaggiamento dopo il porting dell'inventario.
2. Inventario e attacchi introdotti nella v0.21 con liste paginate e dettaglio
   dei bonus; estendere il linguaggio a magie e testi lunghi. Icone specifiche e pochi
   tocchi per le azioni frequenti, evitando di stipare tutto nella schermata Eroe.
3. Illustrazioni coordinate per nuove sezioni, stati vuoti e feedback delle azioni.
4. Transizioni leggere e preferenza per ridurre le animazioni. Le animazioni
   ambientali restano subordinate alla gestione energetica da implementare.

Questa versione aggiunge personalizzazione e animazione alla base visiva; non
completa il trasferimento della scheda estesa, il disegno pixel per pixel
dell'avatar o le funzioni di alimentazione.

La v0.21 aggiunge tre oggetti per pagina, quantità e arma scelta, dettaglio
leggibile e vassoio compatto fino a 18 dadi. `inventory-preview.png` è una
cattura LVGL host, non una foto dell'AMOLED. I nomi completi restano nel dettaglio;
la lista li abbrevia per conservare quantità e aree touch.

La v0.22 estende lo stile a filtri di magie/capacità, costi, testi paginati
e impostazioni tasti. I testi in estratto sono dichiarati, senza rimpicciolire
il font per far entrare intere descrizioni. Anteprima `magic-preview.png`,
sempre dal renderer LVGL host e non da una foto del dispositivo.
