# Riposi e dadi vita sul dispositivo — v0.23

> Aggiornamento v0.30: [effetti collegati alla concentrazione](EFFETTI-CONCENTRAZIONE.md),
> fine/sostituzione congiunta e PDD12. I limiti e formati sotto sono storici.

> Documento dell’incremento v0.23. La v0.24 aggiunge
> [registro morte e condizioni](STATO-CONDIZIONI-MORTE.md), USB v7/PDD7 e il
> blocco del lungo a tre fallimenti/zero PF. Le omissioni v0.23 sotto sono storiche.

Il dispositivo gestisce riposi e cure con dadi vita offline. Il PC prepara e
trasferisce la scheda; Foundry, un server e Beyond non servono durante il gioco.
Questo incremento usa esclusivamente personaggi sintetici per le verifiche:
non ripete la lettura personale di Beyond e non modifica la raccolta personale.

## Uso

**Usi → Riposi** apre breve e lungo. **Eroe → Aspetto → Tasti** permette anche
di assegnare **Riposi** a PWR breve, BOOT breve o BOOT lungo. PWR lungo rimane
riservato all'alimentazione. I tasti non confermano, consumano o chiudono le
finestre di conferma; un'importazione USB durante una conferma di riposo/cura
riceve `busy`.

| Azione confermata | PF e stato | Risorse |
| --- | --- | --- |
| Breve | PF e temporanei invariati; termina Ira e attacco potente; concentrazione invariata | Tutti gli slot del patto e gli utilizzi dichiarati a recupero breve |
| Lungo | PF massimi, temporanei a zero; termina Ira, attacco potente e concentrazione | Slot ordinari/patto e utilizzi dichiarati a recupero breve o lungo |

Le capacità passive e gli utilizzi a recupero manuale restano invariati. Gli
utilizzi innati seguono il proprio recupero. Inventario, preparazione, CA,
condizioni fotografate e contatori generici, incluso **Slancio**, restano
invariati. Lo Slancio della demo PC conserva il suo recupero dimostrativo sul
PC; sul dispositivo è esplicitamente un contatore manuale.

L'anteprima indica PF/temporanei prima e dopo, numero di slot restituiti per
riserva, voci con utilizzi recuperati, dadi vita recuperati ed effetti terminati.
**Indietro** non modifica la scheda. **Conferma** salva insieme tutti i recuperi.
Un riposo che non cambia dati evita una scrittura flash. **Eroe → Annulla**
ripristina l'ultima modifica salvata della sessione, non una serie di operazioni.

## Dadi vita

La copia PC trasferisce fino a **20 riserve per classe**, ciascuna con nome,
d6/d8/d10/d12, massimo e dadi spesi. Non ricava dadi da livello, caratteristiche
o descrizioni. Dati assenti sono segnalati; una lista vuota non inventa riserve.
I nomi sono traslitterati in ASCII e limitati a 40 caratteri; gli originali
restano sul PC. Un nome sorgente vuoto appare come “Classe senza nome”.

Dopo aver confermato il breve, **Spendi un dado vita** apre le riserve.
**Spendi 1 dado → Conferma** estrae un solo dado e applica la cura base
**max(0, dado + modificatore COS)**, limitata ai PF massimi. I temporanei restano
invariati. Il dado viene speso anche con cura zero. A PF pieni, a zero PF o con
riserva esaurita non viene estratta entropia né consumato un dado: le eccezioni
si risolvono con il DM. Una cura fuori da un breve confermato non è disponibile.

PF e dado speso sono salvati insieme, quindi viene mostrata l'animazione pixel
art con faccia, totale e PF effettivamente recuperati. Accelerare il volo non
rilancia e non applica nuovamente la cura. **Altro** torna alla scelta, **Riposo**
al breve e **Fine** termina la schermata del riposo. Per spendere un altro dado
serve una nuova conferma. La cura può essere annullata insieme al consumo.

### Recupero lungo dei dadi vita

La scelta riguarda **il recupero dei dadi vita**, non l'intero regolamento:

- **Manuale**, scelta iniziale a ogni nuovo lungo: nessun dado restituito.
- **2014**: limite pari a metà della somma dei massimi delle riserve, arrotondata
  per difetto, minimo uno se ci sono dadi; mai oltre i dadi effettivamente spesi.
  Il giocatore sceglie quanti restituire a ciascuna classe. Il firmware impedisce
  di superare il limite o i dadi spesi della riserva. Nessuna scelta multiclass
  viene fatta in silenzio; è possibile confermare un recupero inferiore al limite.
- **2024**: restituisce tutti i dadi spesi delle riserve trasferite.

