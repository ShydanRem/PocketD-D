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

## Attivazione iniziale su GitHub

Il repository `ShydanRem/PocketD-D` è privato. Il ramo **gh-pages** contiene
l’output distribuibile, separato dal codice firmware e dalla cronologia di main.
Il sito espone i file compilati, i look e il pacchetto PC opzionale (che include
il codice JS necessario all’esecuzione locale), mai raccolte o credenziali.

Una volta pubblicato il ramo, il proprietario apre:

**Repository → Settings → Pages → Build and deployment**

- Source: **Deploy from a branch**.
- Branch: **gh-pages**, cartella **/ (root)**.
- **Save**.

GitHub mostra poi il link del sito; il percorso previsto è
`https://shydanrem.github.io/PocketD-D/`. La sola esistenza del ramo non dimostra
che Pages sia attivo: verificare il completamento della distribuzione GitHub.
Il connettore di questa sessione consente il push ma non espone la modifica
delle impostazioni Pages; questa prima impostazione può richiedere l’intervento
del proprietario. Nessun token va incollato in chat o nella pagina.

Pages da repository privato dipende dal piano GitHub. Se GitHub richiede un piano
diverso, non rendere pubblico automaticamente il progetto: il repository rimane
privato. Un repository pubblico separato contenente solo il sito è un’alternativa
da concordare. L’installer precedente resta utilizzabile durante l’attivazione.

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
Nessun nuovo flash fisico, accesso Beyond o modifica della raccolta personale.
