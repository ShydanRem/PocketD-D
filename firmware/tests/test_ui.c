/* Actual firmware UI and LVGL rendered on host. No ESP peripherals simulated as verified. */
#include "lvgl.h"
static void *ui_malloc(size_t size);
#define PD_UI_TEST 1
#define lv_malloc ui_malloc
#include "../main/main.c"
#undef lv_malloc
#include <stdlib.h>
#include <math.h>
static bool fail_save,fail_canvas;
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
    state.training.available=false;page=SHEET;sheet_tab=1;render();bounds();screenshot("skills-legacy.ppm");
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
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, animated single/double dice, RNG isolation, skip, disabled input, timer cleanup, circular touch bounds >=44px passed");
}
