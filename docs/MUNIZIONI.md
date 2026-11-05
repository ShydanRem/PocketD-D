# Munizioni nel flusso di attacco · v0.111

Dal Pocket: **Attacca → faretra → scegli una pila**. Il collegamento è esplicito:
nessuna arma acquisisce un consumo o un bonus dal nome. Usa questa funzione
solo quando l'arma richiede munizioni e la pila è adatta.

## Al tavolo

1. Con un'arma configurata a distanza, tocca la **faretra** in basso. I pulsanti
   della riga usano icone comuni; tienili premuti per leggerne il significato.
2. Scegli la pila dallo zaino: viene mostrato **−1 per attacco**, senza scrivere
   o consumare durante la scelta. Indietro conserva il collegamento precedente;
   **Senza consumo** torna esplicitamente alla gestione manuale.
3. Regola CA e vantaggio/contesto come prima, quindi **Lancia**. Una munizione e
   il d20 vengono confermati insieme, anche su mancato o 1 naturale.
4. Sul risultato vedi quantità prima/dopo. **Danni**, critico, componenti di True
   Strike e consultazione dello storico non consumano altre munizioni.
5. Con zero unità, il comando principale diventa **Munizioni**. Scegli un'altra
   pila, correggi la quantità nello zaino o disattiva esplicitamente il consumo;
   non viene scelta un'altra pila né fatto un tiro gratuito in automatico.

Il collegamento resta disponibile per gli attacchi successivi della stessa arma
nella sessione, compreso **True Strike 2024**, senza un nuovo menu a ogni colpo.
La quantità salvata segue scheda, backup e raccolta SD. Il collegamento stesso
è invece **temporaneo**: dopo un riavvio va riselezionato. È attivo per una sola
arma/pila alla volta; un cambio PG non eredita la scelta precedente.
La modalità manuale è il comportamento iniziale e non scrive NVS per il solo
tiro per colpire, come nelle versioni precedenti.

## Sicurezza del salvataggio

Se il costo non si salva, il risultato non viene annunciato: **Riprova** conserva
lo stesso d20 e la stessa proposta, senza ritirare o sommare spese. Durante
questa fase importazioni, backup e scorciatoie non sostituiscono la scheda.
Il tasto di ritorno non scarta il risultato in attesa. Il riavvio non recupera
un risultato ancora soltanto in RAM; non è dichiarata una transazione unica
resistente all'interruzione fra NVS e storico.

Un errore dello storico mantiene il tiro in attesa e la quantità già spesa.
La ripresa del registro non rimborsa né consuma di nuovo. Il successivo tiro
rimane bloccato finché il registro non è risolto. L'Annulla standard della scheda
ripristina la quantità precedente, conservando i tiri già annotati nel registro.
Cambiare la fonte durante una scelta invalida la proposta: nessun indice nuovo
viene accettato al posto di quello visto. Una pila riconfigurata o sostituita
richiede una nuova scelta, senza ricadere silenziosamente sul consumo manuale.

## Quali oggetti e quali regole

Sono selezionabili pile separate di oggetti generici o consumabili, anche vuote
per consultazione. Si escludono armi, oggetti curativi, dotazioni con proprietà
configurate e oggetti con cariche. L'ultima munizione lascia equipaggiata l'arma;
cariche, bonus ed effetti non vengono ricalcolati o recuperati dalla spesa.

La funzione gestisce **una unità per tiro per colpire**. Le quantità sono
quelle già importate/configurate; non c'è un nuovo riconoscimento Beyond delle
proprietà Ammunition/Loading, né un riconoscimento per nome che abilita regole.
Il selettore non certifica la compatibilità fra arco/balestra e pila scelta.

Restano manuali: bonus delle munizioni magiche, recupero dei proiettili dopo lo
scontro, ricarica, mani libere, limiti di azioni/attacchi, consumi diversi da uno,
armi lanciate o che generano munizioni e magie con componenti particolari.
La modalità Lancio dell'arma non consuma una copia dell'arma in questo aggiornamento.
Impugnature versatili/due mani, scudi, due armi e maestrie restano nella roadmap.

## Aggiornamento e verifiche

Aggiorna dal [sito](https://shydanrem.github.io/PocketD-D/) con **Conserva dati**.
Non servono reimportazione del PG, aggiornamento dei look SD o nuova estensione.
Nessun nuovo formato di scheda, backup o protocollo USB rispetto alla v0.110.

Prove sintetiche del motore: **4.800** combinazioni di indice, tipo, quantità,
dotazione, cariche e arma a distanza, con confronti dell'intero stato; nessuna
modifica dell'ingresso e nessun risultato parziale su errore. L'ultima unità e
la pila a zero passano la codifica/decodifica nativa.

Prove LVGL: selezione e annullamento senza scritture/RNG, costo su mancato,
critico e ultima unità, blocco a zero, riprova NVS con d20 fissato, guardie USB e
tasti, annullamento, tutte le sei caratteristiche di True Strike, storico in
errore, scelta superata, cambio PG e indice 59 con paginazione e nomi duplicati.
[Anteprima del renderer di produzione](previews/v111-ammunition.png), con dati
inventati e senza fotografie o prove fisiche del Pocket.

Compilazione ESP32, suite generale e sanitizzatori sono riportati nel
[resoconto software](STATO-SOFTWARE.md). Nessuna nuova lettura Beyond, scrittura
verso Beyond, accesso a una raccolta personale o prova USB/SD fisica in questa
versione. La persistenza dopo interruzione dell'alimentazione resta da collaudare.
