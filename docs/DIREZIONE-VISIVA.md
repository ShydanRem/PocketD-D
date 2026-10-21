# Direzione visiva del dispositivo

PocketD-D deve essere bello da tenere sul tavolo e da guardare tra un turno e
l'altro: un piccolo RPG fantasy contemporaneo sul display AMOLED circolare.
La qualità visiva è un requisito del prodotto insieme alle funzioni di scheda.
Il PC rimane lo strumento per importazione, configurazione e backup.

## Nuovo riferimento — compagno fantasy

La direzione riparte da un avatar intero che abita nella home, come un piccolo
Tamagotchi fantasy: pixel art con profondità e luce, grandi icone e feedback
legato al personaggio. La revisione estetica v0.47 non è piaciuta e non è la base
da ampliare. La [prima prova nativa](COMPAGNO-PIXEL.md) esegue home, menu e PF
su LVGL a 466×466, separatamente dal codice di produzione. Un solo avatar di
prova; personalizzazioni, pose e collegamento completo ancora da sviluppare.

## Revisione precedente, ancora nel firmware v0.47

La v0.47 porta toni da grimorio e dettagli bronzo ai pannelli, icone nelle
liste e feedback dei saldi applicati. [Uso, anteprime e verifiche](PANNELLI-FEEDBACK.md).

La v0.45 è la base estetica approvata. La v0.46 ne affina scena e avatar, con
aloni delle lanterne anche in Aspetto. [Confronto e verifiche](AFFINAMENTO-SCENA-AVATAR.md).

La v0.45 avvia una revisione più videogiocosa, ispirata a un RPG pixel art 2D
con profondità e luce. [Schermate, contratti, verifiche e lavoro rimasto](RESTYLING-PIXEL-HD.md).
I paragrafi v0.18–44 sotto descrivono le versioni precedenti. La revisione
attuale ingrandisce la scena a 300×96 e l'avatar a 72×90, aggiunge cifre pixel,
cornice illustrata, pannelli con rilievo, oggetti di sezione e luci ambientali.

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
superficie `#252536`, testo `#f0eee5`, menta `#76dfc5`, oro `#f2cc81`,
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

La v0.23 aggiunge riposi e scelta delle riserve dadi vita con lo stesso tema:
valori recuperati in menta/oro, anteprima compatta prima della conferma e
pulsanti di almeno 44 px. Le cure riusano i dadi pixel art, mostrando faccia,
cura calcolata e PF effettivi. `rest-preview.png` raccoglie quattro catture del
firmware LVGL su host. Nessun nuovo asset esterno o mockup sostitutivo del codice.

La v0.24 aggiunge Stato, liste di condizioni e registro morte con la stessa
palette. Riuscite/fallite e stabilizzazione sono leggibili prima del tiro;
il d20 morte riusa l'animazione pixel art e conserva la distinzione fra faccia
e modifica salvata. Nomi delle note su più righe nel dettaglio, pulsanti di
almeno 44 px e tasti fisici protetti nelle conferme. `status-preview.png` viene
dal firmware LVGL su host; non è un mockup o una prova del pannello fisico.

La v0.25 aggiunge Schermo mantenendo font, palette e aree touch. La luminosità
agisce sul pannello tramite driver; le anteprime host mostrano la UI, non il
risultato elettrico della regolazione. screen-configured.png e screen-error.png
provengono dai test LVGL. In attenuazione l'avatar smette di ridisegnarsi; un
risveglio non è un'azione di gioco.


La v0.26 aggiunge elenco/editor/anteprima degli effetti locali nello stesso
cerchio. Palette e font distinguono basi, valori effettivi e perdita di PF;
le bozze non modificano la scheda. La conferma espone CA, PF massimi e correnti
prima/dopo, senza affidare una perdita di PF a una semplice pressione del tasto.
Righe numerate e comandi di almeno 44 px; nessun nuovo asset esterno. Anteprima
host: firmware/tests/out/ui-build/effects-preview.png; leggibilità fisica aperta.


La v0.27 mantiene il tema nelle durate: una vista separata evita di comprimere
l'editor dei bonus. Elenco con round/pausa/scaduto, anteprima con indici degli
effetti che avanzano e scadono, valori prima/dopo e perdita PF distinta. La
scorciatoia Round apre la stessa vista senza confermarla. Aree ≥44 px nel cerchio,
nessun asset esterno; anteprima host durations-preview.png. Il collaudo fisico
rimane aperto.


La v0.28 aggiunge modulo Danni, difese per tipo e riepilogo. L'anteprima separa
importo, temporanei e PF; distingue profili mancanti, esito concentrazione da
risolvere e casi bloccati. Il risultato resta leggibile dopo la conferma; Nuovo
richiede una nuova anteprima. Controllate anche le sovrapposizioni fra pulsanti,
con almeno 44 px nel cerchio; nessun nuovo asset esterno. Anteprima host:
firmware/tests/out/ui-build/damage-overview.png, non una foto dell'hardware.


