# Effetti numerici locali sul dispositivo — v0.26

> Aggiornamento v0.30: [effetti collegati alla concentrazione](EFFETTI-CONCENTRAZIONE.md),
> fine/sostituzione congiunta e PDD12. I limiti e formati sotto sono storici.

> Documento della v0.26. La v0.27 aggiunge [durate a round e scadenze](DURATE-EFFETTI.md)
> e PDD10; i limiti sulle durate riportati sotto sono storici.

Primo incremento del blocco effetti: fino a **otto modificatori numerati**, ciascuno
con destinazione CA oppure PF massimi, valore positivo/negativo e stato attivo.
Sono variazioni additive esplicite; il dispositivo non interpreta descrizioni o
nomi delle condizioni. Funzionano offline, senza Foundry, PC o servizi durante
il gioco. Nessuna nuova lettura personale Beyond o modifica della raccolta PC.

## Uso e valori base

**Usi → Stato → Effetti** apre l'elenco. **Aggiungi** crea una bozza; toccare una
riga modifica quella esistente. Si sceglie CA/PF massimi e si regola il valore
con −10/−1/+1/+10. Cambiare destinazione azzera il valore della bozza. Attivo
include o esclude quel modificatore dal totale. Elenco abbandona la bozza.
Le righe sono numerate, senza nomi liberi o associazione automatica a magie.

**Anteprima** mostra i valori base importati, CA effettiva, PF massimi effettivi e
PF correnti prima/dopo. Soltanto **Conferma** salva. Rimuovi apre un'anteprima di
rimozione della voce salvata, ignorando le modifiche non confermate della bozza.

CA e PF massimi importati restano conservati come **basi**, non vengono riscritti
a ogni attivazione. Possono già contenere bonus della sorgente: **aggiungere
soltanto variazioni non già incluse**. Esempio: base CA 16, effetto locale +2,
CA effettiva 18; disattivandolo torna 16. Questo non ricostruisce la CA da
armatura, scudo, DES, stile di combattimento o proprietà degli oggetti.
Le caratteristiche importate e gli altri bonus fotografati restano invariati.

I modificatori attivi si sommano; decidere con il DM quali possano coesistere.
Non ci sono regole implicite di cumulabilità, sostituzione o precedenza. Anche
voci a zero o inattive possono essere conservate. Per ogni voce sono ammessi
−20…+20 alla CA e −999…+999 ai PF massimi. Il totale deve restare **CA 1–40** e
**PF massimi 1–999**: valori fuori limite vengono rifiutati, senza arrotondamenti
o correzioni silenziose. Anche rimuovere un effetto può essere bloccato se
lascerebbe altri modificatori con un totale non valido: modificarli prima.

## PF, cure e riposi

Aumentare il massimo **non cura**. Ridurlo limita i PF correnti al nuovo massimo,
lasciando invariati i PF temporanei. La perdita è indicata in anteprima e avviene
solo con la conferma; riattivare l'effetto non restituisce i PF persi. Non è danno,
non usa resistenze e non avvia prove di concentrazione. Un massimo zero è fuori
perimetro: morte istantanea ed eccezioni richiedono ancora gestione con il DM.

Esempio: base PF massimi 32, effetto +10, massimo 42. Aggiungerlo lascia i PF
correnti invariati; si può poi curare fino a 42. Se i correnti sono 41 e l'effetto
viene disattivato, l'anteprima indica massimo 42 → 32 e correnti 41 → 32.

PF ±1, pozioni, cure con dadi vita e riposo lungo usano tutti il massimo
effettivo, anche quando supera o è inferiore alla base importata. I blocchi
esistenti a tre fallimenti/zero PF restano applicati. Gli effetti non riportano
in piedi da soli un personaggio a zero PF e non azzerano il registro morte.

Gli effetti restano attivi attraverso riposi, zero PF, fine concentrazione e
riavvio, finché non vengono modificati esplicitamente. Non sono legati a una
magia o a una scadenza: durata e condizioni di fine sono gestite dal giocatore.
Le annotazioni delle condizioni e il flag Avvelenato rimangono indipendenti.

## Comandi e conservazione

