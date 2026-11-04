# Magie: costo e tiro nello stesso percorso · v0.108

Dal Pocket: **Magie → scegli la magia → Lancia**. Per una magia generica scegli:

- **Solo costo**: il percorso precedente, con anteprima e conferma; effetti al tavolo.
- **Danni**: tira una formula esplicita. PF del bersaglio, colpire, TS e difese
  si risolvono al tavolo; Pocket non applica il danno ai tuoi PF.
- **Cura me**: tira e applica ai tuoi PF, entro il massimo effettivo dopo il
  costo e l'eventuale cambio di concentrazione. Non disponibile a PF pieni.
- **Cura altri**: tira la cura; conserva i tuoi PF, salvo le conseguenze già
  previste dal cambio di concentrazione. Nessuna scheda esterna viene modificata.

Scegli dado, quantità e bonus, poi **Rivedi → Tira e salva**. Il pulsante della
caratteristica copia il suo modificatore come bonus della formula: non aggiunge
competenza né bonus al tiro per colpire/CD. Sono disponibili d4/d6/d8/d10/d12/d20,
da 1 a 18 dadi e bonus da −15 a +60, con minimo zero sia per danni sia per cure.
Il risultato della cura distingue la formula tirata dai PF effettivamente recuperati.

**La proposta iniziale 1d6+0 è un valore da impostare, non una conversione della
magia.** La formula è una bozza in RAM per questo lancio: non viene dedotta da
nome/testo/edizione, non viene salvata nel PG e non scala cambiando lo slot.
Controllala prima della conferma, comprese capacità, regole della tua edizione,
critici e aumenti di livello. Formule miste, effetti speciali e ulteriori bersagli
restano manuali. Questo non conclude il blocco delle automazioni degli incantesimi.

I profili riconosciuti di True Strike, Hex, Marchio e Punizione, le capacità e
le voci informative mantengono i propri percorsi. Il nuovo tiro riguarda solo
magie generiche, preparate e utilizzabili: non aggira Ira, zero PF, slot esauriti
o utilizzi limitati. Gli slot ordinari e patto restano scelte distinte; un lancio
gratuito usa il livello base, senza consumare uno slot implicito.

## Conferma e recupero

La scelta e l'anteprima non consumano risorse e non tirano dadi. Dettagli mostra
anche CA, massimi e concentrazione prima/dopo il costo; la cura non nasconde
un'eventuale riduzione dei PF causata dalla perdita di un massimo temporaneo.

La conferma fissa prima un solo risultato e salva costo e cura in una singola
scheda candidata. Se il salvataggio fallisce, **Riprova** mantiene facce, totale e
spese. Durante questa attesa non si può cambiare formula, annullare il tiro o
importare un'altra scheda. Se la memoria è già indisponibile non viene tirato
alcun dado. Un tiro gratuito senza cambi di stato non riscrive la scheda in NVS.

Dopo il salvataggio partono i dadi animati; il totale compare a fine volo.
**Riepilogo** consulta il risultato e le spese già fissati, senza ripetere il
lancio. Tornando alla Home, Annulla ripristina insieme costo e PF dell'ultima
modifica; lo storico mantiene il fatto che il tiro è stato eseguito, come negli
altri percorsi. Un errore del registro resta separato dal salvataggio della scheda:
la voce rimane in attesa e blocca nuovi tiri fino a risoluzione.

PWR breve seleziona, BOOT breve attiva, BOOT lungo torna indietro nei nuovi
editor; dopo un tiro non salvato il ritorno non scarta il risultato. Le
assegnazioni personali dei tasti restano invariate. Nessun nuovo formato PG,
backup, USB, partizione o pacchetto SD; non occorre reimportare i personaggi.

## Verifica

648 combinazioni native di formula, destinazione e pagamento ordinario/patto;
saldi, modifiche complete, limite dei PF, temporanei, perdita del massimo da
concentrazione, utilizzi gratuiti, minimo zero e richieste invalide senza RNG.
Scenari LVGL con indice sorgente 119, nomi lunghi, tasti, annullamento,
anteprima obsoleta, errore/riprova con dadi fissi, undo, registro in attesa,
trucchetto senza NVS, risultato e ricevuta consultabili e assenza del canvas.

Sono prove sintetiche del motore e del renderer. Non costituiscono una nuova
lettura Beyond, una prova USB/radio o un collaudo del dispositivo fisico.
Per la verifica sul Pocket usa un PG di prova: cura personale con slot, cura
esterna, danno e annullamento; confronta slot/utilizzi/PF e lo storico.

Verifiche di release: 357 test JavaScript e suite nativa completa passati,
157/157 viste esercitate. ASan/UBSan sui nuovi scenari e compilazione ESP32-S3
riusciti. La prova JavaScript usa servizi locali sintetici; nessun account reale.
