# Abilità con caratteristica personalizzata — v0.74

28 ottobre 2026. Un'abilità può usare una caratteristica diversa da quella
standard: per esempio Intimidire con FOR. L'associazione viene importata da
Beyond o configurata sul PC e resta un dato separato da competenza e bonus.
I punteggi successivi vengono usati dinamicamente anche sul dispositivo.

## Importazione e configurazione

Il campo standard Beyond `characterValues.typeId = 27` identifica un'abilità
con `valueId` e la caratteristica scelta con `value` da 1 a 6. Si usano gli ID
stabili Beyond già verificati nella v0.73, non gli indici della lista locale.
Null/0 significano non impostato; identificativi sconosciuti, valori fuori
intervallo, formule, requisiti o scelte duplicate discordanti bloccano.
Nomi delle abilità e descrizioni non determinano l'associazione.

Sul PC: **Personalizza eroe → Abilità → Caratteristica**. Predefinita conserva
l'associazione normale; FOR/DES/COS/INT/SAG/CAR sono scelte esplicite.
L'anteprima Beyond, la scheda e il dettaglio del tiro mostrano la caratteristica
effettiva. Grado, mezza competenza e bonus extra restano indipendenti.

Esempio sintetico: Intimidire con FOR 14, maestria a livello 3 e bonus +1 dà
+2 +4 +1 = **+7**. Portando FOR a 20 diventa **+10**; scegliendo nuovamente
la caratteristica standard CAR 8 diventa **+4**. Non si memorizza un bonus
compensativo che diventerebbe sbagliato dopo modifiche dei punteggi.

La scelta segue anche gli effetti supportati: il vantaggio FOR da Ira si
applica a un'abilità associata a FOR; con Avvelenato si annulla secondo il
modello esistente. Una mappa personalizzata non sblocca un tiro marcato
Da verificare per altri motivi. Le salvezze e le prove di caratteristica
mantengono le proprie associazioni; le scelte non cambiano armi o magie.

L'editor conserva la bozza fino al salvataggio. Annullare non scrive; quota
esaurita o conflitti di salvataggio lasciano intatta anche la scheda attiva
in memoria, mantenendo aperto l'editor con l'errore. Le altre schede restano
conservate. Per rimuovere tutte le associazioni scegliere Predefinita e salvare.

## Dispositivo e compatibilità

- Backup PC **v17** con il campo opzionale `extended.skillAbilities`. I formati
  precedenti restano leggibili e non possono contenere silenziosamente il campo.
- Trasferimento **USB15**, solo quando l'associazione opzionale è presente.
  Il firmware precedente viene rifiutato prima di inviare il PG.
- Firmware **0.74**, stato **PDD34/PDDY**: aggiunti diciotto byte al record
  nativo, uno per abilità (0 Predefinita, 1–6 scelta esplicita). Dimensione
  massima **75.204 byte**. Partizioni e layout della flash invariati.
- Lettura dei vecchi stati, incluso PDD33: associazioni nuove inizializzate
  a Predefinita, nessuna scrittura di migrazione all'avvio. NVS, backup nativo
  e ripristino conservano la mappa completa insieme a scheda e progressi.
- I lettori di backup PC/Python riconoscono PDD34 e conservano il record
  originale: la normalizzazione degli offset serve solo all'anteprima.
  Il dispositivo esegue sempre la validazione completa prima del ripristino.
- Senza associazioni personalizzate, il PC conserva il formato minimo
  precedente pertinente (v11–v16, USB9–USB14). Un firmware 0.74 salva comunque
  nel nuovo formato nativo. Per aprire i nuovi backup serve l'app aggiornata.

Sul dispositivo il dettaglio del tiro mostra la caratteristica effettiva,
il contributo della competenza e gli extra. La navigazione e i tasti
configurabili usano i percorsi esistenti. La verifica visiva di questa versione
è sul renderer host, non sul display fisico dell'utente.

## Prove sintetiche

- **246 test JavaScript** complessivi: tutte le 18 abilità per sei
  caratteristiche, aggiornamento dei punteggi, competenze ed extra separati,
  effetti FOR/Avvelenato, mappe non valide, namespace/target/duplicati,
  compatibilità e conservazione della raccolta.
- **324 schede sorgente** (18 × 6 × tre livelli) attraverso conversione,
  USB15, parser C e codec nativo; bonus, mezza competenza, modalità e
  persistenza verificati; undici pacchetti incompatibili/malformati respinti
  senza modificare lo stato del chiamante.
- Calcolo C diretto su 18 × 6 × 20 livelli × tre gradi: caratteristica,
  bonus ed effetti dipendenti dalla caratteristica. Controllati anche NVS,
  migrazione PDD33 con 0/60/120 capacità e backup completi con la nuova mappa.
- Browser: importazione senza scritture prima della conferma, editor PC,
  annullamento, errore quota atomico, esportazione USB15, ritorno ai default
  e ai formati precedenti, anteprima/scaricamento del backup PDD34 identico.
  Layout desktop, 390 e 320 px; regressioni di scheda estesa e USB simulato.
- Regressioni native dell'intera scheda e dei 118 pannelli; prova UI nativa di
  Intimidire con FOR, competenza e vantaggio pertinente. Parser/codec
  delle 324 copie e UI host anche con ASan/UBSan. Build ESP32-S3 riuscita.

Sono dati inventati. Nessuna nuova lettura reale di Beyond in questo incremento,
nessun accesso alla raccolta personale e nessun invio USB fisico. L'ultima
lettura reale del PG rimane quella della v0.73, distinta da queste nuove prove.
La conversione globale resta parziale; non vengono dichiarate risolte altre
formule, origini opzionali o regole condizionali.

Riferimento di formato, non dipendenza: `getCustomSkillAbility` in
[skills.ts di MrPrimate/ddb-importer, commit 44bb451175da3e96dbfcb71547de59995655609e](https://github.com/MrPrimate/ddb-importer/blob/44bb451175da3e96dbfcb71547de59995655609e/src/parser/character/skills.ts).
