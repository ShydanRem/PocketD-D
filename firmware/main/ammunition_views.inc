static void render_ammunition(void){
    char text[120];
    if(page==AMMO_RETRY||ammo_pending){
        wrapped_name(ammo_identity.name,99,48,&lv_font_montserrat_14);
        label("Tiro da salvare",83,155,300,true);
        delta_tile(PD_UI_AMMO,"Munizioni",ammo_before,ammo_after,83,203);status_tile(PD_UI_DICE,"Attacco","Stesso d20",239,203);
        connection_note(ammo_notice,281,42,PD_GOLD);
        connection_note("Il risultato resta fermo durante la riprova",327,25,PD_MUTED);
        button("Riprova",83,359,300,48,AC_RETRY,power_storage_ready());return;
    }
    unsigned count=ammunition_count(),pages=(count+2)/3;if(!pages)pages=1;if(ammo_page>=pages)ammo_page=pages-1;
    bool current=ammunition_picker_current();
    snprintf(text,sizeof text,"Munizioni %u/%u",ammo_page+1,pages);small_label(text,139,113,188,PD_TEXT);
    button("<",83,99,48,44,AC_PREV,current&&ammo_page>0);button(">",335,99,48,44,AC_NEXT,current&&ammo_page+1<pages);
    for(unsigned row=0;row<3;row++){
        unsigned index=ammunition_at(ammo_page*3+row);if(index>=PD_ITEMS)break;
        snprintf(text,sizeof text,"%u rimaste | -1 / attacco",state.items[index].quantity);
        lv_obj_t *b=scene_control(state.items[index].name,pd_ui_icon(PD_UI_AMMO),state.items[index].name,text,83,151+row*64,300,56,AC_ITEM_FIRST+index,current&&state.items[index].quantity);
        rules_selected_style(b,ammunition_bound()&&ammo_item==index);
    }
    if(!count)connection_note("Nessuna pila disponibile: aggiungila allo zaino dal PC",193,80,PD_MUTED);
    connection_note(ammo_notice[0]?ammo_notice:"Scegli solo munizioni adatte all'arma",339,18,PD_GOLD);
    button("Indietro",83,359,144,48,AC_BACK,true);lv_obj_t *none=button("Senza consumo",239,359,144,48,AC_NONE,current);label_font(lv_obj_get_child(none,0),&lv_font_montserrat_14);
}
