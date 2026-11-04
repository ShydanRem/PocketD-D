/* Native connection/backup views. Snapshots only; commands keep existing gates. */
static void connection_banner(pd_ui_icon_id icon,const char *title,const char *detail,unsigned color){
    lv_obj_t *box=decoration(83,153,300,72,0x122b38,2);lv_obj_set_style_pad_all(box,0,0);
    lv_obj_t *im=art(pd_ui_icon(icon),9,20,box);lv_image_set_pivot(im,0,0);lv_image_set_scale(im,32*256/pd_ui_icon(icon)->header.w);lv_image_set_antialias(im,false);
    tile_label(box,title,48,8,244,&lv_font_montserrat_16,color);
    lv_obj_t *note=tile_label(box,detail,48,32,244,&lv_font_montserrat_14,PD_TEXT);lv_label_set_long_mode(note,LV_LABEL_LONG_WRAP);lv_obj_set_height(note,34);
}
static void connection_note(const char *text,int y,int height,unsigned color){
    lv_obj_t *note=small_label(text,83,y,300,color);lv_obj_set_style_text_font(note,&lv_font_montserrat_14,0);lv_label_set_long_mode(note,height<32?LV_LABEL_LONG_DOT:LV_LABEL_LONG_WRAP);lv_obj_set_height(note,height);
}
static void render_backup(void){
    char text[180];
    if(page==BACKUP_HOME){
        label("Backup USB",83,111,300,true);
        connection_banner(PD_UI_USB,"Copia e ripristino",backup_notice,backup_problem?PD_DANGER:PD_GOLD);
        status_tile(PD_UI_SHEET,"Scheda","Copia completa",83,233);
        snprintf(text,sizeof text,"%u tiri",history.count);status_tile(PD_UI_DICE,"Registro",text,239,233);
        connection_note("Dal PC: Backup > Crea copia o Ripristina. Confermi qui sul Pocket.",303,48,PD_TEXT);
        button("Indietro",83,359,300,48,1600,true);return;
    }
    bool restoring=page==BACKUP_RESTORE_CONFIRM;
    label(connection_details?"Contenuto copia":restoring?"Ripristina copia?":"Invia copia al PC?",83,111,300,true);
    wrapped_name(pending.name,145,48,&lv_font_montserrat_14);
    if(connection_details){
        if(pending.inventory_available)snprintf(text,sizeof text,"Zaino: %u oggetti",pending.item_count);else snprintf(text,sizeof text,"Zaino: non importato");small_label(text,83,201,300,PD_TEXT);
        if(pending.magic.available)snprintf(text,sizeof text,"Magie e usi: %u",pending.magic.count);else snprintf(text,sizeof text,"Magie e usi: non importati");small_label(text,83,225,300,PD_TEXT);
        snprintf(text,sizeof text,"%u tiri | monete %s",backup_history.count,pending.wallet.available?"disponibili":"non importate");small_label(text,83,249,300,PD_TEXT);
        connection_note("Aspetto, tasti, schermo | durata 2 min",279,20,PD_MUTED);
        connection_note(backup_problem?backup_notice:restoring?"Sostituisce scheda attiva e registro. Altri PG su SD, Wi-Fi e associazioni conservati.":"Copia della scheda e del registro. Wi-Fi e credenziali esclusi. Conserva il file sul PC.",307,48,backup_problem?PD_DANGER:restoring?PD_GOLD:PD_MUTED);
    }else{
        if(restoring){
            delta_tile(PD_UI_HEART,"PF",state.hp,pending.hp,83,199);delta_tile(PD_UI_SHIELD,"CA",pd_armor(&state),pd_armor(&pending),239,199);
            delta_tile(PD_UI_HEART,"PF massimi",pd_max_hp(&state),pd_max_hp(&pending),83,267);delta_tile(PD_UI_HEART,"Temporanei",state.temp_hp,pending.temp_hp,239,267);
        }else{
            snprintf(text,sizeof text,"%d/%d",pending.hp,pd_max_hp(&pending));status_tile(PD_UI_HEART,"PF",text,83,199);snprintf(text,sizeof text,"%d",pd_armor(&pending));status_tile(PD_UI_SHIELD,"CA",text,239,199);
            snprintf(text,sizeof text,"%u",pending.temp_hp);status_tile(PD_UI_HEART,"Temporanei",text,83,267);snprintf(text,sizeof text,"%u",backup_history.count);status_tile(PD_UI_DICE,"Tiri",text,239,267);
        }
        connection_note(backup_problem?backup_notice:restoring?"Sostituisce scheda attiva e registro":"Copia USB: conserva il file sul PC",335,20,backup_problem?PD_DANGER:restoring?PD_GOLD:PD_MUTED);
    }
    const char *names[]={"Annulla",connection_details?"Riepilogo":"Dettagli",restoring?"Ripristina":"Invia"};const unsigned actions[]={2801,2810,restoring?2803:2802};
    for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}

