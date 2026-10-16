# Proposta di prodotto

## Visione

PocketD-D deve sembrare un piccolo oggetto fantasy personale e risultare utile durante una sessione: consultare la scheda, effettuare tiri e aggiornare risorse senza interrompere il gioco.

Ipotesi iniziale da discutere: si gioca al tavolo con un Dungeon Master. Il dispositivo assiste il giocatore; il DM mantiene il controllo delle decisioni e delle eccezioni. Il gioco senza DM è un possibile progetto futuro e non è un requisito del primo prototipo.

## Principi

- Funzioni principali disponibili offline dopo il trasferimento del personaggio.
- Azioni frequenti raggiungibili con pochi tocchi; testo lungo in viste dedicate.
- Pixel art coerente, animazioni brevi e risultato dei dadi sempre leggibile.
- Valori calcolati spiegabili: mostrare dado, bonus e totale separatamente.
- Modifiche manuali consentite per gestire decisioni del DM ed eccezioni.
- Nessun account obbligatorio per utilizzare una scheda locale.

## Esperienza sul display rotondo

La zona centrale ospita contenuti e pulsanti; il bordo può mostrare indicatori, evitando testo e controlli importanti nelle parti tagliate dal cerchio. Dimensioni dei bersagli touch e leggibilità andranno provate sul dispositivo.

| Vista | Contenuto | Azione principale |
| --- | --- | --- |
| Eroe | Avatar animato, PF attuali/massimi, CA | Aprire i controlli PF |
| Scheda | Caratteristiche, abilità, tiri salvezza | Toccare una voce per tirare |
| Azioni | Attacchi e capacità selezionate | Risolvere un tiro con bonus visibili |
| Zaino | Oggetti, quantità, equipaggiamento | Consultare o cambiare una voce |
| Risorse | Slot e utilizzi limitati | Consumare o ripristinare con conferma |
| Dadi | Tipo di dado e modificatore | Tirare e vedere il dettaglio |

Navigazione proposta: scorrimento laterale tra sezioni, tocco per aprire, pulsante indietro visibile. Non affidare funzioni indispensabili a gesti nascosti. Ridurre i gesti in conflitto con lo scorrimento degli elenchi.

## Primo prototipo: una sequenza completa

1. Accendere e visualizzare un eroe dimostrativo originale.
2. Aprire una caratteristica ed effettuare un tiro di d20 con bonus.
3. Vedere animazione, risultato naturale, modificatore e totale.
4. Modificare i PF, annullare un errore e applicare una condizione annotata.
5. Riavviare e ritrovare lo stato salvato.

Il tiro deve essere calcolato indipendentemente dall'animazione. Un 20 naturale viene mostrato come tale; gli effetti dipendono dal tipo di tiro e dal regolamento scelto.

Non fanno parte di questo traguardo: importazione Beyond, editor completo, incantesimi automatizzati, griglia o multiplayer.

## Roadmap

| Fase | Risultato | Criterio di completamento |
| --- | --- | --- |
| 0 — Verifica hardware | Esempio ufficiale con display, touch e SD se supportata | Disegno e input funzionanti; lettura/scrittura verificata |
| 1 — Prova dell'esperienza | Simulazione interattiva circolare con dati originali | Sequenza del primo prototipo completabile con mouse/touch |
| 2 — Prototipo sul dispositivo | Avatar, dadi, PF e salvataggio | Sequenza completata sull'hardware, anche dopo riavvio |
| 3 — Scheda utilizzabile | Inventario, risorse, capacità e backup | Una sessione di prova senza perdere aggiornamenti |
| 4 — Personalizzazione | Editor avatar e trasferimento dall'app di supporto | Creazione, trasferimento e ripristino di un personaggio |
| 5 — Party locale | Associazione dispositivi e iniziativa condivisa | Riconnessione senza duplicare eventi o perdere stato |
| 6 — Griglia | Movimento e posizioni condivise | Piccolo incontro con visibilità e controlli leggibili |

L'integrazione Beyond procede come verifica separata. Un suo eventuale blocco non impedisce le altre fasi. L'ordine delle fasi 0 e 1 può sovrapporsi se l'hardware non è immediatamente disponibile.

## D&D Beyond e contenuti

Non è ancora verificata la disponibilità di un'integrazione ufficialmente supportata per autenticazione, lettura o creazione dei personaggi. Non promettere queste funzioni prima della verifica.

