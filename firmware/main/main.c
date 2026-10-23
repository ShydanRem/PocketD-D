#include "pocketdd.h"
#include "board.h"
#include "storage.h"
#include "history_store.h"
#include "backup.h"
#include "dice_animation.h"
#include "visual.h"
#include "avatar.h"
#include "companion.h"
#include "theme.h"
#include "checks.h"
#include "combat.h"
#include "magic.h"
#include "buttons.h"
#include "input_check.h"
#include "rest.h"
#include "recharge.h"
#include "status.h"
#include "damage.h"
#ifndef PD_UI_TEST
#include "esp_attr.h"
#define PD_EXT EXT_RAM_BSS_ATTR
#else
#define PD_EXT
#endif
#include "../assets/device_art.h"
#include "../assets/pixel_font.h"
#include "src/misc/cache/instance/lv_image_cache.h"
#ifndef PD_UI_TEST
#include "driver/usb_serial_jtag.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#endif
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

typedef enum { HERO, SHEET, DICE, RESOURCES, APPEARANCE, CHECK_DETAIL, INVENTORY, ITEM_DETAIL, ITEM_CONFIRM, COMBAT, COMBAT_DETAIL, MAGIC_LIST, MAGIC_DETAIL, MAGIC_CONFIRM, MAGIC_TEXT, MAGIC_STATUS, MAGIC_SLOTS, MAGIC_RECOVER, MAGIC_STOP, KEY_SETTINGS, REST_HOME, REST_LONG_SETTINGS, REST_PREVIEW, REST_REGAIN, REST_SESSION, HIT_DICE, HIT_CONFIRM, STATUS_HOME, DEATH_VIEW, DEATH_EDIT, CONDITION_LIST, CONDITION_DETAIL, CONDITION_PICKER, STATUS_CONFIRM, DEVICE_SETTINGS, EFFECT_LIST, EFFECT_EDIT, EFFECT_CONFIRM, EFFECT_DURATION, EFFECT_ROUND, EFFECT_LINK, ROUND_DEFENCES, DAMAGE_HOME, DEFENCE_LIST, DEFENCE_EDIT, DEFENCE_CONFIRM, DAMAGE_CONFIRM, DAMAGE_RESULT, CONCENTRATION_SETUP, DEFENCE_TEMPORARY, DEFENCE_DURATION, DEFENCE_LINK, DAMAGE_PARTS, DAMAGE_PART_EDIT, DAMAGE_TYPES, DEFENCE_RULE, DEFENCE_RULE_REVIEW, DAMAGE_CONTEXT, DEFENCE_GRANTS, DEFENCE_GRANT_REVIEW, DAMAGE_REDUCTION, DAMAGE_MATH, PROFILE_LIST, PROFILE_VIEW, PROFILE_EDIT, PROFILE_DETAILS, PROFILE_NUMBER, PROFILE_CONFIRM, PROFILE_ALLOCATE, PROFILE_IMPACT, PROFILE_COST, PROFILE_COST_IMPACT, REACTION_HOME, REACTION_CONFIRM, TURN_DETAILS, TIME_HOME, TIME_CONFIRM, TIME_DETAILS, ITEM_CHARGES, ITEM_CHARGE_CONFIRM, ITEM_RECHARGE, ITEM_RECHARGE_CONFIRM, ITEM_RECHARGE_RESULT, CONDITION_EFFECTS, CONDITION_EFFECTS_CONFIRM, ROLL_CONTEXT, HOME_MENU, HOME_VITALS, WALLET_HOME, WALLET_EDIT, WALLET_CONFIRM, HISTORY_HOME, HISTORY_DETAIL, HISTORY_DISCARD, BACKUP_HOME, BACKUP_SEND_CONFIRM, BACKUP_RESTORE_CONFIRM, INPUT_CHECK_HOME, INPUT_CHECK_TOUCH, INPUT_CHECK_KEYS, DAMAGE_DEATH, DEATH_ROLL_SETUP, EXTERNAL_TURN_HOME } page_t;
static const pd_theme_kind page_themes[]={
    [HERO]=PD_THEME_NONE,
    [SHEET]=PD_THEME_SHEET,
    [DICE]=PD_THEME_DICE,
    [RESOURCES]=PD_THEME_USES,
    [APPEARANCE]=PD_THEME_APPEARANCE,
    [CHECK_DETAIL]=PD_THEME_SHEET,
    [INVENTORY]=PD_THEME_PACK,
    [ITEM_DETAIL]=PD_THEME_PACK,
    [ITEM_CONFIRM]=PD_THEME_PACK,
    [COMBAT]=PD_THEME_COMBAT,
    [COMBAT_DETAIL]=PD_THEME_COMBAT,
    [MAGIC_LIST]=PD_THEME_MAGIC,
    [MAGIC_DETAIL]=PD_THEME_MAGIC,
    [MAGIC_CONFIRM]=PD_THEME_MAGIC,
    [MAGIC_TEXT]=PD_THEME_MAGIC,
    [MAGIC_STATUS]=PD_THEME_MAGIC,
    [MAGIC_SLOTS]=PD_THEME_MAGIC,
    [MAGIC_RECOVER]=PD_THEME_MAGIC,
    [MAGIC_STOP]=PD_THEME_MAGIC,
    [KEY_SETTINGS]=PD_THEME_KEYS,
    [REST_HOME]=PD_THEME_REST,
    [REST_LONG_SETTINGS]=PD_THEME_REST,
    [REST_PREVIEW]=PD_THEME_REST,
    [REST_REGAIN]=PD_THEME_REST,
    [REST_SESSION]=PD_THEME_REST,
    [HIT_DICE]=PD_THEME_REST,
    [HIT_CONFIRM]=PD_THEME_REST,
    [STATUS_HOME]=PD_THEME_STATUS,
    [DEATH_VIEW]=PD_THEME_STATUS,
    [DEATH_EDIT]=PD_THEME_STATUS,
    [CONDITION_LIST]=PD_THEME_STATUS,
    [CONDITION_DETAIL]=PD_THEME_STATUS,
    [CONDITION_PICKER]=PD_THEME_STATUS,
    [STATUS_CONFIRM]=PD_THEME_STATUS,
    [DEVICE_SETTINGS]=PD_THEME_SCREEN,
    [EFFECT_LIST]=PD_THEME_EFFECTS,
    [EFFECT_EDIT]=PD_THEME_EFFECTS,
    [EFFECT_CONFIRM]=PD_THEME_EFFECTS,
    [EFFECT_DURATION]=PD_THEME_EFFECTS,
    [EFFECT_ROUND]=PD_THEME_EFFECTS,
    [EFFECT_LINK]=PD_THEME_EFFECTS,
    [ROUND_DEFENCES]=PD_THEME_EFFECTS,
    [DAMAGE_HOME]=PD_THEME_DAMAGE,
    [DEFENCE_LIST]=PD_THEME_DAMAGE,
    [DEFENCE_EDIT]=PD_THEME_DAMAGE,
    [DEFENCE_CONFIRM]=PD_THEME_DAMAGE,
    [DAMAGE_CONFIRM]=PD_THEME_DAMAGE,
    [DAMAGE_RESULT]=PD_THEME_DAMAGE,
    [CONCENTRATION_SETUP]=PD_THEME_DAMAGE,
    [DEFENCE_TEMPORARY]=PD_THEME_DAMAGE,
    [DEFENCE_DURATION]=PD_THEME_DAMAGE,
    [DEFENCE_LINK]=PD_THEME_DAMAGE,
    [DAMAGE_PARTS]=PD_THEME_DAMAGE,
    [DAMAGE_PART_EDIT]=PD_THEME_DAMAGE,
    [DAMAGE_TYPES]=PD_THEME_DAMAGE,
    [DEFENCE_RULE]=PD_THEME_DAMAGE,
    [DEFENCE_RULE_REVIEW]=PD_THEME_DAMAGE,
    [DAMAGE_CONTEXT]=PD_THEME_DAMAGE,
    [DEFENCE_GRANTS]=PD_THEME_DAMAGE,
    [DEFENCE_GRANT_REVIEW]=PD_THEME_DAMAGE,
    [DAMAGE_REDUCTION]=PD_THEME_DAMAGE,
    [DAMAGE_MATH]=PD_THEME_DAMAGE,
    [PROFILE_LIST]=PD_THEME_DAMAGE,
    [PROFILE_VIEW]=PD_THEME_DAMAGE,
    [PROFILE_EDIT]=PD_THEME_DAMAGE,
    [PROFILE_DETAILS]=PD_THEME_DAMAGE,
    [PROFILE_NUMBER]=PD_THEME_DAMAGE,
    [PROFILE_CONFIRM]=PD_THEME_DAMAGE,
    [PROFILE_ALLOCATE]=PD_THEME_DAMAGE,
    [PROFILE_IMPACT]=PD_THEME_DAMAGE,
    [PROFILE_COST]=PD_THEME_DAMAGE,
    [PROFILE_COST_IMPACT]=PD_THEME_DAMAGE,
    [REACTION_HOME]=PD_THEME_STATUS,
    [REACTION_CONFIRM]=PD_THEME_STATUS,
    [TURN_DETAILS]=PD_THEME_TIME,
    [TIME_HOME]=PD_THEME_TIME,
    [TIME_CONFIRM]=PD_THEME_TIME,
    [TIME_DETAILS]=PD_THEME_TIME,
    [ITEM_CHARGES]=PD_THEME_PACK,
    [ITEM_CHARGE_CONFIRM]=PD_THEME_PACK,
    [ITEM_RECHARGE]=PD_THEME_PACK,
    [ITEM_RECHARGE_CONFIRM]=PD_THEME_PACK,
    [ITEM_RECHARGE_RESULT]=PD_THEME_PACK,
    [CONDITION_EFFECTS]=PD_THEME_EFFECTS,
    [CONDITION_EFFECTS_CONFIRM]=PD_THEME_EFFECTS,
    [ROLL_CONTEXT]=PD_THEME_SHEET,
    [HOME_MENU]=PD_THEME_NONE,
    [HOME_VITALS]=PD_THEME_NONE,
    [WALLET_HOME]=PD_THEME_WALLET,
    [WALLET_EDIT]=PD_THEME_WALLET,
    [WALLET_CONFIRM]=PD_THEME_WALLET,
    [HISTORY_HOME]=PD_THEME_HISTORY,
    [HISTORY_DETAIL]=PD_THEME_HISTORY,
    [HISTORY_DISCARD]=PD_THEME_HISTORY,
    [BACKUP_HOME]=PD_THEME_BACKUP,
    [BACKUP_SEND_CONFIRM]=PD_THEME_BACKUP,
    [BACKUP_RESTORE_CONFIRM]=PD_THEME_BACKUP,
    [INPUT_CHECK_HOME]=PD_THEME_CHECK,
    [INPUT_CHECK_TOUCH]=PD_THEME_CHECK,
    [INPUT_CHECK_KEYS]=PD_THEME_CHECK,
    [DAMAGE_DEATH]=PD_THEME_DAMAGE,
    [DEATH_ROLL_SETUP]=PD_THEME_STATUS,
    [EXTERNAL_TURN_HOME]=PD_THEME_EFFECTS,
};
PD_EXT static pd_state state,previous,pending,scratch;
static bool undo_available,import_pending,has_roll;
static pd_roll result;
static pd_dice_animation dice_animation;
static lv_obj_t *dice_canvas;
static uint16_t *dice_pixels;
static lv_timer_t *dice_timer;
static lv_obj_t *avatar_sprite;
static lv_timer_t *avatar_timer;
enum { FEEDBACK_HEAL=1,FEEDBACK_HURT,FEEDBACK_RECOVER,FEEDBACK_SPEND,FEEDBACK_MIXED,FEEDBACK_WALLET };
#define FEEDBACK_POOLS (PD_RESOURCES+PD_POWERS+18+PD_ITEMS*2+PD_HIT_POOLS)
typedef struct { unsigned hp,temp;uint16_t pools[FEEDBACK_POOLS];pd_wallet wallet; } feedback_snapshot;
static feedback_snapshot feedback_before;
static struct { bool active;unsigned kind;uint32_t started;int hp_delta,temp_delta;char text[64]; } feedback;
static lv_obj_t *feedback_panel,*feedback_motes[3];
static lv_timer_t *feedback_timer;
static void feedback_frame(lv_timer_t *timer);
static lv_obj_t *scene_lights[5];
static lv_obj_t *scene_halos[2];
static uint32_t avatar_started;
static unsigned avatar_phase;
static unsigned avatar_scale;
_Alignas(4) static uint8_t avatar_data[PD_AVATAR_LARGE_BYTES];
static lv_image_dsc_t avatar_image={
    .header={.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_I8,
             .w=PD_AVATAR_WIDTH,.h=PD_AVATAR_HEIGHT,.stride=PD_AVATAR_WIDTH},
    .data_size=PD_AVATAR_IMAGE_BYTES,.data=avatar_data
};
static void avatar_frame(lv_timer_t *timer);
static void dice_frame(lv_timer_t *timer);
static page_t page;
static int ability=-1,mode,resource_index;
static pd_check_kind check_kind=PD_CHECK;
static bool ignore_effects;
static pd_roll_context roll_context,context_draft;
static bool context_ignore_draft;
static page_t context_return;
static unsigned condition_effects_draft;
static const char *const roll_condition_names[]={"Accecato","Spaventato","Prono","Trattenuto"};
static int sheet_tab,skill_page;
static bool pool_view,dice_setup,die_picker,pool_damage;
static unsigned pool_sides=6,pool_count=1;
static int pool_bonus;
static unsigned charge_draft;
static pd_recharge_plan recharge_plan={.count=1,.sides=6};
static pd_pool recharge_result;
static bool recharge_rolled;
static unsigned recharge_gained,recharge_before,recharge_after,recharge_maximum,recharge_index;
static char recharge_name[PD_ITEM_NAME_BYTES];
static unsigned inventory_page,item_index,target_armor=15,healing_gained;
/* 0: checks/pools, 1: attack, 2: damage, 3: item cure, 4: hit die cure,
   5: committed death save, 6: committed concentration save + incoming damage, 7: item charge recovery. */
static unsigned combat_roll;
static pd_strike strike;
static bool strike_ready,damage_used;
static pd_pool damage_result;
static pd_buttons physical_keys;
static pd_screen display_screen;
static pd_input_check input_check;
static lv_obj_t *input_check_counts[3];
static bool input_check_open(void){return page>=INPUT_CHECK_HOME&&page<=INPUT_CHECK_KEYS;}
static const char *input_check_notice="PWR lungo: alimentazione";
static unsigned magic_filter=2,magic_page,power_index,cast_level,text_page,slot_index;
static bool cast_pact,slot_pact,stop_rage;
static const char *const shortcut_names[]={"Spento","Eroe","Scheda","Dadi","Zaino","Magie","Lancia","Riposi","Stato","Schermo","Effetti","Round","Danni","Storico"};
static void shortcut(unsigned gesture);
static bool can_use_power(void);
static pd_rest_plan rest_plan;
static unsigned hit_index;
static bool rest_active,hit_rolled;
PD_EXT static pd_state hit_candidate;
static pd_pool hit_result;
static unsigned hit_gained;
static bool confirmation_open(void){return page==EXTERNAL_TURN_HOME||page==DEATH_ROLL_SETUP||page==DAMAGE_DEATH||input_check_open()||page==BACKUP_SEND_CONFIRM||page==BACKUP_RESTORE_CONFIRM||page==HISTORY_DISCARD||page==WALLET_EDIT||page==WALLET_CONFIRM||page==ITEM_CONFIRM||page==MAGIC_CONFIRM||page==MAGIC_STOP||page==REST_PREVIEW||page==HIT_CONFIRM||page==STATUS_CONFIRM||page==EFFECT_CONFIRM||page==EFFECT_ROUND||page==ROUND_DEFENCES||page==DEFENCE_CONFIRM||page==DAMAGE_CONFIRM||page==CONCENTRATION_SETUP||page==DAMAGE_TYPES||page==DEFENCE_RULE_REVIEW||page==DAMAGE_CONTEXT||page==DEFENCE_GRANT_REVIEW||page==DAMAGE_REDUCTION||page==DAMAGE_MATH||(page>=PROFILE_LIST&&page<=ROLL_CONTEXT);}
static pd_damage_plan damage_plan; /* First component + explicit concentration choice. */
static pd_damage_component damage_extra[PD_DAMAGE_COMPONENTS-1],damage_part_draft;
static pd_damage_context damage_context,damage_extra_context[PD_DAMAGE_COMPONENTS-1],damage_part_context,source_context_draft;
static page_t source_context_return,rule_review_return;
static unsigned damage_extra_count,damage_part_index,damage_part_page,damage_type_page,incoming_components;
static page_t damage_confirm_return,damage_details_return;
static pd_damage_event_summary incoming_event,concentration_event_report,damage_details;
static unsigned damage_reductions[PD_DAMAGE_COMPONENTS],damage_reduction_after,damage_part_reduction;
static unsigned reduction_drafts[PD_DAMAGE_COMPONENTS+1],reduction_target;
static page_t reduction_return;
static unsigned damage_profile,damage_allocation[PD_DAMAGE_COMPONENTS],reduction_profile_draft,reduction_allocation_draft[PD_DAMAGE_COMPONENTS];
static void clear_damage_reductions(void){damage_profile=0;memset(damage_allocation,0,sizeof damage_allocation);memset(damage_reductions,0,sizeof damage_reductions);damage_reduction_after=0;}
static bool has_reductions(const pd_damage_event_summary *r){return r->requested_before||r->requested_after||r->profile;}
static pd_damage_event current_damage_event(void){
    pd_damage_event event={.count=damage_extra_count+1,.concentration=damage_plan.concentration,.components={{damage_plan.type,damage_plan.amount}}};
    event.profile=damage_profile;memcpy(event.allocation,damage_allocation,sizeof damage_allocation);event.reduction_after=damage_reduction_after;for(unsigned i=0;i<event.count;i++)event.reductions[i]=damage_reductions[i];
    event.contexts[0]=damage_context;
    for(unsigned i=0;i<damage_extra_count;i++){event.components[i+1]=damage_extra[i];event.contexts[i+1]=damage_extra_context[i];}
    return event;
}
static bool current_concentration_info(int mode,int extra,pd_check_info *info,pd_damage_summary *summary,bool cap30){
    pd_damage_event event=current_damage_event();event.concentration=0;pd_damage_event_summary report;
    if(!pd_concentration_event_info(&state,&event,mode,extra,info,&report,cap30))return false;
    *summary=report.total;return true;
}
static pd_death_damage_choice death_damage_choice;
static pd_death_damage_summary incoming_death;
static bool incoming_death_used;
static pd_damage_summary incoming_result;
static pd_damage_summary concentration_damage;
static pd_roll concentration_result;
static int concentration_mode,concentration_extra;
static bool concentration_rolled,concentration_cap30;
static unsigned damage_hp_before,damage_temp_before,defence_index,defence_page,defence_flags;
static bool defence_configured,defence_rule_temporary,rule_review_temporary;
static uint8_t defence_conditions[3];
static unsigned defence_rule_property,rule_review_property;
static const char *const rule_names[]={"Sempre","Magico","Non magico","Att. non magico","Non argentato","Non adamantio","Nessuno dei due"};
static const char *const rule_descriptions[]={"Ogni fonte","Danno magico","Danno non magico","Attacco non magico","Attacco non magico, non argentato","Attacco non magico, non in adamantio","Attacco non magico, non argentato e non in adamantio"};
static const char *const defence_property_names[]={"Resistenza","Vulnerabilita","Immunita"};
static page_t defence_return_page,defence_confirm_return,round_return_page;
static unsigned defence_slot;
static pd_defence_temporary defence_temporary_drafts[PD_DEFENCE_SLOTS];
#define defence_temporary_draft defence_temporary_drafts[defence_slot]
static const pd_defence_temporary *old_defence(void){return pd_defence_grant(&state,defence_index,defence_slot);}
static unsigned active_defences(const pd_state *s,unsigned type){unsigned n=0;for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)n+=pd_defence_grant(s,type,slot)->active;return n;}
static unsigned potential_defences(const pd_state *s,unsigned type){unsigned flags=pd_defence_base_flags(s,type);for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(s,type,slot);if(d->active)flags|=d->flags;}return flags;}
static unsigned round_defence_page;
static const char *defence_caption(bool configured,unsigned flags);
static const char *defence_profile(const pd_state *s,unsigned type);
static unsigned profile_index,profile_confirm_tab,profile_impact_row,allocation_row,allocation_draft[PD_DAMAGE_COMPONENTS];
static pd_reduction_profile profile_draft;
static unsigned profile_cost_row;
static bool profile_number_rounds,profile_number_cost,profile_cost_editing,reaction_draft,turn_end;
static page_t reaction_return;
static unsigned turn_detail_page,time_minutes=1,time_detail_page,turn_owner;
static bool preview_time(pd_state *next,pd_round_summary *r){*next=state;return pd_time_advance(next,time_minutes,r);}
static const char *duration_mark(unsigned tick){return (const char *const[]){"r","i","f","m","m"}[tick];}
static unsigned duration_left(unsigned tick,unsigned rounds,unsigned minutes){return tick>=PD_TICK_MINUTES?minutes:rounds;}
static void time_text(char *text,size_t size,unsigned minutes){snprintf(text,size,"%uh %umin",minutes/60,minutes%60);}
static bool preview_turn(pd_state *next,pd_round_summary *r){*next=state;return pd_turn_advance_subject(next,turn_owner,!turn_end,r);}
static const char *duration_mode(bool timed,unsigned tick){return !timed?"manuale":(const char *const[]){"round","inizio turno","fine turno","minuti","ore"}[tick];}
static const char *duration_short(unsigned tick){return (const char *const[]){"round","inizio","fine","min","min"}[tick];}
static const char *turn_subject(unsigned owner){return (const char *const[]){"mio","A","B","C"}[owner];}
static const char *duration_owned(bool timed,unsigned tick,unsigned owner){return owner?(tick==PD_TICK_START?(const char *const[]){"","inizio A","inizio B","inizio C"}[owner]:(const char *const[]){"","fine A","fine B","fine C"}[owner]):duration_mode(timed,tick);}
static const char *duration_owned_mark(unsigned tick,unsigned owner){return owner?(tick==PD_TICK_START?(const char *const[]){"","iA","iB","iC"}[owner]:(const char *const[]){"","fA","fB","fC"}[owner]):duration_mark(tick);}
static bool turn_clock(unsigned tick){return tick==PD_TICK_START||tick==PD_TICK_END;}
static void duration_next(bool *timed,uint8_t *tick,uint8_t *count,uint16_t *minutes,uint8_t *owner){
    if(!*timed){*timed=true;*tick=0;}else if(++*tick>=PD_TICK_COUNT){*timed=false;*tick=0;}
    *owner=0;*count=*timed&&*tick<PD_TICK_MINUTES?1:0;*minutes=*timed&&*tick>=PD_TICK_MINUTES?(*tick==PD_TICK_HOURS?60:1):0;
}
static void duration_adjust(unsigned tick,uint8_t *count,uint16_t *minutes,unsigned button){
    int delta=(tick==PD_TICK_HOURS?(const int[]){-60,-1,1,60}:(const int[]){-10,-1,1,10})[button];
    int value=duration_left(tick,*count,*minutes)+delta;unsigned maximum=tick==PD_TICK_HOURS?5940:99;value=value<0?0:(unsigned)value>maximum?(int)maximum:value;
    if(tick>=PD_TICK_MINUTES)*minutes=value;else *count=value;
}
static const pd_reduction_profile *cost_profile(void){return profile_cost_editing?&profile_draft:&state.reductions[profile_index];}
static const char *cost_name(const pd_reduction_profile *p){return pd_reduction_cost_name(&state,pd_profile_cost(p,profile_cost_row).kind,pd_profile_cost(p,profile_cost_row).index);}
static unsigned cost_current(const pd_reduction_profile *p){return pd_reduction_cost_current(&state,pd_profile_cost(p,profile_cost_row).kind,pd_profile_cost(p,profile_cost_row).index);}
static bool cost_used(unsigned kind,unsigned index){
    for(unsigned row=0;row<PD_PROFILE_COSTS;row++){pd_cost other=pd_profile_cost(&profile_draft,row);if(row!=profile_cost_row&&other.kind==kind&&other.index==index)return true;}return false;
}
static void cost_next(void){
    pd_cost selected=pd_profile_cost(&profile_draft,profile_cost_row);unsigned kind=selected.kind,index=selected.index;
    do{if(!kind){kind=PD_COST_COUNTER;index=0;}else if(++index>=(kind==PD_COST_COUNTER?PD_RESOURCES:kind==PD_COST_POWER?PD_POWERS:kind==PD_COST_ITEM?PD_ITEMS:9)){kind=(kind+1)%PD_COST_COUNT;index=0;}}while(kind&&(!pd_reduction_cost_max(&state,kind,index)||cost_used(kind,index)));
    pd_profile_cost_set(&profile_draft,profile_cost_row,(pd_cost){kind,index,kind?1:0});
}

static bool context_relevant(unsigned bit){return bit==PD_CONTEXT_SIGHT?(combat_roll!=1&&check_kind!=PD_SAVE&&(state.roll_conditions&PD_BLINDED)):(combat_roll==1||check_kind!=PD_SAVE)&&(state.roll_conditions&PD_FRIGHTENED);}
static bool current_condition_context(pd_roll_context context,bool ignore,pd_condition_result *out){
    if(combat_roll==1){pd_attack_resolution r;if(!pd_attack_resolve(&state,mode,context,ignore,&r))return false;*out=(pd_condition_result){.needed=r.needed,.disadvantage=r.mode<0};return true;}
    pd_check_resolution r;if(ability<0||!pd_check_resolve(&state,check_kind,ability,mode,ignore,context,&r))return false;*out=(pd_condition_result){.needed=r.needed,.automatic_failure=r.automatic_failure,.disadvantage=r.mode<0};return true;
}
static const char *context_answer(unsigned bit){return !(context_draft.known&bit)?"?":context_draft.values&bit?"si":"no";}
static bool recharge_available(void){return state.inventory_available&&item_index<state.item_count&&state.items[item_index].quantity&&state.items[item_index].charges.current<state.items[item_index].charges.maximum;}
static bool preview_profile(pd_state *s){*s=state;return pd_reduction_profile_set(s,profile_index,profile_draft);}
static const char *profile_types(unsigned types){if(types==8191)return "Tutti";if(types==7)return "Cont./Perf./Tagl.";if(!types)return "Nessuno";for(unsigned i=0;i<13;i++)if(types==(1u<<i))return pd_damage_name(i);return "Personalizzati";}
static unsigned allocation_sum(void){unsigned n=0;for(unsigned i=0;i<=damage_extra_count;i++)n+=allocation_draft[i];return n;}
static const char *profile_problem(const pd_damage_event_summary *r){return r->cost_unavailable?"Oggetto non disponibile":r->cost_missing?"Risorsa insufficiente":r->reaction_missing?"Reazione gia usata":r->profile_unavailable?"Profilo non attivo":r->profile_budget?"Budget da verificare":r->profile_ineligible?"Parti escluse dal profilo":NULL;}
static bool local_defences(void){for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){const pd_reduction_profile *p=&state.reductions[i];if(p->extra_costs[0].kind||p->extra_costs[1].kind||p->value||p->types||p->stage||p->rule||p->active||p->timed||p->rounds||p->concentration||p->cost_kind||p->reaction)return true;}if(state.defences.configured)return true;for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(&state,i,slot);if(d->flags||d->active||d->timed||d->rounds||d->concentration)return true;}return false;}
static unsigned bit_count(uint64_t mask){unsigned count=0;while(mask){count+=mask&1u;mask>>=1;}return count;}
static bool preview_defence(pd_state *next){
    *next=state;if(!pd_defence_set(next,defence_index,defence_configured,defence_flags))return false;
    for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)if(!pd_defence_grant_set(next,defence_index,slot,defence_temporary_drafts[slot]))return false;
    for(unsigned i=0;i<3;i++)if(!pd_defence_rule_set(next,defence_index,false,i,defence_conditions[i]))return false;
    return true;
}
static void edit_defence(unsigned type,page_t return_page){defence_index=type;defence_flags=pd_defence_base_flags(&state,type);memcpy(defence_conditions,state.defences.conditions[type],3);defence_slot=0;for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)defence_temporary_drafts[slot]=*pd_defence_grant(&state,type,slot);defence_confirm_return=DEFENCE_EDIT;defence_configured=(state.defences.configured&(1u<<type))!=0;defence_return_page=return_page;page=DEFENCE_EDIT;}
static unsigned effect_index,effect_page;
static pd_effect effect_draft;
static bool effect_removing;
static bool preview_effect(pd_state *next){*next=state;return effect_removing?pd_effect_remove(next,effect_index):pd_effect_set(next,effect_index,effect_draft);}
static bool timed_effects(void){for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++)if(state.reductions[i].active&&state.reductions[i].timed&&!state.reductions[i].tick)return true;for(unsigned i=0;i<state.effect_count;i++)if(state.effects[i].active&&state.effects[i].timed&&!state.effects[i].tick)return true;for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){const pd_defence_temporary *d=pd_defence_grant(&state,i,slot);if(d->active&&d->timed&&!d->tick)return true;}return false;}
static bool preview_round(pd_state *next,pd_round_summary *summary){*next=state;return pd_round_advance(next,summary);}
enum { STATUS_ROLL,STATUS_POISON,STATUS_DEATH,STATUS_LEVEL,STATUS_REMOVE,STATUS_ADD };
static unsigned status_command,death_action,condition_page,condition_index,preset_page,preset_index;
static int condition_delta;
static bool status_rolled;
static page_t status_return_page;
static pd_roll status_result;
static pd_death_roll_options status_options;
static unsigned status_outcome; /* Frozen receipt: failure, success, stable, dead, 1 HP. */
static const char *const condition_presets[]={"Accecato","Affascinato","Assordato","Spossatezza","Spaventato","Afferrato","Incapacitato","Invisibile","Paralizzato","Pietrificato","Avvelenato","Prono","Trattenuto","Stordito","Privo di sensi"};
static bool status_change(pd_state *next) {
    *next=state;
    if(status_command==STATUS_ROLL)return pd_death_ready(&state);
    if(status_command==STATUS_POISON){next->training.poisoned=!state.training.poisoned;return true;}
    if(status_command==STATUS_DEATH)return pd_death_change(next,death_action);
    if(status_command==STATUS_LEVEL)return pd_condition_level(next,condition_index,condition_delta);
    if(status_command==STATUS_REMOVE)return pd_condition_remove(next,condition_index);
    if(status_command==STATUS_ADD)return preset_index<15&&pd_condition_add(next,condition_presets[preset_index],0);
    return false;
}
static void request_status(unsigned command){status_return_page=page;status_command=command;status_rolled=false;if(status_change(&scratch))page=STATUS_CONFIRM;}
static const char *death_caption(const pd_state *s){
    if(!s->status.available)return "Registro non disponibile";
    if(s->hp)return (s->status.successes||s->status.failures||s->status.stabilized)?"Contatori da verificare con il DM":"PF positivi: nessun tiro";
    if(s->status.failures==3)return "Tre fallimenti: verifica con il DM";
    if(s->status.stabilized)return "Stabilizzato: niente tiri";
    if(s->status.successes==3)return "Tre successi: verifica con il DM";
    return "A zero PF: tiro base disponibile";
}
static unsigned selected_hit_dice(void){unsigned n=0;for(unsigned i=0;i<PD_HIT_POOLS;i++)n+=rest_plan.regain[i];return n;}



#ifndef PD_UI_TEST
static QueueHandle_t packets,backup_packets,diagnostic_requests;
static SemaphoreHandle_t usb_output;
static TaskHandle_t usb_thread;
static TickType_t import_started;
PD_EXT static char line[PD_PACKET_MAX+1];
#endif
static pd_wallet wallet_draft,wallet_origin;
static unsigned wallet_page,wallet_coin,wallet_step;
static const uint32_t wallet_steps[]={1,10,100,1000,10000,100000,1000000,10000000,100000000};
static const char *const coin_names[]={"Rame","Argento","Electrum","Oro","Platino"};
static const char *const coin_units[]={"MR","MA","ME","MO","MP"};
static bool wallet_same(const pd_wallet *a,const pd_wallet *b){return a->available==b->available&&!memcmp(a->coins,b->coins,sizeof a->coins);}
static bool wallet_can_undo(void){return undo_available&&!wallet_same(&state.wallet,&previous.wallet);}
static void render(void);
static void render_companion(void);
#ifndef PD_UI_TEST
static bool usb_write_all(const char *data,size_t size,void *context){
    TickType_t started=*(TickType_t *)context;size_t at=0;
    while(at<size){
        if(xTaskGetTickCount()-started>pdMS_TO_TICKS(30000))return false;
        size_t chunk=size-at;if(chunk>512)chunk=512;
        int wrote=usb_serial_jtag_write_bytes(data+at,chunk,pdMS_TO_TICKS(200));if(wrote<=0)return false;at+=(size_t)wrote;
    }
    return true;
}
static void reply(const char *text){
    xSemaphoreTake(usb_output,portMAX_DELAY);TickType_t started=xTaskGetTickCount();usb_write_all(text,strlen(text),&started);xSemaphoreGive(usb_output);
}
static bool backup_send(const pd_state *s,const pd_history *h){
    xSemaphoreTake(usb_output,portMAX_DELAY);TickType_t started=xTaskGetTickCount();bool ok=pd_backup_transmit(s,h,usb_write_all,&started);xSemaphoreGive(usb_output);return ok;
}
#else
void pd_ui_reply(const char *text);
static void reply(const char *text) { pd_ui_reply(text); }
bool pd_ui_backup_send(const pd_state *s,const pd_history *h);
static bool backup_send(const pd_state *s,const pd_history *h){return pd_ui_backup_send(s,h);}
#endif
static pd_history backup_history;
static uint32_t backup_started;
static const char *backup_notice="Richiedi la copia dal PC via USB";
static bool stage_backup(const pd_backup *snapshot); /* NULL requests read-only export. */

