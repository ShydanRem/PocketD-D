/* Actual firmware UI and LVGL rendered on host. No ESP peripherals simulated as verified. */
#include "lvgl.h"
static void *ui_malloc(size_t size);
#define PD_UI_TEST 1
#define lv_malloc ui_malloc
#include "../main/main.c"
#undef lv_malloc
#include <stdlib.h>
#include <math.h>
static bool fail_save,fail_canvas,fail_brightness;
static unsigned brightness_calls,brightness_last;
bool pd_board_brightness(unsigned percent,void *unused){(void)unused;brightness_calls++;brightness_last=percent;return !fail_brightness;}
static void *ui_malloc(size_t size) { return fail_canvas?NULL:lv_malloc(size); }
static unsigned random_calls,save_calls;
static uint32_t random_values[2]={9,9};
uint32_t pd_board_random(void *unused) { return random_values[(random_calls++)%2]; }
bool pd_storage_save(const pd_state *s) { save_calls++;return !fail_save; }
const char *pd_storage_status(void) { return fail_save?"Errore memoria: riavvia":"Salvato offline"; }
static uint16_t pixels[466*466];
static void paint(lv_display_t *d,const lv_area_t *a,uint8_t *data) {
    uint16_t *p=(uint16_t*)data;
    for(int y=a->y1;y<=a->y2;y++)for(int x=a->x1;x<=a->x2;x++)pixels[y*466+x]=*p++;
    lv_display_flush_ready(d);
}
static void screenshot(const char *name) {
    lv_obj_update_layout(lv_screen_active());lv_obj_invalidate(lv_screen_active());lv_refr_now(NULL);
    FILE *f=fopen(name,"wb");assert(f);fprintf(f,"P6\n466 466\n255\n");
    for(int y=0;y<466;y++)for(int x=0;x<466;x++) {
        uint16_t c=pixels[y*466+x];
        unsigned char rgb[]={((c>>11)&31)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};
        if((x-233)*(x-233)+(y-233)*(y-233)>233*233)memset(rgb,0,sizeof rgb);
        fwrite(rgb,1,3,f);
    }
    fclose(f);
}
static lv_obj_t *button_label(lv_obj_t *button) {
    for(unsigned i=0;i<lv_obj_get_child_count(button);i++) {
        lv_obj_t *o=lv_obj_get_child(button,i);if(lv_obj_check_type(o,&lv_label_class))return o;
    }
    assert(!"button label missing");return NULL;
}
static void bounds(void) {
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    /* Text must not grow into another field/control for long imported names. */
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *l=lv_obj_get_child(root,i);if(!lv_obj_check_type(l,&lv_label_class))continue;
        lv_area_t a;lv_obj_get_coords(l,&a);
        for(unsigned j=0;j<lv_obj_get_child_count(root);j++) {
            lv_obj_t *other=lv_obj_get_child(root,j);
            if(other==l||(!lv_obj_check_type(other,&lv_label_class)&&!lv_obj_check_type(other,&lv_button_class)))continue;
            lv_area_t b;lv_obj_get_coords(other,&b);
            if(!(a.x2<b.x1||a.x1>b.x2||a.y2<b.y1||a.y1>b.y2)) {
                fprintf(stderr,"Text overlaps a field/control: %s\n",lv_label_get_text(l));abort();
            }
        }
    }
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *o=lv_obj_get_child(root,i);if(!lv_obj_check_type(o,&lv_button_class))continue;
        lv_area_t a;lv_obj_get_coords(o,&a);
        assert(a.y2-a.y1+1>=44);
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2};
        for(int x=0;x<2;x++)for(int y=0;y<2;y++)assert((xs[x]-233)*(xs[x]-233)+(ys[y]-233)*(ys[y]-233)<=233*233);
        lv_obj_t *l=button_label(o);lv_area_t text;lv_obj_get_coords(l,&text);
        if(!(text.x1>=a.x1&&text.x2<=a.x2&&text.y1>=a.y1&&text.y2<=a.y2)) { fprintf(stderr,"Overflow %s: button %d,%d,%d,%d text %d,%d,%d,%d\n",lv_label_get_text(l),a.x1,a.y1,a.x2,a.y2,text.x1,text.y1,text.x2,text.y2); abort(); }
    }
}
static void click(const char *name) {
    lv_obj_t *root=lv_screen_active();
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *o=lv_obj_get_child(root,i);
        if(lv_obj_check_type(o,&lv_button_class)&&!strcmp(lv_label_get_text(button_label(o)),name)) {
            assert(!lv_obj_has_state(o,LV_STATE_DISABLED));lv_obj_send_event(o,LV_EVENT_CLICKED,NULL);bounds();return;
        }
    }
    fprintf(stderr,"Button not found: %s\n",name);assert(!"button not found");
}
/* Exercise hit coordinates when the form has multiple + and - buttons. */
static void tap(int x,int y) {
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);lv_area_t box;lv_obj_get_coords(b,&box);
        if(lv_obj_check_type(b,&lv_button_class) && x>=box.x1&&x<=box.x2&&y>=box.y1&&y<=box.y2) {
            assert(!lv_obj_has_state(b,LV_STATE_DISABLED));lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);bounds();return;
        }
    }
    assert(!"touch target not found");
}
int main(void) {
    lv_init();lv_display_t *d=lv_display_create(466,466);static uint16_t buffer[466*40];
    lv_display_set_buffers(d,buffer,NULL,sizeof buffer,LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,paint);
    pd_demo(&state);render();bounds();screenshot("hero.ppm");
    pd_state animated_before=state;unsigned idle_saves=save_calls,idle_rng=random_calls;
    assert(avatar_timer&&avatar_sprite&&!dice_timer);
    for(int frame=0;frame<16;frame++) {
        char name[48];snprintf(name,sizeof name,"avatar-frame-%02d.ppm",frame);
        screenshot(name);lv_tick_inc(200);lv_timer_handler();
    }
    assert(!memcmp(&state,&animated_before,sizeof state)&&save_calls==idle_saves&&random_calls==idle_rng);
    click("Aspetto");assert(page==APPEARANCE&&avatar_timer);screenshot("appearance.ppm");
    click("Animazione: si");assert(!state.idle_animation&&!avatar_timer);screenshot("appearance-still.ppm");
    uint8_t still[PD_AVATAR_LARGE_BYTES];memcpy(still,avatar_data,sizeof still);
    lv_tick_inc(2000);lv_timer_handler();assert(!memcmp(still,avatar_data,sizeof still));
    click("Indietro");assert(page==HERO&&!avatar_timer);click("-1 PF");assert(state.hp==23);
    click("Aspetto");click("Animazione: no");assert(state.idle_animation&&avatar_timer);
    click("Indietro");click("Annulla");assert(state.hp==24&&state.idle_animation&&!undo_available);
    click("Aspetto");fail_save=true;click("Animazione: si");assert(state.idle_animation&&avatar_timer);
    fail_save=false;click("Indietro");
    const pd_avatar presets[]={{0,0,0,0},{1,1,1,1},{2,2,2,2}};
    for(int i=0;i<3;i++) {
        state.avatar=presets[i];render();char name[40];snprintf(name,sizeof name,"avatar-preset-%d.ppm",i);screenshot(name);
    }
    state.avatar=presets[0];render();
    /* Long names and maximum supported values must remain usable in the new layout. */
    pd_state demo=state;
    memset(state.name,'W',PD_NAME_BYTES-1);state.name[PD_NAME_BYTES-1]=0;
    state.hp=state.max_hp=999;state.temp_hp=999;state.level=20;state.armor=40;
    render();bounds();screenshot("hero-maximum.ppm");
    state.hp=0;state.temp_hp=0;render();bounds();screenshot("hero-zero.ppm");
    state=demo;fail_save=true;render();bounds();screenshot("hero-storage-error.ppm");
    fail_save=false;render();
    click("-1 PF");assert(state.hp==23&&undo_available);click("Annulla");assert(state.hp==24&&!undo_available);
    fail_save=true;click("-1 PF");assert(state.hp==24);fail_save=false;
    click("Scheda");assert(!avatar_timer&&!avatar_sprite);screenshot("sheet.ppm");click("DES 16\n+3");
    unsigned saved_before=save_calls;
    click("Lancia");assert(result.total==13&&result.count==1&&dice_animation.active&&!has_roll);
    unsigned rng_before=random_calls;
    /* Forged events from disabled navigation/mode controls must not change the roll. */
    lv_obj_t *root=lv_screen_active();
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);
        if(lv_obj_check_type(b,&lv_button_class)&&lv_obj_has_state(b,LV_STATE_DISABLED))lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);
    }
    assert(page==DICE&&mode==0&&dice_animation.active&&random_calls==rng_before);
    for(int frame=0;frame<=20;frame++) {
        char name[48];snprintf(name,sizeof name,"dice-frame-%02d.ppm",frame);
        screenshot(name);lv_tick_inc(66);lv_timer_handler();
    }
    assert(has_roll&&!dice_animation.active&&!dice_timer);assert(random_calls==rng_before);
    screenshot("dice.ppm");
    rng_before=random_calls;lv_obj_send_event(dice_canvas,LV_EVENT_CLICKED,NULL);
    assert(random_calls==rng_before&&!dice_animation.active); /* Resting face never re-rolls on tap. */
    click("Rilancia");rng_before=random_calls;
    lv_obj_send_event(dice_canvas,LV_EVENT_CLICKED,NULL);
    assert(random_calls==rng_before&&!dice_animation.active);
    click("Rilancia");rng_before=random_calls;click("Accelera");
    assert(has_roll&&!dice_animation.active&&random_calls==rng_before&&result.total==13);
    click("Base");assert(mode==1);random_values[0]=0;random_values[1]=19;random_calls=0;
    click("Lancia");assert(result.total==23&&result.count==2&&result.natural==20);rng_before=random_calls;
    for(int frame=0;frame<=24;frame++) {
        char name[48];snprintf(name,sizeof name,"dice-double-%02d.ppm",frame);
        screenshot(name);lv_tick_inc(66);lv_timer_handler();
    }
    assert(has_roll&&!dice_animation.active&&random_calls==rng_before);screenshot("dice-advantage.ppm");
    click("Vant");assert(mode==-1);random_calls=0;click("Lancia");
    assert(result.natural==1&&result.total==4);click("Accelera");screenshot("dice-disadvantage.ppm");
    assert(save_calls==saved_before);
    /* No orphan timer or pixel buffer after leaving/re-entering the dice page. */
    for(int i=0;i<8;i++) { click("Eroe");assert(!dice_timer&&!dice_pixels&&!dice_canvas);click("Dadi"); }
    ability=-1;has_roll=false;pd_dice_idle(&dice_animation,2);render();
    assert(dice_animation.preview&&dice_animation.roll.count==2);
    rng_before=random_calls;fail_canvas=true;click("Lancia");
    assert(has_roll&&!dice_animation.active&&!dice_timer&&!dice_pixels);
    assert(result.natural==1&&random_calls==rng_before+2);
    fail_canvas=false;click("Dadi");assert(dice_pixels);
    click("Scheda");state.abilities[1]=30;render();click("DES 30\n+10");
    random_values[0]=random_values[1]=19;random_calls=0;click("Lancia");click("Accelera");
    assert(result.natural==20&&result.total==30);screenshot("dice-twenty.ppm");
    /* An import preview stops a flight without replacing its already-generated result. */
    click("Rilancia");pd_roll before_import=result;
    pd_state incoming=state;assert(stage_import(&incoming));assert(!avatar_timer&&!dice_timer);click("Annulla");
    assert(has_roll&&!memcmp(&result,&before_import,sizeof result)&&!dice_timer);
    click("Altri");click("Prova");assert(!pool_view&&ability==-1&&mode==0&&!has_roll);
    /* Prepare a manual 6d6 +3 damage roll through the actual touch UI. */
    unsigned saves=save_calls;pd_state untouched=state;
    click("Altri");assert(dice_setup);screenshot("pool-setup.ppm");
    click("Tipo: d6");assert(die_picker);screenshot("pool-picker.ppm");click("d6");
    for(int i=1;i<6;i++)tap(359,234);
    assert(pool_count==6);for(int i=0;i<3;i++)tap(359,296);
    click("Somma");assert(pool_damage&&pool_bonus==3);screenshot("pool-configured.ppm");
    random_values[0]=0;random_values[1]=5;random_calls=0;click("Lancia");
    assert(pool_view&&dice_animation.active&&dice_animation.pool.count==6&&dice_animation.pool.total==24);
    unsigned calls=random_calls;assert(calls==6);
    for(int frame=0;frame<=29;frame++) {
        char name[48];snprintf(name,sizeof name,"pool-frame-%02d.ppm",frame);
        screenshot(name);lv_tick_inc(66);lv_timer_handler();
    }
    assert(has_roll&&!dice_animation.active&&random_calls==calls);screenshot("pool-six.ppm");
    pd_pool kept=dice_animation.pool;
    click("Altri");tap(107,234);tap(107,296);click("Danni");
    click("Esci");assert(!dice_setup&&!memcmp(&kept,&dice_animation.pool,sizeof kept));
    click("Rilancia");calls=random_calls;click("Accelera");assert(random_calls==calls&&has_roll);
    /* Small and large pools, all sprite shapes, with a legible natural value on each die. */
    const unsigned face_types[]={4,6,8,10,12,20};
    for(unsigned i=0;i<6;i++) {
        click("Altri");char old[32],choice[16],shot[40];snprintf(old,sizeof old,"Tipo: d%u",pool_sides);
        click(old);snprintf(choice,sizeof choice,"d%u",face_types[i]);click(choice);
        while(pool_count>1)tap(107,234);
        pool_bonus=0;pool_damage=false;render();random_values[0]=face_types[i]-1;random_values[1]=face_types[i]-1;random_calls=0;
        click("Lancia");click("Accelera");
        assert(dice_animation.pool.total==(int)face_types[i]);
        snprintf(shot,sizeof shot,"pool-d%u.ppm",face_types[i]);screenshot(shot);
    }
    /* Sum may be negative; damage is explicitly clamped, never applied to hero HP. */
    click("Altri");click("Tipo: d20");click("d4");pool_bonus=-15;pool_damage=false;render();
    random_values[0]=random_values[1]=0;click("Lancia");click("Accelera");assert(dice_animation.pool.total==-14);screenshot("pool-negative.ppm");
    click("Altri");click("Somma");click("Lancia");click("Accelera");assert(dice_animation.pool.total==0);screenshot("pool-zero.ppm");
    click("Altri");pool_sides=20;pool_count=6;pool_bonus=30;pool_damage=true;render();
    random_values[0]=random_values[1]=19;click("Lancia");click("Accelera");assert(dice_animation.pool.total==150);screenshot("pool-maximum.ppm");
    assert(!memcmp(&state,&untouched,sizeof state)&&save_calls==saves);
    click("Prova");assert(!pool_view&&ability==-1&&mode==0);click("Base");click("Lancia");click("Accelera");assert(result.count==2);
    click("Altri");pool_count=2;pool_sides=8;render();fail_canvas=true;calls=random_calls;click("Lancia");
    assert(pool_view&&has_roll&&!dice_animation.active&&random_calls==calls+2&&!dice_timer);fail_canvas=false;
    click("Eroe");assert(!dice_pixels&&!dice_timer);click("Dadi");assert(dice_pixels);
    click("Usi");click("-1");assert(state.resources[0].current==0);click("+1");assert(state.resources[0].current==1);screenshot("resources.ppm");
    incoming=state;strcpy(incoming.name,"Nuovo eroe");incoming.hp=6;incoming.avatar=presets[2];
    assert(stage_import(&incoming));assert(!stage_import(&incoming));bounds();screenshot("confirm.ppm");
    click("Annulla");assert(state.hp==24&&!import_pending);
    state.idle_animation=false;assert(stage_import(&incoming));assert(!pending.idle_animation);
    click("Conferma");assert(state.hp==6&&!import_pending&&!undo_available&&state.avatar.skin==2&&!state.idle_animation&&!avatar_timer);
    screenshot("hero-imported.ppm");
    for(int i=0;i<8;i++) {
        click("Aspetto");click("Animazione: no");assert(avatar_timer);
        click("Indietro");click("Scheda");assert(!avatar_timer&&!avatar_sprite);
        click("Eroe");click("Aspetto");click("Animazione: si");assert(!avatar_timer);click("Indietro");
    }
    /* Full touch flow: training, half proficiency, extras and automatic modes. */
    pd_demo(&state);state.level=5;state.training.ranks[11]=2;state.training.skill_bonus[11]=2;
    state.training.half_skills=1;state.training.skill_bonus[0]=-1;
    state.training.half_checks=true;state.training.check_bonus[1]=-2;
    state.training.saves=2;state.training.save_bonus[1]=1;
    state.training.blocked=1u<<(6+2);state.training.poisoned=true;state.training.strength_advantage=true;
    pd_state trained=state;unsigned check_saves=save_calls;
    page=SHEET;sheet_tab=0;skill_page=0;render();bounds();screenshot("sheet-trained.ppm");
    click("Abilita");screenshot("skills-first.ppm");
    for(int i=0;i<5;i++) { click(">");assert(skill_page==i+1); }
    screenshot("skills-last.ppm");for(int i=0;i<5;i++)click("<");assert(!skill_page);
    click("Acrobazia  +3");assert(check_kind==PD_SKILL&&ability==0&&pd_dice_count(&dice_animation)==2);
    click("Acrobazia +3 >");assert(page==CHECK_DETAIL);screenshot("check-detail.ppm");
    random_values[0]=random_values[1]=9;calls=random_calls;click("Lancia");
    assert(result.count==2&&result.total==13&&random_calls==calls+2);click("Accelera");screenshot("skill-result.ppm");
    click("Acrobazia +3 >");click("Base");assert(mode==1);
    calls=random_calls;click("Lancia");assert(result.count==1&&result.total==13&&random_calls==calls+1);click("Accelera");
    click("Acrobazia +3 >");click("Effetti PC: si");assert(ignore_effects);
    click("Lancia");assert(result.count==2);click("Accelera");
    click("Scheda");click("Arcano  ?");assert(!dice_animation.active);calls=random_calls;
    for(unsigned i=0;i<lv_obj_get_child_count(lv_screen_active());i++) {
        lv_obj_t *b=lv_obj_get_child(lv_screen_active(),i);
        if(lv_obj_check_type(b,&lv_button_class)&&!strcmp(lv_label_get_text(button_label(b)),"Lancia"))lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);
    }
    assert(random_calls==calls&&!dice_animation.active);
    click("Arcano ?");screenshot("check-blocked.ppm");click("Libero");
    click("Lancia");assert(result.count==1&&result.bonus==0&&result.total==10);click("Accelera");
    click("Scheda");click("Salvezze");screenshot("saves.ppm");click("DES\n+7");
    click("Lancia");assert(result.count==1&&result.total==17);click("Accelera");
    click("Salv. DES +7 >");screenshot("save-detail.ppm");click("Indietro");
    click("Scheda");click("FOR\n+2");click("Lancia");assert(result.count==2&&result.bonus==2);click("Accelera");
    assert(save_calls==check_saves&&!memcmp(&state,&trained,sizeof state));
    state.training=(pd_training){0};page=SHEET;sheet_tab=1;render();bounds();screenshot("skills-legacy.ppm");
    click("Caratt.");click("DES 16\n+3");click("Lancia");assert(result.count==1&&result.bonus==3);click("Accelera");
    /* Device inventory: atomic consumables, attack -> critical -> one damage roll. */
    pd_demo(&state);combat_roll=0;has_roll=false;page=RESOURCES;inventory_page=0;render();bounds();
    click("Zaino");assert(page==INVENTORY);screenshot("inventory.ppm");
    click("Spada *  x1");assert(page==ITEM_DETAIL);screenshot("weapon.ppm");click("Riponi");assert(state.equipped==PD_NO_WEAPON);click("Scegli");assert(state.equipped==0);click("Attacco");
    assert(page==COMBAT);screenshot("combat.ppm");click("Attacco +4 >");screenshot("combat-detail.ppm");click("Indietro");
    random_values[0]=random_values[1]=19;calls=random_calls;unsigned combat_saves=save_calls;
    click("Lancia");assert(strike.critical&&strike.hits&&random_calls==calls+1);
    click("Accelera");screenshot("attack-critical.ppm");
    random_values[0]=random_values[1]=0;click("Danni");assert(damage_used&&dice_animation.pool.count==2&&dice_animation.pool.total==4);
    click("Accelera");screenshot("weapon-damage.ppm");assert(save_calls==combat_saves);
    calls=random_calls;root=lv_screen_active();
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);
        if(lv_obj_check_type(b,&lv_button_class)&&!strcmp(lv_label_get_text(button_label(b)),"Danni"))lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);
    }
    assert(random_calls==calls);click("Nuovo");click("Lancia");assert(!strike.hits&&strike.roll.natural==1);
    click("Accelera");screenshot("attack-miss.ppm");click("Arma");click("Zaino");
    click("Pozione  x2");assert(page==ITEM_DETAIL);calls=random_calls;click("Usa");assert(page==ITEM_CONFIRM&&random_calls==calls);
    screenshot("item-confirm.ppm");click("Indietro");assert(state.items[1].quantity==2&&state.hp==24);
    click("Usa");state.temp_hp=4;click("Conferma");assert(combat_roll==3&&state.hp==28&&state.temp_hp==4&&state.items[1].quantity==1&&random_calls==calls+2);
    click("Accelera");screenshot("healing.ppm");click("Eroe");click("Annulla");assert(state.hp==24&&state.temp_hp==4&&state.items[1].quantity==2);
    click("Usi");click("Zaino");click("Torcia  x3");calls=random_calls;click("Usa");click("Conferma");assert(state.items[2].quantity==2&&random_calls==calls);
    click("Zaino");click("Pozione  x2");click("Usa");pd_state before_failed_use=state;fail_save=true;click("Conferma");
    assert(!memcmp(&state,&before_failed_use,sizeof state)&&page==ITEM_CONFIRM);fail_save=false;click("Indietro");click("Zaino");
    state.hp=state.max_hp;click("Pozione  x2");calls=random_calls;
    root=lv_screen_active();for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);if(lv_obj_check_type(b,&lv_button_class)&&!strcmp(lv_label_get_text(button_label(b)),"PF al massimo"))lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);
    }
    assert(page==ITEM_DETAIL&&random_calls==calls&&state.items[1].quantity==2);
    pd_demo(&state);state.items[0].count=6;state.combat.critical_dice=6;page=COMBAT;combat_roll=1;pool_view=false;has_roll=false;render();
    random_values[0]=random_values[1]=19;click("Lancia");click("Accelera");random_values[0]=random_values[1]=0;click("Danni");
    assert(dice_animation.pool.count==18&&dice_animation.pool.total==20);click("Accelera");screenshot("damage-eighteen.ppm");
    pd_demo(&state);for(unsigned i=3;i<PD_ITEMS;i++){state.items[i]=(pd_item){.name="Nome molto lungo dell oggetto da vedere",.quantity=999};}
    state.item_count=PD_ITEMS;page=INVENTORY;inventory_page=0;render();bounds();
    for(unsigned i=0;i<19;i++)click(">");
    assert(inventory_page==19);screenshot("inventory-last.ppm");tap(210,177);screenshot("item-long-name.ppm");
    memset(state.items[item_index].name,'W',40);state.items[item_index].name[40]=0;render();bounds();
    lv_point_t extent;lv_text_get_size(&extent,state.items[item_index].name,&lv_font_montserrat_16,0,0,300,LV_TEXT_FLAG_NONE);assert(extent.y<=58);
    page=INVENTORY;render();bounds();
    pd_demo(&state);state.items[0].quantity=0;state.equipped=PD_NO_WEAPON;item_index=0;page=ITEM_DETAIL;render();bounds();screenshot("weapon-empty.ppm");page=INVENTORY;
    state.inventory_available=false;state.item_count=0;state.equipped=PD_NO_WEAPON;inventory_page=0;render();bounds();screenshot("inventory-legacy.ppm");
    /* Full magic and configurable-key flows use the actual LVGL views/actions. */
    pd_demo(&state);state.magic.count=6;state.magic.slots[0]=(pd_slot){2,3};state.magic.pact[0]=(pd_slot){1,1};state.combat.rage_bonus=2;
    state.magic.powers[0]=(pd_power){.name="Luce sintetica",.description="Una luce originale per il test.",.flags=PD_PREPARED};
    state.magic.powers[1]=(pd_power){.name="Scudo sintetico",.level=1,.flags=PD_PREPARED|PD_USES_SLOT|PD_CONCENTRATES|PD_EXCERPT,.ability=3};
    snprintf(state.magic.powers[1].description,sizeof state.magic.powers[1].description,"%s",
      "Una barriera di luce si accende intorno al viandante. Il mantello si solleva e piccoli frammenti luminosi disegnano un cerchio sul terreno. "
      "Questa descrizione originale serve a verificare la lettura sul display rotondo. "
      "Prima di usare una magia controlla preparazione, slot disponibile e concentrazione. "
      "I valori di attacco e CD derivano dalla caratteristica indicata e dalla competenza. "
      "Gli effetti sul bersaglio e le eccezioni si risolvono al tavolo con il DM. "
      "Il testo completo resta nella copia sul PC quando questa vista contiene solo un estratto della descrizione personale.");
    state.magic.powers[2]=(pd_power){.name="Innato sintetico",.level=2,.flags=PD_PREPARED|PD_LIMITED,.maximum=1,.remaining=1};
    state.magic.powers[3]=(pd_power){.name="Ira sintetica",.kind=1,.effect=1,.maximum=2,.remaining=2};
    state.magic.powers[4]=(pd_power){.name="Nota sintetica",.kind=1,.flags=PD_PASSIVE};
    state.magic.powers[5]=(pd_power){.name="Resistenza sintetica",.kind=1,.effect=2,.maximum=1,.remaining=1};
    assert(pd_valid(&state));page=RESOURCES;magic_filter=2;magic_page=0;render();bounds();click("Magie");screenshot("magic-list.ppm");
    click("Scudo sintetico");assert(page==MAGIC_DETAIL&&cast_level==1&&!cast_pact);click("Slot 1 ordinario >");assert(cast_level==1&&cast_pact);screenshot("magic-detail.ppm");
    click("Testo");assert(page==MAGIC_TEXT);screenshot("magic-text.ppm");click(">");assert(text_page==1);click("Indietro");
    static pd_state before_magic;before_magic=state;unsigned magic_saves=save_calls;calls=random_calls;
    click("Lancia");assert(page==MAGIC_CONFIRM&&!memcmp(&state,&before_magic,sizeof state)&&save_calls==magic_saves&&random_calls==calls);screenshot("magic-confirm.ppm");
    shortcut(0);assert(page==MAGIC_CONFIRM&&!memcmp(&state,&before_magic,sizeof state));click("Indietro");click("Lancia");click("Conferma");
    assert(page==MAGIC_DETAIL&&state.magic.concentration==2&&!state.magic.pact[0].current&&state.magic.slots[0].current==2&&random_calls==calls);
    click("Elenco");click("Usi");click("Eroe");click("Annulla");assert(!state.magic.concentration&&state.magic.pact[0].current==1);
    click("Usi");click("Magie");click("Scudo sintetico");click("Lancia");click("Conferma");click("Elenco");click("Risorse");screenshot("magic-status.ppm");
    click("Concentrazione: termina");assert(page==MAGIC_STOP);shortcut(0);assert(page==MAGIC_STOP);click("Conferma");assert(!state.magic.concentration);
    click("Slot ordinari / patto");assert(page==MAGIC_SLOTS);screenshot("magic-slots.ppm");unsigned slot_before=state.magic.slots[0].current;click("-1");assert(state.magic.slots[0].current==slot_before-1);click("+1");assert(state.magic.slots[0].current==slot_before);click("Patto");assert(slot_pact);click("-1");assert(!state.magic.pact[0].current);click("+1");assert(state.magic.pact[0].current==1);
    click("Risorse");click("Elenco");click("Innato sintetico");click("Lancia");click("Conferma");assert(!state.magic.powers[2].remaining);click("Utilizzi");screenshot("magic-recovery.ppm");click("+1");assert(state.magic.powers[2].remaining==1);click("Indietro");
    click("Elenco");click("Capacita");click("Ira sintetica");click("Usa");click("Conferma");assert(state.combat.raging&&state.training.strength_advantage&&state.magic.powers[3].remaining==1);
    click("Elenco");click("Magie");click("Luce sintetica");before_magic=state;act(831);assert(page==MAGIC_DETAIL&&!memcmp(&state,&before_magic,sizeof state));
    click("Elenco");click("Risorse");click("Ira: termina");click("Conferma");assert(!state.combat.raging&&!state.training.strength_advantage);
    click("Elenco");click("Luce sintetica");magic_saves=save_calls;click("Lancia");click("Conferma");assert(save_calls==magic_saves); /* A cantrip with no state changes does not wear flash. */
    click("Elenco");click("Scudo sintetico");click("Lancia");before_magic=state;fail_save=true;click("Conferma");assert(page==MAGIC_CONFIRM&&!memcmp(&state,&before_magic,sizeof state));fail_save=false;click("Indietro");
    click("Elenco");click("Usi");click("Eroe");click("Aspetto");click("Tasti");assert(page==KEY_SETTINGS);screenshot("keys.ppm");
    click("BOOT breve: Dadi");assert(state.shortcuts[1]==4);calls=random_calls;shortcut(1);assert(page==INVENTORY&&random_calls==calls);
    shortcut(0);assert(page==HERO);state.shortcuts[1]=6;act(14);shortcut(1);assert(dice_animation.active);calls=random_calls;shortcut(0);assert(page==DICE&&dice_animation.active);shortcut(1);assert(!dice_animation.active&&random_calls==calls);
    static pd_state imported_magic;pd_demo(&imported_magic);assert(stage_import(&imported_magic)&&pending.shortcuts[1]==6);shortcut(0);assert(import_pending);click("Annulla");
    act(100);click("Aspetto");click("Tasti");before_magic=state;fail_save=true;act(860);assert(!memcmp(&state,&before_magic,sizeof state));fail_save=false;
    pd_demo(&state);state.magic.count=PD_POWERS;for(unsigned i=0;i<PD_POWERS;i++)state.magic.powers[i]=(pd_power){.name="Magia sintetica lunga da consultare",.flags=PD_PREPARED};
    page=MAGIC_LIST;magic_filter=2;magic_page=39;render();bounds();screenshot("magic-last.ppm");
    memset(state.magic.powers[0].description,'W',500);state.magic.powers[0].description[500]=0;power_index=0;page=MAGIC_TEXT;text_page=3;render();bounds();
    state.magic.available=false;state.magic.count=0;magic_page=0;page=MAGIC_LIST;render();bounds();screenshot("magic-legacy.ppm");
    state.shortcuts[0]=7;shortcut(0);assert(page==REST_HOME);state.shortcuts[0]=1;
    /* Actual rest pages: preview/cancel, multiclass policies, atomic animated cures. */
    pd_demo(&state);undo_available=false;combat_roll=0;has_roll=false;state.hp=20;state.temp_hp=7;
    state.magic.count=3;for(unsigned i=0;i<3;i++)state.magic.powers[i]=(pd_power){.name="Risorsa sintetica",.kind=1,.maximum=3,.remaining=1,.recovery=i};
    state.magic.slots[0]=(pd_slot){0,3};state.magic.pact[1]=(pd_slot){0,2};state.combat.rage_bonus=2;state.combat.raging=true;state.training.strength_advantage=true;
    state.recovery=(pd_recovery){.available=true,.count=2,.pools={{.name="Classe del viandante",.die=8,.maximum=3,.used=1},{.name="Classe del guardiano",.die=12,.maximum=2,.used=1}}};
    state.shortcuts[1]=6;page=RESOURCES;render();bounds();click("Riposi");screenshot("rest-home.ppm");
    static pd_state before_rest;before_rest=state;calls=random_calls;unsigned rest_saves=save_calls;
    click("Riposo breve");screenshot("rest-short.ppm");assert(page==REST_PREVIEW&&!memcmp(&state,&before_rest,sizeof state)&&random_calls==calls&&save_calls==rest_saves);
    shortcut(0);act(100);assert(page==REST_PREVIEW);assert(!stage_import(&imported_magic)&&!import_pending);click("Indietro");assert(!memcmp(&state,&before_rest,sizeof state));
    click("Riposo breve");fail_save=true;click("Conferma");assert(page==REST_PREVIEW&&!memcmp(&state,&before_rest,sizeof state));fail_save=false;click("Conferma");
    assert(page==REST_SESSION&&rest_active&&state.hp==20&&state.temp_hp==7&&!state.combat.raging&&!state.training.strength_advantage&&state.magic.pact[1].current==2&&!state.magic.slots[0].current);
    assert(state.magic.powers[0].remaining==1&&state.magic.powers[1].remaining==3&&state.magic.powers[2].remaining==1&&random_calls==calls);
    click("Spendi un dado vita");screenshot("hit-dice.ppm");click("Spendi 1 dado");screenshot("hit-confirm.ppm");
    assert(page==HIT_CONFIRM&&random_calls==calls);shortcut(1);act(100);assert(page==HIT_CONFIRM&&!stage_import(&imported_magic));
    click("Indietro");assert(page==HIT_DICE&&state.recovery.pools[0].used==1);click("Spendi 1 dado");
    before_rest=state;random_values[0]=random_values[1]=5;fail_save=true;click("Conferma");assert(hit_rolled&&page==HIT_CONFIRM&&!memcmp(&state,&before_rest,sizeof state)&&random_calls==calls+1);
    click("Conferma");assert(random_calls==calls+1&&!memcmp(&state,&before_rest,sizeof state));fail_save=false;click("Conferma");
    assert(page==DICE&&combat_roll==4&&state.hp==28&&state.temp_hp==7&&state.recovery.pools[0].used==2&&random_calls==calls+1);
    calls=random_calls;shortcut(1);assert(!dice_animation.active&&random_calls==calls&&state.hp==28);bounds();screenshot("hit-healing.ppm");
    click("Riposo");click("Spendi un dado vita");click("Spendi 1 dado");random_values[0]=random_values[1]=7;click("Conferma");click("Accelera");assert(state.hp==32&&state.recovery.pools[0].used==3&&healing_gained==4);
    click("Altro");calls=random_calls;act(915);assert(page==HIT_DICE&&random_calls==calls);click("Indietro");click("Fine");assert(!rest_active);
    click("Eroe");click("Annulla");assert(state.hp==28&&state.recovery.pools[0].used==2);click("Usi");click("Riposi");
    before_rest=state;click("Riposo lungo");screenshot("rest-settings.ppm");click("Anteprima");assert(page==REST_PREVIEW&&!memcmp(&state,&before_rest,sizeof state));click("Indietro");
    click("Dadi vita: manuale >");click("Scegli dadi da recuperare");click("+1");click(">");click("+1");screenshot("rest-regain.ppm");
    act(917);assert(selected_hit_dice()==2&&rest_plan.regain[0]==1&&rest_plan.regain[1]==1);
    click("Indietro");click("Anteprima");screenshot("rest-long.ppm");calls=random_calls;shortcut(1);assert(page==REST_PREVIEW&&random_calls==calls);click("Indietro");
    click("Dadi vita: 2014 >");assert(rest_plan.policy==PD_HD_2024&&!selected_hit_dice());click("Anteprima");assert(scratch.recovery.pools[0].used==0&&scratch.recovery.pools[1].used==0);click("Indietro");
    click("Dadi vita: 2024 >");click("Dadi vita: manuale >");click("Scegli dadi da recuperare");click("+1");click(">");click("+1");click("Indietro");click("Anteprima");click("Conferma");
    assert(page==REST_HOME&&state.hp==32&&!state.temp_hp&&state.magic.slots[0].current==3&&state.magic.powers[2].remaining==3&&state.magic.powers[0].remaining==1&&state.recovery.pools[0].used==1&&!state.recovery.pools[1].used&&random_calls==calls);
    assert(!memcmp(state.items,before_rest.items,sizeof state.items)&&!memcmp(state.resources,before_rest.resources,sizeof state.resources));
    click("Usi");click("Eroe");click("Annulla");assert(!memcmp(&state,&before_rest,sizeof state));
    click("Usi");click("Riposi");click("Riposo lungo");click("Dadi vita: manuale >");click("Dadi vita: 2014 >");click("Anteprima");click("Conferma");assert(!state.recovery.pools[0].used&&!state.recovery.pools[1].used);
    rest_saves=save_calls;click("Riposo lungo");click("Anteprima");click("Conferma");assert(save_calls==rest_saves); /* No-op rest avoids flash wear. */
    state.recovery.available=false;state.recovery.count=0;click("Riposo breve");click("Conferma");screenshot("rest-legacy.ppm");click("Fine");
    state.recovery=(pd_recovery){.available=true,.count=20};for(unsigned i=0;i<20;i++)state.recovery.pools[i]=(pd_hit_die){.name="Classe",.die=12,.maximum=20,.used=i};
    memset(state.recovery.pools[19].name,'W',40);state.recovery.pools[19].name[40]=0;state.hp=20;rest_active=true;hit_index=19;page=HIT_DICE;render();bounds();screenshot("hit-last.ppm");
    /* State notes, independent poison flag and committed death-save animations. */
    pd_demo(&state);state.hp=0;state.temp_hp=4;combat_roll=0;has_roll=false;undo_available=false;rest_active=false;
    state.status=(pd_status){.available=true,.count=3,.successes=1,.failures=1,.conditions={{.name="Poisoned",.level=0},{.name="Spossatezza",.level=1},{.name="Nota del DM",.level=2}}};state.training.poisoned=true;state.shortcuts[1]=6;
    page=RESOURCES;render();bounds();click("Stato");assert(page==STATUS_HOME);screenshot("status-home.ppm");
    static pd_state before_status;before_status=state;calls=random_calls;unsigned status_saves=save_calls;
    click("Avvelenato: si");screenshot("status-poison.ppm");assert(page==STATUS_CONFIRM&&!memcmp(&state,&before_status,sizeof state));shortcut(0);act(100);assert(page==STATUS_CONFIRM&&!stage_import(&imported_magic));
    click("Indietro");assert(!memcmp(&state,&before_status,sizeof state)&&random_calls==calls&&save_calls==status_saves);click("Avvelenato: si");click("Conferma");assert(!state.training.poisoned&&state.status.count==3);
    click("Condizioni: 3 note");screenshot("conditions.ppm");click("Poisoned");screenshot("condition-detail.ppm");
    click("+1");assert(page==STATUS_CONFIRM&&state.status.conditions[0].level==0);click("Indietro");assert(state.status.conditions[0].level==0);
    click("+1");before_status=state;fail_save=true;click("Conferma");assert(!memcmp(&state,&before_status,sizeof state)&&page==STATUS_CONFIRM);fail_save=false;click("Conferma");assert(state.status.conditions[0].level==1&&!state.training.poisoned);
    click("Rimuovi nota");click("Conferma");assert(state.status.count==2&&!strcmp(state.status.conditions[0].name,"Spossatezza")&&!state.training.poisoned);
    click("Aggiungi");screenshot("condition-picker.ppm");click(">");click("Spaventato");click("Conferma");assert(state.status.count==3&&!strcmp(state.status.conditions[2].name,"Spaventato"));
    click("Indietro");click("Salvezze contro morte");screenshot("death-tracker.ppm");
    state.training.poisoned=true;before_status=state;calls=random_calls;status_saves=save_calls;click("Lancia d20 base");screenshot("death-confirm.ppm");assert(page==STATUS_CONFIRM&&save_calls==status_saves&&random_calls==calls);
    shortcut(1);assert(page==STATUS_CONFIRM);click("Indietro");assert(!memcmp(&state,&before_status,sizeof state));click("Lancia d20 base");random_values[0]=random_values[1]=9;
    fail_save=true;click("Conferma");assert(status_rolled&&random_calls==calls+1&&page==STATUS_CONFIRM&&!memcmp(&state,&before_status,sizeof state));click("Conferma");assert(random_calls==calls+1);
    fail_save=false;click("Conferma");assert(page==DICE&&combat_roll==5&&result.count==1&&!result.bonus&&state.status.successes==2&&state.status.failures==1&&random_calls==calls+1);
    calls=random_calls;shortcut(1);assert(!dice_animation.active&&random_calls==calls);bounds();screenshot("death-result.ppm");act(13);assert(random_calls==calls&&state.status.successes==2);
    click("Stato");click("Correggi / stabilizza");screenshot("death-edit.ppm");click("Riuscita +");click("Conferma");assert(state.status.stabilized&&!state.status.successes&&!state.status.failures&&state.hp==0);screenshot("death-stable.ppm");act(1510);assert(page==DEATH_VIEW&&random_calls==calls);
    click("Correggi / stabilizza");click("Critico +2");click("Conferma");assert(!state.status.stabilized&&state.status.failures==2);
    click("Lancia d20 base");random_values[0]=random_values[1]=0;click("Conferma");click("Accelera");assert(state.status.failures==3&&!state.hp);screenshot("death-three.ppm");click("Fine");
    before_status=state;status_saves=save_calls;act(2);assert(!memcmp(&state,&before_status,sizeof state)&&save_calls==status_saves);click("Usi");click("Riposi");act(903);assert(page==REST_HOME);click("Usi");click("Zaino");click("Pozione  x2");calls=random_calls;act(88);assert(page==ITEM_DETAIL&&random_calls==calls&&state.items[1].quantity==2);
    act(1501);click("Correggi / stabilizza");click("Fallita -");click("Conferma");assert(state.status.failures==2);click("Lancia d20 base");random_values[0]=random_values[1]=19;click("Conferma");click("Accelera");assert(state.hp==1&&!state.status.failures&&!state.status.successes);screenshot("death-twenty.ppm");
    click("Eroe");click("Annulla");assert(!state.hp&&state.status.failures==2);state.shortcuts[0]=7;act(850);click("PWR breve: Riposi");assert(state.shortcuts[0]==8);shortcut(0);assert(page==STATUS_HOME);before_status=state;assert(stage_import(&imported_magic)&&pending.shortcuts[0]==8);click("Annulla");assert(!memcmp(&state,&before_status,sizeof state));
    state.status=(pd_status){0};page=DEATH_VIEW;render();bounds();screenshot("death-legacy.ppm");click("Avvia registro");before_status=state;click("Indietro");assert(!state.status.available&&!memcmp(&state,&before_status,sizeof state));click("Avvia registro");click("Conferma");assert(state.status.available&&!state.status.failures&&!state.status.successes);
    state.status.count=30;for(unsigned i=0;i<30;i++){memset(state.status.conditions[i].name,'W',60);state.status.conditions[i].name[60]=0;state.status.conditions[i].level=i%7;}
    page=CONDITION_LIST;condition_page=9;render();bounds();screenshot("condition-last.ppm");tap(210,302);assert(condition_index==29);screenshot("condition-long.ppm");
    page=DEATH_VIEW;state.status.successes=3;state.status.failures=3;render();bounds();calls=random_calls;act(1510);assert(page==DEATH_VIEW&&random_calls==calls); /* Imported conflicts require a correction. */
    /* Device-local display prefs, real main-loop policy and swallowed wake gestures. */
    pd_demo(&state);display_screen=(pd_screen){0};physical_keys=(pd_buttons){0};combat_roll=0;has_roll=false;page=HERO;render();
    click("-1 PF");click("Aspetto");click("Schermo");assert(page==DEVICE_SETTINGS);screenshot("screen-settings.ppm");
    calls=random_calls;click("+10");click("+10");click("Attenua: mai >");click("Attenua: 30 sec >");assert(state.brightness==70&&state.dim_timeout==2&&random_calls==calls);
    static pd_state prefs_before;prefs_before=state;fail_save=true;click("+10");assert(!memcmp(&state,&prefs_before,sizeof state));fail_save=false;
    click("Indietro");click("Indietro");click("Annulla");assert(state.hp==24&&state.brightness==70&&state.dim_timeout==2);
    assert(stage_import(&imported_magic)&&pending.brightness==70&&pending.dim_timeout==2);click("Annulla");assert(state.brightness==70&&state.dim_timeout==2);
    state.dim_timeout=1;state.shortcuts[1]=1;page=DEVICE_SETTINGS;pd_screen_activity(&display_screen,lv_tick_get());render();screen_service(-1,0);assert(brightness_last==70);
    lv_tick_inc(29999);screen_service(-1,0);assert(!display_screen.dimmed);lv_tick_inc(1);screen_service(-1,0);assert(display_screen.dimmed&&brightness_last==10);
    calls=random_calls;screen_service(-1,1);assert(!display_screen.dimmed&&page==DEVICE_SETTINGS&&brightness_last==70&&random_calls==calls);
    lv_tick_inc(100);screen_service(-1,1);screen_service(-1,0);lv_tick_inc(31);screen_service(-1,0);lv_tick_inc(31);screen_service(-1,0);
    screen_service(-1,1);lv_tick_inc(31);screen_service(-1,1);screen_service(-1,0);lv_tick_inc(31);screen_service(-1,0);assert(page==HERO);
    lv_tick_inc(30000);screen_service(-1,0);assert(display_screen.dimmed);assert(!pd_screen_touch(&display_screen,true,true,lv_tick_get()));screen_service(-1,0);assert(!display_screen.dimmed);
    assert(!pd_screen_touch(&display_screen,true,true,lv_tick_get()));pd_screen_touch(&display_screen,true,false,lv_tick_get());assert(pd_screen_touch(&display_screen,true,true,lv_tick_get()));
    pd_screen_touch(&display_screen,true,false,lv_tick_get());act(14);act(13);lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed&&dice_animation.active);act(16);
    act(1504);assert(page==STATUS_CONFIRM);lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed);act(1506);
    page=DEVICE_SETTINGS;state.brightness=80;fail_brightness=true;screen_service(-1,0);assert(display_screen.failed&&display_screen.applied==70);screenshot("screen-error.ppm");
    unsigned attempts=brightness_calls;screen_service(-1,0);assert(brightness_calls==attempts);fail_brightness=false;lv_tick_inc(1000);screen_service(-1,0);assert(!display_screen.failed&&display_screen.applied==80);
    state.shortcuts[0]=8;act(850);click("PWR breve: Stato");assert(state.shortcuts[0]==9);shortcut(0);assert(page==DEVICE_SETTINGS);screenshot("screen-configured.ppm");
    /* Explicit local deltas: draft/cancel, atomic confirmation, HP cap, undo and import. */
    pd_demo(&state);page=STATUS_HOME;render();click("Effetti");assert(page==EFFECT_LIST);screenshot("effects-empty.ppm");
    static pd_state effects_before;effects_before=state;unsigned effects_calls=random_calls,effects_saves=save_calls;
    click("Aggiungi");click("+1");click("+1");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("effect-editor.ppm");
    click("Anteprima");assert(page==EFFECT_CONFIRM);screenshot("effect-confirm.ppm");shortcut(0);act(100);assert(page==EFFECT_CONFIRM&&!stage_import(&imported_magic));
    click("Indietro");click("Elenco");assert(!state.effect_count&&save_calls==effects_saves);click("Aggiungi");click("+1");click("+1");click("Anteprima");
    fail_save=true;click("Conferma");assert(page==EFFECT_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");
    assert(page==EFFECT_LIST&&state.armor==16&&pd_armor(&state)==18&&state.effects[0].value==2&&random_calls==effects_calls);
    effects_saves=save_calls;click("1. CA +2 | si");click("Anteprima");click("Conferma");assert(save_calls==effects_saves);
    click("Aggiungi");click("Modifica: CA >");click("+10");click("Anteprima");click("Conferma");assert(state.hp==24&&state.max_hp==32&&pd_max_hp(&state)==42);
    pd_hp(&state,17);assert(state.hp==41);render();screenshot("effects-list.ppm");click("2. PF max +10 | si");click("Attivo: si");click("Anteprima");screenshot("effect-cap.ppm");
    assert(state.hp==41&&scratch.hp==32);click("Conferma");assert(state.hp==32&&pd_max_hp(&state)==32);act(100);click("Annulla");assert(state.hp==41&&pd_max_hp(&state)==42&&pd_armor(&state)==18);screenshot("effects-hero.ppm");
    act(1700);click("2. PF max +10 | si");click("Rimuovi");assert(effect_removing&&page==EFFECT_CONFIRM);click("Indietro");click("Elenco");assert(state.effect_count==2&&state.hp==41);
    click("2. PF max +10 | si");click("Rimuovi");click("Conferma");assert(state.effect_count==1&&state.hp==32&&pd_max_hp(&state)==32);
    /* Invalid totals cannot be confirmed through touch or synthetic actions. */
    state.armor=38;render();click("Aggiungi");click("+1");click("Anteprima");effects_before=state;effects_saves=save_calls;act(1709);assert(page==EFFECT_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("effect-invalid.ppm");
    click("Indietro");click("Elenco");state.armor=16;
    for(unsigned i=state.effect_count;i<PD_EFFECTS;i++)assert(pd_effect_set(&state,i,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=1,.active=true}));
    page=EFFECT_LIST;effect_page=2;render();bounds();screenshot("effects-last.ppm");click("8. PF max +1 | si");assert(effect_index==7);click("Elenco");
    assert(stage_import(&imported_magic)&&pending.effect_count==0);screenshot("effects-import.ppm");click("Annulla");assert(state.effect_count==8);
    state.shortcuts[0]=9;act(850);click("PWR breve: Schermo");assert(state.shortcuts[0]==10);shortcut(0);assert(page==EFFECT_LIST);
    assert(stage_import(&imported_magic));click("Conferma");assert(!state.effect_count&&state.shortcuts[0]==10&&state.max_hp==32&&pd_max_hp(&state)==32);
    /* Explicit round durations: drafts, expiry preview, protected input and atomic undo. */
    pd_demo(&state);page=EFFECT_LIST;effect_page=0;render();click("Aggiungi");click("+1");click("+1");click("Durata");
    assert(page==EFFECT_DURATION&&!state.effect_count);screenshot("duration-manual.ppm");click("Durata: manuale >");click("+1");
    assert(effect_draft.timed&&effect_draft.rounds==2);screenshot("duration-editor.ppm");click("Anteprima");click("Conferma");assert(state.effects[0].rounds==2&&pd_armor(&state)==18);
    click("Aggiungi");click("Modifica: CA >");click("+10");click("Durata");click("Durata: manuale >");click("Anteprima");click("Conferma");
    pd_hp(&state,17);assert(state.hp==41&&pd_max_hp(&state)==42);render();screenshot("duration-list.ppm");effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    lv_tick_inc(120000);screen_service(-1,0);assert(!memcmp(&state,&effects_before,sizeof state));
    click("1 round");assert(page==EFFECT_ROUND&&scratch.hp==32&&scratch.effects[0].rounds==1&&!scratch.effects[1].active);screenshot("duration-round.ppm");
    shortcut(0);act(100);assert(page==EFFECT_ROUND&&!stage_import(&imported_magic)&&save_calls==effects_saves&&random_calls==effects_calls);
    state.dim_timeout=1;pd_screen_activity(&display_screen,lv_tick_get());lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed&&page==EFFECT_ROUND);state.dim_timeout=0;
    click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));click("1 round");fail_save=true;click("Conferma");assert(page==EFFECT_ROUND&&!memcmp(&state,&effects_before,sizeof state));
    click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");
    assert(page==EFFECT_LIST&&state.effects[0].rounds==1&&state.effects[0].active&&!state.effects[1].active&&state.effects[1].timed&&!state.effects[1].rounds&&state.hp==32&&random_calls==effects_calls);
    effects_saves=save_calls;act(1742);assert(save_calls==effects_saves&&state.effects[0].rounds==1);screenshot("duration-expired.ppm");
    act(100);click("Annulla");assert(state.hp==41&&state.effects[0].rounds==2&&state.effects[1].rounds==1&&state.effects[1].active);
    /* Round is configurable and always opens a touch confirmation. */
    state.shortcuts[0]=10;act(850);click("PWR breve: Effetti");assert(state.shortcuts[0]==11);effects_before=state;shortcut(0);assert(page==EFFECT_ROUND&&!memcmp(&state,&effects_before,sizeof state));
    shortcut(0);assert(page==EFFECT_ROUND);click("Conferma");assert(state.hp==32&&state.effects[0].rounds==1);shortcut(0);click("Conferma");assert(!state.effects[0].active&&pd_armor(&state)==16);screenshot("duration-all-expired.ppm");
    effects_saves=save_calls;shortcut(0);assert(page==EFFECT_LIST&&save_calls==effects_saves);
    /* An expired entry cannot be silently re-enabled with a zero timer. */
    click("2. PF max +10 | scaduto");click("Attivo: no");click("Anteprima");effects_before=state;effects_saves=save_calls;act(1709);assert(page==EFFECT_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("duration-zero.ppm");
    click("Indietro");click("Durata");click("+1");click("Anteprima");click("Conferma");assert(state.effects[1].active&&state.effects[1].rounds==1&&state.hp==32);
    /* Paused effects retain time; returning to manual is an explicit choice. */
    click("2. PF max +10 | 1r");click("Attivo: si");click("Anteprima");click("Conferma");effects_before=state;act(1740);assert(page==EFFECT_LIST&&!memcmp(&state,&effects_before,sizeof state));
    click("2. PF max +10 | 1r pausa");click("Durata");click("Durata: round >");click("Anteprima");click("Conferma");assert(!state.effects[1].timed&&!state.effects[1].rounds);
    /* Invalid simultaneous expiry leaves every timer and HP unchanged. */
    pd_demo(&state);assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=true,.timed=true,.rounds=1}));assert(pd_effect_set(&state,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true,.timed=true,.rounds=2}));
    page=EFFECT_LIST;render();click("1 round");effects_before=state;effects_saves=save_calls;act(1742);assert(page==EFFECT_ROUND&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("duration-invalid.ppm");click("Indietro");
    assert(stage_import(&imported_magic));click("Annulla");assert(state.effects[0].rounds==1&&state.effects[1].rounds==2);
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, animated single/double dice, RNG isolation, skip, disabled input, timer cleanup, circular touch bounds >=44px, rest previews/policies, hit dice, rollback, condition notes/poison, death saves, display preferences, idle/wake policy explicit CA/max-HP effects, round durations/expiry and guarded physical buttons passed");
}
