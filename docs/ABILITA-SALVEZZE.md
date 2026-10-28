# Abilità, prove e tiri salvezza sul dispositivo — v0.20

> Aggiornamento v0.74: [associazioni personalizzate delle abilità](ABILITA-CARATTERISTICHE.md),
> Beyond tipo 27 ed editor PC, USB15 / backup PC v17, PDD34/PDDY. Lettura dei precedenti
> e partizioni invariate; nuova prova sintetica, verifica USB fisica ancora aperta.
> Le descrizioni e prove datate sotto conservano il proprio contesto.


La sezione **Scheda** del firmware offre Caratt., Abilita e Salvezze. Le 18
abilità sono divise in sei pagine da tre righe; le salvezze usano una griglia
da sei pulsanti. Tutti i controlli restano nell'area circolare del display.

Toccare una voce prepara il d20. Il pulsante con il nome della prova, il bonus e
`>` apre la composizione: caratteristica, competenza/maestria o mezza competenza,
extra e modalità effettiva. Dal dettaglio si può lanciare o tornare al dado.
Il risultato tiene separati facce naturali, bonus e totale; per due dadi indica
anche Vant/Svant. Animazione e accelerazione non ricalcolano il tiro.

## Valori rappresentati

- Sei caratteristiche e relative prove, 18 abilità, sei salvezze.
- Competenza dal livello: `2 + floor((livello - 1) / 4)`, livelli 1–20.
- Abilità: grado 0/1/2 per assente/competenza/maestria.
- Salvezze: competenza presente o assente; nessuna maestria o mezza competenza.
- Bonus aggiuntivi distinti per prova/abilità/salvezza, interi da −30 a +30.
- Mezza competenza alle prove riconosciute dal PC; sulle abilità si applica
  soltanto se il grado è zero. Arrotondamento per difetto, senza sommare anche
  una competenza piena.

Esempio: DES +3, mezza competenza +1 ed extra −1 danno **+3**. A livello 20,
caratteristica 30, maestria ed extra +30 danno **+52**; senza competenza,
caratteristica 1 ed extra −30 danno **−35**. Il firmware non tronca questi totali.

`firmware/core/checks.c` contiene il calcolo puro; non usa RNG né salvataggi.
Il renderer del dado riceve un risultato già generato. I tipi, gli indici e le
associazioni caratteristica/abilità sono confrontati con il modello PC.

## Effetti e valori da verificare

La copia PC trasferisce anche due istantanee già gestite dal modello:
**Avvelenato** dà svantaggio a prove e abilità, mai alle salvezze; il vantaggio
FOR da **Ira** si applica ai contesti FOR riconosciuti dal modello, incluse le
salvezze FOR. Una fonte di vantaggio e una di svantaggio si annullano; fonti
multiple dello stesso segno non si sommano.

Base/Vant/Svant sceglie la richiesta manuale. **Effetti PC: si/no**, nel
dettaglio, permette di escludere le due istantanee per il contesto selezionato,
per gestire una decisione del DM. La modalità risultante è visibile prima del
lancio. Selezionare un'altra prova riattiva gli effetti PC; queste scelte di tiro
sono di sessione, senza scritture NVS.

Le istantanee non sono una gestione completa di condizioni e capacità: il
dispositivo non aggiorna durate, utilizzi di Ira, equipaggiamento o altri effetti.
Le altre condizioni e i bonus condizionali non risolti restano fuori dal perimetro.

Le voci in `blockedChecks` arrivano come **? / Da verificare**. Non mostrano un
bonus totale come verificato e non permettono di lanciare quel contesto, nemmeno
con un evento inviato a un pulsante disabilitato. Il dettaglio mostra i contributi
noti e permette di passare al **dado libero**, con bonus zero, per risolvere la
prova con il DM. Disattivare Effetti PC non rimuove un blocco di conversione.

## Trasferimento PC e compatibilità

L'anteprima PC contiene una tabella espandibile delle **30 prove**, con bonus,
composizione e voci da verificare. L'esportazione è una copia limitata e conserva
la raccolta; non contiene la risposta originale di Beyond. Nessuna dipendenza da
Foundry. La conversione reale Beyond resta descritta separatamente in
[BEYOND-PROVE.md](BEYOND-PROVE.md): i test qui usano dati sintetici.

La copia `pocketdd-device` **v3** aggiunge l'oggetto obbligatorio `training`:

| Campo | Contenuto |
| --- | --- |
| `skillRanks` | 18 chiavi canoniche, interi 0–2 |
| `saveProficiencies` | Sei chiavi caratteristica, booleani |
| `skillBonuses`, `saveBonuses`, `checkBonuses` | Mappe complete, interi −30…+30 |
| `halfAbilityChecks` | Booleano |
| `halfSkills` | Abilità canoniche senza duplicati |
| `blockedChecks` | Contesti `check:`, `skill:`, `save:` senza duplicati |
| `poisoned`, `strengthAdvantage` | Booleani per le due istantanee |

Il parser rifiuta campi mancanti, sconosciuti/duplicati, chiavi o contesti non
canonici, booleani numerici e numeri fuori intervallo/frazionari. Massimo sempre
4096 byte. Il firmware dichiara `PDD READY 3`; il nuovo uploader invia v1/v2/v3
solo se il dispositivo dichiara una capacità compatibile.

Le vecchie copie USB v1/v2 e i record NVS v1/v2 non contengono competenze.
Vengono letti conservando i dati esistenti, con `training.available=false`:
prove di caratteristica di base ancora utilizzabili, abilità e salvezze con
messaggio **Importa una copia aggiornata**. Non si deduce che una competenza
assente dal file equivalga a un personaggio non competente.

Le nuove scritture NVS sono record **PDD3 da 512 byte**, con CRC e due banche.
Il caricamento legge anche PDD1/PDD2 da 384 byte senza riscrivere; la prima
modifica confermata scrive nella banca opposta. Record futuri con CRC valido o
errori I/O bloccano le scritture, senza cancellazione automatica. Il ritorno a
un firmware precedente non è una migrazione automatica. Backup PC v10 invariato.

## Verifiche

```sh
npm test
npm run firmware:test
python3 scripts/preview-device.py
```

127 test JavaScript. Il confronto `scripts/check-training.js` passa dal JSON PC
al parser C effettivo e verifica **1.890 combinazioni di bonus/modalità**:
livelli 1–20, tutti i contesti e le tre richieste manuali, più la conversione di
una fixture Beyond sintetica. Nessuna lettura del personaggio personale.

Test C per punteggi 1–30, gradi 0–2, extra −30/0/+30, estremi, blocchi ed effetti.
Migrazione da record golden prodotti dagli encoder effettivi della v0.18/v0.19,
recupero della banca precedente, CRC, troncamenti e versioni future.

UI LVGL host: sei pagine di abilità, salvezze, dettaglio, tiro, annullamento di
vantaggio/svantaggio, override manuale, blocco contro eventi forzati, dado libero
e messaggio per copie vecchie. Nessun salvataggio durante i tiri. Core, adapter
NVS, calcoli e UI verificati anche con ASan/UBSan; leak detector disabilitato per
l'istanza LVGL globale del test. Anteprime in `firmware/tests/out/ui-build/`,
inclusa `skills-preview.png`, dal codice reale eseguito su host.

Restano da collaudare sul dispositivo fisico leggibilità, precisione touch,
memoria, fluidità e consumi; nessun flash eseguito da questi controlli.
