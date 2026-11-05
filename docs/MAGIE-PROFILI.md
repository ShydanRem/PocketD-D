# Formule salvate e formule base Beyond · v0.109

> v0.110: [Attacco, TS nemico e critici delle magie](MAGIE-COMBATTIMENTO.md).

5 novembre 2026. Il tiro assistito della v0.108 ora può riutilizzare una formula
salvata con la magia. La scelta del bersaglio resta esplicita a ogni lancio.

## Sul Pocket

**Magie → scegli → Gestisci → Formula**:

1. Scegli **Danni** o **Cura**; **Senza** rimuove il profilo.
2. Tocca la formula per impostare dado, quantità ed extra numerico. Il pulsante
   FOR/DES/COS/INT/SAG/CAR collega il modificatore della caratteristica della
   magia: non la competenza né il bonus al tiro per colpire o alla CD.
3. Scegli **Fissa**, **Per livello slot** o, per un trucchetto, **Livelli PG
   5 / 11 / 17**. Il passo indica quanti dadi aggiungere, dello stesso tipo.
4. **Rivedi → Salva** conserva il profilo senza lanciare né consumare risorse.

La quantità è quella al livello base della magia, anche se hai selezionato uno
slot superiore per il prossimo lancio. Per esempio, 2d8 + caratteristica con
passo 2 per livello slot diventa 6d8 + caratteristica a slot 3. Un uso innato
senza slot mantiene il livello base; non ottiene gratuitamente l'aumento.

Poi **Lancia → Danni / Cura me / Cura altri** apre direttamente il riepilogo
quando il profilo scelto è applicabile. **Modifica** cambia solo quel lancio,
senza riscrivere la base salvata. Gli aumenti dipendono dallo slot scelto o dal
livello totale del PG; il bonus collegato segue i punteggi attuali, anche dopo
modifiche supportate dell'equipaggiamento. Gli altri bonus restano espliciti.

Formule e tiri hanno limiti distinti: una formula valida può produrre più dei
18 dadi consentiti scegliendo uno slot elevato. Il lancio viene bloccato, senza
tagliare dadi o sostituire la formula con 1d6. La gestione manuale **Solo costo**
resta disponibile. Bonus finali ammessi da −15 a +60, minimo zero; nessun dado
tirato prima della conferma. Errore e riprova mantengono lo stesso risultato.

Annullare l'editor non salva. Una scheda cambiata durante l'anteprima non può
ricevere la vecchia bozza. PWR/BOOT permettono scelta, attivazione e ritorno
locale senza modificare le assegnazioni personali.

## Riconoscimento Beyond: primo insieme verificato

Il convertitore richiede identità, fonte PHB 2024 e struttura coerenti; non
ricava una regola dal nome o dalla descrizione. Controlla dado base, uso della
caratteristica, restrizioni e tutti gli aumenti. Dati discordanti restano manuali.

| Identità 2024 | Formula base | Aumento |
| --- | --- | --- |
| Cure Wounds · `2619079` | 2d8 + caratteristica della magia | +2d8 per livello slot oltre il primo |
| Fire Bolt · `2618890` | 1d10 | 2d10 / 3d10 / 4d10 ai livelli PG 5 / 11 / 17 |
| Sacred Flame · `2618967` | 1d8 | 2d8 / 3d8 / 4d8 ai livelli PG 5 / 11 / 17 |

Queste sono **formule di base**, non tutte le meccaniche dell'incantesimo.
Colpire, TS, critici, immunità/resistenze del bersaglio, bonus speciali di classe
o oggetti, più componenti, ulteriori bersagli e condizioni restano al tavolo.
Per esempio, un dado di danno aggiuntivo concesso da una capacità non viene
aggiunto solo perché è stata riconosciuta la formula del trucchetto.

Il profilo indica **Base Beyond** oppure **Profilo manuale**. Modificarlo sul
Pocket lo rende manuale; cambiare solo un lancio non riscrive il profilo e quel
lancio non viene presentato come formula originale Beyond.

