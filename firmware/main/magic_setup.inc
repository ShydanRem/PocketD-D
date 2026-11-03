/* Shared slot picker and batched preparation. Drafts never spend resources. */
enum {MC_BACK=6700,MC_ORDINARY,MC_PACT,MC_LEVEL=6710,
      MP_OPEN=6800,MP_BACK,MP_PREV,MP_NEXT,MP_ALL,MP_READY,MP_CHANGED,MP_REVIEW,MP_SAVE,MP_EDIT,MP_TOGGLE=6820};
static bool magic_cost_smite,magic_cost_pact;
static unsigned magic_cost_power;
static uint8_t preparation_draft[PD_POWERS];
static unsigned preparation_page,preparation_filter;
static uint32_t preparation_origin,preparation_epoch;
static const char *preparation_notice="";
static bool magic_setup_open(void){return page==MAGIC_COST_PICKER||page==MAGIC_PREPARE_LIST||page==MAGIC_PREPARE_CONFIRM;}
static void slot_picker_begin(bool smite){
    unsigned power=smite?smite_source:power_index;pd_magic_cost costs[18];
    if(!pd_magic_costs(&state,power,costs))return;
    magic_cost_smite=smite;magic_cost_power=power;magic_cost_pact=smite?smite_pact:cast_pact;
    page=MAGIC_COST_PICKER;wifi_focus=0;wifi_keyboard=false;
}
static bool magic_cost_available(unsigned level,bool pact){
    pd_magic_cost costs[18];unsigned count=pd_magic_costs(&state,magic_cost_power,costs);
    for(unsigned i=0;i<count;i++)if(costs[i].level==level&&costs[i].pact==pact)
        return !magic_cost_smite||pd_smite_prepare(&state,&strike,magic_cost_power,level,pact,false,NULL,NULL);
    return false;
}
static bool preparation_current(void){return state.history_epoch==preparation_epoch&&pd_crc32((const uint8_t *)&state.magic,sizeof state.magic)==preparation_origin;}
static bool preparation_spell(unsigned i){return i<state.magic.count&&!state.magic.powers[i].kind&&state.magic.powers[i].level;}
static unsigned preparation_changes(bool ready){
    unsigned n=0;for(unsigned i=0;i<state.magic.count;i++)if(preparation_spell(i)&&preparation_draft[i]!=!!(state.magic.powers[i].flags&PD_PREPARED)&&!!preparation_draft[i]==ready)n++;
    return n;
}
static bool preparation_visible(unsigned i){return preparation_spell(i)&&
    (!preparation_filter||(preparation_filter==1?preparation_draft[i]:preparation_draft[i]!=!!(state.magic.powers[i].flags&PD_PREPARED)));}
