# Riconciliazione Beyond — correzione PC del 26 ottobre 2026

> Aggiornamento v0.71, 27 ottobre 2026: [inventario operativo e difese Beyond](BEYOND-INVENTARIO-DIFESE.md).
> Cariche, consumabili e cure esplicite; profilo difese opzionale con USB14 / backup PC v16.
> Stato nativo PDD33 invariato. Prova reale e dati sintetici distinti; conversione ancora parziale.
> Le descrizioni e prove datate sotto conservano il proprio contesto; verifica USB fisica ancora aperta.


Correzione dell'importatore PC sulla base della 0.65.0. Il firmware rimane
0.65.0; nessuna dipendenza Foundry, autenticazione privata o scrittura verso
Beyond. La raccolta personale dell'utente non è stata aperta né modificata.

## Perché la spunta era disabilitata

La spunta dipende da `canImport`, non dal successo HTTP. La conversione
precedente trattava come irrisolti anche nomi/note personalizzati degli oggetti,
preferenze opzionali senza sostituzioni effettive e aumenti di caratteristica
selezionati con `isGranted:false`. Gli oggetti custom potevano già comparire
nell'inventario con tipo nullo, oppure esistere soltanto in `customItems`.

La nuova normalizzazione lavora su una copia e riconcilia solo dati verificabili:

- I flag di preferenza non bloccano da soli. Sostituzioni effettive in
  `optionalOrigins`/`optionalClassFeatures` restano bloccate.
- I bonus numerici +1/+2 alle sei caratteristiche richiedono ID di scelta,
  opzione selezionata e definizione concordante. Non si accettano indiscriminatamente
  tutti i modificatori `isGranted:false`.
- `characterValues` tipo 8/9 sono nomi/note di un oggetto individuato univocamente
  per ID, con tipo sorgente previsto. Modifiche numeriche sconosciute, riferimenti
  mancanti e personalizzazioni discordanti restano bloccati.
- I custom sono riconciliati per ID e quantità. Una definizione già presente
  non viene duplicata; le definizioni mancanti dall'inventario sono aggiunte con
  quantità esplicita e stato equipaggiato sconosciuto. Stessi nomi con ID distinti
  restano distinti. Dati discordanti o meccaniche custom non risolte bloccano.
- I contatori morte nulli sono vuoti (zero). Valori non validi restano rifiutati.
- Per una singola classe con divisore slot 2, la tabella esplicita
  `spellRules.levelSpellSlots[level]` può fornire i massimi; `spellSlots.used`
  fornisce il consumo. Nessun massimo viene dedotto dal nome della classe.
  Tabella corrotta, consumi mancanti/incompatibili o casi non supportati
  producono un avviso e slot da configurare, senza riserve inventate.

I blocchi compaiono sotto **Problemi che bloccano l'importazione**, separati
dai limiti accettabili della scheda parziale. La spunta non aggira i blocchi.

## Lettura reale del personaggio 144777811

Lettura con il connettore ordinario PocketD-D dal servizio
`character-service.dndbeyond.com`, senza credenziali. Il JSON completo è rimasto
in memoria; non è stato salvato, stampato integralmente o inserito negli archivi.

| Controllo | Risultato sul sorgente reale |
| --- | --- |
| Accesso HTTP | 200, 426.718 byte; sola lettura. |
| Formato | JSON valido, struttura e ID corrispondenti al link autorizzato. |
| Caratteristiche | FOR 8, DES 18, COS 14, INT 18, SAG 10, CAR 10. Controllati valori base, bonus e scelte attive. |
| PF | 53 base + 20 da COS = 73 massimi; 41 rimossi → 32 attuali al momento della lettura. |
| CA | 12 armatura + 4 DES + 1 bonus oggetto = 17. |
| Inventario | 38 righe + 20 definizioni custom, di cui 17 già presenti e 3 aggiunte → 41 righe. Tutte le quantità conservate, senza fusione per nome. |
| Armi | 6 configurazioni; 4 armi equipaggiate nella fonte. Scelta esplicita dell'arma attiva. |
| Magie e capacità | 17 magie e 33 capacità/note, non 50 effetti automatizzati. |
| Slot ordinari | Massimi 4/3/2 e residui 4/2/1, dalla tabella e dai consumi espliciti. |
| Conversione | `canImport:true`, nessun blocco; `conversion.complete:false`. |
| Copia dispositivo | Proiezione USB9 riuscita. Il parser C del firmware eseguito sull'host accetta il pacchetto; confrontati caratteristiche, PF, CA, tutte le quantità, numero di capacità e slot con la proiezione PC. |

