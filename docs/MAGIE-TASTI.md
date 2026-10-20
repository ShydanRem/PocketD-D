# Magie, capacità e tasti sul dispositivo — v0.22

Questo incremento porta gestione e consultazione delle risorse magiche nel
firmware offline. Il PC prepara importazione/configurazione; non serve al tavolo.
Nessuna dipendenza Foundry, nuova lettura Beyond o modifica della raccolta personale.

## Uso sul dispositivo

**Usi → Magie** apre l'elenco, tre righe per pagina, con filtri Magie/Capacità/Tutte.
Sono trasferite fino a **120 voci**, incluse quelle esaurite, non preparate o
informative. Il dettaglio espone livello, caratteristica di lancio, attacco/CD,
concentrazione, utilizzi e recupero dichiarato. **Testo** apre pagine leggibili.

**Lancia/Usa → Conferma** applica i soli costi ed effetti riconosciuti:

- Trucchetti senza consumo di slot.
- Magie con slot: scelta del livello disponibile, anche superiore, separando
  **slot ordinari e del patto**. Un solo slot della riserva scelta viene consumato.
- Magie innate: utilizzi separati; se il modello richiede anche uno slot, entrambi
  i costi vengono validati prima di cambiarne uno.
- Magie non preparate, utilizzi esauriti o slot insufficienti: pulsante disabilitato.
- Concentrazione unica: il nuovo incantesimo concentrato sostituisce quello
  precedente; gli altri non la interrompono. La conferma segnala la sostituzione.
- Ira riconosciuta: richiede PF positivi, bonus disponibile, un utilizzo e Ira
  non già attiva. Consuma l'uso, interrompe concentrazione, abilita i bonus
  pertinenti e impedisce il lancio di magie finché non termina.
- Resistenza Implacabile riconosciuta: solo a zero PF, dopo conferma/verifica DM;
  consuma un uso e porta a 1 PF. Il tracker completo di morte resta successivo.
- Capacità passive: consultabili, senza falsi pulsanti di consumo.

Costi, concentrazione ed effetti supportati sono salvati **nello stesso record**.
Un errore di scrittura mantiene lo stato visibile precedente; un errore di commit
può essere ambiguo, quindi richiede controllo dopo riavvio. **Eroe → Annulla**
ripristina l'ultima modifica di sessione. Un trucchetto che non cambia alcun dato
non scrive flash. Le azioni magiche non estraggono dadi né applicano danni/cure
arbitrari dal testo.

**Risorse** mostra slot, concentrazione e Ira. Concentrazione/Ira possono terminare
con conferma. Slot e utilizzi offrono correzioni manuali ±1 nei limiti importati;
la modalità breve/lungo/manuale del recupero è informativa. **Riposi automatici,
dadi vita e recuperi secondo regolamento sono il blocco successivo.** A zero PF
si interrompono concentrazione e Ira. Prove di concentrazione dopo danni, durata,
incapacità e altre condizioni restano da gestire con il DM.

## Tasti configurabili

Fonti esatte: [hardware reference ufficiale](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/HARDWARE_REFERENCE.md)
e ButtonTest/BSP dello stesso commit, scheda standard **senza suffisso C**.

| Comando | Collegamento | Impostazione iniziale |
| --- | --- | --- |
| PWR breve | TCA9554 `0x20`, P4/EXIO4 `SYS_OUT`, attivo alto | Eroe |
| BOOT breve | GPIO0, attivo basso, pull-up della scheda | Dadi liberi |
| BOOT lungo | Stesso GPIO0, almeno 800 ms, azione al rilascio | Magie |
| PWR lungo | Percorso alimentazione AXP2101 | Riservato all'alimentazione |

**Eroe → Aspetto → Tasti** cicla l'azione per ciascuno dei primi tre comandi:
Spento, Eroe, Scheda, Dadi, Zaino, Magie, Lancia. Le preferenze sono locali,
persistenti e conservate quando si importa una nuova scheda. Anche Annulla
mantiene le preferenze più recenti. Tutte le viste conservano l'accesso touch.

