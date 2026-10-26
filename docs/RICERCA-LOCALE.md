# Ricerca locale nello zaino e nelle magie — v0.63

La ricerca per nome sul dispositivo riduce lo scorrimento degli elenchi fino
a 60 oggetti e 120 magie/capacità. Funziona offline e conserva l'ordine e
l'identità originali delle voci. Grafica del compagno e tasti configurabili
rimangono quelli approvati; il PC continua a servire per preparazione e backup.

## Cercare una voce

Nello **Zaino**, aprire **Cerca nello zaino**. Nell'elenco **Magie**, aprire
**Cerca** tra Usi e Risorse. Il pulsante ha una lente in pixel art.

La tastiera offre lettere maiuscole, numeri, spazio, apostrofo, trattino,
punto, più e barra, in quattro pagine. Le frecce cambiano pagina; il limite
è **20 caratteri**. Il contatore in alto mostra le voci corrispondenti mentre
si scrive. La ricerca è una parte letterale del nome e ignora maiuscole e
minuscole: `light` trova anche `Daylight` e `Light step`. Gli spazi interni e
la punteggiatura contano; `wand light` non trova `Wand of Light`.

- **Cancella** rimuove l'ultimo carattere.
- **Azzera** svuota la bozza; **Mostra** con testo vuoto ripristina l'elenco completo.
- **Mostra** applica il testo e torna alla prima pagina dei risultati.
- **Indietro** scarta la bozza e conserva filtro e pagina precedenti.

Gli spazi iniziali/finali vengono rimossi quando si applica il testo. I nomi
originali rimangono invariati. La tastiera non aggiunge accenti o Unicode:
ricerca i nomi ASCII già rappresentati nella copia firmware. Non cerca nel
testo lungo delle descrizioni e non ordina i risultati per rilevanza.

## Risultati, dettagli e utilizzi

L'elenco indica numero di risultati e pagina corrente. Le frecce scorrono
solo le corrispondenze. Nessun risultato mostra **Nessun nome trovato** e
mantiene disponibile Cerca per cambiare o rimuovere il filtro.

Nello zaino il pulsante mostra la ricerca attiva, abbreviata se necessario;
nell'elenco magie **Cerca*** e il bordo oro segnalano il filtro. Nell'editor
il testo completo va a capo. L'asterisco accanto a un'arma mantiene il
significato precedente di arma selezionata.

I filtri **Magie / Capacità / Tutte** si combinano con il testo, conservandolo.
Cambiare categoria torna alla prima pagina. Una ricerca nello zaino e una
nelle magie restano separate. Tornare dai dettagli mantiene filtro e pagina.
I nomi duplicati restano voci distinte e gli oggetti con quantità zero restano
consultabili; i normali blocchi di utilizzo continuano ad applicarsi.

Ogni riga filtrata apre l'indice originale, anche quando è l'oggetto 59 o
la capacità 119. Consumi, cariche, equipaggiamento, slot e utilizzi operano
sulla voce originale e usano le conferme esistenti. La ricerca non consuma,
non corregge saldi e non applica effetti. Nessuna regola viene dedotta dal nome.

## Sessione e salvataggi

Le ricerche e la bozza sono **solo in RAM**. Aprire, digitare, applicare o
scartare una ricerca non scrive flash, non estrae dadi, non cambia lo storico
e non sostituisce l'ultima modifica annullabile. Funziona anche quando la
memoria di gioco non è scrivibile, finché la scheda è consultabile.

Nell'editor, tasti, importazione, ripristino e pausa automatica sono bloccati,
così la bozza non viene interrotta. La pausa di un elenco conserva il filtro.
Un riavvio o uno spegnimento lo azzera. Annullare importazione/ripristino
conserva i filtri; confermare una nuova copia li azzera e riporta entrambi
gli elenchi alla prima pagina. I filtri di categoria delle magie restano
una scelta di navigazione della sessione.

PDD28/PDDS, PDH2 con lettura PDH1, USB9, backup nativo v1, backup PC v11,
diagnostica v1 e partizioni sono invariati. I filtri non entrano nel backup
nativo e non modificano le schede della raccolta PC. Nessuna dipendenza da
Foundry, rete o servizi Beyond durante l'uso.

## Verifiche e limiti

Il core verifica **11.506** casi di sottostringhe/maiuscole/indici, limiti
60/120, intersezione delle categorie, nomi duplicati e oggetti esauriti,
spazi/punteggiatura, query troppo lunghe o non ASCII, dati non disponibili e
indici mancanti. L'output della normalizzazione resta invariato se rifiutato.

La UI host esercita le **112 viste** e verifica le quattro pagine della
tastiera, limite e testo lungo, risultati vuoti, scorrimento, cancel/clear,
ricerca con memoria non scrivibile, tasti/import protetti e pausa. Sono provati
il consumo della pozione all'indice originale 59 e della capacità 119 con
nomi duplicati, poi importazione e ripristino nativo confermati/annullati.
Suite completa, sanitizzatori e build sono riportati in [FIRMWARE.md](FIRMWARE.md).

Le catture `search-*.png` e `catalog-search-gallery.png` provengono dal vero
codice LVGL su host con dati inventati. Nessuna nuova lettura Beyond,
modifica della raccolta personale o prova sul pannello fisico è stata eseguita.

Restano possibili affinamenti di preferiti, ordinamento, ricerca nelle
descrizioni e inserimento più rapido, da valutare con la scheda e i comandi
reali. Il prossimo controllo fisico H30 è in
[COLLAUDO-HARDWARE.md](COLLAUDO-HARDWARE.md).