static pd_history history;
static pd_history_entry history_unsaved;
static bool history_pending;
static void collect_diagnostics(pd_diagnostics *out){
    pd_board_diagnostics(out);
#ifndef PD_UI_TEST
    /* ESP-IDF reports the stack high-water mark in bytes, not words. */
    out->ui_stack_min=uxTaskGetStackHighWaterMark(NULL);
    out->usb_stack_min=uxTaskGetStackHighWaterMark(usb_thread);
#endif
    out->state_writable=pd_storage_writable();
    out->history_available=pd_history_store_available();
    out->history_pending=history_pending;
    out->history_matches=history.epoch==state.history_epoch;
    out->state_valid=pd_valid(&state);
    out->dimmed=display_screen.dimmed;out->screen_failed=display_screen.failed;
    out->busy=import_pending||confirmation_open()||dice_animation.active;
}
static void report_diagnostics(void){
    pd_diagnostics data;collect_diagnostics(&data);
    char message[PD_DIAGNOSTICS_MAX+32];
    const char *prefix="PDD DIAGNOSTICS ";size_t start=strlen(prefix);
    memcpy(message,prefix,start);
    size_t size=pd_diagnostics_encode(&data,message+start,PD_DIAGNOSTICS_MAX+1);
    if(!size){reply("PDD ERROR diagnostics\n");return;}
    message[start+size]='\n';message[start+size+1]=0;reply(message);
}
static unsigned history_page,history_index,history_dice_page;
static page_t history_return=HERO;
static bool journal_flush(void){
    if(!history_pending)return true;
    pd_history candidate=history;
    if(!pd_history_append(&candidate,&history_unsaved)||!pd_history_store_save(&candidate))return false;
    history=candidate;history_pending=false;return true;
}
static void journal_record(const pd_history_entry *entry){
    assert(!history_pending&&pd_history_entry_valid(entry));
    if(history.epoch!=state.history_epoch)pd_history_store_load(state.history_epoch,&history);
    history_unsaved=*entry;history_pending=true;journal_flush();
}
static void journal_d20(unsigned kind,const char *name,const pd_roll *r,int effective,const char *note){
    pd_history_entry e={.kind=kind,.count=r->count,.sides=20,.mode=effective,.bonus=r->bonus,.total=r->total};
    memcpy(e.dice,r->dice,r->count);snprintf(e.label,sizeof e.label,"%s",name);snprintf(e.note,sizeof e.note,"%s",note);journal_record(&e);
}
static void journal_pool(unsigned kind,const char *name,const pd_pool *r,const char *note){
    pd_history_entry e={.kind=kind,.count=r->count,.sides=r->sides,.bonus=r->bonus,.total=r->total,.floor_zero=r->damage};
    memcpy(e.dice,r->dice,r->count);snprintf(e.label,sizeof e.label,"%s",name);snprintf(e.note,sizeof e.note,"%s",note);journal_record(&e);
}
static const char *ui_storage_status(void){
    const char *status=pd_storage_status();
    return !strstr(status,"Errore")&&history_pending?"Tiro non registrato":status;
}
static void invalidate_strike(void) {
    strike_ready=false;
    if(combat_roll) {combat_roll=0;pool_view=false;has_roll=false;ability=-1;pd_dice_idle(&dice_animation,1);}
}
static void adopt_pending_state(void){
    clear_damage_reductions();state=pending;rest_active=false;hit_rolled=false;combat_roll=0;strike_ready=false;inventory_page=0;item_index=0;undo_available=false;resource_index=0;has_roll=false;ability=-1;mode=0;page=HERO;pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1);
}
static void apply(pd_state *next) {
    if (pd_storage_save(next)) { previous=state; undo_available=true; state=*next;invalidate_strike(); }
}
static void label_font(lv_obj_t *l,const lv_font_t *font) {
    lv_obj_set_style_text_font(l,font,0);
    /* DOT needs a fixed height: a long imported name must not wrap over the stats. */
    lv_obj_set_height(l,lv_font_get_line_height(font));
}
static lv_obj_t *label(const char *text,int x,int y,int width,bool large) {
    lv_obj_t *l=lv_label_create(lv_screen_active());
    lv_obj_set_pos(l,x,y); lv_obj_set_width(l,width);
    lv_label_set_long_mode(l,LV_LABEL_LONG_DOT); lv_label_set_text(l,text);
    lv_obj_set_style_text_align(l,LV_TEXT_ALIGN_CENTER,0);
    lv_obj_set_style_text_color(l,lv_color_hex(PD_TEXT),0);
    label_font(l,large?&lv_font_montserrat_26:&lv_font_montserrat_20);
    return l;
}
static lv_obj_t *small_label(const char *text,int x,int y,int w,unsigned color) {
    lv_obj_t *l=label(text,x,y,w,false);
    label_font(l,&lv_font_montserrat_16);
    lv_obj_set_style_text_color(l,lv_color_hex(color),0);return l;
}
/* Decoration never participates in hit testing; only explicit buttons/canvas do. */
static lv_obj_t *decoration(int x,int y,int w,int h,unsigned color,int radius) {
    lv_obj_t *o=lv_obj_create(lv_screen_active());lv_obj_remove_style_all(o);
    lv_obj_set_pos(o,x,y);lv_obj_set_size(o,w,h);
    lv_obj_set_style_bg_color(o,lv_color_hex(color),0);lv_obj_set_style_bg_opa(o,LV_OPA_COVER,0);
    lv_obj_set_style_radius(o,radius,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
static lv_obj_t *art(const lv_image_dsc_t *source,int x,int y,lv_obj_t *parent) {
    lv_obj_t *o=lv_image_create(parent);lv_image_set_src(o,source);lv_obj_set_pos(o,x,y);
    lv_obj_remove_flag(o,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    return o;
}
static bool feedback_action(intptr_t a){
    return a==2411||a==2414||a==1||a==2||a==20||a==21||a==89||a==832||a==840||a==841||a==848||a==849||a==907||a==914||a==1805||a==1865||a==2168||a==2179;
}
static void feedback_capture(feedback_snapshot *out){
    out->hp=state.hp;out->temp=state.temp_hp;out->wallet=state.wallet;unsigned n=0;
    for(unsigned i=0;i<PD_RESOURCES;i++)out->pools[n++]=i<state.resource_count?state.resources[i].current:0;
    for(unsigned i=0;i<PD_POWERS;i++)out->pools[n++]=i<state.magic.count&&(state.magic.powers[i].kind||(state.magic.powers[i].flags&PD_LIMITED))?state.magic.powers[i].remaining:0;
    for(unsigned i=0;i<9;i++){out->pools[n++]=state.magic.slots[i].current;out->pools[n++]=state.magic.pact[i].current;}
    for(unsigned i=0;i<PD_ITEMS;i++){out->pools[n++]=i<state.item_count?state.items[i].charges.current:0;out->pools[n++]=i<state.item_count?state.items[i].quantity:0;}
    for(unsigned i=0;i<PD_HIT_POOLS;i++)out->pools[n++]=i<state.recovery.count?state.recovery.pools[i].maximum-state.recovery.pools[i].used:0;
    assert(n==FEEDBACK_POOLS);
}
static void feedback_prepare(void){
    feedback_snapshot after;feedback_capture(&after);int hp=(int)after.hp-(int)feedback_before.hp,temp=(int)after.temp-(int)feedback_before.temp;
    feedback.hp_delta=hp;feedback.temp_delta=temp;
    if(hp||temp){feedback.kind=hp>0||(!hp&&temp>0)?FEEDBACK_HEAL:FEEDBACK_HURT;
        if(hp&&temp)snprintf(feedback.text,sizeof feedback.text,"PF %+d | temp %+d",hp,temp);
        else snprintf(feedback.text,sizeof feedback.text,hp?"PF %+d":"Temp %+d",hp?hp:temp);
    }else{
        bool up=false,down=false;for(unsigned i=0;i<FEEDBACK_POOLS;i++){up|=after.pools[i]>feedback_before.pools[i];down|=after.pools[i]<feedback_before.pools[i];}
        if(!up&&!down){
            if(wallet_same(&after.wallet,&feedback_before.wallet))return;
            feedback.kind=FEEDBACK_WALLET;snprintf(feedback.text,sizeof feedback.text,"Monete aggiornate");
            feedback.active=true;feedback.started=lv_tick_get();return;
        }
        feedback.kind=up&&down?FEEDBACK_MIXED:up?FEEDBACK_RECOVER:FEEDBACK_SPEND;
        snprintf(feedback.text,sizeof feedback.text,"%s",up&&down?"Risorse aggiornate":up?"Risorse recuperate":"Risorse utilizzate");
    }
    feedback.active=true;feedback.started=lv_tick_get();
}
static void feedback_show(void){
    if(!feedback.active)return;
    uint32_t elapsed=lv_tick_get()-feedback.started;
    if(import_pending||history_pending||display_screen.dimmed||strstr(pd_storage_status(),"Errore")||elapsed>=2000){feedback.active=false;return;}
    unsigned color=feedback.kind==FEEDBACK_HURT?PD_DANGER:feedback.kind==FEEDBACK_SPEND?PD_VIOLET:(feedback.kind==FEEDBACK_MIXED||feedback.kind==FEEDBACK_WALLET)?PD_GOLD:PD_MINT;
    feedback_panel=decoration(78,68,310,27,PD_INK,0);
    lv_obj_set_style_border_color(feedback_panel,lv_color_hex(color),0);lv_obj_set_style_border_width(feedback_panel,1,0);
    const lv_image_dsc_t *icon=feedback.kind==FEEDBACK_HEAL?&theme_receipt_heart:feedback.kind==FEEDBACK_HURT?&theme_receipt_shield:(feedback.kind==FEEDBACK_RECOVER||feedback.kind==FEEDBACK_WALLET)?&theme_receipt_confirm:&theme_receipt_crystal;
    art(icon,5,0,feedback_panel);
    lv_obj_t *text=small_label(feedback.text,34,3,242,color);lv_obj_set_parent(text,feedback_panel);lv_obj_set_pos(text,34,3);
    for(unsigned i=0;i<3;i++){feedback_motes[i]=decoration(280+i*7,10,2,2,color,0);lv_obj_set_parent(feedback_motes[i],feedback_panel);lv_obj_set_pos(feedback_motes[i],280+i*7,10);}
    feedback_timer=lv_timer_create(feedback_frame,state.idle_animation?66:2000-elapsed,NULL);
}
static void scene_atmosphere(lv_obj_t *stage,int x,int y){
    scene_halos[0]=art(&pd_art_lantern_halo,x+45,y+30,stage);
    scene_halos[1]=art(&pd_art_lantern_halo,x+215,y+30,stage);
    const int xy[][2]={{64,48},{234,48},{48,40},{250,36},{188,52}};
    for(unsigned i=0;i<5;i++){
        scene_lights[i]=decoration(xy[i][0],xy[i][1],2,i<2?6:2,PD_GOLD,0);
        lv_obj_set_parent(scene_lights[i],stage);lv_obj_set_pos(scene_lights[i],x+xy[i][0],y+xy[i][1]);
    }
}
static unsigned avatar_current_pose(void) {
    if(!state.idle_animation||display_screen.dimmed)return 0;
    unsigned reaction=page==HERO&&feedback.active?pd_avatar_reaction(feedback.hp_delta,feedback.temp_delta,lv_tick_get()-feedback.started):0;
    return reaction?reaction:pd_avatar_pose(lv_tick_get()-avatar_started);
}
static void avatar_set_pose(const pd_avatar *appearance,unsigned pose) {
    lv_image_cache_drop(&avatar_image);
    if(avatar_scale==4)pd_avatar_large(appearance,pose,avatar_data);else if(avatar_scale==3)pd_avatar_hero(appearance,pose,avatar_data);else pd_avatar_image(appearance,pose,avatar_data);
    avatar_phase=pose;
    if(avatar_sprite)lv_obj_invalidate(avatar_sprite);
}
static void avatar_prepare(const pd_avatar *appearance,unsigned scale) {
    avatar_scale=scale;
    avatar_image.header.w=24*scale;avatar_image.header.h=30*scale;
    avatar_image.header.stride=avatar_image.header.w;
    avatar_image.data_size=scale==4?PD_AVATAR_LARGE_BYTES:scale==3?PD_AVATAR_HERO_BYTES:PD_AVATAR_IMAGE_BYTES;
    avatar_started=lv_tick_get();avatar_set_pose(appearance,avatar_current_pose());
}
static void avatar_show(const pd_avatar *appearance,int x,int y,lv_obj_t *parent,unsigned scale,bool animate) {
    avatar_prepare(appearance,scale);
    avatar_sprite=lv_image_create(parent);lv_image_set_src(avatar_sprite,&avatar_image);
    lv_obj_set_pos(avatar_sprite,x,y);lv_obj_remove_flag(avatar_sprite,LV_OBJ_FLAG_CLICKABLE|LV_OBJ_FLAG_SCROLLABLE);
    if(animate&&state.idle_animation)avatar_timer=lv_timer_create(avatar_frame,200,NULL);
}
static void meter(unsigned current,unsigned maximum,int y,unsigned color) {
    decoration(87,y-2,292,8,PD_PANEL_LIGHT,0);
    decoration(89,y,288,4,PD_PANEL_LOW,0);
    if(current&&maximum){unsigned filled=(288*current+maximum-1)/maximum;decoration(89,y,filled,4,color,0);for(unsigned x=24;x<filled;x+=24)decoration(89+x,y,2,4,PD_PANEL_LOW,0);}
}
static lv_obj_t *button(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled);
static void frame(void) {
    pd_theme_kind kind=import_pending?PD_THEME_IMPORT:page_themes[page];
    pd_theme_begin(lv_screen_active(),kind,page==DICE&&!dice_setup&&!import_pending);
    const pd_theme_spec *theme=pd_theme_get(kind);assert(theme);
    lv_obj_t *menu=button("Menu",126,27,54,44,2300,!import_pending&&!confirmation_open()&&!dice_animation.active);
    lv_obj_set_style_text_font(menu,&lv_font_montserrat_16,0);
    lv_obj_t *title=label(theme->title,187,36,118,false);
    lv_obj_set_style_text_color(title,lv_color_hex(theme->accent),0);
    const char *status=ui_storage_status();
    unsigned c=strstr(status,"Errore")?PD_DANGER:(strstr(status,"Recuperata")||strstr(status,"Layout"))?PD_GOLD:PD_MUTED;
    small_label(status,91,73,284,c);
    feedback_show();
}
static void input_check_leave(void){
    pd_input_check_close(&input_check);physical_keys=(pd_buttons){0};
    display_screen.swallow_keys=display_screen.swallow_touch=true;
    pd_screen_activity(&display_screen,lv_tick_get());
    page=KEY_SETTINGS;render();
}
static bool input_check_service(void){
    if(!input_check_open()||!pd_input_check_tick(&input_check,lv_tick_get()))return false;
    input_check_notice="Prova scaduta; scheda conservata";input_check_leave();return true;
}
static void input_check_action(intptr_t action){
    if(import_pending||dice_animation.active)return;
    if(input_check_service())return;
    if(action==2900&&page==KEY_SETTINGS){
        pd_input_check_begin(&input_check,lv_tick_get());physical_keys=(pd_buttons){0};
        input_check_notice="PWR lungo: alimentazione";feedback.active=false;page=INPUT_CHECK_HOME;
    }else if(!input_check_open())return;
    else if(action==2901&&(page==INPUT_CHECK_HOME||page==INPUT_CHECK_TOUCH)){
        pd_input_check_select(&input_check,PD_INPUT_TOUCH,lv_tick_get());page=INPUT_CHECK_TOUCH;
    }else if(action==2902&&(page==INPUT_CHECK_HOME||page==INPUT_CHECK_KEYS)){
        pd_input_check_select(&input_check,PD_INPUT_KEYS,lv_tick_get());page=INPUT_CHECK_KEYS;
    }else if(action==2903&&(page==INPUT_CHECK_TOUCH||page==INPUT_CHECK_KEYS)){
        pd_input_check_select(&input_check,PD_INPUT_HOME,lv_tick_get());
        physical_keys=(pd_buttons){0};display_screen.swallow_keys=true;page=INPUT_CHECK_HOME;
    }else if(action==2904&&page==INPUT_CHECK_HOME){input_check_leave();return;}
    else if(action>=2908&&action<2908+PD_INPUT_CHECK_TARGETS&&page==INPUT_CHECK_TOUCH){
        if(!pd_input_check_touch(&input_check,(unsigned)(action-2908)))return;
    }else return;
    pd_screen_activity(&display_screen,lv_tick_get());render();
}
static void act(intptr_t action) {
    if(action>=2900&&action<2913){input_check_action(action);return;}
    if(input_check_open())return;
    if((action==2300||action==2301)&&(import_pending||confirmation_open()))return;
    if(import_pending&&action!=30&&action!=31)return;
    if(page==BACKUP_SEND_CONFIRM&&action!=2801&&action!=2802)return;
    if(page==BACKUP_RESTORE_CONFIRM&&action!=2801&&action!=2803)return;
    if(page==HISTORY_DISCARD&&action!=2607&&action!=2608)return;
    if(page==WALLET_EDIT&&(action<2404||action>2410))return;
    if(page==WALLET_CONFIRM&&action!=2411&&action!=2412&&action!=2409)return;
    if(page==MAGIC_STOP&&action!=852&&action!=853)return;
    if(page==MAGIC_CONFIRM&&action!=832&&action!=833)return;
    if(page==DEFENCE_CONFIRM&&action!=1816&&action!=1817&&action!=2010)return;
    if(page==DEFENCE_GRANT_REVIEW&&action!=1817&&action!=2010&&action!=2035&&action!=2036)return;
    if(page==DEFENCE_RULE_REVIEW&&action!=1817&&(action<2011||action>2014))return;
    if(page==DAMAGE_CONTEXT&&(action<2022||action>2027))return;
    if(page==EXTERNAL_TURN_HOME&&(action<3201||action>3206))return;
    if(page==DEATH_ROLL_SETUP&&(action<3101||action>3106))return;
    if(page==DAMAGE_DEATH&&action!=1930&&(action<3001||action>3004))return;
    if(page==DAMAGE_CONFIRM&&action!=3000&&action!=1804&&action!=1805&&action!=1806&&action!=1807&&action!=1860&&action!=1930&&action!=2021&&action!=2117)return;
    if(page==DAMAGE_TYPES&&action!=2050&&(action<1931||action>1933))return;
    if(page==DAMAGE_REDUCTION&&action!=2060&&(action<2042||action>2048))return;
    if(page==DAMAGE_MATH&&action!=2051&&action!=2090)return;
    if(page>=PROFILE_LIST&&page<=PROFILE_COST_IMPACT&&(action<2061||action>2110)&&!(page==PROFILE_NUMBER&&action==3212))return;
    if(page==REACTION_HOME&&(action<2118||action>2123)&&action!=3200)return;
    if(page==TIME_HOME&&(action<2141||action>2146))return;
    if(page==TIME_CONFIRM&&action!=2147&&action!=2148&&action!=2149)return;
    if(page==TIME_DETAILS&&(action<2150||action>2152))return;
    if(page==CONDITION_EFFECTS&&(action<2201||action>2206))return;
    if(page==CONDITION_EFFECTS_CONFIRM&&action!=2207&&action!=2208)return;
    if(page==ROLL_CONTEXT&&(action<2231||action>2235))return;
    if(page==ITEM_CHARGES&&(action<2161||action>2166)&&action!=2170)return;
    if(page==ITEM_RECHARGE&&(action<2171||action>2177))return;
    if(page==ITEM_RECHARGE_CONFIRM&&(action!=2178&&action!=2179))return;
    if(page==ITEM_RECHARGE_CONFIRM&&recharge_rolled&&action!=2179)return;
    if(page==ITEM_RECHARGE_RESULT&&action!=2181&&action!=100)return;
    if(page==ITEM_CHARGE_CONFIRM&&action!=2167&&action!=2168)return;
    if(page==TURN_DETAILS&&(action<2125||action>2127))return;
    if(page==REACTION_CONFIRM&&action!=2121&&action!=2122&&action!=2124)return;
    if(page==CONCENTRATION_SETUP&&(action<1861||action>1866||(concentration_rolled&&action!=1865)))return;
    if(page==EFFECT_CONFIRM&&action!=1708&&action!=1709)return;
    if(page==EFFECT_ROUND&&action!=1741&&action!=1742&&action!=1743)return;
    if(page==ROUND_DEFENCES&&(action<1744||action>1746))return;
    if(page==STATUS_CONFIRM&&action!=1505&&action!=1506)return;
    if(page==STATUS_CONFIRM&&status_command==STATUS_ROLL&&status_rolled&&action!=1505)return;
    if(page==REST_PREVIEW&&action!=902&&action!=907)return;
    if(page==HIT_CONFIRM&&action!=910&&action!=914)return;
    /* Disabled controls also reject queued/synthetic events while dice are in flight. */
    if (dice_animation.active && action!=13 && action!=16) return;
    bool new_roll=((action==13||action==72)&&!dice_animation.active)||action==46||(action==89&&item_index<state.item_count&&state.items[item_index].kind==PD_HEALING)||action==90||action==914||action==1865||action==2179||(action==1505&&status_command==STATUS_ROLL);
    if(new_roll&&history_pending){history_return=HERO;history_page=0;page=HISTORY_HOME;render();return;}
    pd_screen_activity(&display_screen,lv_tick_get());
    feedback.active=false;bool report_feedback=feedback_action(action);if(report_feedback)feedback_capture(&feedback_before);
    #define next scratch
    next=state;
    if (action>=100 && action<104) { page=action-100;if(combat_roll>=4)invalidate_strike();rest_active=false;dice_setup=false;die_picker=false; }
    else if ((action>=200&&action<206)||(action>=220&&action<238)||(action>=240&&action<246)) {
        pd_check_kind selected_kind=action>=240?PD_SAVE:action>=220?PD_SKILL:PD_CHECK;
        int index=action-(selected_kind==PD_SAVE?240:selected_kind==PD_SKILL?220:200);
        pd_check_info info;if(!pd_describe_check(&state,selected_kind,index,&info))return;
        combat_roll=0;strike_ready=false;check_kind=selected_kind;ability=index;
        page=DICE;has_roll=false;mode=0;ignore_effects=false;roll_context=(pd_roll_context){0};pool_view=false;dice_setup=false;die_picker=false;
        pd_dice_idle(&dice_animation,pd_check_mode(&state,check_kind,ability,mode,false)?2:1);
    }
    else if(action==2300)page=HOME_MENU;
    else if(action==2301)page=HOME_VITALS;
    else if(action==2800){page=BACKUP_HOME;}
    else if(action==2801&&(page==BACKUP_SEND_CONFIRM||page==BACKUP_RESTORE_CONFIRM)){pd_history_store_cancel_restore();page=BACKUP_HOME;backup_notice="Richiesta annullata";reply("PDD CANCELLED backup\n");}
    else if(action==2802&&page==BACKUP_SEND_CONFIRM){
        bool sent=backup_send(&pending,&backup_history);page=BACKUP_HOME;backup_notice=sent?"Copia inviata; verifica sul PC":"Invio fallito; scheda conservata";
        if(!sent)reply("PDD ERROR backup-send\n");
    }
    else if(action==2803&&page==BACKUP_RESTORE_CONFIRM){
        if(!pd_history_store_prepare_restore(&backup_history)){backup_notice="Errore storico: riprova";reply("PDD ERROR backup-history\n");}
        else if(!pd_storage_save(&pending)){pd_history_store_uncertain_restore();backup_notice="Errore scheda: riavvia";reply("PDD ERROR storage\n");}
        else {pd_history_store_finish_restore();history=backup_history;adopt_pending_state();history_pending=false;backup_notice="Copia ripristinata";reply("PDD RESTORED\n");}
    }
    else if(action==2600){if(page!=HISTORY_HOME&&page!=HISTORY_DETAIL)history_return=page;history_page=0;page=HISTORY_HOME;}
    else if(action==2601&&page==HISTORY_HOME&&history_page)history_page--;
    else if(action==2602&&page==HISTORY_HOME&&(history_page+1)*3<history.count+(history_pending?1u:0u))history_page++;
    else if(action==2603&&(page==HISTORY_HOME||page==HISTORY_DETAIL)){journal_flush();page=HISTORY_HOME;history_page=0;}
    else if(action==2604&&page==HISTORY_HOME)page=history_return;
    else if(action==2605&&page==HISTORY_DETAIL)page=HISTORY_HOME;
    else if(action==2606&&(page==HISTORY_HOME||page==HISTORY_DETAIL)&&history_pending)page=HISTORY_DISCARD;
    else if(action==2607&&page==HISTORY_DISCARD)page=HISTORY_HOME;
    else if(action==2608&&page==HISTORY_DISCARD){history_pending=false;page=HISTORY_HOME;}
    else if(action==2609&&page==HISTORY_HOME&&history_pending){history_index=PD_HISTORY_LIMIT;history_dice_page=0;page=HISTORY_DETAIL;}
    else if((action==2610||action==2611)&&page==HISTORY_DETAIL){if(action==2610&&history_dice_page)history_dice_page--;if(action==2611&&history_dice_page<2)history_dice_page++;}
    else if(action>=2700&&action<2700+PD_HISTORY_LIMIT&&page==HISTORY_HOME&&action-2700<history.count){history_index=action-2700;history_dice_page=0;page=HISTORY_DETAIL;}
    else if(action==2400){wallet_page=0;page=WALLET_HOME;}
    else if(action==2401&&page==WALLET_HOME&&wallet_page)wallet_page--;
    else if(action==2402&&page==WALLET_HOME&&wallet_page<1&&state.wallet.available)wallet_page++;
    else if((action==2403||(action>=2500&&action<2505))&&page==WALLET_HOME){
        wallet_origin=state.wallet;wallet_draft=state.wallet;wallet_draft.available=true;
        wallet_coin=action==2403?0:action-2500;wallet_step=0;page=WALLET_EDIT;
    }
    else if(action==2404&&page==WALLET_EDIT&&wallet_coin)wallet_coin--;
    else if(action==2405&&page==WALLET_EDIT&&wallet_coin+1<PD_COINS)wallet_coin++;
    else if(action==2406&&page==WALLET_EDIT&&wallet_draft.coins[wallet_coin]>=wallet_steps[wallet_step])wallet_draft.coins[wallet_coin]-=wallet_steps[wallet_step];
    else if(action==2407&&page==WALLET_EDIT&&wallet_draft.coins[wallet_coin]<=PD_COIN_MAX-wallet_steps[wallet_step])wallet_draft.coins[wallet_coin]+=wallet_steps[wallet_step];
    else if(action==2408&&page==WALLET_EDIT)wallet_step=(wallet_step+1)%9;
    else if(action==2409&&(page==WALLET_EDIT||page==WALLET_CONFIRM))page=WALLET_HOME;
    else if(action==2410&&page==WALLET_EDIT)page=WALLET_CONFIRM;
    else if(action==2412&&page==WALLET_CONFIRM)page=WALLET_EDIT;
    else if(action==2411&&page==WALLET_CONFIRM){
        if(!wallet_same(&wallet_origin,&state.wallet)||wallet_same(&wallet_draft,&state.wallet)||!pd_wallet_set(&next,&wallet_draft))return;
        apply(&next);if(wallet_same(&state.wallet,&wallet_draft))page=WALLET_HOME;
    }
    else if(action==2414&&page==WALLET_HOME&&wallet_can_undo()){
        if(pd_storage_save(&previous)){state=previous;undo_available=false;invalidate_strike();}
    }
    else if (action==1) { pd_hp(&next,-1);if(memcmp(&next,&state,sizeof state))apply(&next); }
    else if (action==2) { pd_hp(&next,1);if(memcmp(&next,&state,sizeof state))apply(&next); }
    else if (action==3 && undo_available) {
        if (pd_storage_save(&previous)) { state=previous; undo_available=false;invalidate_strike(); }
    }
    else if (action>=10 && action<=12) { mode=action-11;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==15) { mode=mode==0?1:mode==1?-1:0;has_roll=false;pd_dice_idle(&dice_animation,mode?2:1); }
    else if (action==13||action==72) {
        if (dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; }
        else {
            if(combat_roll==1) {
                if(!pd_attack_with_context(&state,target_armor,mode,roll_context,ignore_effects,pd_board_random,NULL,&strike))return;
                pd_attack_resolution gate;pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);
                char note[49];snprintf(note,sizeof note,"CA %u: %s%s",strike.armor,strike.hits?"colpito":"mancato",strike.critical?"; critico":"");
                journal_d20(PD_H_ATTACK,state.items[state.equipped].name,&strike.roll,gate.mode,note);
                result=strike.roll;strike_ready=true;damage_used=false;
                pd_dice_start(&dice_animation,&result,lv_tick_get());
            } else if(combat_roll>1)return;
            else if(pool_view) {
                pd_pool next_roll;
                pd_pool *last=&dice_animation.pool;
                if(!pd_roll_pool(last->sides,last->count,last->bonus,last->damage,pd_board_random,NULL,&next_roll))return;
                journal_pool(PD_H_FREE,next_roll.damage?"Danni liberi":"Somma libera",&next_roll,"Tiro libero; nessun effetto applicato");
                pd_dice_pool_start(&dice_animation,&next_roll,lv_tick_get());
            } else {
                if(ability<0)result=pd_d20(0,mode,pd_board_random,NULL);
                else if(!pd_check_roll(&state,check_kind,ability,mode,ignore_effects,roll_context,pd_board_random,NULL,&result))return;
                int effective=mode;char note[49]="Dado libero";
                if(ability>=0){pd_check_resolution gate;pd_check_resolve(&state,check_kind,ability,mode,ignore_effects,roll_context,&gate);effective=gate.mode;snprintf(note,sizeof note,"Vista %s; paura %s; effetti %s",!(roll_context.known&PD_CONTEXT_SIGHT)?"?":roll_context.values&PD_CONTEXT_SIGHT?"si":"no",!(roll_context.known&PD_CONTEXT_FEAR)?"?":roll_context.values&PD_CONTEXT_FEAR?"si":"no",ignore_effects?"esclusi":"attivi");}
                journal_d20(ability<0?PD_H_FREE:check_kind==PD_SKILL?PD_H_SKILL:check_kind==PD_SAVE?PD_H_SAVE:PD_H_CHECK,
                    ability<0?"d20 libero":check_kind==PD_SKILL?pd_skill_names[ability]:pd_abilities[ability],&result,effective,note);
                pd_dice_start(&dice_animation,&result,lv_tick_get());
            }
            page=DICE;
            has_roll=false;
        }
    }
    else if (action==16) {
        if(!dice_animation.active)return;
        pd_dice_finish(&dice_animation);has_roll=true;
    }
    else if (action==14) { combat_roll=0;strike_ready=false;ability=-1;page=DICE;has_roll=false;mode=0;ignore_effects=false;roll_context=(pd_roll_context){0};pool_view=false;dice_setup=false;die_picker=false;pd_dice_idle(&dice_animation,1); }
    else if (action==40 && !combat_roll) {
        dice_setup=true;die_picker=false;
        if(pool_view) { pool_sides=dice_animation.pool.sides;pool_count=dice_animation.pool.count;pool_bonus=dice_animation.pool.bonus;pool_damage=dice_animation.pool.damage; }
    }
    else if (action==41) { if(die_picker)die_picker=false;else dice_setup=false; }
    else if (action==42)die_picker=true;
    else if (action==43)pool_damage=!pool_damage;
    else if (action==44 && pool_count>1)pool_count--;
    else if (action==45 && pool_count<6)pool_count++;
    else if (action==47 && pool_bonus>PD_POOL_MIN_BONUS)pool_bonus--;
    else if (action==48 && pool_bonus<30)pool_bonus++;
    else if (action==46) {
        pd_pool next_roll;
        if(!pd_roll_pool(pool_sides,pool_count,pool_bonus,pool_damage,pd_board_random,NULL,&next_roll))return;
        journal_pool(PD_H_FREE,next_roll.damage?"Danni liberi":"Somma libera",&next_roll,"Tiro libero; nessun effetto applicato");
        combat_roll=0;pool_view=true;dice_setup=false;die_picker=false;has_roll=false;
        pd_dice_pool_start(&dice_animation,&next_roll,lv_tick_get());
    }
    else if(action>=404 && action<=420 && pd_pool_sides(action-400) && die_picker) { pool_sides=action-400;die_picker=false; }
    else if (action==20 || action==21) { if(pd_resource_change(&next,resource_index,action==20?-1:1))apply(&next); }
    else if (action==22 && state.resource_count) resource_index=(resource_index+1)%state.resource_count;
    else if (action==30) {
        if (pd_storage_save(&pending)) {
            pd_history_store_load(pending.history_epoch,&history);
            adopt_pending_state();
            reply("PDD IMPORTED\n"); import_pending=false;
        } else { reply("PDD ERROR storage\n");import_pending=false; }
    }
    else if (action==31) { import_pending=false;reply("PDD CANCELLED\n"); }
    else if (action==50)page=APPEARANCE;
    else if (action==51) {
        next.idle_animation=!state.idle_animation;
        if(pd_storage_save(&next)) {
            state=next;previous.idle_animation=next.idle_animation;
        }
    }
    else if (action==52)page=HERO;
    else if(action>=60&&action<=62) { sheet_tab=action-60;skill_page=0; }
    else if(action==63&&skill_page>0)skill_page--;
    else if(action==64&&skill_page<5)skill_page++;
    else if(action==70&&ability>=0&&!pool_view&&!combat_roll)page=CHECK_DETAIL;
    else if(action==71)page=DICE;
    else if(action==73) { ignore_effects=!ignore_effects;has_roll=false;pd_dice_idle(&dice_animation,1); }
    else if(action==800){page=MAGIC_LIST;}
    else if(action>=802&&action<=804){magic_filter=action-802;magic_page=0;}
    else if(action==805&&magic_page)magic_page--;
    else if(action==806)magic_page++;
    else if(action>=1000&&action<1000+PD_POWERS){
        power_index=action-1000;if(power_index>=state.magic.count)return;
        page=MAGIC_DETAIL;text_page=0;cast_level=0;cast_pact=false;
    }
    else if(action==830){page=MAGIC_TEXT;text_page=0;}
    else if(action==831&&page==MAGIC_DETAIL&&can_use_power())page=MAGIC_CONFIRM;
    else if(action==832&&page==MAGIC_CONFIRM) {
        if(!pd_magic_use(&next,power_index,cast_level,cast_pact))return;
        if(!memcmp(&next,&state,sizeof state)){page=MAGIC_DETAIL;}
        else if(pd_storage_save(&next)){previous=state;undo_available=true;state=next;invalidate_strike();page=MAGIC_DETAIL;}
    }
    else if(action==833)page=MAGIC_DETAIL;
    else if(action==834){ /* Costs are cycled in the renderer, without consuming. */
        cast_level|=0x80;
    }
    else if(action==835)page=MAGIC_LIST;
    else if(action==836||action==853)page=MAGIC_STATUS;
    else if(action==837||action==838){stop_rage=action==838;page=MAGIC_STOP;}
    else if(action==839)page=MAGIC_RECOVER;
    else if(action==840||action==841){if(pd_slot_change(&next,slot_index+1,slot_pact,action==840?-1:1))apply(&next);}
    else if(action==842&&slot_index)slot_index--;
    else if(action==843&&slot_index<8)slot_index++;
    else if(action==844||action==845)slot_pact=action==845;
    else if(action==846&&text_page)text_page--;
    else if(action==847)text_page++;
    else if(action==848||action==849){if(pd_magic_recover(&next,power_index,action==848?-1:1))apply(&next);}
    else if(action==850)page=KEY_SETTINGS;
    else if(action==851)page=MAGIC_SLOTS;
    else if(action==852&&page==MAGIC_STOP){
        if(stop_rage)pd_end_rage(&next);else pd_concentration_end(&next);
        if(pd_storage_save(&next)){previous=state;undo_available=true;state=next;invalidate_strike();page=MAGIC_STATUS;}
    }
    else if(action>=860&&action<=862) {
        unsigned i=action-860;next.shortcuts[i]=(state.shortcuts[i]+1)%14;
        if(pd_storage_save(&next)){state=next;memcpy(previous.shortcuts,next.shortcuts,3);}
    }
    else if(action==1600)page=DEVICE_SETTINGS;
    else if(action==1800){damage_plan=(pd_damage_plan){.amount=1};damage_extra_count=0;damage_context=(pd_damage_context){0};memset(damage_extra_context,0,sizeof damage_extra_context);damage_part_page=0;clear_damage_reductions();page=DAMAGE_HOME;}
    else if(action==1801&&page==DAMAGE_HOME){damage_plan.type=(damage_plan.type+1)%PD_DAMAGE_TYPES;damage_plan.concentration=0;}
    else if(action==1802&&page==DAMAGE_HOME){defence_page=damage_plan.type/3;page=DEFENCE_LIST;}
    else if(action==1803&&(page==DAMAGE_HOME||page==DAMAGE_PARTS)){damage_plan.concentration=0;damage_confirm_return=page;page=DAMAGE_CONFIRM;}
    else if(action==1804&&page==DAMAGE_CONFIRM)page=damage_confirm_return;
    else if(action==1805&&page==DAMAGE_CONFIRM){
        pd_damage_event event=current_damage_event();pd_damage_event_summary report;if(!pd_damage_event_apply(&next,&event,&report))return;pd_damage_summary r=report.total;
        bool changed=memcmp(&next,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&next)){
            incoming_death_used=false;damage_hp_before=state.hp;damage_temp_before=state.temp_hp;incoming_result=r;incoming_event=report;incoming_components=event.count;
            if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}page=DAMAGE_RESULT;
        }
    }
    else if(action==1806&&page==DAMAGE_CONFIRM){pd_damage_event event=current_damage_event();pd_damage_event_summary r;if(pd_damage_event_preview(&state,&event,&r)&&r.total.needs_concentration)damage_plan.concentration=(damage_plan.concentration+1)%3;}
    else if(action==1807&&page==DAMAGE_CONFIRM){
        pd_damage_event event=current_damage_event();pd_damage_event_summary r;if(!pd_damage_event_preview(&state,&event,&r))return;
        unsigned type=damage_plan.type;if(r.missing)for(unsigned i=0;i<PD_DAMAGE_TYPES;i++)if(r.missing&(1u<<i)){type=i;break;}
        edit_defence(type,damage_extra_count?DAMAGE_CONFIRM:DAMAGE_HOME);
    }
    else if(action==1808&&page==DAMAGE_RESULT){damage_plan.amount=1;damage_plan.concentration=0;damage_extra_count=0;damage_context=(pd_damage_context){0};memset(damage_extra_context,0,sizeof damage_extra_context);damage_part_page=0;clear_damage_reductions();page=DAMAGE_HOME;}
    else if(action==1809&&page==DICE&&combat_roll==6){page=DAMAGE_RESULT;}
    else if(action==3000&&page==DAMAGE_CONFIRM){
        pd_damage_event event=current_damage_event();pd_death_damage_summary r;
        if(!pd_death_damage_preview(&state,&event,(pd_death_damage_choice){0},&r)||!r.eligible)return;
        death_damage_choice=(pd_death_damage_choice){0};page=DAMAGE_DEATH;
    }
    else if(action==3001&&page==DAMAGE_DEATH){death_damage_choice=(pd_death_damage_choice){0};page=DAMAGE_CONFIRM;}
    else if((action==3002||action==3003)&&page==DAMAGE_DEATH){
        pd_damage_event event=current_damage_event();pd_death_damage_summary r;
        if(!pd_death_damage_preview(&state,&event,death_damage_choice,&r)||!r.eligible||!r.registry_available||r.registry_conflict||r.already_dead||r.reaction_at_zero)return;
        if(action==3002){if(!r.critical_needed)return;death_damage_choice.critical=(death_damage_choice.critical+1)%3;death_damage_choice.verified=false;}
        else death_damage_choice.verified=!death_damage_choice.verified;
    }
    else if(action==3004&&page==DAMAGE_DEATH){
        pd_damage_event event=current_damage_event();pd_death_damage_summary r;
        if(!pd_death_damage_apply(&next,&event,death_damage_choice,&r))return;
        if(pd_storage_save(&next)){
            damage_hp_before=state.hp;damage_temp_before=state.temp_hp;incoming_death=r;incoming_death_used=true;
            incoming_result=r.impact.total;incoming_event=r.impact;incoming_components=event.count;
            previous=state;undo_available=true;state=next;invalidate_strike();page=DAMAGE_RESULT;
        }
    }
    else if(action==1860&&page==DAMAGE_CONFIRM){
        pd_check_info info;pd_damage_summary summary;
        if(!current_concentration_info(0,0,&info,&summary,false))return;
        damage_plan.concentration=0;concentration_mode=0;concentration_extra=0;concentration_cap30=false;concentration_rolled=false;page=CONCENTRATION_SETUP;
    }
    else if(action==1861&&page==CONCENTRATION_SETUP)page=DAMAGE_CONFIRM;
    else if(action==1862&&page==CONCENTRATION_SETUP)concentration_mode=concentration_mode==0?1:concentration_mode==1?-1:0;
    else if((action==1863||action==1864)&&page==CONCENTRATION_SETUP){int value=concentration_extra+(action==1863?-1:1);concentration_extra=value< -30?-30:value>30?30:value;}
    else if(action==1866&&page==CONCENTRATION_SETUP)concentration_cap30=!concentration_cap30;
    else if(action==1865&&page==CONCENTRATION_SETUP){
        if(!concentration_rolled){
            pd_damage_event event=current_damage_event();
            if(!pd_concentration_event_roll(&state,&event,concentration_mode,concentration_extra,pd_board_random,NULL,&hit_candidate,&concentration_result,&concentration_event_report,concentration_cap30))return;
            concentration_damage=concentration_event_report.total;concentration_rolled=true;
        }
        next=hit_candidate;
        if(pd_storage_save(&next)){
            incoming_death_used=false;damage_hp_before=state.hp;damage_temp_before=state.temp_hp;incoming_result=concentration_damage;incoming_event=concentration_event_report;incoming_components=damage_extra_count+1;
            previous=state;undo_available=true;state=next;invalidate_strike();
            char note[49];snprintf(note,sizeof note,"CD %u; %s; PF %u",concentration_damage.concentration_dc,concentration_damage.ends_concentration?"persa":"mantenuta",state.hp);
            journal_d20(PD_H_CONCENTRATION,"Concentrazione",&concentration_result,concentration_mode,note);
            result=concentration_result;mode=concentration_mode;concentration_rolled=false;combat_roll=6;pool_view=false;has_roll=false;ability=-1;page=DICE;
            pd_dice_start(&dice_animation,&result,lv_tick_get());
        }
    }
    else if(action==1900&&page==DAMAGE_HOME){damage_part_page=0;page=DAMAGE_PARTS;}
    else if(action==1901&&page==DAMAGE_PARTS&&damage_extra_count+1<PD_DAMAGE_COMPONENTS){
        damage_part_index=damage_extra_count+1;pd_damage_event event=current_damage_event();unsigned used=0;for(unsigned i=0;i<event.count;i++)used|=1u<<event.components[i].type;
        unsigned type=0;while(used&(1u<<type))type++;damage_part_draft=(pd_damage_component){type,1};damage_part_context=(pd_damage_context){0};damage_part_reduction=0;page=DAMAGE_PART_EDIT;
    }
    else if(action==1902&&page==DAMAGE_PARTS&&damage_part_page)damage_part_page--;
    else if(action==1903&&page==DAMAGE_PARTS&&(damage_part_page+1)*3<damage_extra_count+1)damage_part_page++;
    else if(action==1904&&page==DAMAGE_PARTS)page=DAMAGE_HOME;
    else if(action>=1910&&action<1910+PD_DAMAGE_COMPONENTS&&page==DAMAGE_PARTS&&action-1910<damage_extra_count+1){damage_part_index=action-1910;damage_part_draft=damage_part_index?damage_extra[damage_part_index-1]:(pd_damage_component){damage_plan.type,damage_plan.amount};damage_part_context=damage_part_index?damage_extra_context[damage_part_index-1]:damage_context;damage_part_reduction=damage_reductions[damage_part_index];page=DAMAGE_PART_EDIT;}
    else if(action==1920&&page==DAMAGE_PART_EDIT)damage_part_draft.type=(damage_part_draft.type+1)%PD_DAMAGE_TYPES;
    else if(action>=1921&&action<=1924&&page==DAMAGE_PART_EDIT){int amount=(int)damage_part_draft.amount+(const int[]){-10,-1,1,10}[action-1921];damage_part_draft.amount=amount<1?1:amount>9999?9999:amount;}
    else if(action==1925&&page==DAMAGE_PART_EDIT){
        if(!damage_part_index){damage_plan.type=damage_part_draft.type;damage_plan.amount=damage_part_draft.amount;damage_context=damage_part_context;}
        else if(damage_part_index<=damage_extra_count+1&&damage_part_index<PD_DAMAGE_COMPONENTS){if(damage_part_index==damage_extra_count+1)damage_allocation[damage_part_index]=0;damage_extra[damage_part_index-1]=damage_part_draft;damage_extra_context[damage_part_index-1]=damage_part_context;if(damage_part_index==damage_extra_count+1)damage_extra_count++;}
        else return;
        damage_reductions[damage_part_index]=damage_part_reduction;damage_plan.concentration=0;damage_part_page=damage_part_index/3;page=DAMAGE_PARTS;
    }
    else if(action==1926&&page==DAMAGE_PART_EDIT)page=DAMAGE_PARTS;
    else if(action==1927&&page==DAMAGE_PART_EDIT&&damage_extra_count&&damage_part_index<=damage_extra_count){
        memmove(damage_allocation+damage_part_index,damage_allocation+damage_part_index+1,(damage_extra_count-damage_part_index)*sizeof *damage_allocation);damage_allocation[damage_extra_count]=0;
        memmove(damage_reductions+damage_part_index,damage_reductions+damage_part_index+1,(damage_extra_count-damage_part_index)*sizeof *damage_reductions);damage_reductions[damage_extra_count]=0;
        unsigned remove=damage_part_index?damage_part_index-1:0;
        if(!damage_part_index){damage_plan.type=damage_extra[0].type;damage_plan.amount=damage_extra[0].amount;damage_context=damage_extra_context[0];}
        memmove(damage_extra_context+remove,damage_extra_context+remove+1,(damage_extra_count-remove-1)*sizeof *damage_extra_context);
        memmove(damage_extra+remove,damage_extra+remove+1,(damage_extra_count-remove-1)*sizeof *damage_extra);memset(&damage_extra[--damage_extra_count],0,sizeof *damage_extra);memset(&damage_extra_context[damage_extra_count],0,sizeof *damage_extra_context);
        damage_plan.concentration=0;page=DAMAGE_PARTS;
    }
    else if(action==1928&&page==DAMAGE_PART_EDIT)edit_defence(damage_part_draft.type,DAMAGE_PART_EDIT);
    else if(action==1930&&(page==DAMAGE_CONFIRM||page==DAMAGE_DEATH||page==DAMAGE_RESULT)){
        damage_details_return=page;damage_type_page=0;
        if(page==DAMAGE_RESULT)damage_details=incoming_event;
        else{pd_damage_event event=current_damage_event();if(!pd_damage_event_preview(&state,&event,&damage_details))return;}
        page=DAMAGE_TYPES;
    }
    else if(action==1931&&page==DAMAGE_TYPES)page=damage_details_return;
    else if(action==1932&&page==DAMAGE_TYPES&&damage_type_page)damage_type_page--;
    else if(action==1933&&page==DAMAGE_TYPES&&(damage_type_page+1)*3<damage_details.count)damage_type_page++;
    else if((action==2040&&page==DAMAGE_HOME)||(action==2041&&page==DAMAGE_PART_EDIT)){
        reduction_return=page;reduction_profile_draft=damage_profile;memcpy(reduction_allocation_draft,damage_allocation,sizeof damage_allocation);memcpy(reduction_drafts,damage_reductions,sizeof damage_reductions);reduction_drafts[PD_DAMAGE_COMPONENTS]=damage_reduction_after;
        reduction_target=page==DAMAGE_PART_EDIT?damage_part_index:PD_DAMAGE_COMPONENTS;if(page==DAMAGE_PART_EDIT)reduction_drafts[reduction_target]=damage_part_reduction;page=DAMAGE_REDUCTION;
    }
    else if(action==2042&&page==DAMAGE_REDUCTION&&reduction_return!=DAMAGE_PART_EDIT)reduction_target=reduction_target==PD_DAMAGE_COMPONENTS?0:reduction_target==damage_extra_count?PD_DAMAGE_COMPONENTS:reduction_target+1;
    else if(action>=2043&&action<=2046&&page==DAMAGE_REDUCTION){int value=(int)reduction_drafts[reduction_target]+(const int[]){-10,-1,1,10}[action-2043];reduction_drafts[reduction_target]=value<0?0:value>9999?9999:value;}
    else if(action==2047&&page==DAMAGE_REDUCTION)page=reduction_return;
    else if(action==2048&&page==DAMAGE_REDUCTION){
        if(reduction_return==DAMAGE_PART_EDIT)damage_part_reduction=reduction_drafts[reduction_target];
        else{damage_profile=reduction_profile_draft;memcpy(damage_allocation,reduction_allocation_draft,sizeof damage_allocation);memcpy(damage_reductions,reduction_drafts,sizeof damage_reductions);damage_reduction_after=reduction_drafts[PD_DAMAGE_COMPONENTS];}
        damage_plan.concentration=0;page=reduction_return;
    }
    else if(action==2060&&page==DAMAGE_REDUCTION&&reduction_return!=DAMAGE_PART_EDIT)page=PROFILE_LIST;
    else if(action>=2061&&action<=2063&&page==PROFILE_LIST){profile_index=action-2061;page=PROFILE_VIEW;}
    else if(action==2064&&page==PROFILE_LIST)page=DAMAGE_REDUCTION;
    else if(action==2065&&page==PROFILE_LIST){reduction_profile_draft=0;memset(reduction_allocation_draft,0,sizeof reduction_allocation_draft);page=DAMAGE_REDUCTION;damage_plan.concentration=0;}
    else if(action==2066&&page==PROFILE_VIEW)page=PROFILE_LIST;
    else if(action==2067&&page==PROFILE_VIEW){profile_draft=state.reductions[profile_index];page=PROFILE_EDIT;}
    else if(action==2068&&page==PROFILE_VIEW&&state.reductions[profile_index].active){
        if(reduction_profile_draft==profile_index+1&&!state.reductions[profile_index].stage)memcpy(allocation_draft,reduction_allocation_draft,sizeof allocation_draft);else memset(allocation_draft,0,sizeof allocation_draft);allocation_row=0;page=PROFILE_ALLOCATE;
    }
    else if(action==2069&&page==PROFILE_EDIT){profile_number_cost=false;profile_number_rounds=false;page=PROFILE_NUMBER;}
    else if(action==2070&&page==PROFILE_EDIT){unsigned types=profile_draft.types;if(!types)types=8191;else if(types==8191)types=7;else if(types==7)types=1;else if(types<4096&&(types&(types-1))==0)types<<=1;else types=8191;profile_draft.types=types;}
    else if(action==2071&&page==PROFILE_EDIT)profile_draft.stage=!profile_draft.stage;
    else if(action==2072&&page==PROFILE_EDIT)profile_draft.active=!profile_draft.active;
    else if(action==2073&&page==PROFILE_EDIT)page=PROFILE_VIEW;
    else if(action==2074&&page==PROFILE_EDIT)page=PROFILE_DETAILS;
    else if(action==2075&&(page==PROFILE_EDIT||page==PROFILE_DETAILS)){profile_confirm_tab=0;profile_cost_row=0;page=PROFILE_CONFIRM;}
    else if(action==2076&&page==PROFILE_DETAILS)profile_draft.rule=(profile_draft.rule+1)%PD_RULE_COUNT;
    else if(action==2077&&page==PROFILE_DETAILS){duration_next(&profile_draft.timed,&profile_draft.tick,&profile_draft.rounds,&profile_draft.minutes,&profile_draft.owner);}
    else if(action==2078&&page==PROFILE_DETAILS&&(profile_draft.concentration||state.magic.concentration))profile_draft.concentration=profile_draft.concentration?0:state.magic.concentration;
    else if(action==2079&&page==PROFILE_DETAILS&&profile_draft.timed){profile_number_cost=false;profile_number_rounds=true;page=PROFILE_NUMBER;}
    else if(action==2080&&page==PROFILE_DETAILS)page=PROFILE_EDIT;
    else if(action>=2081&&action<=2084&&page==PROFILE_ALLOCATE&&!state.reductions[profile_index].stage){
        unsigned other=allocation_sum()-allocation_draft[allocation_row],budget=state.reductions[profile_index].value;unsigned maximum=budget>other?budget-other:0;
        int value=(int)allocation_draft[allocation_row]+(const int[]){-10,-1,1,10}[action-2081];allocation_draft[allocation_row]=value<0?0:(unsigned)value>maximum?maximum:(unsigned)value;
    }
    else if(action==2085&&page==PROFILE_ALLOCATE)page=PROFILE_VIEW;
    else if(action==2088&&page==PROFILE_ALLOCATE){pd_reduction_profile *p=&state.reductions[profile_index];unsigned sum=allocation_sum();if(!p->active||(!p->stage&&(!sum||sum>p->value)))return;reduction_profile_draft=profile_index+1;memcpy(reduction_allocation_draft,allocation_draft,sizeof reduction_allocation_draft);damage_plan.concentration=0;page=DAMAGE_REDUCTION;}
    else if(action>=2081&&action<=2084&&page==PROFILE_NUMBER){if(profile_number_rounds){duration_adjust(profile_draft.tick,&profile_draft.rounds,&profile_draft.minutes,action-2081);}else{int value=(profile_number_cost?pd_profile_cost(&profile_draft,profile_cost_row).amount:profile_number_rounds?profile_draft.rounds:profile_draft.value)+(const int[]){-10,-1,1,10}[action-2081];unsigned maximum=profile_number_cost?pd_reduction_cost_max(&state,pd_profile_cost(&profile_draft,profile_cost_row).kind,pd_profile_cost(&profile_draft,profile_cost_row).index):profile_number_rounds?99:9999;value=value<0?0:(unsigned)value>maximum?(int)maximum:value;if(profile_number_cost){pd_cost c=pd_profile_cost(&profile_draft,profile_cost_row);c.amount=value;pd_profile_cost_set(&profile_draft,profile_cost_row,c);}else if(profile_number_rounds)profile_draft.rounds=value;else profile_draft.value=value;}}
    else if(action==2085&&page==PROFILE_NUMBER)page=profile_number_cost?PROFILE_COST:profile_number_rounds?PROFILE_DETAILS:PROFILE_EDIT;
    else if(action==2086&&page==PROFILE_CONFIRM)profile_confirm_tab=(profile_confirm_tab+1)%4;
    else if(action==2087&&page==PROFILE_CONFIRM)page=PROFILE_EDIT;
    else if(action==2088&&page==PROFILE_CONFIRM){if(!preview_profile(&next))return;bool changed=memcmp(&next,&state,sizeof state)!=0;if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}damage_plan.concentration=0;page=PROFILE_LIST;}}
    else if(action==2089&&page==PROFILE_ALLOCATE)allocation_row=(allocation_row+1)%(damage_extra_count+1);
    else if(action==2091&&page==PROFILE_IMPACT)page=DAMAGE_MATH;
    else if(action==2090&&page==DAMAGE_MATH&&damage_details.profile){profile_impact_row=0;page=PROFILE_IMPACT;}
    else if(action==2089&&page==PROFILE_IMPACT&&!damage_details.profile_snapshot.stage)profile_impact_row=(profile_impact_row+1)%damage_details.profile_count;
    else if(action==2100&&(page==PROFILE_VIEW||page==PROFILE_DETAILS)){profile_cost_editing=page==PROFILE_DETAILS;profile_cost_row=0;page=PROFILE_COST;}
    else if(action==2101&&page==PROFILE_COST&&profile_cost_editing)cost_next();
    else if(action==2102&&page==PROFILE_COST&&profile_cost_editing&&pd_profile_cost(&profile_draft,profile_cost_row).kind){profile_number_cost=true;profile_number_rounds=false;page=PROFILE_NUMBER;}
    else if(action==2103&&page==PROFILE_COST&&profile_cost_editing)profile_draft.reaction=!profile_draft.reaction;
    else if(action==2104&&page==PROFILE_COST)page=profile_cost_editing?PROFILE_DETAILS:PROFILE_VIEW;
    else if(action==2105&&page==PROFILE_IMPACT){profile_cost_row=0;page=PROFILE_COST_IMPACT;}
    else if((action==2107&&page==PROFILE_COST)||(action==2108&&page==PROFILE_COST_IMPACT)||(action==2109&&page==PROFILE_CONFIRM&&profile_confirm_tab==3))profile_cost_row=(profile_cost_row+1)%PD_PROFILE_COSTS;
    else if(action==2106&&page==PROFILE_COST_IMPACT)page=PROFILE_IMPACT;
    else if(action==2140&&page==STATUS_HOME){time_minutes=1;page=TIME_HOME;}
    else if(action==2141&&page==TIME_HOME)page=STATUS_HOME;
    else if(action>=2142&&action<=2145&&page==TIME_HOME){int n=time_minutes+(const int[]){-60,-1,1,60}[action-2142];time_minutes=n<1?1:n>5940?5940:n;}
    else if(action==2146&&page==TIME_HOME){time_detail_page=0;page=TIME_CONFIRM;}
    else if(action==2147&&page==TIME_CONFIRM)page=TIME_HOME;
    else if(action==2148&&page==TIME_CONFIRM){pd_round_summary r;if(!preview_time(&next,&r))return;bool changed=memcmp(&next,&state,sizeof state)!=0;if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}page=TIME_HOME;}}
    else if(action==2149&&page==TIME_CONFIRM){pd_round_summary r;if(preview_time(&next,&r)&&(r.effects_advanced||r.defences_advanced))page=TIME_DETAILS;}
    else if(action==2150&&page==TIME_DETAILS)page=TIME_CONFIRM;
    else if(action==2151&&page==TIME_DETAILS&&time_detail_page)time_detail_page--;
    else if(action==2152&&page==TIME_DETAILS){pd_round_summary r;if(preview_time(&next,&r)&&(time_detail_page+1)*3<bit_count(r.effects_advanced)+bit_count(r.defences_advanced))time_detail_page++;}
    else if(action==2200&&page==CONDITION_LIST){condition_effects_draft=state.roll_conditions;page=CONDITION_EFFECTS;}
    else if(action>=2201&&action<=2204&&page==CONDITION_EFFECTS)condition_effects_draft^=1u<<(action-2201);
    else if(action==2205&&page==CONDITION_EFFECTS)page=CONDITION_LIST;
    else if(action==2206&&page==CONDITION_EFFECTS)page=CONDITION_EFFECTS_CONFIRM;
    else if(action==2207&&page==CONDITION_EFFECTS_CONFIRM)page=CONDITION_EFFECTS;
    else if(action==2208&&page==CONDITION_EFFECTS_CONFIRM){if(!pd_condition_effects_set(&next,condition_effects_draft))return;bool changed=memcmp(&next,&state,sizeof state)!=0;if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();has_roll=false;roll_context=(pd_roll_context){0};ignore_effects=false;}page=CONDITION_EFFECTS;}}
    else if(action==2230&&(page==COMBAT||page==COMBAT_DETAIL||page==CHECK_DETAIL||(page==DICE&&(combat_roll==1||(!combat_roll&&!pool_view&&ability>=0))))&&state.roll_conditions&&!dice_animation.active){context_return=page;context_draft=roll_context;context_ignore_draft=ignore_effects;page=ROLL_CONTEXT;}
    else if(action==2231&&page==ROLL_CONTEXT)page=context_return;
    else if((action==2232||action==2233)&&page==ROLL_CONTEXT){unsigned bit=action==2232?PD_CONTEXT_SIGHT:PD_CONTEXT_FEAR;pd_condition_result effect;if(current_condition_context(context_draft,context_ignore_draft,&effect)&&context_relevant(bit)){if(!(context_draft.known&bit))context_draft.known|=bit;else if(!(context_draft.values&bit))context_draft.values|=bit;else{context_draft.known&=~bit;context_draft.values&=~bit;}}}
    else if(action==2234&&page==ROLL_CONTEXT)context_ignore_draft=!context_ignore_draft;
    else if(action==2235&&page==ROLL_CONTEXT){pd_condition_result effect;if(!current_condition_context(context_draft,context_ignore_draft,&effect)||effect.needed)return;roll_context=context_draft;ignore_effects=context_ignore_draft;has_roll=false;strike_ready=false;damage_used=false;page=context_return;}
    else if(action==2170&&page==ITEM_CHARGES&&recharge_available()){recharge_plan=(pd_recharge_plan){.count=1,.sides=6};recharge_rolled=false;page=ITEM_RECHARGE;}
    else if(action==2171&&page==ITEM_RECHARGE)page=ITEM_CHARGES;
    else if(action==2172&&page==ITEM_RECHARGE){const unsigned sides[]={4,6,8,10,12,20};for(unsigned i=0;i<6;i++)if(recharge_plan.sides==sides[i]){recharge_plan.sides=sides[(i+1)%6];break;}}
    else if(action==2173&&page==ITEM_RECHARGE&&recharge_plan.count>1)recharge_plan.count--;
    else if(action==2174&&page==ITEM_RECHARGE&&recharge_plan.count<PD_RECHARGE_MAX_DICE)recharge_plan.count++;
    else if(action==2175&&page==ITEM_RECHARGE&&recharge_plan.bonus>PD_POOL_MIN_BONUS)recharge_plan.bonus--;
    else if(action==2176&&page==ITEM_RECHARGE&&recharge_plan.bonus<PD_POOL_MAX_BONUS)recharge_plan.bonus++;
    else if(action==2177&&page==ITEM_RECHARGE&&recharge_available()){recharge_rolled=false;page=ITEM_RECHARGE_CONFIRM;}
    else if(action==2178&&page==ITEM_RECHARGE_CONFIRM&&!recharge_rolled)page=ITEM_RECHARGE;
    else if(action==2179&&page==ITEM_RECHARGE_CONFIRM){
        if(!recharge_rolled){if(!pd_item_recharge(&state,item_index,&recharge_plan,pd_board_random,NULL,&hit_candidate,&recharge_result,&recharge_gained))return;
            recharge_before=state.items[item_index].charges.current;recharge_after=hit_candidate.items[item_index].charges.current;recharge_maximum=state.items[item_index].charges.maximum;recharge_index=item_index;strcpy(recharge_name,state.items[item_index].name);recharge_rolled=true;}
        bool changed=memcmp(&hit_candidate,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&hit_candidate)){if(changed){previous=state;undo_available=true;state=hit_candidate;invalidate_strike();}
            char note[49];snprintf(note,sizeof note,"Cariche %u -> %u/%u",recharge_before,recharge_after,recharge_maximum);
            journal_pool(PD_H_RECHARGE,recharge_name,&recharge_result,note);
            recharge_rolled=false;combat_roll=7;pool_view=true;has_roll=false;ability=-1;page=DICE;pd_dice_pool_start(&dice_animation,&recharge_result,lv_tick_get());}
    }
    else if(action==2180&&page==DICE&&combat_roll==7&&has_roll&&!dice_animation.active)page=ITEM_RECHARGE_RESULT;
    else if(action==2181&&page==ITEM_RECHARGE_RESULT){item_index=recharge_index;charge_draft=state.items[item_index].charges.current;page=ITEM_CHARGES;}
    else if(action==2160&&page==ITEM_DETAIL&&state.items[item_index].charges.maximum){charge_draft=state.items[item_index].charges.current;page=ITEM_CHARGES;}
    else if(action==2161&&page==ITEM_CHARGES)page=ITEM_DETAIL;
    else if(action>=2162&&action<=2165&&page==ITEM_CHARGES&&state.items[item_index].quantity){int n=(int)charge_draft+(const int[]){-10,-1,1,10}[action-2162];charge_draft=n<0?0:n>state.items[item_index].charges.maximum?state.items[item_index].charges.maximum:n;}
    else if(action==2166&&page==ITEM_CHARGES&&pd_item_charges_change(&next,item_index,(int)charge_draft-state.items[item_index].charges.current))page=ITEM_CHARGE_CONFIRM;
    else if(action==2167&&page==ITEM_CHARGE_CONFIRM)page=ITEM_CHARGES;
    else if(action==2168&&page==ITEM_CHARGE_CONFIRM){if(!pd_item_charges_change(&next,item_index,(int)charge_draft-state.items[item_index].charges.current))return;bool changed=memcmp(&next,&state,sizeof state)!=0;if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}page=ITEM_CHARGES;}}
    else if(action==2117&&(page==STATUS_HOME||page==DAMAGE_CONFIRM)){if(page==DAMAGE_CONFIRM){pd_damage_event e=current_damage_event();pd_damage_event_summary r;if(!pd_damage_event_preview(&state,&e,&r)||!r.reaction_missing)return;}reaction_return=page;turn_owner=0;page=REACTION_HOME;}
    else if(action==2118&&page==REACTION_HOME)page=reaction_return;
    else if((action==2119||action==2120||action==2123)&&page==REACTION_HOME){if(action==2119&&state.reaction_spent)return;turn_owner=0;reaction_draft=action==2119;turn_end=action==2123;turn_detail_page=0;page=REACTION_CONFIRM;}
    else if(action==2121&&page==REACTION_CONFIRM)page=turn_owner?EXTERNAL_TURN_HOME:REACTION_HOME;
    else if(action==2122&&page==REACTION_CONFIRM){pd_round_summary r;if(reaction_draft?!pd_reaction_change(&next,true):!preview_turn(&next,&r))return;bool changed=memcmp(&next,&state,sizeof state)!=0;if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}page=turn_owner?EXTERNAL_TURN_HOME:REACTION_HOME;}}
    else if(action==3200&&page==REACTION_HOME){turn_owner=1;page=EXTERNAL_TURN_HOME;}
    else if(page==EXTERNAL_TURN_HOME&&action>=3201&&action<=3206){
        if(action<=3203)turn_owner=action-3200;
        else if(action==3206){turn_owner=0;page=REACTION_HOME;}
        else{reaction_draft=false;turn_end=action==3205;turn_detail_page=0;page=REACTION_CONFIRM;}
    }
    else if(action==3210&&page==EFFECT_DURATION&&effect_draft.timed&&turn_clock(effect_draft.tick)){effect_draft.owner=(effect_draft.owner+1)%PD_TURN_SUBJECTS;effect_draft.rounds=1;}
    else if(action==3211&&page==DEFENCE_DURATION&&defence_temporary_draft.timed&&turn_clock(defence_temporary_draft.tick)){defence_temporary_draft.owner=(defence_temporary_draft.owner+1)%PD_TURN_SUBJECTS;defence_temporary_draft.rounds=1;}
    else if(action==3212&&page==PROFILE_NUMBER&&profile_number_rounds&&profile_draft.timed&&turn_clock(profile_draft.tick)){profile_draft.owner=(profile_draft.owner+1)%PD_TURN_SUBJECTS;profile_draft.rounds=1;}
    else if(action==2124&&page==REACTION_CONFIRM&&!reaction_draft){pd_round_summary r;if(preview_turn(&next,&r)&&(r.effects_advanced||r.defences_advanced))page=TURN_DETAILS;}
    else if(action==2125&&page==TURN_DETAILS)page=REACTION_CONFIRM;
    else if(action==2126&&page==TURN_DETAILS&&turn_detail_page)turn_detail_page--;
    else if(action==2127&&page==TURN_DETAILS){pd_round_summary r;if(preview_turn(&next,&r)&&(turn_detail_page+1)*3<bit_count(r.effects_advanced)+bit_count(r.defences_advanced))turn_detail_page++;}
    else if(action==2050&&page==DAMAGE_TYPES)page=DAMAGE_MATH;
    else if(action==2051&&page==DAMAGE_MATH)page=DAMAGE_TYPES;
    else if(action==2020&&page==DAMAGE_PART_EDIT){source_context_draft=damage_part_context;source_context_return=DAMAGE_PART_EDIT;page=DAMAGE_CONTEXT;}
    else if(action==2021&&page==DAMAGE_CONFIRM){
        pd_damage_event event=current_damage_event();pd_damage_event_summary report;if(!pd_damage_event_preview(&state,&event,&report)||!report.context_missing)return;
        damage_part_index=0;while(!(report.context_missing&(1u<<damage_part_index)))damage_part_index++;
        damage_part_draft=event.components[damage_part_index];damage_part_context=event.contexts[damage_part_index];source_context_draft=damage_part_context;source_context_return=DAMAGE_CONFIRM;page=DAMAGE_CONTEXT;
    }
    else if(action==2022&&page==DAMAGE_CONTEXT)page=source_context_return;
    else if(action==2023&&page==DAMAGE_CONTEXT){
        damage_part_context=source_context_draft;
        if(source_context_return==DAMAGE_CONFIRM){if(!damage_part_index)damage_context=damage_part_context;else damage_extra_context[damage_part_index-1]=damage_part_context;damage_plan.concentration=0;}
        page=source_context_return;
    }
    else if(action>=2024&&action<=2027&&page==DAMAGE_CONTEXT){unsigned bit=1u<<(action-2024);if(!(source_context_draft.known&bit)){source_context_draft.known|=bit;source_context_draft.values&=~bit;}else if(!(source_context_draft.values&bit))source_context_draft.values|=bit;else{source_context_draft.known&=~bit;source_context_draft.values&=~bit;}}
    else if((action==2000&&page==DEFENCE_EDIT)||(action==2001&&page==DEFENCE_TEMPORARY)){
        defence_rule_temporary=action==2001;unsigned flags=defence_rule_temporary?defence_temporary_draft.flags:defence_flags;defence_rule_property=0;
        for(unsigned i=0;i<3;i++){if(flags&(1u<<i)){defence_rule_property=i;break;}}
        page=DEFENCE_RULE;
    }
    else if(action==2002&&page==DEFENCE_RULE)page=defence_rule_temporary?DEFENCE_TEMPORARY:DEFENCE_EDIT;
    else if(action>=2006&&action<=2008&&page==DEFENCE_RULE){unsigned flags=defence_rule_temporary?defence_temporary_draft.flags:defence_flags;if(flags&(1u<<(action-2006)))defence_rule_property=action-2006;}
    else if(action==2004&&page==DEFENCE_RULE){unsigned flags=defence_rule_temporary?defence_temporary_draft.flags:defence_flags;uint8_t *rules=defence_rule_temporary?defence_temporary_draft.conditions:defence_conditions;if(flags&(1u<<defence_rule_property))rules[defence_rule_property]=(rules[defence_rule_property]+1)%PD_RULE_COUNT;}
    else if(action==2005&&page==DEFENCE_RULE){defence_confirm_return=DEFENCE_RULE;rule_review_return=DEFENCE_RULE;rule_review_temporary=defence_rule_temporary;rule_review_property=defence_rule_property;page=DEFENCE_RULE_REVIEW;}
    else if(action==2010&&(page==DEFENCE_CONFIRM||page==DEFENCE_GRANT_REVIEW)){rule_review_temporary=page==DEFENCE_GRANT_REVIEW;rule_review_property=0;rule_review_return=page;page=DEFENCE_RULE_REVIEW;}
    else if((action==2011||action==2012)&&page==DEFENCE_RULE_REVIEW){if(action==2012&&rule_review_temporary)defence_slot=(defence_slot+1)%PD_DEFENCE_SLOTS;rule_review_temporary=action==2012;}
    else if(action==2013&&page==DEFENCE_RULE_REVIEW)rule_review_property=(rule_review_property+1)%3;
    else if(action==2014&&page==DEFENCE_RULE_REVIEW)page=rule_review_return;
    else if(action>=1810&&action<=1813&&page==DEFENCE_EDIT){
        if(action==1813){defence_configured=!defence_configured;if(!defence_configured){defence_flags=0;memset(defence_conditions,0,3);}}
        else {defence_flags^=1u<<(action-1810);if(!(defence_flags&(1u<<(action-1810))))defence_conditions[action-1810]=0;defence_configured=true;}
    }
    else if(action==1814&&page==DEFENCE_EDIT)page=defence_return_page;
    else if(action==1815&&page==DEFENCE_EDIT){defence_confirm_return=DEFENCE_EDIT;page=DEFENCE_CONFIRM;}
    else if(action==1816&&page==DEFENCE_CONFIRM)page=defence_confirm_return;
    else if(action==1817&&(page==DEFENCE_CONFIRM||page==DEFENCE_RULE_REVIEW||page==DEFENCE_GRANT_REVIEW)){
        if(!preview_defence(&next))return;
        bool changed=memcmp(&next,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&next)){if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}damage_plan.concentration=0;page=defence_return_page;}
    }
    else if(action==2030&&page==DEFENCE_TEMPORARY)page=DEFENCE_GRANTS;
    else if(action>=2031&&action<2031+PD_DEFENCE_SLOTS&&page==DEFENCE_GRANTS){defence_slot=action-2031;page=DEFENCE_TEMPORARY;}
    else if(action==2034&&page==DEFENCE_GRANTS)page=DEFENCE_EDIT;
    else if(action==2035&&(page==DEFENCE_GRANTS||page==DEFENCE_GRANT_REVIEW))page=page==DEFENCE_GRANTS?DEFENCE_GRANT_REVIEW:DEFENCE_GRANTS;
    else if(action==2036&&page==DEFENCE_GRANT_REVIEW)defence_slot=(defence_slot+1)%PD_DEFENCE_SLOTS;
    else if(action==1870&&page==DEFENCE_EDIT)page=DEFENCE_TEMPORARY;
    else if(action>=1871&&action<=1874&&page==DEFENCE_TEMPORARY){if(action==1874)defence_temporary_draft.active=!defence_temporary_draft.active;else{defence_temporary_draft.flags^=1u<<(action-1871);if(!(defence_temporary_draft.flags&(1u<<(action-1871))))defence_temporary_draft.conditions[action-1871]=0;}}
    else if(action==1875&&page==DEFENCE_TEMPORARY)page=DEFENCE_EDIT;
    else if(action==1876&&page==DEFENCE_TEMPORARY)page=DEFENCE_DURATION;
    else if(action==1877&&page==DEFENCE_DURATION){duration_next(&defence_temporary_draft.timed,&defence_temporary_draft.tick,&defence_temporary_draft.rounds,&defence_temporary_draft.minutes,&defence_temporary_draft.owner);}
    else if(action==1878&&page==DEFENCE_DURATION)page=DEFENCE_LINK;
    else if(action==1879&&page==DEFENCE_DURATION)page=DEFENCE_TEMPORARY;
    else if(action>=1880&&action<=1883&&page==DEFENCE_DURATION&&defence_temporary_draft.timed)duration_adjust(defence_temporary_draft.tick,&defence_temporary_draft.rounds,&defence_temporary_draft.minutes,action-1880);
    else if(action==1884&&page==DEFENCE_LINK&&(defence_temporary_draft.concentration||state.magic.concentration))defence_temporary_draft.concentration=defence_temporary_draft.concentration?0:state.magic.concentration;
    else if(action==1885&&page==DEFENCE_LINK)page=DEFENCE_DURATION;
    else if(action==1887&&(page==DEFENCE_TEMPORARY||page==DEFENCE_DURATION||page==DEFENCE_LINK)){defence_confirm_return=page;page=DEFENCE_CONFIRM;}
    else if(action>=1820&&action<1820+PD_DAMAGE_TYPES)edit_defence(action-1820,DEFENCE_LIST);
    else if(action>=1840&&action<=1843&&page==DAMAGE_HOME){int amount=(int)damage_plan.amount+(const int[]){-10,-1,1,10}[action-1840];damage_plan.amount=amount<1?1:amount>9999?9999:amount;damage_plan.concentration=0;}
    else if(action==1850&&defence_page)defence_page--;
    else if(action==1851&&(defence_page+1)*3<PD_DAMAGE_TYPES)defence_page++;
    else if(action==1852&&page==DEFENCE_LIST)page=DAMAGE_HOME;
    else if(action==1700)page=EFFECT_LIST;
    else if(action==1701&&state.effect_count<PD_EFFECTS){effect_index=state.effect_count;effect_draft=(pd_effect){.active=true};effect_removing=false;page=EFFECT_EDIT;}
    else if(action>=1720&&action<1720+PD_EFFECTS&&action-1720<state.effect_count){effect_index=action-1720;effect_draft=state.effects[effect_index];effect_removing=false;page=EFFECT_EDIT;}
    else if(action==1702&&page==EFFECT_EDIT){effect_draft.target=!effect_draft.target;effect_draft.value=0;}
    else if(action>=1730&&action<=1733&&page==EFFECT_EDIT){
        int delta=(const int[]){-10,-1,1,10}[action-1730],bound=effect_draft.target==PD_EFFECT_ARMOR?20:999;
        int value=effect_draft.value+delta;effect_draft.value=value< -bound?-bound:value>bound?bound:value;
    }
    else if(action==1703&&page==EFFECT_EDIT)effect_draft.active=!effect_draft.active;
    else if((action==1704||action==1705)&&(page==EFFECT_EDIT||page==EFFECT_DURATION||page==EFFECT_LINK)){effect_removing=action==1705;if(!effect_removing||effect_index<state.effect_count)page=EFFECT_CONFIRM;}
    else if(action==1706&&effect_page)effect_page--;
    else if(action==1707&&(effect_page+1)*3<state.effect_count)effect_page++;
    else if(action==1708&&page==EFFECT_CONFIRM)page=EFFECT_EDIT;
    else if(action==1710&&page==EFFECT_EDIT)page=EFFECT_DURATION;
    else if(action==1711&&page==EFFECT_DURATION){duration_next(&effect_draft.timed,&effect_draft.tick,&effect_draft.rounds,&effect_draft.minutes,&effect_draft.owner);}
    else if(action>=1712&&action<=1715&&page==EFFECT_DURATION&&effect_draft.timed)duration_adjust(effect_draft.tick,&effect_draft.rounds,&effect_draft.minutes,action-1712);
    else if(action==1716&&page==EFFECT_DURATION)page=EFFECT_EDIT;
    else if(action==1750&&page==EFFECT_DURATION)page=EFFECT_LINK;
    else if(action==1751&&page==EFFECT_LINK&&(effect_draft.concentration||state.magic.concentration))effect_draft.concentration=effect_draft.concentration?0:state.magic.concentration;
    else if(action==1752&&page==EFFECT_LINK)page=EFFECT_DURATION;
    else if(action==1740&&timed_effects()){round_return_page=page==DEFENCE_LIST?DEFENCE_LIST:EFFECT_LIST;round_defence_page=0;page=EFFECT_ROUND;}
    else if(action==1741&&page==EFFECT_ROUND)page=round_return_page;
    else if(action==1743&&page==EFFECT_ROUND){pd_round_summary r;if(preview_round(&next,&r)&&r.defences_advanced)page=ROUND_DEFENCES;}
    else if(action==1744&&page==ROUND_DEFENCES)page=EFFECT_ROUND;
    else if(action==1745&&page==ROUND_DEFENCES&&round_defence_page)round_defence_page--;
    else if(action==1746&&page==ROUND_DEFENCES){pd_round_summary r;if(preview_round(&next,&r)&&(round_defence_page+1)*3<bit_count(r.defences_advanced))round_defence_page++;}
    else if(action==1742&&page==EFFECT_ROUND){
        pd_round_summary r;if(!preview_round(&next,&r))return;
        if(!r.effects_advanced&&!r.defences_advanced){page=round_return_page;}
        else if(pd_storage_save(&next)){previous=state;undo_available=true;state=next;invalidate_strike();page=round_return_page;}
    }
    else if(action==1709&&page==EFFECT_CONFIRM){
        if(!preview_effect(&next))return;
        bool changed=memcmp(&next,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&next)){
            if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}
            page=EFFECT_LIST;
        }
    }
    else if(action==1601||action==1602||action==1603){
        if(action==1601&&next.brightness>10)next.brightness=next.brightness>20?next.brightness-10:10;
        if(action==1602&&next.brightness<100)next.brightness=next.brightness<90?next.brightness+10:100;
        if(action==1603)next.dim_timeout=(next.dim_timeout+1)%4;
        if(memcmp(&next,&state,sizeof state)&&pd_storage_save(&next)){
            state=next;previous.brightness=state.brightness;previous.dim_timeout=state.dim_timeout;
        }
    }
    else if(action==1500)page=STATUS_HOME;
    else if(action==1501)page=DEATH_VIEW;
    else if(action==1502)page=DEATH_EDIT;
    else if(action==1503)page=CONDITION_LIST;
    else if(action==1504)request_status(STATUS_POISON);
    else if(action==1510&&pd_death_ready(&state)){status_options=(pd_death_roll_options){0};request_status(STATUS_ROLL);}
    else if(action==3100&&page==DEATH_VIEW&&pd_death_ready(&state)){status_options=(pd_death_roll_options){0};page=DEATH_ROLL_SETUP;}
    else if(page==DEATH_ROLL_SETUP&&action>=3101&&action<=3106){
        if(action==3101)status_options.advantage=!status_options.advantage;
        else if(action==3102)status_options.disadvantage=!status_options.disadvantage;
        else if(action==3103&&status_options.bonus> -30)status_options.bonus--;
        else if(action==3104&&status_options.bonus<30)status_options.bonus++;
        else if(action==3105){status_options=(pd_death_roll_options){0};page=DEATH_VIEW;}
        else if(action==3106)request_status(STATUS_ROLL);
    }
    else if(action>=1511&&action<=1517){death_action=action-1511;request_status(STATUS_DEATH);}
    else if(action==1520){page=CONDITION_PICKER;preset_page=0;}
    else if(action==1521&&condition_page)condition_page--;
    else if(action==1522&&(condition_page+1)*3<state.status.count)condition_page++;
    else if(action==1523||action==1524){condition_delta=action==1523?-1:1;request_status(STATUS_LEVEL);}
    else if(action==1525)request_status(STATUS_REMOVE);
    else if(action==1526&&preset_page)preset_page--;
    else if(action==1527&&preset_page<4)preset_page++;
    else if(action>=1200&&action<1200+PD_CONDITIONS){condition_index=action-1200;if(condition_index<state.status.count)page=CONDITION_DETAIL;}
    else if(action>=1300&&action<1315){preset_index=action-1300;request_status(STATUS_ADD);}
    else if(action==1506&&page==STATUS_CONFIRM){
        status_rolled=false;page=status_return_page;
    }
    else if(action==1505&&page==STATUS_CONFIRM){
        bool rolling=status_command==STATUS_ROLL;
        if(rolling){if(!status_rolled){if(!pd_death_roll_configured(&state,status_options,pd_board_random,NULL,&hit_candidate,&status_result))return;status_rolled=true;}next=hit_candidate;}
        else if(!status_change(&next))return;
        bool changed=memcmp(&next,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&next)) {
            if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}
            status_rolled=false;
            if(rolling){char note[49];snprintf(note,sizeof note,"PF %u; riuscite %u; fallite %u",state.hp,state.status.successes,state.status.failures);journal_d20(PD_H_DEATH,"Salvezza morte",&status_result,pd_death_mode(status_options),note);
                status_outcome=state.hp?4:state.status.failures==3?3:state.status.stabilized?2:status_result.natural!=1&&status_result.total>=10?1:0;}
            if(rolling){result=status_result;combat_roll=5;pool_view=false;has_roll=false;mode=pd_death_mode(status_options);ability=-1;page=DICE;pd_dice_start(&dice_animation,&result,lv_tick_get());}
            else page=status_command==STATUS_POISON?STATUS_HOME:status_command==STATUS_DEATH?DEATH_VIEW:status_command==STATUS_LEVEL?CONDITION_DETAIL:CONDITION_LIST;
        }
    }
    else if(action==900){page=REST_HOME;rest_active=false;rest_plan=(pd_rest_plan){0};hit_index=0;}
    else if(action==901){rest_plan=(pd_rest_plan){0};page=REST_PREVIEW;}
    else if(action==902){page=rest_plan.long_rest?REST_LONG_SETTINGS:REST_HOME;}
    else if(action==903&&(state.hp||!state.status.available||state.status.failures<3)){rest_plan=(pd_rest_plan){.long_rest=true};page=REST_LONG_SETTINGS;hit_index=0;}
    else if(action==904&&page==REST_LONG_SETTINGS){rest_plan.policy=(rest_plan.policy+1)%3;memset(rest_plan.regain,0,sizeof rest_plan.regain);}
    else if(action==905&&page==REST_LONG_SETTINGS&&rest_plan.policy==PD_HD_2014&&state.recovery.count){page=REST_REGAIN;hit_index=0;}
    else if(action==906&&page==REST_LONG_SETTINGS)page=REST_PREVIEW;
    else if(action==907&&page==REST_PREVIEW){
        if(!pd_rest_apply(&next,&rest_plan))return;
        bool changed=memcmp(&next,&state,sizeof state)!=0;
        if(!changed||pd_storage_save(&next)) {
            if(changed){previous=state;undo_available=true;state=next;invalidate_strike();}
            rest_active=!rest_plan.long_rest;page=rest_active?REST_SESSION:REST_HOME;
        }
    }
    else if(action==908&&page==REST_REGAIN)page=REST_LONG_SETTINGS;
    else if(action==909&&rest_active&&state.recovery.count){page=HIT_DICE;hit_index=0;}
    else if(action==910){hit_rolled=false;page=HIT_DICE;}
    else if(action==911&&rest_active)page=REST_SESSION;
    else if((action==912||action==913)&&(page==REST_REGAIN||page==HIT_DICE)) {
        if(action==912&&hit_index)hit_index--;
        if(action==913&&hit_index+1<state.recovery.count)hit_index++;
    }
    else if(action==914&&page==HIT_CONFIRM&&rest_active){
        if(!hit_rolled){if(!pd_spend_hit_die(&state,hit_index,pd_board_random,NULL,&hit_candidate,&hit_result,&hit_gained))return;hit_rolled=true;}
        if(pd_storage_save(&hit_candidate)){
            previous=state;undo_available=true;state=hit_candidate;invalidate_strike();
            char note[49];snprintf(note,sizeof note,"Cura +%u; PF %u/%u",hit_gained,state.hp,pd_max_hp(&state));journal_pool(PD_H_HIT_DIE,state.recovery.pools[hit_index].name,&hit_result,note);
            healing_gained=hit_gained;combat_roll=4;pool_view=true;has_roll=false;page=DICE;hit_rolled=false;
            pd_dice_pool_start(&dice_animation,&hit_result,lv_tick_get());
        }
    }
    else if(action==915&&page==HIT_DICE&&rest_active&&state.hp>0&&state.hp<pd_max_hp(&state)&&hit_index<state.recovery.count&&state.recovery.pools[hit_index].used<state.recovery.pools[hit_index].maximum){page=HIT_CONFIRM;hit_rolled=false;}
    else if((action==916||action==917)&&page==REST_REGAIN&&hit_index<state.recovery.count){
        if(action==916&&rest_plan.regain[hit_index])rest_plan.regain[hit_index]--;
        if(action==917&&rest_plan.regain[hit_index]<state.recovery.pools[hit_index].used&&selected_hit_dice()<pd_hit_budget(&state,rest_plan.policy))rest_plan.regain[hit_index]++;
    }
    else if(action==918){rest_active=false;invalidate_strike();page=RESOURCES;}
    else if(action==80)page=INVENTORY;
    else if(action==81||action==85) {
        pd_weapon_info w;if(!pd_weapon_stats(&state,state.equipped,&w))return;
        if(action==81){roll_context=(pd_roll_context){0};ignore_effects=false;}
        page=COMBAT;combat_roll=1;pool_view=false;has_roll=false;mode=0;strike_ready=false;damage_used=false;
        pd_dice_idle(&dice_animation,pd_attack_mode(&state,mode)?2:1);
    }
    else if(action==82&&inventory_page)inventory_page--;
    else if(action==83&&(inventory_page+1)*3<state.item_count)inventory_page++;
    else if(action>=500&&action<500+PD_ITEMS) { item_index=action-500;if(item_index>=state.item_count)return;page=ITEM_DETAIL; }
    else if(action==84&&page==ITEM_DETAIL) { if(pd_equip(&next,state.equipped==item_index?PD_NO_WEAPON:item_index)) {apply(&next);strike_ready=false;} }
    else if(action==86&&target_armor>1)target_armor--;
    else if(action==87&&target_armor<30)target_armor++;
    else if(action==88&&page==ITEM_DETAIL&&item_index<state.item_count) {
        const pd_item *v=&state.items[item_index];
        if(!v->quantity||(v->kind!=PD_CONSUMABLE&&v->kind!=PD_HEALING)||(v->kind==PD_HEALING&&(state.hp==pd_max_hp(&state)||(!state.hp&&state.status.available&&state.status.failures==3))))return;
        page=ITEM_CONFIRM;
    }
    else if(action==89&&page==ITEM_CONFIRM) {
        pd_pool roll;unsigned gained;
        if(!pd_use_item(&state,item_index,pd_board_random,NULL,&next,&roll,&gained))return;
        if(pd_storage_save(&next)) {
            previous=state;undo_available=true;state=next;invalidate_strike();
            if(roll.count) {
                char note[49];snprintf(note,sizeof note,"Cura +%u; PF %u/%u",gained,state.hp,pd_max_hp(&state));journal_pool(PD_H_HEAL,state.items[item_index].name,&roll,note);
                healing_gained=gained;combat_roll=3;pool_view=true;has_roll=false;page=DICE;
                pd_dice_pool_start(&dice_animation,&roll,lv_tick_get());
            } else page=ITEM_DETAIL;
        }
    }
    else if(action==90&&combat_roll==1&&has_roll&&strike_ready&&strike.hits&&!damage_used) {
        if(!pd_weapon_damage(&strike,pd_board_random,NULL,&damage_result))return;
        journal_pool(PD_H_DAMAGE,state.items[state.equipped].name,&damage_result,"Danni tirati; PF bersaglio non modificati");
        damage_used=true;combat_roll=2;pool_view=true;has_roll=false;page=DICE;
        pd_dice_pool_start(&dice_animation,&damage_result,lv_tick_get());
    }
    else if(action==91)page=ITEM_DETAIL;
    else if(action==93)page=COMBAT_DETAIL;
    else if(action==92&&page==COMBAT&&state.equipped!=PD_NO_WEAPON&&state.items[state.equipped].thrown_range) {
        next.combat.thrown=!state.combat.thrown;apply(&next);strike_ready=false;
    }
    if(report_feedback)feedback_prepare();
    if(page==HOME_VITALS&&(action==1||action==2||action==3)&&!strstr(pd_storage_status(),"Errore"))page=HERO;
    render();
    #undef next
}
static void event(lv_event_t *e){
    intptr_t action=(intptr_t)lv_event_get_user_data(e);
    if(action>=2908&&action<2908+PD_INPUT_CHECK_TARGETS){
        lv_indev_t *input=lv_event_get_indev(e);
        if(!input||lv_indev_get_type(input)!=LV_INDEV_TYPE_POINTER)return;
        lv_point_t point;lv_area_t area;lv_indev_get_point(input,&point);
        lv_obj_get_coords(lv_event_get_current_target(e),&area);
        if(point.x<area.x1||point.x>area.x2||point.y<area.y1||point.y>area.y2)return;
    }
    act(action);
}
static lv_obj_t *button(const char *text,int x,int y,int w,int h,intptr_t action,bool enabled) {
    lv_obj_t *b=lv_button_create(lv_screen_active());lv_obj_remove_style_all(b);lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
    bool primary=action==13||action==46||action==30||!strcmp(text,"Conferma")||!strcmp(text,"Applica")||!strcmp(text,"Lancia")||!strcmp(text,"Usa");
    pd_theme_button(b,primary);
    lv_obj_t *l=lv_label_create(b);lv_label_set_text(l,text);lv_obj_center(l);
    if (!enabled) lv_obj_add_state(b,LV_STATE_DISABLED);
    lv_obj_add_event_cb(b,event,LV_EVENT_CLICKED,(void*)action);return b;
}
static void render_dice_setup(void) {
    char text[80];
    if(die_picker) {
        label("Scegli il dado",83,105,300,true);
        const unsigned sides[]={4,6,8,10,12,20};
        for(int i=0;i<6;i++) {
            snprintf(text,sizeof text,"d%u",sides[i]);
            lv_obj_t *b=button(text,83+(i%3)*104,158+(i/3)*72,92,56,400+sides[i],true);
            if(pool_sides==sides[i]) {
                lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
                lv_obj_set_style_text_color(b,lv_color_hex(PD_GOLD),0);
            }
        }
        button("Indietro",83,308,300,48,41,true);return;
    }
    label("Prepara il tiro",83,105,300,true);
    snprintf(text,sizeof text,"Tipo: d%u",pool_sides);button(text,83,148,144,48,42,true);
    button(pool_damage?"Danni":"Somma",239,148,144,48,43,true);
    button("-",83,210,48,48,44,pool_count>1);
    snprintf(text,sizeof text,"Quantita %u",pool_count);label(text,139,223,188,false);
    button("+",335,210,48,48,45,pool_count<6);
    button("-",83,272,48,48,47,pool_bonus>PD_POOL_MIN_BONUS);
    snprintf(text,sizeof text,"Bonus %+d",pool_bonus);label(text,139,285,188,false);
    button("+",335,272,48,48,48,pool_bonus<30);
    button("Esci",83,332,76,48,41,true);button("Lancia",167,332,132,48,46,true);button("Prova",307,332,76,48,14,true);
}
static void check_name(char *out,size_t size,bool bonus) {
    pd_check_info info={0};bool known=ability>=0&&pd_describe_check(&state,check_kind,ability,&info);
    const char *name=ability<0?"d20 libero":check_kind==PD_SKILL?pd_skill_names[ability]:pd_abilities[ability];
    if(bonus&&known&&!info.blocked)snprintf(out,size,"%s%s %+d >",check_kind==PD_SAVE?"Salv. ":check_kind==PD_CHECK?"d20 ":"",name,info.total);
    else snprintf(out,size,"%s%s%s",check_kind==PD_SAVE?"Salvezza ":"",name,known&&info.blocked?" ?":"");
}
static void render_check_detail(void) {
    char text[160];pd_check_info info={0};bool known=pd_describe_check(&state,check_kind,ability,&info);
    check_name(text,sizeof text,false);lv_obj_t *title=label(text,83,111,300,true);
    if(check_kind==PD_SKILL)label_font(title,&lv_font_montserrat_20);
    if(!known||info.blocked)snprintf(text,sizeof text,"Da verificare");else snprintf(text,sizeof text,"Bonus %+d",info.total);
    lv_obj_t *total=label(text,83,150,300,true);lv_obj_set_style_text_color(total,lv_color_hex(info.blocked?PD_GOLD:PD_MINT),0);
    snprintf(text,sizeof text,"%s %+d",pd_abilities[info.ability],info.base);label(text,83,194,300,false);
    snprintf(text,sizeof text,"%s %+d",info.half?"Mezza comp.":info.rank==2?"Maestria":info.rank==1?"Competenza":"Non competente",info.trained);
    label(text,83,224,300,false);snprintf(text,sizeof text,"Extra %+d",info.extra);label(text,83,254,300,false);
    pd_check_resolution gate={0};pd_check_resolve(&state,check_kind,ability,mode,ignore_effects,roll_context,&gate);int effective=gate.mode;
    bool poison=state.training.poisoned&&check_kind!=PD_SAVE;
    bool strength=state.training.strength_advantage&&info.ability==0;
    const char *reason=ignore_effects?"Effetti PC esclusi":poison&&strength?"PC: avvelenato + Ira":poison?"PC: avvelenato":strength?"PC: Ira FOR":"Manuale";
    snprintf(text,sizeof text,"%s | %s",effective>0?"Vantaggio":effective<0?"Svantaggio":"Normale",reason);
    if(state.roll_conditions)snprintf(text,sizeof text,"%s",gate.needed?"Dichiara le circostanze":gate.automatic_failure?"Vista richiesta: fallisce":ignore_effects?"Effetti tiro esclusi":"Condizioni e modificatori del tiro");
    else if(!state.training.available)snprintf(text,sizeof text,"Vecchia copia: solo caratteristica");
    small_label(text,83,282,300,PD_MUTED);
    button(mode<0?"Svant":mode>0?"Vant":"Base",83,307,144,44,15,true);
    lv_obj_t *effects=button(state.roll_conditions?(ignore_effects?"Effetti: no":"Effetti: si"):(ignore_effects?"Effetti PC: no":"Effetti PC: si"),239,307,144,44,73,state.training.available||state.roll_conditions);
    lv_obj_set_style_text_font(effects,&lv_font_montserrat_16,0);
    if(state.roll_conditions){const char *names[]={"Indietro","Contesto",info.blocked||!known?"Libero":gate.automatic_failure?"Fallisce":"Lancia"};const unsigned actions[]={71,2230,info.blocked||!known?14:72};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i!=2||info.blocked||!known||gate.allowed);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}}
    else{button("Indietro",83,359,144,48,71,true);button(info.blocked||!known?"Libero":"Lancia",239,359,144,48,info.blocked||!known?14:72,true);}
}
static void render_sheet(void) {
    char text[120];
    if(sheet_tab==1) {
        snprintf(text,sizeof text,"Abilita %d/6",skill_page+1);label(text,139,111,188,false);
        button("<",83,99,48,44,63,skill_page>0);button(">",335,99,48,44,64,skill_page<5);
    } else label(sheet_tab==2?"Tiri salvezza":"Caratteristiche",83,111,300,true);
    const char *tabs[]={"Caratt.","Abilita","Salvezze"};
    for(int i=0;i<3;i++) {
        lv_obj_t *b=button(tabs[i],83+i*104,149,92,44,60+i,true);
        lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        if(sheet_tab==i)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
    }
    if(sheet_tab&&!state.training.available) {
        small_label("Competenze non disponibili",83,221,300,PD_GOLD);
        small_label("Importa una copia aggiornata",83,251,300,PD_TEXT);
        small_label("La scheda attuale e conservata",83,281,300,PD_MUTED);return;
    }
    if(sheet_tab==1)for(int i=0;i<3;i++) {
        unsigned index=skill_page*3+i;pd_check_info info;
        pd_describe_check(&state,PD_SKILL,index,&info);
        if(info.blocked)snprintf(text,sizeof text,"%s  ?",pd_skill_names[index]);
        else snprintf(text,sizeof text,"%s  %+d",pd_skill_names[index],info.total);
        button(text,83,199+i*48,300,44,220+index,true);
    } else for(int i=0;i<6;i++) {
        pd_check_info info;pd_describe_check(&state,sheet_tab==2?PD_SAVE:PD_CHECK,i,&info);
        if(sheet_tab==2&&info.blocked)snprintf(text,sizeof text,"%s\n?",pd_abilities[i]);
        else if(sheet_tab==2)snprintf(text,sizeof text,"%s\n%+d",pd_abilities[i],info.total);
        else if(info.blocked)snprintf(text,sizeof text,"%s %u\n?",pd_abilities[i],state.abilities[i]);
        else snprintf(text,sizeof text,"%s %u\n%+d",pd_abilities[i],state.abilities[i],info.total);
        lv_obj_t *b=button(text,83+(i%3)*104,199+(i/3)*73,92,64,(sheet_tab==2?240:200)+i,true);
        lv_obj_set_style_text_color(b,lv_color_hex(info.blocked?PD_GOLD:PD_MINT),0);
    }
}
static void render_inventory(void) {
    char text[160];
    snprintf(text,sizeof text,"Zaino %u/%u",inventory_page+1,state.item_count?(state.item_count+2)/3:1);
    label(text,139,111,188,false);button("<",83,99,48,44,82,inventory_page>0);
    button(">",335,99,48,44,83,(inventory_page+1)*3<state.item_count);
    if(!state.inventory_available) {
        small_label("Inventario non disponibile",83,190,300,PD_GOLD);
        small_label("Importa una copia aggiornata",83,220,300,PD_TEXT);
    } else if(!state.item_count)label("Zaino vuoto",83,210,300,false);
    else for(unsigned row=0;row<3;row++) {
        unsigned i=inventory_page*3+row;if(i>=state.item_count)break;
        const pd_item *v=&state.items[i];
        /* Reserve space for quantity and equipment marker even for 40-char names. */
        size_t length=strlen(v->name),shown=length;lv_point_t extent;
        do {
            snprintf(text,sizeof text,"%.*s%s%s  x%u",(int)shown,v->name,shown<length?"...":"",i==state.equipped?" *":"",v->quantity);
            lv_text_get_size(&extent,text,&lv_font_montserrat_20,0,0,1000,LV_TEXT_FLAG_NONE);
            if(extent.x<=234||!shown)break;
            shown--;
        } while(true);
        lv_obj_t *b=button(text,83,155+row*49,300,44,500+i,true);
        lv_obj_set_style_pad_all(b,0,0);
        const lv_image_dsc_t *icon=v->kind==PD_WEAPON?&theme_sword:v->kind==PD_HEALING?&theme_potion:v->kind==PD_CONSUMABLE?&theme_bundle:&theme_pack;
        art(icon,5,5,b);
        lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,236);lv_obj_set_height(l,lv_font_get_line_height(&lv_font_montserrat_20));
        lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_align(l,LV_ALIGN_LEFT_MID,45,0);
    }
    small_label("* arma scelta | note sul PC",83,312,300,PD_MUTED);
    const char *actions[]={"Monete","Usi","Attacco"};const unsigned ids[]={2400,103,81};
    for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,359,92,48,ids[i],i!=2||state.equipped!=PD_NO_WEAPON);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}
