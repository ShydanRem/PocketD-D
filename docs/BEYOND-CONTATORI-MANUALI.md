# Beyond: contatori manuali delle azioni — v0.76

Le azioni con contatore `limitedUse.resetType: 4` vengono conservate come
**saldi manuali separati dalle capacità**. Non vengono più presentate come un
numero di attivazioni da consumare automaticamente premendo Usa.

Il residuo è esattamente **massimo − numberUsed**, dopo le formule supportate
basate su caratteristica finale e competenza. Non viene dedotto se il contatore
rappresenti oggetti attivi, PF di una creazione, un limite o altre informazioni.
Non viene dedotto un legame fra modalità che condividono una capacità.
Il massimo è una fotografia all’importazione: dopo cambi di livello o
caratteristica va verificato e corretto manualmente.

## Uso

- Nell’anteprima Beyond compaiono nome, residuo e massimo di ogni contatore.
- Sul PC: **Stato, risorse e note → Contatori manuali**. Correggere residuo e
  massimo, quindi Salva. Annullamento, quota o conflitto conservano la scheda.
- Sul dispositivo: **Usi**, oppure Contatori dal menu. `−1`, `+1` e Prossimo
  modificano i saldi già supportati dal firmware. È disponibile l’annullamento.
- La capacità corrispondente resta consultabile come nota e segnala il contatore
  separato. Non espone un consumo automatico. Riposi e uso di altre capacità
  lasciano invariati questi saldi.

Gli utilizzi con recupero breve/lungo supportato restano capacità consumabili
separate. Le cariche degli oggetti continuano a usare l’inventario; non vengono
duplicate nei contatori.

## Lettura reale del 28 ottobre 2026

Personaggio autorizzato 144777811, connettore PocketD-D esistente, senza
credenziali, Foundry o scritture remote. **HTTP 200, 426.718 byte, JSON valido**.
La conversione è consentita e resta parziale.

| Contatore manuale | Residuo / massimo |
| --- | --- |
| Magical Tinkering | 4 / 4 |
| Infuse Item | 1 / 4 |
| Eldritch Cannon: Flamethrower | 50 / 50 |
| Eldritch Cannon: Force Ballista | 35 / 50 |
| Eldritch Cannon: Protector | 50 / 50 |

Questi numeri sono saldi del sorgente: **non sono dichiarati colpi disponibili**
e non dimostrano che tutte le modalità siano contemporaneamente attive.
**Create Eldritch Cannon** resta un utilizzo distinto: **0/1**, recupero lungo.
Spendere uno slot per ricrearlo, attivazione/distruzione e collegamenti alle
modalità restano da configurare; questa versione non li automatizza.

Caratteristiche **8/18/14/18/10/10**, PF **32/73**, CA **17**, 41 voci inventario.
Nel parser e codec C eseguiti sul PC coincidono i cinque saldi, tutte le quantità,
i campi operativi delle magie/capacità e i 30 tiri della proiezione.
Sono verifiche della risposta e della conversione, non un confronto con una
schermata autenticata Beyond né una prova USB sul dispositivo.

Il sorgente completo è stato elaborato in memoria e non pubblicato o salvato.
Una copia temporanea della sola proiezione nativa, con permessi 0600, è stata
eliminata dopo i controlli. Nessuna raccolta personale aperta o modificata.

## Limiti e compatibilità

Fino a **8 contatori**, massimo 999, nomi PC fino a 40 caratteri; nomi sul
dispositivo ASCII fino a 32, come i contatori già esistenti. Zero residuo è
un valore valido. Il nono contatore interrompe la conversione: nessun saldo
viene tagliato. Duplicati identici della stessa azione non duplicano il saldo;
ID discordanti interrompono la conversione. Modalità con ID distinti restano
separate, senza unione di risorse presunta.

Formule, costi o riferimenti a risorse non supportati vengono segnalati e
rimangono note. Non vengono creati saldi approssimativi. Per classi senza
origine/livello univoci l’azione non viene convertita come risorsa.

**Backup PC v18** solo quando il campo opzionale `extended.manualResources`
è presente. Schede precedenti e raccolte miste rimangono leggibili, senza
migrazione all’avvio. Le app precedenti respingono v18. Vecchie importazioni
non vengono riscritte: reimportare crea una nuova scheda e non conserva
automaticamente i progressi di quella vecchia.

**Nessun nuovo protocollo USB, stato nativo o partizione**: si usano gli otto
contatori già presenti; USB9 o il formato superiore richiesto dagli altri campi,
PDD34/PDDY. I contatori arrivano al dispositivo con il pacchetto ordinario.

**Testi estesi rimandati per scelta dell’utente:** manteniamo i tagli attuali
(PC 6.000 caratteri per capacità, dispositivo 500) e i relativi avvisi.
Non sono un blocco da risolvere in questa fase di chiusura Beyond.

## Verifiche sintetiche

253 test JS. Nuovi casi: contatori separati dalle capacità, formula, stato zero,
limiti 8/999, duplicati e riferimenti sconosciuti, backup v18 e raccolte miste.
**120 schede inventate** attraversano convertitore, pacchetto USB esistente,
parser e codec C. Il browser verifica anteprima, creazione, editor, annullamento,
quota atomica, valori invalidi, reload, vecchie schede, download e viewport 320/390.
Il renderer LVGL verifica otto contatori, limite 999, errore/riprova e annullamento,
nota passiva e nessun tiro casuale; 119 viste, anche con ASan/UBSan.

La verifica USB fisica rimandata e il riavvio col PG personale restano aperti.
