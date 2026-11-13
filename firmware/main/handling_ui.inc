/* Small local draft; weapon properties are explicit and survive native backups. */
enum {WH_OPEN=8300,WH_BACK,WH_TYPE,WH_GRIP,WH_FINESSE,WH_REVIEW,WH_EDIT,WH_SAVE,WH_SHIELD,WH_SWAP};
static unsigned handling_index;
static uint32_t handling_origin;
static pd_handling handling_draft;
static const char *handling_notice="";
static bool handling_open(void){return page==HANDLING_SETUP||page==HANDLING_REVIEW;}
static bool handling_current(void){return handling_origin==pd_crc32((const uint8_t *)&state,sizeof state);}
static bool handling_preview(void){scratch=state;return handling_current()&&pd_handling_set(&scratch,handling_index,handling_draft);}
static void handling_begin(bool swap){
    if(state.equipped>=state.item_count||state.items[state.equipped].kind!=PD_WEAPON)return;
    handling_index=state.equipped;handling_draft=state.handling[handling_index];handling_origin=pd_crc32((const uint8_t *)&state,sizeof state);handling_notice="";page=HANDLING_SETUP;
    if(swap&&(handling_draft.flags&PD_VERSATILE)){handling_draft.grip=!handling_draft.grip;if(handling_preview())page=HANDLING_REVIEW;}
}
static void handling_action(intptr_t action){
    if(action==WH_BACK){page=COMBAT;return;}if(action==WH_EDIT){page=HANDLING_SETUP;return;}
    if(!handling_current()){handling_notice="Scheda cambiata: annulla e riapri";return;}
    if(action==WH_SHIELD){for(unsigned i=0;i<state.item_count;i++)if(state.gear[i].kind==PD_WEAR_SHIELD&&state.gear[i].equipped){item_index=i;page=ITEM_DETAIL;play_action(84);if(page==PLAY_CONFIRM)play_return=COMBAT;return;}}
    if(page==HANDLING_SETUP){
        if(action==WH_TYPE){
            unsigned modes[6]={0,PD_TWO_HANDED,PD_VERSATILE|(6u<<8),PD_VERSATILE|(8u<<8),PD_VERSATILE|(10u<<8),PD_VERSATILE|(12u<<8)},at=0;
            for(unsigned i=0;i<6;i++)if((modes[i]&3)==(handling_draft.flags&3)&&(modes[i]>>8)==handling_draft.sides)at=i;
            for(unsigned n=1;n<=6;n++){unsigned next=modes[(at+n)%6];pd_handling h=handling_draft;h.flags=(h.flags&PD_FINESSE)|(next&3);h.sides=next>>8;h.grip=0;scratch=state;scratch.handling[handling_index]=h;if(pd_handling_valid(&scratch)){handling_draft=h;break;}}
        }else if(action==WH_GRIP&&(handling_draft.flags&PD_VERSATILE))handling_draft.grip=!handling_draft.grip;
        else if(action==WH_FINESSE&&state.items[handling_index].ability<2)handling_draft.flags^=PD_FINESSE;
        else if(action==WH_REVIEW&&handling_preview())page=HANDLING_REVIEW;
        return;
    }
    if(page==HANDLING_REVIEW&&action==WH_SAVE&&power_storage_ready()&&handling_preview()){
        bool changed=memcmp(&scratch,&state,sizeof state)!=0;
        if(changed&&!pd_storage_save(&scratch)){handling_notice="Non salvato: riprova";return;}
        if(changed){previous=state;undo_available=true;state=scratch;invalidate_strike();}attack_select(attack_spell);
    }
}