static void render_item(void) {
    char text[160];const pd_item *v=&state.items[item_index];
    lv_obj_t *title=small_label(v->name,83,99,300,PD_TEXT);lv_label_set_long_mode(title,LV_LABEL_LONG_WRAP);lv_obj_set_height(title,58);
    snprintf(text,sizeof text,"Quantita %u",v->quantity);label(text,83,164,300,false);
    if(page==ITEM_CONFIRM) {
        label("Usare una unita?",83,201,300,false);
        if(v->kind==PD_HEALING)snprintf(text,sizeof text,"Cura %ud%u %+d PF",v->count,v->sides,v->healing_modifier);
        else snprintf(text,sizeof text,"Altri effetti: gestisci con il DM");
        small_label(text,83,237,300,PD_GOLD);
        small_label("PF e quantita salvati insieme",83,273,300,PD_MUTED);
        button("Indietro",83,359,144,48,91,true);button("Conferma",239,359,144,48,89,true);return;
    }
    if(v->kind==PD_WEAPON) {
        pd_weapon_info w={0};bool available=pd_weapon_stats(&state,item_index,&w);
        if(available) {
        snprintf(text,sizeof text,"%s: base %+d / comp. %+d",pd_abilities[v->ability],w.base,w.trained);small_label(text,83,196,300,PD_TEXT);
        snprintf(text,sizeof text,"Attacco %+d (extra %+d)",w.attack_bonus,w.attack_extra);label(text,83,224,300,false);
        snprintf(text,sizeof text,"Danni %ud%u %+d %s",v->count,v->sides,w.damage_bonus,v->damage_type);small_label(text,83,255,300,PD_GOLD);
        } else {label("Arma esaurita",83,224,300,false);small_label("Quantita zero: tiri non disponibili",83,255,300,PD_MUTED);}
        button(state.equipped==item_index?"Riponi":"Scegli",83,299,144,44,84,v->quantity);
        button("Attacco",239,299,144,44,81,v->quantity&&state.equipped==item_index);
    } else {
        label(v->kind==PD_HEALING?"Cura":v->kind==PD_CONSUMABLE?"Consumabile":"Oggetto",83,199,300,false);
        small_label("Note ed effetti speciali sul PC",83,237,300,PD_MUTED);
        if(v->kind==PD_HEALING) { snprintf(text,sizeof text,"%ud%u %+d PF | limite PF massimi",v->count,v->sides,v->healing_modifier);small_label(text,83,268,300,PD_GOLD); }
        if(v->kind==PD_CONSUMABLE||v->kind==PD_HEALING)button(v->kind==PD_HEALING&&!state.hp&&state.status.available&&state.status.failures==3?"Tre fallimenti: DM":v->kind==PD_HEALING&&state.hp==pd_max_hp(&state)?"PF al massimo":"Usa",83,299,300,44,88,v->quantity&&(v->kind!=PD_HEALING||(state.hp<pd_max_hp(&state)&&(state.hp||!state.status.available||state.status.failures<3))));
    }
    button("Zaino",83,359,v->charges.maximum?144:300,48,80,true);
    if(v->charges.maximum)button("Cariche",239,359,144,48,2160,true);
}
static void render_history(void){
    char text[160];
    if(page==HISTORY_DISCARD){
        label("Scarta dal registro?",83,111,300,true);
        small_label("Il tiro non sara registrato.",83,180,300,PD_GOLD);
        small_label("PF e usi salvati restano invariati.",83,214,300,PD_TEXT);
        small_label("Non e un annullamento di gioco.",83,248,300,PD_MUTED);
        button("Indietro",83,359,144,48,2607,true);button("Scarta",239,359,144,48,2608,true);return;
    }
    if(page==HISTORY_HOME){
        unsigned total=history.count+(history_pending?1:0),pages=(total+2)/3;if(!pages)pages=1;
        if(history_page>=pages)history_page=pages-1;
        snprintf(text,sizeof text,"Storico %u/%u",history_page+1,pages);label(text,139,111,188,false);
        button("<",83,99,48,44,2601,history_page>0);button(">",335,99,48,44,2602,history_page+1<pages);
        if(!total){label("Nessun tiro registrato",83,183,300,false);small_label(pd_history_store_status(),83,232,300,PD_MUTED);}
        for(unsigned row=0;row<3;row++){
            unsigned index=history_page*3+row;if(index>=total)break;
            bool unsaved=history_pending&&!index;unsigned saved=index-(history_pending?1:0);
            const pd_history_entry *e=unsaved?&history_unsaved:&history.entries[saved];
            if(unsaved)snprintf(text,sizeof text,"Da registrare: %s",e->label);
            else snprintf(text,sizeof text,"#%u %s = %d",(unsigned)e->sequence,e->label,e->total);
            lv_obj_t *b=button(text,83,155+row*49,300,44,unsaved?2609:2700+saved,true);
            lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,288);lv_obj_set_height(l,lv_font_get_line_height(&lv_font_montserrat_16));lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);
        }
        snprintf(text,sizeof text,"%u salvati%s",history.count,history_pending?" | 1 non registrato":" | ultimi 20 tiri");small_label(text,83,308,300,history_pending?PD_GOLD:PD_MUTED);
        if(history_pending){
            small_label("Da salvare prima di spegnere",83,332,300,PD_GOLD);
            const char *names[]={"Riprova","Scarta","Indietro"};const unsigned actions[]={2603,2606,2604};
            for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        }else button("Indietro",83,359,300,48,2604,true);
        return;
    }
    bool unsaved=history_index==PD_HISTORY_LIMIT;
    if((unsaved&&!history_pending)||(!unsaved&&history_index>=history.count)){page=HISTORY_HOME;render_history();return;}
    const pd_history_entry *e=unsaved?&history_unsaved:&history.entries[history_index];
    unsigned pages=(e->count+5)/6;if(history_dice_page>=pages)history_dice_page=pages-1;
    snprintf(text,sizeof text,unsaved?"Non registrato":"Tiro #%u",(unsigned)e->sequence);label(text,139,111,188,false);
    button("<",83,99,48,44,2610,history_dice_page>0);button(">",335,99,48,44,2611,history_dice_page+1<pages);
    lv_obj_t *name=small_label(e->label,83,148,300,PD_TEXT);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,57);
    snprintf(text,sizeof text,"%ud%u %+d = %d",e->count,e->sides,e->bonus,e->total);small_label(text,83,208,300,PD_GOLD);
    const char *kinds[]={"Libero","Prova","Abilita","Salvezza","Attacco","Danni","Cura","Dado vita","Morte","Concentrazione","Ricarica"};
    snprintf(text,sizeof text,"%s | %s",kinds[e->kind],e->mode>0?"vantaggio":e->mode<0?"svantaggio":e->sides==20&&e->count==1?"base":"somma");small_label(text,83,231,300,PD_MUTED);
    for(unsigned row=0;row<6;row++){
        unsigned i=history_dice_page*6+row;if(i>=e->count)break;
        snprintf(text,sizeof text,"D%u: %u",i+1,e->dice[i]);small_label(text,83+(row%3)*104,258+(row/3)*23,92,PD_TEXT);
    }
    lv_obj_t *note=small_label(e->note,83,300,300,PD_MUTED);lv_label_set_long_mode(note,LV_LABEL_LONG_WRAP);lv_obj_set_height(note,57);
    button("Elenco",83,359,144,48,2605,true);button(unsaved?"Riprova":"Dadi",239,359,144,48,unsaved?2603:14,true);
}
static void render_wallet(void){
    char text[160];
    if(page==WALLET_HOME){
        snprintf(text,sizeof text,"Borsello %u/2",wallet_page+1);label(text,139,111,188,false);
        button("<",83,99,48,44,2401,state.wallet.available&&wallet_page>0);button(">",335,99,48,44,2402,state.wallet.available&&wallet_page<1);
        if(!state.wallet.available){
            label("Saldo non disponibile",83,173,300,false);
            small_label("Monete non ancora verificate",83,215,300,PD_GOLD);
            button("Configura saldo",83,261,300,48,2403,true);
            small_label("Nessun saldo inventato",83,319,300,PD_MUTED);
        }else{
            for(unsigned row=0;row<3;row++){
                unsigned i=wallet_page*3+row;if(i>=PD_COINS)break;
                snprintf(text,sizeof text,"%s: %u %s",coin_names[i],(unsigned)state.wallet.coins[i],coin_units[i]);
                button(text,83,155+row*49,300,44,2500+i,true);
            }
            snprintf(text,sizeof text,"Equiv. rame: %llu MR",(unsigned long long)pd_wallet_copper(&state.wallet));small_label(text,83,302,300,PD_GOLD);
            small_label("Nessun cambio automatico",83,325,300,PD_MUTED);
        }
        lv_obj_t *undo=button("Annulla monete",83,359,144,48,2414,wallet_can_undo());lv_obj_set_style_text_font(undo,&lv_font_montserrat_16,0);
        button("Zaino",239,359,144,48,80,true);return;
    }
    if(page==WALLET_EDIT){
        snprintf(text,sizeof text,"%s (%s)",coin_names[wallet_coin],coin_units[wallet_coin]);label(text,139,111,188,false);
        button("<",83,99,48,44,2404,wallet_coin>0);button(">",335,99,48,44,2405,wallet_coin+1<PD_COINS);
        snprintf(text,sizeof text,"%u",(unsigned)wallet_draft.coins[wallet_coin]);label(text,83,161,300,true);
        small_label("Nuovo saldo da confermare",83,196,300,PD_GOLD);
        snprintf(text,sizeof text,"Passo: %u >",(unsigned)wallet_steps[wallet_step]);button(text,83,223,300,44,2408,true);
        button("-",83,279,144,44,2406,wallet_draft.coins[wallet_coin]>=wallet_steps[wallet_step]);
        button("+",239,279,144,44,2407,wallet_draft.coins[wallet_coin]<=PD_COIN_MAX-wallet_steps[wallet_step]);
        small_label("Altri tagli: frecce in alto",83,326,300,PD_MUTED);
        button("Annulla",83,359,144,48,2409,true);button("Anteprima",239,359,144,48,2410,true);return;
    }
    label("Conferma borsello",83,111,300,true);
    for(unsigned i=0;i<PD_COINS;i++){
        if(wallet_origin.available)snprintf(text,sizeof text,"%s %u -> %u",coin_units[i],(unsigned)wallet_origin.coins[i],(unsigned)wallet_draft.coins[i]);
        else snprintf(text,sizeof text,"%s ? -> %u",coin_units[i],(unsigned)wallet_draft.coins[i]);
        small_label(text,83,153+i*32,300,wallet_origin.coins[i]!=wallet_draft.coins[i]||!wallet_origin.available?PD_GOLD:PD_TEXT);
    }
    small_label(wallet_origin.available?"Salva tutti i tagli insieme":"Crea un saldo locale verificato",83,327,300,PD_MUTED);
    button("Indietro",83,359,144,48,2412,true);button("Conferma",239,359,144,48,2411,!wallet_same(&wallet_draft,&state.wallet)&&wallet_same(&wallet_origin,&state.wallet));
}
static void render_combat(void) {
    pd_weapon_info w;char text[160];if(!pd_weapon_stats(&state,state.equipped,&w)) {page=INVENTORY;render_inventory();return;}
    combat_roll=1;pool_view=false;
    const pd_item *v=&state.items[state.equipped];label(v->name,83,111,300,false);
    snprintf(text,sizeof text,"Attacco %+d >",w.attack_bonus);button(text,83,144,state.roll_conditions?204:300,44,93,true);if(state.roll_conditions){lv_obj_t *c=button("Contesto",299,144,84,44,2230,true);lv_obj_set_style_text_font(c,&lv_font_montserrat_16,0);}
    snprintf(text,sizeof text,"%s %+d / comp. %+d / extra %+d",pd_abilities[v->ability],w.base,w.trained,w.attack_extra);small_label(text,83,193,300,PD_TEXT);
    snprintf(text,sizeof text,"Danni %ud%u %+d | gittata %u q.",w.count,w.sides,w.damage_bonus,w.reach);small_label(text,83,215,300,PD_GOLD);
    small_label(state.roll_conditions?"Condizioni sui tiri: apri Contesto":"Effetti PC fotografati; speciali: DM",83,239,300,PD_MUTED);
    button("-",83,263,48,44,86,target_armor>1);snprintf(text,sizeof text,"CA bersaglio %u",target_armor);label(text,139,274,188,false);button("+",335,263,48,44,87,target_armor<30);
    pd_attack_resolution gate={0};pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);int effective=gate.mode;
    button(effective>0?"Vantaggio":effective<0?"Svantaggio":"Normale",83,315,144,44,15,true);button("Lancia",239,315,144,44,72,gate.allowed);
    button("Zaino",83,363,144,44,80,true);
    button(state.combat.thrown?"Lancio: si":"Lancio: no",239,363,144,44,92,v->thrown_range>0);
}
static unsigned visible_powers(void) {
    unsigned n=0;for(unsigned i=0;i<state.magic.count;i++)if(magic_filter==2||state.magic.powers[i].kind==magic_filter)n++;return n;
}
static int visible_power(unsigned row) {
    unsigned n=0;for(unsigned i=0;i<state.magic.count;i++)if(magic_filter==2||state.magic.powers[i].kind==magic_filter){if(n++==row)return i;}return -1;
}
static void choose_cost(void) {
    const pd_power *p=&state.magic.powers[power_index];
    bool advance=(cast_level&0x80)!=0;cast_level&=0x7f;
    if(p->kind||!p->level||!(p->flags&PD_USES_SLOT)){cast_level=0;cast_pact=false;return;}
    unsigned costs[18],count=0;
    for(unsigned k=0;k<2;k++)for(unsigned i=p->level;i<=9;i++)if((k?state.magic.pact:state.magic.slots)[i-1].current)costs[count++]=i+(k?16:0);
    if(!count){cast_level=0;return;}
    unsigned selected=cast_level+(cast_pact?16:0),at=0;bool found=false;
    for(unsigned i=0;i<count;i++)if(costs[i]==selected){at=i;found=true;break;}
    if(advance&&found)at=(at+1)%count;
    selected=costs[at];cast_pact=selected>16;cast_level=selected&15;
}
static bool can_use_power(void) {scratch=state;return pd_magic_use(&scratch,power_index,cast_level,cast_pact);}
static void cost_text(char *text,size_t cap) {
    const pd_power *p=&state.magic.powers[power_index];
    if(p->kind&&(p->flags&PD_PASSIVE))snprintf(text,cap,"Informativa: nessun consumo");
    else if(p->kind||(p->flags&PD_LIMITED))snprintf(text,cap,"Utilizzi %u/%u%s",p->remaining,p->maximum,p->kind?"":" + costo magia");
    else if(p->level&&(p->flags&PD_USES_SLOT))snprintf(text,cap,cast_level?"Slot %u %s >":"Nessuno slot disponibile",cast_level,cast_pact?"patto":"ordinario");
    else snprintf(text,cap,"Nessuno slot richiesto");
}
static void render_magic_list(void) {
    char text[160];unsigned count=visible_powers(),pages=count?(count+2)/3:1;
    if(magic_page>=pages)magic_page=pages-1;
    snprintf(text,sizeof text,"Magie e usi %u/%u",magic_page+1,pages);small_label(text,139,113,188,PD_TEXT);
    button("<",83,99,48,44,805,magic_page>0);button(">",335,99,48,44,806,magic_page+1<pages);
    const char *tabs[]={"Magie","Capacita","Tutte"};for(unsigned i=0;i<3;i++) {
        lv_obj_t *b=button(tabs[i],83+i*104,149,92,44,802+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        if(magic_filter==i)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
    }
    if(!state.magic.available){small_label("Magie non disponibili",83,225,300,PD_GOLD);small_label("Importa una copia aggiornata",83,255,300,PD_TEXT);}
    else if(!count)label("Nessuna voce in questa vista",83,241,300,false);
    else for(unsigned row=0;row<3;row++) {
        int i=visible_power(magic_page*3+row);if(i<0)break;
        const pd_power *p=&state.magic.powers[i];
        lv_obj_t *b=button(p->name,83,199+row*48,300,44,1000+i,true);
        lv_obj_set_style_pad_all(b,0,0);art(p->kind?&theme_crystal:&theme_magic,5,5,b);
        lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,236);lv_obj_set_height(l,lv_font_get_line_height(&lv_font_montserrat_20));lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_align(l,LV_ALIGN_LEFT_MID,45,0);
    }
    button("Usi",83,359,144,48,103,true);button("Risorse",239,359,144,48,836,state.magic.available);
}
static void render_magic_detail(void) {
    char text[200];const pd_power *p=&state.magic.powers[power_index];choose_cost();
    lv_obj_t *name=small_label(p->name,83,99,300,PD_TEXT);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,58);
    snprintf(text,sizeof text,p->kind?(p->flags&PD_PASSIVE?"Capacita informativa":"Capacita con utilizzi"):"Magia livello %u%s",p->level,p->flags&PD_CONCENTRATES?" / conc.":"");label(text,83,164,300,false);
    int attack,dc;
    if(pd_magic_stats(&state,power_index,&attack,&dc))snprintf(text,sizeof text,"%s | attacco %+d | CD %d%s",pd_abilities[p->ability],attack,dc,p->flags&PD_PREPARED?"":" | non preparata");
    else snprintf(text,sizeof text,"%s | recupero %s",p->effect==1?"Effetto: Ira":p->effect==2?"Effetto a zero PF":"Effetto da gestire con il DM",(const char *const[]){"manuale","breve","lungo"}[p->recovery]);
    small_label(text,83,198,300,PD_GOLD);
    cost_text(text,sizeof text);
    if(!p->kind&&p->level&&(p->flags&PD_USES_SLOT)) {
        snprintf(text,sizeof text,cast_level?"Slot %u %s >":"Nessuno slot disponibile",cast_level,cast_pact?"patto":"ordinario");
        button(text,83,224,300,44,834,cast_level>0);
    } else label(text,83,235,300,false);
    if(p->flags&PD_LIMITED){snprintf(text,sizeof text,"Usi innati %u/%u",p->remaining,p->maximum);small_label(text,83,274,300,PD_MUTED);}
    else small_label(p->flags&PD_EXCERPT?"Testo in estratto; completo sul PC":"Effetti e risultati: verifica con il DM",83,274,300,PD_MUTED);
    button("Testo",83,301,144,44,830,true);
    const char *use_label=p->kind?((p->flags&PD_PASSIVE)?"Informativa":!p->remaining?"Esaurita":p->effect==1&&state.combat.raging?"Gia attiva":p->effect==2&&state.hp?"A zero PF":p->effect==2&&state.status.available&&state.status.failures==3?"Verifica DM":"Usa"):
        state.combat.raging?"Ira attiva":!(p->flags&PD_PREPARED)?"Non preparata":(p->flags&PD_LIMITED)&&!p->remaining?"Esaurita":"Lancia";
    lv_obj_t *use=button(use_label,239,301,144,44,831,can_use_power());lv_obj_set_style_text_font(use,&lv_font_montserrat_16,0);
    button("Elenco",83,359,144,48,835,true);
    button("Utilizzi",239,359,144,48,839,!(p->flags&PD_PASSIVE)&&(p->kind||(p->flags&PD_LIMITED)));
}
static void render_magic_text(void) {
    const pd_power *p=&state.magic.powers[power_index];char chunk[PD_POWER_TEXT_BYTES];
    unsigned starts[16]={0},lengths[16]={0},pages=0;size_t at=0,total=strlen(p->description);
    do {
        size_t take=0,last_space=0;lv_point_t size={0};
        while(at+take<total) {
            chunk[take]=p->description[at+take];chunk[++take]=0;
            lv_text_get_size(&size,chunk,&lv_font_montserrat_16,0,0,300,LV_TEXT_FLAG_NONE);
            if(size.y>138){take--;break;}
            if(chunk[take-1]==' ')last_space=take;
        }
        if(at+take<total&&last_space>take/2)take=last_space;
        if(!take&&at<total)take=1;
        starts[pages]=at;lengths[pages++]=take;at+=take;
        while(at<total&&p->description[at]==' ')at++;
    } while(at<total&&pages<16);
    if(text_page>=pages)text_page=pages-1;
    char heading[80];snprintf(heading,sizeof heading,"Testo %u/%u",text_page+1,pages);label(heading,139,111,188,false);
    button("<",83,99,48,44,846,text_page>0);button(">",335,99,48,44,847,text_page+1<pages);
    small_label(p->name,83,151,300,PD_GOLD);
    memcpy(chunk,p->description+starts[text_page],lengths[text_page]);chunk[lengths[text_page]]=0;
    lv_obj_t *body=small_label(chunk[0]?chunk:"Nessuna descrizione disponibile",83,183,300,PD_TEXT);
    lv_label_set_long_mode(body,LV_LABEL_LONG_WRAP);lv_obj_set_height(body,138);lv_obj_set_style_text_align(body,LV_TEXT_ALIGN_LEFT,0);
    small_label(p->flags&PD_EXCERPT?"Estratto: testo completo sul PC":"Testo personale; effetti al tavolo",83,325,300,PD_MUTED);
    button("Indietro",83,359,300,48,833,true);
}
static void render_magic_other(void) {
    char text[180];
    if(page==MAGIC_CONFIRM) {
        const pd_power *p=&state.magic.powers[power_index];choose_cost();
        label("Conferma utilizzo",83,111,300,true);label(p->name,83,159,300,false);cost_text(text,sizeof text);small_label(text,83,204,300,PD_GOLD);
        if(!p->kind&&p->level&&(p->flags&PD_USES_SLOT)){snprintf(text,sizeof text,"Slot %u %s",cast_level,cast_pact?"patto":"ordinario");small_label(text,83,231,300,PD_TEXT);}
        small_label(p->flags&PD_CONCENTRATES&&state.magic.concentration?"Sostituisce la concentrazione attuale":p->effect==1?"Attiva Ira e termina concentrazione":"Effetti da risolvere con il DM",83,275,300,PD_MUTED);
        if(((p->flags&PD_CONCENTRATES)||p->effect==1)&&state.magic.concentration){
            scratch=state;pd_concentration_end(&scratch);
            snprintf(text,sizeof text,"Termina %u effetti | PF %u -> %u",pd_concentration_effects(&state),state.hp,scratch.hp);small_label(text,83,306,300,PD_GOLD);
        }
        button("Indietro",83,359,144,48,833,true);button("Conferma",239,359,144,48,832,can_use_power());
    } else if(page==MAGIC_STATUS) {
        label("Risorse magiche",83,111,300,true);button("Slot ordinari / patto",83,153,300,44,851,true);
        button(state.magic.concentration?"Concentrazione: termina":"Concentrazione: no",83,207,300,44,837,state.magic.concentration>0);
        button(state.combat.raging?"Ira: termina":"Ira: no",83,261,300,44,838,state.combat.raging);
        if(state.magic.concentration)small_label(state.magic.powers[state.magic.concentration-1].name,83,320,300,PD_GOLD);
        button("Usi",83,359,144,48,103,true);button("Elenco",239,359,144,48,835,true);
    } else if(page==MAGIC_STOP) {
        label(stop_rage?"Terminare Ira?":"Termina concentrazione?",83,153,300,false);
        scratch=state;if(!stop_rage)pd_concentration_end(&scratch);
        snprintf(text,sizeof text,"Effetti associati che terminano: %u",stop_rage?0:pd_concentration_effects(&state));small_label(text,83,201,300,PD_GOLD);
        snprintf(text,sizeof text,"CA %u -> %u | PF max %u -> %u",pd_armor(&state),pd_armor(&scratch),pd_max_hp(&state),pd_max_hp(&scratch));small_label(text,83,233,300,PD_TEXT);
        snprintf(text,sizeof text,"PF correnti %u -> %u",state.hp,scratch.hp);label(text,83,267,300,false);
        small_label(state.hp>scratch.hp?"PF oltre il nuovo massimo: persi":"Annullabile da Eroe",83,309,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);
        button("Indietro",83,359,144,48,853,true);button("Conferma",239,359,144,48,852,true);
    } else if(page==MAGIC_SLOTS) {
        snprintf(text,sizeof text,"Slot livello %u",slot_index+1);label(text,139,111,188,false);
        button("<",83,99,48,44,842,slot_index>0);button(">",335,99,48,44,843,slot_index<8);
        button("Ordinari",83,149,144,44,844,true);button("Patto",239,149,144,44,845,true);
        const pd_slot *slot=&(slot_pact?state.magic.pact:state.magic.slots)[slot_index];
        snprintf(text,sizeof text,"%u / %u",slot->current,slot->maximum);label(text,83,210,300,true);
        button("-1",83,257,144,44,840,slot->current>0);button("+1",239,257,144,44,841,slot->current<slot->maximum);
        small_label("Correzione manuale; riposi in Usi",83,319,300,PD_MUTED);button("Risorse",83,359,300,48,836,true);
    } else if(page==MAGIC_RECOVER) {
        const pd_power *p=&state.magic.powers[power_index];label(p->name,83,111,300,false);
        label("Utilizzi manuali",83,159,300,true);snprintf(text,sizeof text,"%u / %u",p->remaining,p->maximum);label(text,83,205,300,true);
        button("-1",83,257,144,44,848,p->remaining>0);button("+1",239,257,144,44,849,p->remaining<p->maximum);
        small_label("Recupero: verifica con il DM",83,319,300,PD_MUTED);button("Indietro",83,359,300,48,833,true);
    } else if(page==KEY_SETTINGS) {
        label("Tasti configurabili",83,111,300,true);
        const char *gestures[]={"PWR breve","BOOT breve","BOOT lungo"};
        const lv_image_dsc_t *icons[]={&theme_gear,&theme_hero,&theme_sheet,&theme_dice,&theme_pack,&theme_magic,&theme_dice,&theme_rest,&theme_heart,&theme_sun,&theme_notes,&theme_time,&theme_shield,&theme_notes};
        for(unsigned i=0;i<3;i++) {
            snprintf(text,sizeof text,"%s: %s",gestures[i],shortcut_names[state.shortcuts[i]]);
            lv_obj_t *b=button(text,83,157+i*54,300,48,860+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
            art(icons[state.shortcuts[i]],6,7,b);
            lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,246);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_align(l,LV_ALIGN_LEFT_MID,44,0);
        }
        small_label(input_check_notice,83,326,300,PD_GOLD);button("Indietro",83,359,144,48,50,true);button("Collaudo",239,359,144,48,2900,true);
    }
}

