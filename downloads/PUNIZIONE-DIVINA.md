# Punizione Divina 2024 — v0.89

30 ottobre 2026. Nuovo effetto dopo il colpo, comune a tutti i PG che possiedono
l'incantesimo. La capacità Divine Smite 2014 rimane distinta e manuale: non si
trasforma automaticamente nella regola 2024.

## Flusso sul Pocket

1. Prepara la magia, se necessario. **Magie → Divine Smite → Attacca** oppure la
   scelta da **Attacca** portano all'arma; non consumano ancora slot o utilizzi.
2. Effettua un attacco normale o True Strike. Dopo un colpo riuscito con un'arma
   di categoria da mischia e almeno un costo disponibile, **Danni** apre Punizione.
3. Scegli l'origine se ne possiedi più di una e lo **slot ordinario o del patto**.
   Un uso innato finito compare solo se esiste un saldo esplicito nella scheda.
   Non si concede un utilizzo gratuito a tutti i Paladini.
4. Indica se il bersaglio è Immondo/Non morto. Se Hex o Marchio sono concentrati,
   indica anche se questo colpo riguarda il loro bersaglio. Il titolo mostra i d8
   risultanti e ricorda l'azione bonus.
5. **Usa e tira** conferma il costo e il tiro. **Senza** tira senza Punizione
   (con l'eventuale altro extra selezionato); **Indietro** non tira e non spende.

Un mancato, un'arma a distanza, la magia non preparata o un costo esaurito non
producono Punizione. Il modello distingue categoria dell'arma e attacco: un'arma
da mischia lanciata mantiene la propria categoria. Colpi senz'armi non sono
rappresentati automaticamente dal profilo arma attuale.

Si applica il modello 2024: d8 radianti che dipendono dal costo scelto, bonus per
Immondo/Non morto e critico, senza il vecchio limite della capacità 2014. Per i
critici con più di 18 dadi, i d8 sono divisi in due gruppi congelati e sommati.
Il tiro non termina la concentrazione già attiva. I bonus agli attacchi magici
e alla CD non diventano danni della Punizione.

Fonte ufficiale: [Divine Smite 2024](https://www.dndbeyond.com/spells/2618833-divine-smite).
Le regole sono implementate indipendentemente; non viene incluso un catalogo di
testi estesi.

## Costi e riprove

Prima della conferma sono verificati colpo, categoria dell'arma, preparazione,
PF, Ira e costo scelto. Non si passa automaticamente da patto a ordinario quando
un saldo è vuoto e non si lancia la magia separatamente dal colpo.

Se il salvataggio del costo fallisce, risultato e scelta rimangono congelati.
La schermata mostra **Riprova** e blocca modifiche, uscita, import e spegnimento
software finché la conferma non riesce. La riprova non rilancia dadi; il costo
rimane quello originale e viene applicato una volta sola.

Dopo il salvataggio, arma, eventuali radianti di True Strike, Hex/Marchio e
Punizione sono registrati insieme nello storico. Si possono avere fino a cinque
parti quando i d8 richiedono due gruppi. Una riprova dello storico conserva le
facce e non spende altri slot. Il registro conserva 20 voci/parti, non 20 attacchi
completi con un numero illimitato di componenti. I PF del bersaglio restano al DM.

## Import e schede già presenti

Il riconoscimento automatico richiede definizione **2618833**, fonte **145/tipo 1**,
livello 1, nessuna concentrazione e un costo rappresentabile. Nome, classe o icona
non concedono l'incantesimo né un uso gratuito. Homebrew, Legacy e identità
sconosciute restano manuali.

Per una magia già presente come incantesimo di livello 1 senza concentrazione:
**Gestisci → Regole della magia → Punizione 2024 → Anteprima → Conferma**.
Questo configura il profilo, non concede una nuova magia e non la lancia.
Una vecchia nota/capacità non viene trasformata automaticamente in incantesimo.

L'import riconcilia ora anche i massimi degli slot lasciati a zero dal servizio
quando esistono tabelle non ambigue: un solo mezzo-incantatore ordinario e la
classe Warlock 2024 per il patto. I consumi restano quelli ricevuti, separati per
pool. Tabelle invalide, consumi mancanti, massimi discordanti e combinazioni non
supportate restano segnalati; non si creano slot freschi per riempire un vuoto.
Altri multiclassi e potenziamenti dei massimi richiedono ancora verifica.

Per correggere gli slot irrisolti di una vecchia importazione, crea una nuova
importazione con l'app PC aggiornata. Il firmware non possiede il JSON sorgente
per ricostruire da solo i vecchi massimi. La raccolta PC conserva le schede
esistenti; acquisisci un backup nativo prima di sostituire sul Pocket una copia
con progressi da conservare. Non è un aggiornamento automatico della vecchia scheda.

App PC e firmware **0.89** richiesti per import/backup USB. **PC24 / USB21** solo
per i nuovi profili; stato nativo **PDDe/40**, dimensioni e partizioni invariate,
lettura dei salvataggi precedenti conservata. La SD non richiede modifiche.

## Cosa resta al tavolo

**Usa e tira** presuppone che siano disponibili l'azione bonus e il lancio secondo
le regole del turno. L'app non tiene ancora un bilancio generale delle azioni
bonus o degli slot spesi nel turno: questi requisiti, componenti verbali,
Counterspell, capacità che cambiano costi/danni ed eccezioni rimangono manuali.
Non applicare la regola 2024 alla capacità 2014. Altre Punizioni, Eldritch Smite,
attacchi senz'armi e altri effetti non ottengono automaticamente questo profilo.

## Lettura reale e prove

Nuova lettura del PG **167361077**, tramite connettore esistente: HTTP 200 e JSON
valido. La prima conversione conservava Punizione come nota per i massimi slot
irrisolti. Dopo la correzione delle tabelle: **ordinari di 1° 1/2**, **patto di 3°
1/2**; Punizione 2024 mantiene **CAR**, costo slot e stato **non preparata** della
sorgente. PF **35/53** e CA **16** rimangono coerenti con i dati già confermati.

Il pacchetto USB21 è passato in memoria al vero parser C e al codec nativo:
valori principali, slot e profilo coincidono con la conversione PC. Nessun file
con il JSON completo, nessuna raccolta personale modificata, nessun invio USB
fisico e nessuna scrittura verso Beyond. Il tiro di Punizione sul PG reale non è
stato eseguito: preparazione e arma devono essere configurate/verificate dall'utente.

327 test JavaScript, 140 viste native, ASAN/UBSAN e compilazione ESP32.
Prove sintetiche: **2.592 combinazioni** di slot, pool, bersaglio, critico,
concentrazione, True Strike e caratteristica; rifiuti prima di RNG/mutazioni,
usi finiti, parser/codec e rifiuto del downgrade. Percorsi LVGL con annullamento,
errore NVS, riprova congelata, storico di quattro/cinque parti, critico da 22d8,
arma a distanza e uso gratuito esplicito. Queste prove software non sostituiscono
il collaudo delle nuove interazioni sul dispositivo fisico.

[Schermate del renderer firmware](previews/v089-divine-smite.png), dati sintetici.
