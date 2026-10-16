# Formato scheda locale — v8

Il backup individuale è un singolo JSON UTF-8. Non è un formato D&D Beyond. Dalla versione 0.9 l'app conserva una raccolta di fino a 20 personaggi, uno attivo alla volta. Lo schema individuale rimane v8; il formato della raccolta è descritto in fondo al documento.

| Campo | Significato e limiti |
| --- | --- |
| `version` | `8`; lettura compatibile con v1–v7 |
| `name`, `title` | Nome non vuoto fino a 24 caratteri, titolo fino a 40 |
| `level` | Intero da 1 a 20 |
| `maxHp`, `hp` | Massimi da 1 a 999, attuali da 0 ai massimi |
| `armorClass` | Intero da 1 a 30, inserito manualmente |
| `weaponReach` | Portata manuale dell’arma, intero da 1 a 9 caselle |
| `abilities` | FOR, DES, COS, INT, SAG, CAR: interi da 1 a 30 |
| `avatar` | `skin`, `hair`, `cloak`, `style`: indici da 0 a 2 nelle palette locali |
| `condition` | Booleano: avvelenato |
| `bonus`, `mode` | Modificatore del dado libero (-5…15) e normal/advantage/disadvantage |
| `swordEquipped` | Booleano |
| `potions`, `torches` | Quantità intere da 0 a 99 |
| `resolve` | Risorsa narrativa Slancio, intero 0 o 1 |
| `skillRanks` | Mappa dei 18 identificatori in `src/model.js`: 0 senza competenza, 1 competente, 2 maestria |
| `saveProficiencies` | FOR, DES, COS, INT, SAG, CAR: booleani |
| `history` | Fino a 20 tiri, dal più recente: label, dice, bonus, total, kind, mode, timestamp |
| `powers` | Fino a 40 magie/capacità manuali, con ID univoco |
| `slots` | Nove oggetti `{max,current}`, livelli 1–9; quantità 0–9 |
| `spellcastingAbility` | Una delle sei caratteristiche |
| `concentration` | `null` oppure ID di una magia della lista che richiede concentrazione |
| `encounter` | Partecipanti, turno, round e danni in attesa del combattimento locale |

L'importazione rifiuta versioni sconosciute, campi necessari mancanti e valori fuori limite prima di sostituire lo stato. I campi sconosciuti non sono conservati. Il file è limitato a 250 KB dall'interfaccia. Nome e titolo vengono trattati come testo.

I backup v1/v2 ricevono il profilo originale di Elian e conservano PF, condizioni, impostazioni dei dadi e, per v2, consumi ed equipaggiamento. I backup v3 conservano il profilo personalizzato. Tutte le versioni precedenti alla v4 inizializzano le competenze a zero e lo storico vuoto; non si inferiscono competenze dalla classe. La migrazione della raccolta conserva la chiave localStorage storica senza modificarla.

## Valori derivati

- Modificatore caratteristica: `floor((punteggio - 10) / 2)`.
- Competenza: `2 + floor((livello - 1) / 4)`.
- Spada dimostrativa: Forza + competenza al tiro per colpire; 1d8 + Forza ai danni, 2d8 + Forza al critico, minimo zero danni.
- Cambiare livello o Costituzione non ricalcola i PF massimi; la CA resta manuale.
- Prove semplici: modificatore della caratteristica. Abilità: modificatore + competenza × rango (0, 1 o 2). Salvezze: modificatore + competenza se selezionata.
- Avvelenato impone svantaggio ad attacchi, prove semplici e abilità; non alle salvezze. Se è selezionato vantaggio, i due si annullano. Il dado libero mantiene la modalità manuale.
- Il tiro selezionato mantiene il proprio contesto e calcola il bonus al momento del lancio. Per inserire un bonus manuale si passa a “Dado libero”.

Modificare il profilo conserva PF attuali e consumi, limitando i PF al nuovo massimo. Creare un nuovo eroe parte a PF pieni e con l'inventario dimostrativo e lo aggiunge alla raccolta. L'editor è una funzione dell'app di supporto nel browser: non si propone di inserire un'intera scheda sul piccolo display.

