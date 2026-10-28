# Wi-Fi sul dispositivo — v0.78

Wi-Fi **2,4 GHz** opzionale. La scheda funziona offline. Il firmware parte con
radio spenta se non è stata salvata e abilitata una rete. Wi-Fi collegato non
significa ancora importazione autonoma o sincronizzazione con Beyond.

## Configurazione da telefono o PC

1. Sul Pocket: **Aspetto → Schermo → Wi-Fi → Configura rete**.
2. Sul telefono/PC connettiti alla rete **PocketDD-…**. La password temporanea
   è sul display. Se il telefono segnala assenza di Internet, resta connesso.
3. Apri **http://192.168.4.1** nel browser. Inserisci SSID e password della
   rete di casa a 2,4 GHz, poi **Prova e salva la rete**.
4. Attendi la conferma di rete collegata e salvata sul portale/display.
   Il solo invio del modulo non è una conferma. Con password errata riprova;
   nome e password della precedente rete salvata restano conservati se la
   nuova connessione non riesce.
5. Torna alla rete abituale del telefono/PC. Il portale si chiude entro cinque
   minuti; la connessione salvata resta attiva. Al riavvio viene riprovata.

Non è necessario che il PC resti acceso per configurare il Wi-Fi. Puoi usare
**Spegni** per interrompere connessione e portale; la rete resta memorizzata,
con riaccensione esplicita da **Accendi**. **Dimentica → Conferma** cancella la
configurazione Wi-Fi, conservando scheda, aspetto e backup del personaggio.
I tasti configurabili continuano a navigare i controlli dell’interfaccia.

Supportati WPA2-Personal e rete aperta solo con scelta esplicita del modulo.
Password 8–63 caratteri ASCII oppure PSK esadecimale di 64; SSID UTF-8 fino a
32 byte. Reti solo 5 GHz, enterprise e accessi tramite portale pubblico non
sono supportati. Il modulo non ricerca automaticamente i nomi delle reti.

## Errori e sospensione

“Collegato” indica un indirizzo IP locale ottenuto, non una prova di accesso
Internet/Beyond. Un errore di memoria indica **salvataggio non confermato**:
controlla il dispositivo prima di riavviare. Non viene dichiarato successo
per il solo collegamento radio. Dopo un errore resta disponibile il gioco offline.

Tre tentativi entro circa 30 secondi per una connessione; una disconnessione
successiva riapre il ciclo. La sospensione arresta prima radio e portale,
aspettando la conferma del servizio; al risveglio riprende la rete salvata solo
se prima era richiesta. Il portale non viene riaperto automaticamente.
Una radio che non si arresta impedisce la sospensione anziché simulare uno
spegnimento riuscito. Anche prima dello spegnimento PMIC viene richiesta la pausa.

## Memoria e riservatezza

Configurazione in `pdd_nvs`, namespace **pdd_wifi**, chiave `config`;
completamente distinta da `pdd_state` e dalle schede. Nessuna credenziale entra
in JSON del PG, diagnostica, storico, backup o pacchetti pubblicati.
La memoria NVS della build non è cifrata: non protegge da lettura fisica della flash.

Le nuove credenziali vengono salvate solo dopo la connessione alla rete
corrispondente e l’ottenimento dell’IP, mai al ricevimento del modulo. Nessuna
cancellazione globale NVS. Il driver usa solo RAM per le proprie impostazioni,
con NVS SDK disabilitata esplicitamente: la partizione di default non serve.

Portale temporaneo WPA2 con password casuale di 16 caratteri, token del modulo,
controlli Host/Origin e ricezione solo dall’interfaccia AP. Nessun servizio di
configurazione disponibile dalla LAN domestica. Le password non vengono
stampate nei log né ripresentate dal server. Il passaggio SAR/RF della sorgente
casuale è serializzato con i tiri; la sorgente offline ritorna dopo arresto RF.

## Cosa è provato

Compilazione ESP-IDF per la scheda standard 1.75. Parser dei dati del portale,
20.000 corpi malformati, portale nel browser con backend simulato, interfaccia LVGL e servizio di produzione con SDK
simulato: errori d’inizializzazione/start/stop, autenticazione del modulo,
scadenza, fallback, NVS fallita, pause confermate, riaggancio e separazione dei
namespace. Queste sono prove software, **non prove della radio o della NVS reale**.

Sul tuo hardware restano da verificare collegamento, password errata, riavvio,
sospensione/risveglio, dimentica rete e consumo. Non sostituiscono il collaudo
USB del PG, ancora aperto.
