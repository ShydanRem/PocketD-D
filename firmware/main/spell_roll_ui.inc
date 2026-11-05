/* Explicit, per-cast assistance for generic spells. All drafts stay in RAM. */
enum {SR_BACK=7800,SR_COST,SR_DAMAGE,SR_SELF,SR_OTHER,SR_DIE,SR_ABILITY,SR_FEWER,SR_MORE,SR_MINUS,SR_PLUS,SR_REVIEW,SR_EDIT,SR_DETAILS,SR_COMMIT,SR_RECEIPT,SR_DONE,SR_DIE_BACK,SR_HOME,SR_D4=7830};
static pd_spell_roll_plan spell_plan;
PD_EXT static pd_state spell_origin,spell_candidate;
static pd_pool spell_result;
static unsigned spell_gained;
static bool spell_rolled,spell_saved,spell_dice_picker,spell_details,spell_from_formula;
static const char *spell_notice="";
static bool spell_roll_open(void){return page>=SPELL_METHOD&&page<=SPELL_BATTLE_RECEIPT;}
static bool spell_current(void){return !memcmp(&spell_origin,&state,sizeof state);}
static const char *spell_kind(void){return spell_plan.kind==PD_SPELL_DAMAGE?"Danni":spell_plan.kind==PD_SPELL_HEAL_SELF?"Cura me":"Cura altri";}
static bool spell_generic(void){return state.magic.available&&power_index<state.magic.count&&!state.magic.powers[power_index].kind&&!state.magic.powers[power_index].effect;}
#include "spell_formula_edit.inc"
#include "spell_battle.inc"
static void spell_pick(unsigned kind,unsigned resolution){
    spell_battle=(pd_spell_battle_plan){.mode=resolution,.armor=resolution==PD_SPELL_ATTACK?15:0,.save_ability=resolution==PD_SPELL_TARGET_SAVE?6:0};
    spell_plan.kind=kind;pd_spell_formula f=state.formulas[spell_plan.power];bool matching=f.kind==(kind==PD_SPELL_DAMAGE?1:2);spell_from_formula=matching;
    if(matching){pd_spell_roll_plan saved;if(!pd_spell_formula_resolve(&state,spell_plan.power,spell_plan.slot,spell_plan.pact,&saved))return;spell_plan=saved;spell_plan.kind=kind;}
    page=matching?(resolution?SPELL_BATTLE_SETUP:SPELL_REVIEW):SPELL_FORMULA;
}
static void spell_begin(void){
    spell_battle_reset();
    spell_origin=spell_candidate=state;spell_plan=(pd_spell_roll_plan){.power=power_index,.slot=cast_level,.pact=cast_pact,.count=1,.sides=6};
    spell_rolled=spell_saved=spell_dice_picker=spell_details=spell_from_formula=false;spell_notice="";spell_gained=0;page=SPELL_METHOD;
    pd_spell_roll_plan saved;if(pd_spell_formula_resolve(&state,power_index,cast_level,cast_pact,&saved)){spell_plan=saved;spell_from_formula=true;}else if(state.formulas[power_index].kind)spell_notice="Formula oltre i limiti: Gestisci";
}
static intptr_t spell_back(void){return spell_battle_page()?spell_battle_back():formula_configuring()?(page==SPELL_FORMULA_SAVE?SF_EDIT:SF_BACK):page==SPELL_RECEIPT?SR_DONE:page==SPELL_REVIEW?SR_EDIT:spell_dice_picker?SR_DIE_BACK:SR_BACK;}
static void spell_action(intptr_t action){
    if(spell_battle_page()){spell_battle_action(action);return;}
    if(formula_configuring()){formula_action(action);return;}
    if(page==SPELL_RECEIPT){if(action==SR_DONE)page=MAGIC_DETAIL;else if(action==SR_HOME)page=HERO;else if(action==SR_DETAILS)spell_details=!spell_details;return;}
    /* Once drawn, the only transition is committing this exact candidate. */
    if(spell_rolled&&action!=SR_COMMIT)return;
    if(action==SR_BACK){page=page==SPELL_METHOD?MAGIC_DETAIL:SPELL_METHOD;spell_notice="";return;}
    if(action==SR_EDIT&&page==SPELL_REVIEW){page=SPELL_FORMULA;spell_notice="";return;}
    if(action==SR_DIE_BACK){spell_dice_picker=false;return;}
    if(!spell_current()){spell_notice="Scheda cambiata: annulla e riapri";return;}
    if(page==SPELL_METHOD){
        if(action==SR_COST){cast_details=false;page=MAGIC_CONFIRM;}
        else if(action==SC_ATTACK||action==SC_SAVE)spell_pick(PD_SPELL_DAMAGE,action==SC_ATTACK?PD_SPELL_ATTACK:PD_SPELL_TARGET_SAVE);
        else if(action>=SR_DAMAGE&&action<=SR_OTHER)spell_pick(action-SR_DAMAGE+1,0);
    }else if(page==SPELL_FORMULA){
        if(spell_dice_picker){
            if(action>=SR_D4&&action<SR_D4+6){spell_from_formula=false;spell_plan.sides=(const unsigned[]){4,6,8,10,12,20}[action-SR_D4];spell_dice_picker=false;}
            return;
        }
        if(action!=SR_REVIEW&&action!=SR_DIE)spell_from_formula=false;
        if(action==SR_DIE)spell_dice_picker=true;
        else if(action==SR_ABILITY)spell_plan.bonus=pd_modifier(state.abilities[state.magic.powers[spell_plan.power].ability]);
        else if(action==SR_FEWER&&spell_plan.count>1)spell_plan.count--;
        else if(action==SR_MORE&&spell_plan.count<PD_POOL_MAX)spell_plan.count++;
        else if(action==SR_MINUS&&spell_plan.bonus>PD_POOL_MIN_BONUS)spell_plan.bonus--;
        else if(action==SR_PLUS&&spell_plan.bonus<PD_POOL_MAX_BONUS)spell_plan.bonus++;
        else if(action==SR_REVIEW){page=spell_battle.mode?SPELL_BATTLE_SETUP:SPELL_REVIEW;spell_details=false;spell_notice="";}
    }else if(page==SPELL_REVIEW){
        if(action==SR_EDIT){page=SPELL_FORMULA;spell_notice="";}
        else if(action==SR_DETAILS)spell_details=!spell_details;
        else if(action==SR_COMMIT){
            if(!power_storage_ready()||history_pending){spell_notice="Prima risolvi memoria e storico";return;}
            if(!spell_rolled){
                if(!pd_spell_roll(&state,&spell_plan,pd_board_random,NULL,&spell_candidate,&spell_result,&spell_gained)){spell_notice="Lancio non disponibile: verifica formula e costi";return;}
                spell_rolled=true;
            }
            bool changed=memcmp(&spell_candidate,&state,sizeof state)!=0;
            if(changed&&!pd_storage_save(&spell_candidate)){spell_notice="Non salvato: riprova lo stesso tiro";return;}
            if(changed){previous=state;undo_available=true;state=spell_candidate;invalidate_strike();}
            char note[PD_HISTORY_NOTE];
            if(spell_plan.kind==PD_SPELL_HEAL_SELF)snprintf(note,sizeof note,"Cura su me +%u; PF %u/%u",spell_gained,state.hp,pd_max_hp(&state));
            else snprintf(note,sizeof note,"%s; PF bersaglio invariati",spell_plan.kind==PD_SPELL_DAMAGE?"Danni da risolvere":"Cura altri");
            journal_pool(spell_plan.kind==PD_SPELL_DAMAGE?PD_H_DAMAGE:PD_H_HEAL,state.magic.powers[spell_plan.power].name,&spell_result,note);
            spell_saved=true;spell_rolled=false;spell_notice="";combat_roll=9;pool_view=true;has_roll=false;page=DICE;
            pd_dice_pool_start(&dice_animation,&spell_result,lv_tick_get());
        }
    }
}
