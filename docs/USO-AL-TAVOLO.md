# Uso pratico al tavolo — v0.61

> Aggiornamento v0.65: [saldo degli oggetti locale](QUANTITA-OGGETTI.md) con
> anteprima, conferma e gestione dell’arma a zero. Nessuna ricarica di cariche.

> Aggiornamento v0.63: [ricerca locale per nome](RICERCA-LOCALE.md) nello zaino
> e nelle magie/capacità. Filtri di sessione, nessuna scrittura o modifica
> degli indici originali. Le descrizioni successive sono storiche.

> Aggiornamento v0.62: [alimentazione e batteria](ALIMENTAZIONE.md).
> Telemetria, avvisi, pausa/risveglio e spegnimento con salvataggio sono ora
> implementati; le prove fisiche e le misure restano da eseguire. Descrizioni
> e limiti successivi sono quelli storici della versione del titolo.

Controllo del 23 ottobre 2026 sul firmware di `main`. L'obiettivo è una prima
sessione individuale offline: scheda, dadi, PF, risorse e progressi recuperabili.
Il PC prepara/importa la scheda e conserva i backup. Regole speciali, party,
griglia, importazione autonoma e automatismi avanzati restano evoluzioni.

## Esito del controllo

| Percorso | Stato software e intervento | Limite attuale |
| --- | --- | --- |
| Home e azioni comuni | PF toccabili, menu a icone, accessi a scheda/arma/dadi/zaino/magie/riposi | Resa e tocco fisici da provare |
| Cure di importo noto | **Aggiunto:** tastiera numerica, confronto PF/temporanei, conferma e annullamento | L'importo è dichiarato dal giocatore; nessuna deduzione delle regole |
| PF temporanei | **Aggiunto:** assegna conservando il maggiore; correzione esplicita anche a zero | Origine, durata e proprietà speciali restano manuali |
| Danni ricevuti | **Migliorato:** accesso diretto dai PF e numero inseribile per prima/altre componenti | Tipo, difese e fatti necessari devono essere dichiarati |
| Pozioni e dadi vita | **Corretto:** un errore di scrittura conserva candidato e tiro; uscita bloccata dopo il tiro | Recupero dopo un vero guasto I/O o perdita di alimentazione da collaudare |
| Prove, attacchi e dadi | Percorsi esistenti verificati dalla suite, animazioni e risultato separati, storico persistente | Eccezioni e bonus speciali possono richiedere correzioni |
| Magie, utilizzi e riposi | Slot ordinari/patto, capacità, concentrazione, recuperi e dadi vita disponibili | Effetti arbitrari delle descrizioni non automatizzati |
| Tasti e navigazione | PWR breve, BOOT breve/lungo configurabili, conferme protette | Debounce e precisione sulla scheda reale da misurare |
| Persistenza e backup | Salvataggio, annullamento della sessione, copia USB e backup nativo già disponibili | Trasferimento fisico, riavvio e recupero dopo guasti da verificare |

I problemi nuovi trovati erano concreti: solo correzioni PF da un punto nella
home, nessun comando locale per concedere/rimuovere temporanei, inserimento
dei danni a passi e una pozione ricalcolata dopo una scrittura fallita. Anche
l'uscita dal dado vita già tirato consentiva di abbandonare il candidato dopo
un errore: ora conserva lo stesso risultato per il nuovo tentativo.

## Pannello PF

Toccare il valore dei PF sulla home. Il pannello mostra otto azioni:

- **−1 PF / +1 PF:** correzione manuale immediata di un punto, come prima.
  La correzione negativa assorbe prima i temporanei. Per risolvere un colpo
  con difese, concentrazione e registro morte usare **Danni**.
- **Cura:** inserire l'importo positivo ricevuto, poi Anteprima e Conferma.
  La cura si ferma al massimo effettivo, inclusi i modificatori locali; lascia
  invariati i temporanei. Riprendere PF da zero azzera il registro morte
  disponibile; tre fallimenti bloccano la cura finché lo stato viene corretto
  esplicitamente con il DM. Nessuno slot, uso o oggetto viene consumato da
  questo comando: rappresenta una cura di importo già noto.
- **Temp.:** inserire nuovi temporanei in modalità **assegna**. Il saldo
  finale è il maggiore fra attuale e nuovo; non si sommano e non curano PF.
  Il pulsante in alto passa a **correggi**, che sostituisce esplicitamente
  il saldo. Zero in questa modalità rimuove i temporanei. È una correzione
  manuale, anche per una scadenza dichiarata dal DM.
- **Danni / Stato:** aprono direttamente i percorsi esistenti, con le loro
  verifiche e conferme.
- **Annulla / Eroe:** ripristino dell'intera ultima modifica o ritorno alla home.

