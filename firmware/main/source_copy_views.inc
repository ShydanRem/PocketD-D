static void render_source_copy(void){
    char text[140];bool current=source_copy_current();wrapped_name(pending.name,99,48,&lv_font_montserrat_14);
    snprintf(text,sizeof text,"Beyond #%s | copie %u",pending.beyond_id,source_copy_count);lv_obj_t *origin=small_label(text,83,151,300,PD_GOLD);label_font(origin,&lv_font_montserrat_14);lv_obj_t *direction=small_label(source_copy_slot?"Copia SD > Beyond":"Attuale > Beyond",83,173,300,PD_MUTED);label_font(direction,&lv_font_montserrat_14);
    delta_tile(PD_UI_HEART,"PF",hit_candidate.hp,pending.hp,83,191);delta_tile(PD_UI_SHIELD,"CA",pd_armor(&hit_candidate),pd_armor(&pending),239,191);
    snprintf(text,sizeof text,"PF max %u>%u | Liv %u>%u\nZaino %u>%u | Magie %u>%u",pd_max_hp(&hit_candidate),pd_max_hp(&pending),hit_candidate.level,pending.level,hit_candidate.item_count,pending.item_count,hit_candidate.magic.count,pending.magic.count);connection_note(text,267,36,PD_TEXT);
    connection_note(!current?"Scheda cambiata: annulla e riapri":source_copy_notice[0]?source_copy_notice:pd_board_library_root()?"Nuova copia; progressi e aspetto non vengono uniti":"Senza SD sostituisce il PG attivo. Conserva un backup.",311,40,PD_MUTED);
    button(source_copy_slot?"Apri locale":"Tieni locale",83,359,144,48,IC_CANCEL,true);lv_obj_t *go=button(pd_board_library_root()?"Nuova copia":"Sostituisci",239,359,144,48,IC_NEW,current);label_font(lv_obj_get_child(go,0),&lv_font_montserrat_16);
}
