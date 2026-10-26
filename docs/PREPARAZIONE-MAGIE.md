# Preparazione locale delle magie — v0.64

Il dispositivo permette di cambiare la preparazione delle magie già presenti
nella copia della scheda, senza tornare al PC. Si conserva la grafica approvata
e il funzionamento dei tasti configurabili.

## Uso sul dispositivo

Aprire **Menu → Magie**, selezionare la magia, quindi **Gestisci → Preparazione >**.
La conferma mostra nome, stato prima/dopo e risorse conservate. **Indietro**
scarta la proposta; **Conferma** salva il nuovo stato. Tornando al dettaglio,
una magia non preparata ha il lancio disabilitato. Prepararla rende possibile
il lancio soltanto se anche gli altri requisiti già rappresentati sono rispettati:
slot ordinari/patto, utilizzi innati, PF per la concentrazione e incompatibilità
con Ira.

Per le magie innate, **Gestisci → Utilizzi manuali** conserva le correzioni
precedenti. Le capacità mantengono **Utilizzi** nel dettaglio e non hanno un
comando di preparazione. Il nome lungo va a capo e la scelta tramite ricerca
continua a riferirsi alla voce originale, anche nell'ultima delle 120 posizioni.
Il filtro resta attivo tornando all'elenco.

## Che cosa significa preparare

È una **scelta esplicita di disponibilità locale**, basata sul flag di
preparazione già presente nella scheda. È offerta anche per trucchetti e magie
innate perché il modello attuale usa quel flag per la loro disponibilità.
Non equivale a un motore delle regole di preparazione di ogni classe.

Numero massimo, lista conosciuta, livello, momento consentito per cambiare
selezione, regole 2014/2024 ed eccezioni vanno verificati con il DM. Il comando
non insegna nuove magie, non cancella voci e non aggiunge automaticamente
incantesimi alla scheda. L'editor PC resta necessario per il catalogo importato
e le altre impostazioni.

Cambiare preparazione modifica **soltanto quel flag**:

- Slot ordinari e del patto, utilizzi innati e altre risorse mantengono il saldo.
- Non viene lanciata alcuna magia, estratto alcun dado o aggiunto un tiro allo storico.
- Una concentrazione in corso e i suoi effetti/difese/riduzioni collegati restano
  attivi, anche togliendo la preparazione alla magia concentrata. Terminarli
  richiede il percorso esplicito già disponibile.
- Riposi brevi e lunghi conservano la selezione; preparare una magia esaurita
  non le restituisce utilizzi o slot.

## Salvataggio, annullamento e copie

La modifica viene resa visibile dopo il salvataggio riuscito. Se la scrittura
fallisce, la conferma resta aperta con lo stato precedente e permette di
riprovare la stessa scelta. Come negli altri salvataggi NVS, un errore di commit
può essere ambiguo: controllare lo stato dopo un riavvio prima di proseguire.
**Menu → Eroe → Annulla** ripristina l'ultima modifica di gioco della sessione,
inclusa la preparazione. Una nuova modifica sostituisce il precedente Annulla.

Nella conferma sono bloccati scorciatoie fisiche, navigazione esterna,
importazione/ripristino e pausa automatica. Non serve tenere aperta una bozza
per conservare la selezione: quella confermata è già nel salvataggio.

Il flag esisteva già in PDD28/PDDS, USB9 e backup nativo v1: i formati non
cambiano. Il backup conserva la selezione; il ripristino confermato o una nuova
importazione adotta quella della copia scelta. Annullare conserva la selezione
corrente. Nessuna sincronizzazione automatica verso Beyond o verso la raccolta
PC. PDH2/lettura PDH1, backup PC v11, diagnostica v1 e partizioni sono invariati.

## Verifiche e limiti

Il core prova **3.840 combinazioni** di indice, tipo di costo, stato iniziale,
scelta finale e vincoli di concentrazione. Il confronto dell'intera scheda
verifica che cambi soltanto il flag. Sono coperti idempotenza, blocco del lancio,
costi normali dopo preparazione, rifiuti senza mutazione, riposi, record massimo
e backup nativo con 120 voci.

La UI host esercita **114/114 viste** e prova annullamento, errore/riprova del
salvataggio, Annulla, protezione dei comandi, lancio successivo, concentrazione
e legami conservati, magie innate/trucchetti, capacità escluse, nome lungo,
ricerca con indice originale 119 e importazione/ripristino annullati o confermati.
Esiti di suite completa, sanitizzatori e build in [FIRMWARE.md](FIRMWARE.md).

Le catture `prepare-*.png` e `spell-preparation-gallery.png` provengono dal
codice LVGL eseguito su host con dati inventati. Nessuna nuova lettura Beyond,
modifica della raccolta personale o prova sul dispositivo fisico. Il controllo
H31 e la procedura di avvio sono in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md)
e [PRIMO-AVVIO.md](PRIMO-AVVIO.md).
