# Alimentazione, batteria e pausa — v0.62

Il firmware integra lettura AXP2101, indicatore nella home, avvisi di carica
bassa, pausa manuale/automatica e spegnimento software con salvataggio.
Destinazione: **ESP32-S3-Touch-AMOLED-1.75 standard**, senza suffisso C.
Le funzioni sono implementate e compilate; avvio a batteria, risveglio,
arresto, calibrazione della stima e consumi **non sono ancora collaudati**.

## Indicatore e pagina Alimentazione

La home mostra una piccola batteria e la percentuale stimata. Se la percentuale
non è disponibile, mostra **USB** quando il PMIC rileva alimentazione USB
valida, altrimenti **?**. È un indicatore in sola lettura.

Aprire **Aspetto → Schermo → Alimentazione**, oppure usare il tasto configurato
su Schermo e aprire Alimentazione. La pagina mostra:

- presenza della batteria e **stima percentuale del PMIC**;
- ricarica in corso e alimentazione USB valida rilevata;
- tensione della batteria, se la misura ADC è già abilitata e valida;
- avviso basso/critico, eventuali errori e stato della pausa automatica.

Il valore non viene ricavato da una curva tensione/percentuale inventata.
Non viene promessa un'autonomia residua. La lettura avviene ogni due secondi;
i dati più vecchi di dieci secondi sono considerati indisponibili. Un errore
di comunicazione obbligatoria o un chip diverso rende il campione sconosciuto.
Una misura opzionale non valida nasconde soltanto quella misura. Gauge o ADC
disabilitati non vengono accesi modificando la configurazione di fabbrica.

Il PMIC può richiedere apprendimento/calibrazione della batteria: anche un
numero valido è una stima da confrontare con il comportamento fisico. Batteria
assente, lettura fallita e percentuale sconosciuta rimangono stati distinti;
nessun caso viene trasformato in un finto 100%.

## Avvisi

A batteria, senza USB valida e senza ricarica rilevata:

| Stima | Indicazione |
| --- | --- |
| 6–20% | Batteria bassa, colore oro e invito a collegare USB |
| 0–5% | Quasi scarica, colore rosso e invito a collegare USB |

L'indicatore e la pagina si aggiornano; le viste interne mostrano l'avviso
nella riga di stato. Errori di memoria e tiri non registrati mantengono la
precedenza. L'avviso non chiude conferme, non modifica la scheda e non estrae
dadi. Non comanda un taglio automatico dell'alimentazione: i progressi normali
sono già salvati dalle singole azioni, mentre le operazioni pendenti restano
esplicite. Le protezioni elettriche del PMIC non sono riprogrammate.

## Pausa manuale e automatica

**Pausa** spegne il pannello e ferma i timer di presentazione LVGL. Mantiene
scheda, pagina, bozze, risultato corrente e Annulla in RAM, senza scrivere NVS.
Touch, BOOT o PWR risvegliano il dispositivo; il gesto di risveglio viene
consumato fino al rilascio, senza eseguire la scorciatoia o il pulsante sotto
il dito. La luminosità scelta viene ripristinata.

Solo con batteria rilevata e USB assente si esegue anche **light sleep della
CPU**, a intervalli di 20 ms. GPIO0/BOOT e GPIO11/touch sono sorgenti di
risveglio; PWR, collegato all'espansore, viene controllato fra gli intervalli.
I trasferimenti DMA vengono drenati prima dei comandi al pannello e prima
del sonno. La sorgente SAR di entropia viene disabilitata durante il light
sleep e riabilitata prima di consentire altri tiri. Non viene usato deep sleep.
Precisione, latenza e risparmio effettivo richiedono misure sulla scheda.

Con USB o alimentazione sconosciuta, la pausa spegne il pannello e sospende
il rendering mantenendo la CPU e il trasporto USB attivi. Se l'USB viene
collegata durante una pausa iniziata a batteria, lo schermo si risveglia.
Durante la pausa importazioni e ripristini ricevono Busy; risvegliare prima
di trasferire una scheda. La diagnostica in sola lettura rimane disponibile.

Con **Schermo → Attenua** impostato a 30/60/120 secondi, cinque minuti di
inattività a batteria attivano automaticamente la stessa pausa. **Attenua:
mai** disabilita anche la pausa automatica. La logica funziona anche con
luminosità già al 10%. Non si aggiunge una nuova preferenza persistente.

Pausa manuale e automatica sono bloccate durante import/conferme/collaudo
comandi, dadi in volo, storico da registrare o problemi di memoria/stato.
I turni e le durate di gioco non avanzano da soli. Un errore entrando in pausa
lascia visibile la pagina; un errore del light sleep tenta il ritorno alla
pagina. Se il pannello non si riaccende, gli input di gioco restano bloccati
e il firmware ritenta il risveglio. I limiti del driver fisico vanno collaudati.

