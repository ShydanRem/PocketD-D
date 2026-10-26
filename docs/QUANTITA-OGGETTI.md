# Quantità degli oggetti sul dispositivo — v0.65

Lo zaino permette di correggere il saldo di una voce già presente, per esempio
aggiungere pozioni trovate o annotare torce cedute, senza passare dal PC.
La modifica è manuale e separata dall'uso dell'oggetto.

## Uso

Aprire **Zaino → oggetto → Quantita >**. La tastiera parte dal saldo corrente;
la prima cifra lo sostituisce. Sono ammessi valori **0–999**; cifre che superano
il limite vengono ignorate. Azzera riparte da zero, Canc. rimuove l'ultima cifra.

**Anteprima** mostra nome completo, prima/dopo e risorse conservate.
**Indietro** torna alla tastiera mantenendo il numero; un secondo Indietro
scarta la bozza e torna al dettaglio. **Conferma** salva il saldo e torna
all'oggetto. Se il valore coincide con quello corrente, la conferma è disabilitata:
nessuna scrittura e nessuna sostituzione dell'ultima modifica annullabile.

**Menu → Eroe → Annulla** ripristina l'intera ultima modifica di gioco della
sessione. Un salvataggio fallito conserva saldo precedente e anteprima per
riprovare lo stesso numero. Come per ogni commit NVS, un errore reale può
avere esito ambiguo e richiede controllo dopo il riavvio.

## Quantità, cariche e arma scelta

Il comando cambia soltanto la quantità, con un'eccezione necessaria:
**zero sull'arma scelta la ripone e azzera la modalità lancio** nello stesso
salvataggio. Non può rimanere equipaggiata un'arma con quantità zero.
Se la quantità torna positiva, l'arma va scelta nuovamente con Scegli.
Il colpo preparato viene invalidato dopo una modifica riuscita.

- PF, temporanei, monete, concentrazione ed effetti restano invariati.
- Non viene usata una pozione, estratto un dado o aggiunto un tiro allo storico.
- Le cariche mantengono saldo, massimo e recupero, anche portando quantità a
  zero e poi aumentandola. Le cariche restano un solo pool per voce, distinto
  dal numero di oggetti: aggiungere una unità non crea un pool aggiuntivo.
- Una riga a zero resta consultabile, ricercabile e modificabile. Non viene
  eliminata o riordinata; utilizzi e costi che richiedono quantità positiva
  restano bloccati finché il saldo non torna disponibile.
- Modificare un altro oggetto non ripone l'arma scelta. CA e PF massimi non
  vengono ricalcolati dalla quantità o dall'equipaggiamento.

La tastiera e l'anteprima sono protette da tasti configurabili, importazione,
ripristino e pausa automatica. I filtri di ricerca e gli indici originali sono
conservati, anche sulla voce 59 con nome lungo. L'importazione confermata
adotta le quantità della nuova copia PC; il ripristino nativo adotta quelle
del backup. Annullare questi percorsi conserva il saldo corrente.

## Perimetro

È possibile correggere **voci già presenti**, inclusi gli oggetti generici,
consumabili, cure e armi. Creazione/rinomina di una nuova voce, prezzi,
compravendita, peso/ingombro e consumo automatico delle munizioni restano
successivi. Il saldo dichiarato va verificato con il DM. Non sono dedotti
effetti o proprietà dal nome e non viene modificata la raccolta personale PC.

PDD28/PDDS, PDH2/lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni sono invariati. Il saldo era già nel record;
nessun campo persistente aggiunto. La v0.65 richiede il nuovo firmware per
questo comando; trasferimento e backup dal browser restano compatibili.

## Verifiche

**40.320 casi core** con tutti i 60 indici, quattro tipi di oggetto, quantità
iniziali/finali, equipaggiamento e cariche. Confronto dell'intera scheda
attesa, input fuori limite, idempotenza, assenza di ricarica/effetti, blocchi
su arma a zero, rifiuti atomici e persistenza tramite record e backup nativo.

La UI host esercita **115/115 viste**: tastiera e limiti, annullamento,
conferma invariata senza scritture, errore/riprova/Annulla, risorse conservate,
zero e ritorno dell'arma, indice originale 59 filtrato, nome lungo e
import/ripristino annullati o confermati. Suite completa, sanitizzatori e
build sono riportati in [FIRMWARE.md](FIRMWARE.md).

Le catture `quantity-*.png` e `inventory-quantity-gallery.png` sono del codice
LVGL su host con dati inventati. Nessuna prova fisica o nuova lettura Beyond.
H34 in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md) resta **Non eseguito**.
