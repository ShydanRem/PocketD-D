/* Presentation of explicit game-time/turn commands; never a real-time clock. */
static void clock_reaction_tile(bool before,bool after,int x,int y){
    lv_obj_t *tile=transaction_tile(PD_UI_REACTION,"Reazione",x,y);char text[48];snprintf(text,sizeof text,"%s > %s",before?"Usata":"Pronta",after?"Usata":"Pronta");tile_label(tile,text,6,31,132,&lv_font_montserrat_16,before&&!after?PD_MINT:before==after?PD_TEXT:PD_GOLD);
}
static void clock_summary(bool time){
    pd_round_summary r={0};bool valid=time?preview_time(&scratch,&r):preview_turn(&scratch,&r);char text[128];
    if(time){label("Avanzare il tempo?",83,99,300,false);time_text(text,sizeof text,time_minutes);label(text,83,143,300,true);}
    else{if(turn_owner)snprintf(text,sizeof text,"%s turno %s",turn_end?"Fine":"Inizio",turn_subject(turn_owner));else snprintf(text,sizeof text,"%s il tuo turno",turn_end?"Termina":"Inizia");label(text,83,99,300,false);if(turn_owner)wrapped_name(state.turn_names[turn_owner-1],135,40,&lv_font_montserrat_16);else small_label("Timer del tuo turno",83,145,300,PD_GOLD);}
    if(valid){
        delta_tile(PD_UI_SHIELD,"CA",pd_armor(&state),pd_armor(&scratch),83,183);delta_tile(PD_UI_HEART,"PF max",pd_max_hp(&state),pd_max_hp(&scratch),239,183);
        delta_tile(PD_UI_HEART,"PF",state.hp,scratch.hp,83,251);clock_reaction_tile(state.reaction_spent,scratch.reaction_spent,239,251);
        snprintf(text,sizeof text,"Avanzano %u | scadono %u",bit_count(r.effects_advanced)+bit_count(r.defences_advanced),bit_count(r.effects_expired)+bit_count(r.defences_expired));small_label(text,83,315,300,PD_GOLD);
        lv_obj_t *note=small_label(state.hp>scratch.hp?"PF oltre il nuovo massimo: persi":time||turn_owner?"Reazione e risorse invariate":"Nessuna cura o risorsa recuperata",83,337,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);label_font(note,&lv_font_montserrat_14);
    }else{label("Scadenze fuori limite",83,211,300,false);small_label("Durate e reazione invariate",83,259,300,PD_DANGER);small_label("Controlla gli effetti locali",83,303,300,PD_MUTED);}
    const char *names[]={"Indietro","Scadenze","Conferma"};const unsigned actions[]={time?2147:2121,time?2149:2124,time?2148:2122};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i==0||(valid&&(i==2||r.effects_advanced||r.defences_advanced)));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}
