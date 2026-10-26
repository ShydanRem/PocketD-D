# Inventario e attacchi sul dispositivo — v0.21

> Aggiornamento v0.65: [quantità modificabili sul dispositivo](QUANTITA-OGGETTI.md),
> con tastiera, conferma e cariche conservate. I limiti seguenti sono storici.

> Aggiornamento v0.63: [ricerca locale per nome](RICERCA-LOCALE.md) nello zaino
> e nelle magie/capacità. Filtri di sessione, nessuna scrittura o modifica
> degli indici originali. Le descrizioni successive sono storiche.

> Aggiornamento v0.61: [uso pratico al tavolo](USO-AL-TAVOLO.md).
> Cure note e temporanei si gestiscono dai PF. Dopo un errore di scrittura,
> pozioni e dadi vita conservano il candidato/tiro con uscita bloccata e Riprova.
> Le descrizioni e i limiti successivi sono quelli storici della versione del titolo.

**Aggiornamento v0.41:** [cariche degli oggetti](CARICHE-OGGETTI.md) disponibili
come saldo distinto dalla quantità, con configurazione PC, gestione sul dispositivo,
costi e recuperi. Formati correnti: backup PC v11, USB8 e PDD21. Le verifiche
e le limitazioni storiche descritte sotto si riferiscono alla versione del titolo.

Il firmware funziona offline, senza Foundry o PC al tavolo. Il PC prepara la
copia e ne mostra inventario, quantità e arma scelta prima del download. Questa
versione non ripete la lettura del personaggio personale Beyond: verifiche e
anteprime usano soltanto dati sintetici.

## Uso

**Usi → Zaino** apre tre oggetti per pagina, fino a **60 voci**, comprese quelle
esaurite. Quantità originali conservate; nessun taglio dell'inventario. L'asterisco
indica l'arma scelta. Il dettaglio mostra il nome su più righe; **Scegli/Riponi**
persistono la scelta. **Attacco** apre bonus, CA bersaglio manuale 1–30 e modalità
normale/vantaggio/svantaggio. Toccare il bonus apre i contributi: caratteristica,
competenza, extra, base danni, extra danni ed effetti PC pertinenti.

**Lancia → risultato → Danni**: 1 naturale manca, 20 naturale colpisce ed è critico.
Il critico raddoppia i dadi dell'arma e aggiunge gli eventuali dadi critici in
mischia riconosciuti; il bonus si somma una sola volta. Fino a **18 dadi**, senza
scartare risultati: oltre sei si usa un vassoio a sei colonne, fino a tre righe.
Il totale dei danni non scende sotto zero. I danni usano la configurazione del
colpo e si possono tirare una sola volta per quel risultato; **Nuovo** prepara
un altro attacco. Animazione/accelerazione non estraggono nuovi valori. Nessun
danno viene applicato automaticamente a un bersaglio o ai PF del personaggio.
I tiri manuali in Dadi → Altri rimangono limitati a 1–6 dadi e bonus −15…+30.

**Usa → Conferma** consuma una unità. Una cura tira i dadi, limita i PF al massimo
e conserva i PF temporanei; a PF pieni non tira né consuma. PF e quantità vengono
scritti nello stesso record, prima di mostrarli come applicati. **Eroe → Annulla**
ripristina l'ultima modifica di sessione, comprese le due grandezze insieme.
Annullare prima della conferma non usa casualità né memoria. Un errore di
scrittura mantiene lo stato visibile precedente; come per ogni commit NVS,
un errore I/O può avere esito ambiguo e richiede riavvio/verifica del display.
Cambiare arma o consumare un oggetto invalida il colpo preparato e azzera la
modalità lancio, come nel modello PC. Nessun consumo automatico delle armi.

## Regole e limiti espliciti

- Attacco: modificatore della caratteristica scelta, competenza se prevista,
  modificatore dell'arma, −5 da attacco potente quando pertinente.
- Danni: dadi dell'arma, caratteristica se prevista, modificatore dell'arma,
  Ira solo per FOR in mischia, +10 da attacco potente se arma pesante competente
  in mischia. Dadi critici extra solo in mischia. La modalità lancio usa la
  gittata breve riconosciuta ed esclude i benefici riservati alla mischia.
- Avvelenato, Ira, bonus/dadi critici e attacco potente sono **istantanee PC**.
  Non gestiscono durata, utilizzi o recupero delle capacità. Vantaggio manuale
  annulla lo svantaggio da veleno; nessun vantaggio automatico da Ira agli attacchi.
- Il riconoscimento delle armi/cure avviene nel modello PC. Un oggetto generico
  rimane generico: leggere una nota come “arma +99” non gli assegna regole.
- **Non trasferiti:** note, proprietà descrittive, gittata lunga, identificativi
  originali. Il riepilogo PC lo dichiara; non presentare la copia come completa.
  Nessun catalogo, calcolo di peso/monete, munizioni automatiche, resistenze,
  immunità, danni elementali aggiuntivi, effetti speciali, attacchi di incantesimo
  o gestione automatica delle armature. Il tipo di danno è solo descrittivo.