Questo schema è sperimentale. In futuro serviranno definizioni del personaggio e stato di sessione separati, versione esplicita del regolamento, inventario estensibile e migrazioni dedicate.


## Storico

`kind` è `d20` o `damage`; `mode` è `normal`, `advantage` o `disadvantage`. `timestamp` è un timestamp JavaScript in millisecondi. Ogni voce conserva i dadi effettivamente estratti, il bonus e il totale; il parser verifica coerenza del totale, numero e intervallo dei dadi. Le etichette sono testo fino a 80 caratteri. I danni hanno uno o due d8, con totale minimo zero.

Si registrano prove, salvezze, abilità, dado libero, attacchi e danni. Le pozioni non entrano nello storico dei tiri. Il lancio non riceve un esito automatico di successo/fallimento fuori dall'attacco: per prove e salvezze decide il DM. Un 1 o 20 naturale su una prova non implica un effetto automatico.

Annullare una modifica di PF, inventario, condizione o profilo conserva lo storico dei tiri successivi. Si conserva un solo livello di annullamento, non persistente dopo il riavvio e azzerato al cambio personaggio. Le operazioni sulla raccolta (creazione, duplicazione, eliminazione e importazione) non rientrano nell'annullamento della singola scheda.


## Magie e capacità (v5)

La migrazione v1–v4 conserva il profilo esistente e inizializza una lista vuota, nove livelli di slot a zero e nessuna concentrazione. Non aggiunge contenuti o risorse sulla base del livello o della classe.

Ogni voce ha `id` (1–64 caratteri alfanumerici, trattino o underscore), `type` (`spell` o `feature`), `name` (1–40 caratteri) e `description` (fino a 600 caratteri). Tutti i testi sono trattati come testo, non HTML.

- Magia: `level` (0–9), `concentration` (booleano). Il livello zero non consuma slot; un livello superiore richiede uno slot disponibile di livello almeno uguale. Usare uno slot superiore cambia solo il consumo: gli effetti vanno risolti manualmente.
- Capacità: `maxUses` (1–99), `remaining` (0–massimo), `recovery` (`short`, `long`, `manual`). La modifica della voce richiede un valore esplicito degli utilizzi rimasti e non li ricarica automaticamente.

La concentrazione è unica e viene sostituita solo dopo conferma nell'interfaccia. Una magia che non richiede concentrazione lascia quella corrente attiva. Eliminare o modificare la voce in modo incompatibile termina la concentrazione; arrivare a zero PF la termina. Per i danni a PF positivi compare un promemoria del controllo con il DM, senza risoluzione automatica. Non si gestiscono durate, componenti, preparazione, rituali o requisiti di classe.

Riposo breve: recupera solo le capacità con `recovery: short`. Riposo lungo: recupera PF, Slancio, tutti gli slot e capacità con recupero breve/lungo; termina la concentrazione. Entrambi preservano condizioni, inventario e capacità a recupero manuale. Non sono automatizzati dadi vita, recupero di slot da patto o restrizioni dei riposi: questi si gestiscono manualmente.

Il bonus di attacco magico mostrato è modificatore della caratteristica scelta + competenza; la CD mostrata è 8 + tale bonus. Non include modificatori speciali o effetti di oggetti.


## Combattimento locale (v6)

I backup v1–v5 inizializzano un combattimento vuoto; le altre migrazioni restano invariate. `encounter` contiene:

- `participants`: fino a 12 partecipanti. L'eroe è `{id: "hero", initiative}`: PF, CA e nome vengono letti direttamente dalla scheda, senza copie. Gli altri hanno `id`, `name` (1–40 caratteri), `initiative` (-10…50), `maxHp` (1–9999), `hp` (0–massimi), `armorClass` (1–30), `side` (`ally` o `enemy`). Gli ID sono univoci, 1–64 caratteri alfanumerici, trattino o underscore.
- `activeId`: `null` in preparazione, ID esistente durante il combattimento.
- `round`: 0 in preparazione, 1–9999 in combattimento. L'avvio richiede almeno due partecipanti.
- `strike`: `null` oppure `{id,targetId,amount,applied}`. Il danno resta in attesa finché viene confermato; il flag `applied` impedisce di applicare due volte quel risultato e viene salvato nel backup.

