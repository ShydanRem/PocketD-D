# Attacco normale o con magia — v0.87

30 ottobre 2026. Funzione del Pocket, comune a tutti i personaggi e a tutte le
caratteristiche da incantatore. Non occorre reimportare il PG, cancellare salvataggi
o modificare la SD: basta aggiornare il firmware dalla pagina GitHub Pages.

## Dal tavolo al tiro

1. Tocca **Attacca** sulla home, oppure **Attacco** dalla scheda dell'arma.
2. Se il PG possiede varianti con arma riconosciute, scegli **Attacco normale**
   oppure la magia: per esempio **True Strike / Colpo Accurato 2024**. Ogni voce
   mostra l'icona condivisa e i valori dell'attacco: caratteristica, bonus, dadi
   arma e dadi radianti aggiuntivi. Le origini con caratteristiche diverse
   rimangono scelte distinte; non si usa la caratteristica di un altro incantesimo.
3. Regola eventualmente CA bersaglio, vantaggio o contesto e premi **Lancia**.
   Dopo il tiro per colpire, **Danni** tira e registra le componenti pertinenti.

Senza varianti con arma si apre direttamente l'attacco ordinario, senza menu extra.
Se manca l'arma, si sceglie/configura prima quella e poi si sceglie l'attacco.
Il pulsante **Varianti** permette di cambiare modalità dalla schermata dell'arma.
Se torni indietro dal selettore iniziale rientri nella schermata da cui eri partito.

## Preparazione e disponibilità

Le magie pronte compaiono prima; seguono quelle da preparare e infine le proposte
di configurazione esplicita. **Da preparare: tocca qui** apre direttamente la
conferma: se salvi, rientri nell'attacco con quella magia già selezionata.
Annullare conserva il PG; se il salvataggio fallisce puoi riprovare.
Preparare non spende slot, non recupera utilizzi e non termina la concentrazione.
Requisiti e limiti della preparazione restano da verificare al tavolo.

A zero PF e durante Ira la variante magica è disabilitata con il motivo visibile.
Una nota di vecchia importazione propone **Regole da configurare**:
non acquisisce effetti solo in base al nome. La configurazione 2024 è esplicita.

La voce **True Strike: tipo dell'arma / arma radiante** sceglie il tipo dei dadi
dell'arma per questa magia; i dadi aggiuntivi restano radianti. Non cambia il tipo
dell'attacco ordinario. Il selettore non tira dadi e non scrive nella scheda.
Cambiare modalità invalida il risultato di combattimento precedente; occorre
un nuovo tiro prima dei danni. Lo storico già salvato rimane consultabile.

## Tasti fisici

Nel selettore e nella preparazione aperta dall'attacco: **PWR breve** sposta il
fuoco, **BOOT breve** attiva la voce, **BOOT lungo** torna indietro. Le assegnazioni
personali dei tasti rimangono conservate. PWR lungo mantiene la funzione di sistema.

## Regole supportate e confini

La variante numerica rappresentata è **True Strike 2024**: usa la caratteristica
dell'origine da incantatore per il tiro con arma e il bonus danni, mantiene la
competenza e i bonus dell'arma, scala i dadi radianti ai livelli 5/11/17 e raddoppia
i dadi pertinenti sul critico. I bonus agli attacchi magici non diventano bonus
agli attacchi con arma. Le regole restano nel motore di combattimento già condiviso.

Altre magie (Smite, Hex, Bless, Booming Blade, ecc.) non ricevono regole automatiche
da un nome, un'icona o una descrizione. Richiedono modelli espliciti per tempi,
bersaglio, concentrazione, slot e danni; per ora si gestiscono manualmente tramite
le funzioni disponibili. Non vengono offerte come varianti automatiche riuscite.
Questo aggiornamento migliora il percorso contestuale, non dichiara completo
un motore universale degli incantesimi né la sincronizzazione in scrittura Beyond.

## Verifiche

Catture ottenute dal renderer LVGL del firmware, con personaggi sintetici:
[sequenza delle schermate](previews/v087-contextual-attacks.png).
316 test JavaScript, suite nativa, 138 viste e controlli ASAN/UBSAN.
Prove del percorso touch, tutte le sei caratteristiche da incantatore, identità
e ordine delle origini, paginazione, attacco ordinario e magico, preparazione,
annullamento, errore di salvataggio e riprova, blocchi a zero PF/Ira e risultati
precedenti invalidati. Nessun JSON reale, nuova lettura Beyond o scrittura nella
raccolta personale. Formati USB19 / PC22 / PDDc38 e dimensioni delle partizioni
restano quelli della v0.86. Nuove interazioni da verificare sul dispositivo fisico.
