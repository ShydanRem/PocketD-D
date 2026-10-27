# Preset di classe — studio visivo e prossimo blocco

La tavola in `design/classes/concept-board.png` propone dodici look per lo stesso
avatar umano: **Barbaro, Bardo, Chierico, Druido, Guerriero, Monaco, Paladino,
Ranger, Ladro, Stregone, Warlock e Mago**. Abiti, armature e oggetti cambiano
realmente forma: pelliccia/ascia, giacca/liuto, cotta/mazza/scudo, foglie/bastone,
piastre/spada, fasce, piastre sacre, cuoio/arco, cappuccio/pugnali, cappotto/sfera,
tomo/bacchetta e veste/cappello/grimorio.

**È uno studio grafico: i preset non sono ancora attivabili nel software.** La
v0.69 completa il nuovo gruppo di razze e prepara spazio e riferimenti per questo
blocco. Il file `design/classes/presets.json` è una specifica visiva, non un
catalogo eseguibile. L'Artificiere e le sottoclassi sono successivi alle dodici
classi base.

## Risultato previsto

Ogni preset deve combinarsi con **razza e silhouette già scelte**, mantenendo
muso, orecchie, corna, piume, guscio e coda. Il corpo umano maschile della tavola è
soltanto il riferimento per mostrare gli abiti; non limita i corpi supportati.
I look di guerriero e paladino devono essere distinguibili, così come quelli dei
tre incantatori. Non basta cambiare il colore del mantello.

Il preset è **cosmetico**: non aggiunge un'arma all'inventario, non equipaggia
un'armatura reale e non cambia CA, PF o attacchi. Un eventuale futuro collegamento
agli oggetti reali richiederà una configurazione separata e verificata.

## Ordine di implementazione

Prima degli strati grafici, integrare il [caricatore dei pacchetti da SD](ASSET-SD.md). La SD è l’archivio principale previsto per i cataloghi: la varietà dei preset non deve dipendere dalla capienza della flash. Nel firmware resta un look di ripiego.

1. **Strati e ancoraggi.** Separare anatomia/testa, corpo vestito, accessori,
   copricapo e arma. Definire gli agganci per corpi piccoli, ordinari e robusti e
   proteggere code, ali ripiegate e gusci. Verificare dove allargare il buffer e
   l'area dell'avatar per arco, scudo e bastone, senza invadere PF o pulsanti.
2. **Sprite effettivi.** Produrre gli abiti delle dodici classi sulla griglia del
   renderer; normalizzare silhouette e mani e controllare i quattro stati.
   Usare strati riutilizzabili per coerenza e animazione. La SD può ospitare
   anche sprite dedicati quando migliorano la qualità; evitare duplicazioni
   inutili senza limitare le combinazioni per la sola capienza della flash.
3. **Scelta sul dispositivo e sul PC.** Aggiungere un campo di aspetto con
   anteprima, conferma, annullamento, tasti fisici e persistenza. Conservare un
   look originale e la possibilità di scegliere manualmente un altro stile.
4. **Proposta da Beyond.** Usare solo classi esplicite riconosciute. Se il PG è
   multiclasse o la classe è sconosciuta, far scegliere il preset senza decidere
   arbitrariamente una classe dominante. Nessuna sovrascrittura delle schede
   esistenti.
5. **Verifica.** Controllare tutte le anatomie, i due corpi, i salvataggi, gli
   errori di memoria, l'isolamento dall'inventario e la memoria del dispositivo.
   La tavola di stile non sostituisce questi controlli.

La memorizzazione senza blocchi 2×2 duplicati conserva un firmware di base più
piccolo e tutti i pixel approvati. Con la SD, il vincolo da verificare per i
preset è la memoria degli asset attivi e il tempo di caricamento/rendering,
non la necessità di incorporare tutto il catalogo nella flash. La cache attuale
non è ancora un lettore SD; la fluidità va misurata sul dispositivo.
