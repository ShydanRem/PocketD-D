# Pocket come RPG tascabile — revisione UX in corso

3 novembre 2026. La priorità concordata è rivedere l’esperienza complessiva del
dispositivo. I nuovi blocchi funzionali della roadmap restano annotati in
[STATO-SOFTWARE.md](STATO-SOFTWARE.md); la priorità attuale è UI/UX fino a un
risultato soddisfacente per l’utente. Questa revisione non dichiara finito il lavoro.

## Direzione

Pixel art 2D-HD, avatar completo e ambienti già approvati. Toni notte/petrolio,
avorio, ottone e un accento per categoria. Cornici nette e pulsanti con rilievo
sobrio; icone condivise, ingrandite senza sfocatura. Il dispositivo fisico da
466×466 px resta il riferimento, insieme al touch e ai tasti configurabili.

L’azione e il risultato vengono prima della spiegazione. Le formule, le proprietà
e le note estese rimangono nei dettagli. Costi, motivo di un blocco e problemi di
salvataggio devono essere visibili: ridurre il testo non significa nascondere
informazioni necessarie a decidere.

## Prima iterazione sul renderer nativo

- **Home:** avatar e scena al centro, azioni rappresentate da icone più grandi;
  nomi disponibili con pressione prolungata. Niente istruzioni permanenti.
- **PF:** danni, cura, temporanei e stato come azioni principali. Le correzioni
  manuali −1/+1 passano in una vista dedicata; annullamento ancora accessibile.
- **Attacco:** apertura diretta del comando di tiro; arma e variante magica
  selezionabili nella stessa schermata. Dettagli numerici dietro il riepilogo,
  catalogo completo delle varianti ancora disponibile. Nessun tiro o consumo
  automatico entrando nella schermata.
- **Magie e inventario:** icone coerenti, righe con gerarchia fra nome e stato,
  comandi secondari più compatti. Testi lunghi nei dettagli.
- **Struttura comune:** il messaggio sano «Salvato offline» non occupa sempre una
  riga. Errori, memoria incerta, recuperi e avvisi batteria rimangono visibili.

## Criteri di riuscita

1. L’utente riconosce l’azione principale prima di leggere una spiegazione.
2. La home apre l’attacco ordinario in un tap; il tiro richiede il suo comando.
3. Una variante con arma si seleziona senza passare obbligatoriamente da un menu.
4. La vista PF non propone subito modifiche che saltano difese/concentrazione.
5. Nomi lunghi, saldi massimi, stati bloccati ed errori restano leggibili.
6. Nessun cambio a regole, dati, formati, SD o assegnazioni personali dei tasti.

## Iterazioni successive della stessa priorità

Rivedere risultati dei dadi e danni, lancio delle magie e costi, uso/equipaggiamento
degli oggetti, preparazione, riposi e stato. Poi raccolta/importazione, aspetto e
impostazioni. Le viste avanzate non diventano automaticamente una UX completa
perché condividono il nuovo tema: ogni percorso va semplificato e rivisto.

La valutazione estetica dell’utente, le prove del renderer e il riscontro sul
display fisico sono evidenze distinte. Le immagini del renderer non dimostrano
fluidità, touch o consumi sull’hardware.

## Verifica e aggiornamento

Prima iterazione distribuita come v0.96. Aggiornare da GitHub Pages con
**Conserva dati**; nessuna nuova importazione o installazione dei look richiesta.
Le funzioni del componente PC non cambiano e il precedente resta compatibile.

I percorsi vengono esercitati attraverso gli eventi pointer del renderer LVGL:
home → attacco → lancio in due tap, scelta della magia con tutte le sei
caratteristiche, preparazione con annullamento e salvataggio fallito/riprovato,
assenza di consumo di slot durante la scelta, correzioni PF separate e pressione
prolungata senza eseguire l’azione. Restano controllati i tasti configurabili,
i confini circolari e i bersagli di almeno 44 px delle 153 viste.

[Anteprima nativa con personaggio sintetico](previews/v096-rpg-ux.png).
Non è una fotografia dell’ESP32 e non dimostra touch o fluidità fisici.

Da rivedere subito dopo: risultati e azioni successive al tiro, selezione e
lancio magie, equipaggiamento/uso degli oggetti, riposi/stato. Raccolta, aspetto,
importazione e impostazioni seguono nella stessa revisione, non vengono
considerati terminati dal solo cambiamento del tema.

Controlli della release: 356 test JavaScript, suite nativa e 153 viste LVGL,
ASan/UBSan e compilazione ESP32. Nessuna nuova lettura Beyond reale o modifica
alla raccolta personale; le nuove prove usano solo dati sintetici.
