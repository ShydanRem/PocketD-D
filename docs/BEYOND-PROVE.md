# Beyond: importazione estesa PC v0.14 — 19 ottobre 2026

> Aggiornamento v0.71, 27 ottobre 2026: [inventario operativo e difese Beyond](BEYOND-INVENTARIO-DIFESE.md).
> Cariche, consumabili e cure esplicite; profilo difese opzionale con USB14 / backup PC v16.
> Stato nativo PDD33 invariato. Prova reale e dati sintetici distinti; conversione ancora parziale.
> Le descrizioni e prove datate sotto conservano il proprio contesto; verifica USB fisica ancora aperta.


> Aggiornamento v0.69: [21 famiglie e 42 silhouette](AVATAR-SELEZIONE-ESTESA.md),
> PDD32/PDDW; USB12 e backup PC v14 per gli ID nuovi.
> Aggiornamento v0.70: [preset di classe su SD](PRESET-CLASSI.md), PDD33/PDDX,
> USB13 e backup PC v15 per i look classe; Originale conserva i formati precedenti.

> Aggiornamento v0.68: [quattro nuove famiglie e compressione senza perdita](AVATAR-SPECIE-ANIMALI.md).
> Catalogo a 16 specie/32 silhouette, PDD31/PDDV; USB11 e backup PC v13
> per i nuovi ID. Formati precedenti conservati; collaudo fisico ancora aperto.

> Aggiornamento v0.67: [specie, silhouette e proposta da Beyond](AVATAR-SPECIE.md).
> Scrittura nativa PDD30/PDDU; USB10 e backup PC v12 per i nuovi avatar,
> con lettura dei formati precedenti. Le prove reali pregresse non attestano
> questa nuova selezione visiva; collaudo hardware ancora aperto.

**Aggiornamento PC 26 ottobre 2026:** nuova lettura reale del personaggio
144777811 e correzione della riconciliazione di scelte, nomi/note, oggetti custom,
contatori morte vuoti e tabella slot. Vedere il [resoconto attuale](BEYOND-RICONCILIAZIONE.md),
che distingue HTTP, formato, calcoli, test sintetici e limiti residui.
Il firmware rimane 0.65.0; le prove storiche sotto mantengono la propria data.

**Aggiornamento v0.41:** le [cariche oggetto](CARICHE-OGGETTI.md) sono configurabili
manualmente e trasferibili al dispositivo; la conversione Beyond non le estrae
ancora automaticamente. Backup correnti v11, USB9. Nessuna nuova lettura reale
Beyond per questo incremento; i risultati reali sotto restano quelli delle prove datate.

**Aggiornamento v0.51:** i cinque saldi presenti nel sorgente sono marcati verificati,
compreso lo zero; dati assenti restano sconosciuti. Il [borsello](BORSELLO.md)
è ora trasferibile e gestibile offline. Queste modifiche hanno prove sintetiche:
non è stata ripetuta la lettura del personaggio reale.

**L'importazione compila ora anche competenze, salvezze, configurazioni delle armi, magie, slot e capacità riconosciute. I dati conservati e gli effetti di gioco automatizzati rimangono distinti.** `conversion.complete` resta `false`: l'app non è un motore completo di tutte le regole Beyond.

## Prova reale della v0.14

Partenza da `main`, commit `27e8978`. Il connettore ordinario PocketD-D ha letto nuovamente il personaggio autorizzato: HTTP 200, 180.034 byte, ID e struttura validati. Nessuna credenziale, dipendenza Foundry o scrittura remota. Il sorgente completo è stato ispezionato in memoria, non salvato o incluso nel repository/archivio. I resoconti privati contengono soltanto i campi selezionati.

