/* Beyond account pairing and native import. No credentials in PGs or UI. */
enum {B_OPEN=5440,B_BACK,B_ERASE,B_READ,B_CANCEL,B_WIFI,B_NOTES,B_NOTES_BACK,B_PREV,B_NEXT,B_DIGIT=5450,B_ACCOUNT=5460,B_PUBLIC,B_REFRESH,B_FORGET,B_UNLINK,B_REMEMBER,B_APPROVE,B_RETRY,B_GUIDE,B_GUIDE_BACK,B_PICK=5480,B_END=5500};
static char beyond_id[16];
static const char *beyond_notice="Numero alla fine del link Beyond";
PD_EXT static pd_beyond_report beyond_report;
static pd_beyond_info beyond_visible;
static unsigned beyond_note,beyond_focus,beyond_target_count;
static bool beyond_keyboard,beyond_wifi_return,beyond_source_account;
static bool beyond_pairing,beyond_approved,beyond_remember=true;
static char beyond_nonce[33],beyond_code[7];
static uint32_t beyond_pair_started;
static unsigned beyond_catalog_page,beyond_report_http;
static bool beyond_guide;
static page_t beyond_guide_owner=HERO;
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
    if(!beyond_target_count)return;
    if(beyond_focus>=beyond_target_count)beyond_focus=0;
    for(unsigned i=0;i<beyond_target_count&&lv_obj_has_state(beyond_targets[beyond_focus],LV_STATE_DISABLED);i++)beyond_focus=(beyond_focus+1)%beyond_target_count;
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
    beyond_wait_clear();beyond_report_http=0;
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
    if(beyond_guide&&(page==BEYOND_ACCOUNT||page==BEYOND_RECOVER)){
        if(action==B_GUIDE_BACK||action==B_BACK){beyond_guide=false;return;}
        if(action!=B_WIFI||page!=BEYOND_ACCOUNT)return;
    }
    if(action==B_GUIDE&&!current.locked&&(page==BEYOND_ACCOUNT||page==BEYOND_RECOVER)){beyond_guide=true;beyond_focus=0;return;}
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
            beyond_notice="Numero alla fine del link Beyond";
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
                beyond_notice="Numero alla fine del link Beyond";
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
            beyond_report=received->report;beyond_report_http=current.http;
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
