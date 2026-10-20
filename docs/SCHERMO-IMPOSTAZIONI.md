# Schermo e preferenze locali — v0.25

> Documento della v0.25. La v0.26 aggiunge [effetti locali](EFFETTI-LOCALI.md)
> e PDD9; il comportamento dello schermo rimane invariato.

Il dispositivo aggiunge luminosità e attenuazione per inattività. Il PC rimane
lo strumento di importazione/configurazione/backup; nessun servizio serve al tavolo.
Questo blocco non implementa sospensione del microcontrollore o telemetria batteria.

## Uso

**Eroe → Aspetto → Schermo** apre le impostazioni:

- Luminosità da **10% a 100%**, con pulsanti −10/+10. Il minimo mantiene il pannello
  acceso e raggiungibile; non esiste un comando che lo renda nero.
- **Attenua: mai / 30 / 60 / 120 secondi**. La scelta iniziale e quella delle
  vecchie schede è **mai**, con luminosità **50%**.
- Da Tasti è possibile assegnare **Schermo** a PWR breve, BOOT breve o BOOT lungo.
  PWR lungo conserva la funzione hardware di alimentazione.

Dopo l'inattività il pannello passa al **10%**. L'animazione ambientale dell'avatar
smette di ridisegnarsi; l'app, il touch, i tasti, USB e i timer continuano a girare.
Con luminosità già al 10% non c'è attenuazione né un primo gesto da assorbire.

**Il primo contatto o tasto riattiva soltanto lo schermo.** La pressione intera
viene assorbita fino al rilascio; il gesto successivo può navigare o agire. Vale
anche per tasti assegnati a Spento. Il filtro evita che il risveglio consumi una
risorsa, lanci un dado o cambi pagina. Un errore di lettura del touch non viene
interpretato come rilascio del contatto di risveglio.

L'attenuazione è sospesa durante il volo dei dadi, le conferme e l'anteprima USB.
Un tocco mantenuto o un tasto premuto mantiene attivo lo schermo. L'arrivo di una
copia USB riattiva il display per la conferma. La sola ricezione di byte USB non
è una sospensione del timer; attenuare non interrompe il trasferimento.

## Salvataggio e guasti

Le preferenze sono **locali al dispositivo**, salvate dopo una modifica esplicita.
Importare una scheda conserva luminosità, timer, avatar animato e assegnazioni
locali. Anche Annulla di una modifica di gioco conserva le preferenze più recenti.
L'attenuazione e il risveglio non scrivono flash, non estraggono dadi e non
modificano la scheda. Se il salvataggio fallisce, resta il valore precedente.

Il driver applica il valore soltanto dopo un salvataggio riuscito. Un errore di
comando del pannello appare in Schermo; il firmware non dichiara il valore come
applicato e riprova al massimo una volta al secondo, senza riavviare o cancellare
memoria. A valore invariato non invia continuamente comandi SPI. I commit NVS
falliti mantengono la precedente avvertenza di esito ambiguo al riavvio.

## Driver e perimetro energia

Si usa l'API pubblica **esp_lcd_panel_co5300_set_brightness** del driver Espressif
fissato in firmware/dependencies.json. Il driver converte la percentuale nel
comando CO5300 0x51 e gestisce QSPI. Il task proprietario LVGL attende l'eventuale
trasferimento DMA pendente prima del comando. Nessun nuovo GPIO o controllo PMIC.

Non sono implementati spegnimento controllato, deep/light sleep, risveglio da
sospensione, misure SOC/voltaggio, percentuali batteria o stime di autonomia.
CPU e periferiche rimangono operative. L'effettiva luminosità, il consumo e il
comportamento elettrico vanno misurati sulla scheda; una build e i test host non
provano risparmi energetici o autonomia.

## Formati e compatibilità

Le copie USB rimangono **v7**: le nuove preferenze non sono campi trasferibili dal
PC. L'uploader e il formato backup PC v10 non cambiano.

Le nuove scritture sono **PDD8: 7917 + 552 × numero magie/capacità**, massimo
**74157 byte**. Due byte prima del CRC contengono luminosità 10–100 e timer 0–3.
La nuova assegnazione locale 9 è Schermo. Il layout flash e pdd_state da 256 KiB
restano quelli della v0.22; nessuna scrittura di migrazione al boot.

PDD1–PDD7 restano leggibili con 50%/mai. Il fixture PDD7 proviene dal vero encoder
del commit a39413c, prima delle modifiche, con dati sintetici. Preferenze fuori
limite, CRC e versioni future vengono rifiutati senza cancellare le banche.
Un downgrade alla v0.24 non riconosce PDD8 e blocca il caricamento per conservarlo;
non cancellare la NVS per aggirare il blocco. Nessun flash eseguito qui.

## Verifiche

- Regressioni: **138 test JS**, **37751 confronti PC/USB/C**, parser USB v1–v7,
  record e fixture PDD1–PDD7, I/O e allocazioni.
- Controller C: scadenze 30/60/120, disattivazione, wrap del timer, pressione
  mantenuta, quarantena di entrambi gli input, errori touch, blocchi durante
  operazioni, assenza di comandi ripetuti e riprova controllata.
- UI LVGL e vero ciclo di servizio dello schermo: impostazioni, errore NVS,
  import/cancel, preferenze conservate da Annulla, prima pressione assorbita e
  seconda operativa, dice/conferme protetti, errore del comando e recupero,
  scorciatoia Schermo; pulsanti nel cerchio alti almeno 44 pixel.
- Sanitizzatori e build ESP32-S3; immagini screen-settings/configured/error.png
  generate dal renderer host, non fotografate sull'AMOLED.

Nessuna nuova lettura personale Beyond o modifica della raccolta personale.
