# Preset di classe — v0.83

Dodici look selezionabili sul dispositivo e nell’app PC: **Barbaro, Bardo,
Chierico, Druido, Guerriero, Monaco, Paladino, Ranger, Ladro, Stregone, Warlock,
Mago**. Abiti, armature e accessori distinti per tutte le 21 famiglie visive e
per entrambe le silhouette: **504 sprite completi**.

Il pacchetto è esterno, sulla [SD](ASSET-SD.md). La grafica originale rimane
incorporata e utilizzabile senza scheda. Artificiere e sottoclassi sono successivi.

## Installazione e scelta

1. Aggiornare a firmware 0.83 conservando i dati.
2. Inserire una SD FAT32 a dispositivo spento e riavviare.
3. Dall’installer o dall’editor PC: **Installa look sulla SD via USB**, scegliere
   la porta e attendere **Look verificati**. [Procedura e limiti](SD-WEB-USB.md).
   Il pacchetto ZIP con lettore PC resta un’alternativa manuale.
4. Aprire **Aspetto → Modifica → Classe**. `Altre` scorre i gruppi;
   **Anteprima → Conferma** salva la scelta. Annulla conserva l’aspetto precedente.
5. `Rileggi` rimonta la SD e invalida la cache dopo un cambio di scheda/file.
   PWR breve scorre i controlli, BOOT breve attiva quello evidenziato; BOOT lungo
   torna indietro. Le assegnazioni personali dei tasti rimangono conservate.

Senza SD, file mancante, formato incompatibile o CRC errato, viene mostrato lo
stile originale della specie e silhouette scelte. Il preset richiesto resta
salvato. Aspetto/Classe segnala l’assenza degli asset. Il ripiego non richiede formattazione, scrittura sulla SD o modifica della
scheda personale. La scrittura USB del catalogo è un’azione separata ed esplicita.

## Capelli, copricapi e personalizzazione

Ogni look è un disegno completo, con anatomia e vestiti coerenti. Ranger e Ladro
hanno cappucci che coprono la sommità della testa; l’eventuale frangia rimane
nell’apertura del viso. Non viene sovrapposto lo strato capelli originale.
Il cappello del Mago copre la corona della testa.

I preset hanno **colori e taglio propri**. Le quattro scelte originali di pelle,
capelli, mantello e taglio restano memorizzate; tornano modificabili scegliendo
**Originale**. I relativi controlli sono disabilitati sul PC e saltati sul
dispositivo durante l’uso di un preset. Specie e silhouette restano selezionabili.
La combinazione di ogni colore con ogni vestito e il guardaroba per singolo pezzo
richiederanno asset e maschere specifici.

È presente **un fotogramma neutro per look**. Restano il movimento e i feedback
visivi della scena per cure e danni; i nuovi look non hanno ancora le quattro
espressioni dell’originale. Tavole sorgente e cataloghi normalizzati
`design/classes/catalog-0.png` e `catalog-1.png` sono distinti dalle catture
del renderer nativo, che mostrano ciò che il firmware effettivamente disegna.

## Beyond e valori di gioco

Una singola classe esplicita riconosciuta propone il look nell’anteprima Beyond.
Classe sconosciuta, personalizzata o multiclasse richiedono una scelta manuale:
nessuna classe dominante viene dedotta dai livelli. La proposta è modificabile
prima della conferma; viene creata una nuova scheda conservando quelle esistenti.

Il preset è **solo cosmetico**: non assegna equipaggiamento, non equipaggia armi,
non ricalcola CA o PF e non modifica attacchi, caratteristiche, magie o capacità.
Un arco disegnato non sostituisce l’arma realmente selezionata nella scheda.
Non è stata eseguita una nuova lettura reale di Beyond per questo aggiornamento.

## Persistenza e verifiche

- Nativo **PDD33/PDDX**: un byte finale `outfit` 0–12, dopo scena/specie/corpo;
  8946 + 552 × numero capacità, massimo 75186 byte. Lettura dei formati precedenti
  conservata; quelli senza classe usano Originale.
- Backup PC **v15** e USB **13** quando il look è diverso da Originale. Per
  Originale rimangono le precedenti versioni minime. Il firmware vecchio viene
  rifiutato prima di trasmettere una copia che perderebbe il preset.
- Backup nativo USB v1 conserva l’intero record e lo storico; lettore PC e
  strumento Python riconoscono PDDX. Diagnostica aggiornata a USB13/PDD33.
- Verifiche su host: persistenza e parser delle 504 combinazioni, integrità,
  cache, fallback, pixel C/JS a tre scale e renderer LVGL effettivo; anteprima,
  conferma, errore di salvataggio e ritorno all’originale. Dati sintetici e file
  SD sul disco host, distinti da letture reali e prove fisiche.

**Da verificare sull’hardware:** montaggio e tempi della SD, compatibilità FAT32,
rimozione/reinserimento, consumo, fluidità e leggibilità sul pannello.
Il precedente problema USB con riavvii rimane aperto e distinto dal blocco grafico.

## Esito dei controlli della v0.70

27 ottobre 2026: 209 test JavaScript, suite nativa completa e 118/118 viste;
504 look a tre scale (1512 confronti C/JS), 13608 confronti degli avatar
originali e 71 casi di backup PC/C. AddressSanitizer e UndefinedBehaviorSanitizer
su caricatore, persistenza e UI; nessun errore rilevato. Le ultime correzioni
grafiche di Hobgoblin/Kenku sono state rigenerate e ricontrollate nel caricatore
e nella UI nativa. Prove browser con raccolta isolata: editor, annulla/salva,
recupero palette originale, importazione Beyond sintetica con proposta/override
della classe, trasferimento USB simulato e conservazione delle altre schede.

Firmware ESP32-S3 compilato: 2607584 byte, 1586720 liberi nella partizione
applicazione da 4 MiB (38%). Pacchetto SD: 6195200 byte prima della compressione.
Non sono misure di fluidità, memoria libera o consumo sull’hardware reale.
