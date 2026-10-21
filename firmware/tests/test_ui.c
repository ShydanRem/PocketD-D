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
        for(unsigned j=i+1;j<lv_obj_get_child_count(root);j++){
            lv_obj_t *other=lv_obj_get_child(root,j);if(!lv_obj_check_type(other,&lv_button_class))continue;
            lv_area_t b;lv_obj_get_coords(other,&b);
            if(!(a.x2<b.x1||a.x1>b.x2||a.y2<b.y1||a.y1>b.y2)){fprintf(stderr,"Buttons overlap: %s / %s\n",lv_label_get_text(button_label(o)),lv_label_get_text(button_label(other)));abort();}
        }
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2};
        for(int x=0;x<2;x++)for(int y=0;y<2;y++)assert((xs[x]-233)*(xs[x]-233)+(ys[y]-233)*(ys[y]-233)<=233*233);
        lv_obj_t *l=button_label(o);lv_area_t text;lv_obj_get_coords(l,&text);
        if(!(text.x1>=a.x1&&text.x2<=a.x2&&text.y1>=a.y1&&text.y2<=a.y2)) { fprintf(stderr,"Overflow %s: button %d,%d,%d,%d text %d,%d,%d,%d\n",lv_label_get_text(l),a.x1,a.y1,a.x2,a.y2,text.x1,text.y1,text.x2,text.y2); abort(); }
    }
}
/* New cost labels must display the complete 40-character source, even wide glyphs. */
static void wrapped_fits(void){
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){
        lv_obj_t *l=lv_obj_get_child(root,i);if(!lv_obj_check_type(l,&lv_label_class)||lv_label_get_long_mode(l)!=LV_LABEL_LONG_WRAP)continue;
        lv_point_t size;lv_text_get_size(&size,lv_label_get_text(l),lv_obj_get_style_text_font(l,0),lv_obj_get_style_text_letter_space(l,0),lv_obj_get_style_text_line_space(l,0),lv_obj_get_content_width(l),LV_TEXT_FLAG_NONE);
        if(size.y>lv_obj_get_content_height(l)){fprintf(stderr,"Wrapped text clipped (%d > %d): %s\n",size.y,(int)lv_obj_get_content_height(l),lv_label_get_text(l));abort();}
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
    click("2. PF max +10 | 1r pausa");click("Durata");click("Durata: round >");click("Durata: inizio turno >");click("Durata: fine turno >");click("Anteprima");click("Conferma");assert(!state.effects[1].timed&&!state.effects[1].rounds);
    /* Invalid simultaneous expiry leaves every timer and HP unchanged. */
    pd_demo(&state);assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=20,.active=true,.timed=true,.rounds=1}));assert(pd_effect_set(&state,1,(pd_effect){.target=PD_EFFECT_ARMOR,.value=-20,.active=true,.timed=true,.rounds=2}));
    page=EFFECT_LIST;render();click("1 round");effects_before=state;effects_saves=save_calls;act(1742);assert(page==EFFECT_ROUND&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("duration-invalid.ppm");click("Indietro");
    assert(stage_import(&imported_magic));click("Annulla");assert(state.effects[0].rounds==1&&state.effects[1].rounds==2);
    /* Incoming typed damage: explicit unknown profiles, preview, retry and undo. */
    pd_demo(&state);state.temp_hp=3;page=STATUS_HOME;render();click("Danni");assert(page==DAMAGE_HOME&&damage_plan.amount==1);screenshot("damage-home.ppm");
    click("+10");click("Anteprima");assert(page==DAMAGE_CONFIRM);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("damage-unknown.ppm");
    click("Configura difesa");assert(page==DEFENCE_EDIT);click("Resistenza: no");click("Anteprima");screenshot("defence-confirm.ppm");shortcut(0);act(100);assert(page==DEFENCE_CONFIRM&&!stage_import(&imported_magic));click("Indietro");assert(!state.defences.configured);click("Anteprima");click("Conferma");
    assert(page==DAMAGE_HOME&&state.defences.configured==1&&pd_defence_flags(&state,0)==PD_RESISTANCE&&state.hp==24&&state.temp_hp==3);
    click("Anteprima");screenshot("damage-preview.ppm");effects_before=state;effects_saves=save_calls;shortcut(0);act(100);assert(page==DAMAGE_CONFIRM&&!stage_import(&imported_magic));
    state.dim_timeout=1;pd_screen_activity(&display_screen,lv_tick_get());lv_tick_inc(60000);screen_service(-1,0);assert(page==DAMAGE_CONFIRM&&!display_screen.dimmed);state.dim_timeout=0;
    click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);click("Anteprima");fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");
    assert(page==DAMAGE_RESULT&&state.hp==22&&!state.temp_hp&&incoming_result.adjusted==5&&random_calls==effects_calls);screenshot("damage-result.ppm");effects_saves=save_calls;act(1805);assert(state.hp==22&&save_calls==effects_saves);
    act(100);click("Annulla");assert(state.hp==24&&state.temp_hp==3&&state.defences.resistance==1);
    /* Concentration uses damage before temporary HP, and requires a manual outcome. */
    state.temp_hp=20;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;
    act(1800);click("+10");click("Anteprima");effects_before=state;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("damage-concentration.ppm");
    click("Esito: da risolvere");fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.hp==24&&state.temp_hp==15&&state.magic.concentration==1);click("Nuovo");click("+10");click("Anteprima");assert(damage_plan.concentration==0);
    click("Esito: da risolvere");click("Esito: mantieni");click("Conferma");assert(!state.magic.concentration&&state.temp_hp==10&&state.hp==24);
    /* Immunity and resistance-rounded-to-zero avoid NVS writes and preserve state. */
    assert(pd_defence_set(&state,0,true,PD_RESISTANCE|PD_IMMUNITY));act(1800);click("+10");click("Anteprima");effects_before=state;effects_saves=save_calls;click("Conferma");assert(page==DAMAGE_RESULT&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("damage-immune.ppm");
    assert(pd_defence_set(&state,0,true,PD_RESISTANCE));act(1800);click("Anteprima");effects_before=state;effects_saves=save_calls;click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);
    /* Profiles are editable without applying a hit; unknown remains distinct from normal. */
    act(1800);click("Difese per tipo");screenshot("defences-list.ppm");click("Contundente | resistenza");screenshot("defence-editor.ppm");click("Vulnerabilita: no");click("Anteprima");click("Conferma");assert(pd_defence_flags(&state,0)==3);click("Indietro");click("+10");click("Anteprima");screenshot("damage-combined.ppm");click("Conferma");assert(incoming_result.adjusted==10&&!state.temp_hp&&state.hp==24);
    click("Nuovo");click("Difese per tipo");click("Contundente | resist. + vulner.");click("Configurato: si");click("Anteprima");click("Conferma");assert(!state.defences.configured);click("Indietro");click("Anteprima");act(1805);assert(page==DAMAGE_CONFIRM);
    click("Configura difesa");click("Configurato: no");click("Anteprima");click("Conferma");assert(state.defences.configured==1&&pd_defence_flags(&state,0)==0);
    /* Massive and already-zero cases block instead of inventing death rules. */
    damage_plan.amount=56;render();click("Anteprima");effects_before=state;effects_saves=save_calls;act(1805);assert(page==DAMAGE_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("damage-massive.ppm");click("Indietro");
    state.hp=0;state.temp_hp=9;damage_plan.amount=1;render();click("Anteprima");effects_before=state;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("damage-zero.ppm");click("Indietro");
    state.hp=1;state.temp_hp=0;state.status.available=true;state.status.failures=2;click("Anteprima");click("Conferma");assert(!state.hp&&!state.status.failures);
    /* Last type, input limits, configurable key and import cancellation. */
    act(1800);click("Difese per tipo");defence_page=4;render();bounds();screenshot("defences-last.ppm");click("Tuono | da configurare");click("Immunita: no");click("Anteprima");click("Conferma");assert(pd_defence_flags(&state,12)==PD_IMMUNITY);click("Indietro");
    for(unsigned i=0;i<12;i++)act(1801);assert(damage_plan.type==12);damage_plan.amount=9999;render();bounds();screenshot("damage-maximum.ppm");
    state.shortcuts[0]=11;act(850);click("PWR breve: Round");assert(state.shortcuts[0]==12);shortcut(0);assert(page==DAMAGE_HOME&&damage_plan.amount==1);effects_before=state;assert(stage_import(&imported_magic)&&!pending.defences.configured);screenshot("defences-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Concentration rolls apply incoming damage atomically before dice animation. */
    pd_demo(&state);state.temp_hp=99;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia concentrata",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;state.training.saves=4;state.training.save_bonus[2]=1;
    assert(pd_defence_set(&state,0,true,0));act(1800);damage_plan.amount=25;render();click("Anteprima");effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    click("Tira COS");assert(page==CONCENTRATION_SETUP&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("concentration-setup.ppm");
    click("Modalita: Base >");click("Modalita: Vantaggio >");click("Modalita: Svantaggio >");click("+");assert(concentration_extra==1);screenshot("concentration-extra.ppm");
    click("Indietro");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);click("Tira COS");click("+");
    shortcut(0);act(100);assert(page==CONCENTRATION_SETUP&&!stage_import(&imported_magic));state.dim_timeout=1;pd_screen_activity(&display_screen,lv_tick_get());lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed);state.dim_timeout=0;
    random_values[0]=random_values[1]=19;click("Tira e applica");assert(page==DICE&&combat_roll==6&&dice_animation.active&&!has_roll&&state.hp==24&&state.temp_hp==74&&state.magic.concentration==1&&result.bonus==6&&random_calls==effects_calls+1&&save_calls==effects_saves+1);screenshot("concentration-flight.ppm");
    state.shortcuts[1]=6;shortcut(1);assert(!dice_animation.active&&has_roll&&random_calls==effects_calls+1);effects_saves=save_calls;shortcut(1);act(13);assert(random_calls==effects_calls+1&&save_calls==effects_saves);screenshot("concentration-success.ppm");click("Riepilogo");assert(page==DAMAGE_RESULT);screenshot("concentration-summary.ppm");
    act(100);click("Annulla");assert(state.temp_hp==99&&state.magic.concentration==1&&state.hp==24);
    /* Save errors freeze the already-drawn roll and all options until retry. */
    act(1800);damage_plan.amount=25;render();click("Anteprima");click("Tira COS");click("Modalita: Base >");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;
    fail_save=true;click("Tira e applica");assert(page==CONCENTRATION_SETUP&&concentration_rolled&&random_calls==effects_calls+2&&!memcmp(&state,&effects_before,sizeof state));screenshot("concentration-save-error.ppm");
    act(1861);act(1862);act(1864);act(1866);act(1806);act(100);shortcut(0);assert(page==CONCENTRATION_SETUP&&concentration_mode==1&&concentration_extra==0&&concentration_result.natural==1&&!stage_import(&imported_magic));
    click("Riprova");assert(random_calls==effects_calls+2&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;random_values[0]=random_values[1]=19;click("Riprova");assert(page==DICE&&combat_roll==6&&result.natural==1&&result.count==2&&!state.magic.concentration&&state.temp_hp==74&&random_calls==effects_calls+2);
    act(16);screenshot("concentration-failure.ppm");click("Riepilogo");act(100);click("Annulla");assert(state.temp_hp==99&&state.magic.concentration==1);
    /* Missing or blocked training preserves manual resolution and extracts no dice. */
    state.training.available=false;act(1800);damage_plan.amount=25;render();click("Anteprima");effects_calls=random_calls;act(1860);assert(page==DAMAGE_CONFIRM&&random_calls==effects_calls);screenshot("concentration-legacy.ppm");click("Esito: da risolvere");click("Conferma");assert(state.magic.concentration==1&&state.temp_hp==74&&random_calls==effects_calls);
    act(100);click("Annulla");state.training.available=true;state.training.blocked=1u<<26;act(1800);damage_plan.amount=25;render();click("Anteprima");act(1860);assert(page==DAMAGE_CONFIRM&&random_calls==effects_calls);click("Indietro");state.training.blocked=0;
    /* Lost canvas must not lose the saved outcome or permit another roll. */
    click("Anteprima");click("Tira COS");fail_canvas=true;click("Tira e applica");assert(page==DICE&&combat_roll==6&&has_roll&&!dice_animation.active&&state.temp_hp==74);fail_canvas=false;effects_calls=random_calls;act(13);assert(random_calls==effects_calls);screenshot("concentration-no-canvas.ppm");
    /* The explicit maximum-30 DC changes the threshold without altering damage. */
    state.hp=state.max_hp=999;state.temp_hp=999;state.magic.concentration=1;act(1800);damage_plan.amount=120;render();click("Anteprima");click("Tira COS");click("CD base: 60 >");assert(concentration_cap30);
    for(unsigned i=0;i<10;i++)click("+");screenshot("concentration-cap30.ppm");random_values[0]=random_values[1]=19;click("Tira e applica");assert(incoming_result.concentration_dc==30&&state.magic.concentration==1&&state.temp_hp==879&&result.total==35);act(16);screenshot("concentration-capped-result.ppm");
    /* Explicit concentration binding: draft only, rollback, full transition and undo. */
    pd_demo(&state);state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Magia concentrata dal nome molto lungo",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.powers[1]=state.magic.powers[0];state.magic.concentration=1;
    page=EFFECT_LIST;effect_page=0;render();click("Aggiungi");click("Modifica: CA >");click("+10");click("Durata");click("Vincolo");
    effects_before=state;effects_calls=random_calls;effects_saves=save_calls;assert(page==EFFECT_LINK&&!effect_draft.concentration);click("Concentrazione: no >");screenshot("link-editor.ppm");assert(effect_draft.concentration==1&&!memcmp(&state,&effects_before,sizeof state));
    click("Anteprima");screenshot("link-confirm.ppm");shortcut(0);act(100);assert(page==EFFECT_CONFIRM&&!stage_import(&imported_magic));fail_save=true;click("Conferma");assert(page==EFFECT_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.effects[0].concentration==1&&pd_max_hp(&state)==42&&random_calls==effects_calls);
    pd_hp(&state,17);assert(state.hp==41);act(836);click("Concentrazione: termina");screenshot("link-stop.ppm");effects_before=state;effects_saves=save_calls;shortcut(0);act(100);assert(page==MAGIC_STOP&&save_calls==effects_saves&&!stage_import(&imported_magic));click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));
    click("Concentrazione: termina");fail_save=true;click("Conferma");assert(page==MAGIC_STOP&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==MAGIC_STATUS&&!state.magic.concentration&&!state.effects[0].active&&!state.effects[0].concentration&&state.hp==32&&pd_max_hp(&state)==32);act(100);click("Annulla");assert(state.hp==41&&state.effects[0].active&&state.effects[0].concentration==1&&state.magic.concentration==1);
    /* Recasting the very same entry ends its old numeric effects. */
    page=MAGIC_DETAIL;power_index=0;cast_level=0;cast_pact=false;render();click("Lancia");assert(page==MAGIC_CONFIRM);screenshot("link-recast.ppm");act(100);assert(page==MAGIC_CONFIRM);click("Conferma");assert(page==MAGIC_DETAIL&&state.magic.concentration==1&&!state.effects[0].active&&!state.effects[0].concentration&&state.hp==32);act(100);click("Annulla");assert(state.hp==41&&state.effects[0].concentration==1);
    /* A failed concentration roll commits the reduced maximum with damage; retry cannot reroll. */
    assert(pd_defence_set(&state,0,true,0));state.temp_hp=10;act(1800);damage_plan.amount=1;render();click("Anteprima");click("Tira COS");random_values[0]=random_values[1]=0;effects_before=state;effects_calls=random_calls;fail_save=true;click("Tira e applica");assert(page==CONCENTRATION_SETUP&&!memcmp(&state,&effects_before,sizeof state)&&hit_candidate.hp==32&&!hit_candidate.effects[0].active&&concentration_damage.ended_effects==1);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.hp==32&&state.temp_hp==9&&!state.effects[0].concentration);act(16);click("Riepilogo");screenshot("link-damage.ppm");act(100);click("Annulla");assert(state.hp==41&&state.temp_hp==10&&state.effects[0].active&&state.effects[0].concentration==1);
    /* Long-rest preview ends bindings before healing; signed HP delta is a loss here. */
    rest_plan=(pd_rest_plan){.long_rest=true};page=REST_PREVIEW;render();bounds();screenshot("link-rest.ppm");assert(scratch.hp==32&&!scratch.effects[0].active);click("Conferma");assert(state.hp==32&&!state.effects[0].concentration);act(100);click("Annulla");assert(state.hp==41&&state.effects[0].concentration==1);
    /* Paused bound effects also lose their link; old/imported effects remain unbound. */
    state.effects[0].active=false;state.hp=32;act(836);click("Concentrazione: termina");click("Conferma");assert(!state.effects[0].concentration&&!state.effects[0].active);
    act(1700);click("Aggiungi");click("Durata");click("Vincolo");assert(page==EFFECT_LINK&&!effect_draft.concentration);act(1751);assert(!effect_draft.concentration);bounds();screenshot("link-unavailable.ppm");
    /* Temporary defence drafts preserve unknown/base separation until confirmation. */
    pd_demo(&state);state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia protettiva dal nome molto lungo",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;
    act(1800);click("Difese per tipo");click(">");click("Fuoco | da configurare");click("Configurato: no");click("A tempo");assert(page==DEFENCE_TEMPORARY);
    click("Resistenza: no");click("Attiva: no");screenshot("temporary-editor.ppm");click("Durata");click("Durata: manuale >");click("+1");screenshot("temporary-duration.ppm");click("Vincolo");click("Concentrazione: no >");screenshot("temporary-link.ppm");
    effects_before=state;effects_calls=random_calls;effects_saves=save_calls;click("Anteprima");assert(page==DEFENCE_CONFIRM&&!state.defences.configured&&scratch.defences.temporary[5].rounds==2&&pd_defence_base_flags(&scratch,5)==0&&pd_defence_flags(&scratch,5)==1);screenshot("temporary-confirm.ppm");
    shortcut(0);act(100);assert(page==DEFENCE_CONFIRM&&!stage_import(&imported_magic)&&random_calls==effects_calls);fail_save=true;click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state)&&page==DEFENCE_CONFIRM);fail_save=false;click("Conferma");assert(page==DEFENCE_LIST&&state.defences.temporary[5].concentration==1&&state.defences.temporary[5].rounds==2&&state.defences.configured==32&&pd_defence_base_flags(&state,5)==0);screenshot("temporary-list.ppm");
    /* Editing only the base must not make additional resistance permanent. */
    effects_saves=save_calls;click("Fuoco | resistenza +T");assert(!defence_flags&&defence_temporary_draft.flags==1);click("Anteprima");click("Conferma");assert(save_calls==effects_saves&&pd_defence_base_flags(&state,5)==0);
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=1,.concentration=1}));state.hp=41;render();effects_before=state;effects_saves=save_calls;click("1 round");assert(page==EFFECT_ROUND&&scratch.hp==32&&scratch.defences.temporary[5].rounds==1);screenshot("temporary-round.ppm");click("Difese");assert(page==ROUND_DEFENCES);screenshot("temporary-round-defences.ppm");
    shortcut(0);act(100);act(1742);assert(page==ROUND_DEFENCES&&!stage_import(&imported_magic)&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));state.dim_timeout=1;pd_screen_activity(&display_screen,lv_tick_get());lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed);state.dim_timeout=0;
    act(1746);assert(round_defence_page==0);click("Riepilogo");fail_save=true;click("Conferma");assert(page==EFFECT_ROUND&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==DEFENCE_LIST&&state.hp==32&&!state.effects[0].active&&state.defences.temporary[5].rounds==1&&state.defences.temporary[5].active&&save_calls==effects_saves+2);
    /* Unified undo restores both domains, timers, bindings and HP together. */
    act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));state.shortcuts[0]=11;shortcut(0);assert(page==EFFECT_ROUND);click("Conferma");shortcut(0);assert(page==EFFECT_ROUND);click("Difese");screenshot("temporary-expiry.ppm");assert(pd_defence_flags(&scratch,5)==0&&scratch.defences.configured==32);click("Riepilogo");click("Conferma");assert(!state.defences.temporary[5].active&&!state.defences.temporary[5].rounds&&pd_defence_flags(&state,5)==0&&state.defences.configured==32);
    /* Damage snapshot plus a failed CON save commits all bound effects at once. */
    pd_effect renewed=state.effects[0];renewed.active=true;renewed.rounds=3;assert(pd_effect_set(&state,0,renewed));pd_defence_temporary renewed_defence=state.defences.temporary[5];renewed_defence.active=true;renewed_defence.rounds=3;assert(pd_defence_temporary_set(&state,5,renewed_defence));state.hp=41;state.temp_hp=20;act(1800);damage_plan.type=5;damage_plan.amount=25;render();click("Anteprima");screenshot("temporary-damage.ppm");click("Tira COS");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(page==CONCENTRATION_SETUP&&!memcmp(&state,&effects_before,sizeof state)&&concentration_damage.adjusted==12&&concentration_damage.ended_effects==2&&!hit_candidate.defences.temporary[5].active&&hit_candidate.hp==32);fail_save=false;click("Riprova");assert(random_calls==effects_calls+1&&state.temp_hp==8&&state.hp==32&&!state.magic.concentration&&!state.effects[0].active&&!state.defences.temporary[5].active&&!state.defences.temporary[5].concentration&&pd_defence_flags(&state,5)==0);act(16);click("Riepilogo");screenshot("temporary-result.ppm");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    act(836);click("Concentrazione: termina");screenshot("temporary-stop.ppm");assert(pd_concentration_effects(&state)==2);click("Indietro");page=MAGIC_DETAIL;power_index=0;render();click("Lancia");click("Conferma");assert(state.magic.concentration==1&&!state.defences.temporary[5].active&&!state.defences.temporary[5].concentration&&!state.effects[0].active);
    /* An additional immunity never silently resolves an unknown base profile. */
    assert(pd_defence_temporary_set(&state,5,(pd_defence_temporary){.flags=4,.active=true,.timed=true,.rounds=1}));edit_defence(5,DEFENCE_LIST);render();click("Configurato: si");click("Anteprima");click("Conferma");assert(!state.defences.configured&&pd_defence_flags(&state,5)==4);act(1800);damage_plan.type=5;render();click("Anteprima");effects_before=state;effects_saves=save_calls;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("temporary-unknown.ppm");
    click("Configura difesa");click("A tempo");click("Immunita: si");click("Anteprima");effects_saves=save_calls;act(1817);assert(page==DEFENCE_CONFIRM&&save_calls==effects_saves);screenshot("temporary-invalid.ppm");click("Indietro");assert(page==DEFENCE_TEMPORARY);click("Resistenza: no");click("Durata");click("-1");click("Anteprima");act(1817);assert(page==DEFENCE_CONFIRM&&save_calls==effects_saves);click("Indietro");assert(page==DEFENCE_DURATION);click("+1");click("Anteprima");click("Conferma");assert(pd_defence_flags(&state,5)==1&&!state.defences.configured);
    /* Import cancellation preserves temporary profiles; confirmed copies clear them. */
    state.effect_count=0;memset(state.effects,0,sizeof state.effects);effects_before=state;assert(stage_import(&imported_magic));screenshot("temporary-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic)&&!pending.defences.temporary[5].active);click("Conferma");assert(!state.defences.temporary[5].active&&!state.defences.temporary[5].flags&&state.shortcuts[0]==11);
    /* All 13 timed defence types paginate, with the 21-source concentration limit. */
    pd_demo(&state);state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;
    for(unsigned i=0;i<13;i++){assert(pd_defence_set(&state,i,true,0));assert(pd_defence_temporary_set(&state,i,(pd_defence_temporary){.flags=1+i%7,.active=true,.timed=true,.rounds=i%2?1:99,.concentration=1}));}
    for(unsigned i=0;i<PD_EFFECTS;i++)assert(pd_effect_set(&state,i,(pd_effect){.value=i%2?-1:1,.active=true,.timed=true,.rounds=1,.concentration=1}));
    act(836);click("Concentrazione: termina");assert(pd_concentration_effects(&state)==21);screenshot("temporary-stop-maximum.ppm");click("Indietro");page=DEFENCE_LIST;defence_page=0;render();click("1 round");click("Difese");screenshot("temporary-round-full.ppm");
    for(unsigned i=0;i<4;i++)click(">");assert(round_defence_page==4);screenshot("temporary-round-last.ppm");act(1746);assert(round_defence_page==4);for(unsigned i=0;i<4;i++)click("<");assert(round_defence_page==0);click("Riepilogo");click("Indietro");assert(state.defences.temporary[12].rounds==99&&state.effects[7].active);
    /* Mixed damage is composed in RAM, grouped by type, then committed once. */
    pd_demo(&state);state.hp=32;state.temp_hp=7;assert(pd_defence_set(&state,0,true,1)&&pd_defence_set(&state,5,true,2));act(1800);damage_plan.amount=3;render();click("Componenti");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Aggiungi");assert(page==DAMAGE_PART_EDIT&&damage_part_draft.type==1);screenshot("mixed-editor.ppm");
    click("Tipo: Perforante >");click("Tipo: Tagliente >");click("Tipo: Acido >");click("Tipo: Freddo >");for(unsigned i=0;i<6;i++)click("+1");click("Usa parte");assert(damage_extra_count==1&&damage_extra[0].type==5&&damage_extra[0].amount==7);
    click("Aggiungi");for(unsigned i=0;i<12;i++)act(1920);for(unsigned i=0;i<3;i++)click("+1");assert(damage_part_draft.type==0);click("Usa parte");screenshot("mixed-parts.ppm");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    click("Anteprima");assert(page==DAMAGE_CONFIRM);screenshot("mixed-preview.ppm");click("Tipi");assert(page==DAMAGE_TYPES&&damage_details.raw==14&&damage_details.count==2&&damage_details.groups[0].amount==7&&damage_details.groups[0].adjusted==3&&damage_details.groups[1].adjusted==14);screenshot("mixed-types.ppm");
    shortcut(0);act(100);act(1805);assert(page==DAMAGE_TYPES&&!stage_import(&imported_magic)&&save_calls==effects_saves);state.dim_timeout=1;pd_screen_activity(&display_screen,lv_tick_get());lv_tick_inc(60000);screen_service(-1,0);assert(!display_screen.dimmed);state.dim_timeout=0;
    click("Riepilogo");click("Indietro");assert(page==DAMAGE_PARTS&&damage_extra_count==2&&!memcmp(&state,&effects_before,sizeof state));click("Anteprima");fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==DAMAGE_RESULT&&incoming_components==3&&incoming_result.adjusted==17&&state.hp==22&&!state.temp_hp&&random_calls==effects_calls&&save_calls==effects_saves+2);screenshot("mixed-result.ppm");effects_saves=save_calls;act(1805);assert(save_calls==effects_saves);click("Tipi");assert(damage_details.total.adjusted==17);click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Canceled/new drafts and primary-row removal do not mutate the character. */
    act(1800);damage_plan.amount=3;render();click("Componenti");click("Aggiungi");click("+10");click("Indietro");assert(!damage_extra_count);click("Aggiungi");for(unsigned i=0;i<12;i++)act(1920);click("+1");click("+1");click("Usa parte");click("Anteprima");click("Tipi");assert(damage_details.count==1&&damage_details.groups[0].amount==6&&damage_details.total.adjusted==3);screenshot("mixed-rounding.ppm");click("Riepilogo");click("Indietro");click("1. Contundente: 3");click("Rimuovi");assert(!damage_extra_count&&damage_plan.amount==3&&page==DAMAGE_PARTS&&!memcmp(&state,&effects_before,sizeof state));
    /* One unknown type blocks the whole event; configuration selects the missing type. */
    pd_demo(&state);assert(pd_defence_set(&state,0,true,0));act(1800);damage_plan.amount=10;render();click("Componenti");click("Aggiungi");for(unsigned i=0;i<4;i++)act(1920);for(unsigned i=0;i<9;i++)click("+1");click("Usa parte");effects_before=state;effects_saves=save_calls;click("Anteprima");act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("mixed-unknown.ppm");click("Tipi");assert(damage_details.missing==32&&!damage_details.total.adjusted&&damage_details.total.hp_after==24);click("Riepilogo");click("Configura difesa");assert(defence_index==5&&page==DEFENCE_EDIT);click("Configurato: no");click("Anteprima");click("Conferma");assert(page==DAMAGE_CONFIRM&&state.hp==24&&state.defences.configured==33);click("Indietro");assert(page==DAMAGE_PARTS&&damage_extra_count==1);
    /* One frozen concentration roll, all impact defences, one candidate and full undo. */
    pd_demo(&state);state.hp=32;state.temp_hp=99;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia concentrata",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_defence_set(&state,0,true,0)&&pd_defence_set(&state,5,true,0));
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));state.hp=41;assert(pd_defence_temporary_set(&state,0,(pd_defence_temporary){.flags=1,.active=true,.concentration=1})&&pd_defence_temporary_set(&state,5,(pd_defence_temporary){.flags=4,.active=true,.concentration=1}));
    act(1800);damage_plan.amount=21;damage_extra_count=2;damage_extra[0]=(pd_damage_component){5,80};damage_extra[1]=(pd_damage_component){0,4};render();click("Componenti");click("Anteprima");click("Tipi");assert(damage_details.raw==105&&damage_details.total.adjusted==12&&damage_details.total.concentration_dc==10);screenshot("mixed-impact.ppm");click("Riepilogo");click("Tira COS");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(page==CONCENTRATION_SETUP&&!memcmp(&state,&effects_before,sizeof state)&&hit_candidate.hp==32&&hit_candidate.temp_hp==87&&concentration_event_report.total.ended_effects==3&&concentration_event_report.groups[1].flags==4);screenshot("mixed-save-error.ppm");
    act(1900);act(1930);act(1861);act(100);shortcut(0);assert(page==CONCENTRATION_SETUP&&concentration_rolled&&!stage_import(&imported_magic));fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.hp==32&&state.temp_hp==87&&!state.magic.concentration&&!state.effects[0].active&&!state.defences.temporary[5].active);act(16);screenshot("mixed-concentration.ppm");click("Riepilogo");click("Tipi");assert(damage_details.groups[0].flags==1&&damage_details.groups[0].adjusted==12&&damage_details.groups[1].flags==4&&!damage_details.groups[1].adjusted&&pd_defence_flags(&state,5)==0);screenshot("mixed-snapshot.ppm");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Six-row limits, both pages, large totals, massive guard and zero-damage no-op. */
    pd_demo(&state);state.defences.configured=8191;state.defences.vulnerability=8191;act(1800);damage_plan.amount=9999;render();click("Componenti");for(unsigned i=0;i<5;i++){click("Aggiungi");damage_part_draft.amount=9999;render();click("Usa parte");}assert(damage_extra_count==5&&damage_part_page==1);screenshot("mixed-parts-last.ppm");act(1901);assert(page==DAMAGE_PARTS&&damage_extra_count==5);click("Anteprima");effects_before=state;effects_saves=save_calls;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("mixed-massive.ppm");click("Tipi");assert(damage_details.raw==59994&&damage_details.total.adjusted==119988&&damage_details.count==6);click(">");assert(damage_type_page==1);screenshot("mixed-types-last.ppm");act(1933);assert(damage_type_page==1);click("<");click("Riepilogo");click("Indietro");
    state.defences.immunity=8191;render();click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Conferma");assert(page==DAMAGE_RESULT&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls&&!incoming_result.adjusted);screenshot("mixed-immune.ppm");click("Nuovo");assert(page==DAMAGE_HOME&&!damage_extra_count&&damage_plan.amount==1);state.shortcuts[0]=12;click("Componenti");click("Aggiungi");click("Usa parte");effects_before=state;shortcut(0);assert(page==DAMAGE_HOME&&!damage_extra_count&&!memcmp(&state,&effects_before,sizeof state));
    /* Configure an explicit nonmagical, non-silvered attack rule on the base. */
    pd_demo(&state);state.hp=32;act(1800);click("Difese per tipo");click("Contundente | da configurare");click("Resistenza: no");click("Regole");assert(page==DEFENCE_RULE&&!defence_rule_temporary);for(unsigned i=0;i<4;i++)act(2004);screenshot("conditional-rule.ppm");assert(defence_conditions[0]==PD_RULE_NON_SILVERED_ATTACK);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Anteprima");assert(page==DEFENCE_RULE_REVIEW&&!rule_review_temporary);screenshot("conditional-review.ppm");
    shortcut(0);act(100);assert(page==DEFENCE_RULE_REVIEW&&!stage_import(&imported_magic));click("Extra");click("Base");for(unsigned i=0;i<3;i++)act(2013);fail_save=true;click("Conferma");assert(page==DEFENCE_RULE_REVIEW&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==DEFENCE_LIST&&state.defences.conditions[0][0]==PD_RULE_NON_SILVERED_ATTACK&&pd_defence_conditional(&state,0)&&!pd_defence_flags(&state,0)&&random_calls==effects_calls);screenshot("conditional-list.ppm");
    /* Unknown facts block without RNG; canceled declarations keep the old source. */
    click("Indietro");click("+10");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls&&save_calls==effects_saves);screenshot("conditional-unknown.ppm");click("Dichiara fonte");assert(page==DAMAGE_CONTEXT&&!source_context_draft.known);screenshot("conditional-source-unknown.ppm");click("Magico: ?");click("Attacco: ?");click("Attacco: no");click("Indietro");assert(page==DAMAGE_CONFIRM&&!damage_context.known);click("Dichiara fonte");click("Magico: ?");click("Attacco: ?");click("Attacco: no");click("Argentato: ?");click("Argentato: no");screenshot("conditional-source-silver.ppm");shortcut(0);act(100);assert(page==DAMAGE_CONTEXT&&!stage_import(&imported_magic));click("Usa fonte");
    pd_damage_event source_event=current_damage_event();pd_damage_event_summary source_report;assert(pd_damage_event_preview(&state,&source_event,&source_report)&&source_report.total.adjusted==11&&!source_report.context_missing&&damage_context.known==7&&damage_context.values==6&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("conditional-excluded.ppm");
    click("Indietro");click("Componenti");click("1. Contundente: 11");click("Fonte");click("Argentato: si");click("Argentato: ?");screenshot("conditional-source.ppm");click("Usa fonte");assert(page==DAMAGE_PART_EDIT&&damage_part_context.values==2&&damage_context.values==6);click("Usa parte");click("Anteprima");source_event=current_damage_event();assert(pd_damage_event_preview(&state,&source_event,&source_report)&&source_report.total.adjusted==5&&source_report.groups[0].flags==1);screenshot("conditional-applied.ppm");click("Conferma");assert(state.hp==27&&state.defences.conditions[0][0]==PD_RULE_NON_SILVERED_ATTACK);act(100);click("Annulla");assert(state.hp==32&&state.defences.conditions[0][0]==PD_RULE_NON_SILVERED_ATTACK);act(1800);assert(!damage_context.known&&!damage_extra_count);
    /* Same type with different source eligibility: snapshot and a single frozen roll. */
    pd_demo(&state);state.hp=32;state.temp_hp=3;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia non inferita",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_defence_set(&state,0,true,1)&&pd_defence_rule_set(&state,0,false,0,PD_RULE_NONMAGICAL));
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));state.hp=41;assert(pd_defence_temporary_set(&state,0,(pd_defence_temporary){.flags=4,.active=true,.concentration=1,.conditions={0,0,PD_RULE_MAGICAL}}));
    act(1800);damage_plan.amount=11;damage_context=(pd_damage_context){1,0};damage_extra_count=2;damage_extra[0]=(pd_damage_component){0,7};damage_extra[1]=(pd_damage_component){0,2};damage_extra_context[0]=(pd_damage_context){1,1};damage_extra_context[1]=(pd_damage_context){1,0};render();click("Anteprima");click("Tipi");assert(damage_details.count==2&&damage_details.total.adjusted==6&&damage_details.groups[0].amount==13&&damage_details.groups[1].flags==4);screenshot("conditional-groups.ppm");click("Riepilogo");click("Tira COS");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(page==CONCENTRATION_SETUP&&!memcmp(&state,&effects_before,sizeof state)&&hit_candidate.hp==32&&!hit_candidate.temp_hp&&concentration_event_report.total.ended_effects==2);act(2021);act(2024);act(2000);act(1900);assert(page==CONCENTRATION_SETUP);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&!state.magic.concentration&&state.hp==32&&!state.defences.temporary[0].active);act(16);click("Riepilogo");click("Tipi");assert(damage_details.groups[1].flags==4&&!damage_details.groups[1].adjusted&&damage_details.groups[0].adjusted==6);screenshot("conditional-snapshot.ppm");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Extra selectors survive duration/link edits and end with their owning grant. */
    pd_demo(&state);state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_defence_set(&state,0,true,0));edit_defence(0,DEFENCE_LIST);render();click("A tempo");click("Immunita: no");click("Attiva: no");click("Regole");assert(defence_rule_temporary&&defence_rule_property==2);click("Condizione: Sempre >");click("Anteprima");assert(page==DEFENCE_RULE_REVIEW&&rule_review_temporary&&rule_review_property==2);screenshot("conditional-extra.ppm");click("Conferma");assert(state.defences.temporary[0].conditions[2]==PD_RULE_MAGICAL);click("Contundente | condizionale +T");click("A tempo");click("Durata");click("Durata: manuale >");click("Vincolo");click("Concentrazione: no >");click("Anteprima");screenshot("conditional-confirm.ppm");click("Regole");click("Extra");for(unsigned i=0;i<2;i++)act(2013);screenshot("conditional-review-extra.ppm");click("Conferma");assert(state.defences.temporary[0].conditions[2]==PD_RULE_MAGICAL&&state.defences.temporary[0].rounds==1&&state.defences.temporary[0].concentration==1);click("1 round");click("Difese");screenshot("conditional-expiry.ppm");click("Riepilogo");click("Conferma");assert(!state.defences.temporary[0].active&&state.defences.temporary[0].conditions[2]==PD_RULE_MAGICAL);
    /* Longest rule descriptions wrap inside the circular confirmation. */
    edit_defence(0,DEFENCE_LIST);render();click("Resistenza: no");click("Regole");for(unsigned i=0;i<6;i++)act(2004);click("Anteprima");bounds();screenshot("conditional-long.ppm");click("Indietro");assert(page==DEFENCE_RULE);click("Indietro");click("Anteprima");click("Regole");click("Conferma");assert(state.defences.conditions[0][0]==PD_RULE_NEITHER_ATTACK);
    effects_before=state;assert(stage_import(&imported_magic));screenshot("conditional-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.defences.conditions[0][0]&&!state.defences.temporary[0].conditions[2]);
    /* Three independent drafts, all included in one confirmed save. */
    pd_demo(&state);state.hp=32;state.temp_hp=3;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_defence_set(&state,5,true,0));edit_defence(5,DEFENCE_LIST);render();effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    click("A tempo");click("Resistenza: no");click("Attiva: no");click("Durata");click("Durata: manuale >");click("Vincolo");click("Concentrazione: no >");click("Indietro");click("Indietro");click("Extra 1 di 3 >");assert(page==DEFENCE_GRANTS);bounds();click("Extra 2 | inattivo >");assert(defence_slot==1);click("Resistenza: no");click("Attiva: no");click("Durata");click("Durata: manuale >");click("+1");click("Indietro");click("Extra 2 di 3 >");click("Extra 3 | inattivo >");click("Immunita: no");click("Attiva: no");click("Regole");click("Condizione: Sempre >");click("Indietro");click("Durata");click("Durata: manuale >");click("+1");click("+1");click("Indietro");bounds();screenshot("multiple-editor.ppm");click("Extra 3 di 3 >");assert(defence_temporary_drafts[0].concentration==1&&defence_temporary_drafts[1].rounds==2&&defence_temporary_drafts[2].rounds==3&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("multiple-picker.ppm");click("Confronta");assert(page==DEFENCE_GRANT_REVIEW&&defence_slot==2);bounds();screenshot("multiple-review.ppm");
    shortcut(0);act(100);assert(page==DEFENCE_GRANT_REVIEW&&!stage_import(&imported_magic)&&!timed_effects());click("Regole");assert(page==DEFENCE_RULE_REVIEW&&rule_review_temporary);screenshot("multiple-rules.ppm");click("Extra");assert(defence_slot==0);click("Extra");assert(defence_slot==1);click("Indietro");assert(page==DEFENCE_GRANT_REVIEW&&defence_slot==1);click("Fuoco #2/3 >");assert(defence_slot==2);fail_save=true;click("Conferma");assert(page==DEFENCE_GRANT_REVIEW&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves+1);fail_save=false;click("Conferma");assert(page==DEFENCE_LIST&&save_calls==effects_saves+2&&random_calls==effects_calls&&active_defences(&state,5)==3&&pd_defence_grant(&state,5,1)->rounds==2&&pd_defence_grant(&state,5,2)->conditions[2]==PD_RULE_MAGICAL);screenshot("multiple-list.ppm");
    effects_before=state;click("1 round");assert(page==EFFECT_ROUND);click("Difese");bounds();screenshot("multiple-round.ppm");assert(!scratch.defences.temporary[5].active&&pd_defence_grant(&scratch,5,1)->rounds==1&&pd_defence_grant(&scratch,5,2)->rounds==2&&active_defences(&scratch,5)==2);click("Riepilogo");click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Nonmagical hit resolves two equal resistances once; only the linked one ends. */
    act(1800);damage_plan.type=5;damage_plan.amount=11;damage_context=(pd_damage_context){1,0};render();click("Anteprima");click("Tira COS");effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&concentration_damage.adjusted==5&&concentration_damage.ended_effects==1&&!hit_candidate.defences.temporary[5].active&&pd_defence_grant(&hit_candidate,5,1)->active&&pd_defence_grant(&hit_candidate,5,2)->active&&hit_candidate.hp==30);act(2030);act(2031);act(2036);assert(page==CONCENTRATION_SETUP);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&!state.magic.concentration&&active_defences(&state,5)==2);act(16);click("Riepilogo");bounds();screenshot("multiple-impact.ppm");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    act(1800);act(1802);click("1 round");click("Conferma");assert(active_defences(&state,5)==2);click("1 round");click("Difese");bounds();screenshot("multiple-surviving.ppm");click("Riepilogo");click("Conferma");assert(!pd_defence_grant(&state,5,1)->active&&pd_defence_grant(&state,5,2)->active&&pd_defence_grant(&state,5,2)->rounds==1);click("1 round");click("Conferma");assert(!active_defences(&state,5)&&!timed_effects());
    /* An invalid sibling blocks the common save even while reviewing a valid grant. */
    edit_defence(5,DEFENCE_LIST);render();click("A tempo");click("Extra 1 di 3 >");click("Extra 2 | inattivo >");click("Attiva: no");click("Anteprima");assert(page==DEFENCE_CONFIRM&&!preview_defence(&scratch));click("Regole");click("Extra");assert(defence_slot==1);effects_before=state;effects_saves=save_calls;act(1817);assert(page==DEFENCE_RULE_REVIEW&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);click("Indietro");click("Indietro");click("Base");click("Indietro");assert(page==DEFENCE_LIST&&!memcmp(&state,&effects_before,sizeof state));
    /* Cancel/confirm import, and last page uses the high bits of all 39 timers. */
    pd_demo(&state);assert(!state.defences.configured);assert(pd_defence_grant_set(&state,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=99}));effects_before=state;assert(local_defences());act(100);assert(stage_import(&imported_magic));screenshot("multiple-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(!pd_defence_grant(&state,type,slot)->flags);
    pd_demo(&state);state.defences.configured=8191;for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&state,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=99}));state.shortcuts[0]=11;shortcut(0);assert(page==EFFECT_ROUND);bounds();screenshot("multiple-full-round.ppm");click("Difese");for(unsigned i=0;i<12;i++)click(">");assert(round_defence_page==12);bounds();screenshot("multiple-last.ppm");act(1746);assert(round_defence_page==12);click("Riepilogo");click("Conferma");for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant(&state,type,slot)->rounds==98);
    /* Explicit order, canceled drafts, math detail, and one confirmed HP/temp save. */
    pd_demo(&state);state.hp=32;state.temp_hp=3;assert(pd_defence_set(&state,0,true,1));act(1800);damage_plan.amount=11;render();effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Riduci");assert(page==DAMAGE_REDUCTION&&reduction_target==PD_DAMAGE_COMPONENTS);for(unsigned i=0;i<3;i++)click("+1");click("Dopo: intero colpo >");for(unsigned i=0;i<3;i++)click("+1");bounds();screenshot("reduction-before.ppm");shortcut(0);act(100);assert(page==DAMAGE_REDUCTION&&!stage_import(&imported_magic));click("Indietro");assert(!damage_reduction_after&&!damage_reductions[0]&&!memcmp(&state,&effects_before,sizeof state));
    click("Riduci");for(unsigned i=0;i<3;i++)click("+1");screenshot("reduction-after.ppm");click("Dopo: intero colpo >");for(unsigned i=0;i<3;i++)click("+1");click("Usa valori");assert(damage_reduction_after==3&&damage_reductions[0]==3&&save_calls==effects_saves&&random_calls==effects_calls);click("Anteprima");screenshot("reduction-preview.ppm");click("Tipi");click("Conto");assert(page==DAMAGE_MATH&&damage_details.raw==11&&damage_details.reduction_before==3&&damage_details.mitigated==4&&damage_details.reduction_after==3&&damage_details.total.adjusted==1);bounds();screenshot("reduction-math.ppm");shortcut(0);act(100);act(2040);assert(page==DAMAGE_MATH&&!stage_import(&imported_magic));click("Indietro");click("Riepilogo");fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==32&&state.temp_hp==2&&save_calls==effects_saves+2&&random_calls==effects_calls);screenshot("reduction-result.ppm");click("Tipi");click("Conto");assert(damage_details.reduction_after==3&&damage_details.total.adjusted==1);click("Indietro");click("Riepilogo");click("Nuovo");assert(!damage_reduction_after&&!damage_reductions[0]);act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* A part reduction stays in its draft until Usa parte; promotion keeps its own value. */
    act(1800);damage_plan.amount=11;render();click("Componenti");click("1. Contundente: 11");click("Riduci");assert(reduction_return==DAMAGE_PART_EDIT);act(2042);assert(reduction_target==0);for(unsigned i=0;i<3;i++)click("+1");click("Usa valori");assert(damage_part_reduction==3&&!damage_reductions[0]);click("Indietro");assert(!damage_reductions[0]);click("1. Contundente: 11");click("Riduci");for(unsigned i=0;i<3;i++)click("+1");click("Usa valori");click("Usa parte");assert(damage_reductions[0]==3);click("Aggiungi");assert(!damage_part_reduction);click("+10");click("Riduci");for(unsigned i=0;i<5;i++)click("+1");click("Usa valori");click("Usa parte");assert(damage_reductions[1]==5&&damage_extra_count==1);assert(pd_defence_set(&state,1,true,0));click("Indietro");click("Riduci");for(unsigned i=0;i<4;i++)click("+1");click("Usa valori");click("Anteprima");click("Tipi");assert(damage_details.count==2&&damage_details.mitigated==10&&damage_details.total.adjusted==6&&damage_details.reduction_before==8&&damage_details.reduction_after==4);screenshot("reduction-groups.ppm");click("Conto");bounds();screenshot("reduction-mixed.ppm");click("Indietro");click("Riepilogo");click("Indietro");click("Componenti");click("1. Contundente: 11");click("Rimuovi");assert(!damage_extra_count&&damage_reductions[0]==5&&!damage_reductions[1]&&damage_reduction_after==4);click("Aggiungi");assert(!damage_part_reduction);click("Usa parte");assert(!damage_reductions[1]);click("Indietro");
    /* Limits clamp the request; unknown source/profile cannot be bypassed by reducing to zero. */
    pd_demo(&state);state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;act(1800);damage_plan.amount=11;render();click("Riduci");reduction_drafts[PD_DAMAGE_COMPONENTS]=9998;render();click("+10");assert(reduction_drafts[PD_DAMAGE_COMPONENTS]==9999);act(2046);assert(reduction_drafts[PD_DAMAGE_COMPONENTS]==9999);click("Dopo: intero colpo >");reduction_drafts[0]=9998;render();click("+10");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);assert(pd_defence_set(&state,0,true,1)&&pd_defence_rule_set(&state,0,false,0,PD_RULE_NONMAGICAL));render();click("Tipi");click("Conto");assert(!damage_details.total.configured&&!damage_details.mitigated);screenshot("reduction-unknown.ppm");click("Indietro");click("Riepilogo");click("Dichiara fonte");click("Magico: ?");click("Usa fonte");click("Tipi");click("Conto");assert(damage_details.total.allowed&&!damage_details.total.adjusted&&damage_details.reduction_before==11&&!damage_details.reduction_after&&damage_details.requested_after==9999);screenshot("reduction-zero.ppm");click("Indietro");click("Riepilogo");effects_before=state;click("Conferma");assert(page==DAMAGE_RESULT&&!memcmp(&state,&effects_before,sizeof state)&&state.magic.concentration==1&&save_calls==effects_saves&&random_calls==effects_calls);click("Nuovo");assert(!damage_reduction_after&&!damage_reductions[0]);click("Riduci");reduction_drafts[PD_DAMAGE_COMPONENTS]=1;render();click("-10");act(2043);assert(!reduction_drafts[PD_DAMAGE_COMPONENTS]);click("Indietro");
    /* Reduced CON DC and immutable full calculation survive a failed save and retry. */
    pd_demo(&state);state.hp=32;state.temp_hp=3;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_defence_set(&state,0,true,0)&&pd_defence_temporary_set(&state,0,(pd_defence_temporary){.flags=1,.active=true,.concentration=1})&&pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1}));state.hp=41;act(1800);damage_plan.amount=51;render();click("Riduci");click("+1");click("+1");click("Dopo: intero colpo >");for(unsigned i=0;i<3;i++)click("+1");click("Usa valori");click("Anteprima");click("Tira COS");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&concentration_damage.adjusted==22&&concentration_damage.concentration_dc==11&&concentration_damage.ended_effects==2&&concentration_event_report.mitigated==24&&hit_candidate.hp==22&&!hit_candidate.temp_hp&&!hit_candidate.magic.concentration);screenshot("reduction-retry.ppm");act(2040);act(2041);act(2045);act(2048);act(2050);act(1800);assert(page==CONCENTRATION_SETUP&&damage_reductions[0]==3&&damage_reduction_after==2);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.hp==22&&!pd_defence_flags(&state,0));act(16);click("Riepilogo");click("Tipi");assert(damage_details.groups[0].flags==1&&damage_details.groups[0].reduction==3&&damage_details.groups[0].adjusted==24);click("Conto");assert(damage_details.mitigated==24&&damage_details.reduction_after==2&&damage_details.total.adjusted==22);screenshot("reduction-snapshot.ppm");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Canceling import preserves event values; a confirmed replacement clears them. */
    effects_before=state;assert(stage_import(&imported_magic));click("Annulla");assert(damage_reductions[0]==3&&damage_reduction_after==2&&!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!damage_reductions[0]&&!damage_reduction_after);state.shortcuts[0]=12;damage_reductions[0]=3;damage_reduction_after=2;shortcut(0);assert(page==DAMAGE_HOME&&!damage_reductions[0]&&!damage_reduction_after);
    /* Maximum counts/amounts and six same-type allocations remain legible. */
    pd_demo(&state);state.defences.configured=8191;act(1800);damage_plan.amount=9999;damage_extra_count=5;for(unsigned i=0;i<5;i++)damage_extra[i]=(pd_damage_component){0,9999};for(unsigned i=0;i<6;i++)damage_reductions[i]=9999;damage_reduction_after=9999;render();click("Riduci");for(unsigned i=0;i<6;i++)act(2042);assert(reduction_target==5);bounds();screenshot("reduction-last.ppm");click("Usa valori");click("Anteprima");click("Tipi");assert(damage_details.count==1&&damage_details.raw==59994&&damage_details.groups[0].reduction==59994);bounds();screenshot("reduction-maximum-group.ppm");click("Conto");bounds();screenshot("reduction-maximum.ppm");click("Indietro");click("Riepilogo");click("Indietro");memset(damage_reductions,0,sizeof damage_reductions);for(unsigned i=0;i<6;i++)assert(pd_defence_set(&state,i,true,2));for(unsigned i=0;i<5;i++)damage_extra[i]=(pd_damage_component){i+1,9999};render();click("Anteprima");click("Tipi");click("Conto");assert(damage_details.mitigated==119988&&damage_details.total.adjusted==109989);bounds();screenshot("reduction-maximum-damage.ppm");
    /* Configure a reusable shared budget, condition, timer and binding through the real UI. */
    click("Indietro");click("Riepilogo");click("Indietro");pd_demo(&state);state.hp=32;state.temp_hp=3;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;state.defences.configured=8191;act(1800);damage_plan.amount=11;damage_extra_count=1;damage_extra[0]=(pd_damage_component){5,11};render();click("Riduci");click("Profili");assert(page==PROFILE_LIST);bounds();screenshot("profile-list.ppm");click("Profilo 1 | inattivo >");click("Modifica");click("Quantita: 0 >");for(unsigned i=0;i<3;i++)click("+1");click("Indietro");click("Tipi: Nessuno >");click("Tipi: Tutti >");click("Attivo: no >");click("Dettagli");for(unsigned i=0;i<3;i++)act(2076);click("Durata: manuale >");click("Round residui: 1 >");click("+1");click("Indietro");click("Concentrazione: no >");bounds();screenshot("profile-details.ppm");click("Anteprima");assert(page==PROFILE_CONFIRM&&profile_draft.rule==PD_RULE_NONMAGICAL_ATTACK&&profile_draft.types==7&&profile_draft.rounds==2);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_CONFIRM&&!stage_import(&imported_magic));bounds();screenshot("profile-confirm.ppm");click("Dati >");screenshot("profile-rule.ppm");click("Regola >");screenshot("profile-duration.ppm");fail_save=true;click("Conferma");assert(page==PROFILE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==PROFILE_LIST&&state.reductions[0].value==3&&state.reductions[0].types==7&&state.reductions[0].concentration==1&&save_calls==effects_saves+2&&random_calls==effects_calls);
    /* Profile selection belongs to the outer impact draft; cancel preserves the saved definition. */
    click("Profilo 1 | attivo >");bounds();screenshot("profile-view.ppm");click("Usa");click("+10");assert(allocation_sum()==3);act(2084);assert(allocation_sum()==3);screenshot("profile-allocation.ppm");click("Usa profilo");assert(reduction_profile_draft==1&&!damage_profile);click("Indietro");assert(!damage_profile&&state.reductions[0].active);
    /* An excluded row blocks; move its allocation without multiplying the budget. */
    click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("+1");click("+1");act(2089);click("+1");assert(allocation_sum()==3);click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("profile-excluded.ppm");click("Indietro");click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");act(2089);click("-1");act(2089);click("+1");assert(allocation_draft[0]==3&&!allocation_draft[1]);click("Usa profilo");click("Usa valori");click("Anteprima");click("Dichiara fonte");click("Magico: ?");click("Attacco: ?");click("Attacco: no");click("Usa fonte");click("Tipi");click("Conto");assert(damage_details.total.adjusted==19&&damage_details.profile_applied==3&&damage_details.profile_count==2);screenshot("profile-before-math.ppm");click("Profilo");bounds();screenshot("profile-impact.ppm");assert(profile_impact_row==0);act(2089);assert(profile_impact_row==1);click("Indietro");click("Indietro");click("Riepilogo");
    /* One frozen roll ends the selected bound profile, retaining the original account. */
    click("Tira COS");effects_before=state;effects_calls=random_calls;random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&concentration_event_report.profile_snapshot.active&&concentration_event_report.profile_applied==3&&concentration_damage.ended_effects==1&&hit_candidate.hp==16&&!hit_candidate.reductions[0].active);act(2060);act(2072);act(2088);assert(page==CONCENTRATION_SETUP);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.hp==16&&!state.reductions[0].active&&!state.reductions[0].concentration);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");assert(damage_details.profile_snapshot.active&&damage_details.profile_applied==3);screenshot("profile-snapshot.ppm");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    state.shortcuts[0]=11;shortcut(0);click("Difese");bounds();screenshot("profile-round.ppm");assert(scratch.reductions[0].rounds==1);click("Riepilogo");click("Conferma");shortcut(0);click("Difese");screenshot("profile-expiry.ppm");assert(!scratch.reductions[0].active);click("Riepilogo");click("Conferma");assert(!state.reductions[0].active&&!state.reductions[0].rounds&&state.reductions[0].rule==PD_RULE_NONMAGICAL_ATTACK);
    /* A post profile is applied once alongside a separate manual subtraction. */
    act(1800);damage_plan.amount=11;damage_extra_count=1;damage_extra[0]=(pd_damage_component){5,11};render();click("Riduci");click("+1");click("+1");click("Profili");click("Profilo 2 | inattivo >");click("Modifica");click("Quantita: 0 >");for(unsigned i=0;i<3;i++)click("+1");click("Indietro");click("Tipi: Nessuno >");click("Fase: prima >");click("Attivo: no >");click("Anteprima");click("Conferma");click("Profilo 2 | attivo >");click("Usa");assert(state.reductions[1].stage==1);act(2084);assert(!allocation_sum());bounds();screenshot("profile-after.ppm");click("Usa profilo");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");assert(damage_details.mitigated==22&&damage_details.reduction_after==5&&damage_details.profile_applied==3&&damage_details.total.adjusted==17);screenshot("profile-math.ppm");click("Indietro");click("Riepilogo");click("Indietro");click("Riduci");click("Profili");click("Nessuno");click("Indietro");assert(damage_profile==2);click("Riduci");click("Profili");click("Nessuno");click("Usa valori");assert(!damage_profile&&!damage_allocation[0]&&damage_reduction_after==2);
    /* Import guards include profiles alone, and all 42 high timer bits are visible. */
    pd_demo(&state);assert(pd_reduction_profile_set(&state,2,(pd_reduction_profile){.value=3,.types=8191,.active=true}));assert(local_defences());effects_before=state;act(100);assert(stage_import(&imported_magic));screenshot("profile-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");for(unsigned i=0;i<3;i++)assert(!state.reductions[i].value);assert(!damage_profile);
    /* Invalid definitions never save; longest rule, zero rounds and limits are reviewable. */
    pd_demo(&state);act(1800);click("Riduci");click("Profili");click("Profilo 3 | inattivo >");click("Modifica");click("Attivo: no >");click("Anteprima");effects_before=state;effects_saves=save_calls;act(2088);assert(page==PROFILE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("profile-invalid.ppm");click("Indietro");click("Quantita: 0 >");profile_draft.value=9998;render();click("+10");act(2084);assert(profile_draft.value==9999);click("Indietro");click("Tipi: Nessuno >");click("Dettagli");for(unsigned i=0;i<6;i++)act(2076);click("Durata: manuale >");click("Round residui: 1 >");click("-10");assert(!profile_draft.rounds);click("Indietro");click("Anteprima");assert(!preview_profile(&scratch));act(2088);assert(page==PROFILE_CONFIRM&&save_calls==effects_saves);click("Indietro");click("Dettagli");click("Round residui: 0 >");profile_draft.rounds=98;render();click("+10");act(2084);assert(profile_draft.rounds==99);click("Indietro");click("Anteprima");click("Dati >");bounds();screenshot("profile-long.ppm");click("Conferma");assert(state.reductions[2].rounds==99&&state.reductions[2].rule==PD_RULE_NEITHER_ATTACK);click("Indietro");click("Indietro");
    pd_demo(&state);state.defences.configured=8191;for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&state,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=99}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&state,i,(pd_reduction_profile){.value=9999,.types=8191,.active=true,.timed=true,.rounds=99}));state.shortcuts[0]=11;shortcut(0);click("Difese");for(unsigned i=0;i<13;i++)click(">");assert(round_defence_page==13);bounds();screenshot("profile-last-round.ppm");act(1746);assert(round_defence_page==13);click("Riepilogo");click("Conferma");assert(state.reductions[2].rounds==98);
    /* Configure a counter cost and reaction with a full before/after confirmation. */
    pd_demo(&state);state.hp=32;state.temp_hp=3;state.defences.configured=8191;state.resources[0].maximum=4;state.resources[0].current=3;state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Magia",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.powers[1]=(pd_power){.name="Usi sintetici",.kind=1,.maximum=3,.remaining=2,.recovery=1};state.magic.concentration=1;assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.timed=true,.rounds=2,.concentration=1}));act(1800);damage_plan.amount=31;render();click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: nessuna >");click("Costo: 1 >");click("+1");click("Indietro");click("Usa reazione: no >");bounds();screenshot("cost-editor.ppm");click("Indietro");click("Anteprima");for(unsigned i=0;i<3;i++)act(2086);assert(profile_confirm_tab==3);bounds();screenshot("cost-confirm.ppm");effects_before=state;effects_saves=save_calls;for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_CONFIRM&&!stage_import(&imported_magic));fail_save=true;click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.reductions[0].cost==2&&state.reductions[0].reaction&&state.resources[0].current==3&&!state.reaction_spent&&save_calls==effects_saves+2);
    click("Profilo 1 | attivo >");click("Costi");effects_before=state;act(2101);act(2103);assert(!memcmp(&state,&effects_before,sizeof state)&&!profile_cost_editing);screenshot("cost-view.ppm");click("Indietro");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(page==PROFILE_COST_IMPACT&&damage_details.cost_before==3&&damage_details.cost_after==1&&!damage_details.reaction_before&&damage_details.reaction_after);bounds();screenshot("cost-preview.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_COST_IMPACT&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");
    /* The frozen CON candidate charges exactly once and Undo restores everything. */
    click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.hp==7&&!hit_candidate.temp_hp&&hit_candidate.resources[0].current==1&&hit_candidate.reaction_spent&&!hit_candidate.magic.concentration&&!hit_candidate.reductions[0].active&&concentration_event_report.cost_after==1);screenshot("cost-retry.ppm");act(2117);act(2122);act(2101);act(2103);assert(page==CONCENTRATION_SETUP&&state.resources[0].current==3&&!state.reaction_spent);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.resources[0].current==1&&state.reaction_spent&&state.hp==7);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_before==3&&damage_details.cost_after==1&&damage_details.profile_snapshot.reaction&&damage_details.profile_snapshot.active);screenshot("cost-snapshot.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Explicit selection pays even if a separate manual reduction makes damage zero. */
    act(1800);click("Riduci");click("+1");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_calls=random_calls;effects_saves=save_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(!damage_details.total.adjusted&&!damage_details.profile_applied&&damage_details.cost_after==1);screenshot("cost-zero.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==32&&state.temp_hp==3&&state.magic.concentration==1&&state.resources[0].current==1&&state.reaction_spent&&random_calls==effects_calls&&save_calls==effects_saves+1);
    /* Recovering reaction from a blocked preview preserves the prepared impact. */
    click("Nuovo");click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("cost-blocked.ppm");click("Gestisci reazione");assert(reaction_return==DAMAGE_CONFIRM);bounds();screenshot("reaction-used.ppm");click("Inizio turno");screenshot("reaction-turn.ppm");click("Indietro");assert(state.reaction_spent);click("Inizio turno");fail_save=true;click("Conferma");assert(page==REACTION_CONFIRM&&state.reaction_spent&&state.resources[0].current==1);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==REACTION_CONFIRM&&!stage_import(&imported_magic));fail_save=false;click("Conferma");assert(!state.reaction_spent&&state.resources[0].current==1);screenshot("reaction-ready.ppm");effects_saves=save_calls;click("Inizio turno");click("Conferma");assert(save_calls==effects_saves);click("Indietro");assert(page==DAMAGE_CONFIRM&&damage_profile==1&&damage_plan.amount==1);pd_damage_event cost_event=current_damage_event();pd_damage_event_summary cost_report;assert(pd_damage_event_preview(&state,&cost_event,&cost_report)&&cost_report.cost_missing&&!cost_report.reaction_missing);click("Indietro");
    /* Switch to a limited-use pool; long names and zero balances remain explicit. */
    strcpy(state.magic.powers[1].name,"Nome sintetico di quaranta caratteri ABC");click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: contatore 1 >");assert(profile_draft.cost_kind==PD_COST_POWER&&profile_draft.cost_index==1);click("Costo: 1 >");click("+1");click("Indietro");bounds();screenshot("cost-power.ppm");click("Indietro");click("Anteprima");for(unsigned i=0;i<3;i++)act(2086);bounds();screenshot("cost-long.ppm");click("Conferma");click("Indietro");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_before==2&&!damage_details.cost_after&&damage_details.profile_snapshot.cost_kind==PD_COST_POWER);screenshot("cost-power-preview.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Conferma");assert(!state.magic.powers[1].remaining&&state.reaction_spent&&state.resources[0].current==1);
    /* External reaction annotation never consumes another pool; reset/round are separate. */
    click("Stato");click("Reazione");click("Inizio turno");click("Conferma");effects_before=state;click("Usa reazione");click("Conferma");assert(state.reaction_spent&&state.resources[0].current==effects_before.resources[0].current&&state.magic.powers[1].remaining==effects_before.magic.powers[1].remaining);screenshot("reaction-external.ppm");click("Indietro");assert(page==STATUS_HOME);assert(pd_effect_set(&state,0,(pd_effect){.value=1,.active=true,.timed=true,.rounds=1}));click("Effetti");click("1 round");click("Conferma");assert(state.reaction_spent&&state.resources[0].current==1&&!state.magic.powers[1].remaining);
    /* Import cancellation preserves costs/reaction; confirmed replacement clears both. */
    effects_before=state;act(100);assert(stage_import(&imported_magic));screenshot("cost-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.reaction_spent&&!state.reductions[0].cost_kind&&!state.reductions[0].reaction);state.shortcuts[0]=8;shortcut(0);assert(page==STATUS_HOME);click("Reazione");assert(page==REACTION_HOME&&!state.reaction_spent);
    /* A reaction-only profile needs no pool; the empty selector terminates at None. */
    click("Indietro");pd_demo(&state);state.resource_count=0;act(1800);click("Riduci");click("Profili");click("Profilo 1 | inattivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: nessuna >");assert(!profile_draft.cost_kind&&!profile_draft.cost);click("Usa reazione: no >");click("Indietro");click("Anteprima");assert(preview_profile(&scratch));click("Conferma");assert(state.reductions[0].reaction&&local_defences());click("Indietro");click("Indietro");
    /* Zero cost with a selected pool is invalid; the editor clamps at its maximum. */
    pd_demo(&state);state.resources[0].maximum=4;state.resources[0].current=1;act(1800);click("Riduci");click("Profili");click("Profilo 1 | inattivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: nessuna >");click("Costo: 1 >");click("-10");click("Indietro");click("Indietro");click("Anteprima");assert(!preview_profile(&scratch));effects_saves=save_calls;act(2088);assert(page==PROFILE_CONFIRM&&save_calls==effects_saves);click("Indietro");click("Dettagli");click("Costi");click("Costo: 0 >");click("+10");assert(profile_draft.cost==4);act(2084);assert(profile_draft.cost==4);click("Indietro");click("Indietro");click("Anteprima");assert(preview_profile(&scratch));click("Conferma");assert(state.reductions[0].cost==4&&state.resources[0].current==1&&local_defences());click("Indietro");click("Indietro");
    /* Worst-width names fit the editor, before/after comparison and frozen expense. */
    state.magic.count=1;state.magic.powers[0]=(pd_power){.kind=1,.maximum=4,.remaining=4};memset(state.magic.powers[0].name,'W',40);state.magic.powers[0].name[40]=0;assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.cost_kind=2,.cost=4}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | inattivo >");click("Costi");wrapped_fits();screenshot("cost-wide.ppm");click("Indietro");click("Modifica");click("Dettagli");click("Costi");wrapped_fits();click("Indietro");click("Anteprima");for(unsigned i=0;i<3;i++)act(2086);wrapped_fits();screenshot("cost-wide-confirm.ppm");click("Indietro");click("Indietro");click("Indietro");click("Indietro");click("Indietro");
    state.defences.configured=8191;state.resources[0].current=4;assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.cost_kind=2,.cost=4}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");wrapped_fits();screenshot("cost-wide-preview.ppm");
    /* Slot sources are explicit, fixed-level pools; ordinary, pact and exhausted levels differ. */
    click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Indietro");pd_demo(&state);state.resource_count=0;state.hp=32;state.temp_hp=3;state.defences.configured=8191;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;state.magic.slots[0]=(pd_slot){2,4};state.magic.slots[8]=(pd_slot){0,1};state.magic.pact[4]=(pd_slot){2,2};assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.timed=true,.rounds=2,.concentration=1}));act(1800);damage_plan.amount=31;render();click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: nessuna >");assert(profile_draft.cost_kind==PD_COST_SLOT&&!profile_draft.cost_index);click("Costo: 1 >");click("+1");click("Indietro");click("Usa reazione: no >");wrapped_fits();screenshot("slot-cost-editor.ppm");click("Indietro");click("Anteprima");for(unsigned i=0;i<3;i++)act(2086);wrapped_fits();screenshot("slot-cost-confirm.ppm");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_CONFIRM&&!stage_import(&imported_magic));fail_save=true;click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.reductions[0].cost_kind==PD_COST_SLOT&&state.reductions[0].cost==2&&state.magic.slots[0].current==2&&state.magic.pact[4].current==2&&save_calls==effects_saves+2&&random_calls==effects_calls);
    click("Profilo 1 | attivo >");click("Costi");effects_before=state;act(2101);act(2103);assert(!memcmp(&state,&effects_before,sizeof state));screenshot("slot-cost-view.ppm");click("Indietro");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_calls=random_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_before==2&&!damage_details.cost_after&&!strcmp(damage_details.cost_name,"Slot ordinario livello 1"));wrapped_fits();screenshot("slot-cost-preview.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");
    /* One frozen COS roll charges only the chosen ordinary pool, including retry and Undo. */
    click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.hp==7&&!hit_candidate.temp_hp&&!hit_candidate.magic.slots[0].current&&hit_candidate.magic.pact[4].current==2&&hit_candidate.reaction_spent&&!hit_candidate.magic.concentration&&!hit_candidate.reductions[0].active&&!memcmp(&state,&effects_before,sizeof state));screenshot("slot-cost-retry.ppm");act(2101);act(2122);assert(page==CONCENTRATION_SETUP);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&!state.magic.slots[0].current&&state.magic.pact[4].current==2);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_before==2&&!damage_details.cost_after&&!strcmp(damage_details.cost_name,"Slot ordinario livello 1"));screenshot("slot-cost-snapshot.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* An exhausted ninth-level pool remains selectable but blocks; no implicit substitution. */
    act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: slot 1 >");assert(profile_draft.cost_kind==PD_COST_SLOT&&profile_draft.cost_index==8&&profile_draft.cost==1);screenshot("slot-cost-ninth.ppm");click("Costo: 1 >");act(2084);assert(profile_draft.cost==1);click("Indietro");click("Indietro");click("Anteprima");click("Conferma");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("slot-cost-blocked.ppm");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_missing&&!strcmp(damage_details.cost_name,"Slot ordinario livello 9")&&!damage_details.cost_before&&!damage_details.cost_after);screenshot("slot-cost-empty.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Indietro");
    /* Switching to pact never replaces it with ordinary slots; zero damage still pays. */
    click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: slot 9 >");assert(profile_draft.cost_kind==PD_COST_PACT&&profile_draft.cost_index==4&&profile_draft.cost==1);click("Costo: 1 >");click("+1");act(2084);assert(profile_draft.cost==2);click("Indietro");wrapped_fits();screenshot("slot-cost-pact.ppm");click("Indietro");click("Anteprima");click("Conferma");click("Indietro");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(!damage_details.total.adjusted&&damage_details.cost_before==2&&!damage_details.cost_after&&!strcmp(damage_details.cost_name,"Slot patto livello 5"));screenshot("slot-cost-zero.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");effects_calls=random_calls;effects_before=state;fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);fail_save=false;click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==32&&state.temp_hp==3&&state.reaction_spent&&!state.magic.pact[4].current&&state.magic.slots[0].current==2&&random_calls==effects_calls);
    /* Existing rest UI recovers pact on short rest, ordinary on long; neither recovers reaction. */
    act(900);click("Riposo breve");assert(scratch.magic.pact[4].current==2&&scratch.magic.slots[0].current==2&&scratch.reaction_spent);screenshot("slot-cost-short-rest.ppm");effects_before=state;click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));click("Riposo breve");click("Conferma");assert(state.magic.pact[4].current==2&&state.magic.slots[0].current==2&&state.reaction_spent);click("Fine");click("Riposi");click("Riposo lungo");click("Anteprima");assert(scratch.magic.slots[0].current==4&&scratch.magic.slots[8].current==1&&scratch.reaction_spent);screenshot("slot-cost-long-rest.ppm");click("Conferma");assert(state.magic.slots[0].current==4&&state.magic.pact[4].current==2&&state.reaction_spent&&!state.magic.concentration&&!state.reductions[0].active);
    effects_before=state;act(100);assert(stage_import(&imported_magic));screenshot("slot-cost-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.reductions[0].cost_kind&&!state.reaction_spent);
    /* Configure own-turn expiry through numeric, defence and reduction editors. */
    pd_demo(&state);state.hp=32;state.defences.configured=8191;act(1700);click("Aggiungi");click("Modifica: CA >");click("+10");click("Durata");click("Durata: manuale >");click("Durata: round >");assert(effect_draft.tick==PD_TICK_START);screenshot("turn-effect-editor.ppm");click("Anteprima");screenshot("turn-effect-confirm.ppm");click("Conferma");assert(state.effects[0].tick==PD_TICK_START);state.hp=41;
    act(1800);click("Difese per tipo");click("Contundente | normale");click("A tempo");click("Resistenza: no");click("Attiva: no");click("Durata");click("Durata: manuale >");click("Durata: round >");click("Durata: inizio turno >");assert(defence_temporary_draft.tick==PD_TICK_END);screenshot("turn-defence-editor.ppm");click("Anteprima");click("Conferma");click("Indietro");click("Riduci");click("Profili");click("Profilo 1 | inattivo >");click("Modifica");click("Quantita: 0 >");click("+1");click("Indietro");click("Tipi: Nessuno >");click("Attivo: no >");click("Dettagli");click("Durata: manuale >");click("Durata: round >");assert(profile_draft.tick==PD_TICK_START);screenshot("turn-profile-editor.ppm");click("Anteprima");click("Dati >");click("Regola >");screenshot("turn-profile-confirm.ppm");click("Conferma");click("Indietro");click("Indietro");
    /* Round excludes turn-bound timers, including from the configurable shortcut. */
    assert(!timed_effects());state.shortcuts[0]=8;shortcut(0);click("Reazione");assert(page==REACTION_HOME);screenshot("turn-home.ppm");click("Usa reazione");click("Conferma");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Inizio turno");assert(scratch.hp==32&&!scratch.reaction_spent&&!scratch.effects[0].active&&!scratch.reductions[0].active&&scratch.defences.temporary[0].active&&scratch.defences.temporary[0].rounds==1);screenshot("turn-start-preview.ppm");click("Scadenze");assert(page==TURN_DETAILS);bounds();screenshot("turn-start-details.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);act(2122);assert(page==TURN_DETAILS&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);click("Riepilogo");click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));click("Inizio turno");fail_save=true;click("Conferma");assert(page==REACTION_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.hp==32&&!state.reaction_spent&&!state.effects[0].active&&!state.reductions[0].active&&state.defences.temporary[0].active&&random_calls==effects_calls);screenshot("turn-start-applied.ppm");click("Indietro");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* End turn expires only end timers and preserves the reaction; start then applies caps. */
    state.shortcuts[0]=8;shortcut(0);click("Reazione");click("Fine turno");assert(scratch.reaction_spent&&scratch.effects[0].active&&scratch.reductions[0].active&&!scratch.defences.temporary[0].active&&scratch.hp==41);screenshot("turn-end-preview.ppm");click("Scadenze");screenshot("turn-end-details.ppm");click("Riepilogo");click("Conferma");assert(state.reaction_spent&&!state.defences.temporary[0].active&&state.effects[0].active);click("Inizio turno");click("Conferma");assert(!state.reaction_spent&&state.hp==32);effects_saves=save_calls;click("Inizio turno");click("Conferma");assert(save_calls==effects_saves);click("Indietro");
    /* Invalid numeric expiry blocks defence timers and reaction together before writing. */
    pd_demo(&state);state.armor=1;state.reaction_spent=true;assert(pd_effect_set(&state,0,(pd_effect){.value=2,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));assert(pd_effect_set(&state,1,(pd_effect){.value=-2,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_END}));assert(pd_defence_grant_set(&state,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));state.shortcuts[0]=8;shortcut(0);click("Reazione");effects_before=state;effects_saves=save_calls;click("Inizio turno");act(2122);assert(page==REACTION_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("turn-invalid.ppm");click("Indietro");click("Indietro");
    /* All fifty timers fit seventeen detail pages; last profile and high bits are present. */
    pd_demo(&state);for(unsigned i=0;i<8;i++)assert(pd_effect_set(&state,i,(pd_effect){.target=i%2,.value=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));state.hp=36;for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&state,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&state,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));state.shortcuts[0]=8;shortcut(0);click("Reazione");click("Inizio turno");click("Scadenze");for(unsigned i=0;i<16;i++)click(">");assert(turn_detail_page==16);act(2127);assert(turn_detail_page==16);bounds();screenshot("turn-last-details.ppm");click("Riepilogo");click("Conferma");assert(state.hp==32&&!state.reductions[2].active&&!state.effects[7].active);click("Indietro");
    /* Starting the turn can expire a profile selected in a blocked impact; no silent fallback. */
    pd_demo(&state);state.defences.configured=8191;state.reaction_spent=true;assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.cost_kind=PD_COST_COUNTER,.cost=1,.reaction=true}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");click("Gestisci reazione");click("Inizio turno");click("Conferma");click("Indietro");assert(page==DAMAGE_CONFIRM&&damage_profile==1&&!state.reductions[0].active&&!state.reaction_spent&&state.resources[0].current==1);effects_saves=save_calls;effects_calls=random_calls;act(1805);assert(page==DAMAGE_CONFIRM&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("turn-pending-impact.ppm");click("Indietro");
    /* Import cancellation preserves phases; replacement clears the local timers. */
    effects_before=state;act(100);assert(stage_import(&imported_magic));screenshot("turn-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.effect_count&&!state.reductions[2].tick&&!state.defences.additional[12][1].tick);
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, animated single/double dice, RNG isolation, skip, disabled input, timer cleanup, circular touch bounds >=44px, rest previews/policies, hit dice, rollback, condition notes/poison, death saves, display preferences, idle/wake policy explicit CA/max-HP effects, round durations/expiry, typed defences/damage, concentration saves with frozen retry, explicit effect bindings/end/recast/caps, signed rest preview, temporary defences, mixed damage, conditional rules/source declarations/grouping/frozen results, three independent extras/combined drafts/high-bit timers, before/after reductions/full math/frozen retry, reusable profiles/shared budget/42 timers/outer drafts, atomic pool costs/reaction/explicit turn/frozen expense, ordinary/pact slot costs/fixed levels/no fallback/rest recovery own-turn timers/atomic reaction reset/50 expiries/17 detail pages and guarded physical buttons passed");
}