| Parte | Esito verificato sui dati reali |
| --- | --- |
| Caratteristiche, PF, CA, inventario | Confermati i risultati corretti della v0.13 e tutte le 24 righe, con duplicati e quantità conservati. |
| Competenze e salvezze | Tre abilità competenti e due salvezze ricostruite dai modificatori e dalle scelte con ID corrispondenti; confrontati i bonus finali. Alcune scelte ancora vuote nel sorgente sono segnalate, senza inventare competenze. |
| Armi | Quattro armi configurate con caratteristica, competenza, dadi, danno, bonus e proprietà. Verificati attacco base, danni e gittate. Più armi equipaggiate restano distinte: si sceglie quella attiva nell'anteprima o nelle Azioni. |
| Capacità e talenti | 23 voci fra capacità e note di classe/specie/talenti/background; esclusi i privilegi di livelli futuri. Due contatori riconosciuti: 3 e 1 utilizzi, con recupero lungo. Nessun utilizzo fittizio per una capacità passiva. |
| Effetti riconosciuti | Verificati uso di Ira (3 → 2), bonus ai danni in mischia con FOR, opzione GWM 2014 −5/+10 e dado critico aggiuntivo in mischia. Il test browser ha controllato attacco e danni dopo le scelte esplicite dell'utente. |
| Stato | Conservati PF temporanei, condizioni, contatori dei dadi vita, tiri morte e monete presenti nel sorgente. Sul caso reale molti sono zero: i casi non zero hanno prove sintetiche dedicate. |
| Magie | Il personaggio reale non ha incantesimi: nessuna prova reale di conversione di un incantatore. Caratteristiche multiple, preparazione, magie innate, slot ordinari/patto e riposi sono verificati con sorgenti inventati. |
| Persistenza | API reale → anteprima → scelta arma → conferma → nuova scheda v10 → uso risorse → reload, in un profilo browser isolato. Le schede già presenti sono rimaste identiche. Nessun accesso al profilo personale dell'utente. |

L'accesso HTTP, il formato JSON e la correttezza dei campi convertiti sono controlli separati. I confronti sono con il JSON effettivamente ricevuto e con l'aritmetica dei valori, non con una schermata autenticata del sito. Il successo di questo caso non prova ogni combinazione di classe, homebrew o regole 2014/2024.

## Che cosa viene compilato

- **Tiri:** competenza, maestria, mezza competenza e bonus numerici riconosciuti per abilità/salvezze/prove. Le scelte sono abbinate agli identificativi e alle opzioni del sorgente; formule di tiro ignote bloccano il tiro interessato, con avviso e sblocco dopo verifica manuale.
- **Armi:** dadi, tipo di danno, FOR/DES e finesse, competenza, bonus magici riconosciuti con controllo di sintonia, proprietà e gittate. Modalità lancio separata per le armi da lancio; gittate coerenti con la griglia. Più armi non vengono fuse né selezionate arbitrariamente.
- **Magie:** nome, testo, livello, concentrazione, preparazione/disponibilità, caratteristica di lancio per voce, bonus di attacco e CD riconosciuti. Consumi innati separati dai normali slot. Slot del patto in una riserva distinta, recuperata al riposo breve/lungo; slot ordinari solo al lungo. Le formule non risolte diventano note con avviso, senza un pulsante di lancio ingannevole.
- **Capacità e talenti:** descrizioni come testo, contatori `limitedUse`, usi rimasti e recupero riconosciuto. Privilegi filtrati per livello, note passive senza contatori inventati. Azioni personalizzate conservate come note. Nessun catalogo o testo del personaggio viene incorporato nella distribuzione.
- **Stato:** PF temporanei che assorbono i danni anche nel combattimento, condizioni, dadi vita usati, tiri morte e monete. Pannello **Stato, risorse e note** per consultarli/correggerli. Un recupero a PF positivi azzera i contatori di morte; non si risolvono automaticamente esiti di morte/stabilizzazione.

Ira, GWM 2014 e Savage Attacks sono riconosciuti da identità sorgente e condizioni supportate. Ira richiede l'uso della capacità e consuma una risorsa; termina la concentrazione e impedisce il lancio. Il bonus ai danni si applica solo in mischia con FOR, non al lancio dell'arma. GWM si abilita esplicitamente e vale solo con arma pesante competente in mischia. Il dado critico aggiuntivo vale solo in mischia. Resistenza Implacabile richiede zero PF e verifica col DM. Queste regole non vengono estese a varianti sconosciute soltanto perché hanno un nome simile.

## Limiti ancora espliciti

Maestrie 2024, effetti aggiuntivi, armi versatili/due armi, resistenze, durata delle capacità, automazioni delle condizioni diverse da Avvelenato, preparazione completa per tutte le classi e risorse condivise/variabili restano parziali o manuali. Le descrizioni e proprietà disponibili aiutano a gestirli al tavolo. I dadi vita sono conservati e modificabili: recupero e guarigione da riposo non vengono indovinati senza scelta del regolamento.

