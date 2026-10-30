# Equipaggiamento, magie e attacchi sul Pocket — v0.85

> Aggiornamento v0.86: [bonus permanenti e sintonia per tutti i PG](OGGETTI-SINTONIA.md).

30 ottobre 2026. Il dispositivo resta il prodotto principale. Questo aggiornamento
aggiunge comandi utilizzabili anche sulle schede già presenti: non serve cancellare
il PG o la SD. I testi estesi restano negli estratti concordati.

## Preparare una magia

Apri **Magie → magia → Prepara → Conferma**. Se premi l'azione principale su una
magia non preparata, si apre direttamente la stessa anteprima. Per toglierla dalla
lista delle pronte usa **Togli prep.**. Il filtro Pronte non cancella le altre magie.
La preparazione non spende slot, non ripristina utilizzi e non termina una
concentrazione già in corso. Limite delle magie preparabili e requisiti di classe
restano da verificare secondo le regole del personaggio.

**Gestisci** conserva accesso a utilizzi innati e profilo delle regole.
Le liste mostrano nome, icona e disponibilità; capacità passive e utilizzi sono
riconoscibili. Le magie non si lanciano a zero PF o durante Ira.

## Equipaggiare e vedere le conseguenze

Apri **Zaino → oggetto → Equipaggia/Riponi → Conferma**. L'anteprima mostra CA,
caratteristiche, PF massimi e bonus ai tiri che cambiano. Si può annullare;
un salvataggio fallito lascia l'anteprima aperta senza cambiare la scheda.

Le nuove importazioni riconoscono le armature/scudi standard con dati espliciti,
senza proprietà speciali non riconciliate. La CA sorgente include già l'armatura:
il profilo registra questo fatto, senza sommarla una seconda volta. Togliere
una Chain Mail di base 16 con DES 10 e formula senza armatura 10+DES porta a CA 10.
Si gestiscono una sola armatura e un solo scudo; sceglierne un altro ripone il
precedente. L'arma attiva è una scelta distinta.

Per gli oggetti importati senza profilo, apri **Configura → Armatura e bonus**.
Scegli tipo, CA base, formula senza armatura e proprietà numeriche. **Base/Bonus**
salta tra i due gruppi di campi. I bonus di caratteristica sono aumenti espliciti,
non punteggi fissi o nuovi massimi. I bonus CA/arma/magie/prove/TS sono additivi.
Il bonus PF indicato è solo la quota fissa oltre all'effetto di COS.

Alla prima configurazione rispondi esplicitamente a **Bonus già nei valori
importati?**:

- **Sì**: registra il profilo senza cambiare i numeri già letti da Beyond.
- **No**: applica le proprietà al momento della conferma.

Mantieni lo stato equipaggiato coerente con il momento della lettura quando
registri proprietà già incluse. L'anteprima distingue questa registrazione da
un'applicazione nuova. Una formula CA sconosciuta richiede una scelta esplicita;
registrarla come riferimento non corregge da sola una CA sorgente errata.

Cambiare DES aggiorna le formule CA conosciute; cambiare una caratteristica
aggiorna i tiri che la usano. COS aggiorna i PF massimi di **livello × variazione
del modificatore**. Aumentare i PF massimi non cura; ridurli limita i PF attuali
al nuovo massimo. I bonus dell'equipaggiamento si propagano ad armi, magie,
prove e salvezze. Portare a zero la quantità ripone prima l'oggetto.

## Configurare un'arma importata come oggetto

**Attacchi** apre la scelta dell'arma se non ce n'è una utilizzabile. Scegli
l'oggetto, un preset, caratteristica, competenza e bonus attacco/danni, poi
anteprima e conferma. Lo stesso editor si trova in **Zaino → oggetto → Configura**.
I preset propongono dadi/proprietà base, non concedono competenza di classe.
Per un oggetto speciale confronta i valori con la scheda Beyond.

Gli override personalizzati di Beyond e la capacità di usare CAR con un'arma
non vengono attribuiti a tutte le armi. Proprietà come Pact of the Blade,
invocazioni, maestrie, sintonia e bonus condizionali richiedono verifica/configurazione.
Le icone e il nome di un oggetto non gli assegnano regole o statistiche.

## True Strike 2024 / Colpo Accurato

Le nuove importazioni riconoscono la definizione Beyond 2619204 e il manuale
2024, identificato dalla sorgente 145. Il trucco diventa disponibile e mantiene
la caratteristica della classe che lo concede. Una semplice omonimia o la
versione 2014 non abilita questo comportamento.

**Attacchi → Magie → True Strike** oppure **Magie → True Strike → Attacca**.
Se manca l'arma, si apre la scelta/configurazione. Puoi scegliere il tipo normale
dell'arma o radiante. Il tiro usa la caratteristica da incantatore, mantiene
competenza e bonus dell'arma e aggiunge ai danni 1d6 radiante dal livello totale 5,
2d6 dall'11 e 3d6 dal 17. I bonus agli attacchi magici non diventano bonus all'arma.
Il critico raddoppia i dadi dell'arma e quelli radianti; il bonus fisso resta singolo.
Nessuno slot viene speso. Ira, zero PF e mancata preparazione impediscono l'uso.

