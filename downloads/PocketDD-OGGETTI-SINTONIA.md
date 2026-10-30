# Oggetti e sintonia per tutti i personaggi — v0.86

30 ottobre 2026. Funzioni del modello condiviso, indipendenti dal nome del PG,
dalla classe, dal nome dell'oggetto o dalla specie. Il dispositivo resta il
prodotto; le nuove importazioni dal PC compilano i profili riconciliabili.

## Uso sul dispositivo

**Zaino → oggetto → Equipaggia/Riponi** cambia ciò che indossi. Se è richiesta
sintonia, il dettaglio distingue **bonus inattivi** da **in sintonia**.
**Sintonia → anteprima → Conferma** attiva o termina la sintonia e mostra i valori
che cambiano: CA, caratteristiche, PF massimi, attacchi dell'arma, statistiche
delle magie e bonus a prove/salvezze. Le variazioni sono applicate insieme solo
quando il salvataggio riesce. Si può tornare indietro senza modificare il PG.

La conferma registra una sintonia già ottenuta nel gioco: non esegue il riposo
richiesto, non controlla i requisiti narrativi e non consuma il tempo automaticamente.
Il conteggio degli oggetti sintonizzati include quelli riposti; controlla il
limite del tuo personaggio. Non viene imposto un limite fisso di tre a tutte
le classi: capacità che aumentano il limite richiedono verifica.

Un oggetto con sintonia richiesta applica i bonus supportati solo se è sia
equipaggiato sia sintonizzato. Riporlo toglie i bonus mantenendo la sintonia;
terminare la sintonia toglie i bonus mantenendo lo stato equipaggiato. Una
quantità portata a zero ripone l'oggetto e termina la sintonia. Gli aumenti dei
PF massimi non curano; le riduzioni limitano i PF attuali al nuovo massimo.

**Configura → Armatura e bonus → campo Sintonia (20/20)** permette di impostare
Non richiesta / Richiesta, non sintonizzato / In sintonia anche sui PG già nel
Pocket. Rimane la distinzione della v0.85 tra registrare bonus già inclusi
nell'importazione e applicare proprietà nuove. Non è necessaria una nuova
importazione per configurare manualmente la scheda esistente.

## Importazione generale degli oggetti

Oltre alle armature/scudi standard, il connettore riconcilia profili di oggetti
con bonus permanenti additivi rappresentabili:

- CA, attacchi/danni delle armi, attacchi magici e CD delle magie;
- bonus comuni a prove/abilità o tiri salvezza;
- PF massimi fissi e aumenti delle sei caratteristiche quando non intervengono
  minimi fissati, override o tagli al massimo del punteggio.

I profili si importano anche se il PG non indossa un'armatura. Gli identificativi
dei modificatori devono avere un proprietario univoco; per un effetto attivo il
riepilogo e la definizione originale devono concordare. Equipaggiamento e
sintonia sorgente sono trasferiti separatamente. I numeri già presenti nel PG
sono **adottati**, senza sommare una seconda volta i bonus.

Non si assegnano effetti in base a nomi come “anello”, “mantello” o “bastone”.
Questi nomi servono alle icone; i calcoli leggono le proprietà strutturate.
Un profilo con concessioni discordanti, condizionali o non supportate resta
manuale e viene indicato nel resoconto. Non viene presentata come automatica
una parte del profilo lasciando nascosto un altro effetto dello stesso oggetto.

Esempi di configurazioni, non regole assegnate a un oggetto per il suo nome:

- Una reliquia con **+1 CA e +1 a tutti i TS**, se richiede sintonia, modifica
  entrambi i valori quando è attiva e li rimuove quando termina la sintonia.
- Un focus con **+2 attacchi magici e +1 CD** aggiorna le magie applicabili;
  il bonus agli attacchi magici non diventa un bonus all'attacco con True Strike.
- Un oggetto con **COS +2** aggiorna COS e i tiri collegati; se cambia il
  modificatore, aggiorna i PF massimi di livello totale × variazione del modificatore.

## Icone e compatibilità

Il catalogo condiviso PC/firmware passa a **37 icone semantiche**, aggiungendo
anello, mantello/veste, bastone/focus, stivali e amuleto/collana. Per una voce
sconosciuta resta il simbolo di categoria; l'icona non garantisce una regola.

La sintonia usa **backup PC v22 e USB19** quando è presente. Le schede senza
questa proprietà continuano a usare i formati precedenti pertinenti. Lo stato
nativo diventa **PDDc/38**, a dimensione codificata invariata rispetto a PDDb/37:
nessuna variazione delle partizioni. Il nuovo firmware legge i vecchi salvataggi,
compreso PDDb; app e firmware vecchi rifiutano i nuovi formati invece di perdere
la sintonia. Aggiornare il firmware dal sito conserva la scheda e la SD.

## Limiti ancora espliciti

Restano manuali resistenze o altri effetti difensivi concessi dagli oggetti,
vantaggi condizionali, punteggi fissati, massimi modificabili, proprietà con
restrizioni di classe/bersaglio, armi con personalizzazioni non riconciliate,
altre magie che modificano gli attacchi e interazioni non rappresentate.
La procedura non certifica automaticamente tutti i requisiti D&D, non cambia
il livello e non scrive su Beyond. Testi negli estratti concordati e nessun
requisito Foundry.

## Verifiche

316 test JavaScript; 360 confronti JS → USB18/19 → C con stati differenti;
16 pacchetti malformati rifiutati; 240 scenari nativi sulle sei caratteristiche,
livelli 1–20 e stati equipaggiato/sintonizzato indipendenti. Backup con oggetto
sintonizzato attivo, byte esatti e migrazione PDDb verificati. Nessun vincolo al
nome Soren o a un ID di personaggio.

138/138 viste LVGL esercitate, inclusi annullamento, salvataggio fallito/riprova,
conferme protette dai tasti, perdita dei bonus, limite PF e rimozione dell'oggetto.
Controlli ASAN/UBSAN e build ESP-IDF 5.5.5 per il Waveshare standard; leak check
disabilitato nell'ambiente sandbox. Interfaccia PC verificata su raccolta
sintetica con altra scheda conservata. Le prove nuove usano dati sintetici,
separati dalle letture Beyond reali documentate in precedenza; non attestano un
nuovo trasferimento USB fisico o collaudo della sintonia sull'hardware.

[Anteprima del renderer nativo](previews/v086-attunement.png).
Riproduzione: `npm test`, `npm run firmware:test`, `npm run build`,
`python3 scripts/check-session-ui.py`; con IDF attivato, `idf.py -C firmware build`.
