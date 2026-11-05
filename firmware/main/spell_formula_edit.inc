enum {SF_OPEN=7900,SF_BACK,SF_VALUES,SF_SCALING,SF_REVIEW,SF_EDIT,SF_SAVE,SF_DIE,SF_ABILITY,SF_FEWER,SF_MORE,SF_MINUS,SF_PLUS,SF_STEP_LESS,SF_STEP_MORE,SF_KIND=7920,SF_FIXED=7930,SF_SLOT,SF_LEVEL,SF_D4=7940};
static pd_spell_formula formula_draft;
static unsigned formula_panel;
static bool formula_configuring(void){return page==SPELL_FORMULA_SETUP||page==SPELL_FORMULA_SAVE;}
static void formula_begin(void){
    spell_origin=spell_candidate=state;spell_plan.power=power_index;formula_draft=state.formulas[power_index];formula_panel=0;spell_notice="";spell_rolled=false;page=SPELL_FORMULA_SETUP;
}
static void formula_changed(void){formula_draft.source=0;spell_notice="";}
static void formula_action(intptr_t action){
    if(action==SF_BACK){if(formula_panel)formula_panel=formula_panel==3?1:0;else page=MAGIC_MANAGE;return;}
    if(action==SF_EDIT){page=SPELL_FORMULA_SETUP;return;}
    if(!spell_current()){spell_notice="Scheda cambiata: annulla e riapri";return;}
    if(page==SPELL_FORMULA_SAVE){
        if(action!=SF_SAVE)return;
        spell_candidate=state;
        if(!pd_spell_formula_set(&spell_candidate,spell_plan.power,formula_draft)||!pd_storage_writable())return;
        if(memcmp(&spell_candidate,&state,sizeof state)){
            if(!pd_storage_save(&spell_candidate)){spell_notice="Non salvato: riprova";return;}
            previous=state;undo_available=true;state=spell_candidate;invalidate_strike();
        }
        page=MAGIC_MANAGE;return;
    }
    if(action==SF_REVIEW){page=SPELL_FORMULA_SAVE;formula_panel=0;return;}
    if(action>=SF_KIND&&action<=SF_KIND+2){
        unsigned kind=action-SF_KIND;
        if(!kind)formula_draft=(pd_spell_formula){0};
        else {if(!formula_draft.kind)formula_draft=(pd_spell_formula){.count=1,.sides=6};formula_draft.kind=kind;}
        formula_changed();return;
    }
    if(!formula_draft.kind)return;
    if(action==SF_VALUES){formula_panel=1;return;}
    if(action==SF_SCALING){formula_panel=2;return;}
    if(action==SF_DIE){formula_panel=3;return;}
    if(formula_panel==3&&action>=SF_D4&&action<SF_D4+6){formula_draft.sides=(const unsigned[]){4,6,8,10,12,20}[action-SF_D4];formula_panel=1;formula_changed();return;}
    if(formula_panel==2){
        unsigned level=spell_origin.magic.powers[spell_plan.power].level;
        if(action>=SF_FIXED&&action<=SF_LEVEL){
            unsigned mode=action-SF_FIXED;if((mode==1&&!level)||(mode==2&&level))return;
            formula_draft.scaling=mode;formula_draft.step=mode?(formula_draft.step?formula_draft.step:1):0;
        }else if(action==SF_STEP_LESS&&formula_draft.step>1)formula_draft.step--;
        else if(action==SF_STEP_MORE&&formula_draft.scaling&&formula_draft.step<18)formula_draft.step++;
        else return;
        formula_changed();return;
    }
    if(formula_panel!=1)return;
    if(action==SF_ABILITY)formula_draft.ability=!formula_draft.ability;
    else if(action==SF_FEWER&&formula_draft.count>1)formula_draft.count--;
    else if(action==SF_MORE&&formula_draft.count<18)formula_draft.count++;
    else if(action==SF_MINUS&&formula_draft.bonus> -15)formula_draft.bonus--;
    else if(action==SF_PLUS&&formula_draft.bonus<60)formula_draft.bonus++;
    else return;
    formula_changed();
}