L'acquisto di contenuti su Beyond non stabilisce da solo i diritti di utilizzo da parte di PocketD-D. Verificare termini, licenze e modalità di distribuzione. Non includere cataloghi proprietari, immagini, descrizioni o credenziali Beyond nel repository.

Per iniziare: personaggi e grafica originali, inserimento manuale e formato locale documentato. Un eventuale regolamento aperto deve indicare fonte, versione, licenza e attribuzioni richieste. Distinguere le regole 2014 da quelle 2024: la versione sarà un dato esplicito della scheda.

## Decisioni da discutere

- Confermare il tavolo con DM come esperienza iniziale.
- Scegliere direzione artistica: fantasy caldo da taverna, dungeon scuro o altra palette.
- Scegliere il primo regolamento e quali automatismi offrire.
- Definire alimentazione, autonomia desiderata e modalità d'uso durante la sessione.

La grafica iniziale deve restare originale e sostituibile: non serve acquistare o generare un intero catalogo di asset prima di provare il flusso.


## Stato della simulazione browser

Gli incrementi 0.1–0.6 hanno aggiunto scheda, avatar personalizzabile, inventario e risorse manuali, abilità e salvezze, magie/capacità personali e tracker di combattimento locale con bersagli. Il tracker anticipa una parte dell'esperienza di combattimento senza completare le fasi hardware, party o griglia. La verifica sul dispositivo fisico rimane necessaria prima di dichiarare praticabile l'interfaccia sul display da 1,75 pollici.


L'incremento 0.7 aggiunge una prima griglia locale 9 × 9 con muri, pedine e movimento per turno. Anticipa la prova dell'interfaccia di mappa; non completa la fase 6 della roadmap, che richiede ancora visibilità e controlli verificati sull'hardware e posizioni condivise tra dispositivi.

L'incremento 0.8 aggiunge zoom 5 × 5, panoramica e assistenza geometrica agli attacchi. L'incremento 0.9 aggiunge una raccolta locale di personaggi indipendenti, con migrazione del salvataggio, selezione, duplicazione e backup completo. Questa è una parte dell'app di supporto della fase 4; il trasferimento al dispositivo resta da sviluppare.

## Prosecuzione senza hardware

L'hardware non è ancora disponibile. Lo sviluppo procede sulla simulazione e sull'app di supporto, mantenendo aperta la verifica fisica delle fasi 0 e 2:

1. **Completato nella 0.9:** raccolta di più personaggi con progressi separati e backup.
2. **Completato nella 0.10:** inventario estensibile, armi e cure personalizzabili, mantenendo le regole dimostrative esplicite e la compatibilità dei backup.
3. **Completato nella 0.11:** prima revisione di grafica, avatar e navigazione del display circolare con prove browser desktop/mobile. La verifica fisica resta aperta.
4. **Completato nella 0.12:** primo protocollo di party tra finestre dello stesso browser, con iniziativa, turni, PF e mappa condivisi, riconnessione e deduplicazione. Restano collegamento tra dispositivi, associazione, controlli del party nel cerchio e gestione completa della sessione.
5. Preparare la base firmware dagli esempi ufficiali e verificarne la compilazione; display, touch, SD e autonomia saranno provati quando arriverà la scheda.

Codice e stato della roadmap vengono salvati su GitHub a ogni incremento verificato. Il recupero della v0.8 è documentato separatamente in [RECUPERO-v0.8.md](RECUPERO-v0.8.md).


L'incremento 0.10 porta inventario e armi oltre le tre voci dimostrative: ogni personaggio possiede una lista manuale di oggetti, consumabili, cure e armi. Gli attacchi e la portata seguono l'arma equipaggiata. La CA resta manuale; non sono ancora automatizzati peso, monete, munizioni, proprietà speciali o un catalogo di regole.

L'incremento 0.11 rivede la presentazione fantasy, le icone e i comandi del dispositivo, mette la simulazione in primo piano su mobile e aggiunge navigazione rapida, tastiera e gesti. La personalizzazione dell'avatar usa anteprima, scelte visive e preset compatibili con le schede esistenti. Questa è una revisione verificata nel browser; non completa la verifica hardware né l'editor per disegnare asset pixel per pixel.


L'incremento 0.12 anticipa le fasi 5 e 6 con un trasporto browser sostituibile. Il coordinatore conserva lo stato autorevole del party; ogni finestra mantiene la propria identità e riprende lo snapshot corrente dopo un'interruzione. I dati del party sono separati dalla raccolta personale. La prova con più finestre non completa le fasi hardware o la comunicazione tra dispositivi. Uso e limiti sono descritti in [PARTY.md](PARTY.md).
