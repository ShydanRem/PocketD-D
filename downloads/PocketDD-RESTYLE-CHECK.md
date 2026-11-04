# Chiusura del ciclo di restyle · v0.107

4 novembre 2026. Il giro software concordato dopo la v0.106 comprendeva
schermate secondarie e coerenza generale. Gli interventi previsti sono chiusi;
restano la valutazione estetica dell'utente e la convalida pratica sull'hardware.
La roadmap funzionale D&D/Beyond rimane in [Stato software](STATO-SOFTWARE.md).

## Elenco chiuso dei difetti software

| Punto | Correzione |
| --- | --- |
| Borsello lento da modificare | Saldo digitabile, nove passi scelti direttamente, bozza separata e conferma dei cinque tagli insieme. |
| Monete poco distinguibili | Cinque glifi originali con metallo e lettera riconoscibili; stesso catalogo in elenco, modifica e anteprima. |
| Ritorno dalle impostazioni tasti | Indietro e BOOT lungo tornano alla pagina chiamante, anche aprendo Tasti dalle impostazioni del dispositivo. |
| Selettore dei comandi poco uniforme | Icone condivise, indicazione Attuale, paginazione leggibile e navigazione locale anche se la scorciatoia PWR è disattivata. |
| Collaudo denso e poco esplicito | Accessi Tocco/Tasti, conteggi osservati, lettura non disponibile distinta da zero; gesti misurati senza azioni di gioco. |
| Contatori da raggiungere in sequenza | Selettore diretto degli otto contatori, indice originale e quantità visibili; nessun utilizzo speso scegliendoli. |
| Tastierini numerici solo touch | PWR/BOOT per selezione, attivazione e annullamento nei tastierini esistenti di PF, danni e quantità. |
| Titolo comune abbreviato | Font adattato nello spazio centrale; intestazioni come Compagno restano complete e allineate. |

Le assegnazioni dei gesti restano quelle dell'utente durante gioco, consultazione
e annullamento. Scegliere un comando nel selettore tasti salva l'assegnazione,
come già previsto. «Spento» disattiva l'azione di quel gesto; PWR lungo resta
riservato all'alimentazione. Nel collaudo tasti i tre gesti vengono misurati:
per uscire si usa il tocco o si attende la scadenza, senza consumare BOOT lungo
come navigazione. Nel collaudo touch i tasti possono azionare solo Azzera/Indietro,
mai simulare il raggiungimento di un bersaglio.

## Revisione dell'insieme

Sono state riesaminate le prime catture native dei 153 ID pagina, in quattro
tavole, insieme agli scenari interattivi della suite. È una copertura delle viste
software; non equivale a una prova di ogni combinazione, all'approvazione del
risultato estetico o a un collaudo del display fisico. I dati sono sintetici.

| Percorso | Controlli software pertinenti | Riscontro fisico |
| --- | --- | --- |
| Home, menu e vitalità | Accessi rapidi, stato salvato, bozze numeriche, annullamento | Da eseguire |
| Tiri e risultati | Totali congelati, facce, critici contestuali, nessun rilancio consultando | Da eseguire |
| Attacchi e magie contestuali | Fonte originale, preparazione, costi e conferme | Da eseguire |
| Zaino, equipaggiamento e sintonia | Indici filtrati, effetti prima/dopo, dati mancanti | Da eseguire |
| Magie, slot e capacità | Preparazione, ordinari/patto, recuperi e limiti dichiarati | Da eseguire |
| Riposi e dadi vita | Anteprima, consumo, risultati congelati, salvataggio fallito | Da eseguire |
| Condizioni, morte e concentrazione | Distinzione nota/regola, guardie e conferme | Da eseguire |
| Effetti, difese e riduzioni | Ambito, durata, costi atomici e dettagli del calcolo | Da eseguire |
| Turni e tempo | Soggetti, reazione, scadenze e annullamento | Da eseguire |
| Contatori e borsello | Selezione diretta, massimi, saldo ignoto, bozza, riprova e undo | Da eseguire |
| Aspetto e raccolta | Nomi lunghi, conservazione, conferma e ritorno | Da eseguire |
| Profilo e registro | Ricerca, filtri, facce originali, tiri in attesa | Da eseguire |
| Rete, ponte e Beyond | Stati distinti, guida, consenso, blocchi diversi dall'HTTP | Da eseguire |
| Backup e ripristino | Ambito, preferenze, confronto, scadenza e conservazione | Da eseguire |
| Tasti, impostazioni e collaudo | Assegnazioni, chiamante, quarantena degli input e misure isolate | Da eseguire |

Il renderer reale è LVGL; Wi-Fi, USB e NVS nei test usano doppi di prova.
Le prove del filesystem SD sono sul filesystem host. Nessuna nuova lettura di
un PG reale o scrittura verso Beyond è dichiarata da questo ciclo grafico.
Le viste di dettaglio mantengono il testo utile a decidere; la revisione non
rimuove limiti del convertitore, regole manuali o conferme di salvataggio.

## Checklist sul Pocket

Le prove che consumano risorse vanno fatte con una scheda di prova.

- [ ] Aggiornare dal sito con Conserva dati: verificare PG, PF, CA, aspetto e
  assegnazioni dei tasti. I look sulla SD restano quelli presenti.
- [ ] Percorrere Home → Attacco → Lancia → Danni; controllare leggibilità del
  risultato, scelta delle varianti disponibili e feedback al rilascio.
- [ ] Aprire il borsello, digitare una bozza e annullare: saldo e altri tagli
  devono restare invariati. Controllare il confronto dei cinque tagli.
- [ ] Dalle impostazioni aprire Tasti, sfogliare un selettore e annullare.
  Verificare assegnazione invariata e ritorno alla pagina chiamante.
- [ ] Eseguire Collaudo → Tocco sui cinque bersagli e Collaudo → Tasti sui tre
  gesti, sempre rilasciandoli. Verificare la posizione reale dei tocchi e
  l'assenza di azioni di gioco. I conteggi non certificano da soli l'hardware.
- [ ] Valutare contrasto, dimensione dei caratteri, zone raggiungibili col
  pollice e fluidità sul display reale. Annotare pagina, azione e problema.

Questo è il giro di convalida rimasto per il restyle, non una nuova scaletta di
varianti grafiche. Batteria, interruzioni di alimentazione, accesso account
ESP32 e sincronizzazione Beyond hanno i collaudi funzionali separati già
indicati nella roadmap.


## Evidenza software della release

357 test JavaScript, suite nativa completa, GUI completa con 153 viste,
ASan/UBSan e build ESP32 passati. Il controllo dei glifi conferma invariati i
byte delle icone precedenti. Le tavole e il manifesto riproducibile sono
nell'archivio separato PocketDD-UI-Audit-v0.107.zip; il pacchetto PC include questo
resoconto e l'anteprima. Il manifesto dichiara esplicitamente dati sintetici,
commit sorgente, stato del repository e assenza di verifica hardware.