La v0.29 aggiunge conferma Concentrazione, breakdown COS/competenza/extra,
selettore CD e modalità. Lo stesso dado pixel art si anima soltanto dopo il
salvataggio; il risultato indica concentrazione mantenuta/persa senza pulsante
di rilancio. Un errore mostra tiro fissato e comandi disabilitati. Font/contrasto
e pulsanti ≥44 px nel cerchio verificati sul renderer host; anteprima
concentration-preview.png. Nessuna nuova risorsa esterna o prova fisica.


La v0.30 aggiunge Durata → Vincolo, con nome della magia attuale e scelta
esplicita; C nelle righe e conc. sì/no nella conferma. Fine concentrazione
mostra CA/PF massimi/correnti prima e dopo e perdita oltre il nuovo massimo.
Sostituzione, riposo e danni spiegano la fine degli effetti. Nessun nuovo asset;
la vista conserva tema, centro leggibile e pulsanti ≥44 px nel cerchio.
Preview `links-preview.png` e sette schermate sono render reali LVGL su host,
non fotografie hardware. [Uso](EFFETTI-CONCENTRAZIONE.md).


La v0.31 distingue Base e A tempo nelle difese: proprietà effettive e contributi
base/extra nell'anteprima, durata e concentrazione in viste dedicate. +T segnala
un extra attivo; il round comune mostra effetti/difese e apre il dettaglio delle
sole difese con timer in avanzamento, tre per pagina. La navigazione conserva
conferma touch e pulsanti ≥44 px nel cerchio. `temporary-defences-preview.png`
e le viste allegate sono render LVGL su host, non catture hardware. Nessun nuovo
asset esterno. [Uso](DIFESE-TEMPORANEE.md).


La v0.32 aggiunge Componenti ed editor del singolo contributo al colpo, tre
righe per pagina. Tipi espone i gruppi ordinati e le difese applicate prima/dopo
la conferma; il totale rimane al centro con PF/temporanei e risultato della
concentrazione. Il pulsante Tipi resta visibile senza comprimere i tre comandi
inferiori della verifica COS. `mixed-damage-preview.png` e i dettagli sono
render reali LVGL su host; nessuna cattura hardware o asset esterno.
[Uso](DANNI-MISTI.md).


La v0.33 aggiunge Regole nei profili base/extra, proprietà separate e anteprima
del requisito completo prima/dopo. Fonte usa quattro scelte sì/no/? per la
componente; i dati non risolti non sono presentati come no. Tipi distingue
profili risolti di uno stesso tipo; il risultato mantiene i valori all'impatto.
Le descrizioni lunghe vanno a capo, senza comprimere i pulsanti ≥44 px nel cerchio.
`conditional-defences-preview.png` e le viste allegate sono render LVGL su host,
non fotografie hardware. [Uso](DIFESE-CONDIZIONALI.md).


La v0.34 mantiene numeri stabili per i tre extra e permette di consultare
proprietà, attivazione, timer e vincolo prima/dopo. I round distinguono ogni
extra, senza nascondere la difesa complessiva ancora disponibile. Anteprima
host: `firmware/tests/out/ui-build/multiple-defences-preview.png`; nessuna
verifica di leggibilità fisica è stata dedotta dalle catture.


La v0.35 rende visibile l'ordine delle sottrazioni: selettore con parte/tipo o
intero colpo, quantità scelta ed eccedenza ignorata. Tipi → Conto mostra cinque
passaggi fino al danno effettivo, preservati nel risultato. Anteprima host:
`firmware/tests/out/ui-build/reductions-preview.png`; leggibilità fisica ancora
da verificare sulla scheda.


La v0.36 distingue configurazione salvata e profilo scelto nella bozza del colpo.
Budget e somma assegnata sono visibili; i requisiti completi si confrontano prima
della conferma. Conto → Profilo conserva il dettaglio all'impatto. Anteprima
host: `firmware/tests/out/ui-build/reduction-profiles-preview.png`; la cattura
non prova la leggibilità sul display fisico.


La v0.37 aggiunge Costi alla configurazione e al dettaglio del profilo all'impatto:
fonte leggibile, nome completo, saldo prima/dopo, costo e reazione. Il confronto
usa una quarta pagina senza comprimere i tre confronti precedenti. Stato offre
Reazione e una conferma Inizio turno distinta dal round. Gli avvisi spiegano
spesa a danno zero e mancato recupero al round. Anteprima host:
`firmware/tests/out/ui-build/profile-costs-preview.png`; nessuna cattura fisica.


La v0.38 usa la pagina Costi esistente per fonti Slot/Patto con livello visibile
nel selettore e nome completo sotto. Il dettaglio conserva pool/livello e saldi
all'impatto; pool esauriti dichiarati senza sostituzioni implicite. Confronto e
riposi mostrano costi/recuperi distinti. Anteprima host:
`firmware/tests/out/ui-build/slot-costs-preview.png`; collaudo fisico ancora aperto.


La v0.39 distingue Inizio/Fine turno nel selettore Durata, con conteggio dei
passaggi e pausa. Il menu Reazione contiene entrambi i comandi; il riepilogo
mostra scadenze, reazione e CA/PF prima/dopo, con evidenza dei PF eventualmente
persi. Scadenze elenca fino a 50 voci su 17 pagine, senza comprimere le righe.
Anteprima host: `firmware/tests/out/ui-build/turn-durations-preview.png`.


