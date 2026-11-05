static void formula_text(char *text,size_t size,pd_spell_formula f){
    if(!f.kind){snprintf(text,size,"Nessuna formula");return;}
    const pd_power *p=&spell_origin.magic.powers[spell_plan.power];
    snprintf(text,size,"%ud%u%s%s%+d",f.count,f.sides,f.ability?" + ":"",f.ability?pd_abilities[p->ability]:"",f.bonus);
}
static void formula_scaling_text(char *text,size_t size,pd_spell_formula f){
    if(!f.scaling)snprintf(text,size,"Formula fissa");
    else snprintf(text,size,"+%ud%u / %s",f.step,f.sides,f.scaling==1?"livello slot":"5,11,17 PG");
}
static void render_formula_config(void){
    char text[140];const pd_power *p=&spell_origin.magic.powers[spell_plan.power];bool current=spell_current();
    lv_obj_t *name=wrapped_name(p->name,99,48,&lv_font_montserrat_14);lv_obj_set_style_text_color(name,lv_color_hex(PD_GOLD),0);
    if(page==SPELL_FORMULA_SAVE){
        small_label("Salvare nella scheda?",83,151,300,PD_TEXT);
        formula_text(text,sizeof text,spell_origin.formulas[spell_plan.power]);small_label(text,83,195,300,PD_MUTED);
        formula_text(text,sizeof text,formula_draft);small_label(text,83,231,300,PD_GOLD);
        snprintf(text,sizeof text,"%s",!formula_draft.kind?"Rimuovi formula":formula_draft.kind==1?"Danni; bersaglio al tavolo":"Cura; bersaglio scelto al lancio");small_label(text,83,267,300,PD_TEXT);
        formula_scaling_text(text,sizeof text,formula_draft);small_label(text,83,299,300,PD_TEXT);
        small_label(current?(spell_notice[0]?spell_notice:"Nessuno slot speso, nessun tiro"):"Scheda cambiata: annulla e riapri",83,333,300,PD_GOLD);
        button("Modifica",83,359,144,48,SF_EDIT,true);button("Salva",239,359,144,48,SF_SAVE,current&&pd_storage_writable());return;
    }
    if(!formula_panel){
        small_label(formula_draft.source?"Base Beyond: dati strutturati":"Profilo manuale",83,151,300,PD_MUTED);
        const char *names[]={"Senza","Danni","Cura"};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+104*i,185,92,44,SF_KIND+i,current);rules_selected_style(b,formula_draft.kind==i);}
        formula_text(text,sizeof text,formula_draft);button(text,83,247,300,44,SF_VALUES,current&&formula_draft.kind);
        formula_scaling_text(text,sizeof text,formula_draft);button(text,83,299,300,44,SF_SCALING,current&&formula_draft.kind);
        button("Indietro",83,359,144,48,SF_BACK,true);button("Rivedi",239,359,144,48,SF_REVIEW,current);
    }else if(formula_panel==1){
        formula_text(text,sizeof text,formula_draft);label(text,83,149,300,true);
        snprintf(text,sizeof text,"Dado d%u",formula_draft.sides);button(text,83,195,144,44,SF_DIE,current);
        snprintf(text,sizeof text,"%s %s",pd_abilities[p->ability],formula_draft.ability?"si":"no");lv_obj_t *b=button(text,239,195,144,44,SF_ABILITY,current);rules_selected_style(b,formula_draft.ability);
        button("Dadi -",83,247,144,44,SF_FEWER,current&&formula_draft.count>1);button("Dadi +",239,247,144,44,SF_MORE,current&&formula_draft.count<18);
        button("Extra -",83,299,144,44,SF_MINUS,current&&formula_draft.bonus> -15);button("Extra +",239,299,144,44,SF_PLUS,current&&formula_draft.bonus<60);
        button("Indietro",83,359,300,48,SF_BACK,true);
    }else if(formula_panel==2){
        const char *names[]={"Fissa","Per livello slot","Livelli PG 5 / 11 / 17"};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83,151+52*i,300,44,SF_FIXED+i,current&&(!i||(i==1?!!p->level:!p->level)));rules_selected_style(b,formula_draft.scaling==i);}
        snprintf(text,sizeof text,"Passo %u -",formula_draft.step);button(text,83,307,144,44,SF_STEP_LESS,current&&formula_draft.scaling&&formula_draft.step>1);
        snprintf(text,sizeof text,"Passo %u +",formula_draft.step);button(text,239,307,144,44,SF_STEP_MORE,current&&formula_draft.scaling&&formula_draft.step<18);
        button("Indietro",83,363,300,44,SF_BACK,true);
    }else{
        small_label("Scegli il dado",83,151,300,PD_TEXT);
        for(unsigned i=0;i<6;i++){unsigned sides=(const unsigned[]){4,6,8,10,12,20}[i];snprintf(text,sizeof text,"d%u",sides);lv_obj_t *b=button(text,83+(i%2)*156,187+(i/2)*52,144,44,SF_D4+i,current);rules_selected_style(b,formula_draft.sides==sides);}
        button("Indietro",83,359,300,48,SF_BACK,true);
    }
}
