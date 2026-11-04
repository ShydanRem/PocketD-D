/* Presentation only: reuse staged requests, validated candidates and typed reports. */
static lv_obj_t *beyond_card(const char *name,pd_ui_icon_id icon,const char *caption,const char *badge,int x,int y,int w,int h,intptr_t action,bool enabled){
    lv_obj_t *b=scene_control(name,pd_ui_icon(icon),caption,badge,x,y,w,h,action,enabled&&!library_failed);if(beyond_target_count<16)beyond_targets[beyond_target_count++]=b;return b;
}
static void beyond_step(pd_ui_icon_id icon,const char *title,const char *detail,int y){
    lv_obj_t *box=decoration(83,y,300,56,0x122b38,2);lv_obj_set_style_pad_all(box,0,0);
    lv_obj_t *im=art(pd_ui_icon(icon),9,12,box);lv_image_set_pivot(im,0,0);lv_image_set_scale(im,32*256/pd_ui_icon(icon)->header.w);lv_image_set_antialias(im,false);
    tile_label(box,title,48,5,244,&lv_font_montserrat_16,PD_TEXT);tile_label(box,detail,48,31,244,&lv_font_montserrat_14,PD_MUTED);
}
static void render_beyond(void){
    char text[160];beyond_target_count=0;
    pd_beyond_info current;pd_beyond_snapshot(&current);pd_wifi_info wifi;pd_wifi_snapshot(&wifi);bool online=beyond_online(&wifi);
    if(beyond_guide&&(page==BEYOND_ACCOUNT||page==BEYOND_RECOVER)){
        label("Collega dal PC",83,111,300,true);
        beyond_step(PD_UI_PC,"1. Apri Pocket PC","Estensione in Chrome o Edge",165);
        beyond_step(PD_UI_ACCOUNT,"2. Collega Beyond","Accedi, poi Collega Pocket",229);
        beyond_step(PD_UI_LOCK,"3. Conferma il codice","Stesso codice sui due schermi",293);
        beyond_control("Indietro",83,359,page==BEYOND_ACCOUNT?144:300,48,B_GUIDE_BACK,true);
        if(page==BEYOND_ACCOUNT)beyond_control("Wi-Fi",239,359,144,48,B_WIFI,true);
    }else if(page==BEYOND_ACCOUNT){
        if(!current.configured){
            label("Importa un PG",83,111,300,true);
            beyond_card("Collega Beyond",PD_UI_ACCOUNT,"I miei PG","Primo collegamento dal PC",83,165,300,64,B_GUIDE,!current.locked);
            beyond_card("PG pubblico",PD_UI_SHEET,"PG pubblico","ID, senza collegare l'account",83,237,300,64,B_PUBLIC,true);
            connection_note(current.locked?"Accesso incerto: riavvia Pocket":beyond_notice[0]?beyond_notice:online?"Wi-Fi collegato":"Wi-Fi da collegare",315,40,current.locked||beyond_notice[0]||!online?PD_GOLD:PD_MUTED);
            beyond_control("Indietro",83,359,144,48,B_BACK,true);beyond_control("Wi-Fi",239,359,144,48,B_WIFI,true);
        }else{
            unsigned pages=current.count?(current.count+1)/2:1;if(beyond_catalog_page>=pages)beyond_catalog_page=pages-1;
            snprintf(text,sizeof text,"I miei PG (%u)",current.count);label(text,83,111,204,true);beyond_control("Esci",299,99,84,44,B_FORGET,!current.locked);
            beyond_card(current.reconnect?"Ricollega dal PC":online?"Rileggi":"Collega Wi-Fi",current.reconnect?PD_UI_PC:PD_UI_UNDO,current.reconnect?"Ricollega":"Rileggi",current.reconnect?"Dal PC":online?"Elenco":"Con Wi-Fi",83,151,144,44,current.reconnect?B_GUIDE:B_REFRESH,!current.locked);
            beyond_card("ID pubblico",PD_UI_SHEET,"Pubblico","ID del PG",239,151,144,44,B_PUBLIC,true);
            for(unsigned i=0;i<2;i++){
                pd_account_row row;if(!pd_beyond_row(beyond_catalog_page*2+i,&row))break;
                char name[25];size_t n=strlen(row.name);if(n>24)n=24;while(n&&((unsigned char)row.name[n]&0xc0)==0x80)n--;memcpy(name,row.name,n);name[n]=0;
                const char *tail=row.id+strlen(row.id);if(strlen(row.id)>4)tail-=4;else tail=row.id;
                snprintf(text,sizeof text,"%s%s\nLiv %u | ...%s",name,n<strlen(row.name)?"...":"",row.level,tail);char detail[64];snprintf(detail,sizeof detail,"Lv %u | ...%s",row.level,tail);
                beyond_card(text,PD_UI_ACCOUNT,row.name,detail,83,203+i*64,300,54,B_PICK+i,!current.locked&&!current.reconnect);
            }
            if(!current.count)wrapped_name(current.listed?"Nessun PG nell'elenco":"Rileggi per caricare i PG",231,60,&lv_font_montserrat_16);
            snprintf(text,sizeof text,"%s | %u/%u",current.locked?"Accesso incerto: riavvia":current.reconnect?"Ricollega dal PC":beyond_notice[0]?beyond_notice:current.partial?"Elenco parziale":current.remembered?"Accesso ricordato":"Fino allo spegnimento",beyond_catalog_page+1,pages);
            connection_note(text,335,20,PD_GOLD);
            beyond_control("Indietro",83,359,92,48,B_BACK,true);beyond_control("<",187,359,92,48,B_PREV,beyond_catalog_page>0);beyond_control(">",291,359,92,48,B_NEXT,(beyond_catalog_page+1)*2<current.count);
        }
    }else if(page==BEYOND_PAIR){
        label("Confronta il codice",83,111,300,true);lv_obj_t *code=label(beyond_code,83,157,300,true);label_font(code,&pd_pixel_6);lv_obj_set_style_text_color(code,lv_color_hex(PD_GOLD),0);
        connection_note(beyond_approved?"Attendo l'accesso dal browser":"Lo stesso codice deve apparire sul PC",209,20,PD_MUTED);
        beyond_card(beyond_remember?"Ricorda accesso: SI":"Ricorda accesso: NO",PD_UI_LOCK,"Ricorda accesso",beyond_remember?"Si, resta dopo il riavvio":"No, fino allo spegnimento",83,233,300,60,B_REMEMBER,!beyond_approved);
        connection_note(!online?"Collega prima il Wi-Fi e riprova":beyond_remember?"Memoria non cifrata: chi legge la flash puo recuperare l'accesso.":"Solo fino allo spegnimento. Il vecchio accesso salvato sara rimosso.",301,52,!online?PD_GOLD:PD_TEXT);
        beyond_control("Annulla",83,359,144,48,B_BACK,true);beyond_control("Collega",239,359,144,48,B_APPROVE,online&&!beyond_approved);
    }else if(page==BEYOND_FORGET){
        label("Scollegare Beyond?",83,111,300,true);
        status_tile(PD_UI_LOCK,"Accesso","Da rimuovere",83,169);status_tile(PD_UI_HOME,"Personaggi","Conservati",239,169);
        status_tile(PD_UI_PC,"Browser","Invariato",83,237);status_tile(PD_UI_WIFI,"Wi-Fi","Conservato",239,237);
        connection_note("Rimuove l'accesso dal Pocket. PG e progressi conservati.",315,40,PD_GOLD);
        beyond_control("Indietro",83,359,144,48,B_BACK,true);beyond_control("Scollega",239,359,144,48,B_UNLINK,true);
    }else if(page==BEYOND_FORM){
        label("PG pubblico",83,111,300,true);label(beyond_id[0]?beyond_id:"ID del PG su Beyond",83,151,300,false);
        connection_note(online||library_failed?beyond_notice:"Collega prima il Wi-Fi",181,20,PD_GOLD);
        for(unsigned i=0;i<10;i++){char digit[2]={(char)('0'+i),0};beyond_control(digit,83+(i%5)*62,205+(i/5)*52,52,44,B_DIGIT+i,strlen(beyond_id)<15&&(i!=0||beyond_id[0]));}
        beyond_control("Cancella",83,307,144,44,B_ERASE,beyond_id[0]!=0);beyond_control("Wi-Fi",239,307,144,44,B_WIFI,true);
        beyond_control("Indietro",83,359,144,48,B_BACK,true);beyond_control(online?"Leggi":"Collega e leggi",239,359,144,48,B_READ,pd_beyond_id(beyond_id)&&power_storage_ready());
    }else if(page==BEYOND_CONNECT){
        label("Collego il Wi-Fi",83,111,300,true);connection_banner(PD_UI_WIFI,"Uso la rete salvata","Poi riprendo la richiesta Beyond",PD_GOLD);
        status_tile(PD_UI_HOME,"Personaggio","Conservato",83,233);status_tile(PD_UI_DICE,"Progressi","Conservati",239,233);
        connection_note("Puoi scegliere una rete diversa.",315,40,PD_MUTED);
        beyond_control("Annulla",83,359,144,48,B_BACK,true);beyond_control("Scegli rete",239,359,144,48,B_WIFI,true);
    }else if(page==BEYOND_RECOVER){
        bool relink=beyond_failure==PD_BEYOND_SESSION||beyond_failure==PD_BEYOND_IDENTITY||(beyond_retry_operation!=PD_BEYOND_PUBLIC&&current.reconnect);
        bool restart=beyond_failure==PD_BEYOND_STORAGE||(beyond_retry_operation!=PD_BEYOND_PUBLIC&&current.locked);bool link=beyond_retry_operation==PD_BEYOND_LINK;
        bool retry=!link&&!relink&&!restart&&beyond_failure!=PD_BEYOND_DENIED&&beyond_failure!=PD_BEYOND_NOT_FOUND;
        label("Beyond",83,111,300,true);connection_banner(relink?PD_UI_LOCK:PD_UI_LINK,relink?"Ricollega l'account":restart?"Riavvio richiesto":"Lettura interrotta",beyond_error(beyond_failure),PD_GOLD);
        status_tile(PD_UI_HOME,"Personaggio","Conservato",83,233);status_tile(PD_UI_DICE,"Progressi","Conservati",239,233);
        const char *hint=restart?"Il salvataggio non e confermato":relink||link?"Collega di nuovo dal browser PC":beyond_failure==PD_BEYOND_RATE?"Attendi prima di riprovare":beyond_failure==PD_BEYOND_DENIED?"Verifica privacy e accesso su Beyond":beyond_failure==PD_BEYOND_NOT_FOUND?"Controlla il PG o rileggi l'elenco":!power_storage_ready()?"Prima risolvi memoria e storico":"Riprova la stessa richiesta";
        connection_note(hint,309,44,PD_TEXT);bool guide=!restart&&(relink||link);
        beyond_control("Indietro",83,359,retry?92:guide?144:300,48,B_BACK,true);
        if(retry){beyond_control("Wi-Fi",187,359,92,48,B_WIFI,true);beyond_control("Riprova",291,359,92,48,B_RETRY,power_storage_ready()||beyond_retry_operation==PD_BEYOND_LIST);}
        else if(guide)beyond_control("Guida PC",239,359,144,48,B_GUIDE,true);
    }else if(page==BEYOND_BUSY){
        label(current.operation==PD_BEYOND_LINK?"Collego Beyond":current.operation==PD_BEYOND_LIST?"Leggo i tuoi PG":"Leggo Beyond",83,111,300,true);
        const lv_image_dsc_t *glyph=pd_ui_icon(current.phase==PD_BEYOND_CANCELLING?PD_UI_UNDO:PD_UI_LINK);lv_obj_t *im=art(glyph,185,155,lv_screen_active());lv_image_set_pivot(im,0,0);lv_image_set_scale(im,96*256/glyph->header.w);lv_image_set_antialias(im,false);
        const char *phase=current.phase==PD_BEYOND_CANCELLING?"Annullamento in corso":current.phase==PD_BEYOND_CLOCK?"Preparo la connessione":current.phase==PD_BEYOND_HTTP?(current.operation==PD_BEYOND_LIST||current.operation==PD_BEYOND_LINK?"Verifico accesso ed elenco":"Leggo la scheda"):current.phase==PD_BEYOND_CONVERTING?"Controllo e converto i dati":current.phase==PD_BEYOND_FINISHED?"Lettura terminata":current.phase==PD_BEYOND_FAILED?"Richiesta non riuscita":"Avvio la richiesta";
        wrapped_name(phase,263,40,&lv_font_montserrat_16);connection_note("PG attuale conservato fino alla conferma",315,40,PD_MUTED);
        beyond_control("Annulla",83,359,300,48,B_CANCEL,current.phase!=PD_BEYOND_CANCELLING);
    }else{
        unsigned count=beyond_report.count>PD_BEYOND_REPORT_LINES?PD_BEYOND_REPORT_LINES:beyond_report.count;if(beyond_note>=count)beyond_note=count?count-1:0;
        label(beyond_report.blockers?"Import bloccato":"Limiti da verificare",83,111,300,false);
        snprintf(text,sizeof text,"Blocchi: %u | limiti: %u",beyond_report.blockers,beyond_report.limits);connection_note(text,145,20,beyond_report.blockers?PD_DANGER:PD_GOLD);
        const char *kind=beyond_note<beyond_report.blockers?"Blocco":"Nota";
        if(beyond_report_http)snprintf(text,sizeof text,"HTTP %u | %s %u/%u%s",beyond_report_http,kind,count?beyond_note+1:0,count,beyond_report.blockers+beyond_report.limits>count?" + altre":"");
        else snprintf(text,sizeof text,"%s %u/%u%s",kind,count?beyond_note+1:0,count,beyond_report.blockers+beyond_report.limits>count?" + altre":"");
        connection_note(text,169,20,PD_MUTED);
        const char *note=count?beyond_report.lines[beyond_note]:"Nessuna nota aggiuntiva";
        lv_point_t extent;lv_text_get_size(&extent,note,&lv_font_montserrat_16,0,0,300,LV_TEXT_FLAG_NONE);
        lv_obj_t *body=small_label(note,83,193,300,PD_TEXT);if(extent.y>160)lv_obj_set_style_text_font(body,&lv_font_montserrat_14,0);lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);lv_obj_set_height(body,160);
        beyond_control("Indietro",83,359,92,48,B_NOTES_BACK,true);beyond_control("<",187,359,92,48,B_PREV,beyond_note>0);beyond_control(">",291,359,92,48,B_NEXT,beyond_note+1<count);
    }
    beyond_mark_focus();
}
