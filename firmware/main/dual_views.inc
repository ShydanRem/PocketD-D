static void render_dual(void){
    char text[150];
    if(page==MASTERY_HELP){
        snprintf(text,sizeof text,"%s 2024",pd_mastery_name(strike.weapon.mastery));label(text,83,103,300,true);
        char note[220];if(pd_mastery_text(&strike,state.level,note,sizeof note))connection_note(note,159,110,PD_TEXT);
        bool cleave=strike.weapon.mastery==3&&strike.hits;
        connection_note(cleave?!damage_used?"Prima risolvi i danni del colpo iniziale":state.dual.cleave_spent?"Cleave gia usato in questo turno":"Conferma un altro bersaglio valido":strike.weapon.mastery==8?"Vex manuale: vantaggio contro lo stesso nemico. Durata e consumo al tavolo.":strike.weapon.mastery>=4?"Applica sul nemico al tavolo. Nessun TS fittizio o condizione sul tuo PG":"Regola del tiro; nessuna nuova spesa",283,60,PD_GOLD);
        button("Indietro",83,359,cleave?144:300,48,DW_BACK,true);if(cleave)button("Cleave",239,359,144,48,DW_CLEAVE,damage_used&&dual_current()&&pd_cleave_prepare(&state,&strike,true,NULL));return;
    }
    if(page==BONUS_ACTION){
        static const char *const names[]={"Azione bonus","Extra Light / Nick","Extra Dual Wielder","Extra Cleave"};button(names[bonus_counter],83,103,300,44,DW_COUNTER,true);small_label(*extra_counter(&state,bonus_counter)?"Ora: utilizzata":"Ora: disponibile",83,151,300,PD_GOLD);
        button("Segna utilizzata",83,191,300,48,DW_SPEND,true);button("Libera: correzione DM",83,249,300,48,DW_FREE,true);
        snprintf(text,sizeof text,"Dopo: %s",dual_bonus_draft?"utilizzata":"disponibile");small_label(text,83,311,300,PD_MINT);connection_note(dual_notice[0]?dual_notice:"Si recupera a Inizio del tuo turno",335,18,PD_MUTED);
        button("Indietro",83,359,144,48,DW_BACK,true);button("Salva",239,359,144,48,DW_SAVE,dual_current()&&power_storage_ready());return;
    }
    if(page==DUAL_PICKER){
        label("Arma secondaria",83,99,300,true);
        for(unsigned row=0;row<3;row++){unsigned i=dual_page*3+row;if(i>=state.item_count)break;const pd_item *v=state.items+i;snprintf(text,sizeof text,"%s",v->name);lv_obj_t *b=button(text,83,153+row*58,300,50,DW_ITEM+i,i!=dual_primary&&v->kind==PD_WEAPON&&v->quantity);label_font(lv_obj_get_child(b,0),&lv_font_montserrat_16);}
        button("<",83,359,64,48,DW_PREV,dual_page);button("Indietro",159,359,148,48,DW_BACK,true);button(">",319,359,64,48,DW_NEXT,(dual_page+1)*3<state.item_count);return;
    }
    if(page==DUAL_SETUP){
        lv_obj_t *heading=label(dual_notice[0]?dual_notice:"Due armi",83,99,216,true);if(dual_notice[0])label_font(heading,&lv_font_montserrat_16);button("X",323,99,60,44,DW_NONE,true);
        snprintf(text,sizeof text,"%s",dual_draft.weapon?state.items[dual_draft.weapon-1].name:"Scegli secondaria");button(text,83,151,300,44,DW_PICK,dual_primary<state.item_count);
        button(dual_draft.feat?"Regola: Dual Wielder 2024":dual_draft.edition==2?"Regola: Light 2024":"Regola: due armi 2014",83,203,300,44,DW_EDITION,dual_draft.weapon);
        button(dual_draft.style?"Stile: aggiungi caratteristica":"Stile: nessun bonus positivo",83,255,300,44,DW_STYLE,dual_draft.weapon);
        button(dual_draft.feat?(dual_light?"Principale Leggera: si":"Principale Leggera: conferma"):(dual_light&&dual_other_light?"Entrambe Leggere: si":"Entrambe Leggere: conferma"),83,307,300,44,DW_LIGHT,dual_draft.weapon);
        bool valid=dual_preview();button("Indietro",83,359,144,48,DW_BACK,true);button("Salva",239,359,144,48,DW_SAVE,valid&&power_storage_ready());
        return;
    }
    bool cleave=page==CLEAVE_ATTACK;
    pd_weapon_info w={0};bool valid=cleave?pd_cleave_stats(&state,&cleave_origin,&w):pd_dual_stats_for(&state,dual_method,&w),ready=cleave?pd_cleave_prepare(&state,&cleave_origin,true,NULL):pd_dual_prepare_for(&state,dual_method,true,NULL);
    unsigned weapon=cleave?cleave_origin.weapon.index:state.dual.weapon?state.dual.weapon-1:PD_ITEMS;
    wrapped_name(weapon<state.item_count?state.items[weapon].name:"Arma extra",99,32,&lv_font_montserrat_16);
    if(!cleave)for(unsigned i=0;i<3;i++){pd_weapon_info item;bool eligible=pd_dual_stats_for(&state,i,&item);const char *names[]={"Light","Nick","Talento"};bool available=pd_dual_prepare_for(&state,i,true,NULL);lv_obj_t *b=button(names[i],83+104*i,139,92,44,DW_METHOD+i,eligible&&!dual_pending);label_font(lv_obj_get_child(b,0),&lv_font_montserrat_14);rules_selected_style(b,dual_method==i);if(eligible&&!available)lv_obj_set_style_text_color(b,lv_color_hex(PD_MUTED),0);}
    else label("Cleave | parte di Attacco",83,143,300,false);
    if(valid){snprintf(text,sizeof text,"Att %+d | %ud%u%+d",w.attack_bonus,w.count,w.sides,w.damage_bonus);small_label(text,83,191,300,PD_GOLD);}else small_label("Verifica armi, Light e impugnatura",83,191,300,PD_DANGER);
    pd_attack_resolution gate={0};pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);
    button("-",83,223,48,44,86,target_armor>1&&!dual_pending);snprintf(text,sizeof text,"CA %u",target_armor);label(text,139,235,188,false);button("+",335,223,48,44,87,target_armor<30&&!dual_pending);
    bool shield=false;for(unsigned i=0;i<state.item_count;i++)if(state.gear[i].kind==PD_WEAR_SHIELD&&state.gear[i].equipped)shield=true;
    bool ranged=!cleave&&state.dual.weapon&&(state.items[state.dual.weapon-1].flags&4);
    lv_obj_t *adv=button(gate.mode>0?"Vantaggio":gate.mode<0?"Svantaggio":"Normale",83,275,ranged?92:144,44,15,!dual_pending);label_font(lv_obj_get_child(adv,0),&lv_font_montserrat_14);
    lv_obj_t *config=button(shield?"Scudo >":cleave?"Contesto":"Configura",ranged?187:239,275,ranged?92:144,44,shield?DW_SHIELD:cleave?2230:DW_CONFIG,!dual_pending);label_font(lv_obj_get_child(config,0),&lv_font_montserrat_14);
    if(ranged){lv_obj_t *ammo=button("Munizioni",291,275,92,44,DW_AMMO,!dual_pending);label_font(lv_obj_get_child(ammo,0),&lv_font_montserrat_14);}
    const char *problem=cleave?state.dual.cleave_spent?"Cleave gia usato":"Altro nemico: vicino al primo e in portata":dual_method==PD_EXTRA_FEAT?state.dual.feat_spent?"Talento gia usato":state.dual.bonus_spent?"Azione bonus gia usata":"Confermi altra arma in Attacco, tuo turno":state.dual.light_spent?"Extra Light gia usato":dual_method!=PD_EXTRA_NICK&&state.dual.bonus_spent?"Azione bonus gia usata":!ready?"Verifica armi e munizioni":"Confermi altra arma in Attacco, tuo turno";
    connection_note(dual_notice[0]?dual_notice:problem,323,30,ready?PD_MUTED:PD_GOLD);
    button(dual_pending?"In attesa":!ready?"Turno":"Indietro",83,359,112,48,!ready?DW_TURN:DW_BACK,!dual_pending);
    const char *roll=cleave?"Lancia Cleave":dual_method==PD_EXTRA_NICK?"Lancia Nick":"Lancia: bonus";
    lv_obj_t *b=button(dual_pending?"Riprova":gate.allowed?roll:"Contesto",207,359,176,48,dual_pending?DW_RETRY:gate.allowed?DW_ROLL:2230,dual_pending||(ready&&power_storage_ready()));label_font(lv_obj_get_child(b,0),&lv_font_montserrat_16);
}