Questo confronto è con i dati ricevuti e la loro aritmetica. **Non è un confronto
con una schermata autenticata di Beyond**: l'utente deve confermare i valori
nell'anteprima prima di creare la nuova scheda. Non costituisce prova di
trasferimento USB o di uso di questo PG sull'hardware.

L'elenco incollato dall'utente comprendeva talenti e un aumento del massimo di
caratteristica che non compaiono nel sorgente corrente del link. Non si dichiara
risolto quel diverso insieme di dati: armature sconosciute, modifiche dei massimi
(`ability-score-maximum`), override numerici e sostituzioni opzionali non supportate
continuano a bloccare la conversione.

## Limiti che rimangono

Oggetti custom: nomi, quantità e note disponibili, senza dedurre effetti da testo.
Prezzo/peso, contenitori, proprietà speciali e meccaniche custom restano manuali.
Azioni personalizzate sono note, alcuni recuperi giornalieri/variabili non sono
automatizzati, testi lunghi sono estratti dichiarati. Non si ricalcolano CA/PF
massimi dopo modifiche locali. Le condizioni e gli effetti speciali non sono
un motore completo delle regole D&D. Le note degli oggetti rimangono sul PC;
il dispositivo riceve nomi, quantità e configurazioni supportate.

## Prove sintetiche e pacchetto

189 test JavaScript passati. I nuovi casi usano dati inventati per coprire
24 combinazioni di bonus scelti, dati discordanti, override cosmetici/numerici,
custom duplicati/mancanti, quantità zero, capacità inventario, contatori nulli
e massimi/consumi degli slot. Controllata anche la non mutazione della fonte.

Tre prove Chromium separate (`check-beyond-import.py`, `check-beyond-rich.py`,
`check-beyond-reconciliation.py`) verificano anteprima/conferma, spunta, blocchi,
scelta arma, invalidazione cambiando URL, creazione di una nuova scheda,
conservazione della raccolta isolata, reload, backup e gestione errori.
Sono **prove sintetiche**, senza richieste a Beyond e senza il profilo utente.

`npm run installer:build` e `check-pc-package.py` verificano il pacchetto PC:
ZIP integro, estrazione, avvio dell'app compilata senza `node_modules`, risorse
UI e validazione locale del connettore. La shell Windows e il dispositivo
restano da verificare dall'utente; il firmware distribuito è invariato.

## Come riprovare su Windows

1. Chiudere la precedente finestra del programma PC.
2. Scaricare il nuovo [pacchetto PC](https://pocketdd-installer.shydan.chatgpt.site/downloads/PocketDD-PC.zip)
   ed estrarlo completamente in una nuova cartella.
3. Avviare `Avvia-PocketDD.cmd`, usare lo stesso browser e
   `http://127.0.0.1:5173/` per conservare l'accesso alla raccolta esistente.
4. **Importa da D&D Beyond → 144777811 → Leggi e verifica**. Confrontare valori,
   inventario e limiti; scegliere l'arma attiva, spuntare e confermare la creazione.
   Viene aggiunto un nuovo PG; le schede precedenti rimangono conservate.
5. Per il dispositivo seguire [Scheda e backup via USB](USB-BROWSER.md).
   Il trasferimento sostituisce la copia attiva sul dispositivo: leggere
   l'anteprima e confermare sul display.

## Riferimenti di formato

Controllati come riferimento tecnico, senza dipendenza dall'app di riferimento:
`MrPrimate/ddb-importer`, commit `44bb451175da3e96dbfcb71547de59995655609e`,
[tipi di personalizzazione](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/config/dictionary/items/generics.ts),
[inventario](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/inventory.ts),
[caratteristiche](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/abilities.ts),
[stato](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/effects.ts),
[slot](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/spellCasting.ts).