static void render_rest(void) {
    char text[180];
    if(page==REST_HOME) {
        label("Riposi e recuperi",83,111,300,true);
        button("Riposo breve",83,163,300,48,901,true);button("Riposo lungo",83,223,300,48,903,state.hp||!state.status.available||state.status.failures<3);
        small_label(!state.hp&&state.status.available&&state.status.failures==3?"Tre fallimenti: correggi da Stato":"Requisiti e durate: verifica con il DM",83,295,300,PD_GOLD);
        small_label("Contatori: recupero manuale",83,321,300,PD_MUTED);
        button("Usi",83,359,300,48,103,true);
    } else if(page==REST_LONG_SETTINGS) {
        label("Riposo lungo",83,111,300,true);
        snprintf(text,sizeof text,"Dadi vita: %s >",(const char *const[]){"manuale","2014","2024"}[rest_plan.policy]);button(text,83,157,300,48,904,true);
        small_label(rest_plan.policy==PD_HD_2014?"Meta del totale, min. 1; scegli dadi":rest_plan.policy==PD_HD_2024?"Recupera tutti i dadi vita spesi":"Dadi vita invariati; recuperi manuali",83,223,300,PD_GOLD);
        if(state.recovery.available)snprintf(text,sizeof text,"Dadi da recuperare: %u",rest_plan.policy==PD_HD_2024?pd_hit_budget(&state,rest_plan.policy):selected_hit_dice());else snprintf(text,sizeof text,"Dati assenti: importa nuova copia");small_label(text,83,249,300,PD_TEXT);
        button("Scegli dadi da recuperare",83,283,300,48,905,rest_plan.policy==PD_HD_2014&&state.recovery.available&&state.recovery.count);
        button("Indietro",83,359,144,48,900,true);button("Anteprima",239,359,144,48,906,true);
    } else if(page==REST_PREVIEW) {
        pd_rest_summary delta;if(!pd_rest_preview(&state,&rest_plan,&scratch,&delta))return;
        label(rest_plan.long_rest?"Conferma riposo lungo":"Conferma riposo breve",83,107,300,false);
        snprintf(text,sizeof text,"PF %u -> %u | temporanei %u -> %u",state.hp,scratch.hp,state.temp_hp,scratch.temp_hp);small_label(text,83,148,300,PD_MINT);
        snprintf(text,sizeof text,"Slot: +%u ordinari / +%u patto",delta.ordinary,delta.pact);small_label(text,83,175,300,PD_TEXT);
        snprintf(text,sizeof text,"Utilizzi: %u | cariche: +%u",delta.powers,delta.charges);small_label(text,83,202,300,PD_TEXT);
        if(state.recovery.available)snprintf(text,sizeof text,"Dadi vita recuperati: %u (%s)",delta.hit_dice,(const char *const[]){"manuale","2014","2024"}[rest_plan.policy]);else snprintf(text,sizeof text,"Dadi vita: dati non disponibili");small_label(text,83,229,300,PD_GOLD);
        small_label("Termina Ira e attacco potente",83,260,300,PD_TEXT);
        if(rest_plan.long_rest)snprintf(text,sizeof text,"Concentr.: fine | %u effetti terminati",pd_concentration_effects(&state));else strcpy(text,"Concentrazione: invariata");small_label(text,83,284,300,PD_TEXT);
        small_label(rest_plan.long_rest&&state.status.available?"Registro morte: azzera al riposo":"Contatori invariati; eccezioni: DM",83,309,300,PD_MUTED);
        small_label(rest_plan.long_rest?"Requisiti e durate: verifica DM":"Poi spendi dadi vita per curarti",83,333,300,PD_MUTED);
        button("Indietro",83,359,144,48,902,true);button("Conferma",239,359,144,48,907,true);
    } else if(page==REST_SESSION) {
        label("Riposo breve applicato",83,111,300,false);
        snprintf(text,sizeof text,"PF %u/%u | COS %+d",state.hp,pd_max_hp(&state),pd_modifier(state.abilities[2]));label(text,83,159,300,false);
        button("Spendi un dado vita",83,211,300,48,909,state.recovery.available&&state.recovery.count&&state.hp>0&&state.hp<pd_max_hp(&state));
        small_label(!state.recovery.available?"Dadi vita assenti: importa una copia":!state.recovery.count?"Nessuna riserva dadi vita nella scheda":state.hp==0?"A zero PF: verifica con il DM":"Cura base: dado + COS, minimo zero",83,289,300,PD_GOLD);
        small_label("Ogni spesa richiede conferma",83,318,300,PD_MUTED);button("Fine",83,359,300,48,918,true);
    } else if(page==REST_REGAIN||page==HIT_DICE) {
        bool regain=page==REST_REGAIN;const pd_hit_die *h=&state.recovery.pools[hit_index];
        label(regain?"Recupera dadi":"Dadi vita",139,111,188,false);
        button("<",83,99,48,44,912,hit_index>0);button(">",335,99,48,44,913,hit_index+1<state.recovery.count);
        if(regain)snprintf(text,sizeof text,"Scelti %u / limite %u",selected_hit_dice(),pd_hit_budget(&state,rest_plan.policy));
        else snprintf(text,sizeof text,"Riserva %u/%u | COS %+d",hit_index+1,state.recovery.count,pd_modifier(state.abilities[2]));
        small_label(text,83,153,300,PD_GOLD);
        lv_obj_t *name=small_label(h->name,83,183,300,PD_TEXT);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,58);
        snprintf(text,sizeof text,regain?"Spesi %u / %u d%u":"Disponibili %u / %u d%u",regain?h->used:h->maximum-h->used,h->maximum,h->die);label(text,83,243,300,false);
        if(regain) {
            button("-1",83,279,72,44,916,rest_plan.regain[hit_index]>0);snprintf(text,sizeof text,"%u",rest_plan.regain[hit_index]);label(text,163,289,140,false);
            button("+1",311,279,72,44,917,rest_plan.regain[hit_index]<h->used&&selected_hit_dice()<pd_hit_budget(&state,rest_plan.policy));
        } else button(state.hp==pd_max_hp(&state)?"PF al massimo":h->used==h->maximum?"Riserva esaurita":"Spendi 1 dado",83,279,300,44,915,rest_active&&state.hp>0&&state.hp<pd_max_hp(&state)&&h->used<h->maximum);
        button("Indietro",83,359,300,48,regain?908:911,true);
    } else if(page==HIT_CONFIRM) {
        const pd_hit_die *h=&state.recovery.pools[hit_index];label("Spendere un dado vita?",83,111,300,false);
        small_label(h->name,83,159,300,PD_TEXT);
        snprintf(text,sizeof text,"1d%u %+d COS | minimo 0 PF",h->die,pd_modifier(state.abilities[2]));label(text,83,201,300,false);
        snprintf(text,sizeof text,"PF attuali %u/%u",state.hp,pd_max_hp(&state));label(text,83,241,300,false);
        small_label("Consuma il dado anche con cura zero",83,285,300,PD_GOLD);
        small_label("Bonus speciali: verifica con il DM",83,314,300,PD_MUTED);
        button("Indietro",83,359,144,48,910,true);button("Conferma",239,359,144,48,914,true);
    }
}

