> 30 ottobre, v0.89.1: [obiettivo Pocket autonomo e verifica account](POCKET-AUTONOMO.md).
> La verifica browser è separata dall’importazione ESP32 e dalla sincronizzazione.

> Aggiornamento v0.79: la proposta sotto è stata seguita da una prima
> [implementazione sperimentale del ponte](PONTE-BEYOND.md). HTTPS, coda,
> estensione e conversione verificati con dati sintetici. La ricezione e
> persistenza reali sul tuo account e il percorso ESP32 restano da collaudare.
> Nessun aggiornamento Beyond reale è stato eseguito durante lo sviluppo.

# Tempo reale con Beyond: metodo proposto dopo la prima versione

Ricerca del 28 ottobre 2026. Nessuna scrittura Beyond o invio di dadi effettuato.
Il Wi-Fi della v0.78 è una base di rete, non questa integrazione.

## Cosa è accertato e cosa no

| Canale | Evidenza | Limite |
| --- | --- | --- |
| Character service | Lettura pubblica reale HTTP 200/JSON e conversione verificata | Non autentica il proprietario e non autorizza aggiornamenti |
| Registro dei dadi | Codice pubblico Beyond20 usa `window[Symbol.for("@dndbeyond/message-broker-lib")]`, sottoscrizione e dispatch di eventi `dice/roll/…` | Interfaccia interna, non API pubblica stabile; da provare con una sessione Beyond dell’utente |
| Dadi 3D del sito | Beyond20 aziona la barra dei dadi e attende il risultato del sito | Produce un tiro del sito: non prova che si possano animare facce già determinate da Pocket |
| PF | Beyond20 osserva PF/PF massimi/temporanei dalla pagina e invia `hp-update` verso le sue integrazioni | Non dimostra scrittura dei PF verso Beyond. Non è stato verificato un endpoint di aggiornamento utilizzabile da Pocket |

Fonti lette, senza copiarne codice nel prodotto: repository
[Beyond20, commit 5edd6af](https://github.com/kakaroto/Beyond20/tree/5edd6af627557ebb1137d9a64c38a7ce3ea69006),
[message-broker.js](https://github.com/kakaroto/Beyond20/blob/5edd6af627557ebb1137d9a64c38a7ce3ea69006/src/dndbeyond/base/message-broker.js),
[digital-dice.js](https://github.com/kakaroto/Beyond20/blob/5edd6af627557ebb1137d9a64c38a7ce3ea69006/src/dndbeyond/base/digital-dice.js),
[character.js](https://github.com/kakaroto/Beyond20/blob/5edd6af627557ebb1137d9a64c38a7ce3ea69006/src/dndbeyond/base/character.js).
È evidenza di fattibilità da codice pubblico, non collaudo di eventi autenticati.
Non richiede installazione di Beyond20, Foundry o altri VTT.

## Prima soluzione pratica

**Pocket via Wi-Fi → ponte locale sul PC → piccola estensione browser →
la tua pagina Beyond già aperta e autenticata.** La stessa coda eventi potrà
avere un trasporto USB di riserva. Il gioco resta sul Pocket; il ponte è
opzionale e serve durante la sincronizzazione. Il login Beyond resta nel
browser, senza cookie o password dell’account sull’ESP32.

Il server PC attuale ascolta soltanto sul loopback: non è già accessibile
all’ESP32. Il futuro ponte richiederà un servizio LAN dedicato e abilitato
esplicitamente, pairing sul dispositivo, collegamento TLS con certificato
vincolato e autenticazione del Pocket. L’estensione mantiene il token del
ponte fuori dal contesto della pagina; consente solo eventi/PG abbinati.
Se PC, browser o pagina non sono disponibili, il Pocket mostra offline.
Un collegamento autonomo dell’ESP32 a Beyond richiederebbe prima un metodo di
autenticazione appropriato e un canale di scrittura verificato; non è promesso.

## Dadi: un risultato solo

Il Pocket genera, salva nello storico e anima il tiro. Il ponte invia al registro
Beyond **le stesse facce, formula, bonus e totale**, associati allo stesso PG e
ID dell’evento. Non si clicca “tira” anche sul sito: quel comando genererebbe
un altro risultato. Prima prova: verificare pubblicazione e ricezione esatta
nel registro, destinazione personale/campagna e gestione dei messaggi.

L’animazione 3D Beyond con quelle facce è una prova separata. Finché non è
verificata, il requisito realistico è lo stesso risultato nel registro e
l’animazione sul Pocket. Se si scegliesse invece Beyond come generatore,
il Pocket dovrebbe aspettare e animare il risultato ricevuto; cambierebbe
il comportamento offline. Le due modalità non vanno mescolate.

## PF: stato verificato e conflitti visibili

Separare PF attuali, massimi, temporanei, danni e guarigioni. Prima prova su
una scheda di test: l’estensione osserva il valore attuale, usa i normali
comandi della pagina autenticata e rilegge la scheda dopo il salvataggio.
Un eventuale endpoint di aggiornamento potrà sostituire questo adattatore
soltanto dopo aver verificato autenticazione, formato e persistenza.

Un evento contiene stato prima/dopo e identità della scheda. Se Beyond è già
cambiato, si ferma l’invio e si chiede quale stato conservare. Nessuna fusione
silenziosa di danni, cure o PF temporanei. La serializzazione sul ponte evita
concorrenza locale, ma **non garantisce un confronto-e-scrittura atomico sul
server Beyond**: altre finestre/dispositivi possono ancora cambiare il PG.
Senza un’API che lo supporti, questa limitazione va dichiarata e verificata.

## Affidabilità da implementare

- Identità e pairing per dispositivo/PG; avvio e restore richiedono una nuova
  riconciliazione prima di inviare vecchi eventi.
- Coda persistente solo dopo il salvataggio del risultato locale, ID evento
  univoco e deduplicazione. Mai ritirare un dado durante una ritrasmissione.
- Stati visibili: in attesa, inviato, confermato, esito incerto, conflitto.
  Un timeout dopo invio non permette di ripetere alla cieca un danno o un tiro.
- Conferma dal canale remoto e rilettura dei dati: HTTP 200 o dispatch locale
  da soli non significano che il valore sia stato conservato.
- Eventi con origine per impedire rimbalzi Pocket → Beyond → Pocket.
- Limiti degli adattatori rilevati esplicitamente: se cambia il sito o manca
  il broker non vengono simulati successi.

## Ordine della prossima implementazione

1. Collaudo Wi-Fi/USB e backup sul dispositivo; il problema USB attuale resta aperto.
2. Ponte locale con pairing e simulatore di eventi; nessuna scrittura reale.
3. Lettura degli eventi della pagina autenticata e prova di un unico tiro nel
   registro su una scheda di test, con facce/totale identici e deduplicazione.
4. Prova controllata PF attuali/temporanei e rilettura dopo riavvio della pagina.
5. Disconnessioni, doppie finestre, timeout dopo invio, restore e conflitti.
6. Solo poi sincronizzazione bidirezionale e studio dell’animazione del sito.
