# Pannelli fantasy, icone e feedback — v0.47

Questa revisione prosegue la base estetica approvata v0.45 e l'affinamento
scena/avatar della v0.46. Rende i menu più simili a un grimorio e aggiunge una
breve conferma visiva delle variazioni salvate.

## Pannelli e icone

Superfici scure con toni indaco e caldi, bordi bronzo e angoli pixel decorati
nei pulsanti larghi. Menta per l'azione principale, oro per la selezione;
stati premuto/disabilitato distinti. La scena e l'avatar restano quelli v0.46.

Lo zaino distingue armi, oggetti di cura, consumabili e altri oggetti con
spada, flacone, pacchetto e bisaccia. Le magie hanno una stella arcana e le
capacità una runa. Le icone seguono il tipo esplicito della voce: non vengono
dedotte dal nome né introducono scuole di magia, tipi di arma o altri effetti.

Le liste riservano spazio alle icone e mantengono quantità e indicatore
`*` dell'arma scelta. I nomi lunghi sono abbreviati; il dettaglio conserva il
nome completo. Testi di regole e conferme rimangono leggibili e separati dai
disegni. I controlli restano nel cerchio e alti almeno 44 px.

## Feedback delle modifiche

Per circa **due secondi**, sotto il marchio compare un messaggio con icona e
colore; tre piccoli pixel ornamentali si muovono se l'animazione è attiva.
L'errore di memoria mantiene la precedenza. Il pannello non intercetta tocchi,
non blocca la navigazione e non conferma un'azione.

Il messaggio confronta i valori prima e dopo l'operazione:

- `PF +N` o `PF -N` per la variazione effettiva dei PF;
- `Temp +N` / `Temp -N`, oppure PF e temporanei insieme;
- `Risorse recuperate`, `Risorse utilizzate` o `Risorse aggiornate` quando
  aumentano, diminuiscono o cambiano in entrambi i sensi pool indipendenti.

La variazione di PF/temporanei ha precedenza sul messaggio generico delle
risorse. I valori dei diversi pool non sono sommati tra loro. Il testo descrive
un saldo: una perdita di PF dovuta al termine di un effetto non viene trasformata
in un nuovo evento di danno o in un tiro.

Sono coperti pulsanti PF, contatori, uso di consumabili/cure e magie/capacità,
correzioni di slot/utilizzi, riposi, dadi vita, conferme del danno e della
salvezza COS, correzione e ricarica delle cariche. Le capacità sono osservate
anche senza il flag LIMITED usato dalle magie. Il feedback segue solo uno
stato applicato: niente messaggio di successo su errore di salvataggio o se i
saldi osservati non cambiano.

Una nuova azione sostituisce o rimuove il messaggio. Importazione e Annulla non
lo generano; l'importazione rimuove quello precedente. In **Aspetto → Animazione:
no**, il messaggio è statico e scade comunque. In attenuazione viene rimosso.
Il percorso dei dadi rimane quello esistente: risultato e risorse non dipendono
dal feedback, né vengono ripetuti alla sua scadenza.

## Implementazione e compatibilità

Messaggio, timer e oggetti sono transitori. Il confronto usa copie dei soli
saldi, senza leggere nomi o interpretare descrizioni. Il timer da 66 ms anima
solo gli ornamenti; in modalità statica si programma la sola scadenza. Prima
di pulire il root LVGL si elimina il timer. La scadenza elimina il pannello e
rende nuovamente visibile lo stato del salvataggio sottostante.

Nove icone originali 24×24 e due angoli 8×8 aggiungono **3360 byte** I4, palette
incluse: totale immagini collegate **131042 byte**, oltre a maschere avatar,
font e descrittori. Le immagini sono costanti in flash; timer, oggetti, decoder
e snapshot richiedono memoria a runtime. Non sono misure di RAM libera o consumi.

Nessun campo di stato o formato nuovo: PDD23, USB8, backup PC v11 e partizioni
invariati. PWR breve, BOOT breve e BOOT lungo mantengono le associazioni
configurabili; PWR lungo resta riservato all'alimentazione. Il PC rimane lo
strumento di preparazione; raccolta personale e dati Beyond non vengono toccati.
Il feedback non aggiunge chiamate RNG o salvataggi.

## Verifiche e anteprime

- Suite host completa e 41411 confronti sintetici PC → USB → C.
- UI: PF guadagnati/persi, assorbimento nei temporanei, salvataggio fallito,
  limiti senza variazioni, risorse, slot e capacità limitate dal tipo; messaggi
  senza scritture/RNG ulteriori, scadenza anche dopo il wrap dell'orologio.
- Animazione statica, timer/oggetti puliti a navigazione/importazione e in
  attenuazione; pannello non cliccabile, input esistenti conservati.
- Nomi/valori massimi, liste con icone, quantità/arma scelta, conferme e tasti;
  144 test JavaScript e build PC/HTML autonomo.
- ASan/UBSan UI passati con leak detector disattivato; compilazione ESP32-S3
  **994112 byte**, partizione app 4 MiB **76% libera**, ESP-IDF 5.5.5/LVGL 9.4.0.
  Nessun flash, collaudo fisico o misura di fluidità/consumi.

```sh
python3 scripts/generate-device-art.py --check
npm run firmware:test
python3 scripts/preview-device.py
```

Nell'archivio, `firmware-host-preview/fantasy-feedback-preview.png` mostra
cure/PF persi e liste; `feedback-preview.gif` mostra gli ornamenti e la scadenza.
`feedback-*.png` includono errore, temporanei e modalità statica. Sono catture
del firmware LVGL eseguito su host con dati inventati.

Restano transizioni tra pagine, ulteriori illustrazioni e icone per dati oggi
non rappresentati, oltre a leggibilità e fluidità sul display fisico. Batteria,
sospensione e collaudo dell'alimentazione restano nel relativo blocco di roadmap.