Cura e temporanei mostrano PF e saldo temporaneo prima/dopo. Tornare indietro
conserva la bozza nell'editor senza applicarla. Una conferma senza variazioni
resta disabilitata e non scrive flash. Salvataggio riuscito: ritorno all'Eroe
con feedback effettivo. Salvataggio fallito: stessa scheda visibile e stessa
anteprima. La modifica salvata si annulla dalla home; l'annullamento e le bozze
non sopravvivono al riavvio. Le assegnazioni dei tasti restano invariate.

## Inserire gli importi

La tastiera ha cifre, **Azzera** e **Canc.** (rimuove l'ultima cifra). Cura e
temporanei accettano fino a **999**; i danni fino a **9999** per componente.
Le cifre che supererebbero il limite vengono ignorate. I danni richiedono un
importo positivo; zero è disponibile per la correzione del saldo temporaneo.

In **Danni**, toccare il numero con **>**. La prima cifra sostituisce l'importo
iniziale; **Canc.** permette di modificarlo e **Azzera** riparte da zero.
**Usa numero** torna al colpo senza applicare danni. Vale anche nell'editor
delle altre componenti; lì **Usa parte** adotta la bozza come prima.
Il colpo conserva tipo, contesto, difese e percorso di anteprima/concentrazione.

Editor numerico e conferma proteggono la bozza da tasti, import e ripristino.
Non estraggono dadi. Importi, modalità e pagina sono in RAM: nessun nuovo campo
persistente e nessuna conversione dei dati precedenti.

## Un solo tiro anche dopo un errore

Prima di confermare pozione o dado vita si può ancora annullare. Dopo il tiro,
se il salvataggio fallisce, compare **Tiro fissato** e il comando **Riprova**.
L'uscita è disabilitata e i comandi in coda vengono rifiutati. Il nuovo tentativo
usa lo stesso candidato: facce, cura, quantità/dado speso e temporanei restano
coerenti. Solo dopo il salvataggio compare l'animazione e si registra il tiro.
Un guasto all'animazione mostra comunque il risultato senza rilanciarlo.

Un consumabile senza tiro mantiene il normale annullamento dopo una scrittura
fallita. Un vero errore I/O può bloccare il controller fino al riavvio e avere
già scritto la candidata: il retry host non certifica il recupero fisico.
Una successiva mancata registrazione nello storico segue il percorso esistente
**Storico → Riprova**, senza ripetere la cura o il consumo.

## Che cosa serve ancora per la prima sessione

La base software copre i percorsi ordinari e le lacune pratiche sopra sono
risolte. Il traguardo successivo è eseguire [PRIMO-AVVIO.md](PRIMO-AVVIO.md)
sull'hardware, inizialmente alimentato via USB:

1. Identificare la revisione corretta, conservare il firmware originale,
   verificare avvio, display, touch e tasti.
2. Trasferire una scheda sintetica e confrontare caratteristiche, PF, CA,
   zaino, slot e risorse. Configurare le difese usate per i tipi di danno
   della prova; un dato sconosciuto continua a bloccare il colpo.
3. Eseguire una breve sessione offline, controllare riavvio e ripristino del
   backup, poi annotare precisione e numero di passaggi delle azioni frequenti.
4. Verificare memoria e comportamento dopo interruzioni con una procedura
   concordata. Batteria, consumi, sospensione e spegnimento richiedono lavoro
   e misure dedicati per l'uso autonomo.

L'hardware resta il requisito aperto per dichiarare il prodotto utilizzabile
al tavolo. Per una prima prova via USB non occorre completare gruppi di costi,
condizioni arbitrarie, ricariche automatiche o sincronizzazione multiplayer.
Preferiti/ricerca per raccolte molto grandi sono un possibile affinamento
successivo, da valutare sul personaggio e sul pannello reali.

## Verifiche

**6.720 casi core indipendenti**: PF correnti/massimi, bonus al massimo,
temporanei, importi, operazioni e registro morte, con confronto dell'intero
stato atteso; input malformati, nessuna somma e salvataggio PDD28 invariato.
UI host **108/108 viste**, percorsi touch di cura/temporanei/danni, limiti,
annullamento, blocchi, nessuna scrittura senza cambiamenti, errori e retry,
pozioni/dadi vita fissati e fallback senza animazione. Esiti della suite e
build in [FIRMWARE.md](FIRMWARE.md).

PDD28/PDDS, PDH2 con lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni invariati. Le catture `practical-*.png` e la
galleria `practical-table-gallery.png` sono render LVGL su host con dati
sintetici. Nessuna lettura Beyond, modifica di raccolte personali o prova
fisica è stata eseguita. Il futuro controllo H28 resta **Non eseguito**.
