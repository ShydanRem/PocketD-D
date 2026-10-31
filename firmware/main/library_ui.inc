/* Local PG collection. Included after the state/history and UI helpers. */
static pd_library library;
static pd_library_catalog library_catalog;
static unsigned library_page,library_slot,library_focus,library_target_count;
static uint32_t library_origin;
static bool library_keyboard,library_failed;
static lv_obj_t *library_targets[6];
static char library_notice[100];
static bool library_view(void){return page==LIBRARY_HOME||page==LIBRARY_CONFIRM;}
static void library_yield(void *unused){(void)unused;
#ifndef PD_UI_TEST
    vTaskDelay(pdMS_TO_TICKS(1));
#endif
}
static bool library_ready(void){
    const char *root=pd_board_library_root();
    if(!root){snprintf(library_notice,sizeof library_notice,"Inserisci una SD per conservare piu PG");return false;}
    uint64_t owner=pd_library_owner();
    if(!owner){snprintf(library_notice,sizeof library_notice,"Identita non salvata: riavvia");return false;}
    if(!pd_library_init(&library,root,owner,library_yield,NULL)){snprintf(library_notice,sizeof library_notice,"SD non scrivibile: PG attuale conservato");return false;}
    return true;
}
static const char *library_error(pd_library_result r){
    return r==PD_LIBRARY_MEMORY?"Memoria occupata: riprova":r==PD_LIBRARY_FULL?"Raccolta piena: nessuna scheda cancellata":r==PD_LIBRARY_FUTURE?"Copia di versione futura: conservata":r==PD_LIBRARY_CONFLICT?"Identita ambigua: nessuna copia sostituita":r==PD_LIBRARY_CORRUPT?"Copia non leggibile: PG attuale conservato":"Errore SD: PG attuale conservato";
}
static bool library_fingerprint(const pd_backup *snapshot,uint32_t *fingerprint){
    size_t size;char *json=pd_backup_encode(&snapshot->state,&snapshot->history,&size);if(!json)return false;
    *fingerprint=pd_crc32((const uint8_t *)json,size);free(json);return true;
}
/* Called before any confirmed replacement, never during a read-only preview.
   Without SD the existing single-PG/USB workflow remains available. */