static const int input_target_centers[PD_INPUT_CHECK_TARGETS][2]={{233,170},{328,234},{233,298},{138,234},{233,234}};
static void input_check_refresh(void){
    for(unsigned i=0;i<3;i++)if(input_check_counts[i]){
        bool available=input_check.available[i?1:0];
        if(available)lv_label_set_text_fmt(input_check_counts[i],"Ricevuti: %u",input_check.counts[i]);
        else lv_label_set_text(input_check_counts[i],"Lettura non disponibile");
        lv_obj_set_style_text_color(input_check_counts[i],lv_color_hex(!available?PD_GOLD:input_check.counts[i]?PD_MINT:PD_MUTED),0);
    }
}
static void render_input_check(void){
    char text[100];memset(input_check_counts,0,sizeof input_check_counts);
    if(page==INPUT_CHECK_HOME){
        label("Collaudo comandi",83,111,300,true);
        small_label("Prove locali; scheda conservata",83,153,300,PD_MUTED);
        button("Prova tocco",83,187,300,44,2901,true);
        button("Prova tasti",83,243,300,44,2902,true);
        unsigned seen=0;for(unsigned i=0;i<3;i++)seen+=input_check.counts[i]>0;
        snprintf(text,sizeof text,"Tocco %u/5 | gesti osservati %u/3",input_check.touched,seen);
        small_label(text,83,299,300,PD_MINT);
        small_label("Ogni prova si chiude dopo 2 minuti",83,326,300,PD_GOLD);
        button("Esci",83,359,300,48,2904,true);
    }else if(page==INPUT_CHECK_TOUCH){
        if(input_check.touched<PD_INPUT_CHECK_TARGETS)snprintf(text,sizeof text,"Tocca il bersaglio %u/5",input_check.touched+1);
        else snprintf(text,sizeof text,"Cinque tocchi riconosciuti");
        label(text,83,111,300,false);
        for(unsigned i=0;i<PD_INPUT_CHECK_TARGETS;i++){
            snprintf(text,sizeof text,"%u",i+1);
            lv_obj_t *b=button(text,input_target_centers[i][0]-28,input_target_centers[i][1]-28,56,56,2908+i,i==input_check.touched);
            lv_obj_set_style_border_width(b,i==input_check.touched?3:1,0);
            lv_obj_set_style_border_color(b,lv_color_hex(i<input_check.touched?PD_MINT:i==input_check.touched?PD_GOLD:PD_MUTED),0);
        }
        small_label(input_check.touched<5?"Tocca e rilascia; controlla la posizione":"Percorso completo; verifica la resa",83,326,300,PD_MUTED);
        button("Azzera",83,359,144,48,2901,true);button("Indietro",239,359,144,48,2903,true);
    }else{
        label("Prova i tasti",83,111,300,true);
        const char *names[]={"PWR breve","BOOT breve","BOOT lungo"};
        for(unsigned i=0;i<3;i++){
            snprintf(text,sizeof text,"%s: %s",names[i],shortcut_names[state.shortcuts[i]]);
            small_label(text,83,157+i*54,300,PD_TEXT);
            input_check_counts[i]=label("",83,179+i*54,300,false);
        }
        input_check_refresh();
        small_label("Rilascia prima del prossimo gesto",83,326,300,PD_GOLD);
        button("Azzera",83,359,144,48,2902,true);button("Indietro",239,359,144,48,2903,true);
    }
}
static void render_death_damage(void){
    pd_damage_event event=current_damage_event();pd_death_damage_summary r;
    if(!pd_death_damage_preview(&state,&event,death_damage_choice,&r))return;
    char text[160];
    label("Danni e morte",83,111,228,false);lv_obj_t *types=button("Tipi",323,99,60,44,1930,true);lv_obj_set_style_text_font(types,&lv_font_montserrat_16,0);
    snprintf(text,sizeof text,"Danni %u | temp %u -> %u",r.impact.total.adjusted,state.temp_hp,r.impact.total.temp_after);small_label(text,83,151,300,PD_GOLD);
    snprintf(text,sizeof text,"PF %u -> %u | massimo %u",state.hp,r.impact.total.hp_after,r.maximum);label(text,83,179,300,false);
    const char *problem=!r.eligible?"Colpo da ripreparare":!r.registry_available?"Registro non disponibile":r.already_dead?"Tre fallimenti gia presenti":r.registry_conflict?"Registro da correggere":r.reaction_at_zero?"Reazione a zero PF: verifica col DM":NULL;
    if(problem){
        small_label(problem,83,213,300,PD_DANGER);
        small_label(!r.registry_available?"Stato > Avvia registro; poi nuovo colpo":r.reaction_at_zero?"Indietro > Riduci: rimuovi la reazione":"Correggi con il DM; poi nuovo colpo",83,235,300,PD_MUTED);
    }else{
        if(r.instant)snprintf(text,sizeof text,"Morte base: fallite %u -> 3",r.failures_before);
        else if(r.critical_needed&&death_damage_choice.critical==PD_HIT_UNRESOLVED)snprintf(text,sizeof text,"Fallite %u -> ? | riuscite %u",r.failures_before,r.successes_before);
        else snprintf(text,sizeof text,"Fallite %u -> %u | riuscite %u -> %u",r.failures_before,r.failures_after,r.successes_before,r.successes_after);
        small_label(text,83,213,300,r.instant||r.failures_after==3?PD_DANGER:PD_MINT);
        if(r.instant)snprintf(text,sizeof text,"%s %u >= massimo %u",state.hp?"Eccedenza":"Danno a 0 PF",r.excess,r.maximum);
        else snprintf(text,sizeof text,r.stable_before?"Stabile: si -> no; danni gia completi":"Un solo colpo; importo gia completo");
        small_label(text,83,235,300,PD_MUTED);
    }
    snprintf(text,sizeof text,r.critical_needed?"Colpo critico: %s >":"Critico: non necessario",(const char *const[]){"da verificare","no","si"}[death_damage_choice.critical]);
    lv_obj_t *critical=button(text,83,255,300,44,3002,r.critical_needed&&!problem);lv_obj_set_style_text_font(critical,&lv_font_montserrat_16,0);
    lv_obj_t *verified=button(death_damage_choice.verified?"Base 2014: verificata":"Base 2014: da verificare",83,305,300,44,3003,!problem);lv_obj_set_style_text_font(verified,&lv_font_montserrat_16,0);
    button("Indietro",83,359,144,48,3001,true);button("Conferma",239,359,144,48,3004,r.allowed);
}
static void render_backup(void){
    char text[160];
    if(page==BACKUP_HOME){
        label("Backup USB",83,111,300,true);
        lv_obj_t *notice=small_label(backup_notice,83,164,300,PD_GOLD);lv_label_set_long_mode(notice,LV_LABEL_LONG_WRAP);lv_obj_set_height(notice,40);
        small_label("Collega il dispositivo al tuo PC.",83,228,300,PD_TEXT);
        small_label("Richiedi una copia o un ripristino.",83,260,300,PD_TEXT);
        small_label("Conferma qui prima di procedere.",83,304,300,PD_MUTED);
        button("Schermo",83,359,300,48,1600,true);return;
    }
    bool restoring=page==BACKUP_RESTORE_CONFIRM;
    label(restoring?"Ripristina copia?":"Invia copia al PC?",83,111,300,true);
    label(pending.name,83,152,300,false);
    snprintf(text,sizeof text,"PF %u/%u +%u temp | CA %u",pending.hp,pd_max_hp(&pending),pending.temp_hp,pd_armor(&pending));small_label(text,83,187,300,PD_MINT);
    snprintf(text,sizeof text,"Zaino %u | magie/usi %u",pending.item_count,pending.magic.count);small_label(text,83,215,300,PD_TEXT);
    snprintf(text,sizeof text,"Storico %u | monete %s",backup_history.count,pending.wallet.available?"verificate":"non disponibili");small_label(text,83,243,300,PD_GOLD);
    small_label(restoring?"Sostituisce scheda e storico locali.":"Nessuna modifica sul dispositivo.",83,274,300,PD_TEXT);
    small_label(restoring?"Ripristina anche tasti e schermo.":"Conserva il file ricevuto sul PC.",83,299,300,PD_MUTED);
    if(restoring&&strstr(backup_notice,"Errore"))small_label(backup_notice,83,326,300,PD_DANGER);
    button("Annulla",83,359,144,48,2801,true);button(restoring?"Ripristina":"Invia",239,359,144,48,restoring?2803:2802,true);
}
static void render_settings(void){
    char text[120];label("Schermo",83,111,300,true);
    snprintf(text,sizeof text,"Luminosita %u%%",state.brightness);label(text,83,155,300,false);
    button("-10",83,193,144,44,1601,state.brightness>10);button("+10",239,193,144,44,1602,state.brightness<100);
    snprintf(text,sizeof text,"Attenua: %s >",(const char *const[]){"mai","30 sec","60 sec","120 sec"}[state.dim_timeout]);button(text,83,251,300,44,1603,true);
    small_label(display_screen.failed?"Errore comando luminosita":"Attenuato al 10%; primo input riattiva",83,309,300,display_screen.failed?PD_DANGER:PD_GOLD);
    const char *names[]={"Indietro","Tasti","Backup"};const unsigned actions[]={50,850,2800};
    for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}

static void render_external_turn(void){
    char text[100];label("Turni esterni",83,111,300,false);
    for(unsigned i=1;i<PD_TURN_SUBJECTS;i++){
        lv_obj_t *b=button(turn_subject(i),83+(i-1)*104,151,92,44,3200+i,true);
        if(i==turn_owner)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
    }
    small_label("Associa A/B/C ai soggetti con il DM",83,207,300,PD_GOLD);
    snprintf(text,sizeof text,"Inizio turno %s",turn_subject(turn_owner));button(text,83,247,300,44,3204,true);
    snprintf(text,sizeof text,"Fine turno %s",turn_subject(turn_owner));button(text,83,301,300,44,3205,true);
    button("Indietro",83,359,300,48,3206,true);
}
static void render_death_roll_setup(void){
    char text[120];label("Tiro contro morte",83,111,300,false);
    snprintf(text,sizeof text,"Vantaggio: %s",status_options.advantage?"si":"no");lv_obj_t *a=button(text,83,151,144,44,3101,true);lv_obj_set_style_text_font(a,&lv_font_montserrat_16,0);
    snprintf(text,sizeof text,"Svantaggio: %s",status_options.disadvantage?"si":"no");lv_obj_t *d=button(text,239,151,144,44,3102,true);lv_obj_set_style_text_font(d,&lv_font_montserrat_16,0);
    int effective=pd_death_mode(status_options);
    small_label(status_options.advantage&&status_options.disadvantage?"Si annullano: un d20":effective>0?"Due d20: scegli il maggiore":effective<0?"Due d20: scegli il minore":"Un d20: modalita base",83,207,300,PD_GOLD);
    snprintf(text,sizeof text,"Bonus totale %+d",status_options.bonus);label(text,83,239,300,true);
    button("-1",83,279,144,44,3103,status_options.bonus> -30);button("+1",239,279,144,44,3104,status_options.bonus<30);
    small_label("Solo questo tiro; fonti con il DM",83,331,300,PD_MUTED);
    button("Indietro",83,359,144,48,3105,true);button("Anteprima",239,359,144,48,3106,pd_death_ready(&state));
}
static void render_status(void) {
    char text[180];
    if(page==STATUS_HOME){
        label("Stato",83,111,84,false);lv_obj_t *time=button("Tempo",179,99,84,44,2140,true);lv_obj_set_style_text_font(time,&lv_font_montserrat_16,0);lv_obj_t *react=button("Reazione",283,99,100,44,2117,true);lv_obj_set_style_text_font(react,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"PF %u/%u | temporanei %u",state.hp,pd_max_hp(&state),state.temp_hp);small_label(text,83,149,300,PD_MINT);
        button("Salvezze contro morte",83,177,300,44,1501,true);
        snprintf(text,sizeof text,"Condizioni: %u note",state.status.count);button(text,83,231,300,44,1503,true);
        button(state.training.poisoned?"Avvelenato: si":"Avvelenato: no",83,285,300,44,1504,true);
        const char *names[]={"Usi","Effetti","Danni"};const unsigned actions[]={103,1700,1800};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==DEATH_VIEW){
        label("Salvezze contro morte",83,111,300,false);snprintf(text,sizeof text,"PF %u/%u",state.hp,pd_max_hp(&state));label(text,83,149,300,false);
        small_label(death_caption(&state),83,180,300,PD_GOLD);
        if(state.status.available)snprintf(text,sizeof text,"Riuscite %u/3 | fallite %u/3",state.status.successes,state.status.failures);else snprintf(text,sizeof text,"Riuscite -- | fallite --");label(text,83,207,300,false);
        lv_obj_t *base=button("Lancia d20 base",83,243,192,44,1510,pd_death_ready(&state));lv_obj_set_style_text_font(base,&lv_font_montserrat_16,0);
        lv_obj_t *custom=button("Modifica",287,243,96,44,3100,pd_death_ready(&state));lv_obj_set_style_text_font(custom,&lv_font_montserrat_16,0);
        button(state.status.available?"Correggi / stabilizza":"Avvia registro",83,297,300,44,state.status.available?1502:1511,true);
        button("Stato",83,359,300,48,1500,true);
    } else if(page==DEATH_EDIT){
        label("Correzione con il DM",83,111,300,false);
        const char *names[]={"Riuscita +","Fallita +","Riuscita -","Fallita -","Stabilizza","Critico +2"};
        const unsigned actions[]={PD_DEATH_SUCCESS,PD_DEATH_FAILURE,PD_DEATH_MINUS_SUCCESS,PD_DEATH_MINUS_FAILURE,PD_DEATH_STABILIZE,PD_DEATH_CRITICAL};
        for(unsigned i=0;i<6;i++){
            scratch=state;bool enabled=pd_death_change(&scratch,actions[i]);
            lv_obj_t *b=button(names[i],83+(i%2)*156,149+(i/2)*52,144,44,1511+actions[i],enabled);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        }
        button("Azzera",83,305,300,44,1511,true);button("Indietro",83,359,300,48,1501,true);
    } else if(page==CONDITION_LIST||page==CONDITION_PICKER){
        bool pick=page==CONDITION_PICKER;unsigned count=pick?15:state.status.count,pages=count?(count+2)/3:1;
        unsigned *current=pick?&preset_page:&condition_page;if(*current>=pages)*current=pages-1;
        snprintf(text,sizeof text,"%s %u/%u",pick?"Aggiungi nota":"Condizioni",*current+1,pages);label(text,139,111,188,false);
        button("<",83,99,48,44,pick?1526:1521,*current>0);button(">",335,99,48,44,pick?1527:1522,*current+1<pages);
        small_label("Note; Tiri per effetti espliciti",83,151,300,PD_GOLD);
        if(!count)small_label(state.status.available?"Nessuna condizione annotata":"Dati assenti: importa o aggiungi note",83,231,300,PD_TEXT);
        for(unsigned row=0;row<3;row++) {
            unsigned i=*current*3+row;if(i>=count)break;
            lv_obj_t *b=button(pick?condition_presets[i]:state.status.conditions[i].name,83,183+row*48,300,44,(pick?1300:1200)+i,true);
            lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,284);lv_obj_set_height(l,lv_font_get_line_height(&lv_font_montserrat_20));lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);
        }
        if(pick)button("Indietro",83,359,300,48,1503,true);
        else{const char *names[]={"Indietro","Aggiungi","Tiri"};const unsigned actions[]={1500,1520,2200};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i!=1||state.status.count<PD_CONDITIONS);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}}
    } else if(page==CONDITION_DETAIL){
        const pd_condition *c=&state.status.conditions[condition_index];lv_obj_t *name=small_label(c->name,83,99,300,PD_TEXT);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,80);
        small_label("Nota; Avvelenato si regola da Stato",83,185,300,PD_GOLD);
        snprintf(text,sizeof text,"Livello %u",c->level);label(text,83,217,300,true);
        button("-1",83,251,144,44,1523,c->level>0);button("+1",239,251,144,44,1524,c->level<6);
        button("Rimuovi nota",83,305,300,44,1525,true);button("Elenco",83,359,300,48,1503,true);
    } else if(page==STATUS_CONFIRM){
        const char *title=status_command==STATUS_ROLL?"Tirare contro morte?":status_command==STATUS_POISON?"Effetto Avvelenato":status_command==STATUS_DEATH?"Correzione con il DM":status_command==STATUS_REMOVE?"Rimuovere la nota?":status_command==STATUS_ADD?"Aggiungere la nota?":"Cambiare livello?";
        label(title,83,111,300,false);
        if(status_command==STATUS_ROLL){
            int effective=pd_death_mode(status_options);
            snprintf(text,sizeof text,"%s | bonus %+d",effective>0?"Vantaggio":effective<0?"Svantaggio":"Base",status_options.bonus);label(text,83,163,300,false);
            small_label("1 naturale: 2 fallite | 20: 1 PF",83,207,300,PD_GOLD);
            small_label("Altrimenti totale >= 10: riuscita",83,237,300,PD_TEXT);
            if(status_rolled)snprintf(text,sizeof text,"Tiro fissato: %u %+d = %d",status_result.natural,status_result.bonus,status_result.total);
            else snprintf(text,sizeof text,status_options.advantage&&status_options.disadvantage?"Vantaggio e svantaggio si annullano":"Fonti e bonus verificati con il DM");
            small_label(text,83,281,300,status_rolled?PD_GOLD:PD_MUTED);
            if(status_rolled)small_label("Riprova salva lo stesso tiro",83,313,300,PD_MUTED);
        } else if(status_command==STATUS_POISON){
            label(state.training.poisoned?"Disattiva lo svantaggio":"Attiva lo svantaggio",83,163,300,false);
            small_label("Su prove e attacchi",83,207,300,PD_GOLD);small_label("Salvezze e dado libero invariati",83,239,300,PD_TEXT);small_label("Le note delle condizioni non cambiano",83,281,300,PD_MUTED);
        } else if(status_command==STATUS_DEATH){
            if(status_change(&scratch)){
                snprintf(text,sizeof text,"Riuscite %u -> %u",state.status.successes,scratch.status.successes);label(text,83,163,300,false);
                snprintf(text,sizeof text,"Fallite %u -> %u",state.status.failures,scratch.status.failures);label(text,83,201,300,false);
                small_label(scratch.status.stabilized?"Stabilizzato: si":"Stabilizzato: no",83,243,300,PD_GOLD);
                small_label(state.status.available?"Correzione esplicita; verifica con il DM":"Avvia un nuovo registro locale",83,289,300,PD_MUTED);
            }
        } else {
            const char *name=status_command==STATUS_ADD?condition_presets[preset_index]:state.status.conditions[condition_index].name;
            lv_obj_t *l=small_label(name,83,159,300,PD_TEXT);lv_label_set_long_mode(l,LV_LABEL_LONG_WRAP);lv_obj_set_height(l,80);
            if(status_command==STATUS_LEVEL){status_change(&scratch);snprintf(text,sizeof text,"Livello %u -> %u",state.status.conditions[condition_index].level,scratch.status.conditions[condition_index].level);label(text,83,255,300,false);}
            small_label("Annotazione: nessun effetto automatico",83,301,300,PD_GOLD);
        }
        button("Indietro",83,359,144,48,1506,status_command!=STATUS_ROLL||!status_rolled);button(status_command==STATUS_ROLL&&status_rolled?"Riprova":"Conferma",239,359,144,48,1505,true);
    }
}

