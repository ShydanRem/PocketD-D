static void render_handling(void){
    char text[140];const pd_item *item=&state.items[handling_index];bool valid=handling_preview();
    wrapped_name(item->name,99,48,&lv_font_montserrat_14);
    if(page==HANDLING_SETUP){
        if(handling_draft.flags&PD_VERSATILE)snprintf(text,sizeof text,"Versatile: d%u",handling_draft.sides);else snprintf(text,sizeof text,"Tipo: %s",handling_draft.flags&PD_TWO_HANDED?"due mani":"manuale");
        button(text,83,151,300,44,WH_TYPE,handling_current());
        button((handling_draft.grip||(handling_draft.flags&PD_TWO_HANDED))?"Impugnatura: due mani":"Impugnatura: una mano",83,203,300,44,WH_GRIP,handling_current()&&(handling_draft.flags&PD_VERSATILE));
        button(handling_draft.flags&PD_FINESSE?"Finezza: si":"Finezza: no",83,255,144,44,WH_FINESSE,handling_current()&&item->ability<2);
        button(handling_draft.flags&PD_LIGHT?"Leggera: si":"Leggera: no",239,255,144,44,WH_LIGHT,handling_current());
        scratch=state;scratch.handling[handling_index]=handling_draft;
        if(handling_current()&&!pd_weapon_hands_ready(&scratch,handling_index))button("Riponi scudo >",83,307,300,44,WH_SHIELD,true);
        else button(handling_draft.flags&PD_GRAZE?"Maestria: Scalfire 2024":"Maestria: nessuna",83,307,300,44,WH_GRAZE,handling_current()&&!(item->flags&4));
        button("Indietro",83,359,144,48,WH_BACK,true);button("Rivedi",239,359,144,48,WH_REVIEW,valid);return;
    }
    pd_weapon_info old={0},next={0};pd_weapon_stats(&state,handling_index,&old);if(valid)pd_weapon_stats(&scratch,handling_index,&next);
    snprintf(text,sizeof text,"Attacco %+d > %+d",old.attack_bonus,next.attack_bonus);label(text,83,159,300,true);
    snprintf(text,sizeof text,"Danni %ud%u%+d > %ud%u%+d",old.count,old.sides,old.damage_bonus,next.count,next.sides,next.damage_bonus);small_label(text,83,217,300,PD_GOLD);
    snprintf(text,sizeof text,"%s | %s",pd_weapon_two_hands(&scratch,handling_index)?"Due mani":"Una mano",handling_draft.flags&PD_FINESSE?"FOR/DES migliore":"Caratteristica manuale");connection_note(text,263,42,PD_TEXT);
    connection_note(handling_notice[0]?handling_notice:"Salva senza tirare o consumare risorse",323,30,PD_MUTED);
    button("Modifica",83,359,144,48,WH_EDIT,true);button("Salva",239,359,144,48,WH_SAVE,valid&&power_storage_ready());
}