**Eroe → Aspetto → Tasti** permette di assegnare **Effetti** a PWR breve, BOOT
breve o BOOT lungo. Le assegnazioni precedenti sono conservate; PWR lungo rimane
riservato all'alimentazione. I tasti non saltano né confermano l'anteprima.
Durante la conferma l'importazione USB riceve busy e lo schermo non si attenua.
Il primo input di risveglio continua a essere assorbito.

Un salvataggio riuscito applica insieme modificatore e limite dei PF.
**Eroe → Annulla** ripristina l'ultima modifica salvata della sessione, compresi
effetti e PF. Una conferma senza cambiamenti evita scritture. Le bozze non
scrivono flash e gli effetti non estraggono dadi. Un errore conserva lo stato
visibile e permette di riprovare; un errore di commit NVS può essere ambiguo e
richiede controllo dopo riavvio, come per gli altri comandi.

Gli effetti appartengono alla scheda locale, non alle preferenze del dispositivo.
**Una nuova copia USB sostituisce anche gli effetti locali**, dopo anteprima e
conferma: il display lo segnala quando ce ne sono. Annullare conserva tutto.
Luminosità, inattività, animazione e tasti restano locali e vengono mantenuti.
La raccolta PC non viene modificata. Non esiste ancora un backup di ritorno dei
progressi/effetti locali: il backup PC conserva i dati del PC, non questi nuovi
aggiornamenti del dispositivo.

## Formato e compatibilità

USB **v7** e backup PC **v10** invariati: nessun campo effetti accettato dal
parser USB e nessuna automatizzazione della conversione Beyond in questo passo.
Le vecchie copie iniziano con zero effetti locali, senza inventare modificatori.

Nuove scritture **PDD9: 7950 + 552 × numero magie/capacità**, massimo **74190 byte**.
Dopo luminosità/inattività di PDD8, prima del CRC: conteggio (un byte) e otto
record da quattro byte (destinazione, valore +999 uint16 little-endian, attivo
booleano stretto). La scorciatoia locale 10 è Effetti. Nessun dump di struct.
La partizione pdd_state da 256 KiB e il layout flash restano invariati.

PDD1–PDD8 rimangono leggibili, senza riscrittura al boot. Il fixture PDD8 è
prodotto dall'encoder reale del commit **9a5c946**, prima delle modifiche, con
soli dati sintetici. La prima scrittura conserva il record precedente nell'altra
banca. Un downgrade alla v0.25 non riconosce PDD9 e blocca il caricamento per
proteggere i dati; non cancellare NVS per aggirarlo. Nessun flash eseguito qui.

## Verifiche e prossimo passo

- **17266 casi indipendenti** di base/delta/attivazione: tutte le basi CA 1–40 e
  PF massimi 1–999, estremi positivi/negativi e rifiuti senza mutazione.
- Cure, pozioni, dadi vita e lungo sopra/sotto la base; nessuna cura implicita,
  temporanei invariati, blocchi morte e indipendenza delle note.
- Otto effetti, rimozione/compattazione, totale non valido dopo rimozione,
  120 magie e record massimo, CRC valido con campi malformati e output intatti.
- Fixture PDD1–PDD8, migrazione senza scritture al boot e rilettura NVS di PF
  superiori alla base con effetti attivi; errori I/O e versioni future.
- UI LVGL reale su host: bozza, anteprima/cancel/conferma, errore/riprova, limite
  PF, annullamento, nessuna scrittura a stato invariato, otto voci/pagine,
  scorciatoia, import busy e sostituzione esplicita. Aree touch nel cerchio ≥44 px.
- Regressioni: 138 test JS, build PC e 37751 confronti PC → USB → C; sanitizzatori
  e build ESP32-S3. Leak detector disattivato nel sandbox; ASan/UBSan attivi.
  Anteprime prodotte dal renderer host, non foto del display.

**Il blocco effetti resta parziale**: mancano durate/scadenze, resistenze,
immunità, proprietà speciali, regole complete delle condizioni e ricalcolo da
equipaggiamento/classe. I prossimi incrementi estenderanno il modello esplicito;
grafica e tasti accompagnano le viste, mentre batteria/sospensione e collaudo
fisico restano un blocco separato. Backup dispositivo → PC è supporto ancora da
completare, non un cambio di priorità verso un'app principalmente desktop.