static void render_effects(void) {
    char text[160];
    if(page==EFFECT_LIST) {
        unsigned pages=state.effect_count?(state.effect_count+2)/3:1;
        if(effect_page>=pages)effect_page=pages-1;
        label("Effetti locali",83,99,300,true);
        snprintf(text,sizeof text,"CA %u | PF max %u | %u/%u",pd_armor(&state),pd_max_hp(&state),effect_page+1,pages);small_label(text,83,133,300,PD_MINT);
        if(!state.effect_count)label("Nessun modificatore locale",83,225,300,false);
        for(unsigned row=0;row<3;row++){
            unsigned i=effect_page*3+row;if(i>=state.effect_count)break;
            const pd_effect *e=&state.effects[i];
            char duration[24];if(!e->timed)snprintf(duration,sizeof duration,"%s",e->active?"si":"no");
            else if(!duration_left(e->tick,e->rounds,e->minutes))strcpy(duration,"scaduto");else snprintf(duration,sizeof duration,"%u%s%s",duration_left(e->tick,e->rounds,e->minutes),duration_owned_mark(e->tick,e->owner),e->active?"":" pausa");
            snprintf(text,sizeof text,"%u. %s %+d | %s",i+1,e->target==PD_EFFECT_ARMOR?"CA":"PF max",e->value,duration);
            if(e->concentration)strcat(text," | C");
            lv_obj_t *b=button(text,83,159+row*48,300,44,1720+i,true);if(e->timed||e->concentration)lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        }
        button("<",83,305,60,44,1706,effect_page>0);button("1 round",155,305,156,44,1740,timed_effects());button(">",323,305,60,44,1707,effect_page+1<pages);
        button("Stato",83,359,144,48,1500,true);button("Aggiungi",239,359,144,48,1701,state.effect_count<PD_EFFECTS);
    } else if(page==EFFECT_EDIT) {
        snprintf(text,sizeof text,effect_index<state.effect_count?"Effetto locale %u":"Nuovo effetto %u",effect_index+1);label(text,83,107,300,true);
        button(effect_draft.target==PD_EFFECT_ARMOR?"Modifica: CA >":"Modifica: PF massimi >",83,143,300,44,1702,true);
        snprintf(text,sizeof text,"Modificatore %+d",effect_draft.value);label(text,83,199,300,true);
        const char *names[]={"-10","-1","+1","+10"};int bound=effect_draft.target==PD_EFFECT_ARMOR?20:999;
        for(unsigned i=0;i<4;i++)button(names[i],83+i*78,235,66,44,1730+i,i<2?effect_draft.value> -bound:effect_draft.value<bound);
        button(effect_draft.active?"Attivo: si":"Attivo: no",83,291,144,44,1703,true);
        button("Rimuovi",239,291,144,44,1705,effect_index<state.effect_count);
        small_label("Variazioni aggiuntive ai valori base",83,339,300,PD_GOLD);
        const char *actions[]={"Elenco","Durata","Anteprima"};const unsigned codes[]={1700,1710,1704};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==EFFECT_DURATION) {
        label("Durata dell'effetto",83,111,300,true);
        snprintf(text,sizeof text,"Durata: %s >",duration_owned(effect_draft.timed,effect_draft.tick,effect_draft.owner));lv_obj_t *duration=button(text,83,157,300,44,1711,true);lv_obj_set_style_text_font(duration,&lv_font_montserrat_16,0);
        if(effect_draft.timed&&effect_draft.tick>=PD_TICK_MINUTES){if(effect_draft.tick==PD_TICK_HOURS)time_text(text,sizeof text,effect_draft.minutes);else snprintf(text,sizeof text,"%u minuti residui",effect_draft.minutes);}else if(effect_draft.timed)snprintf(text,sizeof text,effect_draft.tick?"%u passaggi residui":"%u round residui",effect_draft.rounds);else strcpy(text,"Senza scadenza");label(text,83,215,300,true);
        const char *names[]={"-10","-1","+1","+10"};
        for(unsigned i=0;i<4;i++)button(effect_draft.tick==PD_TICK_HOURS?(const char *const[]){"-1h","-1m","+1m","+1h"}[i]:names[i],83+i*78,263,66,44,1712+i,effect_draft.timed&&(i<2?duration_left(effect_draft.tick,effect_draft.rounds,effect_draft.minutes)>0:duration_left(effect_draft.tick,effect_draft.rounds,effect_draft.minutes)<(effect_draft.tick==PD_TICK_HOURS?5940:99)));
        if(effect_draft.timed&&turn_clock(effect_draft.tick)){snprintf(text,sizeof text,"Turno: %s >",turn_subject(effect_draft.owner));button(text,83,311,300,44,3210,true);}
        else{small_label(effect_draft.tick>=PD_TICK_MINUTES?"Tempo dal menu Stato":"Round manuali dal menu Effetti",83,319,300,PD_GOLD);small_label("Disattivato: conteggio in pausa",83,339,300,PD_MUTED);}
        const char *names2[]={"Indietro","Vincolo","Anteprima"};const unsigned codes[]={1716,1750,1704};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names2[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==EFFECT_LINK) {
        label("Vincolo dell'effetto",83,111,300,true);
        lv_obj_t *b=button(effect_draft.concentration?"Concentrazione: si >":"Concentrazione: no >",83,157,300,44,1751,effect_draft.concentration||state.magic.concentration);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        small_label(state.magic.concentration?state.magic.powers[state.magic.concentration-1].name:"Nessuna magia concentrata",83,215,300,PD_GOLD);
        small_label("Alla fine: disattiva e scollega",83,255,300,PD_TEXT);
        small_label("Anche rilanciando la stessa magia",83,281,300,PD_TEXT);
        small_label("Nessuna riattivazione automatica",83,311,300,PD_MUTED);
        small_label("Il timer puo scadere prima",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,1752,true);button("Anteprima",239,359,144,48,1704,true);
    } else if(page==EFFECT_CONFIRM) {
        bool valid=preview_effect(&scratch);
        label(effect_removing?"Rimuovere l'effetto?":"Confermare l'effetto?",83,107,300,false);
        snprintf(text,sizeof text,"Base: CA %u | PF max %u",state.armor,state.max_hp);small_label(text,83,151,300,PD_GOLD);
        small_label("Aggiungi solo bonus non gia inclusi",83,179,300,PD_GOLD);
        const pd_effect *e=effect_removing?&state.effects[effect_index]:&effect_draft;
        if(e->timed)snprintf(text,sizeof text,"%u %s | %s",duration_left(e->tick,e->rounds,e->minutes),duration_owned(e->timed,e->tick,e->owner),e->active?"attivo":"pausa");else strcpy(text,"Manuale");strcat(text,e->concentration?" | conc. si":" | conc. no");small_label(text,83,199,300,PD_MUTED);
        if(valid){
            snprintf(text,sizeof text,"CA %u -> %u",pd_armor(&state),pd_armor(&scratch));label(text,83,219,300,false);
            snprintf(text,sizeof text,"PF massimi %u -> %u",pd_max_hp(&state),pd_max_hp(&scratch));label(text,83,251,300,false);
            snprintf(text,sizeof text,"PF correnti %u -> %u",state.hp,scratch.hp);label(text,83,283,300,false);
            small_label(state.hp>scratch.hp?"PF oltre il nuovo massimo: persi":"Il massimo maggiore non cura",83,325,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);
        } else {
            bool expired=e->timed&&e->active&&!duration_left(e->tick,e->rounds,e->minutes);
            label(expired?"Durata da verificare":"Valori fuori limite",83,227,300,false);small_label(expired?"Attivo: serve almeno 1 passaggio":"CA: 1-40 | PF massimi: 1-999",83,267,300,PD_DANGER);
            small_label(expired?"Imposta la durata prima di riattivare":"Verifica anche senza concentrazione",83,305,300,PD_MUTED);
        }
        button("Indietro",83,359,144,48,1708,true);button("Conferma",239,359,144,48,1709,valid);
    } else if(page==EFFECT_ROUND) {
        pd_round_summary r={0};bool valid=preview_round(&scratch,&r);uint8_t advanced=r.effects_advanced,expired=r.effects_expired;
        label("Avanzare 1 round?",83,107,300,true);
        snprintf(text,sizeof text,"Effetti %u | Difese %u a round",bit_count(r.effects_advanced),bit_count(r.defences_advanced));small_label(text,83,151,300,PD_GOLD);
        if(valid){
            if(r.defences_advanced)snprintf(text,sizeof text,"Difese in scadenza: %u (vedi Difese)",bit_count(r.defences_expired));else{strcpy(text,"Avanzano:");for(unsigned i=0;i<PD_EFFECTS;i++)if(advanced&(1u<<i)){size_t n=strlen(text);snprintf(text+n,sizeof text-n," %u",i+1);}}small_label(text,83,171,300,PD_MUTED);
            snprintf(text,sizeof text,"CA %u -> %u",pd_armor(&state),pd_armor(&scratch));label(text,83,193,300,false);
            snprintf(text,sizeof text,"PF massimi %u -> %u",pd_max_hp(&state),pd_max_hp(&scratch));label(text,83,225,300,false);
            snprintf(text,sizeof text,"PF correnti %u -> %u",state.hp,scratch.hp);label(text,83,257,300,false);
            strcpy(text,"Scadono:");for(unsigned i=0;i<PD_EFFECTS;i++)if(expired&(1u<<i)){size_t n=strlen(text);snprintf(text+n,sizeof text-n," %u",i+1);}if(!expired)strcat(text," nessuno");small_label(text,83,299,300,PD_MINT);
            small_label(state.hp>scratch.hp?"PF oltre il nuovo massimo: persi":"Nessuna cura automatica",83,329,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);
        } else {
            label("Scadenze fuori limite",83,211,300,false);small_label("Nessun conteggio viene avanzato",83,259,300,PD_DANGER);
            small_label("Correggi gli effetti prima di proseguire",83,303,300,PD_MUTED);
        }
        const char *names[]={"Indietro","Difese","Conferma"};const unsigned codes[]={1741,1743,1742};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,codes[i],i==0||(valid&&(i==1?r.defences_advanced:(advanced||r.defences_advanced))));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==ROUND_DEFENCES){
        pd_round_summary r={0};if(!preview_round(&scratch,&r)){label("Scadenze da verificare",83,211,300,false);button("Riepilogo",83,359,300,48,1744,true);return;}unsigned types[PD_TIMED_DEFENCES],count=0;
        for(unsigned i=0;i<PD_TIMED_DEFENCES;i++)if(r.defences_advanced&(UINT64_C(1)<<i))types[count++]=i;
        unsigned pages=count?(count+2)/3:1;if(round_defence_page>=pages)round_defence_page=pages-1;
        label("Difese dopo 1 round",83,99,300,true);
        snprintf(text,sizeof text,"Scadono %u | pagina %u/%u",bit_count(r.defences_expired),round_defence_page+1,pages);small_label(text,83,133,300,PD_GOLD);
        for(unsigned row=0;row<3;row++){
            unsigned at=round_defence_page*3+row;if(at>=count)break;unsigned bit=types[at],i=bit%PD_DAMAGE_TYPES,slot=bit/PD_DAMAGE_TYPES;
            if(bit<PD_DEFENCE_GRANTS){
                const pd_defence_temporary *a=pd_defence_grant(&state,i,slot),*b=pd_defence_grant(&scratch,i,slot);
                snprintf(text,sizeof text,"%s #%u: %u -> %ur",pd_damage_name(i),slot+1,a->rounds,b->rounds);label(text,83,159+row*56,300,false);
                snprintf(text,sizeof text,"%s -> %s",defence_profile(&state,i),defence_profile(&scratch,i));
            }else{unsigned index=bit-PD_DEFENCE_GRANTS;const pd_reduction_profile *a=&state.reductions[index],*b=&scratch.reductions[index];snprintf(text,sizeof text,"Riduz. %u: %u -> %ur",index+1,a->rounds,b->rounds);label(text,83,159+row*56,300,false);snprintf(text,sizeof text,"%s -> %s",a->active?"attiva":"inattiva",b->active?"attiva":"inattiva");}
            small_label(text,83,187+row*56,300,PD_MINT);
        }
        button("<",83,359,60,48,1745,round_defence_page>0);button("Riepilogo",155,359,156,48,1744,true);button(">",323,359,60,48,1746,round_defence_page+1<pages);
    }
}