La presenza dei dadi nel sorgente non basta per rendere una magia eseguibile:
servono anche origine, caratteristica e costi riconciliati. Una voce importata
come nota non viene promossa automaticamente a magia. Le versioni 2014, le altre
identità e i casi non supportati conservano il percorso manuale. Non sono stati
aggiunti profili per nomi simili o formule dedotte dal testo.

## Conservazione e aggiornamento

Aggiornare il firmware con **Conserva dati** mantiene i PG presenti. Puoi
configurare le loro magie dal dispositivo senza reimportare. Il riconoscimento
Beyond si applica alle **nuove importazioni**: non arricchisce silenziosamente
le copie già salvate e non unisce dati remoti e progressi locali.

Le formule appartengono alla singola magia e seguono scheda attiva, backup e
raccolta SD. Due PG con lo stesso nome rimangono distinti. Una nuova importazione
con SD salva la copia uscente; senza SD esiste un solo PG attivo. Per una nuova
importazione con progressi importanti conserva prima un backup.

Formati nuovi solo quando ci sono formule: **backup PC v25 / USB22 / stato
nativo PDDf**, leggibili con app e firmware 0.109. I vecchi formati restano
leggibili. Senza profili resta la codifica nativa PDDe precedente, byte per
byte; non cambiano partizioni, asset SD, protocollo del ponte o credenziali.
Un firmware precedente non riceve pacchetti USB22. Per un nuovo collegamento
USB con l’estensione usare anche il componente PC/estensione aggiornato; gli
accessi account già memorizzati sul Pocket restano conservati. Il backup nativo conserva
l'intero record e il PC ne verifica anche la sezione delle formule.

## Lettura reale e prove sintetiche

Le letture pubbliche hanno usato il connettore PocketD-D esistente, in sola
lettura; nessuna raccolta personale o credenziale modificata. Nessun JSON
completo del personaggio è incluso nel repository o nei pacchetti.

- PG `144777811`: HTTP 200 e JSON valido. Due profili base, Cure Wounds e Fire
  Bolt, passano al modello PC e al parser C. Dieci risoluzioni aritmetiche delle
  formule coincidono: non sono dieci lanci o spese effettuati. Il convertitore
  QuickJS nativo sull'host produce inoltre l'intero record identico a quello
  proiettato dal PC sugli stessi dati; picco dell'interprete 2.902.413 byte.
- PG `167361077`: HTTP 200 e JSON valido. Le definizioni strutturate di Cure
  Wounds e Sacred Flame sono riconosciute, ma manca una caratteristica di lancio
  riconciliata: le due voci restano **note**, senza formula eseguibile nel PG
  convertito. L'accesso HTTP non elimina questo limite.

Prove sintetiche: 28.800 combinazioni native di indice, tipo, livello e
aumento/pagamento, con bonus collegato alla caratteristica; 123 confronti PC/USB22/C e 17 pacchetti
malformati respinti; record completo di 120 profili, migrazione dai salvataggi
precedenti, backup e due schede sulla libreria host. I casi LVGL coprono
annullamento, bozza obsoleta, errore/riprova senza RNG, aumento col patto,
modifiche per singolo lancio, bonus collegato, provenienza e rimozione.

Queste verifiche non collaudano Wi-Fi/HTTPS sull'ESP32, USB o SD fisiche. Sul
Pocket usare una scheda di prova: salvare un profilo, riavviare, ritrovarlo,
confrontare slot ordinario/patto e provare backup/cambio PG. Restano separati
autenticazione privata, sincronizzazione Beyond e i collaudi dell'alimentazione.

Controlli di release: 361 test JavaScript, suite nativa e 159 viste LVGL passati;
ASan/UBSan su motore e interfaccia, build ESP32-S3 e verifiche del pacchetto/sito.
La prova fisica dei nuovi profili resta da eseguire.
