# Beyond: creazione del cannone con slot — v0.77

Per la variante riconosciuta dell’Artillerist di **Tasha’s Cauldron of Everything**,
Create Eldritch Cannon ha una creazione gratuita per riposo lungo. Dopo averla
spesa, il giocatore può pagare uno slot di livello 1 o superiore per crearne
un altro. La v0.77 trasferisce e applica questo costo sul PC e sul dispositivo.

## Uso e risultato

- Se il gratuito è disponibile, **Crea** propone di consumare quell’utilizzo.
- Quando il gratuito è esaurito, si sceglie uno slot **ordinario o del patto**
  disponibile, poi si leggono anteprima e conferma. Non viene scelto un livello
  diverso da quello mostrato in conferma e non si passa a un’altra riserva.
- Il pagamento toglie esattamente uno slot. Il gratuito resta a zero: non viene
  ripristinato dalla spesa. Il riposo lungo ripristina il gratuito, il breve no.
- Annullamento e salvataggio fallito non modificano i saldi. Il dispositivo
  mantiene la conferma per riprovare; il PC mantiene la scheda precedente.
  È disponibile l’annullamento dell’ultima modifica riuscita.
- Nessun tiro casuale. PF del PG, concentrazione, inventario e i cinque contatori
  manuali rimangono invariati. La creazione è bloccata a zero PF o senza risorsa.

È una capacità con un costo alternativo, non un incantesimo: non riceve una CD
né un attacco da magia. Spendere uno slot non termina la concentrazione né Ira.
Strumenti richiesti, possibilità di agire e altre condizioni vanno verificati
al tavolo prima della conferma.

## Riconoscimento della fonte

Il nome da solo non abilita l’automazione. Servono insieme:

- Classe Artificer `252717` e sottoclasse Artillerist `254188`, entrambe non homebrew.
- Fonte `67`, tipo `1`, su classe, sottoclasse e capacità.
- Capacità Eldritch Cannon `1138963`, livello richiesto 3, classe a livello 3–20.
- Azione Create Eldritch Cannon `1038966`, componente `1138963` / tipo `12168134`,
  attivazione azione e contatore coerente: massimo 1, recupero lungo, nessuna
  formula di caratteristica/competenza o recupero variabile.

Queste identità e la regola di spesa sono state controllate nella risposta
ricevuta dal servizio. Nessun catalogo, testo completo o codice Foundry è incluso.
Le varianti, homebrew o origini non riconciliate conservano il comportamento
precedente e i limiti generali; non ereditano questo costo dal solo nome.

## Lettura reale del 28 ottobre 2026

Connettore PocketD-D esistente, personaggio autorizzato 144777811:
**HTTP 200**, **426.718 byte**, JSON valido, importazione consentita e ancora
parziale. La capacità è riconosciuta con **0/1** creazioni gratuite.

Una copia in memoria è stata usata per confrontare la spesa di uno slot ordinario
con il calcolo C. Il costo coincide, il gratuito resta zero e i cinque saldi
manuali restano **4/4, 1/4, 50/50, 35/50, 50/50**. Coincidono anche caratteristiche
**8/18/14/18/10/10**, PF **32/73**, CA **17**, quantità delle 41 voci inventario e i
30 tiri della proiezione. Nessuna spesa è stata salvata nella raccolta personale,
nessun invio USB e nessuna scrittura verso Beyond.

È un confronto con i dati ricevuti e il modello C sul PC, non con una schermata
Beyond autenticata o con l’hardware. Il sorgente completo è stato elaborato in
memoria; il file temporaneo della sola proiezione nativa, permessi 0600, è stato
eliminato dopo i controlli. JSON completo e credenziali non sono pubblicati.

## Compatibilità

- **Backup PC v19** solo se è presente l’effetto `eldritch-cannon` della capacità.
  V18 e precedenti rimangono leggibili; nessuna migrazione preventiva.
- **USB16**, firmware **0.77**: l’app controlla la versione prima di inviare la
  copia. Senza la nuova capacità si conserva il minimo formato precedente.
- **PDD35 / PDDZ**, con lettura di PDD34/PDDY e precedenti. Layout e dimensioni
  invariati: massimo **75.204 byte**; nessuna nuova partizione o crescita dello
  stato. Il vecchio formato non può contenere di nascosto il nuovo effetto.
- Per leggere/ripristinare i backup nativi nuovi occorrono app e firmware 0.77.
  Un firmware precedente li rifiuta; conservarli senza tentare conversioni manuali.

Non si aggiornano automaticamente le vecchie schede PC: reimportare crea un nuovo
PG senza fondere i progressi della precedente copia. Prima di sostituire la scheda
sul dispositivo va conservato il backup dei progressi importanti.

## Prove sintetiche e limiti residui

**258 test JS**, 108 sorgenti inventati e **2.052 confronti PC/C** delle scelte
free/ordinario/patto, sette pacchetti vecchi/futuri o malformati respinti.
C: 792 stati indipendenti, slot vuoti/estremi, zero PF, Ira, concentrazione,
roundtrip e migrazione PDD34 senza alterazioni. NVS con salvataggio/riavvio e
spesa persistita. Renderer: 119 viste, nuova creazione gratuita e pagata, conferme,
annullamento, errore/riprova, tasti/import protetti e nessun doppio consumo.
Browser con profilo isolato: nuova scheda, conferma, quota atomica, annullamento,
vecchie schede, USB16 e viewport 320/390. Core e UI anche con ASan/UBSan.

Rimangono manuali scelta del cannone attivo, PF della creazione, durata,
attacchi/effetti delle modalità e distruzione/detonazione. Non si uniscono i tre
contatori delle modalità e non si applica una nuova regola a un’altra variante
per somiglianza del nome. Altre risorse condivise, effetti condizionali e proprietà
speciali rimangono parziali. Testi negli estratti concordati; collaudo USB fisico aperto.
