/* Session-local ammunition binding. Quantities use the existing character codec. */
enum {AC_OPEN=8100,AC_BACK,AC_NONE,AC_PREV,AC_NEXT,AC_RETRY,AC_ITEM_FIRST=8120};
static unsigned ammo_item=PD_ITEMS,ammo_weapon=PD_ITEMS,ammo_page,ammo_before,ammo_after;
static uint32_t ammo_epoch,ammo_picker_origin;
static pd_item ammo_identity;
static bool ammo_pending,ammo_paid;
static int ammo_mode;
static const char *ammo_notice="";
static bool ammunition_open(void){return ammo_pending||page==AMMO_PICKER||page==AMMO_RETRY;}
static bool ammunition_ranged(void){return state.inventory_available&&state.equipped<state.item_count&&(state.items[state.equipped].flags&4)&&!state.combat.thrown;}
static bool ammunition_bound(void){return ammo_item<PD_ITEMS&&ammo_epoch==state.history_epoch&&ammo_weapon==state.equipped&&ammunition_ranged();}
static bool ammunition_same_item(void){
    if(!pd_ammunition_eligible(&state,ammo_item))return false;
    pd_item item=state.items[ammo_item];item.quantity=0;return !memcmp(&item,&ammo_identity,sizeof item);
}
/* Pending history uses the existing roll-to-register recovery route. */
static bool ammunition_storage_ready(void){return !ammunition_bound()||(pd_storage_writable()&&pd_history_store_available());}
static bool ammunition_available(void){return !ammunition_bound()||(ammunition_same_item()&&state.items[ammo_item].quantity);}
static void ammunition_caption(char *text,size_t cap){
    if(!ammunition_bound())snprintf(text,cap,"Munizioni: consumo manuale");
    else if(!ammunition_same_item())snprintf(text,cap,"Munizioni cambiate: scegli di nuovo");
    else snprintf(text,cap,"Munizioni %u | -1 per attacco",state.items[ammo_item].quantity);
}
static unsigned ammunition_count(void){unsigned n=0;for(unsigned i=0;i<state.item_count;i++)if(pd_ammunition_eligible(&state,i))n++;return n;}
static unsigned ammunition_at(unsigned at){for(unsigned i=0;i<state.item_count;i++)if(pd_ammunition_eligible(&state,i)){if(!at)return i;at--;}return PD_ITEMS;}
static void ammunition_begin(void){
    if(!ammunition_ranged())return;
    ammo_picker_origin=pd_crc32((const uint8_t *)&state,sizeof state);ammo_page=0;ammo_notice="";page=AMMO_PICKER;
}
static bool ammunition_picker_current(void){return ammo_picker_origin==pd_crc32((const uint8_t *)&state,sizeof state);}
static bool ammunition_attack(void){
    if(history_pending||!pd_history_store_available())return false;
    bool spend=ammunition_bound();
    if(spend&&!ammo_pending&&page!=COMBAT&&page!=COMBAT_DETAIL)return false;
    if(ammo_pending&&memcmp(&state,&hit_candidate,sizeof state)){ammo_notice="Scheda cambiata: tiro sospeso";return false;}
    if(spend){
        if(!ammunition_available()||!power_storage_ready()||!pd_ammunition_prepare(&state,ammo_item,&scratch))return false;
        if(!ammo_pending){hit_candidate=state;ammo_before=state.items[ammo_item].quantity;ammo_after=scratch.items[ammo_item].quantity;}
    }
    if(!ammo_pending){
        if(attack_spell>=0?!pd_true_strike_attack(&state,(unsigned)attack_spell,attack_radiant,target_armor,mode,roll_context,ignore_effects,pd_board_random,NULL,&strike):!pd_attack_with_context(&state,target_armor,mode,roll_context,ignore_effects,pd_board_random,NULL,&strike))return false;
        pd_attack_resolution gate;pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);ammo_mode=gate.mode;
        ammo_paid=false;
    }
    if(spend){
        ammo_pending=true;page=AMMO_RETRY;
        if(!pd_storage_save(&scratch)){ammo_notice="Non salvato: riprova lo stesso tiro";return false;}
        feedback_capture(&feedback_before);previous=hit_candidate;undo_available=true;state=scratch;ammo_pending=false;ammo_paid=true;feedback_prepare();
    }
    char note[49];snprintf(note,sizeof note,"CA %u: %s%s%s",strike.armor,strike.hits?"colpito":"mancato",strike.critical?"; critico":"",spend?"; -1 muniz.":"");
    journal_d20(PD_H_ATTACK,strike.weapon.true_strike?state.magic.powers[attack_spell].name:state.items[state.equipped].name,&strike.roll,ammo_mode,note);
    result=strike.roll;strike_ready=true;damage_used=false;page=DICE;has_roll=false;combat_roll=1;pool_view=false;
    pd_dice_start(&dice_animation,&result,lv_tick_get());return true;
}
static void ammunition_action(intptr_t action){
    if(ammo_pending||page==AMMO_RETRY){if(action==AC_RETRY)ammunition_attack();return;}
    if(page!=AMMO_PICKER)return;
    if(action==AC_BACK){page=COMBAT;return;}
    if(!ammunition_picker_current()){ammo_notice="Scheda cambiata: riapri la scelta";return;}
    if(action==AC_PREV&&ammo_page)ammo_page--;
    else if(action==AC_NEXT&&(ammo_page+1)*3<ammunition_count())ammo_page++;
    else if(action==AC_NONE){ammo_item=PD_ITEMS;ammo_paid=false;attack_select(attack_spell);}
    else if(action>=AC_ITEM_FIRST&&action<AC_ITEM_FIRST+PD_ITEMS){
        unsigned index=action-AC_ITEM_FIRST;if(!pd_ammunition_eligible(&state,index)||!state.items[index].quantity)return;
        ammo_item=index;ammo_weapon=state.equipped;ammo_epoch=state.history_epoch;ammo_identity=state.items[index];ammo_identity.quantity=0;ammo_paid=false;attack_select(attack_spell);
    }
}
