# Magie sul Pocket: preparazione e slot — v0.95

3 novembre 2026. Funzioni comuni ai personaggi importati e alle schede già
presenti: nessuna eccezione legata al nome del PG e nessuna reimportazione
necessaria. Il PC rimane per installazione, associazione Beyond e configurazione.

## Prepara più magie, salva una volta

**Magie → Prepara** apre la preparazione delle magie di livello 1–9.
Tocca una voce per cambiarne la disponibilità nella bozza. Il bordo evidenzia
le modifiche; i filtri **Tutte / Pronte / Modifiche** e le pagine permettono
di rivedere anche un elenco completo di 120 voci.

**Rivedi → Salva** mostra quante magie diventano preparate e quante vengono
tolte, poi salva l’intero gruppo con una sola operazione. **Annulla** scarta
tutta la bozza. Se il salvataggio fallisce, la bozza resta disponibile per
riprovare. Se la memoria richiede il riavvio, non viene proposta una nuova
scrittura; **Annulla** dalla home dopo un salvataggio riuscito ripristina
l’ultima modifica, secondo il normale comando di annullamento del Pocket.

Trucchetti e capacità non entrano nella modifica multipla. Slot, utilizzi,
PF e concentrazione restano invariati. Una magia già in concentrazione non
viene interrotta perché si toglie la preparazione: sono due stati distinti.
Un cambiamento della raccolta magica durante la bozza blocca il salvataggio
della bozza precedente, senza sovrascrivere la nuova scheda.

La preparazione individuale già presente rimane disponibile dal dettaglio.
Limiti di classe, scambi consentiti durante i riposi e magie sempre preparate
**non vengono ricavati automaticamente**: prima di salvare verifica i requisiti
del tuo PG. La funzione non concede magie che non sono nella scheda.

## Scegli lo slot direttamente

Dal dettaglio della magia, tocca **Slot … >**. Il pannello mostra separatamente
**Ordinari** e **Patto** e tutti i nove livelli: `L2: 2/3` significa livello 2,
due slot rimasti su tre. I livelli troppo bassi o esauriti sono disabilitati.
Tocca il livello per tornare al lancio con quel pagamento selezionato.

La prima proposta usa il **livello minimo disponibile fra entrambi i gruppi**;
a parità di livello propone gli ordinari. Uno slot patto di livello 1 viene
quindi proposto prima di uno slot ordinario di livello 5. Questa è una proposta,
non una spesa: puoi scegliere l’altro gruppo o un livello superiore. Una scelta
esplicita ancora disponibile non viene sostituita dal prossimo render.

Il pannello è lo stesso per:

- magie che richiedono slot, compresi costi con utilizzo innato aggiuntivo;
- Punizione Divina 2024, dopo il colpo ammesso dal motore esistente;
- creazione del cannone, quando è già stata usata la creazione gratuita.

Trucchetti, capacità gratuite e utilizzi senza slot non aprono un pannello
di pagamento fittizio. La scelta non consuma risorse, non tira dadi, non cura
e non cambia la concentrazione. Il consumo avviene solo confermando il lancio
o **Usa e tira** per Punizione, con le regole e le protezioni già presenti.
Il pannello dei costi dell’app PC usa lo stesso ordinamento per livello/gruppo.

Usare uno slot superiore non automatizza i danni o gli altri effetti di una
magia generica. Restano rappresentati i profili già documentati per True Strike,
Hex/Marchio, Punizione e creazione del cannone; altre magie richiedono ancora
profili espliciti. Non è stato aggiunto un motore universale D&D.

## Tasti e compatibilità

Nei nuovi pannelli **PWR breve** sposta il fuoco, **BOOT breve** attiva la voce,
**BOOT lungo** torna indietro. Le assegnazioni personali sono conservate.
Import/backup USB, scorciatoie generali e altre conferme non possono sostituire
il PG durante una bozza. Nessuna scansione o connessione Beyond è richiesta
per preparare magie o scegliere uno slot sul Pocket.

Aggiorna il firmware conservando i dati. Nessun cambio a partizioni, formato
PDDe/40, USB21, backup PC24, raccolta SD PDL1 o look. I formati precedenti
rimangono leggibili; l’estensione di associazione precedente resta compatibile.
Il bundle del convertitore è rigenerato dalla sorgente condivisa e confrontato
nuovamente con il PC su dati sintetici; nessuna nuova lettura reale Beyond.

## Verifiche

356 test JavaScript, suite nativa e **153 viste LVGL**, build ESP32-S3 e controlli
ASan/UBSan. Nuove prove su preparazione multipla, ultimo elemento/120 voci,
filtri, anteprima, annullamento, errore/riprova, annullamento dell’ultima modifica,
rifiuto di una bozza obsoleta, tasti locali, slot ordinari/patto e scelta del
livello minimo. Il motore C controlla che consultare i costi non consumi risorse
e che ciascuna scelta paghi il gruppo corretto soltanto al lancio.

[Schermate del renderer firmware](previews/v095-magic-workflow.png), con magie
e saldi sintetici. Nessun accesso a raccolte personali, nessun JSON reale e
nessun nuovo collaudo fisico USB, touch, tasti, radio o memoria dichiarato.