static unsigned connection_sync_state(const pd_sync_info *info){return info->state<=PD_SYNC_WAIT_NETWORK?info->state:PD_SYNC_ERROR;}
static const char *connection_sync_hint(unsigned status){return (const char *const[]){"Abbina dal PC sulla stessa rete","Attendi la risposta del ponte","Apri ponte e scheda Beyond sul PC","Verifica gli esiti sulla scheda PC","Controlla ponte e rete del PC","Risolvi il confronto sul PC","Controlla Beyond prima di ripetere","Riabbina questo PG dal PC","Collega Pocket e PC alla stessa rete"}[status];}
static void render_sync(void){
    pd_sync_info info;pd_sync_snapshot(&info);char text[180];unsigned status=connection_sync_state(&info);
    unsigned color=status==PD_SYNC_READY?PD_MINT:status==PD_SYNC_ERROR||status==PD_SYNC_CONFLICT||status==PD_SYNC_UNCERTAIN?PD_DANGER:PD_GOLD;
    if(page==SYNC_REVIEW){
        label("Abbina ponte PC?",83,111,300,false);wrapped_name(state.name,145,48,&lv_font_montserrat_14);
        snprintf(text,sizeof text,"Beyond: %s",info.character_id);small_label(text,83,201,300,PD_TEXT);
        snprintf(text,sizeof text,"PC: %s",info.host);small_label(text,83,225,300,PD_TEXT);
        connection_note("Confronta queste 32 cifre sul PC",251,20,PD_MUTED);
        snprintf(text,sizeof text,"%.16s",info.fingerprint);small_label(text,83,277,300,PD_MINT);small_label(info.fingerprint+16,83,301,300,PD_MINT);
        connection_note(sync_notice[0]?sync_notice:info.offer?"Conferma solo se l'impronta coincide":"Richiesta non disponibile",333,20,sync_notice[0]||!info.offer?PD_DANGER:PD_MUTED);
        button("Annulla",83,359,144,48,4701,true);button("Conferma",239,359,144,48,4702,info.offer);return;
    }
    if(page==SYNC_FORGET){
        label("Scollega ponte PC?",83,111,300,false);
        status_tile(PD_UI_LINK,"Ponte","Da scollegare",83,169);status_tile(PD_UI_WIFI,"Wi-Fi","Conservato",239,169);
        status_tile(PD_UI_SHEET,"Scheda","Conservata",83,237);status_tile(PD_UI_DICE,"Eventi","Non annullati",239,237);
        connection_note(sync_notice[0]?sync_notice:"Revoca anche il collegamento sul PC.",315,40,sync_notice[0]?PD_DANGER:PD_GOLD);
        button("Indietro",83,359,144,48,4704,true);button("Conferma",239,359,144,48,4705,true);return;
    }
    label("Ponte Beyond",83,111,300,true);
    if(connection_details){
        wrapped_name(state.name,145,48,&lv_font_montserrat_14);
        snprintf(text,sizeof text,"Beyond: %s",info.character_id[0]?info.character_id:"non abbinato");small_label(text,83,201,300,PD_TEXT);
        snprintf(text,sizeof text,"PC: %s",info.host[0]?info.host:"non impostato");small_label(text,83,225,300,PD_TEXT);
        connection_note(connection_sync_hint(status),255,40,color);
        connection_note("Ponte sperimentale: tieni PC e Beyond aperti. Verifica gli esiti sul PC.",299,56,PD_MUTED);
        button("Indietro",83,359,144,48,4707,true);
    }else{
        const char *titles[]={"Non abbinato","Collegamento...","Da avviare sul PC","Ponte pronto","Ponte non raggiunto","Dati in conflitto","Esito da verificare","PG diverso","Wi-Fi non collegato"};
        connection_banner(PD_UI_LINK,titles[status],sync_notice[0]?sync_notice:connection_sync_hint(status),sync_notice[0]?PD_DANGER:color);
        snprintf(text,sizeof text,"%u",info.queued);status_tile(PD_UI_USES,"In coda",text,83,233);snprintf(text,sizeof text,"%u",info.observed);status_tile(PD_UI_DICE,"Osservati",text,239,233);
        scene_control("Dettagli ponte",pd_ui_icon(PD_UI_SHEET),"Dettagli","PG e PC",83,301,144,48,4707,true);
        button("Rete",83,359,300,48,4706,true);
    }
    scene_control(info.configured?"Rimuovi":"Abbina",pd_ui_icon(info.configured?PD_UI_UNDO:PD_UI_LINK),info.configured?"Scollega":"Abbina","Dal PC",239,connection_details?359:301,144,48,info.configured?4703:4708,true);
}

