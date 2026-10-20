# Tre extra difensivi indipendenti — v0.34

Sul dispositivo ogni tipo di danno conserva una **base e tre extra**. Ogni
extra ha proprietà, condizioni, attivazione, durata e vincolo alla concentrazione
propri. La resistenza concessa da due extra non dimezza due volte il danno;
quando uno termina, l'altro può continuare a proteggerlo.

## Uso sul dispositivo

Da **Danni → Difese per tipo → tipo → A tempo** si apre il primo extra.
Il pulsante **Extra 1 di 3 >** apre la scelta dei tre extra. Scegline uno per
modificarlo; passa agli altri senza perdere le bozze. I numeri sono stabili:
scadenza, pausa e rimozione delle proprietà non spostano gli altri extra.

Ogni extra ha gli stessi controlli già disponibili: resistenza, vulnerabilità,
immunità, attivazione, **Regole**, **Durata** e **Vincolo**. Le
[sette condizioni](DIFESE-CONDIZIONALI.md) sono separate per proprietà e per
extra. Per svuotarne uno, disabilita attivazione, proprietà, timer e vincolo;
non viene cancellata la configurazione degli altri due.

Nella scelta degli extra, **Confronta** mostra proprietà, attivazione, durata e
concentrazione prima/dopo. Il pulsante con tipo e numero scorre i tre extra;
**Regole** mostra i requisiti completi prima/dopo per ciascuna proprietà.
In quel riepilogo, premere **Extra** quando l'extra è già selezionato passa al
successivo; il titolo mostra sempre il numero. **Base** consulta il profilo base.

L'anteprima generale mostra le proprietà potenzialmente disponibili nell'insieme
di base e dei tre extra attivi, insieme al dettaglio dell'extra selezionato.
L'applicabilità effettiva si risolve soltanto con la fonte del colpo. **Conferma**
salva insieme tutte le bozze del tipo e la base, in un solo commit. Da una
qualsiasi anteprima, Indietro permette di rivederle. Tornare all'elenco delle
difese senza confermare le scarta. Un extra non valido blocca la conferma
comune: controlla attivazione, proprietà, round e vincolo di tutti e tre.

L'elenco segnala `+T` per un extra attivo oppure `+2T`/`+3T`; non inventa un
profilo normale per una base ancora da configurare. Una condizione difensiva
non diventa certa perché esiste un altro extra.

## Round, concentrazione e colpi

**1 round**, anche dal tasto configurato, avanza insieme tutti gli effetti
numerici e gli extra attivi a round. **Difese** mostra ogni extra separatamente,
con tipo, numero e round prima/dopo, fino a 39 voci su 13 pagine. Il profilo
riportato sotto è quello complessivo del tipo: può restare resistente dopo
la scadenza di uno dei due extra che concedevano resistenza.

- Timer differenti scadono indipendentemente; un extra disattivato è in pausa.
- La fine della concentrazione disattiva e scollega tutti gli extra vincolati,
  anche se erano già inattivi, senza terminare quelli non vincolati. Questo vale
  anche per rilancio, Ira, zero PF, riposo lungo e fallimento della salvezza.
- Proprietà, condizioni e timer dei profili terminati restano disponibili per
  un eventuale riuso esplicito. Non si riattivano da soli.
- Se le scadenze numeriche porterebbero CA/PF massimi fuori limite, tutto il
  round viene respinto: nessun timer difensivo avanza e nessun PF viene perso.

Per ogni proprietà il calcolo usa l'unione della base e dei tre extra attivi.
Condizioni complementari possono rendere certa una proprietà senza conoscere
un fatto della fonte; i fatti necessari ancora sconosciuti bloccano l'intero
colpo. Nessuna resistenza o vulnerabilità identica si accumula. Immunità certa
rende irrilevanti resistenza/vulnerabilità incerte.

Restano il [colpo misto](DANNI-MISTI.md) con massimo sei componenti, raggruppate
per tipo e proprietà applicate, un assorbimento dei temporanei, un controllo
PF e una salvezza COS. Il risultato conserva le difese all'impatto anche dopo
la fine degli extra. Errore di salvataggio e retry conservano candidato, gruppi
e dado. **Eroe → Annulla** ripristina l'intera ultima modifica della sessione.