static bool library_checkpoint(void){
    if(!pd_board_library_root())return true;
    if(!power_storage_ready()||!library_ready())return false;
    pd_library_result r=pd_library_save(&library,0,&state,&history,NULL);
    if(r!=PD_LIBRARY_OK){snprintf(library_notice,sizeof library_notice,"%s",library_error(r));return false;}return true;
}
static bool library_next_epoch(uint32_t current,uint32_t *next){
    uint32_t highest=current;
    if(pd_board_library_root()){
        pd_library_catalog catalog;if(!library_ready()||!pd_library_scan(&library,&catalog))return false;
        for(unsigned i=0;i<catalog.count;i++)if((catalog.entries[i].status==PD_LIBRARY_OK||catalog.entries[i].status==PD_LIBRARY_RECOVERED)&&catalog.entries[i].owner==library.owner&&catalog.entries[i].epoch>highest)highest=catalog.entries[i].epoch;
    }
    return pd_history_store_next_epoch(highest,next);
}
static void library_open(void){
    library_catalog.count=0;library_page=library_focus=0;library_keyboard=false;library_failed=false;
    page=LIBRARY_HOME;library_notice[0]=0;
    if(!power_storage_ready()){snprintf(library_notice,sizeof library_notice,"Prima risolvi memoria e storico");return;}
    if(!library_ready()||!library_checkpoint())return;
    if(!pd_library_scan(&library,&library_catalog)){snprintf(library_notice,sizeof library_notice,"Errore lettura SD: riprova");return;}
    snprintf(library_notice,sizeof library_notice,"%u PG sulla SD | attuale salvato",library_catalog.count);
}
static void library_select(unsigned index){
    if(index>=library_catalog.count||!power_storage_ready()||!library_ready())return;
    const pd_library_entry *e=library_catalog.entries+index;
    if(e->owner==library.owner&&e->epoch==state.history_epoch&&(e->status==PD_LIBRARY_OK||e->status==PD_LIBRARY_RECOVERED)){page=HERO;return;}
    pd_backup *snapshot=malloc(sizeof *snapshot);if(!snapshot){snprintf(library_notice,sizeof library_notice,"Memoria occupata: riprova");return;}
    pd_library_result r=pd_library_read(&library,e->slot,snapshot);
    if((r==PD_LIBRARY_OK||r==PD_LIBRARY_RECOVERED)&&library_fingerprint(snapshot,&library_origin)){
        pending=snapshot->state;backup_history=snapshot->history;library_slot=e->slot;
        pending.brightness=state.brightness;pending.dim_timeout=state.dim_timeout;pending.idle_animation=state.idle_animation;memcpy(pending.shortcuts,state.shortcuts,3);
        library_notice[0]=0;library_failed=false;library_keyboard=false;library_focus=0;page=LIBRARY_CONFIRM;
    }else snprintf(library_notice,sizeof library_notice,"%s",library_error(r==PD_LIBRARY_OK||r==PD_LIBRARY_RECOVERED?PD_LIBRARY_MEMORY:r));
    free(snapshot);
}
static void library_confirm(void){
    if(library_failed||!power_storage_ready()||!library_ready()||!library_checkpoint())return;
    /* Re-read after preview: no stale snapshot may replace a changed SD copy. */
    pd_backup *check=malloc(sizeof *check);uint32_t fingerprint=0;
    pd_library_result checked=check?pd_library_read(&library,library_slot,check):PD_LIBRARY_MEMORY;
    bool same=check&&(checked==PD_LIBRARY_OK||checked==PD_LIBRARY_RECOVERED)&&library_fingerprint(check,&fingerprint)&&fingerprint==library_origin;
    free(check);if(!same){snprintf(library_notice,sizeof library_notice,"Copia cambiata: torna alla raccolta");return;}
    uint32_t epoch;if(!library_next_epoch(state.history_epoch,&epoch)){snprintf(library_notice,sizeof library_notice,"Storico o SD non disponibili: PG conservato");return;}
    /* The epoch maps the saved slot to the active NVS state without a separate
       pointer. Save target SD BEFORE committing its new NVS identity. */
    pending.history_epoch=epoch;backup_history.epoch=epoch;
    if(!pd_sync_pause(true)){snprintf(library_notice,sizeof library_notice,"Ponte occupato: riprova");return;}
    if(!pd_history_store_prepare_restore(&backup_history)){pd_sync_pause(false);snprintf(library_notice,sizeof library_notice,"Errore storico: riprova");return;}
    pd_library_result r=pd_library_save(&library,library_slot,&pending,&backup_history,NULL);
    if(r!=PD_LIBRARY_OK){pd_history_store_cancel_restore();pd_sync_pause(false);snprintf(library_notice,sizeof library_notice,"%s",library_error(r));return;}
    if(!pd_storage_save(&pending)){
        /* The commit may already be durable. Retain BOTH histories and SD
           snapshots; no retry/navigation/second commit until reboot. */
        pd_history_store_uncertain_restore();library_failed=true;snprintf(library_notice,sizeof library_notice,"Memoria incerta: riavvia. Copie su SD");return;
    }
    pd_history_store_finish_restore();history=backup_history;adopt_pending_state();
    pd_sync_observe(&state,&history,true);pd_sync_pause(false);
}
static lv_obj_t *library_control(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled){
    lv_obj_t *b=button(text,x,y,w,h,action,enabled);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
    if(library_target_count<6)library_targets[library_target_count++]=b;
    return b;
}
static void library_mark_focus(void){
    for(unsigned i=0;i<library_target_count;i++){
        lv_obj_set_style_outline_color(library_targets[i],lv_color_hex(PD_GOLD),0);
        lv_obj_set_style_outline_pad(library_targets[i],2,0);
        lv_obj_set_style_outline_width(library_targets[i],library_keyboard&&i==library_focus?2:0,0);
    }
}
static void render_library(void){
    char text[120];library_target_count=0;
    if(page==LIBRARY_CONFIRM){
        label("Apri questo PG?",83,111,300,true);label(pending.name,83,155,300,false);
        snprintf(text,sizeof text,"Liv. %u | PF %u/%u +%u | CA %u",pending.level,pending.hp,pd_max_hp(&pending),pending.temp_hp,pd_armor(&pending));small_label(text,83,195,300,PD_GOLD);
        snprintf(text,sizeof text,"Aspetto, risorse e %u tiri salvati",backup_history.count);small_label(text,83,231,300,PD_TEXT);
        small_label("Il PG attuale resta nella raccolta",83,267,300,PD_MINT);
        lv_obj_t *notice=small_label(library_notice,83,303,300,PD_DANGER);lv_label_set_long_mode(notice,LV_LABEL_LONG_WRAP);lv_obj_set_height(notice,40);
        library_control("Indietro",83,359,144,48,5304,!library_failed);library_control("Apri e gioca",239,359,144,48,5305,!library_failed&&power_storage_ready());
    }else{
        label("I miei PG",83,111,300,true);
        lv_obj_t *notice=small_label(library_notice,83,153,300,PD_GOLD);lv_label_set_long_mode(notice,LV_LABEL_LONG_WRAP);lv_obj_set_height(notice,38);
        for(unsigned i=0;i<3&&library_page*3+i<library_catalog.count;i++){
            const pd_library_entry *e=library_catalog.entries+library_page*3+i;bool known=e->status==PD_LIBRARY_OK||e->status==PD_LIBRARY_RECOVERED;
            lv_obj_t *b=library_control(known?e->name:"Copia non leggibile",83,195+52*i,300,44,5310+i,known);
            lv_obj_set_style_pad_all(b,0,0);
            lv_obj_t *name=lv_obj_get_child(b,0);lv_label_set_long_mode(name,LV_LABEL_LONG_DOT);lv_obj_set_width(name,272);lv_obj_set_height(name,19);lv_obj_align(name,LV_ALIGN_TOP_LEFT,12,1);lv_obj_set_style_text_align(name,LV_TEXT_ALIGN_LEFT,0);
            lv_obj_t *detail=lv_label_create(b);lv_obj_set_style_text_font(detail,&lv_font_montserrat_16,0);lv_label_set_long_mode(detail,LV_LABEL_LONG_DOT);lv_obj_set_size(detail,272,19);lv_obj_set_style_text_align(detail,LV_TEXT_ALIGN_LEFT,0);
            if(known)snprintf(text,sizeof text,"Livello %u%s%s",e->level,e->owner==library.owner&&e->epoch==state.history_epoch?" | Attuale":"",e->status==PD_LIBRARY_RECOVERED?" | Copia recuperata":"");
            else snprintf(text,sizeof text,"Copia %u | conservata sulla SD",e->slot);
            lv_label_set_text(detail,text);lv_obj_align(detail,LV_ALIGN_TOP_LEFT,12,23);
        }
        library_control("Indietro",83,359,92,48,5301,true);library_control("<",187,359,92,48,5302,library_page>0);library_control(">",291,359,92,48,5303,(library_page+1)*3<library_catalog.count);
    }
    library_mark_focus();
}