static const char *connection_wifi_state(const pd_wifi_info *info){
    if(info->suspended)return "Wi-Fi in pausa";
    if(info->joining)return "Collegamento...";
    return (const char *const[]){"Wi-Fi spento","Collegamento...","Wi-Fi collegato","Rete non raggiunta"}[info->state<=PD_WIFI_FAILED?info->state:PD_WIFI_FAILED];
}
static const char *connection_wifi_notice(const pd_wifi_info *info){
    if(info->error==PD_WIFI_ERROR_STORAGE)return "Salvataggio rete non confermato";
    if(info->error==PD_WIFI_ERROR_UNAVAILABLE)return "Wi-Fi non disponibile";
    if(info->setup_failed)return "Nuova rete non salvata: riprova";
    if(info->error==PD_WIFI_ERROR_NETWORK)return "Controlla rete 2,4 GHz e password";
    if(info->setup_saved&&info->state==PD_WIFI_CONNECTED&&!info->suspended&&!info->joining)return "Rete collegata e salvata";
    return wifi_notice;
}
static lv_obj_t *connection_wifi_card(const char *name,pd_ui_icon_id icon,const char *caption,const char *badge,int x,int y,int w,int h,int action,bool enabled){
    lv_obj_t *b=scene_control(name,pd_ui_icon(icon),caption,badge,x,y,w,h,action,enabled);if(wifi_target_count<22)wifi_targets[wifi_target_count++]=b;return b;
}
static void render_connection_wifi(void){
    wifi_target_count=0;pd_wifi_info info;pd_wifi_snapshot(&info);char text[180];const char *issue=connection_wifi_notice(&info);
    if(page==WIFI_FORGET){
        label("Dimenticare la rete?",83,111,300,false);
        status_tile(PD_UI_KEYS,"Rete salvata","Da eliminare",83,169);status_tile(PD_UI_SHEET,"Personaggio","Conservato",239,169);
        status_tile(PD_UI_USB,"Backup PG","Conservati",83,237);status_tile(PD_UI_LINK,"Associazioni","Conservate",239,237);
        connection_note("Rimuove solo nome e password Wi-Fi.",315,40,PD_GOLD);
        wifi_button("Indietro",83,359,144,48,4506,true);wifi_button("Conferma",239,359,144,48,4507,true);wifi_mark_focus();return;
    }
    if(info.setup){
        connection_details=false;label("Telefono / PC",83,111,300,true);wrapped_name(info.setup_ssid,145,48,&lv_font_montserrat_14);
        connection_note("Collegati alla rete Pocket con:",201,20,PD_MUTED);small_label(info.setup_password,83,225,300,PD_GOLD);
        connection_note("Poi apri il portale nel browser:",257,20,PD_MUTED);small_label("http://192.168.4.1",83,281,300,PD_TEXT);
        snprintf(text,sizeof text,"Portale: %us | %s",info.setup_seconds,info.setup_saved?"rete salvata":info.setup_failed?"riprova":info.state==PD_WIFI_CONNECTING?"connessione...":"attesa");connection_note(issue[0]?issue:text,315,40,issue[0]?PD_GOLD:PD_MUTED);
    }else if(connection_details){
        label("Dettagli rete",83,111,300,false);wrapped_name(info.ssid[0]?info.ssid:info.configured?"Rete salvata":"Nessuna rete salvata",145,48,&lv_font_montserrat_14);
        small_label(connection_wifi_state(&info),83,201,300,info.state==PD_WIFI_CONNECTED&&!info.suspended?PD_MINT:PD_GOLD);
        snprintf(text,sizeof text,"Rete in memoria: %s",info.configured?"si":"no");small_label(text,83,227,300,PD_TEXT);
        snprintf(text,sizeof text,"IP: %s",info.ip[0]?info.ip:"non disponibile");small_label(text,83,253,300,PD_TEXT);
        connection_note(info.suspended?"Radio in pausa":info.radio_active?"Radio attiva | banda 2,4 GHz":"Radio spenta",281,20,PD_MUTED);
        connection_note(issue[0]?issue:"Il collegamento Wi-Fi non verifica l'accesso a Beyond.",311,40,issue[0]?PD_GOLD:PD_MUTED);
        wifi_button("Indietro",83,359,300,48,4508,true);wifi_mark_focus();return;
    }else{
        label(beyond_waiting?"Wi-Fi per Beyond":"Wi-Fi 2,4 GHz",83,111,300,true);
        if(issue[0])connection_note(issue,141,20,PD_GOLD);
        connection_wifi_card("Dettagli rete",PD_UI_WIFI,connection_wifi_state(&info),info.ssid[0]?info.ssid:info.configured?"Rete salvata":"Nessuna rete salvata",83,165,300,64,4508,true);
        connection_wifi_card("Cerca reti >",PD_UI_SEARCH,"Cerca reti","Dal Pocket",83,237,144,60,5000,!info.joining);
        connection_wifi_card("Configura da telefono / PC",PD_UI_PC,"PC / Tel.","Hotspot",239,237,144,60,4504,!info.joining);
        connection_wifi_card("Ponte",PD_UI_LINK,"Ponte Beyond","Opzionale | PC richiesto",83,305,300,44,4700,!beyond_network_flow());
    }
    const char *names[]={"Indietro",info.state==PD_WIFI_OFF&&!info.setup?"Accendi":"Spegni","Dimentica"};const unsigned actions[]={4501,info.state==PD_WIFI_OFF&&!info.setup?4502:4503,4505};
    for(unsigned i=0;i<3;i++)wifi_button(names[i],83+i*104,359,92,48,actions[i],i==0||info.configured||info.setup);
    wifi_mark_focus();
}