L'ordine è iniziativa decrescente; le parità mantengono l'ordine di inserimento. Il passaggio dall'ultimo partecipante al primo incrementa il round. I partecipanti a zero PF non vengono saltati automaticamente: il DM gestisce salvezze, morte e altre eccezioni. Il roster e l'iniziativa si modificano solo in preparazione; terminare il combattimento conserva partecipanti e PF.

Nella vista Arma si può scegliere un bersaglio del roster (escluso il proprio eroe) o mantenere la CA manuale. L'attacco usa la CA del bersaglio; dopo un colpo e il tiro danni compare un comando esplicito per applicarli ai suoi PF. Cambiare bersaglio, iniziare un nuovo attacco, cambiare turno o modificare il roster invalida i danni in attesa. Annullare l'applicazione ripristina i PF e permette di applicare nuovamente quel risultato se desiderato.

Il controllo evita un secondo utilizzo dello stesso risultato nella sessione locale, anche dopo riavvio. Non è ancora un protocollo per eventi distribuiti o una protezione da modifica manuale dei backup. Non si automatizzano resistenze, immunità, reazioni, economia delle azioni, griglia o rete; il DM può correggere i PF manualmente. Il tracker non blocca gli attacchi fuori turno.


## Griglia locale (v7)

Ogni partecipante possiede `speed`, movimento intero da 0 a 18 caselle per turno, predefinito a 6 quando manca. È un valore manuale: non viene ricavato da specie, classe o equipaggiamento.

`encounter.grid` contiene:

- `walls`: elenco di celle bloccate, senza duplicati.
- `positions`: mappa dagli ID dei partecipanti a una cella; le pedine non ancora piazzate non compaiono.
- `spent`: movimento già usato nel turno attuale, tra zero e la velocità del partecipante attivo; in preparazione vale zero.

La mappa è fissa a 9 × 9. Le celle sono numerate 0–80: `cella = riga * 9 + colonna`, con riga e colonna da 0 a 8. Non sono consentiti sovrapposizioni, pedine sopra muri o riferimenti a partecipanti inesistenti. La rimozione di un partecipante elimina anche la sua posizione.

Il percorso usa otto direzioni con costo di una casella per passo, anche diagonale. I muri e tutte le altre pedine bloccano il passaggio; una diagonale è bloccata quando una delle due celle ortogonali adiacenti è occupata. Si cerca il percorso più breve entro il movimento residuo. Queste sono regole semplificate esplicite del prototipo, non tutte le varianti di movimento di D&D.

Si muove solo la pedina del partecipante attivo, se piazzata e con PF positivi. Inizio combattimento, cambio turno e termine azzerano il movimento usato; non cambiano le posizioni. L'annullamento ripristina posizione e movimento. Un movimento invalida i danni in attesa. Le modifiche a muri e piazzamenti sono consentite solo in preparazione.

I backup v6 privi di griglia mantengono il combattimento e aggiungono una mappa vuota; quelli precedenti mantengono le migrazioni già descritte. I backup v7 devono includere la griglia. La gestione dei soli turni può funzionare senza pedine piazzate; per usare la mappa occorre prepararla prima di avviare il combattimento.

L’incremento v8 aggiunge zoom e controlli geometrici descritti sotto. Restano esclusi copertura, terreno difficile, scatto, attacchi di opportunità, movimento forzato e mappe superiori a 9 × 9. Non è ancora presente sincronizzazione di rete.


## Vista e controllo geometrico dell'arma (v8)

`weaponReach` viene aggiunto con valore 1 ai backup precedenti. Modificarlo dall'editor è una scelta manuale da concordare con il DM; non dipende automaticamente dall'equipaggiamento.

