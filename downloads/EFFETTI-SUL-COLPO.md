# Hex e Marchio del Cacciatore 2024 — v0.88

30 ottobre 2026. Il Pocket collega due effetti al flusso degli attacchi, per tutti
i PG che li possiedono. Non vengono aggiunte magie a una scheda che non le ha.

## Al tavolo

1. **Attacca** mostra anche Hex o Marchio configurati. Se l'effetto non è attivo,
   toccalo, prepara la magia se necessario, scegli lo slot ordinario/patto e
   conferma il lancio. Dopo un lancio riuscito dal menu attacco rientri nell'arma.
   Un errore di salvataggio conserva scheda e costi precedenti fino alla riprova.
2. Puoi usare l'attacco normale o True Strike. L'effetto in concentrazione rimane
   separato dalla variante con arma; non occorre lanciarlo di nuovo a ogni colpo.
3. Dopo un colpo riuscito, **Danni** domanda se questo è il bersaglio maledetto o
   marchiato. **No: danni base** esclude il bonus; **Sì: con extra** lo include.
   Il menu non tira dadi e Indietro conserva il tiro per colpire.
4. Vedi il totale comune e le parti distinte: arma, eventuali dadi radianti di
   True Strike ed extra necrotico/forza. **Radianti / Extra / Dadi arma** scorre
   gli stessi risultati congelati. La visualizzazione non rilancia i dadi.

Il colpo non consuma altri slot né utilizzi. Un mancato non apre questa scelta.
Terminare o perdere la concentrazione, oppure sostituirla con un altro lancio,
rimuove la disponibilità dell'extra. Un effetto ancora concentrato continua anche
se togli la preparazione della magia: preparazione e concentrazione sono distinte.

Nel selettore di bersaglio funzionano i tasti locali già usati negli editor:
PWR breve cambia fuoco, BOOT breve attiva, BOOT lungo torna indietro. Le
assegnazioni personali rimangono conservate.

## Regole rappresentate

- **Hex 2024:** extra base 1d6 necrotico sul bersaglio confermato.
- **Marchio del Cacciatore 2024:** extra base 1d6 forza sul bersaglio confermato.
- Su un critico il dado aggiuntivo raddoppia a 2d6. Salire con lo slot non aumenta
  questi dadi base; gli slot rimangono quelli effettivamente scelti nel lancio.
- I bonus agli attacchi magici e alla CD non diventano danni di questi effetti.
  Le schede mostrano il tipo di extra, senza attribuire un attacco o una CD fittizi.
- Arma, radianti ed extra vengono registrati insieme nello storico. Se la scrittura
  fallisce, la riprova salva le parti congelate insieme e non estrae altri dadi.

Fonti ufficiali consultate: [Hex](https://www.dndbeyond.com/spells/2618988-hex)
e [Hunter's Mark](https://www.dndbeyond.com/spells/2619166-hunters-mark).
Le regole sono implementate indipendentemente; nessun catalogo di testi estesi
è incluso. Restano gli estratti concordati per le descrizioni importate.

## Importazione e PG già presenti

L'import riconosce gli identificativi delle due definizioni 2024 (2618988 e
2619166), livello 1, concentrazione e fonte PHB 2024 verificabile (145, tipo 1).
Nome o icona non concedono regole; definizioni sconosciute, homebrew e versioni
Legacy non ottengono questo profilo automaticamente. A livello totale 20 non
si assegna automaticamente il profilo base del Marchio: occorre verificare
il potenziamento del dado e le capacità del personaggio.

Su una magia già presente di livello 1 e concentrazione puoi scegliere
**Gestisci → Regole della magia → Hex 2024 / Marchio 2024 → Anteprima → Conferma**.
La configurazione è esplicita: verifica edizione e regole effettive al tavolo.
Non concede una magia, non la lancia e non spende costi. Per cambiare il profilo di
un incantesimo già in concentrazione occorre terminare prima la concentrazione.

Per import e backup via USB servono app PC e firmware 0.88. Aggiorna anche
l’app PC se la usi con il Pocket aggiornato, compresi i PG senza questi effetti. Le schede esistenti
si possono configurare sul dispositivo senza cancellarle o reimportarle.
Formati: **PC23** e **USB20** solo quando è presente un nuovo profilo; **PDDd/39**
per il salvataggio nativo. Dimensioni e partizioni invariate; lettura dei vecchi
salvataggi conservata. Un firmware USB19 non riceve una copia con i nuovi effetti.
La SD e il catalogo dei look non richiedono modifiche.

## Cosa resta manuale

Bersagli e loro PF non sono registrati: la conferma vale per questo singolo colpo,
non identifica automaticamente un mostro e non sposta il Marchio a un altro.
Durata, distanza, visibilità, azione bonus, requisiti per spostare l'effetto,
svantaggio alle prove da Hex e vantaggio nel seguire la preda dal Marchio rimangono
al tavolo. Non si applicano automaticamente ad abilità o salvezze del proprio PG.

Capacità che modificano il dado o aggiungono altri effetti (per esempio Foe Slayer,
Relentless/Precise Hunter, invocazioni e sottoclassi), colpi multipli delle magie, altre edizioni, altri rider
e Smite non sono automatizzati. L'integrazione dei dadi riguarda gli attacchi con
arma e True Strike rappresentati dal Pocket. Non è un motore completo delle magie.

## Prove e lettura reale

Nuova lettura pubblica del PG 167361077: **HTTP 200**, risposta JSON valida e
conversione estesa eseguita tramite il connettore esistente. Non contiene Hex:
questa lettura non verifica i nuovi danni o la nuova importazione di Hex/Marchio.
Nessun JSON completo salvato o pubblicato e nessuna
raccolta personale modificata.

322 test JavaScript, 139 viste native, controlli ASAN/UBSAN, build ESP32 e
verifica browser con raccolta sintetica. Le nuove regole usano dati sintetici: 480 combinazioni di effetto/caratteristica/
livello/critico nel motore C; costi, concentrazione e mancata applicazione sul
bersaglio escluso; confronto JS/USB/C e backup nativo con profilo attivo;
trasporto USB20 e rifiuto del downgrade. La UI nativa verifica preparazione,
lancio, costi, conferma del bersaglio, totale e parti congelate, errore/riprova
dello storico e profilo esplicito di una scheda già presente.

[Schermate del renderer firmware](previews/v088-on-hit-effects.png), con personaggi
sintetici. Prove software distinte da trasferimento USB e uso fisico del Pocket,
ancora da eseguire per queste nuove interazioni.
