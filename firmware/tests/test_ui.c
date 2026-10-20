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
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, animated single/double dice, RNG isolation, skip, disabled input, timer cleanup, circular touch bounds >=44px passed");
}