CA/PF massimi restano fotografie da correggere dopo modifiche locali. Contenitori, peso, consumo automatico di munizioni e sincronizzazione non sono automatizzati. Alcuni override, origini opzionali, massimi modificati, ASI non risolti o effetti sui valori principali continuano a bloccare l'importazione. Limiti: 20 personaggi, 60 oggetti, nome personaggio 24 caratteri, nome voce 40; v10 fino a 120 magie/capacità con 6000 caratteri ciascuna. Descrizioni più lunghe sono dichiarate come estratti; altre voci non rappresentabili sono segnalate. La raccolta ha un limite complessivo di 5 MB.

Il resoconto v3 espone copertura per sezione, dettagli dei tiri/armi/magie e omissioni. `canImport` abilita una nuova scheda utilizzabile nel sottoinsieme dichiarato; non certifica equivalenza completa di tutti gli effetti D&D. Anteprima e scelta dell'arma non scrivono; soltanto conferma e salvataggio riuscito attivano la nuova scheda. La raccolta non viene sostituita. Non si aggiornano automaticamente le vecchie importazioni v0.13: conservarle o reimportare come nuova scheda, per non sovrascrivere progressi locali.

## Avvio e verifiche

Richiede Node.js 24.19+. `npm ci`, `npm run build`, `npm start`, poi **http://127.0.0.1:5173**. L'archivio include `dist/`: per l'avvio basta `npm start`. Il solo HTML autonomo e Vite non forniscono il connettore. Usare lo stesso browser/hostname/porta per ritrovare la raccolta; esportarla prima di trasferirla. La nuova scheda usa v10, mentre le schede precedenti mantengono v9 senza riscritture preventive. Le app precedenti rifiutano v10 invece di scartare i nuovi dati.

```sh
npm test
npm run build
python scripts/check-beyond-import.py
python scripts/check-beyond-rich.py
python scripts/check-beyond.py
python scripts/check-library.py
python scripts/check-interface.py
python scripts/check-inventory.py
python scripts/check-party.py
```

**121 test locali passati**, build riuscita. Playwright Python e Chromium `/usr/bin/chromium` verificano importazione, scelte, contatori, attacchi e critici, spellcaster multipli, magie innate, patto, riposi, stato, modifiche di profilo/oggetti/capacità, backup/reload, HTML ostile, conflitti/quota/corruzione e viewport 390/320 px. Nessun errore JavaScript rilevato. Queste prove usano sorgenti sintetici, distinti dalla prova reale sopra. Le regressioni della raccolta, dell'inventario, dell'interfaccia e del party non modificano raccolte personali.

Firmware, importazione autonoma dal dispositivo, autenticazione privata e scrittura verso Beyond rimangono successivi. Le fonti e i vincoli di distribuzione restano in [BEYOND-FATTIBILITA.md](BEYOND-FATTIBILITA.md).

---

# Storico delle prove v0.13 — 19 ottobre 2026

**PocketD-D v0.13 importa un sottoinsieme verificato dei dati principali, con anteprima, omissioni e conferma prima di creare una nuova scheda. Non è una conversione completa del personaggio.** L'integrazione usa il connettore PocketD-D, senza Foundry, esportazioni intermedie, servizi MrPrimate o credenziali per la lettura pubblica.

## Che cosa era passato nella prima prova

La base di lavoro è `248e00f3815e23144f7a3994b593407f8f986324`, verificata su `main` remoto. La prima lettura riuscita del 19 ottobre aveva provato tre cose: accesso HTTPS al servizio, risposta HTTP 200 di 180.034 byte e accettazione della struttura JSON con ID corrispondente. I 89 test dell'epoca erano sintetici, distinti da questa richiesta reale.

Questi risultati non giustificavano “conversione quasi completa”. L'anteprima v1 ignorava i modificatori razziali delle caratteristiche, non determinava la CA e non creava una scheda. Un numero plausibile non era una prova di corrispondenza. Il tentativo del 16 ottobre, invece, era stato fermato dal proxy prima di Beyond: quel rifiuto non descriveva la visibilità del personaggio.

## Lettura e verifica reale attuale

È stato usato il personaggio fornito dall'utente, senza fixture o trasporti sintetici. Il sorgente è stato ispezionato in memoria; nome, ID, link, inventario personale e JSON completo non sono inclusi nel repository o nell'archivio.

