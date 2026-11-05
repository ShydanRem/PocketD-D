/* One paid spell, followed by one frozen damage roll. No target state invented. */
enum {SC_ATTACK=8000,SC_SAVE,SC_BACK,SC_REVIEW,SC_COMMIT,SC_ARMOR_LESS,SC_ARMOR_MORE,SC_MODE,SC_FEAR,SC_STAT,SC_ZERO,SC_HALF,SC_FAIL,SC_PASS,SC_DAMAGE,SC_MORE,SC_RECEIPT,SC_DONE,SC_HOME,SC_STAT_BACK,SC_STAT_FIRST=8030};
static pd_spell_battle_plan spell_battle;
static pd_spell_attack_result spell_attack;
static pd_spell_impact spell_impact;
static bool spell_paid,spell_attack_drawn,spell_damage_done,spell_second,spell_stat_picker;
static void spell_pick(unsigned kind,unsigned resolution);
static void spell_battle_reset(void){spell_battle=(pd_spell_battle_plan){0};spell_paid=spell_attack_drawn=spell_damage_done=spell_second=spell_stat_picker=false;spell_attack=(pd_spell_attack_result){0};spell_impact=(pd_spell_impact){0};}
static bool spell_battle_current(void){return !memcmp(spell_paid?&spell_candidate:&spell_origin,&state,sizeof state);}
static bool spell_battle_page(void){return page==SPELL_BATTLE_SETUP||page==SPELL_TARGET_SAVE||page==SPELL_BATTLE_RECEIPT||(page==SPELL_REVIEW&&spell_battle.mode);}
static intptr_t spell_battle_back(void){return page==SPELL_TARGET_SAVE?SC_DONE:page==SPELL_BATTLE_RECEIPT?SC_DONE:spell_stat_picker?SC_STAT_BACK:SC_BACK;}
static void spell_impact_journal(void){
    if(!spell_impact.first.count)return;
    if(history.epoch!=state.history_epoch)pd_history_store_load(state.history_epoch,&history);
    const pd_pool *pools[]={&spell_impact.first,&spell_impact.second};
    for(unsigned i=0;i<2;i++)if(pools[i]->count){
        const pd_pool *r=pools[i];pd_history_entry e={.kind=PD_H_DAMAGE,.count=r->count,.sides=r->sides,.bonus=r->bonus,.total=r->total,.floor_zero=true};memcpy(e.dice,r->dice,r->count);
        snprintf(e.label,sizeof e.label,"%s",spell_origin.magic.powers[spell_plan.power].name);
        snprintf(e.note,sizeof e.note,"%s %d; potenziali %d; parte %u",spell_impact.critical?"Critico":spell_impact.impact==PD_SPELL_HALF?"Meta TS":"Magia",spell_impact.rolled,spell_impact.applicable,i+1);
        assert(pd_history_entry_valid(&e));if(!i)history_unsaved=e;else history_unsaved_extra=e;
    }
    history_pending=true;history_pair_pending=spell_impact.second.count>0;history_third_pending=history_fourth_pending=history_fifth_pending=false;journal_flush();
}
static void spell_impact_begin(unsigned impact){
    if(!spell_paid||spell_damage_done||history_pending||!pd_history_store_available()||!spell_battle_current())return;
    bool critical=spell_battle.mode==PD_SPELL_ATTACK&&spell_attack.critical;
    if(!pd_spell_impact_roll(&spell_plan,critical,impact,pd_board_random,NULL,&spell_impact))return;
    spell_damage_done=true;spell_second=false;spell_impact_journal();
    if(!spell_impact.first.count){page=SPELL_BATTLE_RECEIPT;return;}
    page=DICE;combat_roll=11;pool_view=true;has_roll=false;pd_dice_pool_start(&dice_animation,&spell_impact.first,lv_tick_get());
}
static void spell_battle_action(intptr_t action){
    if(page==SPELL_BATTLE_RECEIPT){if(action==SC_DONE)page=MAGIC_DETAIL;else if(action==SC_HOME)page=HERO;return;}
    if(page==SPELL_TARGET_SAVE){
        if(action==SC_DONE){page=MAGIC_DETAIL;return;}
        if(action==SC_FAIL)spell_impact_begin(PD_SPELL_FULL);
        else if(action==SC_PASS)spell_impact_begin(spell_battle.success==PD_SPELL_SAVE_HALF?PD_SPELL_HALF:PD_SPELL_ZERO);
        return;
    }
    if(spell_attack_drawn&&!spell_paid&&action!=SC_COMMIT)return;
    if(action==SC_BACK){if(page==SPELL_REVIEW)page=SPELL_BATTLE_SETUP;else page=SPELL_METHOD;spell_notice="";return;}
    if(action==SC_STAT_BACK){spell_stat_picker=false;return;}
    if(!spell_battle_current()){spell_notice="Scheda cambiata: annulla e riapri";return;}
    if(page==SPELL_BATTLE_SETUP){
        if(spell_stat_picker){if(action>=SC_STAT_FIRST&&action<SC_STAT_FIRST+6){spell_battle.save_ability=action-SC_STAT_FIRST;spell_stat_picker=false;}return;}
        if(action==SC_ARMOR_LESS&&spell_battle.armor>1)spell_battle.armor--;
        else if(action==SC_ARMOR_MORE&&spell_battle.armor<40)spell_battle.armor++;
        else if(action==SC_MODE)spell_battle.requested=spell_battle.requested==0?1:spell_battle.requested==1?-1:0;
        else if(action==SC_FEAR){unsigned phase=!(spell_battle.context.known&PD_CONTEXT_FEAR)?0:(spell_battle.context.values&PD_CONTEXT_FEAR)?1:2;phase=(phase+1)%3;spell_battle.context=(pd_roll_context){.known=phase?PD_CONTEXT_FEAR:0,.values=phase==1?PD_CONTEXT_FEAR:0};}
        else if(action==SC_STAT)spell_stat_picker=true;
        else if(action==SC_ZERO)spell_battle.success=PD_SPELL_SAVE_ZERO;
        else if(action==SC_HALF)spell_battle.success=PD_SPELL_SAVE_HALF;
        else if(action==SC_REVIEW){page=SPELL_REVIEW;spell_notice="";}
        return;
    }
    if(page!=SPELL_REVIEW||action!=SC_COMMIT)return;
    if(!power_storage_ready()||history_pending){spell_notice="Prima risolvi memoria e storico";return;}
    if(!spell_attack_drawn){
        if(spell_battle.mode==PD_SPELL_ATTACK){if(!pd_spell_attack_cast(&state,&spell_plan,&spell_battle,pd_board_random,NULL,&spell_candidate,&spell_attack))return;spell_attack_drawn=true;}
        else if(!pd_spell_battle_prepare(&state,&spell_plan,&spell_battle,&spell_candidate))return;
    }
    bool changed=memcmp(&state,&spell_candidate,sizeof state)!=0;
    if(changed&&!pd_storage_save(&spell_candidate)){spell_notice=spell_battle.mode==PD_SPELL_ATTACK?"Non salvato: riprova lo stesso tiro":"Costo non salvato: riprova";return;}
    if(changed){previous=state;undo_available=true;state=spell_candidate;invalidate_strike();}
    spell_paid=true;spell_notice="";
    if(spell_battle.mode==PD_SPELL_TARGET_SAVE){page=SPELL_TARGET_SAVE;return;}
    char note[PD_HISTORY_NOTE];snprintf(note,sizeof note,"Magia CA %u: %s",spell_battle.armor,spell_attack.critical?"critico":spell_attack.hits?"colpito":"mancato");
    journal_d20(PD_H_ATTACK,spell_origin.magic.powers[spell_plan.power].name,&spell_attack.roll,spell_attack.mode,note);
    result=spell_attack.roll;page=DICE;combat_roll=10;pool_view=false;has_roll=false;pd_dice_start(&dice_animation,&result,lv_tick_get());
}
static bool spell_battle_dice_action(intptr_t action){
    if(page!=DICE||(combat_roll!=10&&combat_roll!=11)||!spell_paid||dice_animation.active)return false;
    if(action==SC_DONE){page=MAGIC_DETAIL;return true;}
    if(action==SC_RECEIPT){page=SPELL_BATTLE_RECEIPT;return true;}
    if(action==SC_DAMAGE&&combat_roll==10){if(has_roll&&spell_attack.hits)spell_impact_begin(PD_SPELL_FULL);return true;}
    if(action==SC_MORE&&combat_roll==11&&spell_impact.second.count){spell_second=!spell_second;has_roll=false;pd_dice_pool_start(&dice_animation,spell_second?&spell_impact.second:&spell_impact.first,lv_tick_get());return true;}
    return false;
}
