# Ricariche parziali a dado — v0.43

Il dispositivo può ricaricare un pool di cariche con una formula scelta al
momento. La ricarica modifica solo il saldo dell'oggetto; quantità, PF, slot,
utilizzi, reazione ed effetti restano invariati. Funziona offline, senza Foundry.

## Uso sul dispositivo

Apri **Usi → Zaino → oggetto → Cariche → Ricarica**. Occorre un pool configurato,
quantità positiva e almeno una carica mancante. Con saldo pieno o quantità zero
la ricarica è bloccata prima di chiedere numeri casuali.

Scegli da **1 a 6 dadi**, d4/d6/d8/d10/d12/d20, e un bonus da **−15 a +60**.
I pulsanti −/+ della prima riga cambiano il numero di dadi; tocca la formula
per cambiare le facce. La seconda riga modifica il bonus. Il massimo teorico
per evento è 180 cariche; per recuperi fissi o maggiori usa la correzione manuale
del saldo già disponibile. La formula riparte da 1d6 +0 a ogni nuova ricarica:
non è una regola dedotta dall'oggetto o una configurazione salvata.

**Anteprima** mostra formula, saldo e spazio disponibile senza tirare o salvare.
Puoi tornare indietro e scartare la bozza. **Tira e salva** calcola un solo
candidato: recupero minimo zero, massimo limitato alle cariche mancanti.
Il risultato viene mostrato con i dadi pixel art dopo il salvataggio del saldo.
Accelera salta l'animazione senza cambiare dadi o risultato.

**Riepilogo** conserva nome completo, formula, somma dei dadi, recupero applicato,
saldo prima/dopo e parte eccedente non applicata. Un totale negativo mostra il
recupero minimo zero. Il riepilogo è di sessione, non uno storico persistente.
**Cariche** torna al saldo; **Fine** torna all'Eroe. **Annulla** sull'Eroe
ripristina l'intera ultima modifica salvata, compresa la ricarica.

Se il recupero applicato è zero, lo stato non cambia: nessuna scrittura flash
e nessuna sostituzione della modifica precedente annullabile. Il tiro viene
comunque mostrato; non viene rilanciato automaticamente.

## Errori e comandi

Se il salvataggio fallisce, il saldo corrente resta invariato. **Riprova** salva
lo stesso candidato, senza nuova estrazione casuale. Dopo il tiro la formula
e Indietro sono bloccati fino al salvataggio. Il risultato non viene presentato
come applicato prima della riuscita. La prova di Riprova usa un backend host
con errore temporaneo simulato. Il backend NVS attuale blocca le scritture dopo
un errore I/O e richiede il riavvio: Riprova non rimuove quel blocco. Il tiro
in RAM non è un'operazione in attesa persistente; dopo il riavvio fa fede il
saldo dell'ultima banca valida, anche se un commit era riuscito prima di un
errore successivo di rilettura.

Un'animazione non allocabile mostra
comunque l'esito già salvato, senza un secondo tiro.

Tasti configurabili e import USB sono bloccati durante configurazione, conferma
e riepilogo. Durante l'animazione i comandi ordinari restano disabilitati e
Accelera termina solo la presentazione. PWR lungo resta riservato all'hardware;
nessuna nuova assegnazione obbligatoria ai tasti.

## Regole ancora manuali

È una ricarica **esplicitamente autorizzata dal giocatore**, non un calendario.
Alba, tramonto, riposo, probabilità, numero di ricariche consentite e requisiti
dell'oggetto vanno verificati con il DM. Non si deducono eventi dal tempo di
gioco, dal nome, dalle annotazioni o dai dati Beyond. Non esiste ancora un
registro che impedisca di richiedere due ricariche nello stesso evento.

I recuperi pieni da riposo breve/lungo già configurati restano invariati.
La ricarica a dado può essere richiesta per qualunque pool presente e non pieno,
anche se ha un recupero da riposo: il giocatore decide quando è lecita. Non
consuma altri costi, non distrugge oggetti all'ultima carica e non applica
incantesimi o effetti dell'oggetto. La conversione Beyond non ricava ancora
automaticamente i pool di cariche o le loro formule.

## Compatibilità e verifiche

**PDD22, USB8, backup PC v11 e layout partizioni invariati.** Si salva solo il
saldo già rappresentato. Nessuna nuova migrazione, dato locale inventato o
riscrittura al boot. Il firmware v0.42 può leggere gli stessi saldi. Il PC
continua a configurare i pool; il nuovo percorso è interamente sul dispositivo.
Nessun ritorno automatico dei progressi al PC e nessuna raccolta personale toccata.

Tutte le prove nuove sono sintetiche, senza nuove letture Beyond o JSON personali:

- **103.680 casi C** su tutte le 60 posizioni, sei tipi di dado, 1–6 dadi,
  massimi 1/9/999, saldi vuoti/quasi pieni, bonus negativi/zero/massimo e facce
  minime/massime. Candidato completo confrontato con lo stato atteso.
- Zero minimo, limite al massimo, quantità/HP/altri pool esatti, input e puntatori
  invalidi rifiutati prima di RNG con output intatti; persistenza PDD22 del saldo.
- LVGL host: bozza/anteprima senza RNG o scritture, annullamento, salvataggio
  prima dei dadi, retry congelato, risultato eccedente, Annulla, zero senza
  scritture, saldo pieno, quantità zero, indice 59 e nome di 40 caratteri.
- Sei d20 +60, 180 cariche, limiti dei controlli, protezione da tasti/import,
  nessun rilancio dal risultato e fallback senza canvas. Bersagli nel cerchio.
- Suite host completa e 41.411 confronti PC → USB → C passati; 144 test
  JavaScript e build Vite/HTML autonomo. AddressSanitizer/UndefinedBehaviorSanitizer
  su ricariche e UI, con leak detector disattivato.
- Firmware ESP32-S3 compilato; misure in [FIRMWARE.md](FIRMWARE.md).

`firmware-host-preview/item-recharge-preview.png` e `recharge-*.png` nell'archivio
sono render del vero codice firmware su host con dati inventati. Nessun flash
o collaudo fisico di display/touch/tasti/USB/memoria/batteria/alimentazione.