Lancia apre il dado libero se si è in un'altra vista; in Dadi avvia il tiro
selezionato o accelera quello in volo. Le prove bloccate restano bloccate.
Durante il volo le altre scorciatoie sono ignorate. **Nessun tasto conferma un
consumo, una fine effetto o un'importazione**: nelle conferme le scorciatoie
sono sospese e un tasto già tenuto non genera azioni dopo la chiusura.

Filtro antirimbalzo 30 ms, una sola azione al rilascio, niente ripetizione o
azione del tasto tenuto all'avvio. Errori di lettura/reconnessione richiedono
un nuovo rilascio stabile. Le pressioni PWR di almeno 800 ms non generano azioni
di gioco. BOOT mantiene la sua funzione elettrica di recupero se tenuto al reset.

Il driver condivide l'I²C già inizializzato; legge TCA9554 e imposta come ingresso
solo P4, preservando gli altri bit e la polarità. Nessun reset dell'expander,
nessuna scrittura ai registri di carica/voltaggio/spegnimento del PMIC. Un errore
PWR non blocca BOOT o il touch. Collegamenti e codice verificati sulle fonti;
**lettura reale e temporizzazioni dei tasti restano da collaudare sulla scheda**.

## Cosa rimane parziale

Le descrizioni sono proiettate in ASCII, spazi normalizzati, massimo **500
caratteri per voce**. Ogni testo più lungo viene segnalato per nome nell'anteprima
PC e marcato come **estratto** sul dispositivo. Il testo completo resta nella
raccolta PC. Nomi fino a 40 caratteri; nessuna voce viene eliminata per fare spazio.
Il firmware non ricava effetti da parole o nomi: soltanto gli effetti identificati
esplicitamente nel modello vengono applicati.

Attacco/CD sono valori consultabili; non c'è risoluzione automatica degli attacchi,
danni, cure, bersagli o salvezze delle magie. Non gestisce tutte le condizioni o
impedimenti al lancio. Preparazione completa per classe, rituali, componenti,
metamagia, effetti aggiuntivi, durate e interazioni speciali richiedono il DM.
Non è un motore completo D&D 2014/2024. Una magia lanciata non viene interpretata
come un risultato applicato al bersaglio.

## Formato, memoria e compatibilità

USB `pocketdd-device` **v5**, massimo **262144 byte**. Aggiunge `magic` ai campi v4:
`powers` (0–120), `slots` e `pact` (9 coppie `current`/`maximum` ciascuna, 0–9),
`concentration` (0 oppure indice voce + 1). Le preferenze tasti non arrivano dal PC.

Ogni voce contiene esattamente `name`, `description`, `kind`, `level`, `flags`,
`ability`, `maximum`, `remaining`, `recovery`, `effect`, `attackModifier`,
`saveModifier`. `kind`: 0 magia, 1 capacità. `flags`: concentrazione 1, preparata 2,
usa slot 4, utilizzi limitati 8, passiva 16, estratto 32. Livello 0–9,
caratteristica indice FOR/DES/COS/INT/SAG/CAR, modificatori −15…+30,
utilizzi 0–99, recupero manuale/breve/lungo = 0/1/2,
effetto nessuno/Ira/Resistenza Implacabile = 0/1/2.

Record **PDD5 a lunghezza variabile: 5168 + 552 × numero voci**, massimo **71408
byte**. CRC negli ultimi quattro byte effettivi. Profilo, formazione e inventario
mantengono gli offset v4; magia dall'offset 5122. Dopo disponibilità/conteggio/
concentrazione vengono 36 byte di slot, tre preferenze tasti e le voci da 552
byte, codificate esplicitamente senza dump di struct. Il buffer massimo non
viene scritto interamente per una scheda con poche magie.

