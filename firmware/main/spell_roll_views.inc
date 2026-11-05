static void render_spell_roll(void){
    char text[180];const pd_power *p=&spell_origin.magic.powers[spell_plan.power];
    bool current=spell_current(),valid=false;
    if(page==SPELL_REVIEW&&!spell_rolled)valid=current&&pd_spell_roll_prepare(&state,&spell_plan,&spell_candidate);
    if(page==SPELL_RECEIPT||spell_rolled)valid=true;
    if(page==SPELL_METHOD){
        transaction_header(pd_power_icon(p),p->name);small_label(state.formulas[spell_plan.power].kind?"Formula salvata disponibile":"Come vuoi risolverla?",83,174,300,PD_TEXT);
        const char *names[]={"Solo costo","Danni","Cura me","Cura altri"};
        const char *badges[]={"Al tavolo","Da tirare","Ai miei PF","Solo tiro"};
        const pd_ui_icon_id icons[]={PD_UI_MAGIC,PD_UI_ATTACK,PD_UI_HEART,PD_UI_HEART};
        for(unsigned i=0;i<4;i++)scene_control(names[i],pd_ui_icon(icons[i]),names[i],badges[i],83+(i%2)*156,195+(i/2)*72,144,64,SR_COST+i,current&&(!i||!spell_notice[0]||state.formulas[spell_plan.power].kind!=(i==1?1:2))&&(i!=2||state.hp<pd_max_hp(&state)));
        button("Indietro",83,359,300,48,SR_BACK,true);
    }else if(page==SPELL_FORMULA){
        lv_obj_t *name=wrapped_name(p->name,99,48,&lv_font_montserrat_14);lv_obj_set_style_text_color(name,lv_color_hex(PD_GOLD),0);
        if(spell_dice_picker){
            small_label("Scegli il dado",83,151,300,PD_TEXT);
            for(unsigned i=0;i<6;i++){unsigned sides=(const unsigned[]){4,6,8,10,12,20}[i];snprintf(text,sizeof text,"d%u",sides);lv_obj_t *b=button(text,83+(i%2)*156,187+(i/2)*52,144,44,SR_D4+i,true);rules_selected_style(b,spell_plan.sides==sides);}
            button("Indietro",83,359,300,48,SR_DIE_BACK,true);
        }else{
            snprintf(text,sizeof text,"%ud%u%+d",spell_plan.count,spell_plan.sides,spell_plan.bonus);label(current?text:"Scheda cambiata",83,149,300,true);
            snprintf(text,sizeof text,"Dado d%u",spell_plan.sides);button(text,83,195,144,44,SR_DIE,current);
            snprintf(text,sizeof text,"%s %+d",pd_abilities[p->ability],pd_modifier(state.abilities[p->ability]));lv_obj_t *b=button(text,239,195,144,44,SR_ABILITY,current);pd_ui_name(b,"Bonus caratteristica");
            button("Dadi -",83,247,144,44,SR_FEWER,current&&spell_plan.count>1);button("Dadi +",239,247,144,44,SR_MORE,current&&spell_plan.count<PD_POOL_MAX);
            button("Bonus -",83,299,144,44,SR_MINUS,current&&spell_plan.bonus>PD_POOL_MIN_BONUS);button("Bonus +",239,299,144,44,SR_PLUS,current&&spell_plan.bonus<PD_POOL_MAX_BONUS);
            button("Indietro",83,359,144,48,SR_BACK,true);button("Rivedi",239,359,144,48,SR_REVIEW,current);
        }
    }else{
        bool receipt=page==SPELL_RECEIPT;transaction_header(pd_power_icon(p),p->name);
        snprintf(text,sizeof text,"%s | %ud%u%+d",spell_kind(),spell_plan.count,spell_plan.sides,spell_plan.bonus);lv_obj_t *formula=small_label(text,83,174,300,PD_GOLD);label_font(formula,&lv_font_montserrat_14);
        if(spell_details){
            snprintf(text,sizeof text,"CA %u > %u | max PF %u > %u",pd_armor(&spell_origin),pd_armor(&spell_candidate),pd_max_hp(&spell_origin),pd_max_hp(&spell_candidate));small_label(text,83,207,300,PD_TEXT);
            snprintf(text,sizeof text,"Concentrazione: %s",spell_origin.magic.concentration==spell_candidate.magic.concentration?"invariata":!spell_candidate.magic.concentration?"termina":spell_origin.magic.concentration?"sostituisce":"inizia");small_label(text,83,245,300,PD_GOLD);
            connection_note("Formula base: verifica slot ed edizione. TS, critici ed effetti speciali al tavolo.",279,52,PD_MUTED);
        }else{
            bool slot=p->level&&(p->flags&PD_USES_SLOT)&&spell_plan.slot>=1&&spell_plan.slot<=9;
            if(slot){snprintf(text,sizeof text,"Slot %u %s",spell_plan.slot,spell_plan.pact?"patto":"ord.");delta_tile(PD_UI_MAGIC,text,spell_plan.pact?spell_origin.magic.pact[spell_plan.slot-1].current:spell_origin.magic.slots[spell_plan.slot-1].current,spell_plan.pact?spell_candidate.magic.pact[spell_plan.slot-1].current:spell_candidate.magic.slots[spell_plan.slot-1].current,83,199);}
            else status_tile(PD_UI_MAGIC,"Slot","Nessun costo",83,199);
            if(p->flags&PD_LIMITED)delta_tile(PD_UI_USES,"Utilizzi",p->remaining,spell_candidate.magic.powers[spell_plan.power].remaining,239,199);else status_tile(PD_UI_USES,"Utilizzi","Non richiesti",239,199);
            if(receipt){delta_tile(PD_UI_HEART,"PF",spell_origin.hp,spell_candidate.hp,83,267);snprintf(text,sizeof text,"%d",spell_result.total);status_tile(spell_plan.kind==PD_SPELL_DAMAGE?PD_UI_ATTACK:PD_UI_HEART,"Tiro",text,239,267);}
            else {snprintf(text,sizeof text,"%u/%u",spell_candidate.hp,pd_max_hp(&spell_candidate));status_tile(PD_UI_HEART,"Prima del tiro",valid?text:"Non disponibile",83,267);status_tile(PD_UI_HEART,"Destinazione",spell_plan.kind==PD_SPELL_HEAL_SELF?"I miei PF":"Altro bersaglio",239,267);}
        }
        const char *notice=!current&&!receipt?"Scheda cambiata: annulla e riapri":spell_notice[0]?spell_notice:receipt?"Costo salvato; tiro fissato":!valid?"Verifica PF, formula e costi":spell_from_formula?(spell_origin.formulas[spell_plan.power].source?"Base Beyond; effetti speciali al tavolo":"Formula salvata; effetti al tavolo"):"Formula di questo lancio";
        lv_obj_t *warning=small_label(notice,83,333,300,spell_notice[0]||(!valid&&!receipt)?PD_DANGER:PD_GOLD);label_font(warning,&lv_font_montserrat_14);
        lv_obj_t *a=button(receipt?"Magia":"Modifica",83,359,92,48,receipt?SR_DONE:SR_EDIT,receipt||!spell_rolled),*b=button(spell_details?"Riepilogo":"Dettagli",187,359,92,48,SR_DETAILS,!spell_rolled),*c=button(receipt?"Fine":spell_rolled?"Riprova":"Tira e salva",291,359,92,48,receipt?SR_HOME:SR_COMMIT,receipt||(valid&&power_storage_ready()));
        label_font(lv_obj_get_child(a,0),&lv_font_montserrat_16);label_font(lv_obj_get_child(b,0),&lv_font_montserrat_16);label_font(lv_obj_get_child(c,0),&lv_font_montserrat_14);
    }
    if(page==SPELL_METHOD){lv_obj_t *notice=small_label(current?spell_notice:"Scheda cambiata: annulla e riapri",83,337,300,PD_DANGER);label_font(notice,&lv_font_montserrat_14);}
}
