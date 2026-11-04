# PocketD-D: stato e lavoro rimanente

Aggiornamento del **4 novembre 2026 — v0.104**. Il dispositivo è il prodotto;
l’obiettivo è renderlo autonomo anche per l’importazione Beyond. Il firmware
ora contiene lettura e conversione Wi-Fi dei PG pubblici tramite ID, da collaudare
sull’hardware. Il PC resta utile per installazione/aggiornamenti, backup e il
percorso d’importazione già provato. Collegamento account e PG privati sul Pocket
sono implementati nella v0.92 con prove sintetiche, ancora da collaudare realmente.

Il software copre una prima sessione individuale offline. L'utente riferisce
risolti USB, installer e SD e conferma visibile il sito GitHub Pages. Il collaudo
completo di batteria, interruzioni di alimentazione e delle nuove funzioni 0.85–0.92
resta distinto da questa conferma.

## Pagina web di installazione

[GitHub Pages](https://shydanrem.github.io/PocketD-D/) è attivo sul repository
ora pubblico per scelta del proprietario. Il ramo gh-pages contiene il sito
compilato e si aggiorna con il publisher verificato. Firmware e look SD si
aggiornano dal browser; il pacchetto locale conserva il connettore Beyond PC.
Non si pubblicano credenziali, raccolte o JSON completi di personaggi.

## Nona iterazione UX · v0.104

[Consultazione e registro](UX-RPG.md#v0104-consultazione-e-registro).
Profilo con icone per gruppo e ricerca indipendente; iniziativa distinta dalle
voci di sola lettura. Registro con filtri, risultati leggibili e facce originali.
Un tiro in attesa rimane visibile attraverso i filtri, fino a cinque componenti:
riprova atomica e rinuncia esplicita senza annullare l'azione di gioco. Avviso
del registro separato dagli errori della scheda, che mantengono la precedenza.
PWR/BOOT disponibili anche nella consultazione e nei tastierini di ricerca.
Le descrizioni del profilo restano fotografie; nessun nuovo automatismo D&D,
formato persistente o asset SD necessario. Restano altre viste secondarie,
collegamenti e messaggi da uniformare e il riscontro sul dispositivo.

## Ottava iterazione UX · v0.103

[Turni, tempo e riferimenti](UX-RPG.md#v0103-turni-tempo-e-riferimenti).
Reazione e azioni del proprio turno in griglia; A/B/C con nome leggibile e timer
pertinenti a inizio/fine. Quantità rapide per il tempo dichiarato, confronti di
CA/PF/reazione e scadenze con ore/minuti corretti. Focus locale anche nel
tastierino dei nomi, compresi tutti i comandi di fondo. Nessun recupero da un
riposo implicito: round, turni e tempo mantengono percorsi e conferme distinti.
Consultazione, storico e messaggi secondari seguono nella stessa revisione;
restano distinti riscontro hardware e letture reali Beyond.

## Settima iterazione UX · v0.102

[Danni misti, riduzioni e spese](UX-RPG.md#v0102-danni-misti-riduzioni-e-spese).
Tredici tipi con icone coerenti e scelta diretta. Parti leggibili, dichiarazione
esplicita di fonte e materiali, selezione diretta della parte da ridurre o a cui
assegnare budget. Costi scelti per famiglia e voce, conservando indici e vincoli.
Anteprima a riquadri con danni, PF, temporanei e accesso alle spese prima/dopo.
Nessun costo anticipato; danni e spese restano una conferma atomica, anche se il
profilo richiede pagamento a danno zero. Turni, tempo e viste secondarie seguono
nella stessa revisione; nessuna nuova prova Beyond o hardware dichiarata.

## Sesta iterazione UX · v0.101

[Editor di effetti, difese e timer](UX-RPG.md#v0101-editor-di-effetti-difese-e-timer).
Scelte dirette del bersaglio degli effetti, proprietà delle difese in griglia,
confronti prima/dopo con dettagli completi conservati. Selettori condivisi per
sette regole e sei tipi di durata, quantità rapide e regolazioni fini. La stessa
durata riselezionata conserva quantità e soggetto; un nuovo tipo li reimposta.
Focus locale PWR/BOOT negli editor e nelle conferme, importazioni protette durante
le bozze. Nessuna nuova regola meccanica, scadenza in tempo reale o formato.

Le proprietà condizionali e i risultati potenziali sono distinti dai danni
applicabili al singolo colpo. Il resto dei flussi avanzati e le nuove interazioni
sull’hardware rimangono da valutare; nessuna nuova lettura reale Beyond.

## Quinta iterazione UX · v0.100

[Raccolta, aspetto e impostazioni](UX-RPG.md#v0100-raccolta-aspetto-e-impostazioni).
Schede dei PG con identità e stato, anteprima del cambio con avatar completo e
PF/CA; catalogo Beyond coerente e anteprima con tutte le caratteristiche.
Campi dell’aspetto selezionabili direttamente, colori fissi protetti e comandi
fisici locali. Impostazioni come griglia visiva con nuove icone di rete, energia
e tasti; luminosità immediata. Conferme, copie SD e accesso account conservati.
Nessuna nuova lettura reale Beyond o modifica della raccolta personale.

La revisione continua sugli editor avanzati e sulle schermate secondarie;
la resa e i nuovi percorsi richiedono ancora riscontro sull’hardware.

## Quarta iterazione UX · v0.99

[Riposo e stati sul dispositivo](UX-RPG.md#v099-riserve-condizioni-ed-effetti-locali).
Riserve dei dadi vita selezionabili direttamente dal riposo breve, tre per pagina,
per tutte le 20 riserve del modello; quantità e conferma prima di tirare. Riprova
conserva il tiro fissato. Stato come griglia di comandi; note delle condizioni con
icone bilingui condivise, livelli e meccaniche tenuti separati. I quattro stati
supportati sui tiri restano una bozza da confermare; le altre regole rimangono
manuali e Avvelenato mantiene il comando separato.

Effetti locali con attività, durata, pausa e vincolo leggibili. Unità ore/minuti
corrette, modifica e avanzamento del round con confronto di CA/massimi/PF. La
scadenza può far perdere PF: non viene descritta come cura. Nessuna scadenza
avviene leggendo una pagina; resta l’avanzamento esplicito del motore esistente.

Prove nuove con PG, riserve e timer sintetici; nessuna lettura reale Beyond,
modifica della raccolta personale o nuova prova fisica dichiarata. Formati,
partizioni, tasti e asset SD conservati. Prossima priorità UX: raccolta/importazione,
aspetto e impostazioni. Editor avanzati di durate e difese ancora rifinibili;
la roadmap funzionale resta conservata.


Verifiche v0.99: 357 test JavaScript, suite nativa, 153 viste LVGL, nuovi
scenari di sessione/stati, ASan/UBSan e compilazione ESP32. Dati sintetici;
nessuna nuova lettura reale Beyond o modifica della raccolta personale.

## Terza iterazione UX · v0.98

[Confronti prima/dopo e immagini](UX-RPG.md#v098-equipaggiamento-conferme-e-riposi).
Equipaggia/Riponi direttamente dallo zaino per armi e profili noti, con bozza,
conferma e ritorno allo stesso elenco filtrato. Confronto dei valori principali;
tutti gli effetti rimangono nei dettagli. Quantità zero e profili assenti sono
protetti. Sintonia, adozione dei bonus importati e nuove configurazioni rimangono
esplicite, senza inferenze da icone o nomi.

Conferma magia con slot, utilizzi, PF e concentrazione reali prima/dopo; dettagli
per perdita di CA e massimi. Consumabili con quantità e dadi chiari, senza tiro
anticipato. Riposi con riepilogo, regola dadi vita scelta direttamente e dati
assenti dichiarati non disponibili. Il riepilogo mostra anche una riduzione dei
PF causata dalla fine di un massimo temporaneo, anziché presentarla come cura.

Nuove prove native con PG sintetici, annullamento, salvataggio fallito/riprova,
dettagli senza RNG o scritture, tasti/import protetti e confronti di stato completo.
Nuovo collaudo fisico distinto. Formati, partizioni, PG e risorse SD conservati;
nessuna lettura Beyond reale o modifica della raccolta personale in questa attività.
Restano le viste avanzate di riposo, condizioni/effetti, raccolta, aspetto e
impostazioni; la revisione UX e la roadmap funzionale non sono dichiarate concluse.


Verifiche v0.98: 356 test JavaScript, suite nativa, tutte le 153 viste LVGL,
ASan/UBSan e build ESP32. Nuove prove con dati sintetici; nessuna nuova
lettura Beyond reale o modifica della raccolta personale.

## Seconda iterazione UX · v0.97

[Direzione, avanzamento e anteprima](UX-RPG.md). Il risultato del tiro diventa
l’elemento principale: numero pixel a 48 px, esito e prossima azione distinti.
Critico, mancato, cure effettive e danni totali delle componenti sono leggibili
senza ricostruire una frase. Prima della fine del volo il risultato resta nascosto;
la navigazione fra componenti conserva esattamente dadi e storico. Il 20 naturale
di una prova non diventa un critico o successo automatico.

Schede magia e oggetto con emblemi più presenti; magia ordinata per nome,
preparazione, valore, costo e comando, testo esteso apribile dal nome. Gli effetti
non automatizzati restano dichiarati manuali. Nomi larghi interamente leggibili;
scudo distinto dal teschio nel catalogo comune. Home v0.96 conservata.

Nessun nuovo formato, scrittura Beyond, JSON reale o modifica delle schede
personali. Continuano a mancare la revisione delle conferme, equipaggiamento,
riposi/stato e poi raccolta/aspetto/impostazioni. Valutazione estetica e collaudo
fisico restano distinti dalle prove software; la revisione non è dichiarata conclusa.

## Priorità attuale: UX del dispositivo · v0.96

La roadmap funzionale rimane conservata; per richiesta dell’utente la priorità
è la revisione dell’esperienza complessiva, fino a una direzione soddisfacente.
[Direzione e avanzamento](UX-RPG.md). La v0.96 è la prima iterazione, non la
conclusione della revisione né una nuova approvazione estetica dell’utente.

Home con icone più grandi e nomi su pressione prolungata, avatar e scene
conservati. Attacco ordinario in un tap dalla home, lancio nel successivo;
variante con arma selezionabile nella stessa schermata e preparazione nel
percorso. Senza arma si apre la scelta, non lo zaino. Correzioni manuali PF
separate dalle azioni comuni. Tema condiviso più netto, riepiloghi di zaino e
magie più compatti; stato «Salvato offline» nascosto, errori ancora visibili.

I tasti configurabili, le regole, i formati, le partizioni e i dati personali
rimangono compatibili. Non occorre reimportare i PG o reinstallare i look SD.
Le catture sono del renderer nativo con PG sintetico; collaudo fisico e
giudizio estetico dell’utente rimangono distinti dai controlli software.
356 test JS, suite nativa, 153 viste LVGL, ASan/UBSan e compilazione ESP32.
Nessuna nuova lettura Beyond reale o modifica della raccolta personale.

## Nuovo nella v0.95

[Magie: preparazione e slot](MAGIE-USO-RAPIDO.md): più magie di livello in una
bozza, filtri, riepilogo e un solo salvataggio; trucchetti, capacità, slot e
concentrazione conservati. Riprova, annullamento e rifiuto di bozze obsolete.
Selettore comune ordinari/patto per magie, Punizione e cannone: livelli e saldi
visibili, proposta del livello minimo fra entrambi i gruppi e nessuna spesa
prima della conferma. Tasti locali e scelta dei costi coerente anche sul PC.

356 test JS, suite nativa, 153 viste LVGL, ASan/UBSan e compilazione ESP32.
Nuove prove sintetiche; nessuna nuova lettura Beyond o raccolta personale
modificata. Formati PG/USB/backup, SD e partizioni invariati; bundle delle
regole rigenerato e parità PC/nativo verificata. Limiti di preparazione di classe,
magie sempre preparate e meccaniche generiche degli incantesimi restano manuali.

## Nuovo nella v0.94

[Connessione e recupero autonomi](AUTONOMIA-RECUPERO.md): un tentativo sulla rete
salvata, configurazione Wi-Fi nel flusso, ripresa per ID dello stesso PG e
riprova esplicita dopo un errore. Annullare rimuove la richiesta in attesa;
nessuna lettura interrompe editor, revisione rete o tocco. Account temporaneo e
ricordato visibili, elenco valido vuoto distinto dal mai caricato. Errori di
trasporto distinti dall’accesso da ricollegare; un nuovo collegamento fallito
conserva il precedente. Nessun aggiornamento silenzioso delle schede.
Il collegamento web alla sezione firmware funziona anche cambiando il frammento
di una pagina già aperta, oltre all’apertura iniziale e al ricaricamento.

354 test JS, suite nativa e **150 viste LVGL**, controlli ASan/UBSan e build ESP32.
Nuove prove con rete, HTTP e account simulati: non sono un accesso privato reale.
Formati, bundle convertitore, partizioni e credenziali persistenti invariati.
L’estensione rimane la soluzione scelta per l’associazione iniziale; firmware
0.94 compatibile con il componente 0.93. PG, SD e tasti conservati.

## Nuovo nella v0.93

[Pocket PC](POCKET-PC.md): sito come ingresso unico, apertura dell’estensione
rilevata senza passare dal popup, Beyond/SD/backup nella stessa pagina e selezione
USB riutilizzata dopo un riconoscimento valido. Nessuna scansione delle porte
all’avvio; cambio esplicito e invalidazione su scollegamento/errore. Preparazione
account automatica dopo la scelta USB e consenso cookie solo quando serve.
Il ponte sito/estensione scambia soltanto presenza/apertura, mai credenziali o PG.

354 test JS e pagina reale con risposte simulate: una scelta della porta per SD,
Beyond e backup, consenso fisico prima del segreto, blocco delle operazioni
concorrenti, formato backup controllato e cambio Pocket esplicito. Firmware
0.93 compilato; logica nativa/partizioni/formati invariati dalla 0.92. Nessun
nuovo accesso Beyond reale o collaudo fisico dichiarato.

## Nuovo nella v0.92

[Collegamento Beyond](COLLEGAMENTO-BEYOND.md): cookie opzionale nell’estensione,
codice USB e consenso fisico prima del trasferimento; verifica dell’identità dal
Pocket, scelta di persistenza, elenco e lettura autenticati. Nessun fallback
anonimo o elenco campagna. Ricevuta solo dopo verifica e salvataggio/rilettura;
commit incerti bloccano le operazioni account fino al riavvio. Scollegamento del
solo accesso, senza cambiare PG, SD, Wi-Fi o browser. NVS non cifrata dichiarata;
credenziali escluse dai backup PG, nessuna modifica a partizioni/formati/eFuse.

345 test JS, 148 viste LVGL e prove native con sessioni/HTTP/NVS/USB simulati.
Nessuna credenziale reale acquisita, nessuna nuova lettura privata reale.
Il login/elenco reale del browser resta quello confermato dall’utente il
31 ottobre; handshake USB, cookie MV3, autenticazione ESP32, schede private e
persistenza dopo riavvio di questo nuovo percorso restano da collaudare.

## Nuovo nella v0.91

[Beyond sul dispositivo](BEYOND-DISPOSITIVO.md): da I miei PG, icona di ricerca,
ID pubblico e Wi-Fi. HTTPS con orario/certificati verificati, stessa conversione
PC in QuickJS isolato e validazione C finale; anteprima con caratteristiche,
PF, CA, conteggi e note. Annullamento e conferma sul display; checkpoint della
scheda uscente sulla SD e blocco dei salvataggi incerti. Nessun cambio ai formati
PG/USB/backup/PDL1 né alle partizioni, nessuna scrittura verso Beyond.

Lettura reale del PG `167361077`: HTTP 200/JSON atteso, PF53/53, CA16,
caratteristiche15/10/16/8/8/17, 11 oggetti e 69 magie/capacità, 35 note/limiti.
Record nativo identico a quello del PC sugli stessi dati; nessuna modifica alla
raccolta personale. I vecchi PF35/53 erano il saldo della lettura precedente,
non quello restituito dal servizio oggi. La parità non prova tutte le meccaniche
D&D né il percorso Wi-Fi fisico dell’ESP32. Nessuna nuova prova privata/account.

341 test JS, 74 confronti PC/nativo (67 accettati, 7 bloccati), suite native,
145 viste LVGL e build ESP32-S3. La nuova funzione richiede una prova sul Pocket
per HTTPS/radio, memoria totale, tempi, annullamento e conferma. Associazione,
rinnovo della sessione ed elenco autenticato sul dispositivo restano successivi.

## Nuovo nella v0.90

[Raccolta locale sul Pocket](LIBRERIA-LOCALE.md): fino a 32 copie native su SD,
con anteprima, conferma e cambio offline. PF, slot, aspetto e storico separati;
preferenze e tasti del dispositivo conservati. Checkpoint prima di cambi e
import/ripristini USB, doppie copie verificate, identità d’installazione distinta,
recupero CRC e blocco dei commit incerti. Nuovo contenitore SD PDL1; formati PG,
USB21/PDDe40 e backup USB1 invariati. Il collaudo della nuova raccolta su SD
fisica rimane da fare; USB/installer/asset SD precedenti già confermati.

Account Beyond: **l’utente conferma il 31 ottobre login, PG visibili ed elenco
completo**. È una prova riferita dal suo browser, non una nuova lettura autenticata
dell’agente. Sessione sul Pocket, rinnovo e schede private non ancora verificati;
conversione nativa aggiunta nella v0.91. Nessuna credenziale o raccolta personale acquisita.

## Nuovo nella v0.89.1

[Pocket autonomo](POCKET-AUTONOMO.md): percorso concordato e verifica opzionale
dell’account nell’estensione. Browser → token temporaneo → elenco minimo, con
HTTP, JSON, sessione e catalogo distinti; nessuna estrazione cookie, credenziale
esportata o PG modificato. Test sintetici separati dalla prova autenticata ancora
da eseguire nel browser dell’utente. Non implementa conversione o importazione
diretta ESP32, né libreria multipla su SD. Firmware operativo e formati invariati.

## Nuovo nella v0.89

[Punizione Divina 2024](PUNIZIONE-DIVINA.md): scelta dopo un colpo valido, costo
ordinario/patto o uso finito, bonus del bersaglio e critico, insieme a True Strike
e Hex/Marchio. Costi e risultati congelati nelle riprove; storico atomico fino a
cinque parti, compresi critici da 22d8. Azione bonus e regole generali del turno
restano dichiarazioni del giocatore, non un bilancio automatizzato.

Correzione dell'import: massimi dalle tabelle quando sono non ambigue per un solo
mezzo-incantatore ordinario e Warlock 2024; consumi separati. Lettura reale e
parser/codec C sul PC concordano su PF35/53, CA16, slot ordinari1/2 e patto1/2;
Punizione con CAR, non preparata. Nessun tiro o USB fisico effettuato su quel PG.
PC24/USB21 per i nuovi profili, PDDe40 alle stesse dimensioni. Legacy2014,
altre Punizioni e proprietà non rappresentate rimangono manuali.

## Nuovo nella v0.88

[Hex e Marchio 2024 sul colpo](EFFETTI-SUL-COLPO.md): lancio contestuale con costo
ordinario/patto, concentrazione e conferma del bersaglio dopo un colpo riuscito.
Extra base necrotico/forza, critici, compatibilità con True Strike, totale comune
e storico atomico fino a tre parti. Profili per i PG già importati, icone comuni
e nessun attacco/CD fittizio. Identificativi Beyond 2024 verificati con fonti
ufficiali, automatismi rifiutati per origini sconosciute. Durate, azioni,
potenziamenti, bersagli e altri effetti restano espliciti/manuali.
PC23/USB20 per i nuovi profili; PDDd39 alle stesse dimensioni, vecchi salvataggi letti.
Nuova lettura reale HTTP/JSON riuscita su un PG senza questi incantesimi: i nuovi
comportamenti sono verificati con dati sintetici, non dichiarati provati da quel PG.

## Nuovo nella v0.87

[Attacchi contestuali](ATTACCHI-CONTESTUALI.md): scelta normale/magia direttamente
da Attacca, valori confrontabili, preparazione senza cambiare sezione, blocchi
visibili, icone comuni e tasti fisici locali. Nessun menu aggiuntivo se non ci sono
varianti. Cambiare modalità invalida il tiro precedente. Logica senza eccezioni
per un PG; True Strike 2024 supportato per tutte le caratteristiche da incantatore.
Altre magie con arma o attivazioni sul colpo richiedono ancora modelli espliciti.
Nessuna nuova lettura Beyond, nessuna modifica ai formati o ai dati personali.

## Nuovo nella v0.86

[Oggetti e sintonia per tutti i personaggi](OGGETTI-SINTONIA.md): importazione dei
bonus permanenti riconciliati anche senza armatura; sintonia separata dallo stato
equipaggiato, anteprima e comandi sul Pocket. Bonus a CA, caratteristiche, PF,
armi, magie e tiri aggiornati insieme; niente cura automatica. 37 icone semantiche.
Logica condivisa, verificata su tutte le caratteristiche e i livelli 1–20 con
dati sintetici, senza eccezioni per un particolare PG. Nuove proprietà PC22/USB19,
stato PDDc38 a dimensioni e partizioni invariate; vecchi salvataggi conservati.
Resistenze da oggetti, effetti condizionali, valori fissati/cap e regole speciali
non rappresentate restano manuali. Requisiti e tempi di sintonia confermati al tavolo.

## Nuovo nella v0.85

[Equipaggiamento, magie e attacchi sul Pocket](EQUIPAGGIAMENTO-MAGIE.md):
preparazione diretta, equipaggia/riponi con anteprima, editor locale delle armi
e dei bonus degli oggetti. I profili collegano caratteristiche, CA, PF massimi,
armi, magie, prove, TS e iniziativa. Le armature standard importate adottano lo
stato sorgente senza raddoppiare la CA; effetti speciali restano espliciti/manuali.
True Strike 2024 usa la caratteristica da incantatore con un attacco dell'arma,
scala i dadi radianti e gestisce i due gruppi anche su critico. Storico atomico,
risultati congelati nelle riprove e icone semantiche condivise PC/firmware.
I PG già importati possono essere configurati sul Pocket senza reimportazione.
Nuovi formati PC21/USB18/PDDb37; lettura dei formati vecchi e partizioni conservate.

## Nuovo nella v0.84

[Wi-Fi direttamente dal Pocket](WIFI.md): cerca le reti 2,4 GHz, scegli la rete,
inserisci la password con tastiera sul display e conferma. Segnale, tipo di
sicurezza, password nascosta/mostrabile, reti aperte con conferma, annullamento
e comandi fisici locali. Hotspot e portale restano un’alternativa per telefono,
SSID nascosti e abbinamento del ponte. Credenziali separate dal PG e salvate
solo dopo DHCP coerente; errore radio e salvataggio fallito restano distinti.

Compilazione ESP32 e prove del servizio con SDK simulato, 131 viste native,
ASAN/UBSAN e regressioni software. Radio/router fisici ancora da collaudare.
Formati PG, backup, protocollo USB e partizioni invariati.

## Nuovo nella v0.83

[Look SD via web e USB](SD-WEB-USB.md): la scheda resta nel Pocket; installer,
editor avatar e guida PC offrono un comando per trasferire tutti i 504 look.
CRC dei blocchi, SHA256 e verifica integrale dopo rilettura prima della conferma,
file temporaneo e copia di recupero del catalogo precedente. Nessuna formattazione
o modifica di PG/NVS; formati e partizioni invariati. Richiede firmware 0.83.
Ricerca sul TamaPoke pubblico documentata, implementazione indipendente.

Compilazione ESP32, writer/parser su filesystem host con errori iniettati,
128 viste UI native, 303 test JS e browser con USB simulata. Trasferimento e lettura
su SD fisica ancora aperti. Il pulsante Attacchi senza un’arma configurata apre
lo zaino: per le armi speciali mantenute come oggetti non vengono inventati tiri.

## Importazione da Beyond

| Area | Disponibile | Cosa manca o va controllato |
| --- | --- | --- |
| Accesso | Connettore PC; lettura/conversione nativa pubblica e percorso account/PG privati implementati | Nuovo percorso autenticato sul dispositivo da collaudare realmente; nessuna scrittura verso Beyond |
| Creazione | Anteprima, problemi bloccanti distinti dai limiti, consenso e nuovo PG | Nessun aggiornamento automatico di una vecchia scheda con conservazione dei progressi |
| Caratteristiche | Base, modificatori/scelte riconciliati, bonus personalizzati, massimi espliciti, valori fissati e override supportati | Origini/capacità sostitutive, altre restrizioni/formule e override numerici non riconosciuti |
| PF e CA | Calcolati con le caratteristiche finali e gli effetti supportati; PF correnti e temporanei distinti | Profili di equipaggiamento supportati aggiornano i valori; livello e proprietà non rappresentate richiedono riconciliazione |
| Abilità e salvezze | Competenze, maestrie, mezza competenza supportata, bonus, associazioni a caratteristiche e tiri da verificare; personalizzazioni standard | Abilità non standard, mezza competenza/maestria alle salvezze, bonus condizionali e formule |
| Armi | Configurazioni riconosciute: caratteristica, competenza, dadi, bonus, portata e alcune proprietà | Armi versatili/due armi, maestrie, munizioni e danni aggiuntivi speciali; arma attiva da scegliere se più armi sono equipaggiate |
| Inventario | Fino a 60 voci, quantità, custom, consumabili, cariche e cure esplicite; stati/peso/contenitore sorgente consultabili | Dotazioni come fotografie; peso e contenitori non gestiti; sintonia supportata per i profili riconciliati. Pile con saldi individuali e proprietà arbitrarie restano manuali |
| Magie e capacità | Fino a 120 voci, descrizioni, slot ordinari/patto, preparazione e contatori riconosciuti | Contatori manuali separati dagli utilizzi. Non tutti i recuperi o costi condivisi; magie/azioni da oggetto con usi restano note per evitare di duplicare le cariche; nessuna esecuzione generale degli effetti |
| Stato del PG | PF temporanei, annotazioni delle condizioni, dadi vita e contatori morte supportati | Le annotazioni non attivano automaticamente tutti gli effetti; concentrazione in corso e altri stati speciali da impostare/verificare |
| Difese ai danni | Concessioni incondizionate riconciliate, configurabili e trasferibili | Difese condizionali e immunità alle condizioni non sono ricostruite automaticamente; un tipo assente resta sconosciuto, non Normale |
| Monete | Cinque saldi distinti, compreso lo zero verificato | Nessun cambio, acquisto o peso automatico |

La conversione resta **parziale**. HTTP 200 prova l'accesso; JSON valido prova
il formato. Per dichiarare un dato convertito servono il controllo delle sue
origini/regole e il confronto del risultato. Una descrizione importata non
significa che il suo effetto sia automatizzato.

### Correzione v0.82.1

[Consenso Beyond e armi personalizzate](BEYOND-CONSENSO-V0821.md): riprodotto il
blocco sul PG reale 167361077. La sostituzione della caratteristica dell’arma
non è un aumento di punteggio. Personalizzazioni di un’arma con ID/namespace
riconosciuti non bloccano la scheda intera: le armi interessate rimangono oggetti
senza tiri automatici, da configurare esplicitamente. Gli altri blocchi restano.

I problemi sono visibili prima dell’avatar; se manca una bozza convertibile,
una spiegazione sostituisce la casella disabilitata. Risposta coerente e consenso
restano obbligatori. Casella/etichetta verificate con clic reali e pulsante senza
sovrapposizione. 297 test JS, browser classico/guidato e build ESP32. Valori reali
confermati dall’utente, 11 oggetti e 30 bonus concordanti nel parser C; nessuna
creazione nella raccolta reale o prova USB aggiunta. Aggiornare solo il PC è
sufficiente; firmware già presente compatibile. Formati e partizioni invariati.

### Nuovo nella v0.82

[Sessione e preparazione del collaudo](SESSIONE-V082.md): una sequenza continua
con attacco, magia/concentrazione, danni, pozione, consumabile/capacità e riposi
ricarica tutti i campi a ogni salvataggio. Due interruzioni NVS simulate conservano
la copia precedente; la riprova della pozione non tira di nuovo.

Tasti configurabili con scelta diretta, icone e annullamento; identificativi
esistenti conservati. Controllo PC della scheda attuale con collegamenti agli
editor e resoconto Beyond originale conservato. Diagnosi firmware dal browser,
solo su richiesta, e report locale degli ultimi quattro tentativi senza JSON
PG/log integrali/seriali/credenziali. READY non viene confuso con la versione
firmware; diagnosi letta non equivale a importazione salvata.

293 test JS, suite nativa completa/127 viste, UI e sessione con ASan/UBSan
(leak detection non supportata), browser di editor/quota/limiti/report e build
ESP32. Nuove prove esclusivamente sintetiche: nessuna rilettura Beyond reale,
nessuna raccolta personale modificata. USB fisico, avvio Windows e ponte
Beyond autenticato restano aperti. Formati e partizioni invariati.

### Nuovo nella v0.81

[Guida dall’installazione al tavolo](INIZIA-QUI.md) nella stessa app locale:
Pocket → Personaggio → Al tavolo. Il pacchetto Windows x64 include runtime
Node ufficiale non modificato, licenza, app compilata e installer/firmware.
Niente installazione separata di Node o Python. Lo stesso indirizzo/origine
conserva la raccolta; un secondo avvio riutilizza soltanto la stessa versione.

Chi ha già il Pocket pronto salta il firmware; chi non ha il dispositivo può
preparare il PG e rimandare. Nella guida: anteprima Beyond sintetizzata, resoconti
richiudibili, scelte estetiche, consenso ai limiti e singolo pulsante di creazione,
poi invio USB e conferma sul display. Nessun completamento dopo il solo flash,
un file scaricato o un controllo porta. L’editor del solo aspetto conserva gli
altri campi. Errori di memoria non creano duplicati o avanzamenti fittizi.

288 test JS, prove browser guidate e regressioni della UI/USB simulate,
pacchetto estratto, identità/architettura SHA256 del runtime Windows e build
ESP32 verificati. La shell/esecuzione Windows non è eseguita nell’host Linux;
USB fisico e ponte Beyond autenticato restano aperti. Nessuna lettura o scrittura
Beyond reale in queste prove. Formati USB17/PC20/PDDa e partizioni invariati.

### Nuovo nella v0.80

[Revisione dell’uso pratico](USO-PRATICO.md): spada e magie direttamente dalla
home, Danni/Cura in prima riga, menu e ricerca con icone coerenti e pressione
lunga per l’aiuto. Filtri Consumabili e Pronte insieme alla ricerca, paging
con swipe nelle liste, senza attivare righe o conferme al rilascio. Le azioni
usano ancora i controlli di regole, anteprima e persistenza esistenti.

Sul PC: importa/personalizza → verifica/trasferisci → copie/collegamenti;
i prototipi sono sotto una sezione richiudibile. 283 test JS, 126 viste native,
input pointer LVGL, verifiche di memoria e browser. È una prova software con
dati sintetici: nessuna raccolta personale modificata, nessuna prova USB,
touch o Beyond autenticato aggiunta. I problemi fisici e i limiti della v0.79
restano aperti. Nessun nuovo campo nel PG: USB17/PC20/PDDa invariati.

### Nuovo nella v0.79

[Ponte Beyond sperimentale](PONTE-BEYOND.md): ESP32 → HTTPS LAN → coda privata
sul PC → estensione Chrome/Edge → pagina Beyond autenticata. Abbinamento con
certificato e conferma sul display; chiavi separate dal PG e dai backup.
Tiri già salvati, con facce/bonus/totali invariati, e PF attuali/temporanei;
nessun secondo lancio, nessun requisito Foundry. Pausa con conservazione della
coda, riconciliazione esplicita senza reinvio, revoca e storico degli esiti.

| Livello | Esito / limite |
| --- | --- |
| HTTPS e protocollo | Certificato, SAN IPv4, autenticazione, formato e storico C verificati localmente |
| Conversione dei tiri | Confronto facce, modalità, bonus e totale; minimo zero non rappresentabile bloccato |
| Browser | UI e adattatore PF/broker su pagina sintetica; worker con API Chrome simulate. Installazione MV3 bloccata dalla policy amministrativa del browser di sviluppo |
| Ricezione dei dadi | «Osservato» richiede eco corrispondente dal broker; persistenza Beyond dopo reload non provata |
| PF | «Verificato» richiede rilettura concordante dal connettore pubblico; nel test il servizio è sintetico |
| Hardware e account | Nessuna scrittura sul tuo Beyond, nessun collaudo RF/ESP32 o estensione sulla tua sessione |

La sincronizzazione **non è dichiarata pronta per la produzione**. L’adattatore
usa interfacce interne del sito, può richiedere aggiornamenti e riconosce la
gestione PF inglese supportata. Non sono sincronizzati PF massimi, condizioni,
inventario, slot o utilizzi. La direzione inversa resta successiva. Restano
aperti il problema USB già segnalato, le prove Wi-Fi fisiche e il test controllato
su una scheda Beyond di prova. Raccolta personale e account non modificati.
USB17 / backup PC v20 / stato nativo PDD36-PDDa invariati.

### Nuovo nella v0.78

[Profilo Beyond della prima versione](BEYOND-V1.md): consultazione sul dispositivo
con identità/classi, movimento base, sensi/passivi, lingue, strumenti e dotazioni;
tiro di iniziativa con bonus riconciliato e contesto attuale. Incertezza distinta
dallo zero. Duplicati di azioni/magie non creano riserve doppie. Resoconto per PG
con regole manuali, fotografie ed eccezioni specifiche; estratti accettati.
USB17 / backup PC v20 quando c’è il profilo, PDD36 senza crescita dello stato.

[Wi-Fi](WIFI.md): rete temporanea protetta, configurazione dal telefono o PC,
salvataggio dopo connessione verificata, spegni/dimentica e sospensione della radio.
Nessuna credenziale nel PG. Prove software del servizio, non della radio fisica.
[Tempo reale](BEYOND-TEMPO-REALE.md): ricerca e progetto del ponte opzionale;
ponte sperimentale implementato nella v0.79; animazione Beyond non implementata.

La nuova lettura reale conferma i valori v0.77 sotto riportati. Aggiunge 14 pagine
di profilo (64 voci complessive), tutte confrontate nel parser/codec C: passo/volo
30 ft con restrizione del volo, scurovisione 60 ft, passivi 14/10/14 e iniziativa +4.
I dati di dotazione non disponibili restano `?`; azioni custom ed effetti generici
restano manuali. Nessuna raccolta personale modificata e nessun trasferimento USB.

### Nuovo nella v0.77

[Creazione del cannone con slot](BEYOND-CANNONE.md) nella variante riconosciuta
Artillerist di Tasha’s: prima un utilizzo gratuito per riposo lungo, poi uno
slot ordinario o patto scelto e confermato. La spesa non ripristina il gratuito,
non modifica i saldi manuali e non termina la concentrazione. PC e dispositivo
conservano i valori precedenti se il salvataggio fallisce.

Nomi simili e varianti non riconciliate non ricevono questa automazione.
Scelta del cannone attivo, durata, PF, modalità e detonazione restano manuali.
Richiede firmware 0.77 / USB16, backup PC v19; PDD35/PDDZ a dimensioni invariate.

### Disponibile dalla v0.76

[Contatori manuali delle azioni](BEYOND-CONTATORI-MANUALI.md) separati dagli
utilizzi: residui conservati e modificabili sul PC e sul dispositivo, senza
consumo o recupero automatico. Nel PG reale cinque saldi prima presentati
come utilizzi vengono distinti dalla creazione del cannone con recupero lungo.
In questa versione storica non erano automatizzati collegamenti o spese di slot;
la v0.77 aggiunge il costo della creazione riconosciuta, mantenendo manuali gli altri casi.
Backup PC v18 solo con questi contatori; USB e stato nativo invariati.

**Testi estesi rimandati per scelta dell’utente:** i tagli attuali sono accettati
per questa fase e non richiedono espansione su SD per chiudere l’importazione.

### Disponibile dalla v0.75

[Verifica rapida delle difese](DIFESE-VERIFICA-RAPIDA.md) sul dispositivo e sul PC:
anteprima dei tipi mancanti e conferma manuale per dichiararli Normali di base.
Le difese note, regole ed effetti temporanei restano conservati. Annullamento,
errore di salvataggio e riprova non applicano modifiche parziali. Il collegamento
nell’anteprima USB apre direttamente la configurazione sul PC.
Nessun tipo viene completato automaticamente durante l’importazione.

### Disponibile dalla v0.74

[Caratteristica personalizzata per abilità](ABILITA-CARATTERISTICHE.md), importabile
da Beyond e configurabile sul PC: per esempio Intimidire con FOR. La scelta
arriva al dispositivo e segue punteggi ed effetti supportati senza alterare
competenza o extra. Richiede firmware 0.74 / USB15 quando presente.
Annullamento ed errore di salvataggio dell’editor conservano la scheda.

### Disponibile dalla v0.73

Personalizzazioni standard delle abilità e salvezze: bonus numerici separati,
competenze esplicite e mezza competenza delle abilità. Un valore esplicito
Nessuna può rimuovere la competenza derivata dalla classe; Vuoto/Automatico
la conserva. Duplicati identici non si sommano e duplicati discordanti bloccano.
La somma finale degli extra deve restare fra −30 e +30.

L'anteprima mostra anche bonus su tiri senza competenza, gradi rimossi e mezza
competenza. Le personalizzazioni non eliminano blocchi causati da altri effetti
irrisolti. Oggetti a quantità zero o inattivi non forniscono bonus ai tiri;
origini o requisiti ambigui mantengono i tiri interessati da verificare.
[Formato, casi supportati e prove](BEYOND-PERSONALIZZAZIONI-TIRI.md).

### Ultima lettura reale del tuo PG

28 ottobre 2026, personaggio **144777811**, con codice v0.77 e connettore esistente:

- HTTP **200**, **426.718 byte**, struttura JSON valida.
- FOR/DES/COS/INT/SAG/CAR: **8/18/14/18/10/10**; PF **32/73**, CA **17**.
- **41 voci** d'inventario; le quantità della proiezione coincidono nel parser C.
- Quattro pozioni di cura con formula esplicita **2d4+2**.
- I **30 tiri** di caratteristica/abilità/salvezza della proiezione coincidono
  fra modello PC e calcolo C; nessuno risulta bloccato in questo sorgente.
- Cinque contatori manuali coincidono nel parser/codec C: **4/4, 1/4, 50/50,
  35/50, 50/50**. Create Eldritch Cannon è **0/1**, recupero lungo; la nuova
  creazione con uno slot ordinario coincide nel modello PC e nel calcolo C.
  La prova usa una copia temporanea: nessun saldo personale salvato.
- Nessun bersaglio con le personalizzazioni delle abilità/salvezze: per quei casi
  la prova rimane **sintetica**, non dimostrata da questo PG.

È una verifica dei dati ricevuti e della conversione, non un confronto con una
schermata autenticata Beyond. Il parser e il codec C sono stati eseguiti sul PC:
non provano il trasferimento fisico. Nessuna raccolta personale aperta o
modificata, nessun invio USB; il JSON completo non è pubblicato né negli archivi.

## Funzionamento sul dispositivo

| Funzione | Stato del software | Limite pratico |
| --- | --- | --- |
| Home, avatar, menu e dadi | Implementati; dadi animati, risultati e storico | Leggibilità, fluidità e consumo sulla scheda da collaudare con il PG reale |
| Scheda e tiri | Caratteristiche, 18 abilità, sei salvezze, bonus e modalità | Le regole non rappresentate richiedono verifica o dado libero |
| PF, cure e temporanei | Inserimento importi, anteprima/conferma, salvataggio e annullamento | Per un colpo con difese e concentrazione usare Danni, non la correzione diretta dei PF |
| Danni, effetti e turni | Difese/configurazione locale, durate, riduzioni e costi espliciti | Non è un motore completo D&D; contesto, fonti e regole speciali possono richiedere decisioni del giocatore/DM |
| Inventario e risorse | Ricerca, quantità, arma attiva, consumi, cariche e ricariche | Nuove voci dal PC; profili arma/armatura/bonus configurabili sul Pocket. Proprietà speciali manuali |
| Magie, concentrazione e riposi | Slot ordinari/patto, preparazione, utilizzi, dadi vita e recuperi supportati | Effetti, danni delle magie, eccezioni e recuperi speciali restano da gestire |
| Condizioni e morte | Annotazioni, effetti espliciti sui tiri, contatori e regole base | I nomi delle condizioni non attivano automaticamente tutti gli effetti |
| Tasti | PWR breve, BOOT breve/lungo configurabili; protezione delle conferme | Debounce e comportamento fisico da provare; le preferenze locali sono conservate all'importazione |
| Salvataggi e backup | Persistenza NVS, storico, copia/backup/ripristino USB | Mancano prove fisiche dopo riavvio, perdita di alimentazione e ripristino; backup PC e nativo sono archivi distinti |
| Alimentazione | Lettura PMIC/batteria, luminosità, pausa/risveglio e spegnimento con salvataggio implementati | Autonomia, calibrazione, ricarica e risveglio non misurati sul tuo esemplare |

Attenzione pratica: il PG attuale non dichiara un profilo completo di difese
verificate. Prima di usare il calcolo Danni vanno verificati i tipi richiesti
ed eventualmente dichiarati Normali. Questa verifica si può fare dal PC o
sul dispositivo; non viene inventata dall'assenza di modificatori Beyond.

Si gestisce **una scheda attiva sul dispositivo** e una raccolta locale fino a
32 PG su SD. Prima di un cambio/importazione viene salvata la copia uscente;
senza SD resta un solo PG attivo. Le schede PC restano conservate. Non esiste
ancora un aggiornamento selettivo che unisca dati Beyond freschi e progressi
locali della medesima scheda. Conservare comunque un backup dei progressi importanti.

Party/griglia/sincronizzazione fra dispositivi non fanno parte dell'attuale
uso offline; gli strumenti PC relativi sono prototipi locali.

## Grafica e personalizzazione

Disponibili tre ambienti, avatar originale configurabile, **21 famiglie visive**,
due silhouette e **504 look di classe** su SD. Beyond propone specie riconosciuta
e look di classe singola supportata; nei casi ambigui la scelta è manuale.

Restano espressioni/fotogrammi aggiuntivi dei look SD, Artificiere/sottoclassi,
altre illustrazioni/scene e rifiniture di icone e transizioni. I preset SD hanno
colori/taglio propri; lo stile originale conserva le proprie scelte. Il pacchetto
SD è opzionale e il trasferimento SD e l’avvio sono stati riferiti funzionanti dall’utente; nuovi look/interazioni richiedono riscontro: lo spazio
SD non sostituisce il controllo di RAM, fluidità e salvataggi sul dispositivo.

## La roadmap in quattro blocchi

| Blocco originale | Stato al 30 ottobre | Lavoro residuo |
| --- | --- | --- |
| 1. Completare la scheda sul dispositivo | Coperto per il primo uso: magie/slot, preparazione multipla, utilizzi, concentrazione, riposi, dadi vita e morte sono presenti | Eccezioni, completezza dell'import e verifica dei percorsi con il PG reale |
| 2. Gestire effetti e regole | Parziale: diversi effetti sono configurabili localmente; l'import sa ricostruirne solo un sottoinsieme | Regole condizionali, proprietà speciali, ricalcolo e collegamenti automatici alle fonti |
| 3. Prodotto fisico | Avvio, import USB, installer e SD riferiti funzionanti dall'utente | Nuovi flussi 0.85–0.89, touch/tasti, batteria, prestazioni e tenuta dei salvataggi dopo interruzioni |
| 4. Estetica | Base del compagno e selezione di specie/classi disponibili | Espressioni dei nuovi look, ulteriori sprite/scene e rifiniture dopo visione sul display |

Non assegno una percentuale unica: le funzioni riferite funzionanti sull'hardware
sono distinte da quelle nuove ancora verificate solo tramite software.

## Prossimi blocchi software, oltre alle prove hardware

1. **Magie ed effetti:** profili espliciti per cure e danni degli incantesimi,
   condizioni, durate e proprietà concesse dagli oggetti. Un testo importato non
   equivale a un effetto eseguibile. Restano requisiti/limiti di preparazione.
2. **Combattimento e dotazioni:** impugnature versatili/due mani, interazione con
   gli scudi, munizioni, due armi, maestrie e altre proprietà speciali. Nessuna
   regola dedotta soltanto dal nome dell’oggetto.
3. **Rilettura Beyond della stessa scheda:** confrontare i dati nuovi con la copia
   locale e riconciliare progressi, aspetto e risorse. Oggi l’importazione crea
   una nuova copia: non esiste una fusione automatica verificata.
4. **Autonomia e raccolta:** ulteriori collegamenti fra elenco Beyond e copie
   locali, gestione dei duplicati e ulteriori riduzioni dei passaggi frequenti.
   Il nuovo accesso autenticato sull’ESP32 rimane da provare; l’estensione è
   confermata come soluzione per la prima associazione.
5. **Rifiniture:** espressioni/animazioni dei look SD e feedback delle nuove
   azioni. Tenere disponibili testo esplicativo e simboli comuni riconoscibili.

Scrittura in tempo reale verso Beyond e collaborazione fra dispositivi restano
blocchi distinti. Non sono dichiarati funzionanti dalla sola importazione.

## Ordine consigliato per arrivare a una prima sessione

1. **Conservare il percorso USB già funzionante.** Non ripartire dai vecchi
   blocchi di installer/SD come se fossero ancora aperti. Fare un backup prima
   di aggiornare o di sostituire una scheda con progressi importanti.
2. **Confrontare il PG sul display:** caratteristiche, PF, CA, arma attiva,
   quantità, pozioni, slot e tiri principali; configurare difese e tasti.
3. **Verificare i progressi:** modificare un saldo, riavviare, ritrovarlo,
   acquisire un backup e provarne il ripristino con un caso controllato.
4. **Provare una breve sessione:** un attacco, danni, cura, magia/concentrazione,
   spesa/recupero di una risorsa e un riposo. Poi correggere gli attriti osservati.
5. **Ampliare l'import** sui casi concreti rimasti e completare gli altri dati
   consultabili sul dispositivo, poi continuare espressioni e rifiniture grafiche.
   I testi possono restare negli estratti attuali, come concordato.

Direzione del prodotto confermata: [Pocket autonomo](POCKET-AUTONOMO.md).
L’elenco account reale è confermato dall’utente e la raccolta locale SD è nel
codice della v0.90, da collaudare sull’hardware. Conversione nativa, associazione e importazione
Beyond dal Pocket sono ora nel codice delle versioni 0.91–0.94; il collaudo
autenticato su hardware rimane distinto dall’elenco già confermato nel browser.

Non serve attendere tutte le regole D&D per questa prima sessione. Servono un
trasferimento verificato, valori confrontati e progressi recuperabili.

## Verifiche v0.90

Suite JS e nativa, 142 viste LVGL incluse le nuove pagine, engine SD su filesystem
host con errori iniettati e identità NVS con API simulate. ASan/UBSan sul nuovo
engine e sull’interfaccia; build ESP32-S3. Nessuna nuova prova USB/SD fisica
eseguita dall’agente. La conferma reale dell’elenco Beyond proviene dall’utente.
[Dettagli e procedura di collaudo](LIBRERIA-LOCALE.md).

## Verifiche v0.89.1

339 test JS e pagina account a 900/390/320 px con permessi simulati e risposte
sintetiche, compilazione ESP32-S3 riuscita. Il caricamento MV3 è bloccato dalla
policy del browser gestito: permessi e sessione reali non verificati qui.
Nessun accesso autenticato o modifica a PG personali; formati e logica firmware
invariati, solo versione aggiornata. I test nativi della v0.89 non sono ripetuti
come prova di questa nuova verifica browser. Dettagli in [Pocket autonomo](POCKET-AUTONOMO.md).

## Verifiche delle fasi precedenti e compatibilità

**258 test JS**, browser di produzione con profili isolati, **324 schede
sintetiche** nel parser/codec C per le nuove associazioni, undici pacchetti
incompatibili respinti e regressioni dell'intera scheda. Parser/codec e UI host verificati anche
con ASan/UBSan. Compilazione ESP32-S3 riuscita. Tutti gli **8.192 sottoinsiemi**
delle difese verificati in JS e C; **120 schede sintetiche** per i nuovi contatori,
**108 sorgenti / 2.052 scelte di pagamento** per la nuova creazione e **119 viste**
del renderer. Questi risultati
non certificano touch, trasporto USB o memoria fisica.

Compatibilità: **USB16 / backup PC v19** per il costo della creazione del cannone,
con app e firmware 0.77. Le associazioni delle abilità richiedono almeno USB15 /
v17; i contatori manuali almeno app PC 0.76 / v18. Senza i campi nuovi si conserva
il minimo formato precedente pertinente.
Stato nativo **PDD35/PDDZ**, dimensioni e partizioni invariate, lettura dei formati
precedenti. Per leggere/ripristinare i backup nativi nuovi usare app e firmware 0.77.
Il profilo difese richiede almeno 0.71 / USB14; i look classe almeno 0.70 e il
pacchetto SD. Completa sul dispositivo richiede 0.75. L’installer distribuisce 0.77.
I testi rimangono negli estratti concordati.
