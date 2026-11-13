/* Secondary weapon is a saved choice, never a silent equipment swap. */
enum {DW_OPEN=8400,DW_BACK,DW_CONFIG,DW_PICK,DW_EDITION,DW_STYLE,DW_LIGHT,DW_SAVE,DW_NONE,DW_PREV,DW_NEXT,DW_ROLL,DW_RETRY,DW_BONUS,DW_SPEND,DW_FREE,DW_TURN,DW_AMMO,DW_SHIELD,DW_ITEM=8440};
static pd_dual dual_draft;
static unsigned dual_primary,dual_page;
static bool dual_light,dual_pending,dual_bonus_draft;
static uint32_t dual_origin;
static page_t dual_return;
static const char *dual_notice="";
static bool dual_open(void){return dual_pending||page==DUAL_SETUP||page==DUAL_PICKER||page==DUAL_ATTACK||page==BONUS_ACTION;}
static bool dual_current(void){return dual_origin==pd_crc32((const uint8_t *)&state,sizeof state);}
static void dual_begin(bool configure){
    dual_origin=pd_crc32((const uint8_t *)&state,sizeof state);dual_draft=state.dual;dual_primary=state.equipped;dual_notice="";dual_pending=false;dual_page=0;
    dual_light=dual_primary<state.item_count&&dual_draft.weapon&&(state.handling[dual_primary].flags&PD_LIGHT)&&(state.handling[dual_draft.weapon-1].flags&PD_LIGHT);
    page=configure||!dual_draft.weapon?DUAL_SETUP:DUAL_ATTACK;
}
static bool dual_preview(void){
    if(!dual_current()||dual_primary>=state.item_count)return false;
    scratch=state;
    if(dual_draft.weapon){scratch.handling[dual_primary].flags=(scratch.handling[dual_primary].flags&~PD_LIGHT)|(dual_light?PD_LIGHT:0);unsigned i=dual_draft.weapon-1;scratch.handling[i].flags=(scratch.handling[i].flags&~PD_LIGHT)|(dual_light?PD_LIGHT:0);}
    return pd_dual_set(&scratch,dual_draft);
}
static bool dual_roll(void){
    if(history_pending||!power_storage_ready()||!dual_current())return false;
    if(!pd_dual_prepare(&state,true,&scratch)){dual_notice="Verifica armi, bonus e munizioni";return false;}
    if(!dual_pending){
        if(!pd_dual_attack(&state,true,target_armor,mode,roll_context,ignore_effects,pd_board_random,NULL,&strike))return false;
        hit_candidate=state;dual_pending=true;
    }
    if(!pd_storage_save(&scratch)){dual_notice="Non salvato: riprova lo stesso tiro";return false;}
    previous=state;undo_available=true;state=scratch;dual_pending=false;dual_result=true;ammo_paid=false;
    pd_attack_resolution gate;pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);char note[49];snprintf(note,sizeof note,"CA %u: %s; bonus spesa",strike.armor,strike.hits?"colpito":"mancato");
    unsigned graze;if(pd_graze_damage(&strike,&graze)){size_t at=strlen(note);snprintf(note+at,sizeof note-at,"; Scalfire %u",graze);}
    journal_d20(PD_H_ATTACK,state.items[strike.weapon.index].name,&strike.roll,gate.mode,note);
    result=strike.roll;strike_ready=true;damage_used=false;page=DICE;has_roll=false;combat_roll=1;pool_view=false;pd_dice_start(&dice_animation,&result,lv_tick_get());return true;
}
static void dual_action(intptr_t action){
    if(dual_pending){if(action==DW_RETRY)dual_roll();return;}
    if(action==DW_BACK){page=page==BONUS_ACTION?dual_return:page==DUAL_PICKER?DUAL_SETUP:COMBAT;return;}
    if(!dual_current()){dual_notice="Scheda cambiata: annulla e riapri";return;}
    if(page==BONUS_ACTION){
        if(action==DW_SPEND||action==DW_FREE)dual_bonus_draft=action==DW_SPEND;
        else if(action==DW_SAVE&&power_storage_ready()){scratch=state;if(!pd_bonus_change(&scratch,dual_bonus_draft))return;if(memcmp(&scratch,&state,sizeof state)){if(!pd_storage_save(&scratch)){dual_notice="Non salvato: riprova";return;}previous=state;undo_available=true;state=scratch;invalidate_strike();}page=dual_return;}
        return;
    }
    if(action==15){mode=mode==0?1:mode==1?-1:0;return;}
    if(action==86&&target_armor>1){target_armor--;return;}
    if(action==87&&target_armor<30){target_armor++;return;}
    if(action==2230){context_return=page;context_draft=roll_context;context_ignore_draft=ignore_effects;page=ROLL_CONTEXT;return;}
    if(action==DW_SHIELD){for(unsigned i=0;i<state.item_count;i++)if(state.gear[i].kind==PD_WEAR_SHIELD&&state.gear[i].equipped){item_index=i;page=ITEM_DETAIL;play_action(84);if(page==PLAY_CONFIRM)play_return=COMBAT;return;}return;}
    if(action==DW_AMMO&&state.dual.weapon){ammunition_begin_weapon(state.dual.weapon-1);return;}
    if(action==DW_CONFIG){dual_begin(true);return;}
    if(action==DW_TURN){reaction_return=COMBAT;page=REACTION_HOME;return;}
    if(page==DUAL_ATTACK){if(action==DW_ROLL)dual_roll();return;}
    if(action==DW_PICK){dual_page=0;page=DUAL_PICKER;return;}
    if(page==DUAL_PICKER){
        if(action==DW_PREV&&dual_page)dual_page--;else if(action==DW_NEXT&&(dual_page+1)*3<state.item_count)dual_page++;
        else if(action>=DW_ITEM&&action<DW_ITEM+PD_ITEMS){unsigned i=action-DW_ITEM;if(i>=state.item_count||i==dual_primary||state.items[i].kind!=PD_WEAPON||!state.items[i].quantity)return;dual_draft.weapon=i+1;if(!dual_draft.edition)dual_draft.edition=1;dual_light=(state.handling[dual_primary].flags&PD_LIGHT)&&(state.handling[i].flags&PD_LIGHT);page=DUAL_SETUP;}
        return;
    }
    if(action==DW_EDITION&&dual_draft.weapon)dual_draft.edition=dual_draft.edition==1?2:1;
    else if(action==DW_STYLE&&dual_draft.weapon)dual_draft.style=!dual_draft.style;
    else if(action==DW_LIGHT&&dual_draft.weapon)dual_light=!dual_light;
    else if(action==DW_NONE){dual_draft.weapon=dual_draft.edition=dual_draft.style=0;}
    else if(action==DW_SAVE&&power_storage_ready()&&dual_preview()){
        if(memcmp(&scratch,&state,sizeof state)){if(!pd_storage_save(&scratch)){dual_notice="Non salvato: riprova";return;}previous=state;undo_available=true;state=scratch;invalidate_strike();}dual_begin(false);
        if(!state.dual.weapon)page=COMBAT;
    }
}
