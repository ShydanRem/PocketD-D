/* Public native import; account sessions are not transferred or stored here. */
enum {B_OPEN=5440,B_BACK,B_ERASE,B_READ,B_CANCEL,B_WIFI,B_NOTES,B_NOTES_BACK,B_PREV,B_NEXT,B_DIGIT=5450};
static char beyond_id[16];
static const char *beyond_notice="Inserisci l ID di un PG pubblico";
PD_EXT static pd_beyond_report beyond_report;
static pd_beyond_info beyond_visible;
static unsigned beyond_note,beyond_focus,beyond_target_count;
static bool beyond_keyboard,beyond_wifi_return;
static lv_obj_t *beyond_targets[14];
static bool beyond_local(void){return page==BEYOND_FORM||page==BEYOND_BUSY||page==BEYOND_REPORT||(import_pending&&import_from_beyond);}
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
        case PD_BEYOND_ENGINE:return "Conversione non riuscita: PG conservato";
        default:return "Rete o servizio non raggiungibile";
    }
}
static void beyond_open(void){
    page=BEYOND_FORM;beyond_notice="Inserisci l ID di un PG pubblico";beyond_focus=0;beyond_keyboard=false;beyond_wifi_return=false;
}
static void beyond_mark_focus(void){
    if(beyond_focus>=beyond_target_count)beyond_focus=0;
    for(unsigned i=0;i<beyond_target_count;i++){lv_obj_set_style_outline_color(beyond_targets[i],lv_color_hex(PD_GOLD),0);lv_obj_set_style_outline_pad(beyond_targets[i],2,0);lv_obj_set_style_outline_width(beyond_targets[i],beyond_keyboard&&i==beyond_focus?2:0,0);}
}
static lv_obj_t *beyond_control(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled){
    lv_obj_t *b=button(text,x,y,w,h,action,enabled&&!library_failed);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
    if(beyond_target_count<14)beyond_targets[beyond_target_count++]=b;
    return b;
}
static void beyond_action(intptr_t action){
    if(action==B_OPEN){if(!confirmation_open()&&!import_pending&&!dice_animation.active)beyond_open();return;}
    if(!beyond_local())return;
    if(action==B_NOTES&&import_from_beyond&&import_pending){beyond_note=0;beyond_focus=0;page=BEYOND_REPORT;}
    else if(action==B_NOTES_BACK&&page==BEYOND_REPORT){page=import_pending?BEYOND_BUSY:BEYOND_FORM;beyond_focus=0;}
    else if(action==B_PREV&&page==BEYOND_REPORT&&beyond_note)beyond_note--;
    else if(action==B_NEXT&&page==BEYOND_REPORT&&beyond_note+1<beyond_report.count)beyond_note++;
    else if(page==BEYOND_FORM){
        unsigned n=strlen(beyond_id);
        if(action>=B_DIGIT&&action<B_DIGIT+10){unsigned digit=action-B_DIGIT;if(n<15&&(n||digit)){beyond_id[n]='0'+digit;beyond_id[n+1]=0;beyond_notice="Inserisci l ID di un PG pubblico";}}
        else if(action==B_ERASE){if(n)beyond_id[n-1]=0;}
        else if(action==B_BACK){library_open();}
        else if(action==B_WIFI){beyond_wifi_return=true;wifi_notice="";page=WIFI_HOME;}
        else if(action==B_READ&&pd_beyond_id(beyond_id)){
            if(!power_storage_ready()){beyond_notice="Prima risolvi memoria e storico";return;}
            if(pd_beyond_start(beyond_id)){page=BEYOND_BUSY;beyond_focus=0;beyond_keyboard=false;memset(&beyond_visible,0,sizeof beyond_visible);}
            else beyond_notice="Richiesta occupata: riprova";
        }
    }else if(action==B_CANCEL&&page==BEYOND_BUSY&&!import_pending)pd_beyond_cancel();
    pd_screen_activity(&display_screen,lv_tick_get());
}
static void beyond_poll(void){
    if(page!=BEYOND_BUSY||import_pending)return;
    pd_beyond_info current;pd_beyond_snapshot(&current);
    if(!current.running){
        pd_beyond_candidate *received=pd_beyond_take();
        if(current.phase==PD_BEYOND_FINISHED&&received){
            beyond_report=received->report;beyond_note=beyond_focus=0;beyond_keyboard=false;
            if(current.error==PD_BEYOND_READY){if(!stage_import_from(&received->state,true)&&page==BEYOND_BUSY){beyond_notice="Memoria o SD non pronte: PG conservato";page=BEYOND_FORM;}}
            else page=BEYOND_REPORT;
        }else{beyond_notice=beyond_error(current.error);page=BEYOND_FORM;}
        free(received);render();return;
    }
    if(current.phase!=beyond_visible.phase){beyond_visible=current;render();}
}
static void render_beyond(void){
    char text[96];beyond_target_count=0;
    if(page==BEYOND_FORM){
        pd_wifi_info wifi;pd_wifi_snapshot(&wifi);bool online=wifi.state==PD_WIFI_CONNECTED&&!wifi.suspended&&!wifi.setup&&!wifi.joining;
        label("PG pubblico",83,111,300,true);label(beyond_id[0]?beyond_id:"ID del PG su Beyond",83,151,300,false);
        small_label(online||library_failed?beyond_notice:"Collega prima il Wi-Fi",83,181,300,PD_GOLD);
        for(unsigned i=0;i<10;i++){char digit[2]={(char)('0'+i),0};beyond_control(digit,83+(i%5)*62,205+(i/5)*52,52,44,B_DIGIT+i,strlen(beyond_id)<15&&(i!=0||beyond_id[0]));}
        beyond_control("Cancella",83,307,144,44,B_ERASE,beyond_id[0]!=0);beyond_control("Wi-Fi",239,307,144,44,B_WIFI,true);
        beyond_control("Indietro",83,359,144,48,B_BACK,true);beyond_control("Leggi",239,359,144,48,B_READ,online&&pd_beyond_id(beyond_id)&&power_storage_ready());
    }else if(page==BEYOND_BUSY){
        pd_beyond_info current;pd_beyond_snapshot(&current);label("Leggo Beyond",83,111,300,true);
        label(current.phase==PD_BEYOND_CANCELLING?"Annullamento...":current.phase==PD_BEYOND_CLOCK?"Controllo l orologio":current.phase==PD_BEYOND_HTTP?"Scarico la scheda":"Converto il personaggio",83,171,300,false);
        small_label("Il PG attuale resta conservato",83,231,300,PD_MINT);small_label("Nessuna modifica su Beyond",83,271,300,PD_TEXT);small_label("Attendi oppure annulla",83,311,300,PD_MUTED);
        beyond_control("Annulla",83,359,300,48,B_CANCEL,current.phase!=PD_BEYOND_CANCELLING);
    }else{
        label(beyond_report.blockers?"Import bloccato":"Limiti e dati manuali",83,111,300,false);
        unsigned total=beyond_report.blockers+beyond_report.limits;
        snprintf(text,sizeof text,"Voce %u/%u%s",beyond_report.count?beyond_note+1:0,beyond_report.count,total>beyond_report.count?" | altre note in estratto":"");small_label(text,83,151,300,PD_GOLD);
        lv_obj_t *body=small_label(beyond_report.count?beyond_report.lines[beyond_note]:"Nessuna nota aggiuntiva",83,177,300,PD_TEXT);lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);lv_obj_set_height(body,180);
        beyond_control("Indietro",83,359,92,48,B_NOTES_BACK,true);beyond_control("<",187,359,92,48,B_PREV,beyond_note>0);beyond_control(">",291,359,92,48,B_NEXT,beyond_note+1<beyond_report.count);
    }
    beyond_mark_focus();
}
