# Collaudo dei comandi sul dispositivo — v0.55

> Aggiornamento v0.62: [alimentazione e batteria](ALIMENTAZIONE.md).
> Telemetria, avvisi, pausa/risveglio e spegnimento con salvataggio sono ora
> implementati; le prove fisiche e le misure restano da eseguire. Descrizioni
> e limiti successivi sono quelli storici della versione del titolo.

Il blocco 3 dispone ora anche di una prova locale per tocco e tasti, accessibile
senza un programma PC. La scheda fisica è ancora in arrivo: il percorso è
verificato con LVGL su host e campioni sintetici, **non sull'hardware reale**.
Le tre viste aggiunte portano la copertura host a **100/100**.

## Accesso e uscita

**Eroe → Aspetto → Tasti → Collaudo**. Le assegnazioni dei tre gesti restano
quelle configurate. La schermata iniziale offre **Prova tocco** e **Prova tasti**,
con riepilogo dei bersagli e dei gesti osservati nella sessione corrente.

**Indietro** torna al riepilogo; **Esci** torna alle impostazioni dei tasti.
Ogni apertura di una prova, Azzera o ritorno al riepilogo avvia una finestra
di **120 secondi**. Alla scadenza il collaudo torna ai tasti con l'avviso
**Prova scaduta; scheda conservata**. Il limite vale anche con un input
mantenuto; una nuova pressione non prolunga da sola il tempo.

La modalità mantiene lo schermo attivo e blocca la navigazione di gioco,
le scorciatoie assegnate e le nuove importazioni/richieste di backup, che
ricevono `PDD ERROR busy`. La diagnostica USB in sola lettura rimane
disponibile e segnala `screen.busy=true`.

## Cinque bersagli di tocco

I cinque quadrati numerati, da **56×56 pixel**, coprono alto, destra, basso,
sinistra e centro dell'area di prova. È attivo soltanto il prossimo numero;
il bordo oro lo evidenzia e quelli riconosciuti hanno il bordo menta.

1. Toccare il numero evidenziato e rilasciare al suo interno.
2. Controllare che venga riconosciuto il numero previsto nella posizione giusta.
3. Ripetere fino a cinque; usare **Azzera** per ricominciare.

Il conteggio avanza sul click di un puntatore LVGL con rilascio dentro il
bersaglio attivo. Una pressione mantenuta non avanza da sola; il rilascio dopo
un trascinamento fuori non accetta quel bersaglio. Tocchi esterni, altri numeri
e semplici eventi UI senza puntatore non completano la sequenza.

Il riepilogo espone **tocchi riconosciuti**, non una certificazione automatica
del pannello. Posizione, orientamento, precisione vicino ai bordi, colori e
tocchi involontari vanno osservati e registrati. Le coordinate non vengono
ricalibrate né salvate: eventuali correzioni del driver seguiranno le misure.

## PWR e BOOT

Tre righe mostrano gesto, scorciatoia attualmente assegnata e numero di gesti
ricevuti, da 0 a **999**:

| Gesto | Riconoscimento nel collaudo |
| --- | --- |
| PWR breve | Un evento al rilascio dopo debounce |
| BOOT breve | Un evento al rilascio prima degli 800 ms |
| BOOT lungo | Un evento al rilascio dagli 800 ms |

Il debounce è lo stesso dell'app (**30 ms**). Una pressione già mantenuta
all'apertura non conta: occorre prima osservare un rilascio. La pressione
lunga PWR resta riservata all'hardware; l'app non la conteggia come scorciatoia
e non introduce un comando di alimentazione.

Se il livello non è disponibile, la riga mostra **Lettura non disponibile**.
**Azzera** azzera i conteggi e riavvia l'attesa di un rilascio. Nel collaudo la
scorciatoia mostrata è solo un riferimento: PWR assegnato a Lancia non estrae
dadi, e BOOT assegnato a Zaino non cambia pagina. Il riepilogo conta quanti
dei tre tipi di gesto hanno almeno una ricezione, non il totale delle pressioni.

All'uscita e al timeout la pressione portata dalla prova viene assorbita fino
al rilascio. Serve un gesto nuovo per azionare i comandi normali. Lo stesso
vale per un contatto touch ancora mantenuto: il ritorno alle impostazioni non
deve modificare una scorciatoia sottostante al dito.

Questi conteggi usano il riconoscimento dei gesti. I contatori PWR/BOOT della
[diagnostica USB](PRIMO-AVVIO.md) contano invece fronti grezzi e possono
differire per rimbalzi, pressioni lunghe o input bloccati.

## Dati e compatibilità

Risultati, conteggi e timer del collaudo sono solo in RAM. Esci/una nuova
sessione o riavvio non producono un attestato persistente. La registrazione
manuale usa [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).

Nessuna modifica a PF, inventario, risorse, effetti, monete, storico, tasti
assegnati o altre preferenze. Nessuna scrittura di scheda/registro NVS,
estrazione RNG, sincronizzazione con la raccolta PC o accesso Beyond.
Scheda **PDD25**, registro **PDH2** (lettura PDH1), proiezione **USB9**, backup
nativo v1, backup PC v11, diagnostica v1 e partizioni restano invariati.
PMIC, batteria, sospensione e spegnimento controllato non cambiano.

## Verifiche software

- Controller C: ordine/duplicati/limiti dei bersagli, riconoscimento dei tre
  gesti, pressioni mantenute/rimbalzi/input disabilitati o indisponibili,
  PWR lungo riservato, saturazione, azzeramento e timeout attraverso il wrap.
- UI di produzione: vero hit-test LVGL con flusso sintetico di puntatore,
  pressione/rilascio/trascinamento e cinque bersagli. Nessun evento diretto
  sostituisce il puntatore per completare la sequenza nelle prove.
- Stesso controller dei tasti del firmware, assegnazioni Lancia/Zaino/Storico,
  nessuna azione di gioco. Uscita e tre timeout con tasto/dito mantenuti,
  successiva ripresa delle scorciatoie solo dopo un nuovo gesto.
- Scheda, storico, impostazioni, copia pendente e annullamento precedenti
  confrontati; zero scritture ai due store e zero chiamate RNG. Import e
  backup rifiutati durante la prova, diagnostica leggibile.
- Controlli geometrici di tutte le **100 viste**, catture iniziali, sequenza,
  gesti ricevuti, input indisponibile, contatori massimi e timeout.

`firmware-host-preview/input-check-preview.png` e `input-check-*.png` sono
catture native su PC, con periferiche sintetiche. Non dimostrano precisione
del touch, lettura elettrica dei tasti, resa o affidabilità fisica della scheda.