La mappa può mostrare tutte le 9 × 9 caselle o una finestra 5 × 5 con caselle più grandi. I comandi laterali cambiano zoom, centrano sul partecipante attivo e spostano la finestra entro i bordi. Dopo un movimento o un cambio turno la vista segue la pedina attiva. In preparazione, o quando la pedina attiva non è piazzata, si usa l'eroe oppure il centro della mappa. Zoom e finestra non cambiano le coordinate del gioco e sono preferenze temporanee, non salvate nel backup.

Toccare una pedina diversa dall'eroe la seleziona come bersaglio e mostra il comando per aprire l'attacco. La mappa evidenzia le celle attraversate dalla linea geometrica dall'eroe al bersaglio. Il controllo misura la distanza come massimo della differenza di riga e colonna, coerentemente con il passo diagonale da una casella del prototipo.

La linea di vista è il segmento tra i centri delle due celle. Tutte le celle toccate vengono controllate contro i muri; quando il segmento passa esattamente da un angolo vengono controllate anche entrambe le celle laterali. Altre pedine non bloccano questa vista geometrica. Non si modellano altezze, illuminazione, copertura parziale, invisibilità o regole specifiche delle magie.

Se entrambe le pedine sono piazzate, l'attacco con l'arma è disabilitato quando fuori portata o bloccato da muri. **Il DM consente questo attacco** abilita una singola eccezione: il tiro viene marcato `DM` nello storico e l'override viene consumato. La scelta non è salvata e viene azzerata quando cambiano bersaglio o contesto del combattimento. I calcoli dei danni restano invariati.

Se manca una posizione o si usa la CA manuale, distanza e vista sono dichiarate non verificate e si lasciano al DM. Questi controlli riguardano la singola arma dimostrativa, non costituiscono un validatore completo delle regole e non limitano attacchi fuori turno.

## Raccolta personaggi (app 0.9, formato raccolta v1)

```json
{
  "type": "pocketdd-library",
  "version": 1,
  "activeId": "identita-locale",
  "characters": [
    {"id": "identita-locale", "state": {"version": 8, "...": "campi della scheda"}}
  ]
}
```

L'esempio è schematico: `state` deve contenere una scheda completa valida. La raccolta ha da 1 a 20 personaggi, con ID univoci di 1–64 caratteri alfanumerici, trattino o underscore. I nomi possono coincidere. `activeId` deve riferirsi a un elemento esistente. I nuovi ID sono generati con il generatore crittografico del browser. L'importazione della raccolta è limitata a 5.000.000 byte UTF-8 e valida tutte le schede prima di sostituire alcun dato. Le esportazioni contengono schede v8; in lettura sono accettate anche le versioni individuali precedenti.

`src/library.js` gestisce raccolta, validazione e persistenza. La chiave nuova è `pocketdd-library-v1`. Se è assente, viene letta `pocketdd-demo-v1` e il suo personaggio viene inserito nella raccolta con ID `first-hero`. La raccolta viene salvata all'avvio, preservando la chiave storica. Una raccolta esistente ha priorità; se è corrotta non si ripiega sul vecchio salvataggio e non la si sovrascrive con la demo. La UI offre una sessione temporanea, il download del contenuto originale e il ripristino esplicito da una raccolta valida.

Una scrittura confronta il contenuto persistente con quello letto in precedenza per rifiutare i salvataggi di una scheda browser già non aggiornata. Non è sincronizzazione multiplayer né un protocollo di fusione: per recuperare il lavoro temporaneo si esporta la raccolta e si ricarica. In caso di quota esaurita il contenuto persistente precedente resta valido e l'archivio in memoria resta esportabile.

Ogni elemento include l'intero combattimento locale; non collega i personaggi della raccolta come partecipanti dello stesso party. Cambiare personaggio ripristina la sua sessione e azzera solo i controlli temporanei dell'interfaccia. Duplicare conserva anche lo stato corrente del combattimento nella copia indipendente. Eliminare l'attivo seleziona il primo elemento rimanente; eliminare un altro personaggio mantiene la selezione.