La v0.40 aggiunge Tempo nella testata Stato, conservando Reazione. Scelta
leggibile dell'intervallo h/min, anteprima di scadenze e CA/PF, dettaglio con
residui in minuti; negli editor Ore i tasti distinguono ore e minuti. Il confronto
conserva le unità e l'avviso evidenzia i PF persi. Anteprima host:
`firmware/tests/out/ui-build/elapsed-time-preview.png`; nessuna cattura fisica.


## Cariche separate — v0.41

Il dettaglio oggetto conserva il nome completo su più righe e aggiunge Cariche
accanto a Zaino solo quando il pool è configurato. Pagina dedicata con saldo,
massimo, recupero e bozza; conferma prima/dopo. Quantità zero consultabile con
azioni bloccate. Costi mostrano nome e saldi originali; i riposi separano unità
di carica recuperate dalle voci con utilizzi. Nessun nuovo gesto o assegnazione
obbligatoria ai tasti. Anteprima `item-charges-preview.png` e `charges-*.png`
nell'archivio, render reali LVGL host con dati sintetici, non fotografie hardware.


## Tre spese nello stesso spazio — v0.42

Il pulsante Spesa 1/3–3/3 sostituisce la testata dei costi e sfoglia le fonti
senza aggiungere righe o gesti nascosti. La conferma confronta prima/dopo per
la spesa selezionata; il resoconto mostra nome, tipo/indice e saldi originali.
Un blocco conserva tutti i saldi, consultabili per trovare la fonte mancante.
Nome lungo su più righe, controlli nel cerchio e tasti configurabili protetti.
`multiple-costs-preview.png` e `multi-cost-*.png` nell'archivio: render LVGL host
sintetici, nessuna nuova illustrazione o fotografia hardware.


## Ricarica con dadi — v0.43

La pagina Cariche conserva nome, saldo e correzioni; il footer a tre pulsanti
aggiunge Ricarica. Formula e bonus su righe separate, anteprima senza tiro e
conferma esplicita. Stesso renderer poliedrico pixel art delle cure; risultato
marcato Cariche, con saldo, recupero ed eccesso nel riepilogo. Stato pieno o
oggetto assente disabilitano la ricarica; retry conserva il candidato.
Anteprime `item-recharge-preview.png` e `recharge-*.png` nell'archivio, render
LVGL host sintetici, non fotografie hardware. Tasti configurabili conservati.


## Effetti sui tiri e circostanze — v0.44

Tiri nel footer delle note apre quattro interruttori locali, con confronto
prima/dopo. Contesto affianca i dettagli del tiro/arma senza aumentare il numero
di righe; pagina dedicata per vista, fonte di paura e scelta DM. Il punto
interrogativo indica dati sconosciuti, i controlli irrilevanti sono disabilitati.
Fallimento automatico e contesto mancante bloccano Lancia con messaggio visibile;
nessun dado o esito naturale fittizio. `roll-conditions-preview.png` e singoli
`roll-conditions-*.png` sono render LVGL host, non fotografie del dispositivo.
La revisione estetica organica rimane il successivo blocco software concordato.


## Revisione RPG 2D — v0.45

Distanza fredda, lanterne calde, foschia disegnata su griglia e ombre del
lastricato danno profondità al diorama. Cifre originali per PF/contatori,
Montserrat per nomi e descrizioni, pannelli con gradiente e bordi netti,
barre segmentate, oro per selezione e 20 naturale. Luci e scintille condividono
il timer dell'avatar, senza RNG o salvataggi. PWR/BOOT configurabili conservati.
La galleria `device-preview.png` e tutte le altre anteprime vengono dal codice
LVGL host. [Dettagli e limiti](RESTYLING-PIXEL-HD.md); il blocco estetico resta aperto.


## Affinamento scena e avatar — v0.46

Pietra irregolare e con edera, fogliame a gruppi e rami, foschia discontinua,
lastricato in prospettiva e aloni trasparenti condivisi dalle due lanterne.
Avatar meno squadrato con volto, spalle, mani e mantello ridisegnati; pose di
respiro e tessuto, stesse 81 personalizzazioni. Layout e carattere generale
della v0.45 conservati. `pixel-polish-comparison.png` mette a confronto le
catture firmware host prima/dopo; il timer rispetta animazione e attenuazione.
[Contratti e limiti](AFFINAMENTO-SCENA-AVATAR.md).


## Pannelli e feedback — v0.47

Superfici indaco/calde, bordi bronzo e angoli pixel nei pulsanti larghi; icone
in base al tipo esplicito di oggetto o magia/capacità. Nomi completi nel
dettaglio. Il messaggio sotto il marchio usa icona, testo e colore per i saldi
applicati, con ornamenti brevi e modalità statica. Layout della scena e tasti
configurabili conservati. `fantasy-feedback-preview.png` e `feedback-preview.gif`
sono catture LVGL host, non riprese del dispositivo. [Dettagli](PANNELLI-FEEDBACK.md).