| Livello | Controllo effettivo | Risultato e limite |
| --- | --- | --- |
| HTTP | GET alla destinazione fissa `character-service.dndbeyond.com/character/v5/character/ID?includeCustomItems=true`, senza credenziali e redirect | HTTP 200, 180.034 byte. Proxy ereditato e verifica TLS conservati. Dimostra accesso a questo personaggio, non a quelli privati. |
| Formato | JSON, `success: true`, ID richiesto, sei caratteristiche, classi/livelli, campi PF, modificatori e inventario | Risposta accettata; i controlli di struttura precedono la bozza. Non implica correttezza delle regole. |
| Caratteristiche | Confronto dei sei valori con base, bonus, override e modificatori del sorgente | Corretto l'errore sui bonus razziali: +2 FOR e +1 COS erano omessi. La scelta di avanzamento è un talento, quindi i due incrementi alternativi non si sommano. Capacità di livelli futuri escluse. |
| PF | Calcolo indipendente da PF base, modificatore della COS finale × livello, bonus, override e danni | Confermati massimo e attuali. Nel caso reale la correzione di COS non cambia il modificatore né i PF. I temporanei restano separati e non sono importati. |
| CA | Controllo dell'equipaggiamento e della Difesa senza armatura attiva nel sorgente | Ricostruita come 10 + modificatore DES + modificatore COS; nessuna armatura/scudo attivi nel caso verificato. È una fotografia, non un ricalcolo dinamico in app. |
| Inventario | Confronto riga per riga di nome, quantità e stato equipaggiato | Tutte le 24 righe conservate, incluse voci con nomi uguali. Equipaggiamento annotato nelle note; nessuna arma scelta arbitrariamente. |
| Creazione nell'app | Browser isolato → API reale → anteprima → conferma → nuova scheda → reload | Riuscita. Le schede di test già presenti sono rimaste identiche; provenienza e omissioni persistono. Non è stato usato il profilo browser personale dell'utente. |

Il confronto è con i dati JSON effettivamente ricevuti e con le formule documentate dal sorgente, non con uno screenshot della scheda sul sito Beyond. Non è stata verificata l'equivalenza di tutti i tiri o effetti. Un solo personaggio reale non dimostra la compatibilità con tutti i regolamenti o gli incantatori.

La configurazione cloud include ora il dominio; lo strumento ambiente riportava stato della policy `unknown`. La raggiungibilità è provata dalle richieste effettive. Il sandbox dei comandi richiedeva inoltre il permesso di rete per raggiungere il proxy e aprire i server di test locali; nessuna modifica di proxy, TLS o lista dei domini è stata usata per aggirare rifiuti.

## Conversione supportata e blocchi

`src/beyond-conversion.js` è separato da trasporto e UI. Il resoconto v2 distingue `checks.http`, `checks.format` e `checks.conversion`; `conversion.complete` resta `false`. `canImport` significa soltanto che il sottoinsieme `core-v1` è rappresentabile e che una bozza locale validata è disponibile. Non significa personaggio completo né importazione già avvenuta.

Sono gestiti bonus numerici incondizionati ai punteggi, massimo ordinario 20, override espliciti, bonus PF piatti/per livello, sostituzione del massimo tramite override, CA base e armature ordinarie leggere/medie/pesanti/scudo, Difesa senza armatura basata su COS o SAG e bonus CA riconosciuti. Gli effetti di classe sono filtrati per livello; i bonus degli oggetti richiedono un'origine risolta e stato equipaggiato/sintonia. La scelta alternativa del talento è riconosciuta; una scelta ASI non risolta blocca la creazione.

Effetti principali condizionali o sconosciuti, massimi modificati, origini opzionali, override personalizzati non riconciliati, equipaggiamento ambiguo e dati mancanti bloccano la bozza. Questa copertura è conservativa e limitata ai formati riconosciuti: non costituisce un motore D&D generale. Nomi oltre 24 caratteri, oggetti oltre 40 caratteri/999 unità o più di 60 voci bloccano l'importazione senza tagli silenziosi. Gli oggetti personalizzati richiedono ulteriore supporto.

## Omissioni esplicite

