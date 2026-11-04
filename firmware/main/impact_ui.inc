/* Incoming-hit drafts: direct IDs, no persistence or random draw in the views. */
enum {I_BACK=7000,I_PREV,I_NEXT,I_COSTS=7004,I_CHANGE,I_TYPE=7010,I_TARGET=7030,I_KIND=7050,I_SOURCE=7100};
static unsigned impact_page,impact_kind;
static unsigned impact_cost_limit(unsigned kind){return kind==PD_COST_COUNTER?PD_RESOURCES:kind==PD_COST_POWER?PD_POWERS:kind==PD_COST_ITEM?PD_ITEMS:9;}
static bool impact_cost_eligible(unsigned kind,unsigned index){return kind>PD_COST_NONE&&kind<PD_COST_COUNT&&index<impact_cost_limit(kind)&&pd_reduction_cost_max(&state,kind,index)&&!cost_used(kind,index);}
static unsigned impact_cost_count(unsigned kind){unsigned n=0;for(unsigned i=0;i<impact_cost_limit(kind);i++)n+=impact_cost_eligible(kind,i);return n;}
static unsigned impact_cost_at(unsigned kind,unsigned ordinal){for(unsigned i=0;i<impact_cost_limit(kind);i++)if(impact_cost_eligible(kind,i)&&!ordinal--)return i;return impact_cost_limit(kind);}
static unsigned impact_target_count(void){return damage_extra_count+1+(page==DAMAGE_REDUCTION);}
static void impact_open(unsigned panel){
    if(panel==3&&page!=DAMAGE_HOME&&page!=DAMAGE_PART_EDIT)return;
    if(panel==4&&!((page==DAMAGE_REDUCTION&&reduction_return!=DAMAGE_PART_EDIT)||(page==PROFILE_ALLOCATE&&!state.reductions[profile_index].stage)))return;
    if(panel==5&&(page!=PROFILE_COST||!profile_cost_editing))return;
    if(panel==7){pd_damage_event e=current_damage_event();if(page!=DAMAGE_CONFIRM||!pd_damage_event_preview(&state,&e,&damage_details)||!damage_details.profile||!damage_details.total.configured)return;}
    rules_panel=panel;rules_panel_owner=page;rules_focus=0;impact_page=0;
    if(panel==3)impact_page=(page==DAMAGE_HOME?damage_plan.type:damage_part_draft.type)/6;
}
static void impact_picker_action(intptr_t action){
    if(action==I_BACK){if(rules_panel==6){rules_panel=5;impact_page=0;}else rules_panel=0;rules_focus=0;return;}
    if(action==5820){impact_picker_action(I_BACK);return;} /* physical local back */
    unsigned page_size=rules_panel==3?6:3,count=rules_panel==3?PD_DAMAGE_TYPES:rules_panel==4?impact_target_count():rules_panel==6?impact_cost_count(impact_kind):0;
    if(action==I_PREV&&impact_page)impact_page--;
    else if(action==I_NEXT&&(impact_page+1)*page_size<count)impact_page++;
    else if(rules_panel==3&&action>=I_TYPE&&action<I_TYPE+PD_DAMAGE_TYPES){
        unsigned type=action-I_TYPE;if(page==DAMAGE_HOME){if(damage_plan.type!=type){damage_plan.type=type;damage_plan.concentration=0;}}else if(page==DAMAGE_PART_EDIT)damage_part_draft.type=type;rules_panel=0;rules_focus=0;
    }else if(rules_panel==4&&action>=I_TARGET&&action<=I_TARGET+PD_DAMAGE_COMPONENTS){
        unsigned target=action-I_TARGET;if(target<damage_extra_count+1||(target==PD_DAMAGE_COMPONENTS&&page==DAMAGE_REDUCTION)){if(page==DAMAGE_REDUCTION)reduction_target=target;else if(page==PROFILE_ALLOCATE)allocation_row=target;rules_panel=0;rules_focus=0;}
    }else if(rules_panel==5&&page==PROFILE_COST&&profile_cost_editing&&action>=I_KIND&&action<I_KIND+PD_COST_COUNT){
        unsigned kind=action-I_KIND;if(!kind){pd_profile_cost_set(&profile_draft,profile_cost_row,(pd_cost){0});rules_panel=0;rules_focus=0;}else if(impact_cost_count(kind)){impact_kind=kind;impact_page=0;rules_panel=6;rules_focus=0;}
    }else if(rules_panel==6&&page==PROFILE_COST&&profile_cost_editing&&action>=I_SOURCE&&action<I_SOURCE+(intptr_t)impact_cost_limit(impact_kind)){
        unsigned index=action-I_SOURCE;if(!impact_cost_eligible(impact_kind,index))return;
        pd_cost old=pd_profile_cost(&profile_draft,profile_cost_row);if(old.kind!=impact_kind||old.index!=index)pd_profile_cost_set(&profile_draft,profile_cost_row,(pd_cost){impact_kind,index,1});rules_panel=0;rules_focus=0;
    }else if(rules_panel==7&&action==I_CHANGE&&damage_details.profile_snapshot.payment==PD_PAYMENT_CHOOSE){rules_panel=0;page=PAYMENT_CHOICE;rules_focus=0;}
}
static const char *impact_defence_badge(unsigned type){
    if(!(state.defences.configured&(1u<<type)))return "Da verificare";
    if(pd_defence_conditional(&state,type))return "Con regola";
    unsigned flags=pd_defence_flags(&state,type);return flags&PD_IMMUNITY?"Immune":flags==3?"Res. + vuln.":flags&PD_RESISTANCE?"Resiste":flags&PD_VULNERABILITY?"Debole":"Normale";
}
static const lv_image_dsc_t *impact_cost_icon(unsigned kind,unsigned index){return kind==PD_COST_POWER?pd_power_icon(&state.magic.powers[index]):kind==PD_COST_ITEM?pd_item_icon(&state,index):pd_ui_icon(kind==PD_COST_SLOT||kind==PD_COST_PACT?PD_UI_MAGIC:PD_UI_USES);}
static void impact_page_controls(unsigned count,unsigned page_size){
    button("<",83,359,60,48,I_PREV,impact_page>0);button("Indietro",155,359,156,48,I_BACK,true);button(">",323,359,60,48,I_NEXT,(impact_page+1)*page_size<count);
}
static void render_impact_picker(void){
    char text[128];
    if(rules_panel==3){
        label("Tipo di danno",83,105,300,false);snprintf(text,sizeof text,"%u / %u",impact_page+1,(PD_DAMAGE_TYPES+5)/6);small_label(text,83,135,300,PD_MUTED);
        for(unsigned row=0;row<6;row++){unsigned type=impact_page*6+row;if(type>=PD_DAMAGE_TYPES)break;game_choice(pd_damage_name(type),pd_damage_name(type),impact_defence_badge(type),pd_damage_icon(type),83+(row%2)*156,163+(row/2)*56,144,I_TYPE+type,true,type==(page==DAMAGE_HOME?damage_plan.type:damage_part_draft.type));}
        impact_page_controls(PD_DAMAGE_TYPES,6);
    }else if(rules_panel==4){
        label("Dove riduci?",83,105,300,false);small_label(page==PROFILE_ALLOCATE?"Budget condiviso del profilo":"Prima di una parte o dopo il colpo",83,135,300,PD_MUTED);
        pd_damage_event e=current_damage_event();unsigned count=impact_target_count();
        for(unsigned row=0;row<3;row++){unsigned at=impact_page*3+row;if(at>=count)break;bool after=at==e.count;unsigned target=after?PD_DAMAGE_COMPONENTS:at;char caption[64];if(after)strcpy(caption,"Intero colpo");else snprintf(caption,sizeof caption,"%u. %s",at+1,pd_damage_name(e.components[at].type));unsigned n=page==PROFILE_ALLOCATE?allocation_draft[at]:reduction_drafts[target];snprintf(text,sizeof text,"%s | %u",after?"Dopo le difese":"Prima delle difese",n);lv_obj_t *b=scene_control(caption,after?pd_ui_icon(PD_UI_RESIST):pd_damage_icon(e.components[at].type),caption,text,83,159+row*60,300,54,I_TARGET+target,true);rules_selected_style(b,target==(page==PROFILE_ALLOCATE?allocation_row:reduction_target));}
        impact_page_controls(count,3);
    }else if(rules_panel==5){
        label("Fonte della spesa",83,105,300,false);
        static const char *names[]={"Nessuna","Contatori","Poteri","Slot","Patto","Cariche"};static const char *badges[]={"Senza costo","Risorse","Usi limitati","Ordinari","Slot patto","Oggetti"};
        for(unsigned i=0;i<PD_COST_COUNT;i++){lv_obj_t *b=scene_control(names[i],pd_ui_icon(i==PD_COST_SLOT||i==PD_COST_PACT?PD_UI_MAGIC:i==PD_COST_ITEM?PD_UI_PACK:PD_UI_USES),names[i],badges[i],83+(i%2)*156,151+(i/2)*64,144,56,I_KIND+i,!i||impact_cost_count(i));rules_selected_style(b,pd_profile_cost(&profile_draft,profile_cost_row).kind==i);}
        button("Indietro",83,359,300,48,I_BACK,true);
    }else if(rules_panel==6){
        static const char *names[]={"","Contatori","Poteri","Slot ordinari","Slot patto","Cariche oggetti"};label(names[impact_kind],83,105,300,false);unsigned count=impact_cost_count(impact_kind);snprintf(text,sizeof text,"%u / %u",impact_page+1,(count+2)/3);small_label(text,83,135,300,PD_MUTED);
        for(unsigned row=0;row<3;row++){unsigned index=impact_cost_at(impact_kind,impact_page*3+row);if(index>=impact_cost_limit(impact_kind))break;char name[100];const char *title=pd_reduction_cost_name(&state,impact_kind,index);snprintf(name,sizeof name,"%u. %s",index+1,title);snprintf(text,sizeof text,"Saldo %u / %u",pd_reduction_cost_current(&state,impact_kind,index),pd_reduction_cost_max(&state,impact_kind,index));lv_obj_t *b=scene_control(name,impact_cost_icon(impact_kind,index),title,text,83,159+row*60,300,54,I_SOURCE+index,true);pd_cost c=pd_profile_cost(&profile_draft,profile_cost_row);rules_selected_style(b,c.kind==impact_kind&&c.index==index);}
        impact_page_controls(count,3);
    }else if(rules_panel==7){
        label("Spese del colpo",83,105,300,false);small_label("Solo alla conferma del danno",83,135,300,PD_GOLD);
        for(unsigned row=0;row<PD_PROFILE_COSTS;row++){pd_cost c=pd_profile_cost(&damage_details.profile_snapshot,row);const pd_cost_receipt *r=&damage_details.costs[row];if(!c.kind)strcpy(text,"Non configurata");else if(damage_details.profile_snapshot.payment==PD_PAYMENT_CHOOSE&&!r->selected)snprintf(text,sizeof text,"Non scelta | saldo %u",r->before);else if(!damage_details.total.configured)strcpy(text,"Da verificare");else snprintf(text,sizeof text,"%u > %u | costo %u",r->before,r->after,c.amount);scene_control(c.kind?r->name:"Nessuna fonte",c.kind?impact_cost_icon(c.kind,c.index):pd_ui_icon(PD_UI_USES),c.kind?r->name:"Nessuna fonte",text,83,155+row*58,300,54,0,false);}
        small_label(damage_details.profile_snapshot.reaction?"Reazione: pronta > usata":"Reazione invariata",83,331,300,PD_GOLD);
        if(damage_details.profile_snapshot.payment==PD_PAYMENT_CHOOSE){button("Indietro",83,359,144,48,I_BACK,true);lv_obj_t *b=button("Altra fonte",239,359,144,48,I_CHANGE,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}else button("Indietro",83,359,300,48,I_BACK,true);
    }
}
static void render_impact_confirm(const pd_damage_event_summary *report){
    const pd_damage_summary *r=&report->total;if(!r->configured)return;char text[128];
    label("Conferma colpo",83,111,228,false);lv_obj_t *types=button("Tipi",323,99,60,44,1930,true);lv_obj_set_style_text_font(types,&lv_font_montserrat_16,0);
    snprintf(text,sizeof text,"%u > %u",report->raw,r->adjusted);status_tile(PD_UI_ATTACK,"Danni",text,83,157);delta_tile(PD_UI_HEART,"PF",state.hp,r->hp_after,239,157);delta_tile(PD_UI_SHIELD,"Temp",state.temp_hp,r->temp_after,83,225);
    unsigned costs=0;for(unsigned i=0;report->profile&&i<PD_PROFILE_COSTS;i++)costs+=pd_profile_cost(&report->profile_snapshot,i).kind&&(report->profile_snapshot.payment!=PD_PAYMENT_CHOOSE||report->costs[i].selected);bool reaction=report->profile&&report->profile_snapshot.reaction;
    if(costs)snprintf(text,sizeof text,"%u %s%s",costs,costs==1?"fonte":"fonti",reaction?" + R":"");else snprintf(text,sizeof text,"%s",reaction?"Reazione":"Nessuno");scene_control("Rivedi costi",pd_ui_icon(PD_UI_USES),"Costi >",text,239,225,144,60,I_COSTS,costs||reaction);
    if(r->massive||r->at_zero){small_label(r->massive?"Possibile morte istantanea":"Gia a zero PF: verifica il colpo",83,291,300,PD_DANGER);button("Risolvi registro morte",83,311,300,44,3000,true);}
    else if(r->needs_concentration){pd_check_info info;pd_damage_summary summary;bool known=current_concentration_info(0,0,&info,&summary,false);snprintf(text,sizeof text,known?"Concentrazione CD %u":"CD %u | COS da verificare",r->concentration_dc);small_label(text,83,291,300,PD_GOLD);snprintf(text,sizeof text,"Esito: %s",(const char *const[]){"da risolvere","mantieni","termina"}[damage_plan.concentration]);button(text,83,311,300,44,1806,true);}
    else{small_label(r->ends_concentration?"A zero PF: fine concentrazione":costs||reaction?"Spese anche a danno zero":!r->adjusted?"Nessun danno effettivo":"Un colpo, una conferma",83,303,300,PD_GOLD);small_label("Tipi: difese, fonti e conto completo",83,331,300,PD_MUTED);}
    if(r->needs_concentration){pd_check_info info;pd_damage_summary summary;bool ready=current_concentration_info(0,0,&info,&summary,false);const char *names[]={"Indietro","Tira COS","Conferma"};const unsigned actions[]={1804,1860,1805};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i==0||(i==1?ready:r->allowed));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}}
    else{button("Indietro",83,359,144,48,1804,true);button("Conferma",239,359,144,48,1805,r->allowed);}
}
