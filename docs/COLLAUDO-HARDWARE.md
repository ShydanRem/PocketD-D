# Scheda di collaudo fisico PocketD-D

Modello vuoto: **nessuna prova fisica eseguita**. Compilare una copia privata
seguendo [PRIMO-AVVIO.md](PRIMO-AVVIO.md), senza pubblicare dump o schede personali.

- Data e operatore:
- Sigla e revisione fisica della scheda:
- Firmware, commit e archivio utilizzati:
- Sistema operativo / Python / esptool:
- Collegamento e alimentazione:
- Backup originale conservato e recovery disponibile:
- Personaggio di collaudo sintetico / altra scheda:
- Rapporti diagnostici prima/dopo (nomi privati):

Esiti ammessi: **Non eseguito / Passato / Fallito / Bloccato**. Ogni `Passato`
richiede un'osservazione reale; indicare eventuali prove parziali nelle note.

| ID | Prova | Esito | Osservazione / anomalia |
| --- | --- | --- | --- |
| H01 | Modello standard corretto, backup fabbrica e installazione | Non eseguito | |
| H02 | Avvio e stabilità della home | Non eseguito | |
| H03 | Resa: testi, icone, colori e ritagli | Non eseguito | |
| H04 | Prova tocco locale: bersagli, orientamento e comandi in tutte le zone usate | Non eseguito | |
| H05 | Prova tasti locale e PWR breve, BOOT breve/lungo: configurazione, una azione, riavvio | Non eseguito | |
| H06 | Tasti bloccati nelle conferme; risveglio senza azione | Non eseguito | |
| H07 | Luminosità e attenuazione 30 s, ripresa e preferenze persistenti | Non eseguito | |
| H08 | Diagnostica USB: rapporto valido, versione e stati osservati | Non eseguito | |
| H09 | Import USB: annulla/conferma e confronto valori della fixture | Non eseguito | |
| H10 | PF, temporanei e annullamento | Non eseguito | |
| H11 | Prove/dadi: bonus, facce, accelerazione e storico | Non eseguito | |
| H12 | Arma: attacco e danni spiegati | Non eseguito | |
| H13 | Magie: ordinari/patto, utilizzi e concentrazione | Non eseguito | |
| H14 | Breve, dado vita e lungo con recuperi espliciti | Non eseguito | |
| H15 | Quantità, cure da oggetto e monete | Non eseguito | |
| H16 | Effetto CA, durata e scadenza | Non eseguito | |
| H17 | Riavvio normale conserva progressi e impostazioni | Non eseguito | |
| H18 | Backup: file completo, anteprima coerente, raccolta PC conservata | Non eseguito | |
| H19 | Ripristino: annulla/conferma, preferenze/storico e riavvio | Non eseguito | |
| H20 | Ripetizione dei percorsi: heap, stack e fluidità | Non eseguito | |
| H21 | Interruzioni durante scritture: protocollo concordato e recupero | Non eseguito | |
| H22 | Batteria, consumi, sospensione e spegnimento | Non eseguito | Software integrato v0.62; misure e comportamento fisico da verificare |
| H23 | Scheda sintetica dopo backup: danni a zero PF, critico, morte istantanea, annulla, riavvio/ripristino | Non eseguito | [Procedura v0.56](DANNI-ZERO-PF.md) |
| H24 | Scheda sintetica: tiro morte base/modificato, annullamento dei segni, bonus, storico, annulla e reset bozza | Non eseguito | [Procedura v0.57](TIRI-MORTE.md) |
| H25 | Scheda sintetica: riferimenti A/B/C, isolamento turno proprio/reazione, scadenze, cap PF, annulla e backup/riavvio | Non eseguito | [Procedura v0.58](TURNI-ESTERNI.md) |
| H26 | Scheda sintetica: nomi A/B/C, tastiera/pagine/limite, anteprima, annulla, timer invariati e backup/riavvio | Non eseguito | [Procedura v0.59](NOMI-TURNI.md) |
| H27 | Scheda sintetica: due/tre alternative, scelta esplicita, blocchi/saldi, reazione, annulla, nuovo colpo e backup/riavvio | Non eseguito | [Procedura v0.60](COSTI-ALTERNATIVI.md) |
| H28 | Scheda sintetica: pannello PF, cura/cap, temporanei assegna/correggi/zero, importi danni, annulla, tasti e riavvio | Non eseguito | [Procedura v0.61](USO-AL-TAVOLO.md) |
| H29 | Alimentazione v0.62: telemetria reale, USB/ricarica, avvisi, pausa/risveglio/tasti, arresto e persistenza | Non eseguito | [Procedura](ALIMENTAZIONE.md) |
| H30 | Ricerca v0.63: tastiera, filtri, nessun risultato, dettagli/consumi originali, tasti, pausa e reset | Non eseguito | [Procedura](RICERCA-LOCALE.md) |
| H31 | Preparazione v0.64: anteprima, conferma, risorse/concentrazione conservate, innati, ricerca, tasti, Annulla e persistenza | Non eseguito | [Procedura](PREPARAZIONE-MAGIE.md) |
| H32 | Installer browser: prima installazione, aggiornamento/layout, porta/BOOT, avanzamento, verifica MD5, reset e progressi conservati | Non eseguito | [Procedura](INSTALLAZIONE-USB.md) |
| H33 | Web Serial PC: invio/anteprima/conferma, annulla/interrompi, backup esatto e ripristino, porta libera e raccolta conservata | Non eseguito | [Procedura](USB-BROWSER.md) |
| H34 | Quantità v0.65: tastiera/limiti, prima-dopo, cariche/PF conservati, arma a zero, tasti, Annulla e persistenza | Non eseguito | [Procedura](QUANTITA-OGGETTI.md) |

## Problema riproducibile

- ID della prova e schermata:
- Stato iniziale e impostazioni tasti:
- Passaggi esatti, incluso gesto breve/lungo e conferma:
- Risultato atteso:
- Risultato osservato:
- Frequenza / riavvii inattesi:
- Rapporti prima/dopo e foto/video senza dati personali:
- Riproducibile dopo un normale riavvio:
- Passaggi successivi concordati:

## Decisione sul prototipo

- Avvio, comandi e sessione offline verificati:
- Persistenza dopo normale riavvio verificata:
- Acquisizione e ripristino nativo verificati:
- Problemi che impediscono l'uso al tavolo:
- Limiti di regole dichiarati e gestibili manualmente:
- Misure energetiche e tenuta ai guasti ancora aperte:

Nessun risultato automatico della diagnostica compila queste voci da solo.
