static void render_spell_battle(void){
    char text[160];const pd_power *p=&spell_origin.magic.powers[spell_plan.power];bool current=spell_battle_current();
    int attack=0,dc=0,effective=0;unsigned needed=0;pd_magic_stats(&spell_origin,spell_plan.power,&attack,&dc);pd_spell_attack_gate(&spell_origin,spell_battle.requested,spell_battle.context,&effective,&needed);
    bool valid=!spell_paid&&current&&pd_spell_battle_prepare(&state,&spell_plan,&spell_battle,&scratch);
    wrapped_name(p->name,99,48,&lv_font_montserrat_14);
    if(page==SPELL_BATTLE_SETUP){
        if(spell_stat_picker){
            small_label("TS richiesto: scegli tu",83,151,300,PD_TEXT);
            for(unsigned i=0;i<6;i++){lv_obj_t *b=button(pd_abilities[i],83+(i%2)*156,187+(i/2)*52,144,44,SC_STAT_FIRST+i,current);rules_selected_style(b,spell_battle.save_ability==i);}
            button("Indietro",83,359,300,48,SC_STAT_BACK,true);return;
        }
        snprintf(text,sizeof text,spell_battle.mode==PD_SPELL_ATTACK?"Attacco %+d":"CD %d",spell_battle.mode==PD_SPELL_ATTACK?attack:dc);label(text,83,151,300,true);
        if(spell_battle.mode==PD_SPELL_ATTACK){
            snprintf(text,sizeof text,"CA %u -",spell_battle.armor);button(text,83,195,144,44,SC_ARMOR_LESS,current&&spell_battle.armor>1);
            snprintf(text,sizeof text,"CA %u +",spell_battle.armor);button(text,239,195,144,44,SC_ARMOR_MORE,current&&spell_battle.armor<40);
            button(spell_battle.requested>0?"Vantaggio":spell_battle.requested<0?"Svantaggio":"Base",83,247,300,44,SC_MODE,current);
            if(state.roll_conditions&PD_FRIGHTENED)button(!(spell_battle.context.known&PD_CONTEXT_FEAR)?"Paura: da dichiarare":spell_battle.context.values&PD_CONTEXT_FEAR?"Fonte paura visibile":"Fonte paura non visibile",83,299,300,44,SC_FEAR,current);
            else connection_note(effective<0?"Svantaggio dagli effetti attivi":"Distanza, copertura e bersaglio: al tavolo",303,38,PD_MUTED);
        }else{
            snprintf(text,sizeof text,"TS bersaglio: %s",(spell_battle.save_ability<6?pd_abilities[spell_battle.save_ability]:"scegli"));button(text,83,195,300,44,SC_STAT,current);
            small_label("Su TS riuscito:",83,251,300,PD_MUTED);
            lv_obj_t *a=button("Zero danni",83,279,144,44,SC_ZERO,current),*b=button("Meta danni",239,279,144,44,SC_HALF,current);label_font(lv_obj_get_child(a,0),&lv_font_montserrat_16);label_font(lv_obj_get_child(b,0),&lv_font_montserrat_16);rules_selected_style(a,spell_battle.success==PD_SPELL_SAVE_ZERO);rules_selected_style(b,spell_battle.success==PD_SPELL_SAVE_HALF);
            connection_note("Il DM tira il TS del bersaglio",331,18,PD_MUTED);
        }
        button("Indietro",83,359,144,48,SC_BACK,true);button("Rivedi",239,359,144,48,SC_REVIEW,valid);return;
    }
    if(page==SPELL_TARGET_SAVE){
        snprintf(text,sizeof text,"TS %s | CD %d",(spell_battle.save_ability<6?pd_abilities[spell_battle.save_ability]:"scegli"),dc);label(text,83,155,300,true);
        small_label("Esito dichiarato dal DM",83,203,300,PD_TEXT);
        scene_control("Fallito",pd_ui_icon(PD_UI_ATTACK),"Fallito","Danni pieni",83,247,144,64,SC_FAIL,current&&!history_pending&&!spell_damage_done);
        scene_control("Riuscito",pd_ui_icon(PD_UI_SHIELD),"Riuscito",spell_battle.success==PD_SPELL_SAVE_HALF?"Meta danni":"Zero danni",239,247,144,64,SC_PASS,current&&!history_pending&&!spell_damage_done);
        connection_note(current?"Costo salvato; esito al tavolo":"Scheda cambiata: danni non tirati",325,25,PD_GOLD);button("Chiudi",83,359,300,48,SC_DONE,true);return;
    }
    if(page==SPELL_BATTLE_RECEIPT){
        if(spell_battle.mode==PD_SPELL_ATTACK)snprintf(text,sizeof text,"Attacco %+d | CA %u",attack,spell_battle.armor);
        if(spell_battle.mode==PD_SPELL_TARGET_SAVE)snprintf(text,sizeof text,"TS %s | CD %d",(spell_battle.save_ability<6?pd_abilities[spell_battle.save_ability]:"scegli"),dc);
        small_label(text,83,155,300,PD_GOLD);
        snprintf(text,sizeof text,"Dadi %d | potenziali %d",spell_impact.rolled,spell_impact.applicable);label(text,83,199,300,false);
        connection_note(!spell_damage_done?"Costo salvato. Nessun danno tirato.":spell_impact.impact==PD_SPELL_ZERO?"TS riuscito: zero danni, nessun dado tirato.":spell_impact.impact==PD_SPELL_HALF?"TS riuscito: meta del totale, per difetto.":spell_impact.critical?"Critico: dadi doppi, bonus una volta.":"Danno potenziale del singolo bersaglio.",239,54,PD_TEXT);
    }else{
        snprintf(text,sizeof text,"%ud%u%+d | %s",spell_plan.count,spell_plan.sides,spell_plan.bonus,spell_battle.mode==PD_SPELL_ATTACK?"attacco":"TS bersaglio");small_label(text,83,155,300,PD_GOLD);
        if(spell_plan.slot){snprintf(text,sizeof text,"Slot %u %s",spell_plan.slot,spell_plan.pact?"patto":"ord.");delta_tile(PD_UI_MAGIC,text,spell_plan.pact?spell_origin.magic.pact[spell_plan.slot-1].current:spell_origin.magic.slots[spell_plan.slot-1].current,spell_plan.pact?scratch.magic.pact[spell_plan.slot-1].current:scratch.magic.slots[spell_plan.slot-1].current,83,195);}else status_tile(PD_UI_MAGIC,"Slot","Nessun costo",83,195);
        if(p->flags&PD_LIMITED)delta_tile(PD_UI_USES,"Utilizzi",p->remaining,scratch.magic.powers[spell_plan.power].remaining,239,195);else status_tile(PD_UI_USES,"Utilizzi","Non richiesti",239,195);
        if(spell_battle.mode==PD_SPELL_ATTACK)snprintf(text,sizeof text,"Att %+d | CA %u | %s",attack,spell_battle.armor,effective>0?"vant":effective<0?"svant":"base");else snprintf(text,sizeof text,"TS %s CD %d | %s",(spell_battle.save_ability<6?pd_abilities[spell_battle.save_ability]:"scegli"),dc,spell_battle.success==PD_SPELL_SAVE_HALF?"meta":"zero");small_label(text,83,271,300,PD_TEXT);
        snprintf(text,sizeof text,"PF %u>%u | max %u>%u\nCA %u>%u | conc. %s",spell_origin.hp,scratch.hp,pd_max_hp(&spell_origin),pd_max_hp(&scratch),pd_armor(&spell_origin),pd_armor(&scratch),spell_origin.magic.concentration==scratch.magic.concentration?"invariata":!scratch.magic.concentration?"fine":spell_origin.magic.concentration?"sostituisce":"inizia");connection_note(text,295,34,PD_TEXT);
        connection_note(!current?"Scheda cambiata: annulla e riapri":spell_notice[0]?spell_notice:spell_battle.mode==PD_SPELL_ATTACK?"Costo speso anche se manchi":"Costo ora; esito del TS al tavolo",333,18,PD_GOLD);
        button("Indietro",83,359,144,48,SC_BACK,!spell_attack_drawn);lv_obj_t *go=button(spell_attack_drawn?"Riprova":"Lancia e salva",239,359,144,48,SC_COMMIT,valid&&power_storage_ready());label_font(lv_obj_get_child(go,0),&lv_font_montserrat_16);return;
    }
    connection_note("PF bersaglio e difese non applicati",303,40,PD_MUTED);button("Magia",83,359,144,48,SC_DONE,true);button("Eroe",239,359,144,48,SC_HOME,true);
}
