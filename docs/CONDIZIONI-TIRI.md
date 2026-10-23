# Condizioni ed effetti sui tiri — v0.44

> Aggiornamento v0.57: [tiri contro morte con circostanze e bonus espliciti](TIRI-MORTE.md).
> Il lancio base è conservato; gli effetti delle condizioni non vengono dedotti
> automaticamente per questi tiri. I limiti sotto descrivono la versione del titolo.

Il dispositivo gestisce un sottoinsieme esplicito degli effetti di **Accecato,
Spaventato, Prono e Trattenuto**, oltre ad Avvelenato già presente. Si tratta di
effetti sui tiri del personaggio, non dell'intera definizione delle condizioni.
Le annotazioni importate restano testo: nessun effetto è dedotto da nome o livello.

## Attivazione e salvataggio

Apri **Usi → Stato → Condizioni → Tiri**. Attiva gli effetti necessari e scegli
**Anteprima** per confrontare prima/dopo. **Conferma** salva il gruppo insieme;
Indietro consente di scartare la bozza. Un errore di salvataggio conserva lo stato
mostrato; Annulla sull'Eroe ripristina l'intera modifica. Note, PF, risorse e
configurazioni numeriche non vengono cambiati.

Ogni effetto è un interruttore unico: non conta fonti multiple, non ha una durata
propria e non si disattiva con riposo, round, tempo o fine della concentrazione.
Il DM stabilisce quando abilitarlo e quando tutte le sue fonti sono terminate.
Rimuovere una nota non disattiva un effetto; disattivarlo non rimuove la nota.
**Avvelenato** continua a regolarsi separatamente da Stato.

| Effetto | Parte applicata sul dispositivo | Parti ancora manuali |
| --- | --- | --- |
| Accecato | Svantaggio agli attacchi; fallimento automatico delle prove che richiedono vista | Visione, bersagli, vantaggio degli avversari, eccezioni |
| Spaventato | Svantaggio a prove/abilità e attacchi se la fonte è in linea di vista | Fonte, posizioni, movimento e altre restrizioni |
| Prono | Svantaggio agli attacchi del personaggio | Movimento, rialzarsi, distanza e attacchi degli avversari |
| Trattenuto | Svantaggio agli attacchi e alle salvezze DES | Movimento, uscita dalla condizione, attacchi degli avversari |
| Avvelenato | Svantaggio a prove/abilità e attacchi, come prima | Altri effetti specifici della fonte |

Sono effetti di base comuni; non selezionano un intero regolamento 2014/2024 né
implementano talenti o eccezioni. Bonus numerici e danni restano quelli già
rappresentati. Salvezze COS di concentrazione, dado libero e tiro morte base non
acquisiscono svantaggio da queste condizioni.

## Circostanze del tiro

Con effetti locali attivi trovi **Contesto** nella prova, nel suo dettaglio e
nella preparazione/dettaglio dell'attacco. Per una prova con Accecato dichiara
**Richiede vista: sì/no**. Per Spaventato dichiara **Fonte paura in vista: sì/no**,
secondo la linea di vista stabilita con il DM. `?` significa sconosciuto, non no.
I pulsanti ciclici passano da `?` a no, sì e di nuovo `?`.

Solo i dati che possono cambiare l'esito sono necessari. Per esempio, Prono dà
già svantaggio all'attacco: conoscere anche la visibilità della fonte di paura
non può aggiungerne un secondo. Una prova che richiede vista con Accecato è
invece un fallimento automatico, senza bisogno di risolvere anche la paura.
Il core verifica questo anche per le altre combinazioni con Avvelenato, Ira e
modalità manuale.

Se un fatto necessario è sconosciuto, **Lancia resta bloccato prima di RNG**.
Con vista richiesta e Accecato compare **Fallimento automatico**: non viene
lanciato un dado né prodotto un risultato naturale fittizio: il canvas è sostituito
da “Nessun dado”. Per una prova che
non richiede vista, scegli no; non si inventa uno svantaggio a tutte le prove.
Le righe irrilevanti del contesto sono disabilitate, ad esempio vista per una
salvezza o paura per una salvezza DES.