Per una vecchia importazione conservata come nota: **Regole → True Strike 2024 →
caratteristica → Anteprima → Conferma**. È una scelta esplicita: controlla prima
la versione della magia. Le correzioni attacco/danni del profilo sono separate
dai bonus generici alle magie. I profili manuali non vengono interpretati dal testo.

Il risultato mostra **arma + extra radiante = totale**. **Radianti / Dadi arma**
mostra gli stessi risultati già estratti, senza un nuovo lancio. Lo storico
conserva le due parti con totale comune in un'unica operazione di salvataggio;
Riprova conserva entrambi i gruppi anche dopo un errore. I PF del bersaglio
non vengono modificati dal tiro.

Esempio di confronto: livello 6, CAR 17, competenza +3, spadone 2d6 con bonus +1
sia all'attacco sia ai danni: **+7 per colpire, 2d6+4 più 1d6 radiante**.
Altri effetti del personaggio non sono impliciti in questo esempio.

## Icone, tasti e limiti

32 simboli pixel art originali con catalogo condiviso PC/firmware: armi, armature,
scudi, cure, scorte e famiglie di magie, con eccezioni dedicate per True Strike,
Eldritch Blast, Mage Hand, Darkness, Sacred Flame, Chill Touch e Faerie Fire.
Nessun simbolo è scelto casualmente. Per voci sconosciute resta un simbolo di
categoria esplicito; non è ancora un'illustrazione esclusiva per ogni magia D&D.

Nei nuovi editor **PWR breve** sposta il focus, **BOOT breve** attiva il controllo
e **BOOT lungo** torna indietro. Le assegnazioni personali restano salvate;
PWR lungo conserva il comportamento riservato del dispositivo. Le conferme
proteggono la bozza da scorciatoie e importazioni concorrenti.

Non è un motore completo D&D: restano manuali effetti condizionali, resistenze
concesse/tolte dagli oggetti, punteggi fissi, cap delle caratteristiche, requisiti
di sintonia, compatibilità scudo/due mani, munizioni, bonus specifici di bersaglio
e altre magie con effetti sull'arma. Non si applicano regole dedotte dalle note.
Modificare un campo numerico base dal PC richiede ricontrollare i profili che
lo fotografano. La sincronizzazione con Beyond resta separata.

## Compatibilità ed evidenza

Nuove proprietà: **PC v21, USB18, firmware 0.85**, stato nativo **PDDb/37**.
I backup precedenti restano leggibili; l'aggiornamento conserva NVS e SD. Il nuovo
backup nativo richiede app e firmware aggiornati per essere ripristinato.
PC/USB precedenti restano usati quando la scheda non contiene le nuove proprietà.
Le partizioni e l'involucro backup nativo v1 non cambiano.

Lettura reale del PG 167361077: HTTP200, JSON valido, PF35/53, CA16,
FOR/DES/COS/INT/SAG/CAR15/10/16/8/8/17. True Strike 2024 con CAR e Chain Mail16
riconosciuti. Claymore personalizzata conservata come oggetto da configurare.
Nessuna raccolta modificata e nessun JSON completo salvato/pubblicato.

I calcoli, i percorsi UI, il salvataggio con errori simulati e le migrazioni sono
verificati con dati sintetici sul codice C reale e sul renderer LVGL, distinti
dalla lettura Beyond. Le catture sono render del firmware su PC. L'utente ha
riferito risolti i precedenti problemi USB/installer/SD; questa nuova interazione
di equipaggiamento/True Strike richiede ancora riscontro sul dispositivo fisico.

Verifica software della release: 309 test JavaScript; confronto di 320 stati
JS/USB18/C con codec nativo e rifiuto di 11 pacchetti malformati; 138/138 viste
LVGL esercitate, comprese conferme, quantità, errori/riprova e tasti locali.
Loadout, True Strike e UI passano anche con AddressSanitizer/UndefinedBehaviorSanitizer
(controllo leak disabilitato nell'ambiente sandbox). Build ESP-IDF 5.5.5 per il
Waveshare standard riuscita, circa 20% della partizione app disponibile.
La prova browser usa una raccolta sintetica separata e verifica icone, CA,
ricaricamento e conservazione di una seconda scheda.

Riproducibili: `npm test`, `npm run firmware:test`, `npm run build`,
`python3 scripts/check-session-ui.py`. Con IDF attivato: `idf.py -C firmware build`.
Le nuove catture si rigenerano con `python3 scripts/preview-session.py` dopo
l'esecuzione delle prove UI; [anteprima](previews/v085-device-session.png).