static const char *defence_caption(bool configured,unsigned flags){return !configured?"da configurare":(flags&PD_IMMUNITY)?"immunita":flags==3?"resist. + vulner.":(flags&PD_RESISTANCE)?"resistenza":(flags&PD_VULNERABILITY)?"vulnerabilita":"normale";}
static const char *defence_profile(const pd_state *s,unsigned type){return !(s->defences.configured&(1u<<type))?"da configurare":pd_defence_conditional(s,type)?"condizionale":defence_caption(true,pd_defence_flags(s,type));}
static void render_item_charges(void){
    const pd_item *v=&state.items[item_index];char text[160];
    lv_obj_t *name=small_label(v->name,83,99,300,PD_TEXT);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,58);
    snprintf(text,sizeof text,"Cariche %u / %u",v->charges.current,v->charges.maximum);label(text,83,164,300,false);
    if(page==ITEM_CHARGE_CONFIRM){
        snprintf(text,sizeof text,"Saldo: %u -> %u",v->charges.current,charge_draft);label(text,83,207,300,true);
        small_label("Quantita ed effetti invariati",83,255,300,PD_GOLD);small_label("Annulla sull'Eroe ripristina il saldo",83,283,300,PD_MUTED);
        button("Indietro",83,359,144,48,2167,true);button("Conferma",239,359,144,48,2168,v->quantity);return;
    }
    snprintf(text,sizeof text,"Recupero: %s",(const char *const[]){"manuale","breve / lungo","lungo"}[v->charges.recovery]);small_label(text,83,200,300,PD_GOLD);
    if(v->quantity)snprintf(text,sizeof text,"Nuovo saldo: %u",charge_draft);else strcpy(text,"Oggetto non disponibile");label(text,83,233,300,false);
    const char *const labels[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(labels[i],83+i*78,277,66,44,2162+i,v->quantity);
    small_label("Cariche separate dalla quantita",83,331,300,PD_MUTED);
    const char *const names[]={"Indietro","Ricarica","Anteprima"};const intptr_t actions[]={2161,2170,2166};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i==0||(i==1?recharge_available():v->quantity));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
}
static void render_recharge(void){
    char text[180];bool receipt=page==ITEM_RECHARGE_RESULT;const char *name=receipt?recharge_name:state.items[item_index].name;
    lv_obj_t *title=small_label(name,83,99,300,PD_TEXT);lv_label_set_long_mode(title,LV_LABEL_LONG_WRAP);lv_obj_set_height(title,58);
    if(receipt){
        snprintf(text,sizeof text,"Cariche recuperate: +%u",recharge_gained);label(text,83,164,300,false);
        snprintf(text,sizeof text,"Tiro %ud%u %+d = %d",recharge_result.count,recharge_result.sides,recharge_result.bonus,recharge_result.total);small_label(text,83,211,300,PD_GOLD);
        snprintf(text,sizeof text,"Somma dadi: %d",recharge_result.sum);small_label(text,83,245,300,PD_TEXT);
        snprintf(text,sizeof text,"Saldo %u -> %u / %u",recharge_before,recharge_after,recharge_maximum);label(text,83,279,300,false);
        if(recharge_result.sum+recharge_result.bonus<0)strcpy(text,"Recupero minimo zero");else snprintf(text,sizeof text,"Eccesso non applicato: %u",(unsigned)recharge_result.total-recharge_gained);small_label(text,83,315,300,PD_MUTED);
        button("Cariche",83,359,144,48,2181,true);button("Fine",239,359,144,48,100,true);return;
    }
    if(page==ITEM_RECHARGE_CONFIRM){
        label("Ricaricare le cariche?",83,164,300,false);snprintf(text,sizeof text,"%ud%u %+d",recharge_plan.count,recharge_plan.sides,recharge_plan.bonus);label(text,83,207,300,true);
        snprintf(text,sizeof text,"Saldo %u / %u | spazio %u",state.items[item_index].charges.current,state.items[item_index].charges.maximum,state.items[item_index].charges.maximum-state.items[item_index].charges.current);small_label(text,83,249,300,PD_GOLD);
        small_label("Evento e regole: decidi col DM",83,283,300,PD_TEXT);small_label(recharge_rolled?"Tiro conservato: riprova il salvataggio":"Dadi dopo il salvataggio del saldo",83,315,300,PD_MUTED);
        button("Indietro",83,359,144,48,2178,!recharge_rolled);lv_obj_t *b=button(recharge_rolled?"Riprova":"Tira e salva",239,359,144,48,2179,recharge_rolled||recharge_available());lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);return;
    }
    label("Ricarica a dado",83,164,300,true);button("-",83,205,66,44,2173,recharge_plan.count>1);snprintf(text,sizeof text,"%ud%u >",recharge_plan.count,recharge_plan.sides);button(text,161,205,144,44,2172,true);button("+",317,205,66,44,2174,recharge_plan.count<PD_RECHARGE_MAX_DICE);
    button("-",83,259,66,44,2175,recharge_plan.bonus>PD_POOL_MIN_BONUS);snprintf(text,sizeof text,"Bonus %+d",recharge_plan.bonus);label(text,161,271,144,false);button("+",317,259,66,44,2176,recharge_plan.bonus<PD_POOL_MAX_BONUS);
    small_label("Minimo zero; limite al massimo",83,309,300,PD_GOLD);small_label("Alba e altre regole: scelta manuale",83,335,300,PD_MUTED);button("Indietro",83,359,144,48,2171,true);button("Anteprima",239,359,144,48,2177,recharge_available());
}
static void render_roll_conditions(void){
    char text[160];
    if(page==ROLL_CONTEXT){
        label("Circostanze del tiro",83,111,300,true);snprintf(text,sizeof text,"Richiede vista: %s >",context_answer(PD_CONTEXT_SIGHT));lv_obj_t *b=button(text,83,157,300,44,2232,context_relevant(PD_CONTEXT_SIGHT));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        snprintf(text,sizeof text,"Fonte paura in vista: %s >",context_answer(PD_CONTEXT_FEAR));b=button(text,83,205,300,44,2233,context_relevant(PD_CONTEXT_FEAR));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        b=button(context_ignore_draft?"Effetti tiro: esclusi >":"Effetti tiro: applicati >",83,253,300,44,2234,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        pd_condition_result effect={0};bool valid=current_condition_context(context_draft,context_ignore_draft,&effect);small_label(!valid?"Tiro non disponibile":effect.needed?"Dichiara i dati necessari":effect.automatic_failure?"Fallimento automatico: nessun dado":"Dati sufficienti per il tiro",83,309,300,PD_GOLD);small_label("Escludi effetti solo su decisione DM",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,2231,true);lv_obj_t *use=button("Usa contesto",239,359,144,48,2235,valid&&!effect.needed);lv_obj_set_style_text_font(use,&lv_font_montserrat_16,0);return;
    }
    if(page==CONDITION_EFFECTS){label("Effetti sui tiri",83,111,300,true);for(unsigned i=0;i<4;i++){snprintf(text,sizeof text,"%s: %s >",roll_condition_names[i],condition_effects_draft&(1u<<i)?"si":"no");button(text,83,157+i*48,300,44,2201+i,true);}button("Indietro",83,359,144,48,2205,true);button("Anteprima",239,359,144,48,2206,true);return;}
    label("Salvare gli effetti?",83,111,300,false);for(unsigned i=0;i<4;i++){snprintf(text,sizeof text,"%s: %s -> %s",roll_condition_names[i],state.roll_conditions&(1u<<i)?"si":"no",condition_effects_draft&(1u<<i)?"si":"no");small_label(text,83,169+i*38,300,PD_TEXT);}small_label("Note, durate e PF invariati",83,325,300,PD_MUTED);button("Indietro",83,359,144,48,2207,true);button("Conferma",239,359,144,48,2208,true);
}
static void render_damage(void) {
    if(page==CONDITION_EFFECTS||page==CONDITION_EFFECTS_CONFIRM||page==ROLL_CONTEXT){render_roll_conditions();return;}
    if(page==ITEM_RECHARGE||page==ITEM_RECHARGE_CONFIRM||page==ITEM_RECHARGE_RESULT){render_recharge();return;}
    if(page==ITEM_CHARGES||page==ITEM_CHARGE_CONFIRM){render_item_charges();return;}
    char text[180];
    if(page==DAMAGE_HOME){
        label("Danni ricevuti",83,111,204,false);lv_obj_t *reduce=button("Riduci",299,99,84,44,2040,true);lv_obj_set_style_text_font(reduce,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"Tipo: %s >",pd_damage_name(damage_plan.type));button(text,83,157,300,44,1801,true);
        if(damage_extra_count)snprintf(text,sizeof text,"1/%u: %u danni",damage_extra_count+1,damage_plan.amount);else snprintf(text,sizeof text,"%u %s",damage_plan.amount,damage_plan.amount==1?"danno":"danni");label(text,83,215,300,true);
        const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(names[i],83+i*78,251,66,44,1840+i,i<2?damage_plan.amount>1:damage_plan.amount<9999);
        lv_obj_t *parts=button("Componenti",83,305,144,44,1900,true);lv_obj_set_style_text_font(parts,&lv_font_montserrat_16,0);lv_obj_t *def=button("Difese per tipo",239,305,144,44,1802,true);lv_obj_set_style_text_font(def,&lv_font_montserrat_16,0);button("Stato",83,359,144,48,1500,true);button("Anteprima",239,359,144,48,1803,true);
    } else if(page==DAMAGE_PARTS){
        unsigned count=damage_extra_count+1,pages=(count+2)/3;if(damage_part_page>=pages)damage_part_page=pages-1;
        label("Componenti del colpo",83,99,300,true);snprintf(text,sizeof text,"Un solo colpo | %u/%u parti",count,PD_DAMAGE_COMPONENTS);small_label(text,83,133,300,PD_GOLD);
        pd_damage_event event=current_damage_event();for(unsigned row=0;row<3;row++){unsigned i=damage_part_page*3+row;if(i>=count)break;snprintf(text,sizeof text,"%u. %s: %u",i+1,pd_damage_name(event.components[i].type),event.components[i].amount);lv_obj_t *b=button(text,83,159+row*48,300,44,1910+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        button("<",83,305,60,44,1902,damage_part_page>0);button("Anteprima",155,305,156,44,1803,true);button(">",323,305,60,44,1903,damage_part_page+1<pages);
        button("Indietro",83,359,144,48,1904,true);button("Aggiungi",239,359,144,48,1901,count<PD_DAMAGE_COMPONENTS);
    } else if(page==DAMAGE_PART_EDIT){
        snprintf(text,sizeof text,"Componente %u",damage_part_index+1);label(text,83,111,204,false);lv_obj_t *reduce=button("Riduci",299,99,84,44,2041,true);lv_obj_set_style_text_font(reduce,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"Tipo: %s >",pd_damage_name(damage_part_draft.type));button(text,83,157,300,44,1920,true);
        snprintf(text,sizeof text,"%u danni",damage_part_draft.amount);label(text,83,215,300,true);const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(names[i],83+i*78,251,66,44,1921+i,i<2?damage_part_draft.amount>1:damage_part_draft.amount<9999);
        const char *actions[]={"Rimuovi","Fonte","Difese"};const unsigned codes[]={1927,2020,1928};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,305,92,44,codes[i],i|| (damage_extra_count&&damage_part_index<=damage_extra_count));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        button("Indietro",83,359,144,48,1926,true);button("Usa parte",239,359,144,48,1925,true);
    } else if(page==DAMAGE_TYPES){
        unsigned pages=(damage_details.count+2)/3;if(!pages)pages=1;if(damage_type_page>=pages)damage_type_page=pages-1;
        label("Tipi del colpo",83,99,204,false);lv_obj_t *math=button("Conto",299,99,84,44,2050,true);lv_obj_set_style_text_font(math,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"%u gruppi | pagina %u/%u",damage_details.count,damage_type_page+1,pages);small_label(text,83,133,204,PD_GOLD);
        for(unsigned row=0;row<3;row++){unsigned i=damage_type_page*3+row;if(i>=damage_details.count)break;const pd_damage_group *g=&damage_details.groups[i];snprintf(text,sizeof text,"%s | %s",pd_damage_name(g->type),damage_details.profile&&!damage_details.total.configured?"profilo da verificare":g->configured&&g->unresolved?"fonte da verificare":defence_caption(g->configured,g->flags));small_label(text,83,159+row*56,300,g->configured?PD_GOLD:PD_DANGER);if(g->configured&&!g->unresolved&&(!damage_details.profile||damage_details.total.configured)){if(g->reduction)snprintf(text,sizeof text,"%u - %u -> %u",g->amount,g->reduction,g->adjusted);else snprintf(text,sizeof text,"%u -> %u",g->amount,g->adjusted);}else snprintf(text,sizeof text,"%u -> ?",g->amount);label(text,83,183+row*56,300,false);}
        small_label(has_reductions(&damage_details)?"Riduzioni e totale: vedi Conto":"Stesso tipo e difese: somma prima",83,333,300,PD_MUTED);button("<",83,359,60,48,1932,damage_type_page>0);button("Riepilogo",155,359,156,48,1931,true);button(">",323,359,60,48,1933,damage_type_page+1<pages);
    } else if(page==DEFENCE_LIST){
        unsigned pages=(PD_DAMAGE_TYPES+2)/3;if(defence_page>=pages)defence_page=pages-1;
        snprintf(text,sizeof text,"Difese manuali %u/%u",defence_page+1,pages);label(text,83,99,300,true);small_label("Non dedotte da Beyond o dal testo",83,133,300,PD_GOLD);
        for(unsigned row=0;row<3;row++){
            unsigned i=defence_page*3+row;if(i>=PD_DAMAGE_TYPES)break;
            snprintf(text,sizeof text,"%s | %s",pd_damage_name(i),defence_profile(&state,i));
            unsigned active=active_defences(&state,i);if(active==1)strcat(text," +T");else if(active){size_t n=strlen(text);snprintf(text+n,sizeof text-n," +%uT",active);}
            lv_obj_t *b=button(text,83,159+row*48,300,44,1820+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        }
        button("<",83,305,60,44,1850,defence_page>0);button("1 round",155,305,156,44,1740,timed_effects());button(">",323,305,60,44,1851,defence_page+1<pages);
        button("Indietro",83,359,300,48,1852,true);
    } else if(page==DEFENCE_EDIT){
        label(pd_damage_name(defence_index),83,111,204,false);lv_obj_t *rule=button("Regole",299,99,84,44,2000,true);lv_obj_set_style_text_font(rule,&lv_font_montserrat_16,0);small_label("Profilo base; difesa a tempo separata",83,149,300,PD_GOLD);
        const char *names[]={"Resistenza","Vulnerabilita","Immunita","Configurato"};
        for(unsigned i=0;i<4;i++){snprintf(text,sizeof text,"%s: %s",names[i],(i==3?defence_configured:(defence_flags&(1u<<i))!=0)?"si":"no");button(text,83,173+i*46,300,44,1810+i,true);}
        const char *actions[]={"Indietro","A tempo","Anteprima"};const unsigned codes[]={1814,1870,1815};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==DEFENCE_TEMPORARY){
        snprintf(text,sizeof text,"Extra %u di %u >",defence_slot+1,PD_DEFENCE_SLOTS);lv_obj_t *pick=button(text,83,99,204,44,2030,true);lv_obj_set_style_text_font(pick,&lv_font_montserrat_16,0);lv_obj_t *rule=button("Regole",299,99,84,44,2001,true);lv_obj_set_style_text_font(rule,&lv_font_montserrat_16,0);small_label(pd_damage_name(defence_index),83,149,300,PD_GOLD);
        const char *names[]={"Resistenza","Vulnerabilita","Immunita","Attiva"};
        for(unsigned i=0;i<4;i++){snprintf(text,sizeof text,"%s: %s",names[i],(i==3?defence_temporary_draft.active:(defence_temporary_draft.flags&(1u<<i))!=0)?"si":"no");button(text,83,173+i*46,300,44,1871+i,true);}
        const char *actions[]={"Base","Durata","Anteprima"};const unsigned codes[]={1875,1876,1887};
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==DEFENCE_DURATION){
        label("Durata della difesa",83,111,300,true);
        snprintf(text,sizeof text,"Durata: %s >",duration_owned(defence_temporary_draft.timed,defence_temporary_draft.tick,defence_temporary_draft.owner));lv_obj_t *duration=button(text,83,157,300,44,1877,true);lv_obj_set_style_text_font(duration,&lv_font_montserrat_16,0);
        if(defence_temporary_draft.timed&&defence_temporary_draft.tick>=PD_TICK_MINUTES){if(defence_temporary_draft.tick==PD_TICK_HOURS)time_text(text,sizeof text,defence_temporary_draft.minutes);else snprintf(text,sizeof text,"%u minuti residui",defence_temporary_draft.minutes);}else if(defence_temporary_draft.timed)snprintf(text,sizeof text,defence_temporary_draft.tick?"%u passaggi residui":"%u round residui",defence_temporary_draft.rounds);else strcpy(text,"Senza scadenza");label(text,83,215,300,true);
        const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(defence_temporary_draft.tick==PD_TICK_HOURS?(const char *const[]){"-1h","-1m","+1m","+1h"}[i]:names[i],83+i*78,263,66,44,1880+i,defence_temporary_draft.timed&&(i<2?duration_left(defence_temporary_draft.tick,defence_temporary_draft.rounds,defence_temporary_draft.minutes)>0:duration_left(defence_temporary_draft.tick,defence_temporary_draft.rounds,defence_temporary_draft.minutes)<(defence_temporary_draft.tick==PD_TICK_HOURS?5940:99)));
        if(defence_temporary_draft.timed&&turn_clock(defence_temporary_draft.tick)){snprintf(text,sizeof text,"Turno: %s >",turn_subject(defence_temporary_draft.owner));button(text,83,311,300,44,3211,true);}
        else{small_label(defence_temporary_draft.tick>=PD_TICK_MINUTES?"Tempo dal menu Stato":"1 round avanza anche gli effetti",83,319,300,PD_GOLD);small_label("Disattivata: conteggio in pausa",83,339,300,PD_MUTED);}
        const char *actions[]={"Indietro","Vincolo","Anteprima"};const unsigned codes[]={1879,1878,1887};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(actions[i],83+i*104,359,92,48,codes[i],true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==DEFENCE_LINK){
        label("Vincolo della difesa",83,111,300,true);
        lv_obj_t *b=button(defence_temporary_draft.concentration?"Concentrazione: si >":"Concentrazione: no >",83,157,300,44,1884,defence_temporary_draft.concentration||state.magic.concentration);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        small_label(state.magic.concentration?state.magic.powers[state.magic.concentration-1].name:"Nessuna magia concentrata",83,215,300,PD_GOLD);
        small_label("Alla fine: disattiva e scollega",83,255,300,PD_TEXT);small_label("Il profilo base resta invariato",83,281,300,PD_TEXT);
        small_label("Nessuna riattivazione automatica",83,311,300,PD_MUTED);small_label("Non deduce condizioni dal testo",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,1885,true);button("Anteprima",239,359,144,48,1887,true);
    } else if(page==DEFENCE_CONFIRM){
        bool valid=preview_defence(&scratch);bool conditional=false;for(unsigned i=0;i<3;i++){conditional|=defence_conditions[i];for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++)conditional|=defence_temporary_drafts[slot].conditions[i];}snprintf(text,sizeof text,"%s%s",pd_damage_name(defence_index),conditional?" | cond.":"");label(text,83,111,204,false);lv_obj_t *rule=button("Regole",299,99,84,44,2010,true);lv_obj_set_style_text_font(rule,&lv_font_montserrat_16,0);
        if(valid){
            unsigned old=potential_defences(&state,defence_index),updated=potential_defences(&scratch,defence_index);const char *names[]={"Resistenza","Vulnerabilita","Immunita"};
            for(unsigned i=0;i<3;i++){
                snprintf(text,sizeof text,"%s: %s -> %s",names[i],old&(1u<<i)?"si":"no",updated&(1u<<i)?"si":"no");label(text,83,151+i*52,300,false);
                snprintf(text,sizeof text,"Base %s -> %s | #%u %s -> %s",pd_defence_base_flags(&state,defence_index)&(1u<<i)?"si":"no",defence_flags&(1u<<i)?"si":"no",defence_slot+1,old_defence()->flags&(1u<<i)?"si":"no",defence_temporary_draft.flags&(1u<<i)?"si":"no");small_label(text,83,177+i*52,300,PD_MUTED);
            }
            snprintf(text,sizeof text,"Base verificata: %s -> %s",state.defences.configured&(1u<<defence_index)?"si":"no",defence_configured?"si":"no");small_label(text,83,311,300,defence_configured?PD_GOLD:PD_DANGER);
            if(defence_temporary_draft.timed)snprintf(text,sizeof text,"#%u: %s %u%s | conc. %s",defence_slot+1,defence_temporary_draft.active?"attiva":"pausa",duration_left(defence_temporary_draft.tick,defence_temporary_draft.rounds,defence_temporary_draft.minutes),duration_owned_mark(defence_temporary_draft.tick,defence_temporary_draft.owner),defence_temporary_draft.concentration?"si":"no");else snprintf(text,sizeof text,"#%u: %s | conc. %s",defence_slot+1,defence_temporary_draft.active?"attiva":"inattiva",defence_temporary_draft.concentration?"si":"no");small_label(text,83,333,300,PD_MUTED);
        }else{
            label("Difesa da verificare",83,203,300,false);small_label("Attiva: scegli almeno una difesa",83,251,300,PD_DANGER);small_label("Timer: serve durata positiva",83,281,300,PD_DANGER);small_label("Vincolo: magia concentrata attuale",83,311,300,PD_MUTED);
        }
        button("Indietro",83,359,144,48,1816,true);button("Conferma",239,359,144,48,1817,valid);
    } else if(page==DEFENCE_RULE){
        label(defence_rule_temporary?"Regole dell'extra":"Regole della base",83,111,300,true);unsigned flags=defence_rule_temporary?defence_temporary_draft.flags:defence_flags;const uint8_t *rules=defence_rule_temporary?defence_temporary_draft.conditions:defence_conditions;
        const char *names[]={"Resist.","Vulner.","Immune"};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,157,92,44,2006+i,flags&(1u<<i));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        label(defence_property_names[defence_rule_property],83,215,300,true);unsigned selected=rules[defence_rule_property];snprintf(text,sizeof text,"Condizione: %s >",rule_names[selected]);lv_obj_t *b=button(text,83,257,300,44,2004,flags&(1u<<defence_rule_property));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        small_label(!(flags&(1u<<defence_rule_property))?"Difesa non selezionata":selected>=3?"Solo attacchi non magici":rule_descriptions[selected],83,313,300,PD_GOLD);
        small_label(!(flags&(1u<<defence_rule_property))?"Torna e abilita la proprieta":selected==4?"Con arma non argentata":selected==5?"Con arma non in adamantio":selected==6?"Non argentati e non in adamantio":"Nessuna origine dedotta dal nome",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,2002,true);button("Anteprima",239,359,144,48,2005,true);
    } else if(page==DEFENCE_RULE_REVIEW){
        bool valid=preview_defence(&scratch);if(rule_review_temporary)snprintf(text,sizeof text,"Regole extra %u/%u",defence_slot+1,PD_DEFENCE_SLOTS);else strcpy(text,"Regole della base");label(text,83,111,300,true);button("Base",83,157,144,44,2011,true);button("Extra",239,157,144,44,2012,true);
        snprintf(text,sizeof text,"%s >",defence_property_names[rule_review_property]);button(text,83,211,300,44,2013,true);
        unsigned old_flags=rule_review_temporary?old_defence()->flags:pd_defence_base_flags(&state,defence_index),new_flags=rule_review_temporary?defence_temporary_draft.flags:defence_flags;
        unsigned old_rule=rule_review_temporary?old_defence()->conditions[rule_review_property]:state.defences.conditions[defence_index][rule_review_property],new_rule=rule_review_temporary?defence_temporary_draft.conditions[rule_review_property]:defence_conditions[rule_review_property];
        snprintf(text,sizeof text,"Prima: %s",old_flags&(1u<<rule_review_property)?rule_descriptions[old_rule]:"assente");lv_obj_t *a=small_label(text,83,263,300,PD_MUTED);lv_label_set_long_mode(a,LV_LABEL_LONG_WRAP);lv_obj_set_height(a,38);
        snprintf(text,sizeof text,"Dopo: %s",new_flags&(1u<<rule_review_property)?rule_descriptions[new_rule]:"assente");a=small_label(text,83,307,300,PD_GOLD);lv_label_set_long_mode(a,LV_LABEL_LONG_WRAP);lv_obj_set_height(a,38);
        button("Indietro",83,359,144,48,2014,true);button("Conferma",239,359,144,48,1817,valid);
    } else if(page==DEFENCE_GRANTS){
        label("Extra indipendenti",83,103,300,true);small_label(pd_damage_name(defence_index),83,135,300,PD_GOLD);
        for(unsigned slot=0;slot<PD_DEFENCE_SLOTS;slot++){
            const pd_defence_temporary *d=&defence_temporary_drafts[slot];snprintf(text,sizeof text,"Extra %u | %s >",slot+1,d->active?"attivo":"inattivo");button(text,83,159+slot*56,300,44,2031+slot,true);
        }
        small_label("Bozze conservate fino alla conferma",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,2034,true);button("Confronta",239,359,144,48,2035,true);
    } else if(page==DEFENCE_GRANT_REVIEW){
        bool valid=preview_defence(&scratch);label("Confronta extra",83,111,204,false);lv_obj_t *b=button("Regole",299,99,84,44,2010,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        snprintf(text,sizeof text,"%s #%u/%u >",pd_damage_name(defence_index),defence_slot+1,PD_DEFENCE_SLOTS);button(text,83,157,300,44,2036,true);
        const pd_defence_temporary *a=old_defence(),*d=&defence_temporary_draft;
        snprintf(text,sizeof text,"Prima: %s%s",a->flags?defence_caption(true,a->flags):"assente",(a->conditions[0]||a->conditions[1]||a->conditions[2])?" | cond.":"");small_label(text,83,211,300,PD_MUTED);
        snprintf(text,sizeof text,"Dopo: %s%s",d->flags?defence_caption(true,d->flags):"assente",(d->conditions[0]||d->conditions[1]||d->conditions[2])?" | cond.":"");small_label(text,83,241,300,PD_GOLD);
        snprintf(text,sizeof text,"Attivo: %s -> %s",a->active?"si":"no",d->active?"si":"no");small_label(text,83,273,300,PD_TEXT);
        char old[24],updated[24];if(a->timed)snprintf(old,sizeof old,"%u%s",duration_left(a->tick,a->rounds,a->minutes),duration_owned_mark(a->tick,a->owner));else strcpy(old,"manuale");if(d->timed)snprintf(updated,sizeof updated,"%u%s",duration_left(d->tick,d->rounds,d->minutes),duration_owned_mark(d->tick,d->owner));else strcpy(updated,"manuale");snprintf(text,sizeof text,"Durata: %s -> %s",old,updated);small_label(text,83,303,300,PD_TEXT);
        snprintf(text,sizeof text,"Concentrazione: %s -> %s",a->concentration?"si":"no",d->concentration?"si":"no");small_label(text,83,333,300,PD_MUTED);
        button("Indietro",83,359,144,48,2035,true);button("Conferma",239,359,144,48,1817,valid);
    } else if(page==DAMAGE_REDUCTION){
        label("Riduzioni",83,111,204,false);lv_obj_t *profiles=button("Profili",299,99,84,44,2060,reduction_return!=DAMAGE_PART_EDIT);lv_obj_set_style_text_font(profiles,&lv_font_montserrat_16,0);bool after=reduction_target==PD_DAMAGE_COMPONENTS;
        if(after)strcpy(text,"Dopo: intero colpo >");else{pd_damage_event event=current_damage_event();unsigned type=reduction_return==DAMAGE_PART_EDIT?damage_part_draft.type:event.components[reduction_target].type;snprintf(text,sizeof text,"Prima: %u %s >",reduction_target+1,pd_damage_name(type));}lv_obj_t *target=button(text,83,157,300,44,2042,reduction_return!=DAMAGE_PART_EDIT);lv_obj_set_style_text_font(target,&lv_font_montserrat_16,0);
        snprintf(text,sizeof text,"Sottrai %u",reduction_drafts[reduction_target]);label(text,83,215,300,true);
        const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(names[i],83+i*78,251,66,44,2043+i,i<2?reduction_drafts[reduction_target]>0:reduction_drafts[reduction_target]<9999);
        small_label(after?"Una volta, dopo tutte le difese":"Prima delle difese della parte",83,307,300,PD_GOLD);if(reduction_profile_draft&&reduction_return!=DAMAGE_PART_EDIT)snprintf(text,sizeof text,"Profilo %u pronto | Usa valori",reduction_profile_draft);else strcpy(text,"Eccedenze ignorate; minimo zero");small_label(text,83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,2047,true);button("Usa valori",239,359,144,48,2048,true);
    } else if(page==DAMAGE_MATH){
        const pd_damage_event_summary *r=&damage_details;label("Conto del colpo",83,111,r->profile?204:300,false);if(r->profile){lv_obj_t *b=button("Profilo",299,99,84,44,2090,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        snprintf(text,sizeof text,"Danno iniziale: %u",r->raw);label(text,83,159,300,false);
        snprintf(text,sizeof text,"Prima: -%u (scelti %u)",r->reduction_before,r->requested_before);small_label(text,83,205,300,PD_GOLD);
        if(r->total.configured){snprintf(text,sizeof text,"Dopo difese: %u",r->mitigated);label(text,83,243,300,false);snprintf(text,sizeof text,"Dopo: -%u (scelti %u)",r->reduction_after,r->requested_after);small_label(text,83,285,300,PD_GOLD);snprintf(text,sizeof text,"Danno effettivo: %u",r->total.adjusted);label(text,83,321,300,false);}
        else{label(profile_problem(r)?profile_problem(r):"Difese/fonte da verificare",83,249,300,false);small_label("Nessun totale parziale applicabile",83,303,300,PD_DANGER);}
        button("Indietro",83,359,300,48,2051,true);
    } else if(page==PROFILE_LIST){
        label("Profili di riduzione",83,111,300,true);
        for(unsigned i=0;i<PD_REDUCTION_PROFILES;i++){const pd_reduction_profile *p=&state.reductions[i];snprintf(text,sizeof text,"Profilo %u | %s >",i+1,p->active?"attivo":"inattivo");button(text,83,159+i*56,300,44,2061+i,true);}
        if(reduction_profile_draft)snprintf(text,sizeof text,"Nella bozza: profilo %u",reduction_profile_draft);else strcpy(text,"Nella bozza: nessun profilo");small_label(text,83,335,300,PD_GOLD);
        button("Indietro",83,359,144,48,2064,true);button("Nessuno",239,359,144,48,2065,true);
    } else if(page==PROFILE_VIEW){
        const pd_reduction_profile *p=&state.reductions[profile_index];snprintf(text,sizeof text,"Profilo %u",profile_index+1);label(text,83,111,204,false);lv_obj_t *cost=button("Costi",299,99,84,44,2100,true);lv_obj_set_style_text_font(cost,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"Budget %u per colpo",p->value);label(text,83,159,300,false);
        snprintf(text,sizeof text,"Tipi: %s",profile_types(p->types));small_label(text,83,205,300,PD_GOLD);snprintf(text,sizeof text,"%s difese | %s",p->stage?"Dopo":"Prima",p->active?"attivo":"inattivo");small_label(text,83,239,300,PD_TEXT);
        lv_obj_t *r=small_label(rule_descriptions[p->rule],83,273,300,PD_MUTED);lv_label_set_long_mode(r,LV_LABEL_LONG_WRAP);lv_obj_set_height(r,42);
        if(p->timed)snprintf(text,sizeof text,"%u %s | conc. %s",duration_left(p->tick,p->rounds,p->minutes),duration_owned(p->timed,p->tick,p->owner),p->concentration?"si":"no");else snprintf(text,sizeof text,"Manuale | concentrazione %s",p->concentration?"si":"no");small_label(text,83,325,300,PD_MUTED);
        const char *names[]={"Indietro","Modifica","Usa"};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,2066+i,i!=2||p->active);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==PROFILE_EDIT){
        snprintf(text,sizeof text,"Configura profilo %u",profile_index+1);label(text,83,111,300,true);
        snprintf(text,sizeof text,"Quantita: %u >",profile_draft.value);button(text,83,157,300,44,2069,true);
        snprintf(text,sizeof text,"Tipi: %s >",profile_types(profile_draft.types));lv_obj_t *b=button(text,83,205,300,44,2070,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        button(profile_draft.stage?"Fase: dopo >":"Fase: prima >",83,253,300,44,2071,true);button(profile_draft.active?"Attivo: si >":"Attivo: no >",83,301,300,44,2072,true);
        const char *names[]={"Indietro","Dettagli","Anteprima"};for(unsigned i=0;i<3;i++){lv_obj_t *a=button(names[i],83+i*104,359,92,48,2073+i,true);lv_obj_set_style_text_font(a,&lv_font_montserrat_16,0);}
    } else if(page==PROFILE_DETAILS){
        label("Regola e durata",83,111,204,false);lv_obj_t *cost=button("Costi",299,99,84,44,2100,true);lv_obj_set_style_text_font(cost,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"Regola: %s >",rule_names[profile_draft.rule]);lv_obj_t *b=button(text,83,157,300,44,2076,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        snprintf(text,sizeof text,"Durata: %s >",duration_owned(profile_draft.timed,profile_draft.tick,profile_draft.owner));lv_obj_t *duration=button(text,83,205,300,44,2077,true);lv_obj_set_style_text_font(duration,&lv_font_montserrat_16,0);
        b=button(profile_draft.concentration?"Concentrazione: si >":"Concentrazione: no >",83,253,300,44,2078,profile_draft.concentration||state.magic.concentration);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        snprintf(text,sizeof text,profile_draft.tick>=PD_TICK_MINUTES?"Minuti residui: %u >":profile_draft.tick?"Passaggi residui: %u >":"Round residui: %u >",duration_left(profile_draft.tick,profile_draft.rounds,profile_draft.minutes));button(text,83,301,300,44,2079,profile_draft.timed);
        button("Indietro",83,359,144,48,2080,true);button("Anteprima",239,359,144,48,2075,true);
    } else if(page==PROFILE_NUMBER){
        label(profile_number_cost?"Costo del profilo":profile_number_rounds?(profile_draft.tick>=PD_TICK_MINUTES?"Tempo del profilo":profile_draft.tick?"Passaggi del profilo":"Round del profilo"):"Quantita del profilo",83,111,300,true);
        unsigned n=profile_number_cost?pd_profile_cost(&profile_draft,profile_cost_row).amount:profile_number_rounds?duration_left(profile_draft.tick,profile_draft.rounds,profile_draft.minutes):profile_draft.value;
        if(profile_number_rounds&&profile_draft.tick==PD_TICK_HOURS)time_text(text,sizeof text,n);else snprintf(text,sizeof text,"%u",n);label(text,83,199,300,true);
        const char *names[]={"-10","-1","+1","+10"};unsigned maximum=profile_number_cost?pd_reduction_cost_max(&state,pd_profile_cost(&profile_draft,profile_cost_row).kind,pd_profile_cost(&profile_draft,profile_cost_row).index):profile_number_rounds?(profile_draft.tick==PD_TICK_HOURS?5940:99):9999;
        for(unsigned i=0;i<4;i++)button(profile_number_rounds&&profile_draft.tick==PD_TICK_HOURS?(const char *const[]){"-1h","-1m","+1m","+1h"}[i]:names[i],83+i*78,251,66,44,2081+i,i<2?n>0:n<maximum);
        if(profile_number_rounds&&profile_draft.timed&&turn_clock(profile_draft.tick)){snprintf(text,sizeof text,"Turno: %s >",turn_subject(profile_draft.owner));button(text,83,305,300,44,3212,true);}
        else {small_label(profile_number_cost?"Costo positivo richiesto":profile_number_rounds?"Attivo: durata positiva richiesta":"Un budget condiviso per colpo",83,319,300,PD_GOLD);}button("Indietro",83,359,300,48,2085,true);
    } else if(page==PROFILE_CONFIRM){
        bool valid=preview_profile(&scratch);label(valid?"Salvare il profilo?":"Profilo da verificare",83,111,300,true);snprintf(text,sizeof text,"%s >",(const char *const[]){"Dati","Regola","Durata","Costi"}[profile_confirm_tab]);button(text,83,157,profile_confirm_tab==3?144:300,44,2086,true);if(profile_confirm_tab==3){snprintf(text,sizeof text,"Spesa %u/3 >",profile_cost_row+1);lv_obj_t *b=button(text,239,157,144,44,2109,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}const pd_reduction_profile *a=&state.reductions[profile_index],*p=&profile_draft;
        if(!profile_confirm_tab){snprintf(text,sizeof text,"Quantita: %u -> %u",a->value,p->value);small_label(text,83,211,300,PD_TEXT);snprintf(text,sizeof text,"Tipi: %s -> %s",profile_types(a->types),profile_types(p->types));lv_obj_t *types=small_label(text,83,243,300,PD_GOLD);lv_label_set_long_mode(types,LV_LABEL_LONG_WRAP);lv_obj_set_height(types,38);snprintf(text,sizeof text,"Fase: %s -> %s",a->stage?"dopo":"prima",p->stage?"dopo":"prima");small_label(text,83,287,300,PD_TEXT);snprintf(text,sizeof text,"Attivo: %s -> %s",a->active?"si":"no",p->active?"si":"no");small_label(text,83,325,300,PD_MUTED);}
        else if(profile_confirm_tab==1){snprintf(text,sizeof text,"Prima: %s",rule_descriptions[a->rule]);lv_obj_t *r=small_label(text,83,211,300,PD_MUTED);lv_label_set_long_mode(r,LV_LABEL_LONG_WRAP);lv_obj_set_height(r,50);snprintf(text,sizeof text,"Dopo: %s",rule_descriptions[p->rule]);r=small_label(text,83,287,300,PD_GOLD);lv_label_set_long_mode(r,LV_LABEL_LONG_WRAP);lv_obj_set_height(r,50);}
        else if(profile_confirm_tab==2){snprintf(text,sizeof text,"Durata: %s -> %s",duration_owned(a->timed,a->tick,a->owner),duration_owned(p->timed,p->tick,p->owner));small_label(text,83,211,300,PD_TEXT);char before[24],after[24];if(a->tick==PD_TICK_HOURS)time_text(before,sizeof before,a->minutes);else snprintf(before,sizeof before,"%u%s",duration_left(a->tick,a->rounds,a->minutes),a->tick==PD_TICK_MINUTES?"min":"");if(p->tick==PD_TICK_HOURS)time_text(after,sizeof after,p->minutes);else snprintf(after,sizeof after,"%u%s",duration_left(p->tick,p->rounds,p->minutes),p->tick==PD_TICK_MINUTES?"min":"");snprintf(text,sizeof text,"Residui: %s -> %s",before,after);small_label(text,83,249,300,PD_TEXT);snprintf(text,sizeof text,"Concentrazione: %s -> %s",a->concentration?"si":"no",p->concentration?"si":"no");small_label(text,83,287,300,PD_GOLD);small_label(valid?(p->timed?"Scade: disattiva il profilo":"Fine concentrazione: scollega"):"Verifica quantita, tipi e durata",83,325,300,PD_MUTED);}
        else{snprintf(text,sizeof text,"Prima | costo %u | reazione %s",pd_profile_cost(a,profile_cost_row).amount,a->reaction?"si":"no");small_label(text,83,205,300,PD_MUTED);lv_obj_t *r=small_label(cost_name(a),83,225,300,PD_MUTED);lv_label_set_long_mode(r,LV_LABEL_LONG_WRAP);lv_obj_set_height(r,54);snprintf(text,sizeof text,"Dopo | costo %u | reazione %s",pd_profile_cost(p,profile_cost_row).amount,p->reaction?"si":"no");small_label(text,83,281,300,PD_GOLD);r=small_label(cost_name(p),83,301,300,PD_GOLD);lv_label_set_long_mode(r,LV_LABEL_LONG_WRAP);lv_obj_set_height(r,54);}
        button("Indietro",83,359,144,48,2087,true);button("Conferma",239,359,144,48,2088,valid);
    } else if(page==PROFILE_ALLOCATE){
        const pd_reduction_profile *p=&state.reductions[profile_index];snprintf(text,sizeof text,"Usa profilo %u",profile_index+1);label(text,83,111,300,true);pd_damage_event e=current_damage_event();if(p->stage)strcpy(text,"Dopo: intero colpo");else snprintf(text,sizeof text,"%u. %s >",allocation_row+1,pd_damage_name(e.components[allocation_row].type));button(text,83,157,300,44,2089,!p->stage);
        snprintf(text,sizeof text,"%s %u",p->stage?"Sottrai":"Assegna",p->stage?p->value:allocation_draft[allocation_row]);label(text,83,215,300,true);unsigned sum=allocation_sum();
        const char *names[]={"-10","-1","+1","+10"};for(unsigned i=0;i<4;i++)button(names[i],83+i*78,251,66,44,2081+i,!p->stage&&(i<2?allocation_draft[allocation_row]>0:sum<p->value));
        snprintf(text,sizeof text,"Budget %u | assegnati %u",p->value,p->stage?p->value:sum);small_label(text,83,307,300,PD_GOLD);small_label("Tipi e fonte verificati nell'anteprima",83,335,300,PD_MUTED);
        button("Indietro",83,359,144,48,2085,true);button("Usa profilo",239,359,144,48,2088,p->active&&(p->stage||(sum&&sum<=p->value)));
    } else if(page==PROFILE_IMPACT){
        const pd_damage_event_summary *r=&damage_details;const pd_reduction_profile *p=&r->profile_snapshot;snprintf(text,sizeof text,"Profilo %u",r->profile);label(text,83,111,204,false);lv_obj_t *cost=button("Costi",299,99,84,44,2105,true);lv_obj_set_style_text_font(cost,&lv_font_montserrat_16,0);
        if(p->stage)strcpy(text,"Dopo: intero colpo");else snprintf(text,sizeof text,"Parte %u/%u >",profile_impact_row+1,r->profile_count);button(text,83,157,300,44,2089,!p->stage);
        snprintf(text,sizeof text,"Scelti %u | budget %u",p->stage?r->profile_requested:r->profile_rows[profile_impact_row],p->value);small_label(text,83,215,300,PD_GOLD);
        if(r->total.configured)snprintf(text,sizeof text,"Sottratti: %u",p->stage?r->profile_applied:r->profile_used[profile_impact_row]);else strcpy(text,"Profilo/fonte da verificare");label(text,83,249,300,false);
        snprintf(text,sizeof text,"Tipi: %s",profile_types(p->types));small_label(text,83,287,300,PD_TEXT);snprintf(text,sizeof text,"Regola: %s",rule_names[p->rule]);small_label(text,83,325,300,PD_MUTED);button("Indietro",83,359,300,48,2091,true);
    } else if(page==PROFILE_COST){
        const pd_reduction_profile *p=cost_profile();pd_cost c=pd_profile_cost(p,profile_cost_row);snprintf(text,sizeof text,"Spesa %u/3 >",profile_cost_row+1);button(text,83,99,300,44,2107,true);
        if(!c.kind)strcpy(text,"Fonte: nessuna >");else snprintf(text,sizeof text,"Fonte: %s %u >",(const char *const[]){"nessuna","contatore","usi voce","slot","patto","cariche"}[c.kind],c.index+1);lv_obj_t *b=button(text,83,157,300,44,2101,profile_cost_editing);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        if(profile_cost_editing)snprintf(text,sizeof text,"Costo: %u >",c.amount);else snprintf(text,sizeof text,"Costo %u | saldo %u",c.amount,cost_current(p));b=button(text,83,205,300,44,2102,profile_cost_editing&&c.kind);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
        button(p->reaction?"Usa reazione: si >":"Usa reazione: no >",83,253,300,44,2103,profile_cost_editing);
        lv_obj_t *name=small_label(cost_name(p),83,299,300,PD_GOLD);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,58);button("Indietro",83,359,300,48,2104,true);
    } else if(page==PROFILE_COST_IMPACT){
        const pd_damage_event_summary *r=&damage_details;const pd_reduction_profile *p=&r->profile_snapshot;pd_cost c=pd_profile_cost(p,profile_cost_row);const pd_cost_receipt *receipt=&r->costs[profile_cost_row];
        snprintf(text,sizeof text,"Spesa %u/3 >",profile_cost_row+1);button(text,83,99,300,44,2108,true);lv_obj_t *name=small_label(c.kind?receipt->name:"Nessuna risorsa",83,157,300,PD_GOLD);lv_label_set_long_mode(name,LV_LABEL_LONG_WRAP);lv_obj_set_height(name,58);
        if(r->total.configured)snprintf(text,sizeof text,"Risorsa: %u -> %u",receipt->before,receipt->after);else strcpy(text,receipt->unavailable?"Oggetto non disponibile":receipt->missing?"Risorsa insufficiente":"Spese non applicabili");label(text,83,221,300,false);if(c.kind)snprintf(text,sizeof text,"Costo %u | %s %u",c.amount,(const char *const[]){"nessuna","contatore","usi voce","slot","patto","cariche"}[c.kind],c.index+1);else strcpy(text,"Nessun costo");small_label(text,83,259,300,PD_TEXT);
        if(!p->reaction)strcpy(text,"Reazione: invariata");else if(!r->total.configured)strcpy(text,r->reaction_missing?"Reazione gia usata":"Reazione non applicabile");else snprintf(text,sizeof text,"Reazione: %s -> %s",r->reaction_before?"usata":"pronta",r->reaction_after?"usata":"pronta");small_label(text,83,293,300,PD_GOLD);
        small_label("Spese insieme anche a danno zero",83,325,300,PD_MUTED);button("Indietro",83,359,300,48,2106,true);
    } else if(page==REACTION_HOME){
        label("Turni e reazione",83,111,300,true);label(state.reaction_spent?"Gia utilizzata":"Disponibile",83,151,300,false);small_label("Round e turni sono separati",83,185,300,PD_GOLD);
        button("Usa reazione",83,215,300,44,2119,!state.reaction_spent);button("Inizio turno",83,263,300,44,2120,true);button("Fine turno",83,311,300,44,2123,true);button("Indietro",83,363,144,44,2118,true);lv_obj_t *other=button("Altri turni",239,363,144,44,3200,true);lv_obj_set_style_text_font(other,&lv_font_montserrat_16,0);
    } else if(page==REACTION_CONFIRM){
        if(reaction_draft){label("Usare la reazione?",83,111,300,true);label("pronta -> usata",83,211,300,false);small_label("Annota una reazione esterna",83,267,300,PD_GOLD);small_label("Non avanza durate o risorse",83,305,300,PD_MUTED);button("Indietro",83,359,144,48,2121,true);button("Conferma",239,359,144,48,2122,true);}
        else{
            pd_round_summary r={0};bool valid=preview_turn(&scratch,&r);if(turn_owner)snprintf(text,sizeof text,"%s turno %s?",turn_end?"Termina":"Inizia",turn_subject(turn_owner));else snprintf(text,sizeof text,turn_end?"Termina il tuo turno?":"Inizia il tuo turno?");label(text,83,111,300,false);
            if(valid){
                snprintf(text,sizeof text,"Avanzano %u | scadono %u",bit_count(r.effects_advanced)+bit_count(r.defences_advanced),bit_count(r.effects_expired)+bit_count(r.defences_expired));small_label(text,83,151,300,PD_GOLD);
                snprintf(text,sizeof text,"Reazione: %s -> %s",state.reaction_spent?"usata":"pronta",scratch.reaction_spent?"usata":"pronta");small_label(text,83,183,300,PD_TEXT);
                snprintf(text,sizeof text,"CA %u -> %u",pd_armor(&state),pd_armor(&scratch));label(text,83,215,300,false);
                snprintf(text,sizeof text,"PF massimi %u -> %u",pd_max_hp(&state),pd_max_hp(&scratch));label(text,83,247,300,false);
                snprintf(text,sizeof text,"PF correnti %u -> %u",state.hp,scratch.hp);label(text,83,279,300,false);
                small_label(state.hp>scratch.hp?"PF oltre il nuovo massimo: persi":"Nessuna cura o risorsa recuperata",83,325,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);
            }else{label("Scadenze fuori limite",83,211,300,false);small_label("Durate e reazione invariate",83,259,300,PD_DANGER);small_label("Correggi gli effetti locali",83,303,300,PD_MUTED);}
            const char *names[]={"Indietro","Scadenze","Conferma"};for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,(const unsigned[]){2121,2124,2122}[i],i==0||(valid&&(i==2||r.effects_advanced||r.defences_advanced)));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        }
    } else if(page==TIME_HOME){
        label("Tempo trascorso",83,111,300,true);small_label("Tempo di gioco dichiarato",83,165,300,PD_GOLD);time_text(text,sizeof text,time_minutes);label(text,83,215,300,true);
        for(unsigned i=0;i<4;i++)button((const char *const[]){"-1h","-1m","+1m","+1h"}[i],83+i*78,263,66,44,2142+i,i<2?time_minutes>1:time_minutes<5940);
        small_label("Round e turni invariati",83,319,300,PD_MUTED);button("Indietro",83,359,144,48,2141,true);button("Anteprima",239,359,144,48,2146,true);
    } else if(page==TIME_CONFIRM){
        pd_round_summary r={0};bool valid=preview_time(&scratch,&r);label("Avanzare il tempo?",83,111,300,false);time_text(text,sizeof text,time_minutes);small_label(text,83,151,300,PD_GOLD);
        if(valid){
            snprintf(text,sizeof text,"Avanzano %u | scadono %u",bit_count(r.effects_advanced)+bit_count(r.defences_advanced),bit_count(r.effects_expired)+bit_count(r.defences_expired));small_label(text,83,183,300,PD_TEXT);
            snprintf(text,sizeof text,"CA %u -> %u",pd_armor(&state),pd_armor(&scratch));label(text,83,215,300,false);snprintf(text,sizeof text,"PF massimi %u -> %u",pd_max_hp(&state),pd_max_hp(&scratch));label(text,83,247,300,false);snprintf(text,sizeof text,"PF correnti %u -> %u",state.hp,scratch.hp);label(text,83,279,300,false);
            small_label(state.hp>scratch.hp?"Scadenze: PF correnti ridotti":"Reazione e risorse invariate",83,325,300,state.hp>scratch.hp?PD_DANGER:PD_MUTED);
        }else{label("Scadenze fuori limite",83,211,300,false);small_label("Nessun timer viene avanzato",83,259,300,PD_DANGER);small_label("Controlla gli effetti numerici",83,303,300,PD_MUTED);}
        for(unsigned i=0;i<3;i++){lv_obj_t *b=button((const char *const[]){"Indietro","Scadenze","Conferma"}[i],83+i*104,359,92,48,(const unsigned[]){2147,2149,2148}[i],i==0||(valid&&(i==2||r.effects_advanced||r.defences_advanced)));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
    } else if(page==TURN_DETAILS||page==TIME_DETAILS){
        bool time=page==TIME_DETAILS;unsigned *detail=time?&time_detail_page:&turn_detail_page;
        pd_round_summary r={0};if(!(time?preview_time(&scratch,&r):preview_turn(&scratch,&r))){label("Scadenze fuori limite",83,211,300,false);button("Riepilogo",83,359,300,48,time?2150:2125,true);return;}
        unsigned indices[PD_EFFECTS+PD_TIMED_DEFENCES],count=0;for(unsigned i=0;i<PD_EFFECTS;i++)if(r.effects_advanced&(1u<<i))indices[count++]=i;for(unsigned i=0;i<PD_TIMED_DEFENCES;i++)if(r.defences_advanced&(UINT64_C(1)<<i))indices[count++]=PD_EFFECTS+i;
        unsigned pages=count?(count+2)/3:1;if(*detail>=pages)*detail=pages-1;if(time)snprintf(text,sizeof text,"Scadenze del tempo");else if(turn_owner)snprintf(text,sizeof text,"%s %s: scadenze",turn_end?"Fine":"Inizio",turn_subject(turn_owner));else snprintf(text,sizeof text,turn_end?"Scadenze a fine turno":"Scadenze a inizio turno");label(text,83,99,300,false);snprintf(text,sizeof text,time?"Minuti | pagina %u/%u":"Passaggi | pagina %u/%u",*detail+1,pages);small_label(text,83,133,300,PD_GOLD);
        for(unsigned row=0;row<3;row++){
            unsigned at=*detail*3+row;if(at>=count)break;unsigned bit=indices[at],a,b;
            if(bit<PD_EFFECTS){a=time?state.effects[bit].minutes:state.effects[bit].rounds;b=time?scratch.effects[bit].minutes:scratch.effects[bit].rounds;snprintf(text,sizeof text,"%s #%u: %u -> %u",state.effects[bit].target==PD_EFFECT_ARMOR?"CA":"PF max",bit+1,a,b);}
            else if((bit-=PD_EFFECTS)<PD_DEFENCE_GRANTS){unsigned type=bit%PD_DAMAGE_TYPES,slot=bit/PD_DAMAGE_TYPES;a=time?pd_defence_grant(&state,type,slot)->minutes:pd_defence_grant(&state,type,slot)->rounds;b=time?pd_defence_grant(&scratch,type,slot)->minutes:pd_defence_grant(&scratch,type,slot)->rounds;snprintf(text,sizeof text,"%s #%u: %u -> %u",pd_damage_name(type),slot+1,a,b);}
            else{unsigned i=bit-PD_DEFENCE_GRANTS;a=time?state.reductions[i].minutes:state.reductions[i].rounds;b=time?scratch.reductions[i].minutes:scratch.reductions[i].rounds;snprintf(text,sizeof text,"Riduz. %u: %u -> %u",i+1,a,b);}
            lv_obj_t *row_label=label(text,83,159+row*56,300,false);if(time)lv_obj_set_style_text_font(row_label,&lv_font_montserrat_16,0);small_label(b?"Resta attivo":"Scade e si disattiva",83,187+row*56,300,b?PD_MUTED:PD_MINT);
        }
        button("<",83,359,60,48,time?2151:2126,*detail>0);button("Riepilogo",155,359,156,48,time?2150:2125,true);button(">",323,359,60,48,time?2152:2127,*detail+1<pages);
    } else if(page==DAMAGE_CONTEXT){
        label("Fonte del danno",83,103,300,true);snprintf(text,sizeof text,"Parte %u | ? = da verificare",damage_part_index+1);small_label(text,83,135,300,PD_GOLD);
        const char *names[]={"Magico","Attacco","Argentato","Adamantio"};for(unsigned i=0;i<4;i++){unsigned bit=1u<<i;snprintf(text,sizeof text,"%s: %s",names[i],!(source_context_draft.known&bit)?"?":source_context_draft.values&bit?"si":"no");button(text,83,159+i*48,300,44,2024+i,true);}
        button("Indietro",83,359,144,48,2022,true);button("Usa fonte",239,359,144,48,2023,true);
    } else if(page==DAMAGE_CONFIRM){
        pd_damage_event event=current_damage_event();pd_damage_event_summary report;if(!pd_damage_event_preview(&state,&event,&report))return;pd_damage_summary r=report.total;
        if(damage_extra_count||has_reductions(&report)){label("Conferma colpo",83,111,228,false);lv_obj_t *b=button("Tipi",323,99,60,44,1930,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);if(has_reductions(&report))snprintf(text,sizeof text,"Prima -%u | dopo -%u | vedi Tipi",report.reduction_before,report.reduction_after);else snprintf(text,sizeof text,"%u parti | %u gruppi",event.count,report.count);small_label(text,83,149,300,PD_GOLD);}
        else{label("Confermare i danni?",83,103,300,true);snprintf(text,sizeof text,"%s | %s",pd_damage_name(damage_plan.type),defence_caption(r.configured,r.flags));small_label(text,83,137,300,PD_GOLD);}
        if(!r.configured&&profile_problem(&report)){label(profile_problem(&report),83,203,300,false);small_label(report.reaction_missing?"Apri Gestisci reazione":"Indietro > Riduci > Profili / Usi",83,259,300,PD_GOLD);if(report.reaction_missing)button("Gestisci reazione",83,303,300,44,2117,true);else small_label("Nessun danno applicato",83,303,300,PD_MUTED);}
        else if(!r.configured){label(report.missing?"Difesa da configurare":"Fonte da verificare",83,203,300,false);small_label(report.missing?"Il profilo non arriva da Beyond":"Dichiara magico, attacco e materiali",83,249,300,PD_GOLD);button(report.missing?"Configura difesa":"Dichiara fonte",83,303,300,44,report.missing?1807:2021,true);}
        else {
            snprintf(text,sizeof text,"Danni %u -> %u",report.raw,r.adjusted);label(text,83,171,300,false);
            snprintf(text,sizeof text,"Temporanei %u -> %u",state.temp_hp,r.temp_after);label(text,83,207,300,false);
            snprintf(text,sizeof text,"PF %u -> %u",state.hp,r.hp_after);label(text,83,239,300,false);
            if(r.massive||r.at_zero){small_label(r.massive?"Possibile morte istantanea: DM":"Gia a zero PF: verifica il colpo",83,273,300,PD_DANGER);button("Risolvi registro morte",83,303,300,44,3000,true);}
            else if(r.needs_concentration){
                pd_check_info info;pd_damage_summary summary;
                bool known=current_concentration_info(0,0,&info,&summary,false);
                if(known&&pd_concentration_effects(&state))snprintf(text,sizeof text,"CD %u | fallimento: %u effetti",r.concentration_dc,pd_concentration_effects(&state));else snprintf(text,sizeof text,known?"Concentrazione CD %u (con DM)":"CD %u | COS da verificare",r.concentration_dc);small_label(text,83,269,300,PD_GOLD);
                snprintf(text,sizeof text,"Esito: %s",(const char *const[]){"da risolvere","mantieni","termina"}[damage_plan.concentration]);button(text,83,299,300,44,1806,true);
            } else {small_label(r.ends_concentration?"A zero PF: fine concentr. ed effetti":!r.adjusted?"Nessun danno effettivo":"Una sola applicazione alla conferma",83,285,300,PD_MINT);small_label("Difese selezionate manualmente",83,317,300,PD_MUTED);}
        }
        if(r.needs_concentration){
            pd_check_info info;pd_damage_summary summary;
            bool ready=current_concentration_info(0,0,&info,&summary,false);
            const char *names[]={"Indietro","Tira COS","Conferma"};const unsigned actions[]={1804,1860,1805};
            for(unsigned i=0;i<3;i++){lv_obj_t *b=button(names[i],83+i*104,359,92,48,actions[i],i==0||(i==1?ready:r.allowed));lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        } else {button("Indietro",83,359,144,48,1804,true);button("Conferma",239,359,144,48,1805,r.allowed);}
    } else if(page==DAMAGE_RESULT){
        if(incoming_components>1||has_reductions(&incoming_event)){label("Colpo confermato",83,111,228,false);lv_obj_t *b=button("Tipi",323,99,60,44,1930,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);snprintf(text,sizeof text,"%u %s: %u -> %u",incoming_components,incoming_components==1?"parte":"parti",incoming_event.raw,incoming_result.adjusted);}
        else{label("Danni confermati",83,111,300,true);snprintf(text,sizeof text,"%s: %u -> %u",pd_damage_name(damage_plan.type),damage_plan.amount,incoming_result.adjusted);}small_label(text,83,161,300,PD_GOLD);
        if(incoming_death_used){
            snprintf(text,sizeof text,"Temporanei %u -> %u",damage_temp_before,incoming_death.impact.total.temp_after);label(text,83,195,300,false);
            snprintf(text,sizeof text,"PF %u -> %u",damage_hp_before,incoming_death.impact.total.hp_after);label(text,83,231,300,true);
            snprintf(text,sizeof text,"Fallite %u -> %u | riuscite %u -> %u",incoming_death.failures_before,incoming_death.failures_after,incoming_death.successes_before,incoming_death.successes_after);small_label(text,83,279,300,incoming_death.failures_after==3?PD_DANGER:PD_MINT);
            small_label(incoming_death.instant?"Morte: regola base confermata":incoming_death.stable_before?"Stabilizzazione terminata":"Esito del singolo colpo confermato",83,305,300,PD_GOLD);
            small_label("Eroe > Annulla ripristina il colpo",83,331,300,PD_MUTED);
            button("Stato",83,359,144,48,1500,true);button("Nuovo",239,359,144,48,1808,true);return;
        }
        snprintf(text,sizeof text,"Temporanei %u -> %u",damage_temp_before,state.temp_hp);label(text,83,211,300,false);
        snprintf(text,sizeof text,"PF %u -> %u",damage_hp_before,state.hp);label(text,83,251,300,true);
        small_label(incoming_result.ends_concentration?"Concentrazione terminata":incoming_result.needs_concentration?(combat_roll==6?"Concentrazione mantenuta dal tiro":"Concentrazione mantenuta con il DM"):"Concentrazione: non richiesta",83,299,300,PD_MINT);
        if(incoming_result.ended_effects)snprintf(text,sizeof text,"Effetti terminati: %u | PF max %u",incoming_result.ended_effects,incoming_result.max_hp_after);else strcpy(text,"Eroe > Annulla ripristina la modifica");small_label(text,83,329,300,PD_MUTED);button("Stato",83,359,144,48,1500,true);button("Nuovo",239,359,144,48,1808,true);
    } else if(page==CONCENTRATION_SETUP){
        pd_check_info info={0};pd_damage_summary summary={0};bool ready=current_concentration_info(concentration_mode,concentration_extra,&info,&summary,concentration_cap30);
        label("Concentrazione",83,103,300,true);
        snprintf(text,sizeof text,concentration_cap30?"CD max 30: %u >":"CD base: %u >",summary.concentration_dc);button(text,83,143,300,44,1866,!concentration_rolled);
        snprintf(text,sizeof text,"COS %+d | comp %+d | scheda %+d",info.base,info.trained,info.extra);small_label(text,83,193,300,PD_TEXT);
        if(concentration_rolled)snprintf(text,sizeof text,"Tiro fissato: %d",concentration_result.total);else snprintf(text,sizeof text,"Bonus totale %+d",info.total+concentration_extra);label(text,83,219,300,true);
        snprintf(text,sizeof text,"Modalita: %s >",concentration_mode>0?"Vantaggio":concentration_mode<0?"Svantaggio":"Base");button(text,83,257,300,44,1862,!concentration_rolled);
        button("-",83,307,48,44,1863,!concentration_rolled&&concentration_extra> -30);
        if(concentration_rolled)strcpy(text,"Riprova lo stesso tiro");else snprintf(text,sizeof text,"Extra locale %+d",concentration_extra);small_label(text,139,319,188,concentration_rolled?PD_GOLD:PD_TEXT);
        button("+",335,307,48,44,1864,!concentration_rolled&&concentration_extra<30);
        button("Indietro",83,359,144,48,1861,!concentration_rolled);
        lv_obj_t *confirm=button(concentration_rolled?"Riprova":"Tira e applica",239,359,144,48,1865,ready||concentration_rolled);lv_obj_set_style_text_font(confirm,&lv_font_montserrat_16,0);
    }
}

/* Route the new views through the same guarded actions/storage as every other
 * screen. No game mutation or random draw lives in the companion renderer. */
static void companion_event(companion_action selected,void *unused) {
    (void)unused;
    if(import_pending||confirmation_open()||display_screen.dimmed||dice_animation.active||
       (page!=HERO&&page!=HOME_MENU&&page!=HOME_VITALS))return;
    intptr_t action;
    switch(selected){
        case COMPANION_OPEN_MENU:action=2300;break;
        case COMPANION_OPEN_HOME:action=100;break;
        case COMPANION_OPEN_VITALS:action=2301;break;
        case COMPANION_ATTACK:{pd_weapon_info weapon;action=pd_weapon_stats(&state,state.equipped,&weapon)?81:80;break;}
        case COMPANION_DICE:action=102;break;
        case COMPANION_MAGIC:action=800;break;
        case COMPANION_PACK:action=80;break;
        case COMPANION_REST:action=900;break;
        case COMPANION_SHEET:action=101;break;
        case COMPANION_APPEARANCE:action=50;break;
        case COMPANION_RESOURCES:action=103;break;
        case COMPANION_MINUS:if(page!=HOME_VITALS)return;action=1;break;
        case COMPANION_PLUS:if(page!=HOME_VITALS)return;action=2;break;
        case COMPANION_UNDO:if(page!=HOME_VITALS)return;action=3;break;
        default:return;
    }
    act(action);
}
static void render_companion(void){
    const char *status=ui_storage_status();
    bool error=strstr(status,"Errore")!=NULL;
    uint32_t age=lv_tick_get()-feedback.started;
    if(error||display_screen.dimmed||age>=2000)feedback.active=false;
    if(page==HERO)avatar_prepare(&state.avatar,4);
    companion_view view={
        .page=page==HERO?COMPANION_HOME:page==HOME_MENU?COMPANION_MENU:COMPANION_VITALS,
        .name=state.name,.level=state.level,.hp=state.hp,.maximum=pd_max_hp(&state),
        .temporary=state.temp_hp,.armor=pd_armor(&state),.motion=state.idle_animation,
        .paused=display_screen.dimmed,.storage_error=error,.can_undo=undo_available,
        .can_hurt=state.hp||state.temp_hp,
        .can_heal=state.hp<pd_max_hp(&state)&&(state.hp||!state.status.available||state.status.failures<3),
        .applied_hp=feedback.active?feedback.hp_delta:0,.applied_temporary=feedback.active?feedback.temp_delta:0,
        .feedback_age=age,.notice=strcmp(status,"Salvato offline")?status:NULL,.avatar=&avatar_image
    };
    companion_show(lv_screen_active(),&view,companion_event,NULL);
    avatar_sprite=companion_avatar_object();
    if(avatar_sprite&&state.idle_animation)avatar_timer=lv_timer_create(avatar_frame,200,NULL);
    if(feedback.active)feedback_timer=lv_timer_create(feedback_frame,2000-age,NULL);
}
static void render_content(void) {
    /* Destroy the LVGL image/cache before freeing its pixel buffer. A root redraw
       cancels the old timer; the roll itself remains in the pure presentation state. */
    if(dice_timer) { lv_timer_delete(dice_timer);dice_timer=NULL; }
    if(avatar_timer) { lv_timer_delete(avatar_timer);avatar_timer=NULL; }
    if(feedback_timer){lv_timer_delete(feedback_timer);feedback_timer=NULL;}
    feedback_panel=NULL;memset(feedback_motes,0,sizeof feedback_motes);
    pd_theme_close();
    companion_close();
    lv_obj_t *root=lv_screen_active();lv_obj_clean(root);
    avatar_sprite=NULL;memset(scene_lights,0,sizeof scene_lights);memset(scene_halos,0,sizeof scene_halos);lv_image_cache_drop(&avatar_image);
    lv_free(dice_pixels);dice_pixels=NULL;dice_canvas=NULL;
    lv_obj_set_style_bg_color(root,lv_color_hex(PD_INK),0);
    lv_obj_remove_flag(root,LV_OBJ_FLAG_SCROLLABLE);
    char text[160];
    /* Resolve presentation fallback before selecting its title, icon and room. */
    if(!import_pending&&(page==COMBAT||page==COMBAT_DETAIL)){
        pd_weapon_info weapon;if(!pd_weapon_stats(&state,state.equipped,&weapon))page=INVENTORY;
    }
    if(!import_pending&&(page==HERO||page==HOME_MENU||page==HOME_VITALS)){render_companion();return;}
    frame();
    if (import_pending) {
        label("COPIA DA USB",83,112,300,true);
        avatar_show(&pending.avatar,83,151,root,2,false);
        label(pending.name,139,151,244,true);
        snprintf(text,sizeof text,"Liv %u  PF %u/%u  CA %u",pending.level,pending.hp,pending.max_hp,pending.armor);
        small_label(text,139,190,244,PD_GOLD);
        char items[8],powers[8];
        if(pending.inventory_available)snprintf(items,sizeof items,"%u",pending.item_count);else strcpy(items,"?");
        if(pending.magic.available)snprintf(powers,sizeof powers,"%u",pending.magic.count);else strcpy(powers,"?");
        snprintf(text,sizeof text,"Zaino %s | magie/usi %s",items,powers);small_label(text,83,216,300,PD_GOLD);
        small_label(pending.wallet.available?"Monete: saldo verificato":"Monete: saldo non disponibile",83,239,300,PD_GOLD);
        small_label("Sostituisce la copia locale.",83,262,300,PD_TEXT);
        small_label((state.effect_count||state.roll_conditions||local_defences()||state.reaction_spent)?"Stato/storico rimossi; PC intatto":"Nuovo storico; PC conservato",83,287,300,(state.effect_count||state.roll_conditions||local_defences()||state.reaction_spent)?PD_GOLD:PD_TEXT);
        button("Annulla",83,313,144,54,31,true); button("Conferma",239,313,144,54,30,true);
        return;
    }
    if(input_check_open()){render_input_check();return;}
    if(page==DAMAGE_DEATH){render_death_damage();return;}
    if(page==DEATH_ROLL_SETUP){render_death_roll_setup();return;}
    if(page==EXTERNAL_TURN_HOME){render_external_turn();return;}
    if(page==BACKUP_HOME||page==BACKUP_SEND_CONFIRM||page==BACKUP_RESTORE_CONFIRM){render_backup();return;}
    if(page==HISTORY_HOME||page==HISTORY_DETAIL||page==HISTORY_DISCARD){render_history();return;}
    if(page==WALLET_HOME||page==WALLET_EDIT||page==WALLET_CONFIRM){render_wallet();return;}
    if(page>=DAMAGE_HOME){render_damage();return;}
    if(page>=EFFECT_LIST){render_effects();return;}
    if(page==DEVICE_SETTINGS){render_settings();return;}
    if(page>=STATUS_HOME){render_status();return;}
    if(page>=REST_HOME){render_rest();return;}
    if(page==MAGIC_LIST){render_magic_list();return;}
    if(page==MAGIC_DETAIL){render_magic_detail();return;}
    if(page==MAGIC_TEXT){render_magic_text();return;}
    if(page>=MAGIC_CONFIRM){render_magic_other();return;}
    if(page==INVENTORY) {render_inventory();return;}
    if(page==ITEM_DETAIL||page==ITEM_CONFIRM) {render_item();return;}
    if(page==COMBAT) {render_combat();return;}
    if(page==COMBAT_DETAIL) {
        pd_weapon_info w;if(!pd_weapon_stats(&state,state.equipped,&w)){page=INVENTORY;render_inventory();return;}
        label(state.roll_conditions?"Bonus arma":"Bonus dell'arma",83,111,state.roll_conditions?204:300,true);if(state.roll_conditions){lv_obj_t *b=button("Contesto",299,99,84,44,2230,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
        snprintf(text,sizeof text,"Attacco: %+d %+d %+d = %+d",w.base,w.trained,w.attack_extra,w.attack_bonus);label(text,83,155,300,false);
        small_label("Caratteristica / competenza / extra",83,185,300,PD_MUTED);
        snprintf(text,sizeof text,"Danni: base %+d / extra %+d",w.damage_base,w.damage_extra);label(text,83,215,300,false);
        snprintf(text,sizeof text,"Ira %s | potente %s | critico +%ud",state.combat.raging&&w.melee&&state.items[state.equipped].ability==0?"si":"no",w.power_attack?"si":"no",w.critical_dice);small_label(text,83,247,300,PD_GOLD);
        small_label(state.training.poisoned?"PC: avvelenato (svantaggio attacco)":"PC: nessuno svantaggio da veleno",83,277,300,PD_MUTED);
        small_label("Extra include modificatori ed effetti",83,307,300,PD_MUTED);
        pd_attack_resolution gate={0};pd_attack_resolve(&state,mode,roll_context,ignore_effects,&gate);button("Indietro",83,359,144,48,85,true);button("Lancia",239,359,144,48,72,gate.allowed);return;
    }
    if(page==DICE && dice_setup) { render_dice_setup();return; }
    if(page==CHECK_DETAIL) { render_check_detail();return; }
    if(page==APPEARANCE) {
        label("Aspetto",83,111,300,true);
        art(&theme_camp_stage,83,143,root);
        scene_atmosphere(root,83,167);
        avatar_show(&state.avatar,185,143,root,4,true);
        snprintf(text,sizeof text,"Mantello %s / %s",(const char *const[]){"bosco","vino","notte"}[state.avatar.cloak],
                 (const char *const[]){"corti","lunghi","rasati"}[state.avatar.style]);
        small_label(text,83,268,300,PD_GOLD);
        button(state.idle_animation?"Animazione: si":"Animazione: no",83,299,300,48,51,true);
        button("Indietro",83,359,92,48,52,true);lv_obj_t *settings=button("Schermo",187,359,92,48,1600,true);lv_obj_set_style_text_font(settings,&lv_font_montserrat_16,0);button("Tasti",291,359,92,48,850,true);return;
    }
    if (page==SHEET) {
        render_sheet();
    } else if (page==DICE) {
        if(combat_roll==1) {
            pd_weapon_info w={0};pd_weapon_stats(&state,state.equipped,&w);
            snprintf(text,sizeof text,"Attacco %+d",strike_ready?strike.roll.bonus:w.attack_bonus);
        }
        else if(combat_roll==3||combat_roll==4)snprintf(text,sizeof text,"Cura %ud%u %+d",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else if(combat_roll==7)snprintf(text,sizeof text,"Ricarica %ud%u %+d",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else if(combat_roll==5){if(has_roll)snprintf(text,sizeof text,"Morte: %u %+d = %d",result.natural,result.bonus,result.total);else snprintf(text,sizeof text,"Salvezza morte %+d",result.bonus);}
        else if(combat_roll==6)snprintf(text,sizeof text,"COS %+d | CD %u",result.bonus,incoming_result.concentration_dc);
        else if(pool_view)snprintf(text,sizeof text,"%s %ud%u %+d",dice_animation.pool.damage?"Danni":"Somma",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else if(ability<0)snprintf(text,sizeof text,"d20 libero +0");else check_name(text,sizeof text,true);
        if(!combat_roll&&!pool_view&&ability>=0) {
            lv_obj_t *detail=button(text,83,94,300,44,70,!dice_animation.active);
            if(check_kind==PD_SKILL)lv_obj_set_style_text_font(detail,&lv_font_montserrat_20,0);
        } else label(text,83,105,300,true);
        pd_check_resolution check_gate={0};pd_attack_resolution attack_gate={0};bool check_known=ability>=0&&pd_check_resolve(&state,check_kind,ability,mode,ignore_effects,roll_context,&check_gate);pd_attack_resolve(&state,mode,roll_context,ignore_effects,&attack_gate);
        bool allowed=combat_roll==1?attack_gate.allowed:combat_roll>1||pool_view||ability<0||(check_known&&check_gate.allowed);
        int effective=combat_roll==1?attack_gate.mode:pool_view?0:ability<0?mode:check_gate.mode;
        if(!pool_view&&!has_roll&&!dice_animation.active)pd_dice_idle(&dice_animation,effective?2:1);
        if(!pd_dice_count(&dice_animation))pd_dice_idle(&dice_animation,mode?2:1);
        bool automatic_failure=!combat_roll&&!pool_view&&check_known&&check_gate.automatic_failure;
        if(!automatic_failure)dice_pixels=lv_malloc(PD_DICE_PIXELS*sizeof *dice_pixels);
        if(dice_pixels) {
            dice_canvas=lv_canvas_create(root);lv_obj_set_pos(dice_canvas,73,138);
            lv_canvas_set_buffer(dice_canvas,dice_pixels,PD_DICE_WIDTH,PD_DICE_HEIGHT,LV_COLOR_FORMAT_RGB565);
            lv_obj_add_flag(dice_canvas,LV_OBJ_FLAG_CLICKABLE);
            lv_obj_add_event_cb(dice_canvas,event,LV_EVENT_CLICKED,(void*)(intptr_t)16);
            pd_dice_paint(&dice_animation,dice_pixels);
            if(dice_animation.active)dice_timer=lv_timer_create(dice_frame,33,NULL);
        } else if(dice_animation.active) {
            /* Low memory must never hide or re-roll an outcome that already exists. */
            pd_dice_finish(&dice_animation);has_roll=true;
        }
        if(has_roll) {
            if(combat_roll==1)snprintf(text,sizeof text,"%s %d / CA %u",strike.critical?"Critico!":strike.hits?"Colpito":"Mancato",strike.roll.total,strike.armor);
            else if(combat_roll==3||combat_roll==4)snprintf(text,sizeof text,"Cura %d | PF +%u -> %u/%u",dice_animation.pool.total,healing_gained,state.hp,pd_max_hp(&state));
            else if(combat_roll==7)snprintf(text,sizeof text,"Cariche +%u | %u/%u",recharge_gained,recharge_after,recharge_maximum);
            else if(combat_roll==5)snprintf(text,sizeof text,"%s",(const char *const[]){result.natural==1?"1 naturale: due fallite":"Fallita","Riuscita","Stabilizzato","Tre fallimenti","20 naturale: riprendi 1 PF"}[status_outcome]);
            else if(combat_roll==6)snprintf(text,sizeof text,"%u %+d = %d | %s",result.natural,result.bonus,result.total,incoming_result.ends_concentration?"persa":"mantenuta");
            else if(pool_view) {
                pd_pool *r=&dice_animation.pool;
                if(r->damage && r->sum+r->bonus<0)snprintf(text,sizeof text,"Somma %d -> Danni 0",r->sum+r->bonus);
                else snprintf(text,sizeof text,"%d %+d = %d",r->sum,r->bonus,r->total);
            }
            else if(result.count==2)snprintf(text,sizeof text,"%u / %u %+d = %d [%s]",result.dice[0],result.dice[1],result.bonus,result.total,effective>0?"Vant":"Svant");
            else snprintf(text,sizeof text,"%u %+d = %d%s",result.dice[0],result.bonus,result.total,result.natural==20?"  -  20 naturale":"");
        } else snprintf(text,sizeof text,"%s",!allowed?(check_gate.automatic_failure?"Fallimento automatico":(check_gate.needed||(combat_roll==1&&attack_gate.needed))?"Dichiara Contesto":"Da verificare: apri i dettagli"):dice_animation.active?"In volo...":effective>0?"Pronto: vantaggio":effective<0?"Pronto: svantaggio":"Pronto al lancio");
        if(has_roll){decoration(80,266,306,28,PD_PANEL_LOW,2);decoration(80,266,2,28,combat_roll?PD_MINT:result.natural==20?PD_GOLD:PD_MINT,0);}
        lv_obj_t *outcome=label(text,73,269,320,false);if(has_roll&&!combat_roll&&!pool_view)lv_obj_set_style_text_color(outcome,lv_color_hex(result.natural==20?PD_GOLD:result.natural==1?PD_DANGER:PD_TEXT),0);
        if(automatic_failure)label("Nessun dado",73,183,320,false);
        else if(!dice_pixels)label("Animazione non disponibile",73,183,320,false);
        if(combat_roll==7){
            button("Zaino",83,296,76,44,80,!dice_animation.active);button(dice_animation.active?"Accelera":"Riepilogo",167,296,132,44,dice_animation.active?16:2180,true);button("Fine",307,296,76,44,100,!dice_animation.active);
        } else if(combat_roll==6){
            button("Stato",83,296,76,44,1500,!dice_animation.active);
            button(dice_animation.active?"Accelera":"Riepilogo",167,296,132,44,dice_animation.active?16:1809,true);
            button("Fine",307,296,76,44,100,!dice_animation.active);
        } else if(combat_roll) {
            button(combat_roll==5?"Stato":combat_roll==4?"Riposo":combat_roll==3?"Zaino":"Arma",83,296,76,44,combat_roll==5?1501:combat_roll==4?911:combat_roll==3?80:85,!dice_animation.active);
            button(dice_animation.active?"Accelera":combat_roll>=4?"Altro":"Nuovo",167,296,132,44,dice_animation.active?16:combat_roll==5?1501:combat_roll==4?910:combat_roll==3?80:81,true);
            button(combat_roll>=4?"Fine":"Danni",307,296,76,44,combat_roll==5?100:combat_roll==4?918:90,!dice_animation.active&&(combat_roll>=4||(combat_roll==1&&has_roll&&strike_ready&&strike.hits&&!damage_used)));
        } else {
        button(pool_view?"Prova":mode==-1?"Svant":mode==1?"Vant":"Base",83,296,76,44,pool_view?14:15,!dice_animation.active);
        button(dice_animation.active?"Accelera":has_roll?"Rilancia":"Lancia",167,296,132,44,13,allowed);
        lv_obj_t *more=button(state.roll_conditions&&!pool_view&&ability>=0?"Contesto":"Altri",307,296,76,44,state.roll_conditions&&!pool_view&&ability>=0?2230:40,!dice_animation.active);if(state.roll_conditions&&!pool_view&&ability>=0)lv_obj_set_style_text_font(more,&lv_font_montserrat_16,0);
        }
    } else {
        lv_obj_t *counter=button("Stato",83,99,92,44,1500,true);lv_obj_set_style_text_font(counter,&lv_font_montserrat_16,0);
        button("Zaino",187,99,92,44,80,true);button("Magie",291,99,92,44,800,true);
        if(state.resource_count) {
            pd_resource *r=&state.resources[resource_index];label(r->name,83,158,300,false);
            snprintf(text,sizeof text,"%u / %u",r->current,r->maximum);
            art(&theme_crystal,113,191,root);
            lv_obj_t *value=label(text,155,194,228,true);label_font(value,&pd_pixel_4);
            lv_obj_set_style_text_color(value,lv_color_hex(PD_VIOLET),0);meter(r->current,r->maximum,235,PD_VIOLET);
            button("-1",83,247,144,44,20,r->current>0);button("+1",239,247,144,44,21,r->current<r->maximum);
            button("Prossimo",83,299,144,44,22,state.resource_count>1);button("Riposi",239,299,144,44,900,true);
        } else {label("Nessun contatore importato",83,194,300,false);button("Riposi",83,299,300,44,900,true);}
    }
    const char *nav[]={"Eroe","Scheda","Dadi","Usi"};
    const lv_image_dsc_t *icons[]={&theme_hero,&theme_sheet,&theme_dice,&theme_crystal};
    const int nav_x[]={79,153,251,325},nav_w[]={68,92,68,64};
    for (int i=0;i<4;i++) {
        bool journal=page==DICE&&i==2;
        lv_obj_t *b=button(journal?"Storico":nav[i],nav_x[i],350,nav_w[i],56,journal?2600:100+i,!dice_animation.active);
        lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);lv_obj_set_style_pad_all(b,2,0);
        lv_obj_align(lv_obj_get_child(b,0),LV_ALIGN_BOTTOM_MID,0,-2);
        art(journal?&theme_notes:icons[i],(nav_w[i]-2-32)/2,0,b);
        if (page==i&&!journal) {
            lv_obj_set_style_bg_color(b,lv_color_hex(0x294c4b),0);lv_obj_set_style_bg_grad_color(b,lv_color_hex(0x183439),0);
            lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
            lv_obj_set_style_text_color(b,lv_color_hex(PD_GOLD),0);
        }
    }
}
#ifdef PD_UI_TEST
static bool styled_pages[EXTERNAL_TURN_HOME+1];
static void capture_styled_page(unsigned view);
#endif
static void render(void){
    static int last_page=-1;
    render_content();
    pd_theme_finish(state.idle_animation&&!display_screen.dimmed&&!dice_animation.active,last_page!=(int)page);
    last_page=page;
#ifdef PD_UI_TEST
    if(!styled_pages[page]){styled_pages[page]=true;capture_styled_page(page);}
#endif
}
static void dice_frame(lv_timer_t *timer) {
    (void)timer;
    if(pd_dice_tick(&dice_animation,lv_tick_get())) {
        has_roll=true;render();return;
    }
    pd_dice_paint(&dice_animation,dice_pixels);
    lv_obj_invalidate(dice_canvas);
}
static void feedback_frame(lv_timer_t *timer){
    uint32_t elapsed=lv_tick_get()-feedback.started;
    if(!feedback.active||elapsed>=2000||display_screen.dimmed){
        feedback.active=false;feedback_timer=NULL;lv_timer_delete(timer);
        if(feedback_panel)lv_obj_delete(feedback_panel);
        feedback_panel=NULL;memset(feedback_motes,0,sizeof feedback_motes);companion_clear_feedback();return;
    }
    if(!state.idle_animation||!feedback_panel)return;
    for(unsigned i=0;i<3;i++){unsigned phase=(elapsed/66+i*3)%12;lv_obj_set_pos(feedback_motes[i],280+i*7,15-phase);lv_obj_set_style_opa(feedback_motes[i],phase<8?LV_OPA_COVER:LV_OPA_30,0);}
}
static void avatar_frame(lv_timer_t *timer) {
    (void)timer;
    if(display_screen.dimmed)return;
    unsigned step=(lv_tick_get()-avatar_started)/200;
    for(unsigned i=0;i<2;i++)if(scene_halos[i])lv_obj_set_style_opa(scene_halos[i],(step+i)%3?LV_OPA_COVER:LV_OPA_70,0);
    for(unsigned i=0;i<5;i++)if(scene_lights[i])lv_obj_set_style_opa(scene_lights[i],i<2?((step+i)%3?LV_OPA_COVER:LV_OPA_50):((step+i*3)%8<2?LV_OPA_COVER:LV_OPA_20),0);
    unsigned pose=avatar_current_pose();
    if(pose==avatar_phase)return;
    avatar_set_pose(&state.avatar,pose);
}
static void shortcut(unsigned gesture) {
    if(gesture>=3||import_pending||confirmation_open())return;
    unsigned binding=state.shortcuts[gesture];if(!binding)return;
    if(dice_animation.active){if(binding==6&&page==DICE)act(16);return;}
    if(binding==6){act(page==DICE?13:14);return;}
    if(binding==1)act(100);else if(binding==2)act(101);else if(binding==3)act(14);
    else if(binding==4)act(80);else if(binding==5)act(800);else if(binding==7)act(900);else if(binding==8)act(1500);else if(binding==9)act(1600);else if(binding==10)act(1700);else if(binding==11)act(1740);else if(binding==12)act(1800);else if(binding==13)act(2600);
}
static void screen_service(int pwr,int boot){
    input_check_service();
    bool allowed=pd_screen_keys(&display_screen,pwr,boot,lv_tick_get());
    bool enabled=allowed&&!import_pending&&!confirmation_open();
    unsigned gestures=pd_buttons_sample(&physical_keys,pwr,boot,lv_tick_get(),enabled);
    for(unsigned i=0;i<3;i++)if(gestures&(1u<<i))shortcut(i);
    if(page==INPUT_CHECK_KEYS&&pd_input_check_keys(&input_check,pwr,boot,lv_tick_get(),allowed))input_check_refresh();
    bool failed=display_screen.failed;
    pd_screen_step(&display_screen,state.brightness,state.dim_timeout,import_pending||confirmation_open()||dice_animation.active,lv_tick_get(),pd_board_brightness,NULL);
    companion_pause(display_screen.dimmed);
    if(display_screen.dimmed)pd_theme_pause();
    if(display_screen.dimmed&&avatar_sprite&&avatar_phase)avatar_set_pose(&state.avatar,0);
    if(display_screen.dimmed){feedback.active=false;if(feedback_timer){lv_timer_delete(feedback_timer);feedback_timer=NULL;}if(feedback_panel){lv_obj_delete(feedback_panel);feedback_panel=NULL;}memset(feedback_motes,0,sizeof feedback_motes);}
    if(failed!=display_screen.failed&&page==DEVICE_SETTINGS)render();
}
static bool stage_backup(const pd_backup *snapshot){
    if(import_pending||confirmation_open()){reply("PDD ERROR busy\n");return false;}
    if(history_pending||!pd_history_store_available()||(!snapshot&&history.epoch!=state.history_epoch)){reply("PDD ERROR history\n");return false;}
    if(snapshot&&!pd_storage_writable()){reply("PDD ERROR storage\n");return false;}
    uint32_t epoch=state.history_epoch;
    if(snapshot&&(!pd_valid(&snapshot->state)||!pd_history_valid(&snapshot->history)||snapshot->state.history_epoch!=snapshot->history.epoch||!pd_history_store_next_epoch(state.history_epoch,&epoch))){reply("PDD ERROR backup\n");return false;}
    if(dice_animation.active){pd_dice_finish(&dice_animation);has_roll=true;}
    pending=snapshot?snapshot->state:state;backup_history=snapshot?snapshot->history:history;
    if(snapshot){pending.history_epoch=epoch;backup_history.epoch=epoch;}
    backup_notice=snapshot?"Controlla la copia da ripristinare":"Conferma l'invio al tuo PC";
    page=snapshot?BACKUP_RESTORE_CONFIRM:BACKUP_SEND_CONFIRM;pd_screen_activity(&display_screen,lv_tick_get());
    backup_started=lv_tick_get();
    reply(snapshot?"PDD WAIT_RESTORE\n":"PDD WAIT_BACKUP\n");render();return true;
}
static void backup_service(void){
    if((page==BACKUP_SEND_CONFIRM||page==BACKUP_RESTORE_CONFIRM)&&(uint32_t)(lv_tick_get()-backup_started)>=120000){
        pd_history_store_cancel_restore();page=BACKUP_HOME;backup_notice="Richiesta scaduta";reply("PDD CANCELLED timeout\n");render();
    }
}
static bool stage_import(const pd_state *candidate) {
    if(import_pending||confirmation_open()) { reply("PDD ERROR busy\n");return false; }
    uint32_t epoch;
    if(history_pending||!pd_history_store_next_epoch(state.history_epoch,&epoch)){reply("PDD ERROR history\n");return false;}
    if(dice_animation.active) { pd_dice_finish(&dice_animation);has_roll=true; }
    pd_screen_activity(&display_screen,lv_tick_get());
    pending=*candidate;pending.history_epoch=epoch;pending.brightness=state.brightness;pending.dim_timeout=state.dim_timeout;pending.idle_animation=state.idle_animation;memcpy(pending.shortcuts,state.shortcuts,3);import_pending=true;
#ifndef PD_UI_TEST
    import_started=xTaskGetTickCount();
#endif
    reply("PDD WAIT_CONFIRM\n");render();return true;
}
#ifndef PD_UI_TEST
static void usb_task(void *unused) {
    size_t used=0;bool overflow=false;uint8_t data[64];
    for (;;) {
        int count=usb_serial_jtag_read_bytes(data,sizeof data,pdMS_TO_TICKS(100));
        for(int i=0;i<count;i++) {
            char c=data[i];
            if(c=='\r')continue;
            if(c=='\n') {
                if(!overflow && used) {
                    line[used]=0;
                    if(!strcmp(line,"PDD HELLO")) reply("PDD READY 9\n");
                    else if(!strcmp(line,"PDD BACKUP HELLO"))reply("PDD BACKUP_READY 1\n");
                    else if(!strcmp(line,"PDD DIAGNOSTICS GET 1")){uint8_t request=1;if(xQueueSend(diagnostic_requests,&request,0)!=pdPASS)reply("PDD ERROR busy\n");}
                    else if(!strcmp(line,"PDD BACKUP GET 1")){pd_backup *request=NULL;if(xQueueSend(backup_packets,&request,0)!=pdPASS)reply("PDD ERROR busy\n");}
                    else {
                        pd_state *candidate=malloc(sizeof *candidate);
                        if(!candidate)reply("PDD ERROR memory\n");
                        else if(pd_parse_packet(line,used,candidate)) {
                            if(xQueueSend(packets,&candidate,0)!=pdPASS){free(candidate);reply("PDD ERROR busy\n");}
                        } else {
                            free(candidate);pd_backup *snapshot=malloc(sizeof *snapshot);
                            if(!snapshot)reply("PDD ERROR memory\n");
                            else if(pd_backup_decode(line,used,snapshot)){if(xQueueSend(backup_packets,&snapshot,0)!=pdPASS){free(snapshot);reply("PDD ERROR busy\n");}}
                            else {free(snapshot);reply("PDD ERROR packet\n");}
                        }
                    }
                } else if(overflow) reply("PDD ERROR size\n");
                used=0;overflow=false;
            } else if (used<PD_PACKET_MAX && !overflow) line[used++]=c;
            else overflow=true;
        }
    }
}
void app_main(void) {
    pd_demo(&state);if(pd_storage_load(&state))pd_history_store_load(state.history_epoch,&history);
    pd_board_screen(&display_screen);pd_board_init();
    pd_screen_activity(&display_screen,lv_tick_get());
    pd_screen_step(&display_screen,state.brightness,state.dim_timeout,true,lv_tick_get(),pd_board_brightness,NULL);
    usb_serial_jtag_driver_config_t usb=USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();
    usb.rx_buffer_size=4096;usb.tx_buffer_size=1024;
    ESP_ERROR_CHECK(usb_serial_jtag_driver_install(&usb));
    usb_output=xSemaphoreCreateMutex();assert(usb_output);
    packets=xQueueCreate(1,sizeof(pd_state *));assert(packets);
    backup_packets=xQueueCreate(1,sizeof(pd_backup *));assert(backup_packets);
    diagnostic_requests=xQueueCreate(1,sizeof(uint8_t));assert(diagnostic_requests);
    assert(xTaskCreate(usb_task,"usb_import",12288,NULL,3,&usb_thread)==pdPASS);
    render();
    for(;;) {
        pd_state *incoming=NULL;
        if(xQueueReceive(packets,&incoming,0)==pdPASS) {stage_import(incoming);free(incoming);}
        if(import_pending && (xTaskGetTickCount()-import_started)>pdMS_TO_TICKS(120000)) {
            import_pending=false;reply("PDD CANCELLED timeout\n");render();
        }
        pd_backup *snapshot=NULL;
        if(xQueueReceive(backup_packets,&snapshot,0)==pdPASS){stage_backup(snapshot);free(snapshot);}
        backup_service();
        uint8_t diagnostic_request;
        if(xQueueReceive(diagnostic_requests,&diagnostic_request,0)==pdPASS)report_diagnostics();
        pd_board_tick();
        int levels[2];pd_board_buttons(levels);
        screen_service(levels[0],levels[1]);
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

#endif
