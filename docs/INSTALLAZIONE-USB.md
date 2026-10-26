# Installazione USB da browser — firmware v0.64

L'installer permette di mettere il firmware già compilato sulla scheda dal PC,
con una pagina, scelta della porta USB e avanzamento. Non richiede Arduino,
ESP-IDF o compilazione da parte di chi installa. Avatar, scene, icone e dadi
sono già nel firmware: non serve un caricamento separato su SD.

Il riferimento è il [web installer TamaPoke](https://github.com/socquique/TamaPoke/blob/main/web/README.md).
PocketD-D usa direttamente [esptool-js](https://github.com/espressif/esptool-js),
la libreria Espressif usata anche da ESP Web Tools, con interfaccia italiana e
senza comando di cancellazione completa. Le librerie sono incluse nel pacchetto,
non scaricate da CDN mentre si installa.

## Percorso rapido

1. Aprire l'[installer PocketD-D](https://pocketdd-installer.shydan.chatgpt.site)
   in **Chrome o Edge su PC**. Il Site è privato del proprietario e può richiedere
   accesso con il relativo account ChatGPT. Firefox, Safari e browser mobili
   non sono il percorso supportato.
2. Collegare un cavo **USB dati**. Chiudere monitor seriali, strumenti USB
   dell'app e altre pagine che usano la porta.
3. Scegliere **Prima installazione** oppure **Aggiorna PocketD-D** e verificare
   sulla scheda la sigla **Waveshare ESP32-S3-Touch-AMOLED-1.75 standard, senza C**.
4. Premere **Installa via USB**, scegliere la porta e attendere scrittura e
   verifica. Non scollegare il cavo durante l'operazione.
5. Controllare l'eroe sul display. Se il reset automatico non riesce, l'installer
   distingue firmware scritto/verificato da riavvio ancora da fare: riavviare
   con PWR secondo le istruzioni della revisione.

La prima installazione sostituisce il firmware presente. Prima di abbandonare
quello di fabbrica conservarne una copia o il recovery ufficiale come descritto
in [PRIMO-AVVIO.md](PRIMO-AVVIO.md). Non viene promessa la conservazione dei
salvataggi di un altro firmware.

**Aggiorna PocketD-D** legge la tabella partizioni del dispositivo e la confronta
con quella del pacchetto. Se differisce, interrompe prima di scrivere; non passa
automaticamente alla prima installazione. Una copia nativa dei progressi resta
consigliata prima dell'aggiornamento. Scheda, preferenze e storico PocketD-D
rimangono nelle aree NVS separate, escluse dalla scrittura del firmware.

Dopo l'installazione si trasferisce il personaggio con il percorso USB già
esistente, con conferma sul dispositivo. L'installer non esegue l'importazione
della scheda e non legge né modifica la raccolta PC o Beyond.

## Pacchetto locale e manutenzione

Il pacchetto `PocketD-D-installer-*.zip` include pagina, librerie, firmware,
licenze e launcher. Estrarre tutto. Su Windows aprire `Avvia-Windows.cmd`
(richiede **Python 3**); su macOS/Linux eseguire `python3 Avvia-PocketDD.py`.
Il launcher apre `http://127.0.0.1:8787/`: usare Chrome o Edge e lasciare aperta
la finestra. Il percorso locale funziona senza rete dopo l'estrazione.
Non aprire `index.html` direttamente dal file manager.

Nel repository, dopo la build firmware corrispondente:

```sh
npm ci
npm run installer:build
npm run installer
```

`installer:build` costruisce anche l'app PC e produce `dist/install/`. L'avvio
Node serve solo l'installer in loopback, senza servizi Beyond. Il pacchetto
completo contiene anch'esso `dist/install/` e i sorgenti di manutenzione.
Per preparare lo ZIP ridotto:

```sh
python3 scripts/package-installer.py /percorso/PocketD-D-installer.zip
```

`dist/install/` è statico e può essere ospitato su HTTPS, anche GitHub Pages.
Il Site distribuisce una copia esatta del build; un push su main di PocketD-D
non aggiorna da solo il firmware ospitato. Per una nuova versione ricostruire
firmware e installer insieme e ripubblicare la copia verificata.

## Controlli e limiti

Prima di aprire la porta, la pagina verifica dimensione e SHA-256 di tutti
i file. Il generatore verifica versione/target del build, layout e configurazione
flash, magic delle immagini e limiti dei settori; include le licenze di tutte
le dipendenze runtime. Sul dispositivo vengono controllati ESP32-S3 e 16 MB.
Questo **non identifica il modello o la revisione**: il controllo della sigla
rimane necessario. Un errore non viene presentato come installazione riuscita.

Si scrivono tre file separati: bootloader a `0x0`, tabella a `0x8000`, app a
`0x10000`. Nessuna immagine unica riempita fino all'app: il riempimento potrebbe
sovrascrivere NVS e calibrazione tra tabella e app. Sono esclusi `0x9000..0xffff`
e l'area di gioco da `0x410000`. Non vengono invocati erase-flash o eraseAll.
La libreria verifica l'MD5 di ciascuna regione dopo la scrittura. Il protocollo
è il bootloader USB Espressif, distinto dal trasferimento USB9 del personaggio.

Verificati sei test automatici del manifest/download/orchestrazione/server:
settori protetti, file alterati, chip/flash/layout errati, errori di connessione,
lettura/scrittura, verifica MD5 simulata, rilascio della porta e riavvio distinto.
La prova Chromium usa il codice compilato e copre file locali senza CDN,
conferma modello, annullamento del selettore USB, file corrotto/riprova,
browser non supportato e layout a 390 px. **Porta e scrittura sono simulate**:
non è un collaudo USB fisico. La verifica MD5 reale è della libreria; il suo
esito sull'hardware resta da provare.

La suite JavaScript complessiva passa 173 test. Il firmware v0.64 è lo stesso
binario già compilato e verificato su host, con 114 viste LVGL; questa aggiunta
non modifica firmware, partizioni, PDD28/PDDS, USB9 o backup. H32 nel
[COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md) resta **Non eseguito**.
