static void render_dual(void){
    char text[150];
    if(page==BONUS_ACTION){
        label("Azione bonus",83,103,300,true);small_label(state.dual.bonus_spent?"Ora: utilizzata":"Ora: disponibile",83,151,300,PD_GOLD);
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
        button(dual_draft.edition==2?"Regola: Light 2024":"Regola: due armi 2014",83,203,300,44,DW_EDITION,dual_draft.weapon);
        button(dual_draft.style?"Stile: aggiungi caratteristica":"Stile: nessun bonus positivo",83,255,300,44,DW_STYLE,dual_draft.weapon);
        button(dual_light?"Entrambe Leggere: si":"Entrambe Leggere: conferma",83,307,300,44,DW_LIGHT,dual_draft.weapon);
        bool valid=dual_preview();button("Indietro",83,359,144,48,DW_BACK,true);button("Salva",239,359,144,48,DW_SAVE,valid&&power_storage_ready());
        return;
    }
    pd_weapon_info w={0};bool valid=pd_dual_stats(&state,&w),ready=pd_dual_prepare(&state,true,NULL);wrapped_name(state.dual.weapon?state.items[state.dual.weapon-1].name:"Secondaria",99,45,&lv_font_montserrat_16);
    if(valid){snprintf(text,sizeof text,"Att %+d | danni %ud%u%+d",w.attack_bonus,w.count,w.sides,w.damage_bonus);label(text,83,155,300,false);}else small_label("Verifica armi Leggere, mani e scudo",83,155,300,PD_DANGER);
    pd_attack_resolution gate={0};pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);
    button("-",83,199,48,44,86,target_armor>1&&!dual_pending);snprintf(text,sizeof text,"CA %u",target_armor);label(text,139,211,188,false);button("+",335,199,48,44,87,target_armor<30&&!dual_pending);
    bool shield=false;for(unsigned i=0;i<state.item_count;i++)if(state.gear[i].kind==PD_WEAR_SHIELD&&state.gear[i].equipped)shield=true;
    bool ranged=state.dual.weapon&&(state.items[state.dual.weapon-1].flags&4);
    lv_obj_t *adv=button(gate.mode>0?"Vantaggio":gate.mode<0?"Svantaggio":"Normale",83,251,ranged?92:144,44,15,!dual_pending);label_font(lv_obj_get_child(adv,0),&lv_font_montserrat_14);
    lv_obj_t *config=button(shield?"Scudo >":"Configura",ranged?187:239,251,ranged?92:144,44,shield?DW_SHIELD:DW_CONFIG,!dual_pending);label_font(lv_obj_get_child(config,0),&lv_font_montserrat_14);
    if(ranged){lv_obj_t *ammo=button("Munizioni",291,251,92,44,DW_AMMO,!dual_pending);label_font(lv_obj_get_child(ammo,0),&lv_font_montserrat_14);}
    connection_note(dual_notice[0]?dual_notice:state.dual.bonus_spent?"Azione bonus gia usata":!ready?"Verifica anche la pila di munizioni":"Confermi un attacco con l'altra arma nell'azione Attacco di questo turno",303,46,ready?PD_MUTED:PD_GOLD);
    button(dual_pending?"In attesa":state.dual.bonus_spent?"Turno":"Indietro",83,359,112,48,state.dual.bonus_spent?DW_TURN:DW_BACK,!dual_pending);
    lv_obj_t *b=button(dual_pending?"Riprova":gate.allowed?"Lancia: bonus":"Contesto",207,359,176,48,dual_pending?DW_RETRY:gate.allowed?DW_ROLL:2230,dual_pending||(ready&&power_storage_ready()));label_font(lv_obj_get_child(b,0),&lv_font_montserrat_16);
}