Nuova partizione NVS **`pdd_state`, 256 KiB a 0x410000**. `pdd_nvs` a 0x9000,
PHY e app a 0x10000 da 4 MiB conservano indirizzi/dimensioni. Due banche alternate,
commit e rilettura prima di applicare lo stato. Nessun erase automatico. Le
vecchie copie **PDD1/PDD2/PDD3/PDD4** sono lette dalla partizione precedente in
sola lettura se quella nuova è vuota; il primo salvataggio va nell'area nuova.
I record precedenti restano intatti; magie mancanti segnalate, mai inventate.

**Aggiornare solo l'app con il layout v0.21 non basta**: senza `pdd_state`, il
firmware mostra la vecchia copia con “Layout da aggiornare” e blocca scritture.
Per l'aggiornamento occorre anche il layout distribuito, dopo backup della flash,
seguendo [FIRMWARE.md](FIRMWARE.md). Nessun flash eseguito qui. Un downgrade mostra
la copia precedente; non riporta automaticamente i progressi nuovi nel vecchio
formato. La raccolta PC resta separata e non viene sincronizzata dal dispositivo.

Stati grandi e linea USB in PSRAM; buffer temporanei nel heap, con errori gestiti.
Coda USB di puntatori con proprietà esplicita, buffer RX 4096 byte indipendente
dalla dimensione logica del pacchetto. Un errore di allocazione nella selezione
non permette di sovrascrivere una banca più recente con una copia vecchia.
Stack configurati main **16384** e USB **12288 byte**. Stime Xtensa statiche:
app_main 96, gestore azioni 112, parser 400, selezione/decodifica 64 byte per
funzione; non comprendono tutti i chiamati né sono misure del margine a runtime.
Il costo delle scritture cresce con il numero delle descrizioni; capacità,
latenza, usura e alimentazione vanno verificati sul dispositivo fisico.

## Verifiche

- **131 test JavaScript**, build PC e HTML autonomo.
- **10400 confronti PC → USB → C** su 51 schede sintetiche, incluse 120 voci,
  bonus/CD, slot e patto, preparazione, utilizzi innati, concentrazione e capacità.
- Restano passanti 1890 confronti delle prove e 1608 di inventario/combattimento.
- Test C per costi atomici, rifiuti senza mutazione, recuperi manuali, zero PF,
  codifica compatta e limite massimo; errori di allocazione senza perdita di stato.
- NVS simulata: migrazione golden prodotta dagli encoder reali v1–v4, nessuna
  riscrittura dei vecchi record al boot, layout mancante, CRC, banche e fallimenti.
- UI LVGL host: filtri, pagine, scelta ordinario/patto, anteprima/conferma/cancel,
  annullamento, concentrazione/Ira, slot/utilizzi, errori memoria, 120 voci,
  testi lunghi e scorciatoie. Pulsanti nel cerchio, altezza almeno 44 pixel.
- Tasti sintetici: rimbalzi, startup/riconnessione, blocco nelle conferme,
  breve/lungo, mancata ripetizione e wrap dell'orologio.
- ASan/UBSan: core/parser, NVS, magie, allocazioni, tasti e UI. Leak detector
  disattivato per l'istanza globale LVGL del test host.
- Chromium: raccolta effimera, anteprima di testo troncato, slot ordinari/patto,
  download e raccolta invariata. Nessuna lettura del personaggio personale.

Build ESP32-S3 con ESP-IDF v5.5.5 e LVGL 9.4.0. **Nessun test fisico o flash**:
display, touch, pulsanti, USB reale, heap/PSRAM, fluidità, NVS/power-cut e batteria
restano da provare. Anteprime riproducibili: `python3 scripts/preview-device.py`,
file `firmware/tests/out/ui-build/magic-preview.png`.

App compilata: **833.072 byte**, partizione da 4 MiB **80% libera**.