static void render_turn_clock(void){
    char text[96];
    if(page==REACTION_HOME){
        label("Turni e reazione",83,99,300,true);small_label(state.reaction_spent?"Reazione utilizzata":"Reazione disponibile",83,143,300,state.reaction_spent?PD_MUTED:PD_MINT);
        scene_control("Usa reazione",pd_ui_icon(PD_UI_REACTION),"Reazione",state.reaction_spent?"Usata":"Pronta",83,179,144,64,2119,!state.reaction_spent);
        scene_control("Inizio turno",pd_ui_icon(PD_UI_TURN_START),"Inizio","Mio turno",239,179,144,64,2120,true);
        scene_control("Fine turno",pd_ui_icon(PD_UI_TURN_END),"Fine","Mio turno",83,255,144,64,2123,true);
        scene_control("Altri turni",pd_ui_icon(PD_UI_SUBJECTS),"Altri","A / B / C",239,255,144,64,3200,true);
        small_label("Round e turni restano separati",83,331,300,PD_MUTED);button("Indietro",83,359,300,48,2118,true);
    }else if(page==TIME_HOME){
        label("Tempo trascorso",83,99,300,true);time_text(text,sizeof text,time_minutes);label(text,83,145,300,true);
        for(unsigned i=0;i<6;i++){unsigned n=clock_presets[i];if(n<60)snprintf(text,sizeof text,"%u min",n);else snprintf(text,sizeof text,"%u h",n/60);lv_obj_t *b=button(text,83+(i%3)*104,187+(i/3)*52,92,44,7400+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);rules_selected_style(b,time_minutes==n);}
        for(unsigned i=0;i<4;i++)button((const char *const[]){"-1h","-1m","+1m","+1h"}[i],83+i*78,295,66,44,2142+i,i<2?time_minutes>1:time_minutes<5940);
        button("Indietro",83,359,144,48,2141,true);button("Anteprima",239,359,144,48,2146,true);
    }else if(page==TIME_CONFIRM)clock_summary(true);
    else if(page==REACTION_CONFIRM){
        if(!reaction_draft){clock_summary(false);return;}
        transaction_header(pd_ui_icon(PD_UI_REACTION),"Usa la reazione");
        lv_obj_t *image=art(pd_ui_icon(PD_UI_REACTION),209,177,lv_screen_active());lv_image_set_pivot(image,0,0);lv_image_set_scale(image,512);lv_image_set_antialias(image,false);
        label("Pronta > Usata",83,245,300,true);small_label("Solo la reazione",83,295,300,PD_GOLD);small_label("Turni e timer invariati",83,325,300,PD_MUTED);
        button("Indietro",83,359,144,48,2121,true);button("Conferma",239,359,144,48,2122,true);
    }
}
static void render_turn_subjects(void){
    char text[112];label("Altri turni",83,99,300,true);
    for(unsigned i=1;i<PD_TURN_SUBJECTS;i++){lv_obj_t *b=button(turn_subject(i),83+(i-1)*104,151,92,44,3200+i,true);rules_selected_style(b,i==turn_owner);}
    snprintf(text,sizeof text,"%s: %s >",turn_subject(turn_owner),state.turn_names[turn_owner-1][0]?state.turn_names[turn_owner-1]:"senza nome");lv_obj_t *name=button(text,83,203,300,64,3300,true);lv_obj_t *caption=lv_obj_get_child(name,0);emblem_name(caption,text,284);lv_point_t extent;lv_text_get_size(&extent,text,lv_obj_get_style_text_font(caption,0),0,0,284,LV_TEXT_FLAG_NONE);lv_obj_set_height(caption,extent.y);lv_obj_center(caption);
    for(unsigned i=0;i<2;i++){pd_round_summary r={0};scratch=state;bool valid=pd_turn_advance_subject(&scratch,turn_owner,i==0,&r);char badge[40];if(valid)snprintf(badge,sizeof badge,"%u timer",bit_count(r.effects_advanced)+bit_count(r.defences_advanced));else strcpy(badge,"Da verificare");snprintf(text,sizeof text,"%s turno %s",i?"Fine":"Inizio",turn_subject(turn_owner));scene_control(text,pd_ui_icon(i?PD_UI_TURN_END:PD_UI_TURN_START),i?"Fine":"Inizio",badge,83+i*156,283,144,64,3204+i,true);}
    button("Indietro",83,359,300,48,3206,true);
}
static void clock_detail_value(char *text,size_t size,bool time,unsigned tick,unsigned value){
    if(!time)snprintf(text,size,"%u pass.",value);else if(tick==PD_TICK_HOURS)time_text(text,size,value);else snprintf(text,size,"%u min",value);
}
static void render_clock_details(void){
    bool time=page==TIME_DETAILS;unsigned *detail=time?&time_detail_page:&turn_detail_page;char text[128];pd_round_summary r={0};
    if(!(time?preview_time(&scratch,&r):preview_turn(&scratch,&r))){label("Scadenze fuori limite",83,211,300,false);button("Riepilogo",83,359,300,48,time?2150:2125,true);return;}
    unsigned indices[PD_EFFECTS+PD_TIMED_DEFENCES],count=0;for(unsigned i=0;i<PD_EFFECTS;i++)if(r.effects_advanced&(1u<<i))indices[count++]=i;for(unsigned i=0;i<PD_TIMED_DEFENCES;i++)if(r.defences_advanced&(UINT64_C(1)<<i))indices[count++]=PD_EFFECTS+i;
    unsigned pages=count?(count+2)/3:1;if(*detail>=pages)*detail=pages-1;if(time)strcpy(text,"Scadenze del tempo");else if(turn_owner)snprintf(text,sizeof text,"%s %s: scadenze",turn_end?"Fine":"Inizio",turn_subject(turn_owner));else snprintf(text,sizeof text,"Scadenze %s turno",turn_end?"fine":"inizio");label(text,83,99,300,false);
    snprintf(text,sizeof text,"%s | %u/%u",time?"Tempo":"Passaggi",*detail+1,pages);small_label(text,83,133,300,PD_GOLD);
    for(unsigned row=0;row<3;row++){
        unsigned at=*detail*3+row;if(at>=count)break;unsigned bit=indices[at],before,after,tick;char name[80],a[32],b[32];const lv_image_dsc_t *icon;
        if(bit<PD_EFFECTS){const pd_effect *x=&state.effects[bit],*y=&scratch.effects[bit];before=time?x->minutes:x->rounds;after=time?y->minutes:y->rounds;tick=x->tick;snprintf(name,sizeof name,"%s #%u",x->target==PD_EFFECT_ARMOR?"CA":"PF max",bit+1);icon=pd_ui_icon(x->target==PD_EFFECT_ARMOR?PD_UI_SHIELD:PD_UI_HEART);}
        else if((bit-=PD_EFFECTS)<PD_DEFENCE_GRANTS){unsigned type=bit%PD_DAMAGE_TYPES,slot=bit/PD_DAMAGE_TYPES;const pd_defence_temporary *x=pd_defence_grant(&state,type,slot),*y=pd_defence_grant(&scratch,type,slot);before=time?x->minutes:x->rounds;after=time?y->minutes:y->rounds;tick=x->tick;snprintf(name,sizeof name,"%s #%u",pd_damage_name(type),slot+1);icon=pd_damage_icon(type);}
        else{unsigned i=bit-PD_DEFENCE_GRANTS;const pd_reduction_profile *x=&state.reductions[i],*y=&scratch.reductions[i];before=time?x->minutes:x->rounds;after=time?y->minutes:y->rounds;tick=x->tick;snprintf(name,sizeof name,"Riduzione #%u",i+1);icon=pd_ui_icon(PD_UI_RESIST);}
        clock_detail_value(a,sizeof a,time,tick,before);clock_detail_value(b,sizeof b,time,tick,after);snprintf(text,sizeof text,"%s > %s%s",a,b,after?"":" | scade");lv_obj_t *card=scene_control(name,icon,name,text,83,159+row*60,300,54,0,false);lv_obj_set_style_text_color(lv_obj_get_child(card,2),lv_color_hex(PD_TEXT),0);lv_obj_set_style_text_color(lv_obj_get_child(card,3),lv_color_hex(after?PD_MUTED:PD_DANGER),0);
    }
    button("<",83,359,60,48,time?2151:2126,*detail>0);button("Riepilogo",155,359,156,48,time?2150:2125,true);button(">",323,359,60,48,time?2152:2127,*detail+1<pages);
}