- Abilità e salvezze: competenze inizializzate a zero, da completare prima di usarne i tiri. Non si inferiscono automaticamente dalla classe.
- Armi: importate come oggetti, con quantità e note; attacchi, danni, portata e proprietà sono da configurare. Nessuna arma dimostrativa o attiva viene aggiunta.
- Magie, slot, capacità, talenti, resistenze e vantaggi condizionali: non convertiti. Nessuna risorsa dimostrativa viene assegnata.
- PF temporanei, condizioni generiche, dadi vita, tiri morte: non rappresentati dal modello locale attuale.
- Peso, monete, contenitori e sintonia: non trasferiti come meccaniche; equipaggiamento e sintonia sorgente sono solo annotati.
- CA e PF massimi: valori importati, da aggiornare manualmente dopo modifiche locali. Avatar neutro; nessuna immagine Beyond scaricata.
- Regolamento 2014/2024 e riconciliazione di aggiornamenti: non implementati. La provenienza minima non abilita sincronizzazione.

L'anteprima elenca questi limiti, richiede accettazione e una conferma di creazione. Un avviso permanente sulla scheda importata riapre le omissioni. La provenienza e le omissioni rimangono nei backup individuali e della raccolta.

## Avvio sul PC

Richiede **Node.js 24.19 o successivo** per `--use-env-proxy`. Dalla cartella del progetto:

```sh
npm ci
npm run build
npm start
```

Aprire **http://127.0.0.1:5173**, scegliere **Importa da D&D Beyond**, inserire URL/ID, leggere anteprima e omissioni, accettare e premere **Crea nuovo personaggio**. Se la conferma viene annullata, non viene scritto nulla. L'importazione aggiunge una scheda; non aggiorna o sostituisce quelle esistenti. Il server ascolta solo su loopback e non conserva un archivio di personaggi sul PC: i salvataggi rimangono nel browser.

L'archivio distribuito include anche `dist/`: se presente è sufficiente `npm start`, senza installare dipendenze. Se i sorgenti vengono modificati, rigenerare prima la build. `POCKETDD_APP_PORT` cambia la porta. Il solo HTML autonomo e `npm run dev` non forniscono il connettore PC.

**Usare lo stesso browser e lo stesso indirizzo, hostname e porta.** `localhost`, `127.0.0.1`, altre porte e un file HTML hanno raccolte separate. Per trasferire una raccolta usare il suo backup; non recuperarla con sostituzioni automatiche. Prima del nuovo flusso è consigliato esportare la raccolta esistente. Errori di quota, dati corrotti, raccolta piena o conflitti con un'altra scheda interrompono la creazione senza sovrascrivere i dati persistenti.

Rimangono disponibili il laboratorio diagnostico, che non scrive raccolte, e il comando:

```sh
npm run beyond:lab
npm run beyond:probe -- "URL_o_ID" --out /percorso/nuovo-resoconto.json
```

Il laboratorio usa la porta 5174. Il resoconto contiene dati personali selezionati e la bozza locale, non il JSON Beyond completo; non è direttamente un backup della raccolta. `--out` crea un file nuovo con permessi `0600` e non sovrascrive file esistenti. Non inserirlo nel repository o nell'archivio da distribuire.

## Verifiche ripetibili

```sh
npm test
npm run build
python scripts/check-beyond.py
python scripts/check-beyond-import.py
python scripts/check-library.py
python scripts/check-interface.py
```

**103 test unitari/integrativi locali passati**, build riuscita. I test Beyond ripetibili usano fixture inventate o server locali: non sono prova di accesso remoto. Coprono filtri di livello/scelta, bonus, override, PF, CA, inventario, formule non supportate, schema/provenienza e protezione dello storage.

La verifica browser richiede Playwright Python e `/usr/bin/chromium`. `check-beyond-import.py` serve l'app compilata con un connettore sintetico dichiarato come tale: anteprima senza scritture, testo ostile, errori, blocchi, cambio URL durante l'attesa, annullamento, creazione, vecchie schede identiche, backup/reload, conflitti fra schede, quota, capacità di 20 personaggi e dati corrotti. Viewport desktop e mobile 390/320 px, senza errori JavaScript. Il laboratorio diagnostico e le regressioni raccolta/interfaccia restano verifiche sintetiche separate. La prova reale descritta sopra è stata eseguita separatamente con il connettore ordinario.

Firmware, autenticazione privata e scrittura verso Beyond rimangono traguardi successivi. Prima di ampliare la copertura servono altri casi reali autorizzati, inclusi incantatore, bonus complessi e regole miste; non si abbassa il controllo perché HTTP risponde 200. Fonti e vincoli di distribuzione restano in [BEYOND-FATTIBILITA.md](BEYOND-FATTIBILITA.md).
