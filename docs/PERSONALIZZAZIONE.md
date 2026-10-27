# Personalizzazione sul dispositivo — v0.66

Il dispositivo permette ora di scegliere **carnagione, colore dei capelli,
taglio, mantello e sfondo**, senza usare il PC. Restano le 81 combinazioni
dell'avatar illustrato già supportate, con quattro pose. I tre ambienti sono
indipendenti: **Accampamento**, **Rovine lunari**, **Porto all'alba**.
In totale sono 243 combinazioni di aspetto e scena.

La grafica dell'accampamento approvata resta quella precedente. I due nuovi
sfondi seguono la stessa direzione pixel art HD-2D, senza sovrapporre una nuova
interfaccia alla home. Nessuna scelta estetica cambia caratteristiche, PF,
CA, inventario, equipaggiamento o regole.

## Uso sul dispositivo

1. Toccare l'avatar nella home, poi **Aspetto → Modifica**.
2. Con le frecce scegliere il campo; i tre pulsanti mostrano le alternative.
   Il contorno verde indica quella selezionata. Anche il battito delle palpebre
   usa l'aspetto della bozza, senza tornare all'avatar salvato.
3. **Scena** apre lo sfondo con l'avatar intero; le frecce cambiano ambiente.
   **Avatar** torna ai colori e al taglio, conservando la bozza.
4. **Anteprima** mostra il risultato e il riepilogo. **Indietro** permette di
   correggerlo; **Conferma** salva avatar e sfondo insieme.
5. **Annulla** scarta la bozza. Un errore di memoria mantiene l'anteprima e
   la scheda precedente: nessun falso salvataggio. Confermare un aspetto già
   identico non esegue una scrittura.

Animazione, luminosità e configurazione dei tasti rimangono accessibili dalla
pagina Aspetto. L'impostazione di animazione precedente conserva il proprio
comportamento di salvataggio; non è parte della bozza di colori/sfondo.

### Pulsanti configurabili

La nuova assegnazione **Aspetto** si aggiunge alle azioni già disponibili per
PWR breve, BOOT breve e BOOT lungo. Gli indici delle vecchie assegnazioni
rimangono invariati; non si sostituiscono le preferenze dell'utente.

Dentro l'editor e la sua conferma si usa una navigazione locale:

- **PWR breve:** seleziona il controllo successivo, indicato dal contorno oro.
- **BOOT breve:** attiva il controllo selezionato; se nessuno è selezionato,
  il primo tocco mostra la selezione.
- **BOOT lungo:** torna alla modifica dall'anteprima, oppure annulla la bozza.

Le scorciatoie di gioco sono sospese durante questa navigazione e riprendono
all'uscita. Nessun gesto estetico tira dadi o modifica risorse. I controlli
touch restano almeno 44 pixel e all'interno del display circolare. Il comportamento
fisico di pressione/alimentazione del tasto PWR resta quello della scheda.

## Salvataggio, importazione e backup

Il record nativo diventa **PDD29 / `PDDT`**, con un byte aggiuntivo per la scena.
La dimensione è `8943 + 552 × numero di poteri`, massimo 75.183 byte. La lettura
dei record precedenti continua: la scena iniziale è Accampamento. Si conserva
la semantica dei costi alternativi PDD28 e dei riferimenti ai turni.

Il salvataggio usa i due banchi NVS esistenti, senza cambiare partizioni.
Una conferma modifica solo avatar/scena. Annulla dei PF conserva l'aspetto
appena salvato. Il backup nativo conserva integralmente la scena e le nuove
assegnazioni; app PC e script Python ne accettano il record e mostrano lo
sfondo nell'anteprima. L'involucro del backup resta v1; il backup della raccolta
PC resta v11.

**USB9 rimane invariato.** L'avatar preparato sul PC viene trasferito come
prima; lo sfondo è una scelta locale del dispositivo e una nuova importazione
non lo sovrascrive. Il ripristino di un backup nativo ripristina invece anche
la scena contenuta nel backup. Non è stata aggiunta una configurazione degli
sfondi alla raccolta PC o una sincronizzazione bidirezionale.

Un'app/firmware precedente non legge il nuovo record PDD29. Conservare il
backup prima di aggiornare; un ritorno al firmware vecchio non è una migrazione
inversa. Il firmware protegge i record futuri da sovrascritture automatiche.

## Grafica e risorse

Le due nuove ambientazioni originali sono generate con OpenAI ImageGen usando
l'accampamento come riferimento di stile. Sorgente integro e provenienza sono
in `firmware/ui/scenes/source/`; non contengono dati del personaggio.

Due pannelli 887×887 vengono esportati con campionamento al vicino più prossimo
a 233×233, ciascuno con palette di 128 colori. I dati indicizzati occupano
55.313 byte per scena, **110.626 byte totali**. Sul dispositivo la replica esatta
2× e la miniatura usano due buffer condivisi in PSRAM, **255.204 byte**; non
esistono tre framebuffer aggiuntivi o trasformazioni dell'immagine a ogni tick.
La cache LVGL viene invalidata cambiando scena. L'accampamento originale
rimane nel proprio asset RGB565, senza modifiche.

Il firmware compila dai dati C versionati: ImageMagick e ImageGen non servono
all'utente o alla compilazione ordinaria. Esportazione opzionale:

```sh
python3 firmware/ui/scenes/build_assets.py --verify
python3 firmware/ui/scenes/build_assets.py --check
python3 scripts/preview-appearance.py
```

## Verifiche e limiti

- **196 test JavaScript**, inclusa la lettura PDD29/vecchi backup, scene fuori
  limite e assegnazioni preservate.
- Nucleo C: tutte le **243 combinazioni**, indipendenza dai valori di gioco,
  roundtrip nativo, migrazione PDD28, dati invalidi rifiutati senza modifiche.
- UI LVGL reale sull'host: bozza, navigazione, conferma/annullamento, errore e
  riprova, nessuna scrittura per assenza di modifiche, battito della bozza,
  Annulla dei PF, tasti locali, importazione e backup/ripristino. **118/118 viste**.
- Controlli NVS e regole esistenti, controlli C↔PC del backup e verifica di
  integrità degli asset. UI e nuovo nucleo con ASan/UBSan.
- Build ESP32-S3: **3.533.824 byte**, circa **16% libero** nella partizione
  applicazione da 4 MiB. Partizioni e limiti USB invariati.

Le anteprime sono catture del codice firmware con un personaggio sintetico:
non sono fotografie o prove di fluidità del dispositivo. **Il collaudo fisico
e il problema USB segnalato restano aperti**, rinviati alla prova con l'utente.
Nessuna lettura Beyond o modifica della raccolta personale per questo incremento.

Restano successivi: altri abiti e accessori disegnati, silhouette/specie diverse,
nome e descrizione del PG modificabili sul dispositivo, editor dello sfondo sul
PC, immagini personali e nuove animazioni. L'abbigliamento illustrato rimane
decorativo e non rappresenta automaticamente l'equipaggiamento di Beyond.

È disponibile una [prima proposta visiva di dodici specie](design/species/README.md).
È una tavola di stile da valutare, separata dagli asset di produzione e
dall'assegnazione automatica all'importazione, che non è ancora implementata.
