# Difese ai danni: verifica rapida — v0.75

Quando un tipo di danno non è riconciliato nell’importazione Beyond, rimane
**Da verificare**. Non viene dedotto Normale dall’assenza di un modificatore.
La v0.75 permette di completare insieme i soli tipi mancanti dopo una scelta
esplicita del giocatore, sia sul dispositivo sia sul PC.

## Sul dispositivo

**Danni → Difese per tipo → Completa**. L’elenco mostra quanti tipi base sono
verificati. La nuova anteprima elenca tutti i tipi da dichiarare Normali, tre
per pagina con le frecce. Controlla la scheda, l’equipaggiamento e le regole del
PG prima di premere **Conferma**. **Indietro** non modifica nulla.

La conferma completa soltanto il profilo base: resistenze, vulnerabilità,
immunità e relative regole già impostate restano identiche. Restano conservate
anche le tre concessioni temporanee per tipo, durate, vincoli alla concentrazione,
risorse, PF e tasti. Un tipo Normale di base può quindi avere una resistenza
aggiuntiva attiva o condizionale: non significa assenza di effetti in ogni contesto.

La modifica viene applicata solo dopo il salvataggio riuscito. Se la memoria
segnala errore, resta l’anteprima e si può riprovare. È disponibile il consueto
annullamento dell’ultima modifica. Quando tutti i 13 tipi sono configurati,
Completa è disabilitato; si possono ancora correggere i singoli tipi.
I tasti configurabili e nuove richieste USB non saltano questa conferma.

## Sul PC

**Stato, risorse e note → Difese ai danni · dispositivo**. In alternativa,
**Prepara per dispositivo → Difese ai danni → Configura difese sul PC** apre
direttamente la sezione. Il profilo va abilitato esplicitamente se assente.

1. Controlla le difese note e correggile nei singoli menu quando necessario.
2. Premi **Completa i tipi da verificare…** e leggi l’elenco dei tipi mancanti.
3. **Ho verificato: imposta Normale** aggiorna soltanto la bozza del modulo.
4. **Salva stato e risorse** salva la scheda. Chiudere il modulo scarta la bozza.

Cambiare un tipo o disabilitare il profilo chiude la conferma precedente:
la lista deve essere riletta. La configurazione dei singoli tipi rimane possibile,
compreso il ritorno a Da verificare. Errori di quota o conflitti interrompono il
salvataggio senza sostituire la scheda precedente. Le altre schede restano conservate.

Il profilo si trasferisce con USB14 o superiore e si conserva nei backup.
Il PC serve a prepararlo: i pulsanti di correzione diretta dei PF sul PC non
applicano le difese. Per il conteggio del colpo usare **Danni** sul dispositivo.

## Compatibilità e limiti

Nessun nuovo formato o partizione: USB15 / backup PC v17 rimangono i massimi,
PDD34/PDDY rimane lo stato nativo. Un semplice profilo senza associazioni
personalizzate delle abilità usa USB14 / backup PC v16. Le versioni precedenti
leggono i profili già completati nei formati che supportano; il nuovo comando
sul dispositivo richiede firmware 0.75.

Questa funzione è una dichiarazione manuale, non amplia la conversione Beyond.
L’importatore non chiama il completamento e non presume mai 13 tipi Normali.
Difese condizionali, immunità alle condizioni, cambi di equipaggiamento e altre
regole non rappresentate richiedono ancora verifica e configurazione separata.

## Prove

Prove **sintetiche**, separate dalla lettura reale v0.73 del 28 ottobre:

- Tutti gli **8.192 sottoinsiemi** dei tipi verificati in JS e C: completamento
  dei soli mancanti, conservazione delle difese note e idempotenza.
- C: regole base condizionali e tre concessioni per tipo conservate byte per
  byte, roundtrip nativo campionato, stato corrotto respinto senza modifiche.
- Browser con raccolta isolata: importazione lascia sconosciuti i tipi assenti,
  lista esatta, bozza/annullamento, cambi durante l’anteprima, profilo disabilitato,
  errore quota atomico, salvataggio/reload, vecchie schede e pacchetto USB.
- Renderer LVGL reale sul PC: tutte le pagine dell’elenco, limiti circolari,
  annullamento, errore/riprova, protezione tasti/nuovo import, nessun dado o
  spesa, conservazione delle regole e annullamento dopo il salvataggio.

Nessuna nuova lettura reale Beyond o raccolta personale modificata. La precedente
anomalia USB fisica resta da collaudare; compilazione e simulazione non ne
certificano la risoluzione. Suite JS: 249 test; 119 viste LVGL, nuovo percorso
anche con ASan/UBSan; firmware ESP32-S3 compilato.
