# Tabaxi, Kenku, Aarakocra e Tortle — v0.68

Il catalogo passa da 12 a **16 famiglie visive**, con **32 silhouette** maschili e
femminili. Le quattro nuove famiglie hanno anatomie dedicate, tre forme della
testa e le quattro pose riposo/battito/cura/danno. Sono 96 nuovi frame, per un
totale di 384 comprendendo l'atlante umano originale.

| Famiglia | Tratti visivi | Personalizzazione |
| --- | --- | --- |
| Tabaxi | Muso felino, orecchie, macchie e coda | Pelliccia oro/argento/cenere; pelo corto, gorgiera o liscio |
| Kenku | Testa e becco da corvo, piume scure, piedi artigliati | Piumaggio corvino/ardesia/prugna; ciuffo, cresta o testa liscia |
| Aarakocra | Testa d'aquila, becco dorato, ali ripiegate e artigli | Piumaggio avorio/ghiaccio/sabbia; ciuffo, ventaglio o testa liscia |
| Tortle | Testa e pelle da tartaruga, guscio segmentato visibile | Squame oliva/salvia/sabbia; testa liscia, scuti o placche |

Entrambe le silhouette hanno lo stesso abbigliamento pratico. Le differenze sono
sobrie e riguardano proporzioni, testa ed espressioni; uccelli e rettili non hanno
anatomia mammifera. Il Tortle mantiene il guscio visibile dietro il mantello.
I Tabaxi femminili mantengono muso e pelliccia felini in tutti i tagli.

## Dispositivo, PC e Beyond

Sul dispositivo si usa **avatar → Aspetto → Modifica → Specie**. **Altre** scorre
sei gruppi; l'ultimo contiene solo Tortle. Le frecce, l'anteprima, la conferma e i
tasti fisici continuano a funzionare. I campi diventano Pelliccia, Piumaggio o
Squame; i capelli sono esclusi per queste famiglie e il taglio diventa una forma
della testa. Il colore del mantello resta indipendente.

Sul PC le stesse scelte sono disponibili in Personalizza eroe. I nomi esatti
Tabaxi, Kenku, Aarakocra e Tortle vengono riconosciuti nell'anteprima di Beyond.
La silhouette deriva soltanto da un genere esplicito riconosciuto; resta
modificabile. Se manca, l'anteprima dichiara la scelta provvisoria. Le specie
sconosciute/homebrew e le corrispondenze ambigue rimangono manuali.

La conferma aggiunge una nuova scheda; l'anteprima e le modifiche estetiche non
riscrivono le altre schede. Il riconoscimento visivo non certifica la conversione
di CA, PF, competenze o capacità: i controlli meccanici e i loro blocchi rimangono
indipendenti. Le nuove illustrazioni non introducono volo, armatura naturale,
attacchi naturali o altri effetti di regole.

## Memoria e conservazione della grafica approvata

L'atlante usa ora **Huffman sui flussi PackBits/XOR**. Le 372 nuove immagini I8
occupano 454.108 byte, invece dei 539.420 richiesti da PackBits/XOR da solo:
**85.312 byte risparmiati**, senza eliminare pixel. Gli altri dodici frame umani
restano nel loro atlante precedente.

La decodifica legge i simboli direttamente nel buffer già esistente: nessun
buffer grafico aggiuntivo, nessuna allocazione per fotogramma. La piccola tabella
Huffman resta in flash. Le maschere e tutti i colori usati dai primi dodici set
sono identici alla v0.67, verificati con impronte e palette di riferimento del
commit `cee8125`. L'estensione utilizza soltanto gli indici di palette ancora
liberi per i nuovi colori fissi.

Le sorgenti PNG trasparenti restano in `firmware/assets/species-source/`, con
hash e provenienza. Il generatore campiona le nuove tavole a 48×60, preserva le
maschere esistenti e produce gli stessi dati per C e JS. I file originali non
vengono ridisegnati da script.

La build ESP32-S3 occupa 4.020.352 byte: restano 173.952 byte, circa il 4% della partizione applicazione da 4 MiB. Le due aree NVS non vengono ingrandite. Il margine
rimane limitato: prima di espandere ulteriormente il catalogo vanno rivalutati
compressione e pacchetti grafici. Il conteggio delle famiglie ora deriva da un
piccolo header generato; i gruppi incompleti non espongono indici fuori catalogo.

## Formati

| Percorso | Versione |
| --- | --- |
| Nuove quattro famiglie nel backup PC | v13 |
| Famiglie precedenti nel backup PC | v12; v11 per l'umano maschile originale |
| Nuove quattro famiglie al dispositivo | USB11, firmware 0.68 o successivo |
| Famiglie precedenti al dispositivo | USB10; USB9 per l'umano maschile originale |
| Salvataggio nativo nuovo | PDD31/PDDV |
| Backup nativo | Involucro v1, validatori C/JS/Python aggiornati |

PDD31 ha le stesse dimensioni e posizioni di PDD30: 8.945 + 552 × capacità byte,
massimo 75.185. Il nuovo tag dichiara il catalogo esteso. PDD30 e USB10 continuano
a consentire solo gli ID originali 0–11: una nuova specie non viene nascosta in
un formato vecchio. Il PC richiede un aggiornamento del firmware prima di inviare
USB11 a un dispositivo USB10. Un vecchio client deve essere aggiornato prima di
leggere i nuovi backup.

La lettura conserva tutti i formati precedenti; PDD30 mantiene specie e corpo,
PDD29 mantiene la scena e assume il precedente umano, PDD28 e precedenti usano
anche la scena predefinita. Non avviene alcuna scrittura durante la migrazione.
Importare un avatar conserva luminosità, sfondo, animazione e tasti locali;
ripristinare il backup nativo recupera anche l'aspetto salvato.

## Verifica

**203 test JavaScript passati**, suite nativa completa e 118/118 viste LVGL. UI, renderer e codec aspetto passano anche AddressSanitizer/UBSan. Il backup C → PC → C copre 47 casi.

I controlli comprendono i 10.368 confronti di buffer C/JS per 16 famiglie, due
corpi, 81 combinazioni di palette/taglio e quattro pose; le impronte di tutti i
frame approvati precedenti; i codec con tutte le 96 combinazioni
specie/corpo/scena; la navigazione dell'ultimo gruppo con una sola voce; anteprima,
annullamento, errori di salvataggio, importazione e backup.

Il browser usa un connettore sintetico per riconoscere Tabaxi, cambiare a Tortle,
creare una nuova scheda senza alterare le precedenti, ricaricarla ed esportare
USB11. Editor, campi senza capelli, backup, conflitti e raccolta piena restano
verificati. La galleria usa le 32 home catturate dal renderer di produzione LVGL
eseguito su PC, non immagini concettuali al posto delle schermate.

**Non è stata eseguita una nuova lettura reale di Beyond né una prova fisica sul
dispositivo dell'utente.** Il problema di riavvio/importazione USB rimane aperto
per il collaudo hardware. La build ESP32-S3 conferma la compilabilità e il budget
di flash, non la fluidità o il comportamento dei tasti fisici reali.

Il prossimo gruppo ancora da disegnare comprende Goblin, Hobgoblin, Bugbear,
Kobold e Lizardfolk. Aspetti di gatti, uccelli e tartarughe ulteriori non vengono
spacciati per famiglie già disegnate: Tabaxi è per ora un felino maculato,
Aarakocra un'aquila e Kenku un corvo.