## Spegnimento controllato

**Spegni** è disponibile con batteria rilevata e USB scollegata. La conferma
ricorda che bozze e Annulla della sessione andranno persi. **Salva e spegni**:

1. aggiorna la lettura di alimentazione e verifica stato/storico coerenti;
2. salva la scheda corrente, poi lo storico;
3. solo dopo entrambi i salvataggi richiede al PMIC il comando Soft PWROFF.

Un salvataggio fallito impedisce qualsiasi richiesta al PMIC. Con un tiro
non registrato, la pagina offre **Apri Storico** per risolverlo. Il PMIC viene
ricontrollato immediatamente prima della scrittura; anche una connessione USB
attiva impedisce la richiesta nella funzione hardware.

La UI distingue **Spegnimento richiesto** da **Esito non confermato**. Una
scrittura I2C riuscita non prova da sola che l'alimentazione sia cessata. Dopo
il tentativo non si ripetono scritture o azioni di gioco: se la scheda resta
accesa, usare il comando PWR hardware e annotare l'anomalia. Alla riaccensione
si devono ritrovare scheda, storico e preferenze salvati. L'effettivo arresto
e la successiva accensione con PWR sono prove fisiche ancora da eseguire.

PWR lungo mantiene la funzione elettrica già prevista dalla scheda; le tre
scorciatoie PWR breve/BOOT breve/BOOT lungo rimangono configurabili. Il firmware
non intercetta il taglio hardware né può garantire il completamento di una
scrittura interrotta da esso. Salvataggi e storico rimangono due commit distinti.

## Fonti hardware e registri

Verificati schema, esempio ESP-IDF e datasheet inclusi nel repository ufficiale
Waveshare al commit `e4344e70c2fa78a13e8a06566507f1ba8af6672a`:

- [Schema standard](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/Schematic/ESP32-S3-Touch-AMOLED-1.75-schematic.pdf).
- [Esempio e configurazione PMIC](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/esp-idf/01_AXP2101/main/port_axp2101.cpp).
- [Driver XPowers di riferimento](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/esp-idf/01_AXP2101/components/XPowersLib/src/XPowersAXP2101.tpp).
- [Datasheet AXP2101](https://github.com/waveshareteam/ESP32-S3-Touch-AMOLED-1.75/blob/e4344e70c2fa78a13e8a06566507f1ba8af6672a/examples/arduino/libraries/XPowersLib/datasheet/AXP2101_Datasheet_V1.0_en.pdf).

Il PMIC è all'indirizzo I2C `0x34` sul bus già usato dal touch, SDA15/SCL14.
Si verifica ID `0x4A` al registro `0x03`. Letture: stato `0x00–01`, gauge
abilitato `0x18`, ADC abilitato `0x30`, tensione `0x34–35` (1 mV per passo),
percentuale `0xA4`. La direzione di carica usa i bit 6:5, ignorando il bit 7.
La tensione visualizzata è accettata nell'intervallo 2–5 V: è un controllo
di plausibilità del dato, non una soglia elettrica o di ricarica.

L'unica scrittura PMIC introdotta è **registro `0x10`, bit 0 Soft PWROFF**,
su conferma; i bit di configurazione sono preservati e il bit comando Reset
viene azzerato. Non vengono scritti limiti di corrente, tensioni di carica,
regolatori, configurazione TS, IRQ o parametri del gauge. Non viene copiata
l'inizializzazione completa dell'esempio, che modifica anche la ricarica.

## Persistenza e verifiche

PDD28/PDDS, PDH2 con lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni restano invariati. La diagnostica v1 conserva
il proprio schema: attenuazione e Busy non sono misure di consumo o un
rapporto batteria. Campioni, avvisi, pausa e richiesta di arresto sono in RAM.
La raccolta PC e Beyond non vengono modificati.

Il core verifica **262.144 combinazioni indipendenti** di stato/direzione/SOC,
**16.384 valori ADC**, letture fallite/obsolete, gauge/ADC disabilitati,
wrap del tempo, blocchi della pausa e singola scrittura del registro di arresto
per tutti i valori di configurazione. Le prove host esercitano le **110 viste**,
telemetria, pausa/risveglio simulati, quarantena degli input, connessione USB,
errori, condizioni di inattività, storico pendente, errori dei salvataggi e
richiesta PMIC unica. Gli adapter hardware di pausa/arresto sono simulati
nel test UI: non è una prova del PMIC, del pannello o del light sleep fisici.
Esiti complessivi in [FIRMWARE.md](FIRMWARE.md).

Le catture `power-*.png` e `power-gallery.png` sono render LVGL host con dati
inventati. Per il collaudo usare H22/H29 in
[COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md), dopo le prove via USB e un backup.
Restano misure di autonomia/temperatura, calibrazione della stima, verifica
della batteria effettivamente montata e prove di accensione/arresto/risveglio.