Cambiare criterio azzera la selezione 2014. L'anteprima espone criterio e numero
effettivo prima della conferma. Il firmware conserva i valori della scheda,
senza correggere automaticamente il totale dei dadi rispetto al livello.

## Limiti dichiarati

Questi sono recuperi espliciti confermati dal giocatore, non un arbitro delle
regole: durata, interruzioni, limiti nelle 24 ore, requisiti dei PF all'inizio,
sonno, eccezioni di classe e validità del riposo restano da verificare con il DM.
La selezione 2014/2024 non cambia automaticamente le capacità della scheda.
Bonus alle cure, talenti, oggetti, dadi speciali e altre regole opzionali non
sono dedotti dal testo. Non c'è un cronometro del riposo.

Non sono ancora rappresentati sul dispositivo tiri contro morte, condizioni
complete/livelli di esaurimento, monete e relativi effetti. Il lungo non simula
il loro recupero. Non sono disponibili creazione/modifica delle riserve sul
display, sincronizzazione di ritorno al PC o storico persistente dei risultati.
La raccolta PC mantiene tutti i dati originali e le schede esistenti.

Un errore di scrittura conserva lo stato visibile. Finché la conferma resta
aperta, un tentativo di salvataggio ripetuto riusa la stessa cura già estratta.
Un commit fallito può comunque essere ambiguo: controllare lo stato dopo il
riavvio. In caso di spegnimento dopo il salvataggio e prima dell'animazione,
restano PF/consumo, mentre la faccia del dado non è nello storico NVS.

## Formato e compatibilità

**USB v6**, limite invariato di 262144 byte. Aggiunge il campo obbligatorio:

```json
"recovery": {
  "available": true,
  "pools": [{"name": "Classe sintetica", "die": 8, "maximum": 3, "used": 1}]
}
```

Chiavi esatte, nessun duplicato, numeri interi, massimo 20 riserve. `maximum`
1–20, `used` 0–maximum; con `available:false` la lista deve essere vuota.
Il parser accetta USB v1–v5 senza inventare dadi vita. L'uploader negozia
`PDD READY 1`–`6` e non invia una copia v6 a firmware precedenti. Usare lo
script distribuito con questa versione. Backup PC v10 e raccolta non cambiano.

Le nuove scritture sono **PDD6: 6050 + 552 × numero magie/capacità**, massimo
**72290 byte**. Il prefisso PDD5 mantiene gli stessi campi; prima del CRC sono
aggiunti due byte disponibilità/conteggio e 20 riserve da 44 byte. Le preferenze
tasti restano locali; il nuovo valore 7 è Riposi.

La partizione `pdd_state` da 256 KiB e il layout v0.22 non cambiano. PDD1–PDD5
restano leggibili; la migrazione non scrive al boot. Il fixture PDD5 è prodotto
dal vero encoder del commit `a6c525f`, prima di modificarlo, con soli dati
sintetici. La vecchia `pdd_nvs` rimane in sola lettura.

**Downgrade:** la v0.22 non riconosce PDD6 e blocca il caricamento quando incontra
un record nuovo valido. Conservare backup privati e usare firmware aggiornato;
non cancellare la NVS per aggirare l'incompatibilità. Nessun flash eseguito qui.

## Verifiche

- **134 test JavaScript**, build PC e HTML autonomo.
- **7840 nuovi confronti PC → esportazione USB → parser reale → C**, su 80
  schede sintetiche, fino a 20 riserve, tutte le facce di d6/d8/d10/d12,
  COS 1–30, PF zero/pieni, esaurimento e criteri espliciti. Totale con le
  regressioni di prove, combattimento e magie: **21738 confronti**.
- Schema e CRC: recuperi malformati rifiutati senza alterare gli output;
  migrazione da fixture indipendenti PDD1–PDD5, salvataggio/rilettura delle
  20 riserve, guasti di allocazione e versioni future.
- UI LVGL reale su host: anteprima/annullamento, breve/lungo, selezione
  multiclass, criterio manuale/2014/2024, cura/cap/annulla, errori e tentativi
  senza nuova entropia, scorciatoie protette, import `busy`, record mancanti,
  ultima delle 20 riserve e aree touch nel cerchio alte almeno 44 px.
- Chromium su raccolta effimera: anteprima dadi vita, copia scaricata v6,
  valori delle riserve e raccolta invariata; nessun errore JavaScript.
- AddressSanitizer/UndefinedBehaviorSanitizer e build ESP32-S3.

Le anteprime `firmware-host-preview/rest-preview.png` nell'archivio provengono
dal renderer LVGL su host. Display, touch, pulsanti elettrici, USB fisico,
alimentazione, prestazioni e recupero da interruzione reale restano da collaudare.
