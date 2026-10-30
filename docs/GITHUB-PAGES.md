# PocketD-D dal web, senza scaricare installer

La pagina ha firmware e SD in primo piano: Chrome/Edge su PC, cavo USB dati,
scelta della porta e verifica. **Aggiorna PocketD-D** è la scelta iniziale;
il PG e i progressi restano nelle partizioni conservate. Per una scheda nuova
scegliere esplicitamente Prima installazione.

Il pacchetto PC è chiuso sotto **Importa Beyond e configura dal PC · opzionale**.
Serve ancora per la lettura Beyond tramite connettore locale: GitHub Pages
ospita file statici, non esegue il server del connettore. Se il pacchetto è già
presente sul PC non occorre riscaricarlo per aggiornare firmware o look SD.
La pagina non legge la raccolta del browser usata dall’app PC locale.

## Stato della pubblicazione

Sito attivo: **https://shydanrem.github.io/PocketD-D/**, confermato visibile dal
proprietario. Il repository è ora **pubblico**, per scelta del proprietario;
la pubblicazione non ne cambia la visibilità. Impostazioni Pages:
**Deploy from a branch → gh-pages → / (root)**. Ogni aggiornamento del ramo
attiva la distribuzione GitHub; verificarne il completamento nelle Actions.
Il sito contiene solo i file previsti dal publisher, mai raccolte o credenziali.

## Pubblicare gli aggiornamenti successivi

Dopo il normale build del firmware e il commit del sorgente:

```sh
npm run installer:build
npm run pages:prepare
# verificare la cartella e il sito sotto /PocketD-D/
npm run pages:publish
```

Il firmware deve essere compilato con la versione corrente. Il publisher verifica
commit/versione del manifesto, aree e SHA256 del firmware, catalogo SD, parti e
SHA256 complessivo del pacchetto PC. Accetta solo i percorsi dell’installer;
JSON arbitrari, symlink, file inattesi e oggetti oltre 16 MiB interrompono il
processo prima del push. Lo ZIP PC grande non viene committato: il browser
ricostruisce lo stesso ZIP dalle parti verificate solo su richiesta.

`pages:prepare` produce `../PocketDD-GitHub-Pages`; non scrive su GitHub.
`pages:publish` richiede main pulito e manifesto dell’esatto commit, prepara
una copia Git temporanea e aggiorna **gh-pages** senza force push. Le pubblicazioni
precedenti restano nella cronologia del ramo. `.nojekyll` evita trasformazioni
Jekyll. `deployment.json` registra commit sorgente e hash dei file pubblicati.
GitHub Pages aggiorna il sito dopo ogni push al ramo configurato: il firmware
non dipende da download manuali dell’utente o da un Site ChatGPT.

## Controlli e limiti

Pagina verificata localmente sotto **/PocketD-D/**, con firmware/SD visibili,
modalità aggiornamento, SHA256, porta USB simulata, PC opzionale e scaricamento
verificato, percorsi delle immagini e ritorno dalla galleria, 1100/390/320 px.
La pubblicazione Git e l’attivazione/distribuzione di Pages sono stati distinti.
Questa verifica del sito non attesta un nuovo flash o Wi-Fi fisico. Le letture
Beyond e le prove firmware sono documentate separatamente.