PWR/BOOT conservano le assegnazioni. I confronti e le conferme proteggono la
bozza da scorciatoie e importazioni USB; PWR lungo resta riservato all'hardware.

## Salvataggi e compatibilità

Le scritture sono **PDD15**, tag **PDDF**, da **8333 + 552 × numero voci** byte,
massimo **74573**. Si aggiungono **208 byte** prima del CRC al payload PDD14:
extra 2, poi extra 3, ciascuno nei 13 tipi; ogni voce contiene proprietà,
attivazione, timer, round, concentrazione e tre selettori di condizione (8 byte).
La serializzazione è esplicita, senza dipendere dal layout delle struct.

Lettura **PDD1–PDD14** conservata senza scritture al boot. Il vecchio extra di
ogni tipo diventa **Extra 1**, senza perdere timer, regole o vincoli; Extra 2 e
3 iniziano vuoti. Fixture sintetica PDD14 prodotta con l'encoder v0.33 originale
`7cc17f8`, prima delle modifiche, verificata anche attraverso il controller NVS.
La prima modifica scrive nella banca alternata e conserva il record originale.

Firmware precedenti alla v0.34 non leggono PDD15 e mantengono il blocco delle
versioni future: non cancellare NVS per aggirarlo. **Layout flash, USB v7 e
backup PC v10 invariati.** Gli extra sono configurazioni locali persistenti;
la raccolta PC non viene modificata. Importare una nuova proiezione USB con
conferma sostituisce lo stato di gioco e tutti e tre gli extra; annullare conserva
tutto. Il backup dello stato dispositivo verso PC resta da implementare.

Per il core, `pd_defence_grant`/`pd_defence_grant_set` accedono agli slot 0–2;
le vecchie API temporanee restano alias del primo. I bit del riepilogo round sono
ora a 64 bit: `slot * 13 + tipo`. I primi 13 bit mantengono il significato
precedente; i successivi individuano ogni extra senza confonderlo con il tipo.

## Verifiche e limiti

- **212992 casi** di tipo/proprietà di base e tre extra, con scadenze progressive;
  confronto indipendente dei flag, del danno e dei timer.
- **83349 fonti parziali** per tre regole e ciascuna proprietà, valutate rispetto
  alle descrizioni complete; anche fonti complementari e immunità.
- Tutti i 39 timer, bit oltre 31, pausa e scadenze numeriche respinte senza
  modifiche; fine/rilancio/riposo/zero PF/Ira/salvezza, vincoli inattivi e
  conteggio massimo di 47 effetti/difese collegati.
- Record massimo e tutti i 208 nuovi campi alterati con CRC valido; booleani,
  selettori, proprietà assenti e riferimenti respinti senza sostituire gli output.
- Migrazione delle fixture PDD1–PDD14, controller NVS con banca originale
  conservata, errore/retry e rilettura dopo riavvio.
- UI LVGL su host: tre bozze, confronto e regole, un solo salvataggio, fallimento,
  annullamento, concentrazione e snapshot, scadenze distinte, importazione,
  scorciatoie protette e ultima delle 13 pagine. Controlli nel display circolare
  con area di tocco almeno 44 px e testi senza sovrapposizioni.
- Regressioni core/UI, 138 test JS e build PC passati; ASan/UBSan su difese,
  danni, effetti, magie/riposi/stato, allocazioni, core/parser, NVS e UI, con leak
  detector disattivato. Build ESP32-S3 riuscita (vedi [dettagli](FIRMWARE.md)).

Il limite è **tre extra per tipo**, senza nomi personalizzati. Restano riduzioni
fisse, condizioni arbitrarie, durate di turno/minuti e ricalcolo completo da
oggetti. Nessun riconoscimento automatico di difese da Beyond o descrizioni.
Questo incremento funziona offline senza Foundry. Nessuna nuova lettura Beyond,
credenziale o JSON personale pubblicato. Compilazione e prove su host non sono
collaudo fisico: alimentazione, sospensione, batteria, touch, USB e recupero dopo
interruzione richiedono ancora il dispositivo reale.
