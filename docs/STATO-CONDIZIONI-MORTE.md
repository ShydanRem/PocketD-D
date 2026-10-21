# Stato, condizioni e salvezze contro morte — v0.24

**Aggiornamento v0.44:** [effetti espliciti sui tiri](CONDIZIONI-TIRI.md) per
Accecato, Spaventato, Prono e Trattenuto, separati dalle note, con circostanze
e blocchi prima di RNG. Non sono condizioni complete. PDD23, USB8/backup v11.
Le limitazioni e misure storiche sotto si riferiscono alla versione del titolo.

> Documento della v0.24. La v0.25 aggiunge [Schermo](SCHERMO-IMPOSTAZIONI.md),
> preferenze locali e PDD8; USB v7 e regole di questo blocco rimangono invariati.

> La v0.28 aggiunge [danni ricevuti e difese manuali](DANNI-DIFESE.md):
> i casi già a zero PF/morte istantanea rimangono bloccati in quel flusso.

Questo blocco porta sul dispositivo le annotazioni delle condizioni e il registro
morte del modello PC, con un d20 base animato e correzioni esplicite. Tutto
funziona offline; il PC rimane lo strumento di importazione, configurazione e
backup. Nessuna dipendenza Foundry e nessuna nuova lettura personale Beyond.

## Accesso e tasti

**Usi → Stato** mostra PF/temporanei, registro morte, condizioni annotate e
Avvelenato. I contatori manuali restano visibili nella schermata Usi.
**Eroe → Aspetto → Tasti** può assegnare **Stato** a PWR breve, BOOT breve o
BOOT lungo. Le assegnazioni precedenti sono conservate; PWR lungo resta
riservato all'alimentazione.

Le conferme richiedono il touch. I tasti fisici non confermano, consumano o
chiudono una conferma. Un'importazione USB mentre è aperta una conferma riceve
`busy`. Durante il volo del dado la scorciatoia Lancia può accelerare il volo,
senza cambiare il risultato.

## Condizioni: annotazioni ed effetto separato

Sono trasferite tutte le **30 condizioni** ammesse dal modello PC, con nome
ASCII fino a **60 caratteri** e livello **0–6**. I nomi vengono traslitterati;
il nome originale resta sul PC. La lista ha tre righe per pagina e il dettaglio
mostra il nome su più righe. Si può modificare il livello o rimuovere una nota,
sempre con anteprima e conferma. Aggiungi propone 15 nomi comuni; il testo
libero si configura sul PC. Le note possono essere duplicate come nel modello
PC; l'app non le interpreta come istanze di effetti cumulativi.

**Nessuna regola viene dedotta dal nome o dal livello di una nota**, anche se
si chiama Poisoned, Avvelenato, Unconscious o Spossatezza. Non vengono applicati
in automatico esaurimento, velocità, immunità, incapacità, concentrazione,
durate o recuperi delle condizioni. Il livello è un'annotazione, non un criterio
che seleziona il regolamento 2014 o 2024.

**Avvelenato: sì/no** è un effetto meccanico indipendente, già riconosciuto dal
modello PC: impone svantaggio a prove/abilità e attacchi, annullando un vantaggio
contrapposto. Non modifica salvezze, dado libero o tiro morte base. Per le prove
rimane disponibile l'esclusione degli effetti per il singolo tiro. Il comando
funziona anche dopo una vecchia copia con competenze mancanti, per le prove di
caratteristica disponibili. Attivare/disattivare l'effetto richiede conferma e
non cambia le note. Rimuovere una nota non spegne l'effetto.

L'editor PC **Stato, risorse e note** offre ora una casella separata per questo
effetto. Modificare annotazioni o contatori non ricalcola più Avvelenato dai
nomi. L'anteprima USB espone note, contatori morte e flag applicato.

## Registro e tiro base

Vengono copiati **riuscite, fallite e stabilizzato** esattamente come conservati
nel modello PC. La loro presenza nel modello non certifica che ciascun valore
fosse presente nella risposta originale Beyond. La conversione Beyond e le sue
omissioni restano documentate separatamente in BEYOND-PROVE.md.

Una vecchia copia o una scheda senza dati estesi mostra **registro non
disponibile**, con `--` e tiro disabilitato. **Avvia registro → Conferma** crea
esplicitamente contatori locali a zero. Aggiungere una nota di condizione non
inizializza il registro morte. Le due operazioni rimangono distinte.

**Lancia d20 base → Conferma** è disponibile soltanto a zero PF, con registro
attivo, senza stabilizzazione e con entrambi i contatori inferiori a tre:

| Faccia naturale | Modifica |
| --- | --- |
| 1 | Due fallimenti, fino al limite di tre |
| 2–9 | Un fallimento |
| 10–19 | Una riuscita |
| 20 | Torna a 1 PF e azzera entrambi i contatori e stabilizzato |

La terza riuscita stabilizza e azzera i due contatori. Tre fallimenti bloccano
altri tiri e mostrano **verifica con il DM**. Una combinazione importata come
tre riuscite senza stabilizzazione o tre riuscite/tre fallite è conservata e
richiede una correzione; non viene dichiarata corretta in silenzio. A PF
positivi i contatori importati non coerenti sono segnalati e il tiro è bloccato.

Il tiro è **un solo d20 senza modificatori**, competenza, vantaggio, bonus
salvezza, Avvelenato o Ira. Talenti, bonus di oggetti, reroll e regole opzionali
si gestiscono con il DM e con le correzioni; il firmware non pretende di
ricostruirli dal testo della scheda.

## Correzioni e interazioni con PF/recuperi

