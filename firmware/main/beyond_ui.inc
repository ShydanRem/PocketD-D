/* Beyond account pairing and native import. No credentials in PGs or UI. */
enum {B_OPEN=5440,B_BACK,B_ERASE,B_READ,B_CANCEL,B_WIFI,B_NOTES,B_NOTES_BACK,B_PREV,B_NEXT,B_DIGIT=5450,B_ACCOUNT=5460,B_PUBLIC,B_REFRESH,B_FORGET,B_UNLINK,B_REMEMBER,B_APPROVE,B_RETRY,B_PICK=5480,B_END=5500};
static char beyond_id[16];
static const char *beyond_notice="Inserisci l ID di un PG pubblico";
PD_EXT static pd_beyond_report beyond_report;
static pd_beyond_info beyond_visible;
static unsigned beyond_note,beyond_focus,beyond_target_count;
static bool beyond_keyboard,beyond_wifi_return,beyond_source_account;
static bool beyond_pairing,beyond_approved,beyond_remember=true;
static char beyond_nonce[33],beyond_code[7];
static uint32_t beyond_pair_started;
static unsigned beyond_catalog_page;
static lv_obj_t *beyond_targets[16];
/* Only operation/ID are retained for a retry, never source JSON or credentials. */
static unsigned beyond_retry_operation,beyond_failure;
static char beyond_retry_id[16];
static bool beyond_waiting;
static uint32_t beyond_wait_started;
static bool beyond_request(unsigned operation,const char *id);
static bool beyond_network_flow(void){return beyond_wifi_return;}
static bool beyond_local(void){return page==BEYOND_ACCOUNT||page==BEYOND_PAIR||page==BEYOND_FORGET||page==BEYOND_FORM||page==BEYOND_BUSY||page==BEYOND_REPORT||page==BEYOND_CONNECT||page==BEYOND_RECOVER||(import_pending&&import_from_beyond);
}
static const char *beyond_error(unsigned error){
    switch(error){
        case PD_BEYOND_CANCELLED:return "Richiesta annullata: PG conservato";
        case PD_BEYOND_MEMORY:return "Memoria insufficiente: nessun taglio";
        case PD_BEYOND_TIMEOUT:return "Tempo scaduto: puoi riprovare";
        case PD_BEYOND_NO_WIFI:return "Collega prima il Wi-Fi";
        case PD_BEYOND_TIME:return "Orologio non pronto: controlla Wi-Fi";
        case PD_BEYOND_DENIED:return "PG privato o accesso rifiutato";
        case PD_BEYOND_NOT_FOUND:return "Personaggio non trovato";
        case PD_BEYOND_RATE:return "Troppe richieste: riprova piu tardi";
        case PD_BEYOND_REDIRECT:return "Reindirizzamento rifiutato";
        case PD_BEYOND_MIME:return "Risposta diversa dal JSON atteso";
        case PD_BEYOND_SIZE:return "Scheda troppo grande: nessun taglio";
        case PD_BEYOND_INVALID:return "Dati non convertibili: PG conservato";
        case PD_BEYOND_SESSION:return "Accesso rifiutato: verifica dal PC";
        case PD_BEYOND_IDENTITY:return "Identita o token non validi";
        case PD_BEYOND_CATALOG_ERROR:return "Elenco non riconosciuto: nessun taglio";
        case PD_BEYOND_STORAGE:return "Accesso non salvato: riavvia";
        case PD_BEYOND_BUSY:return "Operazione occupata: riprova";
        case PD_BEYOND_ENGINE:return "Conversione non riuscita: PG conservato";
        default:return "Rete o servizio non raggiungibile";
    }
}
static void beyond_open(void){
    page=BEYOND_ACCOUNT;
    beyond_notice="";
    beyond_focus=0;
    beyond_keyboard=false;
    beyond_wifi_return=false;
    pd_beyond_info account;pd_beyond_snapshot(&account);
    if(account.configured&&!account.listed&&!account.locked&&!account.reconnect)beyond_request(PD_BEYOND_LIST,NULL);
}
static void beyond_mark_focus(void){
    if(beyond_focus>=beyond_target_count)beyond_focus=0;
    for(unsigned i=0;i<beyond_target_count;i++){lv_obj_set_style_outline_color(beyond_targets[i],lv_color_hex(PD_GOLD),0);
        lv_obj_set_style_outline_pad(beyond_targets[i],2,0);
        lv_obj_set_style_outline_width(beyond_targets[i],beyond_keyboard&&i==beyond_focus?2:0,0);
    }
}
static lv_obj_t *beyond_control(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled){
    lv_obj_t *b=button(text,x,y,w,h,action,enabled&&!library_failed);
    lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
    if(beyond_target_count<16)beyond_targets[beyond_target_count++]=b;
    return b;
}
static page_t beyond_return(void){return beyond_source_account?BEYOND_ACCOUNT:BEYOND_FORM;
}
static bool beyond_online(const pd_wifi_info *wifi){return wifi->state==PD_WIFI_CONNECTED&&!wifi->suspended&&!wifi->setup&&!wifi->joining;}
static void beyond_wait_clear(void){beyond_waiting=beyond_wifi_return=false;}
static void beyond_failed(unsigned error){
    beyond_wait_clear();beyond_failure=error;beyond_keyboard=false;beyond_focus=0;page=BEYOND_RECOVER;
}
static bool beyond_launch(void){
    bool started=beyond_retry_operation==PD_BEYOND_LIST?pd_beyond_list():beyond_retry_operation==PD_BEYOND_OWNED?pd_beyond_owned_id(beyond_retry_id):pd_beyond_start(beyond_retry_id);
    beyond_wait_clear();
    if(started){page=BEYOND_BUSY;beyond_focus=0;beyond_keyboard=false;memset(&beyond_visible,0,sizeof beyond_visible);}
    else beyond_failed(PD_BEYOND_BUSY);
    return started;
}
static bool beyond_request(unsigned operation,const char *id){
    if(operation!=PD_BEYOND_PUBLIC&&operation!=PD_BEYOND_LIST&&operation!=PD_BEYOND_OWNED)return false;
    pd_beyond_info current;pd_beyond_snapshot(&current);
    if(current.running)return false;
    if(operation!=PD_BEYOND_LIST&&!pd_beyond_id(id))return false;
    beyond_retry_operation=operation;beyond_retry_id[0]=0;if(id)strcpy(beyond_retry_id,id);
    beyond_source_account=operation!=PD_BEYOND_PUBLIC;
    if(operation!=PD_BEYOND_PUBLIC&&(!current.configured||current.reconnect||current.locked)){
        beyond_failed(current.locked?PD_BEYOND_STORAGE:PD_BEYOND_SESSION);return false;
    }
    if(operation!=PD_BEYOND_LIST&&!power_storage_ready()){
        beyond_failed(PD_BEYOND_BUSY);return false;
    }
    pd_wifi_info wifi;pd_wifi_snapshot(&wifi);
    if(beyond_online(&wifi))return beyond_launch();
    beyond_waiting=beyond_wifi_return=true;beyond_wait_started=lv_tick_get();
    beyond_focus=0;beyond_keyboard=false;wifi_keyboard=false;wifi_focus=0;
    /* One deliberate attempt on a saved network. No retry loop or scan without
       a user action, and never interrupt a join/hotspot/password editor. */
    if(wifi.configured&&wifi.error!=PD_WIFI_ERROR_STORAGE&&!wifi.suspended&&!wifi.setup&&!wifi.joining&&
       (wifi.state==PD_WIFI_CONNECTING||pd_wifi_command(PD_WIFI_ENABLE)))page=BEYOND_CONNECT;
    else{page=WIFI_HOME;wifi_notice="Dopo il Wi-Fi riprendo Beyond";}
    return true;
}
static bool beyond_wait_poll(void){
    if(!beyond_waiting||(page!=BEYOND_CONNECT&&page!=WIFI_HOME)||power_paused||power_off_attempted||display_screen.touch_down||import_pending)return false;
    pd_wifi_info wifi;pd_wifi_snapshot(&wifi);
    if(beyond_online(&wifi)&&!wifi.setup_failed&&wifi.error!=PD_WIFI_ERROR_STORAGE){beyond_launch();render();return true;}
    if(page==BEYOND_CONNECT&&(uint32_t)(lv_tick_get()-beyond_wait_started)>30000){beyond_failed(PD_BEYOND_NO_WIFI);render();return true;}
    return false;
}
static void beyond_pair_clear(void){beyond_pairing=beyond_approved=false;
    pd_account_wipe(beyond_nonce,sizeof beyond_nonce);
    pd_account_wipe(beyond_code,sizeof beyond_code);
}
static void beyond_pair_reply(const char *result){char message[96];
    snprintf(message,sizeof message,"PDD ACCOUNT %s 1 %s\n",result,beyond_nonce);
    reply(message);
}
static void beyond_pair_cancel(void){beyond_pair_reply("CANCELLED");
    beyond_pair_clear();
    page=BEYOND_ACCOUNT;
}
static void beyond_account_request(const pd_account_request *request){
    pd_beyond_info current;
    pd_beyond_snapshot(&current);
    if(request->kind==PD_ACCOUNT_CANCEL){if(beyond_pairing&&!strcmp(request->nonce,beyond_nonce)){if(page==BEYOND_BUSY&&!current.running&&current.operation==PD_BEYOND_LINK)return;
            if(current.running)pd_beyond_cancel();
            else beyond_pair_cancel();
        }return;
    }
    if(request->kind==PD_ACCOUNT_HELLO){
        if(library_failed||current.locked||current.running||beyond_pairing||import_pending||dice_animation.active||power_paused||power_off_attempted||(confirmation_open()&&page!=BEYOND_ACCOUNT&&page!=BEYOND_RECOVER)){reply("PDD ACCOUNT ERROR busy\n");
            return;
        }
        for(unsigned i=0;i<4;i++)snprintf(beyond_nonce+i*8,9,"%08lx",(unsigned long)pd_wifi_random());
        snprintf(beyond_code,sizeof beyond_code,"%06lu",(unsigned long)(pd_wifi_random()%1000000));
        beyond_pairing=true;
        beyond_approved=false;
        beyond_remember=true;
        beyond_pair_started=lv_tick_get();
        beyond_keyboard=false;
        beyond_focus=0;
        page=BEYOND_PAIR;
        char message[96];
        snprintf(message,sizeof message,"PDD ACCOUNT CODE 1 %s %s\n",beyond_nonce,beyond_code);
        reply(message);
        render();
        return;
    }
    if(request->kind==PD_ACCOUNT_OFFER){
        if(!beyond_pairing||!beyond_approved||strcmp(request->nonce,beyond_nonce)||(uint32_t)(lv_tick_get()-beyond_pair_started)>30000||!pd_beyond_link(&request->access,beyond_remember)){reply("PDD ACCOUNT ERROR offer\n");
            return;
        }
        page=BEYOND_BUSY;
        beyond_focus=0;
        memset(&beyond_visible,0,sizeof beyond_visible);
        render();
    }
}
static void beyond_action(intptr_t action){
    if(action==B_OPEN){if(!confirmation_open()&&!import_pending&&!dice_animation.active)beyond_open();
        return;
    }
    if(!beyond_local())return;
    pd_beyond_info current;
    pd_beyond_snapshot(&current);
    if(action==B_NOTES&&import_from_beyond&&import_pending){beyond_note=0;
        beyond_focus=0;
        page=BEYOND_REPORT;
    }
    else if(action==B_NOTES_BACK&&page==BEYOND_REPORT){page=import_pending?BEYOND_BUSY:beyond_return();
        beyond_focus=0;
    }
    else if(action==B_PREV&&page==BEYOND_REPORT&&beyond_note)beyond_note--;
    else if(action==B_NEXT&&page==BEYOND_REPORT&&beyond_note+1<beyond_report.count)beyond_note++;
    else if(action==B_WIFI&&(page==BEYOND_ACCOUNT||page==BEYOND_FORM||page==BEYOND_CONNECT||page==BEYOND_RECOVER)){
        if(page==BEYOND_ACCOUNT||page==BEYOND_FORM)beyond_source_account=page==BEYOND_ACCOUNT;
        if(page==BEYOND_RECOVER)beyond_waiting=true;
        beyond_wifi_return=true;
        wifi_notice=beyond_waiting?"Dopo il Wi-Fi riprendo Beyond":"";
        wifi_keyboard=false;wifi_focus=0;
        page=WIFI_HOME;
    }
    else if(page==BEYOND_CONNECT||page==BEYOND_RECOVER){
        if(action==B_BACK||action==B_CANCEL){beyond_wait_clear();page=beyond_return();}
        else if(action==B_RETRY&&page==BEYOND_RECOVER){
            char id[16];strcpy(id,beyond_retry_id);beyond_request(beyond_retry_operation,id[0]?id:NULL);
        }
    }
    else if(page==BEYOND_ACCOUNT){
        if(action==B_BACK)library_open();
        else if(action==B_PUBLIC){beyond_source_account=false;
            page=BEYOND_FORM;
            beyond_notice="Inserisci l ID di un PG pubblico";
        }
        else if(action==B_FORGET)page=BEYOND_FORGET;
        else if(action==B_PREV&&beyond_catalog_page)beyond_catalog_page--;
        else if(action==B_NEXT&&(beyond_catalog_page+1)*2<current.count)beyond_catalog_page++;
        else if(action==B_REFRESH){beyond_request(PD_BEYOND_LIST,NULL);
        }
        else if(action>=B_PICK&&action<B_PICK+2){if(!power_storage_ready()){beyond_notice="Prima risolvi memoria e storico";
                return;
            }pd_account_row row;
            if(pd_beyond_row(beyond_catalog_page*2+action-B_PICK,&row))beyond_request(PD_BEYOND_OWNED,row.id);
            else beyond_notice="Accesso non pronto: rileggi elenco";
        }
    }else if(page==BEYOND_PAIR){
        if(action==B_BACK)beyond_pair_cancel();
        else if(action==B_REMEMBER&&!beyond_approved)beyond_remember=!beyond_remember;
        else if(action==B_APPROVE&&!beyond_approved){beyond_approved=true;
            beyond_pair_started=lv_tick_get();
            beyond_pair_reply("ACCEPT");
        }
    }else if(page==BEYOND_FORGET){
        if(action==B_BACK)page=BEYOND_ACCOUNT;
        else if(action==B_UNLINK){beyond_notice=pd_beyond_forget()?"Accesso rimosso: PG conservati":"Rimozione non confermata: riavvia";
            page=BEYOND_ACCOUNT;
        }
    }else if(page==BEYOND_FORM){
        unsigned n=strlen(beyond_id);
        if(action>=B_DIGIT&&action<B_DIGIT+10){unsigned digit=action-B_DIGIT;
            if(n<15&&(n||digit)){beyond_id[n]='0'+digit;
                beyond_id[n+1]=0;
                beyond_notice="Inserisci l ID di un PG pubblico";
        }}
        else if(action==B_ERASE){if(n)beyond_id[n-1]=0;
        }
        else if(action==B_BACK||action==B_ACCOUNT){page=BEYOND_ACCOUNT;
            beyond_notice="";
        }
        else if(action==B_READ&&pd_beyond_id(beyond_id)){
            if(!power_storage_ready()){beyond_notice="Prima risolvi memoria e storico";
                return;
            }
            beyond_request(PD_BEYOND_PUBLIC,beyond_id);
        }
    }else if(action==B_CANCEL&&page==BEYOND_BUSY&&!import_pending)pd_beyond_cancel();
    pd_screen_activity(&display_screen,lv_tick_get());
}
static void beyond_poll(void){
    if(beyond_wait_poll())return;
    if(page==BEYOND_PAIR&&beyond_pairing&&(uint32_t)(lv_tick_get()-beyond_pair_started)>(beyond_approved?30000u:120000u)){beyond_pair_cancel();
        beyond_notice="Collegamento scaduto: riprova";
        render();
        return;
    }
    if(page!=BEYOND_BUSY||import_pending)return;
    pd_beyond_info current;
    pd_beyond_snapshot(&current);
    if(!current.running){
        pd_beyond_candidate *received=pd_beyond_take();
        if(current.operation==PD_BEYOND_LINK||current.operation==PD_BEYOND_LIST){
            if(current.phase==PD_BEYOND_FINISHED){beyond_notice="";
                beyond_catalog_page=0;
                if(current.operation==PD_BEYOND_LINK)beyond_pair_reply("LINKED");
            }
            else{beyond_notice=beyond_error(current.error);
                if(current.operation==PD_BEYOND_LINK)beyond_pair_reply(current.error==PD_BEYOND_CANCELLED?"CANCELLED":"FAILED");
            }
            if(current.operation==PD_BEYOND_LINK)beyond_pair_clear();
            page=BEYOND_ACCOUNT;
            if(current.phase!=PD_BEYOND_FINISHED&&current.error!=PD_BEYOND_CANCELLED){
                if(current.operation==PD_BEYOND_LINK){beyond_retry_operation=PD_BEYOND_LINK;beyond_retry_id[0]=0;}
                beyond_failed(current.error);
            }
        }else if(current.phase==PD_BEYOND_FINISHED&&received){
            beyond_report=received->report;
            beyond_note=beyond_focus=0;
            beyond_keyboard=false;
            if(current.error==PD_BEYOND_READY){if(!stage_import_from(&received->state,true)&&page==BEYOND_BUSY){beyond_notice="Memoria o SD non pronte: PG conservato";
                    page=beyond_return();
            }}else page=BEYOND_REPORT;
        }else{beyond_notice=beyond_error(current.error);
            if(current.error==PD_BEYOND_CANCELLED)page=beyond_return();
            else beyond_failed(current.error);
        }
        free(received);
        render();
        return;
    }
    if(current.phase!=beyond_visible.phase){beyond_visible=current;
        render();
    }
}
static void render_beyond(void){
    char text[96];
    beyond_target_count=0;
    if(page==BEYOND_ACCOUNT){
        pd_beyond_info current;
        pd_beyond_snapshot(&current);
        pd_wifi_info wifi;
        pd_wifi_snapshot(&wifi);
        bool online=wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;
        if(!current.configured){
            label("Collega Beyond",83,111,300,true);
            label("Apri l estensione Pocket",83,167,300,false);
            small_label("Dal PC: Collega Pocket",83,207,300,PD_GOLD);
            small_label("Conferma il codice sul display",83,247,300,PD_TEXT);
            small_label(beyond_notice,83,281,300,PD_GOLD);
            beyond_control("Wi-Fi",83,307,144,44,B_WIFI,true);
            beyond_control("PG pubblico",239,307,144,44,B_PUBLIC,true);
            beyond_control("Indietro",83,359,300,48,B_BACK,true);
        }else{
            snprintf(text,sizeof text,"I miei PG (%u)",current.count);
            label(text,83,111,204,true);
            beyond_control("Esci",299,99,84,44,B_FORGET,!current.locked);
            beyond_control(current.reconnect?"Ricollega dal PC":online?"Rileggi":"Collega Wi-Fi",83,151,144,44,B_REFRESH,!current.locked);
            beyond_control("ID pubblico",239,151,144,44,B_PUBLIC,true);
            for(unsigned i=0;i<2;i++){pd_account_row row;
                if(!pd_beyond_row(beyond_catalog_page*2+i,&row))break;
                char name[25];
                size_t n=strlen(row.name);
                if(n>24)n=24;
                while(n&&((unsigned char)row.name[n]&0xc0)==0x80)n--;
                memcpy(name,row.name,n);
                name[n]=0;
                const char *tail=row.id+strlen(row.id);
                if(strlen(row.id)>4)tail-=4;
                else tail=row.id;
                snprintf(text,sizeof text,"%s%s\nLiv %u | ...%s",name,n<strlen(row.name)?"...":"",row.level,tail);
                beyond_control(text,83,199+i*64,300,54,B_PICK+i,!current.locked&&!current.reconnect);
            }
            if(!current.count)small_label(current.listed?"Nessun PG in questo account":"Premi Rileggi per caricare i PG",83,231,300,PD_TEXT);
            snprintf(text,sizeof text,"%s | %u/%u",current.locked?"Accesso incerto: riavvia":current.reconnect?"Accesso da ricollegare":beyond_notice[0]?beyond_notice:current.partial?"Elenco parziale":current.remembered?"Accesso ricordato":"Accesso fino allo spegnimento",beyond_catalog_page+1,current.count?(current.count+1)/2:1);
            small_label(text,83,335,300,PD_GOLD);
            beyond_control("Indietro",83,359,92,48,B_BACK,true);
            beyond_control("<",187,359,92,48,B_PREV,beyond_catalog_page>0);
            beyond_control(">",291,359,92,48,B_NEXT,(beyond_catalog_page+1)*2<current.count);
        }
    }else if(page==BEYOND_PAIR){
        pd_wifi_info wifi;
        pd_wifi_snapshot(&wifi);
        bool online=wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;
        label("Confronta il codice",83,111,300,true);
        label(beyond_code,83,151,300,true);
        lv_obj_t *note=small_label(beyond_approved?"Attendo l accesso dal browser":beyond_remember?"Accesso nella memoria non cifrata. Chi legge la flash puo recuperarlo.":"Accesso solo fino allo spegnimento. Il vecchio accesso salvato sara rimosso.",83,195,300,PD_TEXT);
        lv_label_set_long_mode(note,LV_LABEL_LONG_WRAP);
        lv_obj_set_height(note,92);
        beyond_control(beyond_remember?"Ricorda accesso: SI":"Ricorda accesso: NO",83,291,300,44,B_REMEMBER,!beyond_approved);
        small_label(online?"PG e backup restano separati":"Collega prima il Wi-Fi e riprova",83,335,300,PD_GOLD);
        beyond_control("Annulla",83,359,144,48,B_BACK,true);
        beyond_control("Collega",239,359,144,48,B_APPROVE,online&&!beyond_approved);
    }else if(page==BEYOND_FORGET){
        label("Scollegare Beyond?",83,111,300,true);
        label("Rimuove l accesso dal Pocket",83,171,300,false);
        small_label("Schede e progressi conservati",83,231,300,PD_MINT);
        small_label("Il browser resta collegato",83,271,300,PD_TEXT);
        beyond_control("Indietro",83,359,144,48,B_BACK,true);
        beyond_control("Scollega",239,359,144,48,B_UNLINK,true);
    }else if(page==BEYOND_FORM){
        pd_wifi_info wifi;
        pd_wifi_snapshot(&wifi);
        bool online=wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;
        label("PG pubblico",83,111,300,true);
        label(beyond_id[0]?beyond_id:"ID del PG su Beyond",83,151,300,false);
        small_label(online||library_failed?beyond_notice:"Collega prima il Wi-Fi",83,181,300,PD_GOLD);
        for(unsigned i=0;i<10;i++){char digit[2]={(char)('0'+i),0};
            beyond_control(digit,83+(i%5)*62,205+(i/5)*52,52,44,B_DIGIT+i,strlen(beyond_id)<15&&(i!=0||beyond_id[0]));
        }
        beyond_control("Cancella",83,307,144,44,B_ERASE,beyond_id[0]!=0);
        beyond_control("Wi-Fi",239,307,144,44,B_WIFI,true);
        beyond_control("Indietro",83,359,144,48,B_BACK,true);
        beyond_control(online?"Leggi":"Collega e leggi",239,359,144,48,B_READ,pd_beyond_id(beyond_id)&&power_storage_ready());
    }else if(page==BEYOND_CONNECT){
        label("Collego il Wi-Fi",83,111,300,true);
        label("Uso la rete salvata",83,171,300,false);
        small_label("Poi riprendo la richiesta Beyond",83,227,300,PD_GOLD);
        small_label("PG e progressi restano conservati",83,273,300,PD_MINT);
        small_label("Preferisci un altra rete?",83,315,300,PD_TEXT);
        beyond_control("Annulla",83,359,144,48,B_BACK,true);
        beyond_control("Scegli rete",239,359,144,48,B_WIFI,true);
    }else if(page==BEYOND_RECOVER){
        pd_beyond_info current;pd_beyond_snapshot(&current);
        bool relink=beyond_failure==PD_BEYOND_SESSION||beyond_failure==PD_BEYOND_IDENTITY||(beyond_retry_operation!=PD_BEYOND_PUBLIC&&current.reconnect);
        bool restart=beyond_failure==PD_BEYOND_STORAGE||(beyond_retry_operation!=PD_BEYOND_PUBLIC&&current.locked);
        bool link=beyond_retry_operation==PD_BEYOND_LINK;
        bool retry=!link&&!relink&&!restart&&beyond_failure!=PD_BEYOND_DENIED&&beyond_failure!=PD_BEYOND_NOT_FOUND;
        label(relink?"Ricollega Beyond":restart?"Riavvia il Pocket":"Richiesta interrotta",83,111,300,true);
        lv_obj_t *reason=small_label(beyond_error(beyond_failure),83,163,300,PD_GOLD);
        lv_label_set_long_mode(reason,LV_LABEL_LONG_WRAP);lv_obj_set_height(reason,48);
        const char *hint=restart?"Il salvataggio non e confermato":relink||link?"Dal PC: Apri Pocket PC > Collega Beyond":beyond_failure==PD_BEYOND_RATE?"Attendi prima di riprovare":beyond_failure==PD_BEYOND_DENIED?"Verifica privacy e accesso su Beyond":beyond_failure==PD_BEYOND_NOT_FOUND?"Controlla il PG o rileggi l elenco":!power_storage_ready()?"Prima risolvi memoria e storico":"Riprova la stessa richiesta";
        lv_obj_t *next=small_label(hint,83,227,300,PD_TEXT);lv_label_set_long_mode(next,LV_LABEL_LONG_WRAP);lv_obj_set_height(next,48);
        small_label("PG e progressi restano conservati",83,293,300,PD_MINT);
        if(beyond_retry_id[0]){snprintf(text,sizeof text,"PG ...%s",beyond_retry_id+((strlen(beyond_retry_id)>4)?strlen(beyond_retry_id)-4:0));small_label(text,83,325,300,PD_MUTED);}
        beyond_control("Indietro",83,359,retry?92:300,48,B_BACK,true);
        if(retry){beyond_control("Wi-Fi",187,359,92,48,B_WIFI,true);beyond_control("Riprova",291,359,92,48,B_RETRY,power_storage_ready()||beyond_retry_operation==PD_BEYOND_LIST);}
    }else if(page==BEYOND_BUSY){
        pd_beyond_info current;
        pd_beyond_snapshot(&current);
        label(current.operation==PD_BEYOND_LINK?"Collego Beyond":current.operation==PD_BEYOND_LIST?"Leggo i tuoi PG":"Leggo Beyond",83,111,300,true);
        label(current.phase==PD_BEYOND_CANCELLING?"Annullamento...":current.phase==PD_BEYOND_CLOCK?"Controllo l orologio":current.phase==PD_BEYOND_HTTP?(current.operation==PD_BEYOND_LIST||current.operation==PD_BEYOND_LINK?"Verifico accesso ed elenco":"Scarico la scheda"):"Converto il personaggio",83,171,300,false);
        small_label("Il PG attuale resta conservato",83,231,300,PD_MINT);
        small_label("Nessuna modifica su Beyond",83,271,300,PD_TEXT);
        small_label("Attendi oppure annulla",83,311,300,PD_MUTED);
        beyond_control("Annulla",83,359,300,48,B_CANCEL,current.phase!=PD_BEYOND_CANCELLING);
    }else{
        label(beyond_report.blockers?"Import bloccato":"Limiti e dati manuali",83,111,300,false);
        unsigned total=beyond_report.blockers+beyond_report.limits;
        snprintf(text,sizeof text,"Voce %u/%u%s",beyond_report.count?beyond_note+1:0,beyond_report.count,total>beyond_report.count?" | altre note in estratto":"");
        small_label(text,83,151,300,PD_GOLD);
        lv_obj_t *body=small_label(beyond_report.count?beyond_report.lines[beyond_note]:"Nessuna nota aggiuntiva",83,177,300,PD_TEXT);
        lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);
        lv_obj_set_height(body,180);
        beyond_control("Indietro",83,359,92,48,B_NOTES_BACK,true);
        beyond_control("<",187,359,92,48,B_PREV,beyond_note>0);
        beyond_control(">",291,359,92,48,B_NEXT,beyond_note+1<beyond_report.count);
    }
    beyond_mark_focus();
}