**Usa contesto** applica la bozza al tiro corrente senza scrivere flash;
Indietro la scarta. Cambiare contesto scarta un risultato preparato, non modifica
risorse. Una nuova prova o un nuovo attacco riparte con circostanze sconosciute;
Rilancia nello stesso tiro conserva quelle dichiarate. Aggiornale se cambia la
situazione: non esiste rilevamento automatico di vista, luce, fonte o movimento.

Vantaggi e svantaggi sono booleani: non si sommano. Una o più cause di ciascun
segno si annullano una sola volta. La modalità manuale può quindi rappresentare
un'altra fonte di vantaggio o svantaggio, senza duplicare il bonus di competenza.

**Effetti tiro: esclusi** è una decisione esplicita del DM, valida per quel tiro.
Esclude i modi automatici di Avvelenato/Ira e i nuovi effetti, compreso il
fallimento per vista. Non elimina le condizioni salvate, non cambia bonus/danni
e non rende verificati bonus importati sconosciuti. I blocchi sui valori
importati restano: il dado libero è un percorso distinto e non una conversione
riuscita della prova.

## Persistenza, vecchie copie e import

PDD23, tag `PDDN`, aggiunge un byte prima del CRC: bit 1 Accecato, 2 Spaventato,
4 Prono, 8 Trattenuto, valori ammessi 0–15. Dimensione **8854 + 552 × voci magia**,
massimo **75094 byte**. Il contesto del singolo tiro non è persistente.

PDD1–PDD22 sono letti con i nuovi effetti disattivati, mantenendo Avvelenato,
annotazioni, cariche, costi, timer e preferenze precedenti. Nessuna scrittura al
boot. Fixture PDD22 prodotta dall'encoder originale v0.43 `b340a8c` con dati
sintetici. Dopo un salvataggio PDD23 usa firmware v0.44 o successivo; non
cancellare NVS per aggirare un downgrade. Layout partizioni invariato.

USB8 e backup PC v11 non cambiano. Gli effetti nuovi sono configurazioni locali;
una nuova importazione confermata li azzera, l'annullamento li conserva. Le
note importate non li attivano e la raccolta PC non viene modificata.

## Verifiche e limiti

- **107.136 casi C**: tutte le combinazioni dei quattro effetti, prove/abilità/
  salvezze/attacchi, caratteristiche, modalità manuali, Avvelenato, Ira,
  esclusione ed ogni contesto parziale valido. Confronto con un oracolo
  indipendente che enumera i contesti completi e identifica i fatti influenti.
- Numero di estrazioni e risultati controllati; blocchi e fallimenti automatici
  prima di RNG con output intatti; nessuna mutazione dello stato nei tiri.
- Note separate, esclusione che non aggira bonus sconosciuti, copie precedenti,
  riposi/round/tempo senza recuperi dedotti, tutti i 16 valori del byte e 240
  valori invalidi con CRC valido; record massimo e stato/output/generazione intatti.
- Migrazione PDD1–22 senza scritture al boot, primo salvataggio e riavvio con
  effetti conservati e vecchia banca identica.
- LVGL host: bozza/confronto/conferma/errore/Annulla, tasti/import protetti,
  vista sconosciuta/necessaria/non necessaria, fallimento senza dado, esclusione
  DM, paura visibile, annullamento booleano delle fonti di svantaggio, salvezze
  DES/COS, attacchi, dado libero e importazione annullata/confermata.
- Suite host completa e 41.411 confronti PC → USB → C; 144 test JavaScript,
  build Vite/HTML autonomo, sanitizzatori e build ESP32-S3. Dettagli in
  [FIRMWARE.md](FIRMWARE.md).

Anteprime: `firmware-host-preview/roll-conditions-preview.png` e
`roll-conditions-*.png` nell'archivio, render del codice LVGL su host con dati
inventati. Nessun flash, collaudo fisico, lettura Beyond o raccolta personale
modificata. Altre condizioni, recuperi/durate/fonti multiple, movimento, avversari,
percezione passiva e requisiti di magie/azioni restano manuali.
