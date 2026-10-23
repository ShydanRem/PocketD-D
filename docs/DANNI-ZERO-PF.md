# Danni a zero PF e morte istantanea — v0.56

> Aggiornamento v0.57: [circostanze e bonus nei tiri contro morte](TIRI-MORTE.md).
> La risoluzione dei danni descritta qui conserva le stesse regole e conferme.

Il firmware risolve ora esplicitamente questi due casi, offline e senza
Foundry. La grafica del compagno e le assegnazioni PWR/BOOT sono conservate.
La raccolta PC non viene aperta o modificata e non è stata letta una scheda Beyond.

## Percorso e conferma

Da **Usi → Stato → Danni**, preparare il colpo, verificare difese, fonte,
riduzioni e costi, poi aprire l'anteprima. Quando il danno effettivo è positivo
e il personaggio è già a zero PF, oppure l'eccedenza può causare morte
istantanea, compare **Risolvi registro morte**. La normale conferma del danno
resta bloccata per questi casi.

La nuova pagina mostra danno effettivo, temporanei, PF, massimo e registro
prima/dopo. **Tipi** apre il conto del colpo senza modificarlo. Occorre
verificare **Base 2014** e, quando necessario, dichiarare esplicitamente se
il colpo è critico. Cambiare la dichiarazione del critico annulla la verifica.
Solo il touch su **Conferma** salva. Indietro abbandona la risoluzione;
scorciatoie e importazioni non possono confermarla.

La verifica riguarda l'applicabilità di questa regola al singolo evento,
non seleziona il regolamento dell'intera scheda. Se esiste un'eccezione,
annullare e usare le correzioni manuali concordate con il DM.

## Valori e regola rappresentata

- Già a zero PF: un danno effettivo positivo aggiunge un fallimento, oppure
  due per un critico dichiarato, fino a tre. Conserva i successi e rimuove
  la stabilizzazione. I temporanei non evitano questi fallimenti.
- Già a zero PF: il confronto per la morte istantanea usa l'intero danno
  effettivo, prima dei temporanei, rispetto ai PF massimi.
- Partendo sopra zero: il confronto usa l'eccedenza dopo temporanei e PF
  attuali. Arrivare semplicemente a zero non aggiunge fallimenti.
- La soglia è inclusiva e usa i PF massimi al momento dell'impatto,
  prima della fine degli effetti collegati alla concentrazione.
- Sei componenti dello stesso colpo producono un solo evento per il registro.
  Il critico non raddoppia l'importo inserito: i dadi del danno devono già
  essere inclusi nella preparazione del colpo.
- Un danno effettivo pari a zero non aggiunge fallimenti. Le difese non
  configurate e i fatti della fonte mancanti continuano a bloccare il flusso.

Esempi sintetici con massimo 32 e nessun'altra riduzione:

| Prima | Danno effettivo | Dopo |
| --- | --- | --- |
| PF 0, temporanei 3 | 1 normale | Temporanei 2, un fallimento aggiunto |
| PF 0, temporanei 3 | 1 critico | Temporanei 2, due fallimenti aggiunti |
| PF 0, temporanei 99 | 32 | Temporanei 67, morte istantanea, tre fallimenti |
| PF 24, temporanei 3 | 59 | Eccedenza 32, morte istantanea |
| PF 24, temporanei 3 | 58 | Eccedenza 31, PF 0 senza fallimenti aggiunti |

Riferimento verificato: sezione `damage-and-healing` del
[SRD 2014 in 5e-database](https://github.com/5e-bits/5e-database/blob/main/src/2014/en/5e-SRD-Rule-Sections.json).
Il repository contiene l'implementazione e questi esempi originali, non
una copia del catalogo o dei testi delle regole.

## Blocchi, salvataggio e annullamento

Serve un registro morte disponibile. Le schede precedenti senza registro
non ne ricevono uno automaticamente. Sono bloccati un personaggio già a tre
fallimenti, un registro incoerente a zero PF (tre successi, oppure stabilità
con contatori non azzerati) e un profilo di riduzione che spende una reazione
quando il personaggio è già a zero. Tutti i requisiti delle spese restano validi.

PF, temporanei, fino a tre spese, eventuale reazione, concentrazione/effetti
collegati e registro morte sono preparati nella stessa copia candidata e
salvati insieme. La RAM viene aggiornata solo dopo il salvataggio riuscito.
Il test host di scrittura fallita e nuovo tentativo verifica che non siano
addebitati due volte i costi; non certifica un'interruzione reale di NVS.
Un errore fisico ambiguo richiede diagnosi e recupero, senza forzare scritture
o cancellare NVS. **Annulla** ripristina lo stato precedente completo.

Non vengono estratti dadi né scritti elementi nello storico dei tiri.
La ricevuta mostra l'esito e conserva il conto dell'impatto, ma non costituisce
uno storico completo delle modifiche. La causa istantanea o il critico sono
informazioni della ricevuta in RAM: dopo riavvio/backup restano i contatori,
non una causa di morte persistente separata.

## Compatibilità e limiti

PDD25, PDH2 con lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e tabella partizioni invariati. Il vecchio percorso core dei
danni mantiene il rifiuto dei casi speciali; la nuova API richiede la scelta
esplicita e verificata. Lo stato risultante passa il normale codec PDD25.

Non sono automatizzati eccezioni di classe, capacità che evitano lo zero,
colpi non letali, altre decisioni del DM o tutti gli effetti dell'incoscienza.
I blocchi preesistenti su cure, tiro morte e riposo lungo di un personaggio
a tre fallimenti restano attivi; questo non è un controllo completo di tutte
le azioni disponibili a un personaggio incapacitato o morto.

## Verifiche e dispositivo

Le verifiche sintetiche comprendono 67.584 combinazioni indipendenti del core,
sei componenti, soglie esatte, temporanei, stabilizzazione, costi e salvataggi,
codec e guardie sui recuperi. La UI esercita anche dichiarazioni incomplete,
annullamento, errore di scrittura e nuovo tentativo, limiti numerici e assenza
di chiamate RNG/storico. Le catture `death-damage-*.png` sono prodotte dal
firmware LVGL su host, con dati sintetici; non sono fotografie del dispositivo.

Gli esiti di regressione e compilazione sono registrati in
[FIRMWARE.md](FIRMWARE.md). Per il futuro collaudo fisico usare una scheda
sintetica dopo averne conservato il backup, verificare anche riavvio e
ripristino, quindi compilare H23 in [COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).
Nessun flash o collaudo hardware è stato eseguito in questo incremento.
