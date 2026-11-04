/* View-only history: original saved indices and frozen pending components. */
static const lv_image_dsc_t *history_icon(unsigned kind){
    if(kind==PD_H_ATTACK||kind==PD_H_DAMAGE)return pd_ui_icon(PD_UI_ATTACK);
    if(kind==PD_H_HEAL||kind==PD_H_HIT_DIE)return pd_ui_icon(PD_UI_HEART);
    if(kind==PD_H_SAVE||kind==PD_H_CONCENTRATION)return pd_ui_icon(PD_UI_SHIELD);
    if(kind==PD_H_RECHARGE)return pd_ui_icon(PD_UI_USES);
    return pd_ui_icon(PD_UI_DICE);
}
static unsigned history_pending_parts(void){return history_fifth_pending?5:history_fourth_pending?4:history_third_pending?3:history_pair_pending?2:1;}
static void render_history(void){
    char text[160];
    if(page==HISTORY_DISCARD){
        label("Scarta dal registro?",83,111,300,true);
        status_tile(PD_UI_DICE,"Registro","Tiro non salvato",83,169);
        status_tile(PD_UI_HEART,"Personaggio","PF e usi invariati",239,169);
        wrapped_name("Perdi solo questa voce del registro. L'azione di gioco non viene annullata.",250,82,&lv_font_montserrat_16);
        button("Indietro",83,359,144,48,2607,true);button("Scarta",239,359,144,48,2608,true);return;
    }
    if(page==HISTORY_HOME){
        unsigned total=history_visible_count(),pages=total?(total+2)/3:1;
        if(history_page>=pages)history_page=pages-1;
        snprintf(text,sizeof text,"Storico %u/%u",history_page+1,pages);label(text,139,109,188,false);
        button("<",83,99,48,44,2601,history_page>0);button(">",335,99,48,44,2602,history_page+1<pages);
        snprintf(text,sizeof text,history_pending?"%u salvati | 1 in attesa":"%u salvati | ultime 20 voci",history.count);
        lv_obj_t *meta=small_label(text,139,136,188,history_pending?PD_GOLD:PD_MUTED);lv_obj_set_style_text_font(meta,&lv_font_montserrat_14,0);
        static const char *filters[]={"Tutti","d20","Danni","Recuperi"};
        for(unsigned i=0;i<4;i++){lv_obj_t *b=button(filters[i],83+78*i,159,66,44,2620+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_14,0);rules_selected_style(b,history_filter==i);}
        if(!total){wrapped_name(history.count?"Nessun tiro in questo filtro":"Nessun tiro registrato",240,60,&lv_font_montserrat_16);if(!history.count)small_label(pd_history_store_status(),83,310,300,pd_history_store_available()?PD_MUTED:PD_GOLD);}
        for(unsigned row=0;row<3;row++){
            unsigned index=history_page*3+row;if(index>=total)break;
            bool unsaved=history_pending&&!index;unsigned saved=unsaved?0:history_saved_at(index-(history_pending?1:0));
            const pd_history_entry *e=unsaved?&history_unsaved:&history.entries[saved];
            char name[100];
            if(unsaved){snprintf(name,sizeof name,"Da registrare: %.40s",e->label);snprintf(text,sizeof text,"1/%u | %d | solo in memoria",history_pending_parts(),e->total);}
            else{snprintf(name,sizeof name,"#%u %.40s = %d",(unsigned)e->sequence,e->label,e->total);snprintf(text,sizeof text,"Totale %d | #%u",e->total,(unsigned)e->sequence);}
            scene_control(name,history_icon(e->kind),e->label,text,83,211+row*48,300,44,unsaved?2609:2700+saved,true);
        }
        if(history_pending){
            const char *names[]={"Riprova","Scarta","Indietro"};const unsigned actions[]={2603,2606,2604};
            for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        }else button("Indietro",83,359,300,48,2604,true);
        return;
    }
    bool unsaved=history_index>=PD_HISTORY_LIMIT;
    if((unsaved&&(!history_pending||history_index>=PD_HISTORY_LIMIT+history_pending_parts()))||(!unsaved&&history_index>=history.count)){page=HISTORY_HOME;render_history();return;}
    const pd_history_entry *e=unsaved?(history_index==PD_HISTORY_LIMIT?&history_unsaved:history_index==PD_HISTORY_LIMIT+1?&history_unsaved_extra:history_index==PD_HISTORY_LIMIT+2?&history_unsaved_third:history_index==PD_HISTORY_LIMIT+3?&history_unsaved_fourth:&history_unsaved_fifth):&history.entries[history_index];
    unsigned pages=(e->count+5)/6;if(history_dice_page>=pages)history_dice_page=pages-1;
    if(unsaved)snprintf(text,sizeof text,"Da salvare %u/%u",history_index-PD_HISTORY_LIMIT+1,history_pending_parts());else snprintf(text,sizeof text,"Tiro #%u",(unsigned)e->sequence);
    label(text,139,111,188,false);button("<",83,99,48,44,2610,history_dice_page>0);button(">",335,99,48,44,2611,history_dice_page+1<pages);
    wrapped_name(e->label,145,48,&lv_font_montserrat_14);
    snprintf(text,sizeof text,"%d",e->total);lv_obj_t *total=label(text,83,196,unsaved?144:300,true);label_font(total,&pd_pixel_4);lv_obj_set_style_text_color(total,lv_color_hex(PD_GOLD),0);
    snprintf(text,sizeof text,"%ud%u %+d",e->count,e->sides,e->bonus);small_label(text,83,229,144,PD_TEXT);
    const char *kinds[]={"Libero","Prova","Abilita","Salvezza","Attacco","Danni","Cura","Dado vita","Morte","Concentrazione","Ricarica"};
    snprintf(text,sizeof text,"%s | %s",kinds[e->kind],e->mode>0?"vantaggio":e->mode<0?"svantaggio":e->sides==20&&e->count==1?"base":"somma");
    lv_obj_t *kind=small_label(text,83,249,300,PD_MUTED);lv_obj_set_style_text_font(kind,&lv_font_montserrat_14,0);
    for(unsigned row=0;row<6;row++){
        unsigned i=history_dice_page*6+row;if(i>=e->count)break;
        snprintf(text,sizeof text,"D%u: %u",i+1,e->dice[i]);small_label(text,83+(row%3)*104,267+(row/3)*21,92,PD_TEXT);
    }
    lv_obj_t *note=small_label(e->note,83,309,300,PD_MUTED);lv_obj_set_style_text_font(note,&lv_font_montserrat_14,0);lv_label_set_long_mode(note,LV_LABEL_LONG_WRAP);lv_obj_set_height(note,48);
    if(unsaved){lv_obj_t *hint=small_label("Riprova prima di spegnere",239,196,144,PD_GOLD);lv_obj_set_style_text_font(hint,&lv_font_montserrat_14,0);lv_label_set_long_mode(hint,LV_LABEL_LONG_WRAP);lv_obj_set_height(hint,34);}
    if(unsaved&&history_pair_pending){button("Elenco",83,359,92,48,2605,true);button(history_third_pending?"Parte >":history_index==PD_HISTORY_LIMIT?(strstr(history_unsaved_extra.label,"True Strike")?"Radiante":"Extra"):"Arma",187,359,92,48,6423,true);button("Riprova",291,359,92,48,2603,true);}
    else{button("Elenco",83,359,144,48,2605,true);lv_obj_t *next=button(unsaved?"Riprova":"Nuovo tiro",239,359,144,48,unsaved?2603:14,true);lv_obj_set_style_text_font(next,&lv_font_montserrat_16,0);}
}