**Correggi / stabilizza** offre riuscita/fallita +1, correzioni −1,
stabilizzazione, critico +2 e azzeramento. Ogni comando mostra l'effetto prima
della conferma. Stabilizzare azzera i contatori; segnare un fallimento su un
personaggio stabilizzato riapre il registro. Correggere o azzerare tre fallimenti
è una decisione esplicita con il DM; non applica resurrezione o PF da solo.

Il passaggio da PF positivi a zero inizia un nuovo conteggio; una cura da zero a
PF positivi lo azzera. Resistenza Implacabile riconosciuta e pozioni seguono lo
stesso azzeramento quando possono essere usate. Il lungo azzera il registro e
lo dichiara nell'anteprima. I temporanei non riportano in piedi il personaggio.

**Tre fallimenti a zero PF bloccano PF +1, pozioni, Resistenza Implacabile e
riposo lungo**, prima di consumare risorse o entropia. Occorre una correzione
esplicita del registro. Il breve non cura; conserva il registro. Contatori
generici, riserve e note non diventano un motore completo degli effetti.

I danni a zero PF **non aggiungono automaticamente fallimenti**: il comando
PF −1 continua a gestire PF/temporanei. Il giocatore segna fallita +1 o critico
+2 dopo aver verificato con il DM danni effettivi, temporanei e critico. Non
sono implementati morte istantanea, danni massicci, resistenze, immunità o
autorizzazione generale delle azioni secondo le condizioni. Il registro non è
un certificato automatico di vita/morte e non sostituisce il DM.

## Salvataggio, annullamento e animazione

Il risultato viene estratto dopo conferma e **salvato prima dell'animazione**,
insieme a PF e contatori. Accelerare o ridisegnare non rilancia e non applica
due volte il risultato. Al termine, Altro torna al registro e un altro tiro
richiede una nuova conferma. Un comando Lancia sul risultato già impegnato non
genera un nuovo tiro. **Eroe → Annulla** ripristina l'ultima modifica salvata di
sessione, compresi PF/registro/note/effetto.

Un errore di scrittura conserva lo stato visibile. Un tentativo ripetuto nella
stessa conferma riusa il tiro già estratto; non estrae nuova entropia. Un errore
di commit può essere ambiguo: controllare lo stato al riavvio. Dopo un'interruzione
fra salvataggio e animazione restano PF e contatori, mentre non esiste ancora
uno storico NVS delle facce. Nessuna sincronizzazione di ritorno al PC.

## Formato e compatibilità

**USB v7**, limite invariato di 262144 byte. Aggiunge il campo obbligatorio:

```json
"status": {
  "available": true,
  "successes": 1,
  "failures": 2,
  "stabilized": false,
  "conditions": [{"name": "Nota sintetica", "level": 1}]
}
```

Chiavi esatte, interi e booleani stretti; contatori 0–3, livelli 0–6, massimo
30 note. `available` indica la disponibilità del **registro morte**: quando è
false, i contatori devono essere zero e stabilizzato false, ma sono ammesse
annotazioni locali. Le copie PC v7 lo impostano quando esiste il modello
esteso. I formati USB v1–v6 restano leggibili senza creare registri.
L'uploader negozia `PDD READY 1`–`7` e rifiuta l'invio a firmware troppo vecchio.
Il backup PC resta v10 e la raccolta esistente mantiene le proprie schede.

**PDD7: 7915 + 552 × numero magie/capacità**, massimo **74155 byte**. Prima del
CRC PDD6 aggiunge cinque byte di stato e 30 note da 62 byte. Il layout flash
v0.22/v0.23 rimane invariato, compresa `pdd_state` da 256 KiB. Stato e candidati
delle conferme rimangono in PSRAM; non vengono aggiunti grandi oggetti sullo
stack. La nuova scorciatoia locale 8 è Stato.

PDD1–PDD6 restano leggibili. Il fixture PDD6 è prodotto dal vero encoder del
commit `da7b8e2`, prima delle modifiche, usando dati sintetici. Nessuna scrittura
di migrazione al boot e nessuna modifica della partizione legacy. Un downgrade
alla v0.23 incontra PDD7 sconosciuti e blocca il caricamento per conservarli:
usare firmware aggiornato e backup privati, senza cancellare la NVS.

## Verifiche

- **138 test JavaScript** e build PC/HTML autonomo.
- **16013 nuovi confronti PC → USB → parser reale → C**, su tutte le 64
  combinazioni di contatori, PF zero/positivi e stabilizzazione; ogni faccia
  d20, 30 note/livelli, cure e lungo. Totale con le regressioni: **37751**.
- Schema USB v1–v7, fixture indipendenti PDD1–PDD6, salvataggio/rilettura PDD7,
  dati di stato malformati con CRC valido, errori I/O e allocazioni.
- UI LVGL reale su host: note/limiti/nome da 60 caratteri, effetto separato,
  anteprima/cancel/conferma, 1/10/20, stabilizzazione, correzioni, tre fallimenti,
  riuso del tiro su errore, annullamento, import busy e tasti protetti; pulsanti
  nel cerchio alti almeno 44 px.
- Chromium su raccolta effimera: configurazione Avvelenato indipendente dalle
  note, anteprima ed esportazione v7 con valori verificati e raccolta invariata.
- Sanitizzatori AddressSanitizer/UndefinedBehaviorSanitizer e build ESP32-S3.

Anteprima `firmware-host-preview/status-preview.png` nell'archivio: catture del
renderer LVGL su host. Non sono prove di display, touch, USB, pulsanti elettrici,
alimentazione, entropia fisica o perdita di corrente sulla scheda reale.