static unsigned preparation_at(unsigned at){
    for(unsigned i=0;i<state.magic.count;i++)if(preparation_visible(i)){if(!at)return i;at--;}
    return PD_POWERS;
}
static unsigned preparation_count(void){unsigned n=0;for(unsigned i=0;i<state.magic.count;i++)if(preparation_visible(i))n++;return n;}
static void preparation_begin(void){
    for(unsigned i=0;i<PD_POWERS;i++)preparation_draft[i]=i<state.magic.count&&!!(state.magic.powers[i].flags&PD_PREPARED);
    preparation_origin=pd_crc32((const uint8_t *)&state.magic,sizeof state.magic);preparation_epoch=state.history_epoch;
    preparation_page=preparation_filter=wifi_focus=0;preparation_notice="";wifi_keyboard=false;page=MAGIC_PREPARE_LIST;
}
static intptr_t magic_setup_back(void){return page==MAGIC_COST_PICKER?MC_BACK:page==MAGIC_PREPARE_CONFIRM?MP_EDIT:MP_BACK;}
static void magic_setup_action(intptr_t action){
    if(page==MAGIC_COST_PICKER){
        if(action==MC_BACK)page=magic_cost_smite?HIT_SMITE:MAGIC_DETAIL;
        else if(action==MC_ORDINARY||action==MC_PACT)magic_cost_pact=action==MC_PACT;
        else if(action>=MC_LEVEL&&action<MC_LEVEL+9){unsigned level=action-MC_LEVEL+1;
            if(magic_cost_available(level,magic_cost_pact)){
                if(magic_cost_smite){smite_cost=level;smite_pact=magic_cost_pact;}else{cast_level=level;cast_pact=magic_cost_pact;}
                page=magic_cost_smite?HIT_SMITE:MAGIC_DETAIL;
            }
        }
    }else if(page==MAGIC_PREPARE_LIST){
        if(action==MP_BACK){page=MAGIC_LIST;memset(preparation_draft,0,sizeof preparation_draft);}
        else if(action==MP_PREV&&preparation_page)preparation_page--;
        else if(action==MP_NEXT&&(preparation_page+1)*3<preparation_count())preparation_page++;
        else if(action>=MP_ALL&&action<=MP_CHANGED){preparation_filter=action-MP_ALL;preparation_page=0;}
        else if(action>=MP_TOGGLE&&action<MP_TOGGLE+3&&preparation_current()){
            unsigned i=preparation_at(preparation_page*3+action-MP_TOGGLE);if(i<PD_POWERS)preparation_draft[i]=!preparation_draft[i];
        }else if(action==MP_REVIEW&&(preparation_changes(true)||preparation_changes(false))){page=MAGIC_PREPARE_CONFIRM;wifi_focus=0;}
    }else if(page==MAGIC_PREPARE_CONFIRM){
        if(action==MP_EDIT){page=MAGIC_PREPARE_LIST;wifi_focus=0;}
        else if(action==MP_SAVE){
            if(!preparation_current()){preparation_notice="Scheda cambiata: riapri la preparazione";return;}
            if(!power_storage_ready()){preparation_notice="Prima risolvi memoria e storico";return;}
            scratch=state;
            if(!pd_magic_prepare_many(&scratch,preparation_draft,state.magic.count)){preparation_notice="Preparazione non valida: riapri";return;}
            if(!memcmp(&scratch,&state,sizeof state)){page=MAGIC_LIST;return;}
            if(pd_storage_save(&scratch)){previous=state;undo_available=true;state=scratch;invalidate_strike();page=MAGIC_LIST;memset(preparation_draft,0,sizeof preparation_draft);}
            else preparation_notice=pd_storage_writable()?"Salvataggio non riuscito: riprova":"Memoria incerta: riavvia il Pocket";
        }
    }
    pd_screen_activity(&display_screen,lv_tick_get());
}
static void render_magic_setup(void){
    char text[128];wifi_target_count=0;
    bool current=page==MAGIC_COST_PICKER||preparation_current();
    if(page==MAGIC_COST_PICKER){
        label(magic_cost_smite?"Slot Punizione":"Scegli lo slot",83,111,300,true);
        const pd_power *p=&state.magic.powers[magic_cost_power];lv_obj_t *name=small_label(p->name,83,147,300,PD_GOLD);lv_label_set_long_mode(name,LV_LABEL_LONG_DOT);lv_obj_set_height(name,18);
        for(unsigned pool=0;pool<2;pool++){lv_obj_t *b=wifi_button(pool?"Patto":"Ordinari",83+pool*156,173,144,44,MC_ORDINARY+pool,true);if(magic_cost_pact==!!pool)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);}
        pd_magic_cost costs[18];unsigned count=pd_magic_costs(&state,magic_cost_power,costs);bool allowed[9]={false};
        for(unsigned i=0;i<count;i++)if(costs[i].pact==magic_cost_pact&&(!magic_cost_smite||pd_smite_prepare(&state,&strike,magic_cost_power,costs[i].level,costs[i].pact,false,NULL,NULL)))allowed[costs[i].level-1]=true;
        for(unsigned i=0;i<9;i++){
            const pd_slot *slot=&(magic_cost_pact?state.magic.pact:state.magic.slots)[i];
            snprintf(text,sizeof text,"L%u: %u/%u",i+1,slot->current,slot->maximum);
            lv_obj_t *b=wifi_button(text,83+(i%3)*104,225+(i/3)*44,92,44,MC_LEVEL+i,allowed[i]);
            lv_obj_set_style_text_font(b,&lv_font_montserrat_14,0);
            if((magic_cost_smite?smite_cost:cast_level)==i+1&&(magic_cost_smite?smite_pact:cast_pact)==magic_cost_pact)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
        }
        wifi_button("Indietro: nessun consumo",83,363,300,44,MC_BACK,true);
    }else if(page==MAGIC_PREPARE_LIST){
        unsigned count=preparation_count(),pages=count?(count+2)/3:1;if(preparation_page>=pages)preparation_page=pages-1;
        snprintf(text,sizeof text,"Prepara %u/%u",preparation_page+1,pages);label(text,139,111,188,false);
        wifi_button("<",83,99,48,44,MP_PREV,preparation_page>0);wifi_button(">",335,99,48,44,MP_NEXT,preparation_page+1<pages);
        snprintf(text,sizeof text,"Da preparare %u | da togliere %u",preparation_changes(true),preparation_changes(false));small_label(text,83,147,300,PD_GOLD);
        const char *tabs[]={"Tutte","Pronte","Modifiche"};for(unsigned i=0;i<3;i++){lv_obj_t *b=wifi_button(tabs[i],83+104*i,173,92,44,MP_ALL+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_14,0);if(preparation_filter==i)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);}
        for(unsigned row=0;row<3;row++){
            unsigned i=preparation_at(preparation_page*3+row);if(i==PD_POWERS)break;
            const pd_power *p=&state.magic.powers[i];lv_obj_t *b=wifi_button(p->name,83,223+44*row,300,44,MP_TOGGLE+row,current);
            lv_obj_set_style_pad_all(b,0,0);art(pd_power_icon(p),10,10,b);
            lv_obj_t *name=lv_obj_get_child(b,0);lv_obj_set_size(name,236,18);lv_label_set_long_mode(name,LV_LABEL_LONG_DOT);lv_obj_align(name,LV_ALIGN_TOP_LEFT,45,1);
            snprintf(text,sizeof text,"%s | livello %u",preparation_draft[i]?"Pronta":"Da preparare",p->level);
            lv_obj_t *badge=lv_label_create(b);lv_label_set_text(badge,text);lv_obj_set_style_text_font(badge,&lv_font_montserrat_14,0);lv_obj_set_style_text_color(badge,lv_color_hex(preparation_draft[i]?PD_MINT:PD_MUTED),0);lv_obj_set_pos(badge,45,23);
            if(preparation_draft[i]!=!!(p->flags&PD_PREPARED))lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
        }
        if(!count)small_label(preparation_filter==2?"Nessuna modifica in attesa":"Nessuna magia in questa vista",83,259,300,PD_TEXT);
        wifi_button("Annulla",83,363,144,44,MP_BACK,true);wifi_button("Rivedi",239,363,144,44,MP_REVIEW,preparation_changes(true)||preparation_changes(false));
    }else{
        label("Salva preparazione?",83,111,300,true);
        snprintf(text,sizeof text,"Da preparare: %u",preparation_changes(true));label(text,83,165,300,false);
        snprintf(text,sizeof text,"Da togliere: %u",preparation_changes(false));label(text,83,205,300,false);
        small_label("Slot e utilizzi invariati",83,251,300,PD_TEXT);small_label("Concentrazione e trucchetti invariati",83,281,300,PD_TEXT);
        small_label("Limiti e requisiti: verifica col DM",83,311,300,PD_GOLD);
        small_label(current?preparation_notice:"Scheda cambiata: annulla e riapri",83,337,300,PD_DANGER);
        wifi_button("Modifica",83,363,144,44,MP_EDIT,true);wifi_button("Salva",239,363,144,44,MP_SAVE,current&&power_storage_ready());
    }
    wifi_mark_focus();
}
