# Grafica e UX del dispositivo — v0.50

Il **blocco 4 è completato nel software**: la home del compagno, il menu e i PF
delle v0.48–49 sono ora affiancati dal restyling delle **85 viste interne**.
Tutte le **88 viste** del firmware sono attraversate dal banco UI LVGL. Il
collaudo di resa, touch e fluidità sul pannello fisico resta nel blocco 3.

## Un'unica interfaccia

| Percorsi | Ambiente e controlli |
| --- | --- |
| Zaino, oggetti, attacco, cariche e ricarica | Bottega, icone per arma/pozione/scorte, quantità e azioni leggibili |
| Magie, capacità, slot, utilizzi e concentrazione | Biblioteca, libro/cristallo, schede e filtri evidenziati |
| Riposi, recuperi e dadi vita | Locanda, camera e camino, anteprime/conferme dello stesso tema |
| Scheda, prove, abilità, salvezze e dadi | Studio del cartografo, schede numeriche, vassoio dei dadi incorniciato |
| Stato, morte, condizioni, effetti, difese, danni e profili | Stesso sistema di pannelli, icone e gerarchia, inclusi i flussi avanzati |
| Tempo, reazione, turno, Aspetto, Schermo e Tasti | Ambiente coerente, icone contestuali e assegnazioni dei tasti illustrate |
| Copia USB, errori, assenze dati, esaurimento e conferme | Stessi controlli e testi espliciti, senza dedurre dati dalle illustrazioni |

I pannelli scuri separano testo e illustrazione; Montserrat conserva leggibilità
per nomi, valori e regole. Le icone condividono contorni, materiali e proporzioni
con il menu della home. Gli stati scelto, premuto e disabilitato rimangono
distinguibili. Le immagini decorative non partecipano al touch.

Le liste di oggetti e magie usano icone specifiche; quantità, arma selezionata,
nomi lunghi e paginazione restano rappresentati. Nei dettagli si conservano i
testi e le anteprime degli effetti già supportati. Le nuove illustrazioni non
trasformano una regola manuale in un automatismo.

## Navigazione, feedback e tasti

Il pulsante **Menu** nella testata apre direttamente il menu principale dalle
viste di consultazione. È disabilitato durante importazione, conferme protette
e dadi in volo. Anche un evento inviato direttamente al controllo disabilitato
non può saltare la conferma. Se manca un'arma valida, il ritorno allo zaino aggiorna anche titolo, icona
e ambiente prima del rendering. Le viste conservano Indietro/Annulla/Conferma nei
rispettivi percorsi; il nuovo accesso al menu non applica operazioni in sospeso.

PWR breve, BOOT breve e BOOT lungo conservano le **13 associazioni** e i blocchi
esistenti. La pagina Tasti mostra un'icona per l'azione assegnata, oltre al nome
del gesto e dell'azione. Non cambia il formato dei salvataggi e non vengono
riassegnati gesti locali. Il focus del menu non è una nuova associazione fisica.

Le transizioni di sezione durano **160 ms**: varia l'opacità dell'icona e del
bordo del pannello, mentre testo e comandi sono subito presenti. Non c'è attesa
prima di un'azione. Le transizioni terminano, si cancellano al ridisegno/cambio
vista e si fermano nell'attenuazione; Animazione disattivata le elimina. Non
sono ereditate transizioni automatiche del tema LVGL sui pulsanti disabilitati.

Le ricevute dei saldi salvati usano le nuove icone e conservano delta e scadenze
precedenti. Le reazioni dell'avatar della v0.49 e i risultati dei dadi mantengono
il loro significato. La sola presentazione non salva NVS e non usa RNG.

## Asset e memoria

`firmware/ui/theme/` contiene il renderer condiviso, i raster C versionati,
gli atlas PNG originali e la provenienza. I nuovi ambienti e le icone sono
generati con OpenAI ImageGen a partire dai riferimenti della home; hash e
riepiloghi delle richieste restano in `source/provenance.json`.

Sono esportate **27 immagini, 1871456 byte**: quattro ambienti RGB565 466×466,
icone RGB565A8 32×32, quattro icone ricevuta 24×24 e una scena Aspetto 300×120.
Gli ambienti sono campionati a 233×233 e replicati esattamente; le icone sono
preparate sulla loro griglia di pixel prima della compilazione. Non si scala
un'immagine indicizzata a runtime e non viene aggiunto un canvas del display.

I dati sono costanti in flash. Palette, decoder, oggetti e buffer LVGL restano
distinti: i byte degli asset e lo spazio libero della partizione non misurano
RAM disponibile, frame rate o autonomia del dispositivo.

```sh
python3 firmware/ui/theme/build_assets.py --verify
# Rigenerazione e confronto esatto, solo per sviluppo: richiede ImageMagick 7.
python3 firmware/ui/theme/build_assets.py --check
npm test
npm run build
npm run firmware:test
python3 scripts/preview-device.py
python3 scripts/preview-dice.py
python3 scripts/preview-dice.py --pool-only
python3 scripts/preview-ui.py
```

Il firmware compila dai raster versionati, senza ImageMagick. Il vecchio avatar
SVG delle altre viste PC rimane separato: il PC continua a servire importazione,
configurazione e backup, mentre questo restyling riguarda il dispositivo.

## Verifiche e anteprime

La suite UI esercita **88/88 viste**, verifica la presenza del tema per ogni
vista interna e produce una cattura di ciascuna. Controlla pulsanti alti almeno
44 px e contenuti nel cerchio, testo contenuto nei controlli, assenza di
sovrapposizioni, nomi lunghi, valori massimi, dati assenti ed errori. I percorsi
precedenti comprendono conferme/annulla, importazione, dadi e tasti configurati.

Sono verificati il Menu reale di undici sezioni senza scritture/estrazioni,
il rifiuto del Menu nella conferma, la fine/cancellazione delle transizioni,
la preferenza statica e la pausa. Le suite C/NVS, i 41411 confronti sintetici
PC → USB → C e i 324 confronti dell'avatar coprono le funzioni conservate.

Le anteprime provengono dal codice firmware eseguito su host con dati sintetici:

- `ui-overview.png`: sei percorsi principali.
- `ui-all-views.png`: foglio di tutte le 88 viste.
- `ui-catalog.html`: catalogo locale, con ogni immagine alla risoluzione originale.
- `ui-views.json`: indice di pagina, tema e cattura.
- `ui-transition.gif`: fotogrammi effettivi della transizione, con pausa finale.

Il catalogo e le immagini sono inclusi nell'archivio di rilascio. Non sono
concept o fotografie dell'hardware.

Verifiche finali passate: **147 test JavaScript**, build PC/HTML autonomo,
suite C/NVS/UI completa, 41411 confronti sintetici PC → USB → C e 324 confronti
avatar, oltre a UI con **ASan/UBSan** (leak detector disattivato). Gli asset
sono stati riesportati e confrontati byte per byte con i dati versionati.

Firmware ESP-IDF 5.5.5 / LVGL 9.4.0 compilato e collegato: **3364272 byte**,
partizione app da 4 MiB **20% libera**. Nessun flash o misura sul dispositivo.

PDD23, USB8, backup PC v11, partizioni e raccolta personale restano conservati.
Nessun accesso Beyond o flash sul dispositivo per questa revisione. I blocchi
1–2 e 3 della [roadmap](PROGETTO.md) restano distinti dal completamento grafico.