- CA e avatar non cambiano scegliendo un'arma. Oggetti e quantità sono configurati
  sul PC; sul dispositivo questa versione permette consultazione, scelta e
  consumo, non aggiunta/modifica libera. Nessuna sincronizzazione verso il PC o
  scrittura verso Beyond. Una sola scheda attiva, raccolta PC conservata.

## Trasferimento e salvataggi

`pocketdd-device` **v4**, massimo **32768 byte**, aggiunge `inventory` ai campi v3.
Campi esatti: `items` (0–60), `equipped` (0 = nessuna arma, altrimenti indice
nell'array + 1), `combat`. Tutte le voci hanno esattamente:

`name`, `damageType`, `quantity`, `kind`, `ability`, `count`, `sides`, `flags`,
`reach`, `thrownRange`, `attackModifier`, `damageModifier`, `healingModifier`.

Nome ASCII 1–40, tipo danni ASCII 0–24, quantità 0–999; `kind` = 0 oggetto,
1 consumabile, 2 cura, 3 arma. `ability` = indice FOR/DES/COS/INT/SAG/CAR;
`flags` bit 0 competente, 1 caratteristica ai danni, 2 distanza, 3 pesante.
Armi/cure: 1–6 dadi d4/d6/d8/d10/d12. Modificatori attacco/danni −10…+10,
cura 0–30; portata 1–120 per armi, gittata lancio 0–120 in quadretti.
`combat`: `rageBonus` 0–10, `criticalDice` 0–6, booleani `raging`, `powerAttack`,
`thrown`. Arma scelta di tipo arma con quantità positiva; lancio richiede gittata.
Chiavi duplicate/sconosciute, frazioni, combinazioni non valide e array oltre
limite sono rifiutati senza sostituire lo stato. Nessun JSON Beyond originale.

Nuovi record **PDD4, 5184 byte**, CRC negli ultimi quattro byte. Profilo e
formazione conservano gli offset precedenti; dall'offset **434**: disponibilità
inventario, conteggio e indice arma (255 = nessuna), cinque byte combattimento,
60 voci da **78 byte** ciascuna. Campo attacco/danni memorizzato con offset +10.
Niente dump della struct C. Partizioni e chiavi NVS restano le stesse, due copie
alternate nella partizione da 24 KiB: nessun erase o ridimensionamento.

Lettura **PDD1/PDD2 da 384 byte e PDD3 da 512 byte**, senza scritture al boot.
Formazione PDD3 conservata; zaino segnalato non disponibile nei vecchi formati,
mai sostituito da oggetti dimostrativi. Prima modifica scrive PDD4 nell'altra
banca. Golden v3 prodotto dall'encoder effettivo del commit `69d96d0`.
Firmware accetta USB v1–v4; uploader negozia la versione e non invia v4 a firmware
precedenti. Un vecchio firmware non interpreta PDD4: conservare i backup.

## Verifiche e loro portata

- **129 test JavaScript** e build PC/HTML autonomo.
- **1608 confronti PC → JSON USB → parser/regole C**, 140 schede sintetiche:
  caratteristiche/livelli, bonus, armi mischia/distanza/lancio, competenza,
  avvelenato, Ira, attacco potente, critici, cure e limite di 60 voci.
- Restano passanti i **1890 confronti** per abilità/salvezze.
- C: validazione prima dell'RNG, esiti 1/20, critico da 18 dadi, cure a PF pieni,
  equipaggiamento, CRC, 5268 corruzioni singolo bit distribuite nel record,
  5184 troncamenti, generazioni e migrazione golden v1/v2/v3 senza scritture.
- NVS simulata: commit/rilettura, riavvio, copia precedente intatta, recupero e
  errori I/O. Non simula usura, capacità o interruzione di alimentazione fisiche.
- UI LVGL effettiva su host: navigazione e dettagli, selezione/rimozione arma,
  attacco/critico/danni singoli, cure/conferma/annullamento/errore, vecchie copie,
  nomi lunghi, pagina 20 dello zaino, pulsanti nel cerchio alti almeno 44 px.
- ASan/UBSan per core/parser, adapter NVS, regole, renderer e UI; nessun errore
  rilevato. Leak detector spento per l'istanza globale LVGL del test host.
- Protocollo USB con porta simulata; Chromium in profilo effimero verifica
  anteprima inventario, download e raccolta sintetica invariata.

Build ESP32-S3 riuscita con ESP-IDF v5.5.5/LVGL 9.4.0: **822.368 byte**.
partizione app da 4 MiB **80% libera**. Heap/PSRAM e margini degli
stack aumentati (main 49152, USB 24576 byte) vanno misurati sulla scheda. Nessun
flash o test fisico: touch, display, fluidità, USB reale, batteria, riavvio,
capacità NVS dopo molte scritture e perdite di alimentazione restano da provare.
Anteprime riproducibili con `python3 scripts/preview-device.py`, incluso
`firmware/tests/out/ui-build/inventory-preview.png`.

Il compilatore Xtensa con `-fstack-usage` riporta: app_main 5184, caricamento
NVS 15568, selezione banca 10304, decodifica 5184 byte (cammino cumulativo
36240 byte, prima degli ulteriori chiamati); USB 5232 e parser 5504. Sono
stime statiche utili a dimensionare gli stack, non misure del margine a runtime.
