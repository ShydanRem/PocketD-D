/* Native draft controls only. No rule inference, RNG, storage or radio calls. */
static bool rules_editing(void){
    return page==REACTION_HOME||page==REACTION_CONFIRM||page==TIME_HOME||page==TIME_CONFIRM||page==TURN_DETAILS||page==TIME_DETAILS||page==EXTERNAL_TURN_HOME||page==TURN_NAME_EDIT||page==TURN_NAME_CONFIRM||page==DAMAGE_HOME||page==DAMAGE_PARTS||page==DAMAGE_PART_EDIT||page==DAMAGE_REDUCTION||page==DAMAGE_CONTEXT||page==DAMAGE_CONFIRM||page==DAMAGE_TYPES||page==DAMAGE_MATH||page==PAYMENT_CHOICE||page==PROFILE_LIST||page==PROFILE_VIEW||page==PROFILE_ALLOCATE||page==PROFILE_IMPACT||page==PROFILE_COST||page==PROFILE_COST_IMPACT||page==CONCENTRATION_SETUP||page==EFFECT_CONFIRM||page==DEFENCE_CONFIRM||page==DEFENCE_RULE_REVIEW||page==DEFENCE_GRANT_REVIEW||page==EFFECT_EDIT||page==EFFECT_DURATION||page==EFFECT_LINK||page==DEFENCE_EDIT||page==DEFENCE_TEMPORARY||page==DEFENCE_DURATION||page==DEFENCE_LINK||page==DEFENCE_RULE||page==PROFILE_DETAILS||page==PROFILE_NUMBER;
}
/* Read-only browsing gets local keys without becoming a modal game edit. */
static bool rules_navigating(void){return rules_editing()||(page==RESOURCES&&resource_picker)||page==WALLET_HOME||page==WALLET_EDIT||page==WALLET_CONFIRM||page==KEY_SETTINGS||page==KEY_PICKER||page==NUMBER_ENTRY||page==INPUT_CHECK_HOME||page==INPUT_CHECK_TOUCH||page==BACKUP_HOME||page==BACKUP_SEND_CONFIRM||page==BACKUP_RESTORE_CONFIRM||page==SYNC_HOME||page==SYNC_REVIEW||page==SYNC_FORGET||page==HISTORY_HOME||page==HISTORY_DETAIL||page==HISTORY_DISCARD||page==REFERENCE_HOME||page==REFERENCE_TEXT||page==ITEM_SEARCH||page==POWER_SEARCH;}
static bool rules_picker_open(void){return rules_panel&&rules_panel_owner==page&&rules_editing();}
static bool impact_menu_allowed(void){return (page==RESOURCES&&resource_picker)||((page==DAMAGE_HOME||page==DAMAGE_PARTS||page==REACTION_HOME||page==TIME_HOME)&&!rules_picker_open());}
static bool rules_duration_fields(bool **timed,uint8_t **tick,uint8_t **rounds,uint16_t **minutes,uint8_t **owner){
    if(page==EFFECT_DURATION){*timed=&effect_draft.timed;*tick=&effect_draft.tick;*rounds=&effect_draft.rounds;*minutes=&effect_draft.minutes;*owner=&effect_draft.owner;}
    else if(page==DEFENCE_DURATION){*timed=&defence_temporary_draft.timed;*tick=&defence_temporary_draft.tick;*rounds=&defence_temporary_draft.rounds;*minutes=&defence_temporary_draft.minutes;*owner=&defence_temporary_draft.owner;}
    else if(page==PROFILE_DETAILS||(page==PROFILE_NUMBER&&profile_number_rounds)){*timed=&profile_draft.timed;*tick=&profile_draft.tick;*rounds=&profile_draft.rounds;*minutes=&profile_draft.minutes;*owner=&profile_draft.owner;}
    else return false;
    return true;
}
static uint8_t *rules_selected(void){
    if(page==PROFILE_DETAILS)return &profile_draft.rule;
    if(page!=DEFENCE_RULE)return NULL;
    unsigned flags=defence_rule_temporary?defence_temporary_draft.flags:defence_flags;
    if(!(flags&(1u<<defence_rule_property)))return NULL;
    return (defence_rule_temporary?defence_temporary_draft.conditions:defence_conditions)+defence_rule_property;
}
static void rules_open(unsigned panel){
    if(panel==2&&!rules_selected())return;
    if(panel==1&&page!=EFFECT_DURATION&&page!=DEFENCE_DURATION&&page!=PROFILE_DETAILS)return;
    rules_panel=panel;rules_panel_owner=page;rules_focus=0;
    if(panel==2)rules_picker_page=*rules_selected()/3;
}
static void rules_picker_action(intptr_t action){
    if(rules_panel>=3){impact_picker_action(action);return;}
    if(action==5820){rules_panel=0;rules_focus=0;return;}
    if(rules_panel==1&&action>=5821&&action<=5826){
        bool *timed;uint8_t *tick,*rounds,*owner;uint16_t *minutes;
        if(!rules_duration_fields(&timed,&tick,&rounds,&minutes,&owner))return;
        unsigned mode=action-5821,current=*timed?*tick+1:0;
        /* Re-selecting a mode preserves the draft count and subject. */
        if(mode!=current){*timed=mode!=0;*tick=mode?mode-1:0;*owner=0;*rounds=*timed&&*tick<PD_TICK_MINUTES?1:0;*minutes=*timed&&*tick>=PD_TICK_MINUTES?(*tick==PD_TICK_HOURS?60:1):0;}
        rules_panel=0;rules_focus=0;
    }else if(rules_panel==2){
        if(action==5830&&rules_picker_page)rules_picker_page--;
        else if(action==5831&&(rules_picker_page+1)*3<PD_RULE_COUNT)rules_picker_page++;
        else if(action>=5840&&action<5840+PD_RULE_COUNT){uint8_t *rule=rules_selected();if(rule){*rule=action-5840;rules_panel=0;rules_focus=0;}}
    }
}
static unsigned rules_preset_value(unsigned tick,unsigned index){return tick==PD_TICK_HOURS?(const unsigned[]){60,480,1440}[index]:tick==PD_TICK_MINUTES?(const unsigned[]){1,10,60}[index]:(const unsigned[]){1,5,10}[index];}
static void rules_preset(unsigned index){
    if(index>=3)return;
    if(page!=EFFECT_DURATION&&page!=DEFENCE_DURATION&&!(page==PROFILE_NUMBER&&profile_number_rounds))return;
    bool *timed;uint8_t *tick,*rounds,*owner;uint16_t *minutes;
    if(!rules_duration_fields(&timed,&tick,&rounds,&minutes,&owner)||!*timed)return;
    unsigned value=rules_preset_value(*tick,index);if(*tick>=PD_TICK_MINUTES)*minutes=value;else *rounds=value;
}
static intptr_t rules_back(void){
    switch(page){case RESOURCES:return 5751;case WALLET_HOME:return 80;case WALLET_EDIT:return wallet_panel==1?2429:wallet_panel==2?2452:2409;case WALLET_CONFIRM:return 2412;case KEY_SETTINGS:return 863;case KEY_PICKER:return 4802;case NUMBER_ENTRY:return 3605;case INPUT_CHECK_HOME:return 2904;case INPUT_CHECK_TOUCH:return 2903;case BACKUP_HOME:return 1600;case BACKUP_SEND_CONFIRM:case BACKUP_RESTORE_CONFIRM:return connection_details?2810:2801;case SYNC_HOME:return connection_details?4707:4706;case SYNC_REVIEW:return 4701;case SYNC_FORGET:return 4704;case HISTORY_HOME:return 2604;case HISTORY_DETAIL:return 2605;case HISTORY_DISCARD:return 2607;case REFERENCE_HOME:return 4601;case REFERENCE_TEXT:return 4604;case ITEM_SEARCH:case POWER_SEARCH:return 3805;case REACTION_HOME:return 2118;case REACTION_CONFIRM:return 2121;case TIME_HOME:return 2141;case TIME_CONFIRM:return 2147;case TURN_DETAILS:return 2125;case TIME_DETAILS:return 2150;case EXTERNAL_TURN_HOME:return 3206;case TURN_NAME_EDIT:return 3304;case TURN_NAME_CONFIRM:return 3306;case DAMAGE_HOME:return 1500;case DAMAGE_PARTS:return 1904;case DAMAGE_PART_EDIT:return 1926;case DAMAGE_REDUCTION:return 2047;case DAMAGE_CONTEXT:return 2022;case DAMAGE_CONFIRM:return 1804;case DAMAGE_TYPES:return 1931;case DAMAGE_MATH:return 2051;case PAYMENT_CHOICE:return 3501;case PROFILE_LIST:return 2064;case PROFILE_VIEW:return 2066;case PROFILE_ALLOCATE:return 2085;case PROFILE_IMPACT:return 2091;case PROFILE_COST:return 2104;case PROFILE_COST_IMPACT:return 2106;case CONCENTRATION_SETUP:return 1861;case EFFECT_CONFIRM:return 1708;case DEFENCE_CONFIRM:return 1816;case DEFENCE_RULE_REVIEW:return 2014;case DEFENCE_GRANT_REVIEW:return 2035;case EFFECT_EDIT:return 1700;case EFFECT_DURATION:return 1716;case EFFECT_LINK:return 1752;case DEFENCE_EDIT:return 1814;case DEFENCE_TEMPORARY:return 1875;case DEFENCE_DURATION:return 1879;case DEFENCE_LINK:return 1885;case DEFENCE_RULE:return 2002;case PROFILE_DETAILS:return 2080;case PROFILE_NUMBER:return 2085;default:return 0;}
}
static void rules_mark_focus(void){
    if(!rules_target_count)return;
    if(rules_focus>=rules_target_count)rules_focus=0;
    for(unsigned i=0;i<rules_target_count&&lv_obj_has_state(rules_targets[rules_focus],LV_STATE_DISABLED);i++)rules_focus=(rules_focus+1)%rules_target_count;
    for(unsigned i=0;i<rules_target_count;i++){lv_obj_set_style_outline_color(rules_targets[i],lv_color_hex(PD_GOLD),0);lv_obj_set_style_outline_pad(rules_targets[i],2,0);lv_obj_set_style_outline_width(rules_targets[i],rules_keyboard&&rules_focus==i?2:0,0);}
}
static void rules_focus_refresh(void){
    rules_target_count=0;
    if(!rules_navigating()){rules_keyboard=false;return;}
    if(rules_focus_owner!=page){rules_focus_owner=page;rules_focus=0;}
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){lv_obj_t *b=lv_obj_get_child(root,i);if(lv_obj_check_type(b,&lv_button_class)&&lv_obj_get_y(b)>=(page==INPUT_CHECK_TOUCH?350:99)&&rules_target_count<sizeof rules_targets/sizeof rules_targets[0])rules_targets[rules_target_count++]=b;}
    rules_mark_focus();
}
static void rules_selected_style(lv_obj_t *b,bool selected){if(selected){lv_obj_set_style_border_color(b,lv_color_hex(PD_MINT),0);lv_obj_set_style_border_width(b,2,0);lv_obj_set_style_bg_color(b,lv_color_hex(0x244d48),0);}}
static void render_rules_picker(void){
    if(rules_panel>=3){render_impact_picker();return;}
    char text[80];
    if(rules_panel==1){
        label("Quando finisce?",83,105,300,false);
        bool *timed;uint8_t *tick,*rounds,*owner;uint16_t *minutes;if(!rules_duration_fields(&timed,&tick,&rounds,&minutes,&owner))return;
        static const char *names[]={"Manuale","Round","Inizio turno","Fine turno","Minuti","Ore"};
        static const char *captions[]={"Manuale","Round","Inizio","Fine","Minuti","Ore"};
        static const char *badges[]={"Nessuno","A passi","Del turno","Del turno","Tempo","Tempo"};
        unsigned mode=*timed?*tick+1:0;
        for(unsigned i=0;i<6;i++){lv_obj_t *b=scene_control(names[i],&theme_time,captions[i],badges[i],83+(i%2)*156,151+(i/2)*64,144,56,5821+i,true);rules_selected_style(b,mode==i);}
        button("Indietro",83,359,300,48,5820,true);
    }else{
        label("Scegli regola",83,105,300,false);snprintf(text,sizeof text,"%u / %u",rules_picker_page+1,(PD_RULE_COUNT+2)/3);small_label(text,83,135,300,PD_MUTED);
        uint8_t *selected=rules_selected();
        for(unsigned row=0;row<3;row++){unsigned i=rules_picker_page*3+row;if(i>=PD_RULE_COUNT)break;lv_obj_t *b=scene_control(rule_names[i],pd_ui_icon(PD_UI_SHIELD),i==6?"No argento/adamantio":rule_names[i],i>=3?"Solo attacchi non magici":rule_descriptions[i],83,159+row*60,300,54,5840+i,selected!=NULL);rules_selected_style(b,selected&&*selected==i);}
        button("<",83,359,60,48,5830,rules_picker_page>0);button("Indietro",155,359,156,48,5820,true);button(">",323,359,60,48,5831,(rules_picker_page+1)*3<PD_RULE_COUNT);
    }
}
static void render_rules_presets(bool timed,unsigned tick,unsigned value,int y){
    char text[32];for(unsigned i=0;i<3;i++){unsigned n=rules_preset_value(tick,i);if(tick==PD_TICK_HOURS)snprintf(text,sizeof text,"%u h",n/60);else if(tick==PD_TICK_MINUTES)snprintf(text,sizeof text,"%u min",n);else snprintf(text,sizeof text,"%u %s",n,turn_clock(tick)?"pass.":"round");lv_obj_t *b=button(text,83+i*104,y,92,44,5810+i,timed);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);rules_selected_style(b,timed&&value==n);}
}
static void render_rules_duration(bool defence){
    bool timed=defence?defence_temporary_draft.timed:effect_draft.timed;
    unsigned tick=defence?defence_temporary_draft.tick:effect_draft.tick,rounds=defence?defence_temporary_draft.rounds:effect_draft.rounds,minutes=defence?defence_temporary_draft.minutes:effect_draft.minutes,owner=defence?defence_temporary_draft.owner:effect_draft.owner;
    unsigned value=duration_left(tick,rounds,minutes);char text[96];
    snprintf(text,sizeof text,"Durata: %s >",duration_owned(timed,tick,owner));lv_obj_t *type=scene_control(text,&theme_time,duration_owned(timed,tick,owner),"Scegli tipo",83,99,300,48,defence?1877:1711,true);(void)type;
    if(!timed)strcpy(text,"Senza scadenza");else if(tick==PD_TICK_HOURS)time_text(text,sizeof text,minutes);else if(tick==PD_TICK_MINUTES)snprintf(text,sizeof text,"%u min",minutes);else snprintf(text,sizeof text,"%u %s",rounds,turn_clock(tick)?"passaggi":"round");label(text,83,167,300,true);
    const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(tick==PD_TICK_HOURS?(const char *const[]){"-1h","-1m","+1m","+1h"}[i]:names[i],83+i*78,211,66,44,(defence?1880:1712)+i,timed&&(i<2?value>0:value<(tick==PD_TICK_HOURS?5940:99)));
    render_rules_presets(timed,tick,value,263);
    if(timed&&turn_clock(tick)){snprintf(text,sizeof text,"Turno: %s >",turn_subject(owner));button(text,83,311,300,44,defence?3211:3210,true);}
    else{small_label(timed?(tick>=PD_TICK_MINUTES?"Tempo dal menu Stato":"Round dal menu Effetti"):"Scegli il tipo per attivare il timer",83,317,300,PD_GOLD);small_label("In pausa se disattivato",83,339,300,PD_MUTED);}
    const char *names2[]={"Indietro","Vincolo","Anteprima"};const unsigned codes[]={defence?1879:1716,defence?1878:1750,defence?1887:1704};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names2[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}
static void render_defence_choices(bool temporary){
    unsigned flags=temporary?defence_temporary_draft.flags:defence_flags;bool known=temporary?defence_temporary_draft.active:defence_configured;char text[80];
    static const char *names[]={"Resistenza","Vulnerabilita","Immunita","Configurato"};
    static const char *captions[]={"Resiste","Debole","Immune","Base"};
    static const char *badges[]={"1/2 danni","x2 danni","0 danni"};
    const uint8_t *conditions=temporary?defence_temporary_draft.conditions:defence_conditions;
    for(unsigned i=0;i<4;i++){bool on=i==3?known:(flags&(1u<<i))!=0;snprintf(text,sizeof text,"%s: %s",i==3&&temporary?"Attiva":names[i],on?"si":"no");lv_obj_t *b=scene_control(text,pd_ui_icon(i==0?PD_UI_RESIST:i==1?PD_UI_WEAK:i==2?PD_UI_IMMUNE:PD_UI_READY),i==3&&temporary?"Stato":captions[i],i==3?(temporary?(known?"Attiva":"Inattiva"):(known?"Verificata":"Da vedere")):on?(conditions[i]?"Con regola":badges[i]):"Assente",83+(i%2)*156,173+(i/2)*72,144,64,(temporary?1871:1810)+i,true);rules_selected_style(b,on);}
    const uint8_t *rules=temporary?defence_temporary_draft.conditions:defence_conditions;bool conditional=rules[0]||rules[1]||rules[2];
    small_label(conditional?"Condizioni: consulta Regole":temporary?"Extra separato dalla base":"Bozza: applica solo con Conferma",83,325,300,conditional?PD_GOLD:PD_MUTED);
}

static void rules_defence_delta(pd_ui_icon_id icon,const char *name,bool before,bool after,int x,int y){
    lv_obj_t *tile=transaction_tile(icon,name,x,y);char text[32];snprintf(text,sizeof text,"%s > %s",before?"Si":"No",after?"Si":"No");tile_label(tile,text,6,31,132,&lv_font_montserrat_16,before==after?PD_TEXT:icon==PD_UI_READY&&!after?PD_DANGER:PD_GOLD);
}
