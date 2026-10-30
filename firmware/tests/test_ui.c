/* Actual firmware UI and LVGL rendered on host. No ESP peripherals simulated as verified. */
#include "lvgl.h"
static void *ui_malloc(size_t size);
#define PD_UI_TEST 1
#define lv_malloc ui_malloc
#include "../main/main.c"
#undef lv_malloc
#include <stdlib.h>
#include <math.h>
static bool sd_present,sd_verified;
bool pd_board_assets_available(void){return sd_present;}
bool pd_board_assets_space(uint64_t *available){*available=PD_ASSET_BYTES+4096;return sd_present;}
bool pd_board_assets_verify(const char *path,const char *sha){(void)path;(void)sha;return sd_verified;}
void pd_board_assets_activate(void){pd_looks_set_file(getenv("PD_TEST_CLASS_PACK"));}
static pd_wifi_info wifi_mock;
static pd_wifi_scan_info scan_mock;static pd_wifi_credentials joined_mock;static bool join_rejected;static unsigned join_calls;
static unsigned wifi_requested;
void pd_wifi_boot(void){}
uint32_t pd_wifi_random(void){return 0;}
void pd_wifi_snapshot(pd_wifi_info *out){*out=wifi_mock;}
bool pd_wifi_command(unsigned command){wifi_requested=command;return true;}
void pd_wifi_networks(pd_wifi_scan_info *out){*out=scan_mock;}
bool pd_wifi_join(const pd_wifi_credentials *credentials){if(join_rejected||!pd_wifi_credentials_valid(credentials))return false;joined_mock=*credentials;join_calls++;return true;}
bool pd_wifi_pause(bool paused){(void)paused;return true;}
static pd_sync_info sync_mock;static bool sync_fail_save;static unsigned sync_decisions;
void pd_sync_boot(void){}
void pd_sync_snapshot(pd_sync_info *out){*out=sync_mock;}
void pd_sync_observe(const pd_state *s,const pd_history *h,bool durable){(void)s;(void)h;(void)durable;}
bool pd_sync_pause(bool value){(void)value;return true;}
bool pd_sync_stage_json(const char *json,size_t size){(void)json;(void)size;return true;}
bool pd_sync_decide(bool accept,uint32_t epoch){(void)epoch;if(accept&&sync_fail_save)return false;sync_decisions++;sync_mock.offer=false;sync_mock.configured=accept;if(accept)sync_mock.state=PD_SYNC_BASELINE;return true;}
bool pd_sync_forget(void){if(sync_fail_save)return false;sync_mock=(pd_sync_info){0};return true;}
static bool fail_save,fail_canvas,fail_brightness,fail_storage_commit;
static unsigned brightness_calls,brightness_last;
static char last_reply[PD_DIAGNOSTICS_MAX+32];
void pd_board_assets_refresh(void){pd_looks_set_file(getenv("PD_TEST_CLASS_PACK"));}
void pd_ui_reply(const char *text){assert(strlen(text)<sizeof last_reply);strcpy(last_reply,text);}
void pd_board_diagnostics(pd_diagnostics *out){
    *out=(pd_diagnostics){.firmware="0.54.0-host-test",.idf="v5.5.5",.uptime_ms=123456,
        .internal_free=123000,.internal_min=100000,.internal_largest=90000,
        .psram_total=8000000,.psram_free=7000000,.psram_min=6000000,.psram_largest=5000000,
        .ui_stack_min=4000,.usb_stack_min=3000,.lvgl_ticks=200,.lvgl_max_us=30000,
        .flushes_submitted=100,.touch_samples=100,.touch_contacts=12,.pwr_available=true,
        .pwr_presses=3,.boot_presses=2};
}
bool pd_board_brightness(unsigned percent,void *unused){(void)unused;brightness_calls++;brightness_last=percent;return !fail_brightness;}
static void *ui_malloc(size_t size) { return fail_canvas?NULL:lv_malloc(size); }
static pd_power_info fake_power;
static bool fail_pause,fail_resume,fail_power_off;
static unsigned pause_calls,resume_calls,power_off_calls,power_wait_calls,power_wait_result=PD_POWER_WAIT;
static bool last_low_power;
void pd_board_power(pd_power_info *out,uint32_t now,bool force){(void)force;*out=fake_power;out->sampled_at=now;}
bool pd_board_pause(bool paused){if(paused){pause_calls++;return !fail_pause;}resume_calls++;return !fail_resume;}
unsigned pd_board_power_wait(bool battery_only){power_wait_calls++;last_low_power=battery_only;return power_wait_result;}
bool pd_board_power_off(void){power_off_calls++;return !fail_power_off;}
static unsigned random_calls,save_calls,history_save_calls,history_seen;
static bool fail_history,history_quarantined;static pd_history persisted_history;
bool pd_history_store_load(uint32_t epoch,pd_history *h){history_quarantined=false;if(persisted_history.epoch==epoch)*h=persisted_history;else pd_history_reset(h,epoch);return true;}
bool pd_history_store_save(const pd_history *h){history_save_calls++;if(fail_history||history_quarantined)return false;persisted_history=*h;history_seen|=1u<<h->entries[0].kind;return true;}
const char *pd_history_store_status(void){return fail_history?"Storico non salvato":"Storico salvato";}
static pd_history prepared_history;static bool prepared_ready;
static bool fail_backup_send;
bool pd_history_store_available(void){return !history_quarantined;}
bool pd_history_store_next_epoch(uint32_t current,uint32_t *next){if(history_quarantined||current==UINT32_MAX)return false;*next=current+1;return true;}
bool pd_history_store_prepare_restore(const pd_history *h){if(fail_history||history_quarantined)return false;prepared_history=*h;prepared_ready=true;return true;}
void pd_history_store_finish_restore(void){assert(prepared_ready);persisted_history=prepared_history;prepared_ready=false;}
void pd_history_store_cancel_restore(void){prepared_ready=false;}
void pd_history_store_uncertain_restore(void){prepared_ready=false;history_quarantined=true;}
bool pd_ui_backup_send(const pd_state *s,const pd_history *h){size_t size;char *json=pd_backup_encode(s,h,&size);assert(json);pd_backup *check=malloc(sizeof *check);assert(check&&pd_backup_decode(json,size,check));free(json);free(check);return !fail_backup_send;}
static uint32_t random_values[2]={9,9};
uint32_t pd_board_random(void *unused) { return random_values[(random_calls++)%2]; }
bool pd_storage_save(const pd_state *s) { save_calls++;return !fail_save&&!fail_storage_commit; }
const char *pd_storage_status(void) { return fail_save?"Errore memoria: riavvia":"Salvato offline"; }
bool pd_storage_writable(void){return !fail_save;}
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
    return NULL; /* Avatar and icon-only controls have no visible text label. */
}
static const char *button_name(lv_obj_t *b){lv_obj_t *l=button_label(b);const char *text=l?lv_label_get_text(l):NULL;return text&&*text?text:lv_obj_get_user_data(b);}
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
        lv_obj_t *l=button_label(o);if(!l)continue;lv_area_t text;lv_obj_get_coords(l,&text);
        if(!(text.x1>=a.x1&&text.x2<=a.x2&&text.y1>=a.y1&&text.y2<=a.y2)) { fprintf(stderr,"Overflow %s: button %d,%d,%d,%d text %d,%d,%d,%d\n",lv_label_get_text(l),a.x1,a.y1,a.x2,a.y2,text.x1,text.y1,text.x2,text.y2); abort(); }
    }
}
/* New cost labels must display the complete 40-character source, even wide glyphs. */
static void capture_styled_page(unsigned view){
    char path[64];snprintf(path,sizeof path,"ux-page-%02u.ppm",view);bounds();screenshot(path);
}
static void wrapped_fits(void){
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){
        lv_obj_t *l=lv_obj_get_child(root,i);if(!lv_obj_check_type(l,&lv_label_class)||lv_label_get_long_mode(l)!=LV_LABEL_LONG_WRAP)continue;
        lv_point_t size;lv_text_get_size(&size,lv_label_get_text(l),lv_obj_get_style_text_font(l,0),lv_obj_get_style_text_letter_space(l,0),lv_obj_get_style_text_line_space(l,0),lv_obj_get_content_width(l),LV_TEXT_FLAG_NONE);
        if(size.y>lv_obj_get_content_height(l)){fprintf(stderr,"Wrapped text clipped (%d > %d): %s\n",size.y,(int)lv_obj_get_content_height(l),lv_label_get_text(l));abort();}
    }
}
static void tap(int x,int y);
static void click(const char *name) {
    /* Existing rule scenarios now take the real touch path through home/menu/PF.
     * No calls to act() stand in for the newly introduced navigation controls. */
    if(!import_pending&&(page==HERO||page==HOME_VITALS)){
        bool hp=!strcmp(name,"-1 PF")||!strcmp(name,"+1 PF")||!strcmp(name,"Annulla")||!strcmp(name,"Cura")||!strcmp(name,"Temp.")||!strcmp(name,"Danni")||!strcmp(name,"Stato");
        if(page==HOME_VITALS&&!strcmp(name,"Eroe")){tap(305,391);return;}
        if(hp&&page==HERO){tap(200,104);assert(page==HOME_VITALS);}
        if(!hp&&page==HOME_VITALS){tap(305,391);assert(page==HERO);}
        if(!hp&&!strcmp(name,"Aspetto")){tap(233,264);assert(page==APPEARANCE);return;}
        if(!hp&&(!strcmp(name,"Scheda")||!strcmp(name,"Usi"))){tap(233,375);assert(page==HOME_MENU);}
    }
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *o=lv_obj_get_child(root,i);
        if(lv_obj_check_type(o,&lv_button_class)&&button_name(o)&&!strcmp(button_name(o),name)) {
            if(lv_obj_has_state(o,LV_STATE_DISABLED))fprintf(stderr,"Disabled button: %s page=%u\n",name,page);assert(!lv_obj_has_state(o,LV_STATE_DISABLED));lv_obj_send_event(o,LV_EVENT_CLICKED,NULL);bounds();return;
        }
    }
    fprintf(stderr,"Button not found: %s (page=%u, import=%d, dice=%d)\n",name,page,import_pending,dice_animation.active);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){lv_obj_t *o=lv_obj_get_child(root,i);if(lv_obj_check_type(o,&lv_button_class))fprintf(stderr,"  button: %s\n",button_label(o)?lv_label_get_text(button_label(o)):"(icon)");}
    assert(!"button not found");
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
/* A real LVGL pointer stream for the on-device target test. The peripheral
   samples are synthetic; hit-testing, press/release and callbacks are real. */
static int probe_x,probe_y;
static bool probe_down,probe_valid=true;
static void probe_touch(lv_indev_t *indev,lv_indev_data_t *data){
    (void)indev;
    bool allowed=pd_screen_touch(&display_screen,probe_valid,probe_down,lv_tick_get());
    data->state=allowed?LV_INDEV_STATE_PRESSED:LV_INDEV_STATE_RELEASED;
    data->point=(lv_point_t){probe_x,probe_y};
}
static void probe_sample(lv_indev_t *indev,int x,int y,bool down,unsigned elapsed){
    probe_x=x;probe_y=y;probe_down=down;lv_tick_inc(elapsed);
    lv_obj_update_layout(lv_screen_active());lv_indev_read(indev);bounds();
}
static void probe_click(lv_indev_t *indev,int x,int y){
    probe_sample(indev,x,y,true,20);probe_sample(indev,x,y,false,20);
}
static void probe_key_sample(int pwr,int boot,unsigned elapsed){lv_tick_inc(elapsed);screen_service(pwr,boot);bounds();}
static void probe_key_gesture(unsigned which,unsigned held){
    probe_key_sample(0,0,0);probe_key_sample(0,0,31);
    int pwr=which==0,boot=which!=0;
    probe_key_sample(pwr,boot,0);probe_key_sample(pwr,boot,31);
    probe_key_sample(pwr,boot,held);probe_key_sample(0,0,0);probe_key_sample(0,0,31);
}
static void type_turn_name(const char *text){
    for(const char *at=text;*at;at++){
        const char *key=strchr(name_keys,*at);assert(key);unsigned target=(unsigned)(key-name_keys)/12;
        while(name_key_page<target)click(">");while(name_key_page>target)click("<");
        char caption[2]={*at,0};click(*at==' '?"Spazio":caption);
    }
}
static void type_search(const char *text){
    for(const char *at=text;*at;at++){
        char c=*at>='a'&&*at<='z'?*at-'a'+'A':*at;const char *key=strchr(search_keys,c);assert(key);unsigned target=(unsigned)(key-search_keys)/12;
        while(search_key_page<target)click(">");while(search_key_page>target)click("<");
        char caption[2]={c,0};click(c==' '?"Spazio":caption);
    }
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
    /* The live personalized avatar replaces the demonstration illustration. */
    assert(avatar_scale==4&&avatar_image.header.w==96&&avatar_image.header.h==120);
    assert(avatar_sprite==companion_avatar_object()&&!scene_lights[0]&&!scene_halos[0]);
    click("Aspetto");assert(page==APPEARANCE&&avatar_timer);screenshot("appearance.ppm");
    /* Appearance still uses the original scene lighting; dimming freezes it. */
    lv_opa_t light_before=lv_obj_get_style_opa(scene_lights[0],0);display_screen.dimmed=true;
    lv_opa_t halo_before=lv_obj_get_style_opa(scene_halos[0],0);
    lv_tick_inc(400);lv_timer_handler();assert(lv_obj_get_style_opa(scene_lights[0],0)==light_before&&lv_obj_get_style_opa(scene_halos[0],0)==halo_before);
    display_screen.dimmed=false;
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
    /* Production poses: committed deltas, fixed deadlines across redraws. */
    for(unsigned i=0;i<3;i++) {
        pd_demo(&state);state.avatar=presets[i];feedback.active=false;render();
        unsigned writes=save_calls,random=random_calls;char file[64];
        assert(avatar_phase==0);snprintf(file,sizeof file,"avatar-live-%u-idle.ppm",i);screenshot(file);
        lv_tick_inc(800);lv_timer_handler();assert(avatar_phase==1);
        snprintf(file,sizeof file,"avatar-live-%u-blink.ppm",i);screenshot(file);
        click("-1 PF");assert(avatar_phase==3);snprintf(file,sizeof file,"avatar-live-%u-hurt.ppm",i);screenshot(file);
        lv_tick_inc(600);lv_timer_handler();assert(avatar_phase==0);
        click("+1 PF");assert(avatar_phase==2);snprintf(file,sizeof file,"avatar-live-%u-heal.ppm",i);screenshot(file);
        lv_tick_inc(900);render();assert(avatar_phase==2);lv_tick_inc(100);avatar_frame(NULL);assert(avatar_phase==0);
        assert(save_calls==writes+2&&random_calls==random&&state.hp==24);
    }
    pd_demo(&state);feedback.active=false;render();
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
    click("Scheda");assert(!avatar_timer&&!avatar_sprite&&!scene_lights[0]&&!scene_halos[0]);screenshot("sheet.ppm");click("DES 16\n+3");
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
    fail_canvas=false;act(14);assert(dice_pixels);
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
    pd_demo(&state);state.abilities[0]=18;state.training.ranks[7]=1;state.training.skill_abilities[7]=1;state.training.strength_advantage=true;
    page=SHEET;sheet_tab=1;skill_page=2;ignore_effects=false;mode=0;render();bounds();screenshot("skill-ability-list.ppm");
    random_values[0]=random_values[1]=9;click("Intimidire  +6");click("Lancia");assert(result.bonus==6&&result.total==16&&result.count==2);click("Accelera");
    click("Intimidire +6 >");assert(page==CHECK_DETAIL);bounds();screenshot("skill-ability-detail.ppm");
    state.training=(pd_training){0};page=SHEET;sheet_tab=1;render();bounds();screenshot("skills-legacy.ppm");
    click("Caratt.");click("DES 16\n+3");click("Lancia");assert(result.count==1&&result.bonus==3);click("Accelera");
    /* Device inventory: atomic consumables, attack -> critical -> one damage roll. */
    pd_demo(&state);combat_roll=0;has_roll=false;page=RESOURCES;inventory_page=0;render();bounds();
    click("Zaino");assert(page==INVENTORY);screenshot("inventory.ppm");
    click("Spada [E]  x1");assert(page==ITEM_DETAIL);screenshot("weapon.ppm");click("Riponi");click("Conferma");assert(state.equipped==PD_NO_WEAPON);click("Equipaggia");click("Conferma");assert(state.equipped==0);click("Attacco");
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
    click("Accelera");screenshot("attack-miss.ppm");click("Arma");click("Arma");click("Zaino");
    click("Pozione  x2");assert(page==ITEM_DETAIL);calls=random_calls;click("Usa");assert(page==ITEM_CONFIRM&&random_calls==calls);
    screenshot("item-confirm.ppm");click("Indietro");assert(state.items[1].quantity==2&&state.hp==24);
    click("Usa");state.temp_hp=4;click("Conferma");assert(combat_roll==3&&state.hp==28&&state.temp_hp==4&&state.items[1].quantity==1&&random_calls==calls+2);
    click("Accelera");screenshot("healing.ppm");click("Eroe");click("Annulla");assert(state.hp==24&&state.temp_hp==4&&state.items[1].quantity==2);
    click("Usi");click("Zaino");click("Torcia  x3");calls=random_calls;click("Usa");click("Conferma");assert(state.items[2].quantity==2&&random_calls==calls);
    click("Zaino");click("Pozione  x2");click("Usa");pd_state before_failed_use=state;fail_save=true;click("Conferma");
    assert(!memcmp(&state,&before_failed_use,sizeof state)&&page==ITEM_CONFIRM&&item_rolled);calls=random_calls;act(91);assert(page==ITEM_CONFIRM);fail_save=false;click("Riprova");assert(random_calls==calls);click("Accelera");click("Eroe");click("Annulla");click("Usi");click("Zaino");
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
    click("Risorse");click("Elenco");click("Innato sintetico");click("Lancia");click("Conferma");assert(!state.magic.powers[2].remaining);click("Gestisci");click("Utilizzi manuali");screenshot("magic-recovery.ppm");click("+1");assert(state.magic.powers[2].remaining==1);click("Indietro");
    click("Elenco");click("Capacita");click("Ira sintetica");click("Usa");click("Conferma");assert(state.combat.raging&&state.training.strength_advantage&&state.magic.powers[3].remaining==1);
    click("Elenco");click("Magie");click("Luce sintetica");before_magic=state;act(831);assert(page==MAGIC_DETAIL&&!memcmp(&state,&before_magic,sizeof state));
    click("Elenco");click("Risorse");click("Ira: termina");click("Conferma");assert(!state.combat.raging&&!state.training.strength_advantage);
    click("Elenco");click("Luce sintetica");magic_saves=save_calls;click("Lancia");click("Conferma");assert(save_calls==magic_saves); /* A cantrip with no state changes does not wear flash. */
    click("Elenco");click("Scudo sintetico");click("Lancia");before_magic=state;fail_save=true;click("Conferma");assert(page==MAGIC_CONFIRM&&!memcmp(&state,&before_magic,sizeof state));fail_save=false;click("Indietro");
    click("Elenco");click("Usi");click("Eroe");click("Aspetto");click("Tasti");assert(page==KEY_SETTINGS);screenshot("keys.ppm");
    click("BOOT breve: Dadi");assert(page==KEY_PICKER&&state.shortcuts[1]==3);click("Zaino");assert(state.shortcuts[1]==4);calls=random_calls;shortcut(1);assert(page==INVENTORY&&random_calls==calls);
    shortcut(0);assert(page==HERO);state.shortcuts[1]=6;act(14);shortcut(1);assert(dice_animation.active);calls=random_calls;shortcut(0);assert(page==DICE&&dice_animation.active);shortcut(1);assert(!dice_animation.active&&random_calls==calls);
    static pd_state imported_magic;pd_demo(&imported_magic);assert(stage_import(&imported_magic)&&pending.shortcuts[1]==6);shortcut(0);assert(import_pending);click("Annulla");
    act(100);click("Aspetto");click("Tasti");before_magic=state;act(860);assert(page==KEY_PICKER);while(key_page)click("Pagina precedente");fail_save=true;click("Spento");assert(page==KEY_PICKER&&!memcmp(&state,&before_magic,sizeof state));fail_save=false;click("Indietro");assert(page==KEY_SETTINGS);
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
    act(910);assert(page==HIT_CONFIRM&&hit_rolled);screenshot("practical-hit-retry.ppm");click("Riprova");assert(random_calls==calls+1&&!memcmp(&state,&before_rest,sizeof state));fail_save=false;click("Riprova");
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
    fail_save=true;click("Conferma");assert(status_rolled&&random_calls==calls+1&&page==STATUS_CONFIRM&&!memcmp(&state,&before_status,sizeof state));click("Riprova");assert(random_calls==calls+1);
    fail_save=false;click("Riprova");assert(page==DICE&&combat_roll==5&&result.count==1&&!result.bonus&&state.status.successes==2&&state.status.failures==1&&random_calls==calls+1);
    calls=random_calls;shortcut(1);assert(!dice_animation.active&&random_calls==calls);bounds();screenshot("death-result.ppm");act(13);assert(random_calls==calls&&state.status.successes==2);
    click("Stato");click("Correggi / stabilizza");screenshot("death-edit.ppm");click("Riuscita +");click("Conferma");assert(state.status.stabilized&&!state.status.successes&&!state.status.failures&&state.hp==0);screenshot("death-stable.ppm");act(1510);assert(page==DEATH_VIEW&&random_calls==calls);
    click("Correggi / stabilizza");click("Critico +2");click("Conferma");assert(!state.status.stabilized&&state.status.failures==2);
    click("Lancia d20 base");random_values[0]=random_values[1]=0;click("Conferma");click("Accelera");assert(state.status.failures==3&&!state.hp);screenshot("death-three.ppm");click("Fine");
    before_status=state;status_saves=save_calls;act(2);assert(!memcmp(&state,&before_status,sizeof state)&&save_calls==status_saves);click("Usi");click("Riposi");act(903);assert(page==REST_HOME);click("Usi");click("Zaino");click("Pozione  x2");calls=random_calls;act(88);assert(page==ITEM_DETAIL&&random_calls==calls&&state.items[1].quantity==2);
    act(1501);click("Correggi / stabilizza");click("Fallita -");click("Conferma");assert(state.status.failures==2);click("Lancia d20 base");random_values[0]=random_values[1]=19;click("Conferma");click("Accelera");assert(state.hp==1&&!state.status.failures&&!state.status.successes);screenshot("death-twenty.ppm");
    click("Eroe");click("Annulla");assert(!state.hp&&state.status.failures==2);state.shortcuts[0]=7;act(850);click("PWR breve: Riposi");click("Stato");assert(state.shortcuts[0]==8);shortcut(0);assert(page==STATUS_HOME);before_status=state;assert(stage_import(&imported_magic)&&pending.shortcuts[0]==8);click("Annulla");assert(!memcmp(&state,&before_status,sizeof state));
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
    state.shortcuts[0]=8;act(850);click("PWR breve: Stato");click("Schermo");assert(state.shortcuts[0]==9);shortcut(0);assert(page==DEVICE_SETTINGS);screenshot("screen-configured.ppm");
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
    state.shortcuts[0]=9;act(850);click("PWR breve: Schermo");click("Effetti");assert(state.shortcuts[0]==10);shortcut(0);assert(page==EFFECT_LIST);
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
    state.shortcuts[0]=10;act(850);click("PWR breve: Effetti");click("Round");assert(state.shortcuts[0]==11);effects_before=state;shortcut(0);assert(page==EFFECT_ROUND&&!memcmp(&state,&effects_before,sizeof state));
    shortcut(0);assert(page==EFFECT_ROUND);click("Conferma");assert(state.hp==32&&state.effects[0].rounds==1);shortcut(0);click("Conferma");assert(!state.effects[0].active&&pd_armor(&state)==16);screenshot("duration-all-expired.ppm");
    effects_saves=save_calls;shortcut(0);assert(page==EFFECT_LIST&&save_calls==effects_saves);
    /* An expired entry cannot be silently re-enabled with a zero timer. */
    click("2. PF max +10 | scaduto");click("Attivo: no");click("Anteprima");effects_before=state;effects_saves=save_calls;act(1709);assert(page==EFFECT_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("duration-zero.ppm");
    click("Indietro");click("Durata");click("+1");click("Anteprima");click("Conferma");assert(state.effects[1].active&&state.effects[1].rounds==1&&state.hp==32);
    /* Paused effects retain time; returning to manual is an explicit choice. */
    click("2. PF max +10 | 1r");click("Attivo: si");click("Anteprima");click("Conferma");effects_before=state;act(1740);assert(page==EFFECT_LIST&&!memcmp(&state,&effects_before,sizeof state));
    click("2. PF max +10 | 1r pausa");click("Durata");click("Durata: round >");click("Durata: inizio turno >");click("Durata: fine turno >");click("Durata: minuti >");click("Durata: ore >");click("Anteprima");click("Conferma");assert(!state.effects[1].timed&&!state.effects[1].rounds);
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
    state.shortcuts[0]=11;act(850);click("PWR breve: Round");click("Pagina successiva");click("Danni");assert(state.shortcuts[0]==12);shortcut(0);assert(page==DAMAGE_HOME&&damage_plan.amount==1);effects_before=state;assert(stage_import(&imported_magic)&&!pending.defences.configured);screenshot("defences-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
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
    click("Profilo 1 | attivo >");click("Costi");effects_before=state;act(2101);act(2103);assert(!memcmp(&state,&effects_before,sizeof state)&&!profile_cost_editing);screenshot("cost-view.ppm");click("Indietro");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(page==PROFILE_COST_IMPACT&&damage_details.costs[0].before==3&&damage_details.costs[0].after==1&&!damage_details.reaction_before&&damage_details.reaction_after);bounds();screenshot("cost-preview.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_COST_IMPACT&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");
    /* The frozen CON candidate charges exactly once and Undo restores everything. */
    click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.hp==7&&!hit_candidate.temp_hp&&hit_candidate.resources[0].current==1&&hit_candidate.reaction_spent&&!hit_candidate.magic.concentration&&!hit_candidate.reductions[0].active&&concentration_event_report.costs[0].after==1);screenshot("cost-retry.ppm");act(2117);act(2122);act(2101);act(2103);assert(page==CONCENTRATION_SETUP&&state.resources[0].current==3&&!state.reaction_spent);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.resources[0].current==1&&state.reaction_spent&&state.hp==7);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==3&&damage_details.costs[0].after==1&&damage_details.profile_snapshot.reaction&&damage_details.profile_snapshot.active);screenshot("cost-snapshot.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Explicit selection pays even if a separate manual reduction makes damage zero. */
    act(1800);click("Riduci");click("+1");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_calls=random_calls;effects_saves=save_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(!damage_details.total.adjusted&&!damage_details.profile_applied&&damage_details.costs[0].after==1);screenshot("cost-zero.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==32&&state.temp_hp==3&&state.magic.concentration==1&&state.resources[0].current==1&&state.reaction_spent&&random_calls==effects_calls&&save_calls==effects_saves+1);
    /* Recovering reaction from a blocked preview preserves the prepared impact. */
    click("Nuovo");click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("cost-blocked.ppm");click("Gestisci reazione");assert(reaction_return==DAMAGE_CONFIRM);bounds();screenshot("reaction-used.ppm");click("Inizio turno");screenshot("reaction-turn.ppm");click("Indietro");assert(state.reaction_spent);click("Inizio turno");fail_save=true;click("Conferma");assert(page==REACTION_CONFIRM&&state.reaction_spent&&state.resources[0].current==1);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==REACTION_CONFIRM&&!stage_import(&imported_magic));fail_save=false;click("Conferma");assert(!state.reaction_spent&&state.resources[0].current==1);screenshot("reaction-ready.ppm");effects_saves=save_calls;click("Inizio turno");click("Conferma");assert(save_calls==effects_saves);click("Indietro");assert(page==DAMAGE_CONFIRM&&damage_profile==1&&damage_plan.amount==1);pd_damage_event cost_event=current_damage_event();pd_damage_event_summary cost_report;assert(pd_damage_event_preview(&state,&cost_event,&cost_report)&&cost_report.cost_missing&&!cost_report.reaction_missing);click("Indietro");
    /* Switch to a limited-use pool; long names and zero balances remain explicit. */
    strcpy(state.magic.powers[1].name,"Nome sintetico di quaranta caratteri ABC");click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: contatore 1 >");assert(profile_draft.cost_kind==PD_COST_POWER&&profile_draft.cost_index==1);click("Costo: 1 >");click("+1");click("Indietro");bounds();screenshot("cost-power.ppm");click("Indietro");click("Anteprima");for(unsigned i=0;i<3;i++)act(2086);bounds();screenshot("cost-long.ppm");click("Conferma");click("Indietro");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==2&&!damage_details.costs[0].after&&damage_details.profile_snapshot.cost_kind==PD_COST_POWER);screenshot("cost-power-preview.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Conferma");assert(!state.magic.powers[1].remaining&&state.reaction_spent&&state.resources[0].current==1);
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
    click("Profilo 1 | attivo >");click("Costi");effects_before=state;act(2101);act(2103);assert(!memcmp(&state,&effects_before,sizeof state));screenshot("slot-cost-view.ppm");click("Indietro");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_calls=random_calls;click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==2&&!damage_details.costs[0].after&&!strcmp(damage_details.costs[0].name,"Slot ordinario livello 1"));wrapped_fits();screenshot("slot-cost-preview.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");
    /* One frozen COS roll charges only the chosen ordinary pool, including retry and Undo. */
    click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.hp==7&&!hit_candidate.temp_hp&&!hit_candidate.magic.slots[0].current&&hit_candidate.magic.pact[4].current==2&&hit_candidate.reaction_spent&&!hit_candidate.magic.concentration&&!hit_candidate.reductions[0].active&&!memcmp(&state,&effects_before,sizeof state));screenshot("slot-cost-retry.ppm");act(2101);act(2122);assert(page==CONCENTRATION_SETUP);fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&!state.magic.slots[0].current&&state.magic.pact[4].current==2);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==2&&!damage_details.costs[0].after&&!strcmp(damage_details.costs[0].name,"Slot ordinario livello 1"));screenshot("slot-cost-snapshot.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* An exhausted ninth-level pool remains selectable but blocks; no implicit substitution. */
    act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: slot 1 >");assert(profile_draft.cost_kind==PD_COST_SLOT&&profile_draft.cost_index==8&&profile_draft.cost==1);screenshot("slot-cost-ninth.ppm");click("Costo: 1 >");act(2084);assert(profile_draft.cost==1);click("Indietro");click("Indietro");click("Anteprima");click("Conferma");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("slot-cost-blocked.ppm");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.cost_missing&&!strcmp(damage_details.costs[0].name,"Slot ordinario livello 9")&&!damage_details.costs[0].before&&!damage_details.costs[0].after);screenshot("slot-cost-empty.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Indietro");
    /* Switching to pact never replaces it with ordinary slots; zero damage still pays. */
    click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");click("Fonte: slot 9 >");assert(profile_draft.cost_kind==PD_COST_PACT&&profile_draft.cost_index==4&&profile_draft.cost==1);click("Costo: 1 >");click("+1");act(2084);assert(profile_draft.cost==2);click("Indietro");wrapped_fits();screenshot("slot-cost-pact.ppm");click("Indietro");click("Anteprima");click("Conferma");click("Indietro");click("Usa valori");click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(!damage_details.total.adjusted&&damage_details.costs[0].before==2&&!damage_details.costs[0].after&&!strcmp(damage_details.costs[0].name,"Slot patto livello 5"));screenshot("slot-cost-zero.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");effects_calls=random_calls;effects_before=state;fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);fail_save=false;click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==32&&state.temp_hp==3&&state.reaction_spent&&!state.magic.pact[4].current&&state.magic.slots[0].current==2&&random_calls==effects_calls);
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
    /* Configure minutes and fractional hours through all three existing editors. */
    pd_demo(&state);state.hp=32;state.defences.configured=8191;act(1700);click("Aggiungi");click("Modifica: CA >");click("+10");click("Durata");click("Durata: manuale >");click("Durata: round >");click("Durata: inizio turno >");click("Durata: fine turno >");assert(effect_draft.tick==PD_TICK_MINUTES&&!effect_draft.rounds&&effect_draft.minutes==1);click("+1");screenshot("time-effect-editor.ppm");click("Anteprima");click("Conferma");state.hp=41;
    act(1800);click("Difese per tipo");click("Contundente | normale");click("A tempo");click("Resistenza: no");click("Attiva: no");click("Durata");click("Durata: manuale >");click("Durata: round >");click("Durata: inizio turno >");click("Durata: fine turno >");click("Durata: minuti >");assert(defence_temporary_draft.tick==PD_TICK_HOURS&&defence_temporary_draft.minutes==60);click("+1h");click("-1m");assert(defence_temporary_draft.minutes==119&&!defence_temporary_draft.rounds);screenshot("time-defence-hours.ppm");click("Anteprima");click("Conferma");click("Indietro");
    click("Riduci");click("Profili");click("Profilo 1 | inattivo >");click("Modifica");click("Quantita: 0 >");click("+1");click("Indietro");click("Tipi: Nessuno >");click("Attivo: no >");click("Fase: prima >");click("Dettagli");click("Durata: manuale >");click("Durata: round >");click("Durata: inizio turno >");click("Durata: fine turno >");click("Minuti residui: 1 >");click("+1");click("-1");click("Indietro");click("Costi");click("Fonte: nessuna >");click("Usa reazione: no >");click("Indietro");click("Anteprima");click("Dati >");click("Regola >");screenshot("time-profile-confirm.ppm");click("Conferma");click("Indietro");click("Indietro");assert(state.reductions[0].minutes==1&&state.reductions[0].tick==PD_TICK_MINUTES);
    /* Elapsed game time preserves rounds, own-turn counters, reaction and resource balances. */
    assert(pd_effect_set(&state,1,(pd_effect){.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START}));state.reaction_spent=true;state.shortcuts[0]=8;shortcut(0);click("Tempo");assert(page==TIME_HOME&&time_minutes==1);screenshot("time-home.ppm");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Anteprima");assert(scratch.hp==41&&scratch.effects[0].minutes==1&&!scratch.reductions[0].active&&scratch.defences.temporary[0].minutes==118&&scratch.effects[1].rounds==2&&scratch.reaction_spent&&scratch.resources[0].current==1);screenshot("time-preview.ppm");click("Scadenze");bounds();screenshot("time-details.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);act(2148);assert(page==TIME_DETAILS&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);click("Riepilogo");click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));click("Anteprima");fail_save=true;click("Conferma");assert(page==TIME_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==TIME_HOME&&state.effects[0].minutes==1&&state.defences.temporary[0].minutes==118&&state.reaction_spent&&state.resources[0].current==1&&random_calls==effects_calls);click("Indietro");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    state.shortcuts[0]=8;shortcut(0);click("Tempo");click("+1m");click("Anteprima");assert(scratch.hp==32&&!scratch.effects[0].active&&!scratch.reductions[0].active&&scratch.defences.temporary[0].minutes==117&&scratch.effects[1].rounds==2&&scratch.reaction_spent);screenshot("time-cap-preview.ppm");click("Conferma");assert(state.hp==32&&state.defences.temporary[0].minutes==117&&state.resources[0].current==1);click("Indietro");click("Tempo");click("+1h");click("+1h");for(unsigned i=0;i<4;i++)click("-1m");assert(time_minutes==117);click("Anteprima");click("Conferma");assert(!state.defences.temporary[0].active&&!state.defences.temporary[0].minutes&&state.reaction_spent&&state.effects[1].rounds==2);effects_saves=save_calls;for(unsigned i=0;i<100;i++)act(2145);assert(time_minutes==5940);act(2145);assert(time_minutes==5940);screenshot("time-maximum.ppm");click("Anteprima");click("Conferma");assert(save_calls==effects_saves);click("Indietro");
    /* A valid final total does not conceal an invalid intermediate expiry. */
    pd_demo(&state);state.armor=1;assert(pd_effect_set(&state,0,(pd_effect){.value=2,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=1}));assert(pd_effect_set(&state,1,(pd_effect){.value=-2,.active=true,.timed=true,.tick=PD_TICK_MINUTES,.minutes=2}));state.shortcuts[0]=8;shortcut(0);click("Tempo");click("+1m");effects_before=state;effects_saves=save_calls;click("Anteprima");act(2148);assert(page==TIME_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("time-invalid.ppm");click("Indietro");click("Indietro");
    /* Fifty precise hour timers are reviewable on seventeen pages, including longest rows. */
    pd_demo(&state);for(unsigned i=0;i<8;i++)assert(pd_effect_set(&state,i,(pd_effect){.target=i%2,.value=1,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=5940}));state.hp=36;for(unsigned type=0;type<13;type++)for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&state,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=5940}));for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&state,i,(pd_reduction_profile){.value=3,.types=8191,.active=true,.timed=true,.tick=PD_TICK_HOURS,.minutes=5940}));state.shortcuts[0]=8;shortcut(0);click("Tempo");click("Anteprima");click("Scadenze");for(unsigned i=0;i<3;i++)click(">");screenshot("time-wide-details.ppm");for(unsigned i=3;i<16;i++)click(">");assert(time_detail_page==16);act(2152);assert(time_detail_page==16);screenshot("time-last-details.ppm");click("Riepilogo");click("Conferma");assert(state.effects[0].minutes==5939&&state.reductions[2].minutes==5939);click("Indietro");
    effects_before=state;act(100);assert(stage_import(&imported_magic));screenshot("time-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.effect_count&&!state.reductions[2].minutes&&!state.defences.additional[12][1].minutes);
    /* Item charge correction: draft, atomic save/undo, unavailable rows and key/import isolation. */
    pd_demo(&state);page=HERO;state.items[0].charges=(pd_charges){2,7,1};act(500);bounds();screenshot("charges-item.ppm");click("Cariche");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;wrapped_fits();screenshot("charges-edit.ppm");click("+10");assert(charge_draft==7&&state.items[0].charges.current==2);click("Anteprima");bounds();wrapped_fits();screenshot("charges-confirm.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==ITEM_CHARGE_CONFIRM&&!stage_import(&imported_magic)&&save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");click("Indietro");assert(page==ITEM_DETAIL&&!memcmp(&state,&effects_before,sizeof state));click("Cariche");click("-10");assert(!charge_draft);click("Anteprima");fail_save=true;click("Conferma");assert(page==ITEM_CHARGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("charges-retry.ppm");fail_save=false;click("Conferma");assert(page==ITEM_CHARGES&&!state.items[0].charges.current&&state.items[0].quantity==1&&random_calls==effects_calls);effects_saves=save_calls;click("Anteprima");click("Conferma");assert(save_calls==effects_saves);click("Indietro");click("Zaino");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Charges do not consume a unit; consuming a unit does not spend its charges. */
    state.items[2].charges=(pd_charges){3,9,2};state.items[2].quantity=1;act(502);click("Usa");click("Conferma");assert(!state.items[2].quantity&&state.items[2].charges.current==3);act(502);click("Cariche");effects_before=state;effects_saves=save_calls;act(2162);act(2166);act(2168);assert(page==ITEM_CHARGES&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);bounds();screenshot("charges-unavailable.ppm");click("Indietro");click("Zaino");
    /* Full-length item name, last index, maximum balance and manual recovery. */
    state.item_count=60;for(unsigned i=3;i<60;i++)state.items[i]=(pd_item){.name="Oggetto sintetico",.quantity=1};strcpy(state.items[59].name,"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");state.items[59].charges=(pd_charges){998,999,0};act(559);click("Cariche");click("+10");assert(charge_draft==999);bounds();wrapped_fits();screenshot("charges-long-name.ppm");click("Indietro");click("Zaino");
    /* A charge cost uses its exact row and remains a frozen receipt after a CON retry. */
    pd_demo(&state);page=HERO;state.hp=32;state.items[0].charges=(pd_charges){3,7,1};state.defences.configured=8191;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.cost_kind=PD_COST_ITEM,.cost=2,.reaction=true}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");assert(profile_draft.cost_kind==PD_COST_ITEM);bounds();screenshot("charges-cost-editor.ppm");click("Fonte: cariche 1 >");assert(profile_draft.cost_kind==PD_COST_NONE);click("Fonte: nessuna >");assert(profile_draft.cost_kind==PD_COST_COUNTER);click("Fonte: contatore 1 >");assert(profile_draft.cost_kind==PD_COST_ITEM);click("Indietro");click("Indietro");click("Indietro");click("Indietro");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");damage_plan.amount=20;render();click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==3&&damage_details.costs[0].after==1&&!strcmp(damage_details.costs[0].name,state.items[0].name));screenshot("charges-cost-preview.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");effects_before=state;effects_calls=random_calls;click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.items[0].charges.current==1&&hit_candidate.items[0].quantity==1&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.items[0].charges.current==1);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==3&&damage_details.costs[0].after==1);screenshot("charges-cost-receipt.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    state.equipped=PD_NO_WEAPON;state.items[0].quantity=0;act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_calls=random_calls;effects_saves=save_calls;act(1805);assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls&&save_calls==effects_saves);screenshot("charges-cost-blocked.ppm");click("Indietro");
    pd_demo(&state);page=HERO;state.items[0].charges=(pd_charges){2,7,1};state.items[1].charges=(pd_charges){1,9,2};act(900);click("Riposo breve");assert(scratch.items[0].charges.current==7&&scratch.items[1].charges.current==1);screenshot("charges-rest-short.ppm");click("Indietro");click("Riposo lungo");click("Anteprima");assert(scratch.items[0].charges.current==7&&scratch.items[1].charges.current==9);screenshot("charges-rest-long.ppm");click("Conferma");assert(state.items[0].charges.current==7&&state.items[1].charges.current==9&&state.items[1].quantity==2);

    /* Configure three independent expenses on the device; only explicit confirmation saves. */
    pd_demo(&state);page=HERO;state.hp=32;state.defences.configured=8191;state.resources[0].current=3;state.resources[0].maximum=5;state.items[0].charges=(pd_charges){3,7,1};state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Magia sintetica",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.powers[1]=(pd_power){.name="Utilizzo sintetico",.kind=1,.maximum=3,.remaining=2,.recovery=1};state.magic.concentration=1;
    assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Modifica");click("Dettagli");click("Costi");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    click("Fonte: nessuna >");assert(!profile_cost_row&&profile_draft.cost_kind==PD_COST_COUNTER);click("Costo: 1 >");click("+1");click("Indietro");click("Usa reazione: no >");click("Spesa 1/3 >");click("Fonte: nessuna >");assert(profile_cost_row==1&&profile_draft.extra_costs[0].kind==PD_COST_POWER&&profile_draft.extra_costs[0].index==1);click("Costo: 1 >");click("+1");click("Indietro");bounds();screenshot("multi-cost-editor-second.ppm");click("Spesa 2/3 >");click("Fonte: nessuna >");assert(profile_cost_row==2&&profile_draft.extra_costs[1].kind==PD_COST_ITEM);click("Costo: 1 >");click("+1");click("Indietro");bounds();screenshot("multi-cost-editor-third.ppm");assert(save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));
    click("Indietro");click("Anteprima");click("Dati >");click("Regola >");click("Durata >");assert(profile_confirm_tab==3&&!profile_cost_row);bounds();screenshot("multi-cost-confirm-first.ppm");click("Spesa 1/3 >");click("Spesa 2/3 >");bounds();screenshot("multi-cost-confirm-third.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_CONFIRM&&!stage_import(&imported_magic));fail_save=true;click("Conferma");assert(page==PROFILE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.reductions[0].cost==2&&state.reductions[0].extra_costs[0].amount==2&&state.reductions[0].extra_costs[1].amount==2&&state.reductions[0].reaction);
    click("Profilo 1 | attivo >");click("Costi");effects_before=state;act(2101);act(2102);act(2103);click("Spesa 1/3 >");act(2101);act(2103);click("Spesa 2/3 >");assert(profile_cost_row==2&&!memcmp(&state,&effects_before,sizeof state));bounds();screenshot("multi-cost-view.ppm");click("Indietro");click("Usa");click("Usa profilo");click("Usa valori");damage_plan.amount=20;render();click("Anteprima");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(!profile_cost_row&&damage_details.costs[0].before==3&&damage_details.costs[0].after==1);bounds();screenshot("multi-cost-preview-first.ppm");click("Spesa 1/3 >");assert(damage_details.costs[1].before==2&&!damage_details.costs[1].after&&!strcmp(damage_details.costs[1].name,"Utilizzo sintetico"));screenshot("multi-cost-preview-second.ppm");click("Spesa 2/3 >");assert(damage_details.costs[2].before==3&&damage_details.costs[2].after==1);screenshot("multi-cost-preview-third.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==PROFILE_COST_IMPACT&&!stage_import(&imported_magic)&&save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Tira COS");random_values[0]=random_values[1]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&hit_candidate.resources[0].current==1&&!hit_candidate.magic.powers[1].remaining&&hit_candidate.items[0].charges.current==1&&hit_candidate.items[0].quantity==1&&hit_candidate.reaction_spent&&!hit_candidate.magic.concentration&&!memcmp(&state,&effects_before,sizeof state));screenshot("multi-cost-retry.ppm");fail_save=false;click("Riprova");assert(page==DICE&&random_calls==effects_calls+1&&state.resources[0].current==1&&!state.magic.powers[1].remaining&&state.items[0].charges.current==1);act(16);click("Riepilogo");click("Tipi");click("Conto");click("Profilo");click("Costi");click("Spesa 1/3 >");assert(damage_details.costs[1].before==2&&!damage_details.costs[1].after);screenshot("multi-cost-receipt.ppm");click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* A missing secondary resource blocks the whole event and every projected balance. */
    state.magic.powers[1].remaining=1;act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Usa");click("Usa profilo");click("Usa valori");click("Anteprima");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;act(1805);act(1860);assert(page==DAMAGE_CONFIRM&&save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));screenshot("multi-cost-blocked.ppm");click("Tipi");click("Conto");click("Profilo");click("Costi");assert(damage_details.costs[0].before==damage_details.costs[0].after);click("Spesa 1/3 >");assert(damage_details.costs[1].missing&&damage_details.costs[1].before==damage_details.costs[1].after);screenshot("multi-cost-missing-second.ppm");click("Spesa 2/3 >");assert(damage_details.costs[2].before==damage_details.costs[2].after);click("Indietro");click("Indietro");click("Indietro");click("Riepilogo");click("Indietro");act(100);
    /* Empty middle position, third-position maximum and full name remain readable. */
    pd_demo(&state);page=HERO;state.items[0].charges=(pd_charges){999,999,0};strcpy(state.items[0].name,"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");assert(pd_reduction_profile_set(&state,0,(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.extra_costs={{0},{PD_COST_ITEM,0,999}}}));act(1800);click("Riduci");click("Profili");click("Profilo 1 | attivo >");click("Costi");click("Spesa 1/3 >");assert(!pd_profile_cost(&state.reductions[0],profile_cost_row).kind);click("Spesa 2/3 >");bounds();wrapped_fits();screenshot("multi-cost-long-name.ppm");

    click("Indietro");click("Indietro");click("Indietro");click("Usa valori");act(100);effects_before=state;assert(stage_import(&imported_magic));screenshot("multi-cost-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");for(unsigned i=0;i<3;i++)for(unsigned row=1;row<3;row++)assert(!pd_profile_cost(&state.reductions[i],row).kind);
    /* Explicit item recharge: preview draws no entropy; save precedes animation. */
    pd_demo(&state);page=HERO;state.items[0].charges=(pd_charges){2,7,0};act(500);click("Cariche");click("Ricarica");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;bounds();wrapped_fits();screenshot("recharge-setup.ppm");assert(recharge_plan.count==1&&recharge_plan.sides==6&&!recharge_plan.bonus);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==ITEM_RECHARGE&&!stage_import(&imported_magic));act(2174);act(2176);assert(recharge_plan.count==2&&recharge_plan.bonus==1);click("Anteprima");bounds();screenshot("recharge-confirm.ppm");assert(page==ITEM_RECHARGE_CONFIRM&&random_calls==effects_calls&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");click("Indietro");assert(page==ITEM_CHARGES&&!memcmp(&state,&effects_before,sizeof state));
    click("Ricarica");click("Anteprima");random_values[0]=random_values[1]=5;fail_save=true;click("Tira e salva");assert(page==ITEM_RECHARGE_CONFIRM&&recharge_rolled&&random_calls==effects_calls+1&&save_calls==effects_saves+1&&hit_candidate.items[0].charges.current==7&&!memcmp(&state,&effects_before,sizeof state));bounds();screenshot("recharge-retry.ppm");act(2178);act(2172);act(2174);act(100);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);assert(page==ITEM_RECHARGE_CONFIRM&&!stage_import(&imported_magic)&&random_calls==effects_calls+1&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Riprova");assert(page==DICE&&combat_roll==7&&dice_animation.active&&state.items[0].charges.current==7&&state.items[0].quantity==1&&state.hp==effects_before.hp&&random_calls==effects_calls+1);screenshot("recharge-flight.ppm");act(2180);assert(page==DICE&&random_calls==effects_calls+1);click("Accelera");act(13);assert(page==DICE&&has_roll&&!dice_animation.active&&random_calls==effects_calls+1);bounds();screenshot("recharge-result.ppm");click("Riepilogo");assert(recharge_before==2&&recharge_after==7&&recharge_gained==5&&recharge_result.total==6&&!strcmp(recharge_name,"Spada"));bounds();wrapped_fits();screenshot("recharge-receipt.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);assert(page==ITEM_RECHARGE_RESULT&&!stage_import(&imported_magic));click("Cariche");effects_saves=save_calls;effects_calls=random_calls;act(2170);assert(page==ITEM_CHARGES&&random_calls==effects_calls&&save_calls==effects_saves);screenshot("recharge-full.ppm");click("Indietro");click("Zaino");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* A zero recovery does not write or replace Undo; the event is not retried automatically. */
    act(500);click("Cariche");click("Ricarica");recharge_plan=(pd_recharge_plan){1,4,-15};render();click("Anteprima");effects_before=state;pd_state recharge_previous=previous;bool recharge_undo=undo_available;effects_saves=save_calls;effects_calls=random_calls;random_values[0]=random_values[1]=0;click("Tira e salva");assert(page==DICE&&dice_animation.active&&random_calls==effects_calls+1&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state)&&undo_available==recharge_undo&&!memcmp(&previous,&recharge_previous,sizeof previous));act(16);click("Riepilogo");assert(!recharge_gained&&!recharge_result.total&&recharge_result.sum==1);screenshot("recharge-zero.ppm");click("Fine");assert(page==HERO);
    /* The last row, six d20, maximum bonus, full name and 180-charge candidate. */
    pd_demo(&state);page=HERO;state.item_count=60;for(unsigned i=3;i<60;i++)state.items[i]=(pd_item){.name="Oggetto sintetico",.quantity=1};strcpy(state.items[59].name,"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW");state.items[59].charges=(pd_charges){0,999,2};act(559);click("Cariche");click("Ricarica");recharge_plan=(pd_recharge_plan){6,20,60};render();bounds();wrapped_fits();screenshot("recharge-maximum-setup.ppm");act(2174);act(2176);assert(recharge_plan.count==6&&recharge_plan.bonus==60);click("Anteprima");effects_calls=random_calls;random_values[0]=random_values[1]=19;click("Tira e salva");assert(state.items[59].charges.current==180&&random_calls==effects_calls+6);act(16);bounds();screenshot("recharge-maximum-result.ppm");click("Riepilogo");bounds();wrapped_fits();assert(recharge_gained==180&&recharge_after==180);screenshot("recharge-maximum-receipt.ppm");click("Cariche");click("Ricarica");
    /* Each supported die and both limits can be selected without rolling or saving. */
    for(unsigned i=0;i<6;i++)act(2172);assert(recharge_plan.sides==6);for(unsigned i=0;i<8;i++)act(2173);for(unsigned i=0;i<80;i++)act(2175);assert(recharge_plan.count==1&&recharge_plan.bonus==PD_POOL_MIN_BONUS);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Indietro");click("Indietro");click("Zaino");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    /* A missing object cannot open a recharge or request any RNG. */
    state.items[59].quantity=0;act(559);click("Cariche");act(2170);assert(page==ITEM_CHARGES&&random_calls==effects_calls&&save_calls==effects_saves);screenshot("recharge-unavailable.ppm");click("Indietro");click("Zaino");
    /* Low canvas memory still displays the saved result with no second roll. */
    pd_demo(&state);page=HERO;state.items[0].charges=(pd_charges){2,7,1};act(500);click("Cariche");click("Ricarica");click("Anteprima");effects_calls=random_calls;random_values[0]=random_values[1]=1;fail_canvas=true;click("Tira e salva");assert(page==DICE&&has_roll&&!dice_animation.active&&state.items[0].charges.current==4&&random_calls==effects_calls+1);screenshot("recharge-no-canvas.ppm");fail_canvas=false;click("Riepilogo");assert(recharge_gained==2);click("Cariche");click("Indietro");click("Zaino");act(100);

    /* Mechanical roll effects are explicit, separate from notes, atomic and undoable. */
    pd_demo(&state);page=HERO;assert(pd_condition_add(&state,"Accecato",0));act(1503);click("Tiri");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;click("Accecato: no >");click("Spaventato: no >");click("Prono: no >");click("Trattenuto: no >");assert(condition_effects_draft==15&&!state.roll_conditions);bounds();screenshot("roll-conditions-editor.ppm");click("Anteprima");bounds();screenshot("roll-conditions-confirm.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==CONDITION_EFFECTS_CONFIRM&&!stage_import(&imported_magic)&&save_calls==effects_saves&&random_calls==effects_calls);fail_save=true;click("Conferma");assert(page==CONDITION_EFFECTS_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.roll_conditions==15&&state.status.count==1&&state.hp==effects_before.hp);click("Indietro");click("Indietro");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* A note alone never blocks a sight-related check. Unknown sight blocks before RNG. */
    act(203);effects_calls=random_calls;act(13);assert(page==DICE&&random_calls==effects_calls+1);act(16);assert(pd_condition_effects_set(&state,PD_BLINDED));act(203);effects_before=state;effects_calls=random_calls;effects_saves=save_calls;act(13);act(72);assert(!dice_animation.active&&random_calls==effects_calls&&save_calls==effects_saves);screenshot("roll-conditions-blind-block.ppm");click("Contesto");assert(page==ROLL_CONTEXT);bounds();screenshot("roll-conditions-context-unknown.ppm");for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(page==ROLL_CONTEXT&&!stage_import(&imported_magic));click("Richiede vista: ? >");click("Richiede vista: no >");click("Usa contesto");assert(page==DICE);act(13);assert(random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state)&&!dice_canvas&&!dice_pixels&&!has_roll);bounds();screenshot("roll-conditions-auto-fail.ppm");act(70);assert(page==CHECK_DETAIL);bounds();screenshot("roll-conditions-auto-detail.ppm");act(72);assert(random_calls==effects_calls);click("Contesto");click("Richiede vista: si >");click("Richiede vista: ? >");click("Usa contesto");assert(page==CHECK_DETAIL);random_values[0]=random_values[1]=9;click("Lancia");assert(page==DICE&&random_calls==effects_calls+1&&result.count==1);act(16);screenshot("roll-conditions-no-sight.ppm");
    /* Explicit DM exclusion applies to the roll only, without toggling stored effects. */
    click("Contesto");click("Richiede vista: no >");click("Effetti tiro: applicati >");screenshot("roll-conditions-exclude.ppm");click("Usa contesto");effects_calls=random_calls;act(13);assert(random_calls==effects_calls+1&&result.count==1&&state.roll_conditions==PD_BLINDED);act(16);
    /* Fear needs its source fact only when it can change the outcome. */
    assert(pd_condition_effects_set(&state,PD_FRIGHTENED));act(200);effects_calls=random_calls;act(13);assert(random_calls==effects_calls);click("Contesto");click("Fonte paura in vista: ? >");click("Fonte paura in vista: no >");bounds();screenshot("roll-conditions-fear-visible.ppm");click("Usa contesto");random_values[0]=0;random_values[1]=19;act(13);assert(random_calls==effects_calls+2&&result.count==2&&result.natural==1);act(16);screenshot("roll-conditions-fear-roll.ppm");click("Base");effects_calls=random_calls;act(13);assert(random_calls==effects_calls+1&&result.count==1);act(16);
    /* Prone + frightened + poison impose one disadvantage; manual advantage cancels once. */
    pd_demo(&state);page=HERO;state.training.poisoned=true;assert(pd_condition_effects_set(&state,PD_PRONE|PD_FRIGHTENED));act(81);effects_calls=random_calls;bounds();screenshot("roll-conditions-attack.ppm");act(72);assert(page==DICE&&random_calls==effects_calls+2&&strike.roll.count==2);act(16);act(81);click("Svantaggio");effects_calls=random_calls;act(72);assert(page==DICE&&random_calls==effects_calls+1&&strike.roll.count==1);act(16);screenshot("roll-conditions-cancelled.ppm");
    /* Frightened-only attack is blocked with unknown source, including the detail action. */
    state.training.poisoned=false;assert(pd_condition_effects_set(&state,PD_FRIGHTENED));act(81);effects_calls=random_calls;act(72);assert(random_calls==effects_calls&&page==COMBAT);click("Contesto");click("Fonte paura in vista: ? >");click("Usa contesto");assert(page==COMBAT);act(93);bounds();screenshot("roll-conditions-attack-detail.ppm");act(72);assert(page==DICE&&random_calls==effects_calls+1);act(16);
    /* Restrained affects DEX saves, but not CON saves or the free d20. */
    assert(pd_condition_effects_set(&state,PD_RESTRAINED));act(241);effects_calls=random_calls;act(13);assert(random_calls==effects_calls+2&&result.count==2);act(16);screenshot("roll-conditions-dex-save.ppm");act(242);effects_calls=random_calls;act(13);assert(random_calls==effects_calls+1&&result.count==1);act(16);assert(pd_condition_effects_set(&state,15));act(14);effects_calls=random_calls;act(13);assert(random_calls==effects_calls+1&&result.count==1);act(16);
    /* Cancelled import preserves local flags; confirmed import clears only the device copy. */
    act(100);effects_before=state;assert(stage_import(&imported_magic));screenshot("roll-conditions-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_magic));click("Conferma");assert(!state.roll_conditions);

    /* Feedback observes committed balances in the new home, never requested deltas. */
    pd_demo(&state);page=HERO;combat_roll=0;act(100);effects_calls=random_calls;effects_saves=save_calls;
    click("-1 PF");assert(page==HERO&&feedback.active&&feedback.kind==FEEDBACK_HURT&&!strcmp(feedback.text,"PF -1")&&companion_feedback_visible()&&!feedback_panel&&feedback_timer&&save_calls==effects_saves+1&&random_calls==effects_calls);
    bounds();screenshot("feedback-damage.ppm");effects_before=state;int hero_initial_x=lv_obj_get_x(avatar_sprite);lv_tick_inc(160);lv_timer_handler();assert(lv_obj_get_x(avatar_sprite)!=hero_initial_x);
    for(unsigned i=0;i<10;i++){lv_tick_inc(80);lv_timer_handler();char frame[48];snprintf(frame,sizeof frame,"feedback-frame-%02u.ppm",i);screenshot(frame);}
    lv_tick_inc(2100);lv_timer_handler();assert(!feedback.active&&!companion_feedback_visible()&&!feedback_timer&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves+1&&random_calls==effects_calls);screenshot("feedback-expired.ppm");
    click("+1 PF");assert(feedback.active&&feedback.kind==FEEDBACK_HEAL&&!strcmp(feedback.text,"PF +1")&&companion_feedback_visible());screenshot("feedback-heal.ppm");
    effects_before=state;fail_save=true;click("-1 PF");assert(page==HOME_VITALS&&!feedback.active&&!companion_feedback_visible()&&!memcmp(&state,&effects_before,sizeof state));screenshot("feedback-save-error.ppm");fail_save=false;
    state.temp_hp=3;act(100);click("-1 PF");assert(feedback.kind==FEEDBACK_HURT&&!strcmp(feedback.text,"Temp -1")&&state.hp==effects_before.hp&&companion_feedback_visible()&&avatar_phase==3);screenshot("feedback-temporary.ppm");
    /* Reduced animation keeps a static receipt, with the same automatic expiry. */
    state.idle_animation=false;act(100);click("+1 PF");assert(feedback.active&&feedback_timer&&!avatar_timer&&avatar_phase==0);int static_hero_y=lv_obj_get_y(avatar_sprite);lv_tick_inc(500);lv_timer_handler();assert(lv_obj_get_y(avatar_sprite)==static_hero_y&&companion_feedback_visible());screenshot("feedback-static.ppm");lv_tick_inc(1600);lv_timer_handler();assert(!feedback.active&&!companion_feedback_visible());
    state.hp=state.max_hp;effects_saves=save_calls;act(2);assert(!feedback.active&&save_calls==effects_saves);
    /* Independent pools give an update receipt, rather than a sum of unlike units. */
    pd_demo(&state);act(103);click("-1");assert(feedback.kind==FEEDBACK_SPEND&&!strcmp(feedback.text,"Risorse utilizzate"));screenshot("feedback-spend.ppm");click("+1");assert(feedback.kind==FEEDBACK_RECOVER&&!strcmp(feedback.text,"Risorse recuperate"));screenshot("feedback-recover.ppm");effects_saves=save_calls;act(21);assert(!feedback.active&&save_calls==effects_saves+1);
    /* Capabilities can be limited by kind even without the spell-only LIMITED flag. */
    act(100);state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Capacita sintetica",.kind=1,.remaining=2,.maximum=2};power_index=0;page=MAGIC_CONFIRM;act(832);assert(feedback.active&&feedback.kind==FEEDBACK_SPEND&&state.magic.powers[0].remaining==1);page=MAGIC_RECOVER;act(849);assert(feedback.kind==FEEDBACK_RECOVER&&state.magic.powers[0].remaining==2);
    slot_index=0;slot_pact=false;state.magic.slots[0]=(pd_slot){1,2};page=MAGIC_SLOTS;act(841);assert(feedback.kind==FEEDBACK_RECOVER);act(840);assert(feedback.kind==FEEDBACK_SPEND);
    /* Navigation/import/dimming clean cosmetic objects without confirming anything. */
    act(100);click("-1 PF");act(80);assert(!feedback_panel&&!feedback_timer&&!companion_feedback_visible());screenshot("fantasy-inventory.ppm");state=imported_magic;state.magic.count=3;state.magic.powers[0]=(pd_power){.name="Luce sintetica",.flags=PD_PREPARED};state.magic.powers[1]=(pd_power){.name="Scudo sintetico",.flags=PD_PREPARED|PD_USES_SLOT,.level=1};state.magic.powers[2]=(pd_power){.name="Slancio sintetico",.kind=1,.flags=PD_LIMITED,.remaining=1,.maximum=2};magic_filter=2;act(800);screenshot("fantasy-magic.ppm");act(100);act(1);assert(companion_feedback_visible());assert(stage_import(&imported_magic)&&!feedback.active&&!companion_feedback_visible());click("Annulla");act(1);assert(companion_feedback_visible());
    state.dim_timeout=1;state.brightness=50;display_screen.touch_down=false;display_screen.activity=lv_tick_get()-30001;screen_service(-1,0);assert(display_screen.dimmed&&!companion_feedback_visible()&&!feedback_timer&&avatar_phase==0);pd_screen_activity(&display_screen,lv_tick_get());screen_service(-1,0);assert(!display_screen.dimmed&&avatar_phase==0);
    /* Expiry is safe across uint32 clock wrap and still never writes or rolls. */
    lv_tick_inc(UINT32_MAX-lv_tick_get()-100);act(1);assert(companion_feedback_visible());effects_before=state;effects_saves=save_calls;effects_calls=random_calls;lv_tick_inc(2100);lv_timer_handler();assert(!companion_feedback_visible()&&!feedback_timer&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);

    /* Every main-menu icon reaches its real production destination without a roll/save. */
    pd_demo(&state);page=HERO;combat_roll=0;pd_dice_idle(&dice_animation,1);act(100);
    const char *menu_names[]={"Attacco","Dadi","Magie","Zaino","Riposo","Scheda","Usi"};
    const page_t menu_pages[]={COMBAT,DICE,MAGIC_LIST,INVENTORY,REST_HOME,SHEET,RESOURCES};
    effects_saves=save_calls;effects_calls=random_calls;
    for(unsigned i=0;i<7;i++){act(100);click("Menu");assert(page==HOME_MENU);if(!i)screenshot("home-menu.ppm");click(menu_names[i]);assert(page==menu_pages[i]&&save_calls==effects_saves&&random_calls==effects_calls);}
    state.equipped=PD_NO_WEAPON;act(100);click("Menu");click("Attacco");assert(page==INVENTORY);screenshot("home-no-weapon.ppm");
    act(100);tap(200,104);assert(page==HOME_VITALS);screenshot("home-vitals.ppm");tap(305,391);
    /* The existing 13 bindings keep their meanings on all three new views. */
    const page_t bound_pages[]={HERO,HERO,SHEET,DICE,INVENTORY,MAGIC_LIST,DICE,REST_HOME,STATUS_HOME,DEVICE_SETTINGS,EFFECT_LIST,EFFECT_ROUND,DAMAGE_HOME};
    for(unsigned screen=0;screen<3;screen++)for(unsigned binding=0;binding<13;binding++)for(unsigned gesture=0;gesture<3;gesture++){
        page=(const page_t[]){HERO,HOME_MENU,HOME_VITALS}[screen];state.shortcuts[gesture]=binding;render();
        page_t expected=binding&&(binding!=11||timed_effects())?bound_pages[binding]:page;shortcut(gesture);
        if(page!=expected||save_calls!=effects_saves||random_calls!=effects_calls)fprintf(stderr,"Shortcut screen=%u binding=%u gesture=%u got=%u expected=%u saves=%u/%u rng=%u/%u\n",screen,binding,gesture,page,expected,save_calls,effects_saves,random_calls,effects_calls);
        assert(page==expected&&save_calls==effects_saves&&random_calls==effects_calls);
    }
    /* Pending import, item confirmation and in-flight dice cannot be bypassed by new events. */
    page=HERO;render();click("Menu");effects_before=state;assert(stage_import(&imported_magic));companion_event(COMPANION_OPEN_HOME,NULL);act(2300);act(2301);assert(import_pending&&page==HOME_MENU);click("Annulla");assert(page==HOME_MENU&&!memcmp(&state,&effects_before,sizeof state));
    assert(stage_import(&imported_magic));fail_save=true;click("Conferma");assert(!import_pending&&page==HOME_MENU&&!memcmp(&state,&effects_before,sizeof state));screenshot("home-import-error.ppm");fail_save=false;
    assert(stage_import(&imported_magic));click("Conferma");assert(page==HERO&&!memcmp(state.shortcuts,effects_before.shortcuts,3));screenshot("home-imported.ppm");
    page=ITEM_CONFIRM;item_index=1;render();effects_saves=save_calls;companion_event(COMPANION_OPEN_HOME,NULL);act(2300);act(2301);assert(page==ITEM_CONFIRM&&save_calls==effects_saves);page=HERO;render();click("Dadi");act(14);click("Lancia");effects_calls=random_calls;companion_event(COMPANION_OPEN_MENU,NULL);act(2300);assert(page==DICE&&dice_animation.active&&random_calls==effects_calls);click("Accelera");
    /* New PF controls use adjusted maximum/armor, preserve undo, and reject disabled saves. */
    pd_demo(&state);state.effects[0]=(pd_effect){.active=true,.target=PD_EFFECT_MAX_HP,.value=10};state.effects[1]=(pd_effect){.active=true,.target=PD_EFFECT_ARMOR,.value=2};state.effect_count=2;state.hp=41;act(100);screenshot("home-effects.ppm");tap(200,104);assert(page==HOME_VITALS);screenshot("home-vitals-effects.ppm");click("+1 PF");assert(state.hp==42&&pd_armor(&state)==18);click("Annulla");assert(state.hp==41);tap(233,264);assert(page==APPEARANCE);click("Tasti");assert(page==KEY_SETTINGS);screenshot("home-keys.ppm");
    act(100);state.status.available=true;state.status.failures=3;state.hp=state.temp_hp=0;render();tap(200,104);effects_saves=save_calls;
    bool found_disabled_heal=false;root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){
        lv_obj_t *b=lv_obj_get_child(root,i);
        if(lv_obj_check_type(b,&lv_button_class)&&button_label(b)&&!strcmp(lv_label_get_text(button_label(b)),"+1 PF")){
            assert(lv_obj_has_state(b,LV_STATE_DISABLED));lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);found_disabled_heal=true;break;
        }
    }
    assert(found_disabled_heal);assert(page==HOME_VITALS&&!state.hp&&save_calls==effects_saves);screenshot("home-zero.ppm");

    /* The new header must offer real navigation without bypassing a draft.
       A disabled Menu also rejects a forged click, just like physical shortcuts. */
    pd_demo(&state);feedback.active=false;combat_roll=0;page=HERO;render();
    const page_t inspect[]={INVENTORY,MAGIC_LIST,REST_HOME,SHEET,RESOURCES,APPEARANCE,KEY_SETTINGS,DEVICE_SETTINGS,STATUS_HOME,EFFECT_LIST,DAMAGE_HOME};
    effects_saves=save_calls;effects_calls=random_calls;effects_before=state;
    for(unsigned i=0;i<sizeof inspect/sizeof inspect[0];i++){
        page=inspect[i];render();click("Menu");assert(page==HOME_MENU);
        assert(save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));
    }
    page=ITEM_CONFIRM;item_index=1;render();root=lv_screen_active();bool disabled_menu=false;
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){
        lv_obj_t *b=lv_obj_get_child(root,i);
        if(lv_obj_check_type(b,&lv_button_class)&&button_name(b)&&!strcmp(button_name(b),"Menu")){
            assert(lv_obj_has_state(b,LV_STATE_DISABLED));lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);disabled_menu=true;break;
        }
    }
    assert(disabled_menu&&page==ITEM_CONFIRM&&save_calls==effects_saves&&random_calls==effects_calls);
    state.equipped=PD_NO_WEAPON;
    const page_t missing_weapon[]={COMBAT,COMBAT_DETAIL};
    for(unsigned i=0;i<2;i++){
        page=missing_weapon[i];render();assert(page==WEAPON_PICKER);bounds();
        assert(save_calls==effects_saves&&random_calls==effects_calls);
    }
    /* Transitions finish, cancel on redraw/sleep, and never consume game state. */
    pd_demo(&state);feedback.active=false;page=HERO;render();click("Menu");click("Zaino");
    assert(lv_anim_count_running()>0);
    for(unsigned i=0;i<5;i++){char file[48];snprintf(file,sizeof file,"ux-transition-%02u.ppm",i);screenshot(file);lv_tick_inc(40);lv_timer_handler();}
    assert(!lv_anim_count_running());
    click("Menu");click("Magie");assert(lv_anim_count_running()>0);render();assert(!lv_anim_count_running());
    state.idle_animation=false;page=INVENTORY;render();assert(!lv_anim_count_running());state.idle_animation=true;
    page=MAGIC_LIST;render();assert(lv_anim_count_running()>0);pd_theme_pause();assert(!lv_anim_count_running());
    /* Money: unknown is distinct from verified zero; drafts never spend/save. */
    pd_demo(&state);feedback.active=false;combat_roll=0;act(80);click("Monete");assert(page==WALLET_HOME&&!state.wallet.available);screenshot("wallet-unknown.ppm");
    effects_saves=save_calls;effects_calls=random_calls;effects_before=state;
    click("Configura saldo");click("+");assert(page==WALLET_EDIT&&wallet_draft.coins[0]==1);
    shortcut(0);act(100);act(2400);assert(page==WALLET_EDIT&&!stage_import(&effects_before)&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    click("Annulla");assert(page==WALLET_HOME&&!state.wallet.available);click("Configura saldo");click("Anteprima");assert(page==WALLET_CONFIRM);screenshot("wallet-confirm-zero.ppm");
    click("Conferma");assert(state.wallet.available&&page==WALLET_HOME&&feedback.kind==FEEDBACK_WALLET&&save_calls==effects_saves+1&&wallet_can_undo());screenshot("wallet-zero.ppm");
    fail_save=true;click("Annulla monete");assert(state.wallet.available&&wallet_can_undo()&&!feedback.active);fail_save=false;
    click("Annulla monete");assert(!state.wallet.available&&page==WALLET_HOME&&save_calls==effects_saves+3&&!undo_available);
    effects_before=state;effects_saves=save_calls;
    click("Configura saldo");const unsigned balances[]={5,2,0,12,1};
    for(unsigned coin=0;coin<5;coin++){
        for(unsigned amount=0;amount<balances[coin];amount++)click("+");
        if(coin<4)click(">");
    }
    screenshot("wallet-edit.ppm");click("Anteprima");screenshot("wallet-confirm.ppm");
    fail_save=true;click("Conferma");assert(page==WALLET_CONFIRM&&!state.wallet.available&&!memcmp(&state,&effects_before,sizeof state)&&!feedback.active);screenshot("wallet-save-error.ppm");fail_save=false;
    click("Indietro");assert(wallet_draft.coins[3]==12);click("Anteprima");click("Conferma");assert(page==WALLET_HOME&&state.wallet.available&&pd_wallet_copper(&state.wallet)==2225&&feedback.kind==FEEDBACK_WALLET&&random_calls==effects_calls);
    for(unsigned i=0;i<5;i++)assert(state.wallet.coins[i]==balances[i]);
    screenshot("wallet-home.ppm");click(">");screenshot("wallet-gold.ppm");
    /* No-op confirmations and a stale last-game undo never write a coin balance. */
    act(1);assert(!wallet_can_undo());effects_saves=save_calls;effects_before=state;click("Oro: 12 MO");click("Anteprima");
    root=lv_screen_active();lv_obj_update_layout(root);bool no_op=false;
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++){
        lv_obj_t *b=lv_obj_get_child(root,i);
        if(lv_obj_check_type(b,&lv_button_class)&&button_label(b)&&!strcmp(lv_label_get_text(button_label(b)),"Conferma")){
            assert(lv_obj_has_state(b,LV_STATE_DISABLED));lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);no_op=true;break;
        }
    }
    assert(no_op&&page==WALLET_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));act(2409);
    /* Maximum amounts remain readable; a forged + at the cap cannot overflow. */
    for(unsigned i=0;i<5;i++)state.wallet.coins[i]=PD_COIN_MAX;wallet_page=0;render();screenshot("wallet-maximum.ppm");
    click("Rame: 999999999 MR");effects_before=state;act(2407);assert(wallet_draft.coins[0]==PD_COIN_MAX&&!memcmp(&state,&effects_before,sizeof state));
    for(unsigned i=0;i<8;i++)act(2408);click("-");assert(wallet_draft.coins[0]==899999999);act(2409);assert(state.wallet.coins[0]==PD_COIN_MAX);
    /* USB import explicitly replaces local money only after confirmation. */
    pd_demo(&pending);pending.wallet=(pd_wallet){.available=true,.coins={1,2,3,42,5}};pd_state wallet_import=pending;
    state.shortcuts[0]=12;state.shortcuts[1]=4;state.shortcuts[2]=5;
    assert(stage_import(&wallet_import));screenshot("wallet-import.ppm");click("Annulla");assert(state.wallet.coins[3]==PD_COIN_MAX);
    assert(stage_import(&wallet_import));click("Conferma");assert(page==HERO&&state.wallet.coins[3]==42&&state.shortcuts[0]==12&&state.shortcuts[1]==4&&state.shortcuts[2]==5);
    /* Persistent history uses independent writes, before animation, never new RNG. */
    assert(history_seen==((1u<<PD_H_KINDS)-1));
    pd_demo(&state);pd_history_reset(&history,0);persisted_history=history;history_pending=false;fail_history=false;act(14);effects_saves=save_calls;effects_calls=random_calls;
    click("Storico");assert(page==HISTORY_HOME&&!history.count);screenshot("history-empty.ppm");bounds();click("Indietro");
    click("Lancia");assert(history.count==1&&persisted_history.count==1&&dice_animation.active&&save_calls==effects_saves);pd_history_entry frozen=history.entries[0];
    act(2600);assert(page==DICE&&dice_animation.active);click("Accelera");assert(!memcmp(&frozen,&history.entries[0],sizeof frozen));
    click("Storico");act(2700);assert(page==HISTORY_DETAIL);screenshot("history-detail.ppm");bounds();
    effects_calls=random_calls;effects_saves=save_calls;unsigned journal_writes=history_save_calls;render();bounds();assert(random_calls==effects_calls&&save_calls==effects_saves&&history_save_calls==journal_writes);
    pd_history rebooted;assert(pd_history_store_load(state.history_epoch,&rebooted)&&!memcmp(&history,&rebooted,sizeof history));
    click("Dadi");fail_history=true;click("Lancia");assert(history_pending&&history.count==1&&dice_animation.active);pd_history_entry pending_roll=history_unsaved;click("Accelera");screenshot("history-unsaved-roll.ppm");
    effects_calls=random_calls;effects_saves=save_calls;act(13);assert(page==HISTORY_HOME&&random_calls==effects_calls&&save_calls==effects_saves&&history_pending);screenshot("history-pending.ppm");bounds();
    act(1);assert(history_pending&&random_calls==effects_calls);effects_saves=save_calls;pd_state unchanged=state;assert(!stage_import(&unchanged));act(2609);assert(page==HISTORY_DETAIL);screenshot("history-unsaved-detail.ppm");bounds();
    click("Riprova");assert(history_pending&&random_calls==effects_calls&&history.count==1);fail_history=false;click("Riprova");assert(!history_pending&&history.count==2&&history.entries[0].total==pending_roll.total&&!memcmp(history.entries[0].dice,pending_roll.dice,18)&&random_calls==effects_calls&&save_calls==effects_saves);
    act(2700);click("Dadi");fail_history=true;click("Lancia");click("Accelera");click("Storico");click("Scarta");assert(page==HISTORY_DISCARD);screenshot("history-discard.ppm");bounds();
    shortcut(0);act(100);assert(page==HISTORY_DISCARD&&history_pending);click("Indietro");assert(history_pending);click("Scarta");click("Scarta");assert(!history_pending&&history.count==2&&!memcmp(&state,&unchanged,sizeof state));fail_history=false;
    /* All eighteen faces, wide imported names and pagination fit the display. */
    pd_pool eighteen={.count=18,.sides=20,.sum=360,.bonus=60,.total=420,.damage=true};for(unsigned i=0;i<18;i++)eighteen.dice[i]=20;
    journal_pool(PD_H_DAMAGE,"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW",&eighteen,"Danni tirati; PF bersaglio non modificati");act(2700);screenshot("history-eighteen.ppm");bounds();wrapped_fits();click(">");click(">");assert(history_dice_page==2);screenshot("history-last-dice.ppm");bounds();click("Elenco");
    for(unsigned i=0;i<25;i++)journal_d20(PD_H_FREE,"d20 libero",&(pd_roll){.dice={20},.count=1,.natural=20,.total=20},0,"Dado libero");render();assert(history.count==20);screenshot("history-full.ppm");
    while((history_page+1)*3<history.count)act(2602);screenshot("history-oldest.ppm");bounds();
    act(850);state.shortcuts[0]=12;render();click("PWR breve: Danni");click("Storico");assert(state.shortcuts[0]==13);screenshot("history-key.ppm");shortcut(0);assert(page==HISTORY_HOME);effects_calls=random_calls;effects_saves=save_calls;shortcut(0);assert(page==HISTORY_HOME&&history_return!=HISTORY_HOME&&random_calls==effects_calls&&save_calls==effects_saves);
    unsigned old_sequence=history.sequence;uint32_t old_epoch=state.history_epoch;pd_demo(&pending);pd_state history_import=pending;
    assert(stage_import(&history_import));click("Annulla");assert(history.sequence==old_sequence&&state.history_epoch==old_epoch);
    assert(stage_import(&history_import));fail_save=true;click("Conferma");assert(history.sequence==old_sequence&&state.history_epoch==old_epoch);fail_save=false;
    assert(stage_import(&history_import));click("Conferma");assert(!history.count&&state.history_epoch==old_epoch+1&&state.shortcuts[0]==13);act(14);click("Storico");screenshot("history-import-empty.ppm");bounds();state.history_epoch=UINT32_MAX;assert(!stage_import(&history_import));
    /* Native backup never changes state on export, cancel, timeout or prepare failure. */
    pd_demo(&state);pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;page=HERO;render();act(1600);click("Backup");assert(page==BACKUP_HOME);screenshot("backup-home.ppm");bounds();
    static pd_backup snapshot;pd_demo(&snapshot.state);snapshot.state.hp=7;snapshot.state.temp_hp=3;snapshot.state.shortcuts[0]=13;snapshot.state.brightness=80;snapshot.state.history_epoch=123;
    snapshot.state.wallet=(pd_wallet){.available=true,.coins={5,4,3,2,1}};pd_history_reset(&snapshot.history,123);
    pd_history_entry archived={.kind=PD_H_FREE,.count=1,.sides=20,.dice={20},.total=20,.label="Tiro archiviato"};assert(pd_history_append(&snapshot.history,&archived));
    effects_saves=save_calls;effects_calls=random_calls;effects_before=state;unsigned before_journal=history_save_calls;
    assert(stage_backup(NULL)&&page==BACKUP_SEND_CONFIRM);screenshot("backup-send.ppm");bounds();act(1);shortcut(0);assert(page==BACKUP_SEND_CONFIRM&&!stage_import(&snapshot.state)&&!stage_backup(&snapshot)&&!memcmp(&effects_before,&state,sizeof state));click("Annulla");
    assert(save_calls==effects_saves&&history_save_calls==before_journal&&random_calls==effects_calls);
    assert(stage_backup(NULL));fail_backup_send=true;click("Invia");assert(page==BACKUP_HOME&&strstr(backup_notice,"fallito")&&!memcmp(&state,&effects_before,sizeof state));screenshot("backup-send-error.ppm");fail_backup_send=false;
    assert(stage_backup(NULL));click("Invia");assert(page==BACKUP_HOME&&save_calls==effects_saves&&history_save_calls==before_journal&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));screenshot("backup-sent.ppm");
    lv_tick_inc(UINT32_MAX-lv_tick_get()-60000);assert(stage_backup(NULL));lv_tick_inc(119999);backup_service();assert(page==BACKUP_SEND_CONFIRM);lv_tick_inc(1);backup_service();assert(page==BACKUP_HOME&&save_calls==effects_saves);screenshot("backup-timeout.ppm");
    snapshot.history.epoch=124;assert(!stage_backup(&snapshot));snapshot.history.epoch=123;
    assert(stage_backup(&snapshot));assert(page==BACKUP_RESTORE_CONFIRM&&pending.history_epoch==state.history_epoch+1&&backup_history.epoch==pending.history_epoch);screenshot("backup-restore.ppm");bounds();click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    assert(stage_backup(&snapshot));fail_history=true;click("Ripristina");assert(page==BACKUP_RESTORE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("backup-history-error.ppm");fail_history=false;
    fail_save=true;click("Ripristina");assert(page==BACKUP_RESTORE_CONFIRM&&!prepared_ready&&!memcmp(&state,&effects_before,sizeof state));screenshot("backup-state-error.ppm");assert(history_quarantined);click("Annulla");assert(!stage_backup(&snapshot));fail_save=false;pd_history_store_load(state.history_epoch,&history);assert(stage_backup(&snapshot));
    uint32_t restored_epoch=state.history_epoch+1;click("Ripristina");assert(page==HERO&&state.hp==7&&state.temp_hp==3&&state.shortcuts[0]==13&&state.brightness==80&&state.wallet.coins[0]==5&&state.history_epoch==restored_epoch&&history.epoch==restored_epoch&&history.count==1&&history.entries[0].total==20&&!undo_available&&random_calls==effects_calls);screenshot("backup-restored.ppm");
    pd_history_entry not_saved=archived;history_unsaved=not_saved;history_pending=true;assert(!stage_backup(NULL)&&!stage_backup(&snapshot));history_pending=false;
    /* Read-only diagnostics must not resolve rolls, wake the display, consume
       input, dismiss confirmations, or attempt either NVS store. */
    effects_before=state;pd_history diagnostic_history=history;
    static pd_state diagnostic_pending,diagnostic_previous;
    diagnostic_pending=pending;diagnostic_previous=previous;
    effects_saves=save_calls;effects_calls=random_calls;before_journal=history_save_calls;
    for(unsigned context=0;context<8;context++){
        page=context==1?BACKUP_RESTORE_CONFIRM:HERO;
        import_pending=context==2;dice_animation.active=context==3;
        display_screen.dimmed=context==4;display_screen.failed=context==5;
        fail_save=history_quarantined=context==5;history_pending=context==6;
        history.epoch=context==7?state.history_epoch+1:state.history_epoch;
        pd_screen screen_before=display_screen;pd_dice_animation animation_before=dice_animation;
        pd_history history_before=history;pd_history_entry unsaved_before=history_unsaved;
        bool undo_before=undo_available;page_t page_before=page;
        pd_diagnostics observation;collect_diagnostics(&observation);report_diagnostics();
        assert(observation.busy==(context==1||context==2||context==3));
        assert(observation.dimmed==(context==4)&&observation.screen_failed==(context==5));
        assert(observation.state_writable==(context!=5)&&observation.history_available==(context!=5));
        assert(observation.history_pending==(context==6)&&observation.history_matches==(context!=7)&&observation.state_valid);
        assert(!strncmp(last_reply,"PDD DIAGNOSTICS {",17)&&last_reply[strlen(last_reply)-1]=='\n');
        assert(!strstr(last_reply,state.name)&&!strstr(last_reply,"history_epoch")&&!strstr(last_reply,"dice\""));
        assert(page==page_before&&undo_available==undo_before&&!memcmp(&display_screen,&screen_before,sizeof screen_before)&&!memcmp(&dice_animation,&animation_before,sizeof animation_before));
        assert(!memcmp(&state,&effects_before,sizeof state)&&!memcmp(&pending,&diagnostic_pending,sizeof pending)&&!memcmp(&previous,&diagnostic_previous,sizeof previous));
        assert(!memcmp(&history,&history_before,sizeof history)&&!memcmp(&history_unsaved,&unsaved_before,sizeof history_unsaved));
        assert(save_calls==effects_saves&&random_calls==effects_calls&&history_save_calls==before_journal);
        if(!context){FILE *f=fopen("diagnostics-live.json","wb");assert(f);size_t n=strlen(last_reply+16)-1;assert(fwrite(last_reply+16,1,n,f)==n);fclose(f);}
    }
    history=diagnostic_history;fail_save=history_quarantined=history_pending=import_pending=false;
    dice_animation.active=display_screen.dimmed=display_screen.failed=false;page=HERO;
    puts("Diagnostics UI snapshot: eight normal/busy/dimmed/error/pending/mismatched contexts, no state/history/NVS/RNG/input effects (synthetic)");
    /* Device-local input checks: actual LVGL pointer events and production
       key controller. They must never become gameplay actions or NVS writes. */
    pd_demo(&state);state.shortcuts[0]=6;state.shortcuts[1]=4;state.shortcuts[2]=13;state.dim_timeout=1;
    pd_history_reset(&history,state.history_epoch);history_pending=false;
    display_screen=(pd_screen){0};physical_keys=(pd_buttons){0};has_roll=false;
    effects_before=state;diagnostic_history=history;diagnostic_pending=pending;diagnostic_previous=previous;
    effects_saves=save_calls;effects_calls=random_calls;before_journal=history_save_calls;
    act(850);assert(page==KEY_SETTINGS);screenshot("input-check-entry.ppm");click("Collaudo");
    assert(page==INPUT_CHECK_HOME&&confirmation_open());screenshot("input-check-home.ppm");
    assert(!stage_import(&snapshot.state)&&!strcmp(last_reply,"PDD ERROR busy\n"));
    assert(!stage_backup(NULL)&&!stage_backup(&snapshot));
    act(1);act(100);act(13);act(850);shortcut(0);assert(page==INPUT_CHECK_HOME);
    pd_diagnostics checking;collect_diagnostics(&checking);assert(checking.busy);
    click("Prova tocco");assert(page==INPUT_CHECK_TOUCH&&input_check.touched==0);screenshot("input-check-touch.ppm");
    act(2909);assert(!input_check.touched);click("1");assert(!input_check.touched); /* No pointer event: no observation. */
    lv_indev_t *probe=lv_indev_create();assert(probe);lv_indev_set_type(probe,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(probe,probe_touch);lv_indev_set_display(probe,d);
    probe_click(probe,328,234);probe_click(probe,90,210);assert(!input_check.touched);
    probe_sample(probe,233,170,true,20);probe_sample(probe,233,170,true,1000);assert(!input_check.touched);
    probe_sample(probe,233,170,false,20);assert(input_check.touched==1);
    /* Releasing after a drag off the active target must not accept it. */
    probe_sample(probe,328,234,true,20);probe_sample(probe,233,298,true,20);probe_sample(probe,233,298,false,20);assert(input_check.touched==1);
    for(unsigned i=1;i<PD_INPUT_CHECK_TARGETS;i++){
        probe_click(probe,input_target_centers[i][0],input_target_centers[i][1]);
        assert(input_check.touched==i+1);char image[48];snprintf(image,sizeof image,"input-check-touch-%u.ppm",i+1);screenshot(image);
    }
    assert(input_check.touched==5);screenshot("input-check-touch-done.ppm");
    probe_click(probe,233,234);assert(input_check.touched==5);click("Azzera");assert(!input_check.touched);
    act(2902);assert(page==INPUT_CHECK_TOUCH);click("Indietro");assert(page==INPUT_CHECK_HOME);
    click("Prova tasti");assert(page==INPUT_CHECK_KEYS);
    probe_key_sample(1,1,0);probe_key_sample(1,1,1000);probe_key_sample(0,0,0);probe_key_sample(0,0,31);
    assert(!input_check.counts[0]&&!input_check.counts[1]&&!input_check.counts[2]);
    click("Azzera");probe_key_sample(0,0,0);screenshot("input-check-keys.ppm");
    probe_key_sample(-1,0,0);assert(!input_check.available[0]);screenshot("input-check-unavailable.ppm");
    for(unsigned i=0;i<3;i++){probe_key_gesture(i,i==2?800:100);assert(page==INPUT_CHECK_KEYS&&input_check.counts[i]==1);}
    probe_key_gesture(0,900);assert(input_check.counts[0]==1);screenshot("input-check-keys-done.ppm");
    for(unsigned i=0;i<3;i++)input_check.counts[i]=PD_INPUT_CHECK_COUNT_MAX;
    input_check_refresh();bounds();screenshot("input-check-maximum.ppm");
    for(unsigned i=0;i<3;i++){probe_key_gesture(i,i==2?800:100);assert(input_check.counts[i]==999);}
    assert(!has_roll&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));
    click("Azzera");assert(!input_check.counts[0]&&!input_check.counts[1]&&!input_check.counts[2]);
    /* A held BOOT at exit cannot invoke its configured Zaino shortcut. */
    probe_key_sample(0,0,0);probe_key_sample(0,0,31);probe_key_sample(0,1,0);probe_key_sample(0,1,31);
    click("Indietro");click("Esci");assert(page==KEY_SETTINGS&&input_check.mode==PD_INPUT_IDLE);
    probe_key_sample(0,1,1000);probe_key_sample(0,0,0);probe_key_sample(0,0,31);assert(page==KEY_SETTINGS);
    probe_key_gesture(1,100);assert(page==INVENTORY); /* A fresh gesture works again. */
    for(unsigned view=0;view<3;view++){
        act(850);click("Collaudo");
        if(view==1)click("Prova tocco");if(view==2)click("Prova tasti");
        if(view==2){history_pending=true;probe_key_sample(0,1,0);}
        probe_sample(probe,233,180,false,20);probe_sample(probe,233,180,true,20);
        /* Check absolute timeout across the LVGL clock wrap in each view. */
        lv_tick_inc(UINT32_MAX-lv_tick_get()-60000);input_check.started=lv_tick_get();
        lv_tick_inc(119999);assert(!input_check_service()&&input_check_open());
        lv_tick_inc(1);assert(input_check_service()&&page==KEY_SETTINGS&&input_check.mode==PD_INPUT_IDLE);
        assert(display_screen.swallow_keys&&display_screen.swallow_touch&&strstr(input_check_notice,"scaduta"));
        /* After timeout that same coordinate is a binding button. A contact
           carried from the test must not reconfigure it on release. */
        probe_sample(probe,233,180,true,20);probe_sample(probe,233,180,false,20);
        assert(!display_screen.swallow_touch&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);
        probe_key_sample(0,1,1000);probe_key_sample(0,0,0);probe_key_sample(0,0,31);assert(page==KEY_SETTINGS);
        if(view==2){assert(history_pending);history_pending=false;}
        screenshot("input-check-timeout.ppm");
    }
    probe_sample(probe,233,234,false,20);lv_indev_delete(probe);
    assert(save_calls==effects_saves&&history_save_calls==before_journal&&random_calls==effects_calls);
    assert(!memcmp(&state,&effects_before,sizeof state)&&!memcmp(&history,&diagnostic_history,sizeof history));
    assert(!memcmp(&pending,&diagnostic_pending,sizeof pending)&&!memcmp(&previous,&diagnostic_previous,sizeof previous));
    import_pending=true;act(2900);assert(page==KEY_SETTINGS);import_pending=false;
    dice_animation.active=true;act(2900);assert(page==KEY_SETTINGS);dice_animation.active=false;
    page=MAGIC_CONFIRM;act(2900);assert(page==MAGIC_CONFIRM);page=HERO;
    puts("Local input check: real LVGL pointer stream, five targets/drag/held/reset, production key gestures/held exit, busy USB, unchanged state/history/preferences/NVS/RNG and all three timeout/wrap views passed (synthetic)");
    /* Explicit death resolution is separate from the ordinary damage button. */
    pd_demo(&state);state.hp=0;state.temp_hp=3;state.status.available=true;state.status.successes=2;state.status.failures=1;state.defences.configured=8191;
    pd_history_reset(&history,state.history_epoch);history_pending=false;
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;before_journal=history_save_calls;
    act(1800);click("Anteprima");click("Risolvi registro morte");assert(page==DAMAGE_DEATH&&confirmation_open());screenshot("death-damage-choice.ppm");
    act(3004);assert(save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));shortcut(0);act(100);assert(page==DAMAGE_DEATH);
    click("Base 2014: da verificare");act(3004);assert(death_damage_choice.verified&&save_calls==effects_saves);
    click("Colpo critico: da verificare >");assert(death_damage_choice.critical==1&&!death_damage_choice.verified);
    click("Base 2014: da verificare");click("Tipi");assert(page==DAMAGE_TYPES&&damage_details_return==DAMAGE_DEATH);click("Riepilogo");assert(page==DAMAGE_DEATH&&death_damage_choice.verified);
    fail_save=true;click("Conferma");assert(page==DAMAGE_DEATH&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);screenshot("death-damage-save-error.ppm");
    fail_save=false;click("Conferma");assert(page==DAMAGE_RESULT&&state.hp==0&&state.temp_hp==2&&state.status.failures==2&&state.status.successes==2&&incoming_death_used&&incoming_death.failures_added==1);screenshot("death-damage-result.ppm");
    unsigned death_writes=save_calls;act(3004);assert(save_calls==death_writes);act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    act(1800);click("Anteprima");click("Risolvi registro morte");click("Colpo critico: da verificare >");click("Colpo critico: no >");click("Base 2014: da verificare");
    click("Indietro");click("Risolvi registro morte");assert(!death_damage_choice.critical&&!death_damage_choice.verified); /* No stale acknowledgement. */
    click("Colpo critico: da verificare >");click("Colpo critico: no >");click("Base 2014: da verificare");screenshot("death-damage-critical.ppm");click("Conferma");
    assert(state.status.failures==3&&state.temp_hp==2&&incoming_death.impact.total.adjusted==1&&incoming_death.failures_added==2);
    effects_before=state;death_writes=save_calls;act(2);assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==death_writes);
    pd_demo(&state);state.hp=0;state.temp_hp=9;state.status.available=true;state.status.stabilized=true;assert(pd_defence_set(&state,0,true,PD_RESISTANCE));
    act(1800);damage_plan.amount=3;render();click("Anteprima");click("Risolvi registro morte");click("Colpo critico: da verificare >");click("Base 2014: da verificare");screenshot("death-damage-stable.ppm");click("Conferma");
    assert(!state.status.stabilized&&state.status.failures==1&&state.temp_hp==8);
    /* Instant threshold includes all three costs and ends linked effects in the
       same saved candidate. Failed writes and undo preserve the whole impact. */
    pd_demo(&state);state.status.available=true;state.temp_hp=3;state.defences.configured=8191;state.magic.available=true;state.magic.count=1;
    state.magic.powers[0]=(pd_power){.name="Magia originale",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_ARMOR,.value=2,.active=true,.concentration=1}));
    state.magic.slots[0]=(pd_slot){2,3};state.items[1].charges=(pd_charges){2,3,0};
    state.reductions[0]=(pd_reduction_profile){.value=1,.types=8191,.active=true,.stage=1,.reaction=true,.cost_kind=PD_COST_COUNTER,.cost=1,.extra_costs={{PD_COST_SLOT,0,1},{PD_COST_ITEM,1,1}}};
    assert(pd_valid(&state));effects_before=state;effects_saves=save_calls;
    act(1800);damage_plan.amount=60;damage_profile=1;render();click("Anteprima");click("Risolvi registro morte");
    assert(page==DAMAGE_DEATH);click("Base 2014: da verificare");screenshot("death-damage-instant.ppm");
    fail_save=true;click("Conferma");assert(page==DAMAGE_DEATH&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;
    click("Conferma");assert(page==DAMAGE_RESULT&&incoming_death.instant&&!state.hp&&!state.temp_hp&&state.status.failures==3&&!state.magic.concentration&&!state.effects[0].active);
    assert(!state.resources[0].current&&state.magic.slots[0].current==1&&state.items[1].charges.current==1&&state.reaction_spent&&save_calls==effects_saves+2);screenshot("death-damage-instant-result.ppm");
    assert(incoming_event.costs[0].before==1&&incoming_event.costs[0].after==0&&incoming_event.costs[1].after==1&&incoming_event.costs[2].after==1);
    act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Missing/contradictory registries and a reaction at zero stay blocked. */
    for(unsigned problem=0;problem<4;problem++){
        pd_demo(&state);state.hp=0;state.defences.configured=8191;state.status.available=problem!=0;
        if(problem==1)state.status.failures=3;
        if(problem==2){state.status.failures=1;state.status.stabilized=true;}
        if(problem==3)state.reductions[0]=(pd_reduction_profile){.value=1,.types=8191,.active=true,.stage=1,.reaction=true};
        act(1800);damage_plan.amount=2;if(problem==3)damage_profile=1;render();click("Anteprima");click("Risolvi registro morte");
        effects_before=state;death_writes=save_calls;act(3002);act(3003);act(3004);assert(page==DAMAGE_DEATH&&save_calls==death_writes&&!memcmp(&state,&effects_before,sizeof state));
        char file[64];snprintf(file,sizeof file,"death-damage-blocked-%u.ppm",problem);screenshot(file);click("Indietro");click("Indietro");
    }
    /* Immunity and ordinary descent to zero never add a death failure. */
    pd_demo(&state);state.hp=0;state.status.available=true;assert(pd_defence_set(&state,0,true,PD_IMMUNITY));
    act(1800);click("Anteprima");act(3000);assert(page==DAMAGE_CONFIRM);death_writes=save_calls;click("Conferma");assert(page==DAMAGE_RESULT&&!incoming_death_used&&!state.status.failures&&save_calls==death_writes);
    pd_demo(&state);state.hp=1;state.status.available=true;state.status.failures=2;state.defences.configured=8191;
    act(1800);click("Anteprima");act(3000);assert(page==DAMAGE_CONFIRM);click("Conferma");assert(!state.hp&&!state.status.failures&&!incoming_death_used);
    /* All six largest vulnerable components remain one lethal impact. */
    pd_demo(&state);state.hp=state.max_hp=999;state.temp_hp=999;state.status.available=true;state.defences.configured=state.defences.vulnerability=8191;
    act(1800);damage_plan.amount=9999;damage_extra_count=5;for(unsigned i=1;i<6;i++)damage_extra[i-1]=(pd_damage_component){i,9999};render();
    click("Anteprima");click("Risolvi registro morte");click("Base 2014: da verificare");screenshot("death-damage-maximum.ppm");click("Conferma");assert(incoming_death.instant&&incoming_result.adjusted==119988&&state.status.failures==3&&!state.hp&&!state.temp_hp);
    assert(random_calls==effects_calls&&history_save_calls==before_journal);
    puts("Death damage UI: explicit critical/base acknowledgement, normal/stable/instant/dead/legacy/reaction cases, six parts, costs/linked effects, save failure/undo, frozen receipt, duplicate guard and zero RNG/history writes passed (synthetic)");
    /* v0.57: explicit death-save circumstances, signed bonus and frozen retry. */
    pd_demo(&state);state.hp=0;state.temp_hp=3;state.status.available=true;state.status.successes=1;state.status.failures=1;
    state.training.poisoned=true;state.roll_conditions=PD_ROLL_CONDITIONS;
    pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;fail_history=false;
    effects_before=state;unsigned roll_calls=random_calls,roll_saves=save_calls,roll_history_saves=history_save_calls;
    act(1501);click("Modifica");assert(page==DEATH_ROLL_SETUP&&confirmation_open());screenshot("death-roll-setup.ppm");
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));shortcut(0);act(100);assert(page==DEATH_ROLL_SETUP);
    click("Vantaggio: no");click("Svantaggio: no");for(unsigned i=0;i<9;i++)click("+1");
    assert(!pd_death_mode(status_options)&&status_options.bonus==9);screenshot("death-roll-cancelled.ppm");
    click("Anteprima");assert(page==STATUS_CONFIRM&&status_return_page==DEATH_ROLL_SETUP);screenshot("death-roll-preview-cancelled.ppm");
    click("Indietro");assert(page==DEATH_ROLL_SETUP&&status_options.bonus==9);click("Indietro");assert(page==DEATH_VIEW&&!status_options.bonus&&!status_options.advantage&&!status_options.disadvantage);
    assert(random_calls==roll_calls&&save_calls==roll_saves&&history_save_calls==roll_history_saves&&!memcmp(&state,&effects_before,sizeof state));
    click("Modifica");click("Vantaggio: no");click("+1");click("Anteprima");screenshot("death-roll-preview.ppm");
    random_values[random_calls%2]=0;random_values[(random_calls+1)%2]=8;fail_save=true;click("Conferma");
    assert(page==STATUS_CONFIRM&&status_rolled&&random_calls==roll_calls+2&&!memcmp(&state,&effects_before,sizeof state));
    assert(status_result.natural==9&&status_result.total==10&&status_result.count==2&&status_result.bonus==1);
    screenshot("death-roll-save-error.ppm");act(1506);act(3101);assert(page==STATUS_CONFIRM&&status_rolled&&status_options.advantage);
    click("Riprova");assert(random_calls==roll_calls+2&&history_save_calls==roll_history_saves);
    fail_save=false;click("Riprova");assert(page==DICE&&combat_roll==5&&state.status.successes==2&&state.status.failures==1&&state.temp_hp==3);
    assert(history_save_calls==roll_history_saves+1&&history.entries[0].mode==1&&history.entries[0].bonus==1&&history.entries[0].total==10&&history.entries[0].dice[0]==1&&history.entries[0].dice[1]==9);
    act(1505);assert(random_calls==roll_calls+2);click("Accelera");assert(status_outcome==1&&result.natural==9&&result.total==10);screenshot("death-roll-advantage.ppm");
    act(13);assert(random_calls==roll_calls+2);click("Eroe");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state)&&history.count==1);
    act(1501);click("Modifica");assert(!status_options.bonus&&!status_options.advantage);click("Svantaggio: no");click("-1");click("Anteprima");
    roll_calls=random_calls;random_values[random_calls%2]=18;random_values[(random_calls+1)%2]=9;click("Conferma");click("Accelera");
    assert(random_calls==roll_calls+2&&result.natural==10&&result.total==9&&status_outcome==0&&state.status.failures==2&&state.status.successes==1);
    assert(history.entries[0].mode==-1&&history.entries[0].bonus==-1&&history.entries[0].total==9);screenshot("death-roll-disadvantage.ppm");
    /* Both signs cancel to ONE die; a natural 20 revives even with total -10. */
    click("Stato");click("Modifica");click("Vantaggio: no");click("Svantaggio: no");for(unsigned i=0;i<30;i++)click("-1");
    act(3103);assert(status_options.bonus==-30);screenshot("death-roll-minimum.ppm");click("Anteprima");
    roll_calls=random_calls;random_values[random_calls%2]=19;click("Conferma");click("Accelera");
    assert(random_calls==roll_calls+1&&result.count==1&&result.natural==20&&result.total==-10&&status_outcome==4&&state.hp==1&&!state.status.failures&&!state.status.successes);
    assert(history.entries[0].mode==0&&history.entries[0].bonus==-30&&history.entries[0].total==-10);screenshot("death-roll-twenty.ppm");
    /* The receipt stays attached to the resolved roll, independently of later state. */
    state.hp=0;state.status.failures=3;render();assert(status_outcome==4);state.hp=1;state.status.failures=0;
    click("Eroe");click("Annulla");assert(!state.hp&&state.status.failures==2);
    act(1501);click("Modifica");for(unsigned i=0;i<30;i++)click("+1");act(3104);assert(status_options.bonus==30);screenshot("death-roll-maximum.ppm");click("Anteprima");
    random_values[random_calls%2]=0;click("Conferma");click("Accelera");assert(result.natural==1&&result.total==31&&state.status.failures==3&&status_outcome==3);screenshot("death-roll-one.ppm");
    /* No canvas still resolves and journals once; third ordinary success stabilizes. */
    click("Eroe");pd_demo(&state);state.hp=0;state.status.available=true;state.status.successes=2;state.status.failures=2;
    act(1501);click("Modifica");click("+1");click("Anteprima");random_values[random_calls%2]=8;
    roll_calls=random_calls;fail_canvas=true;click("Conferma");assert(!dice_animation.active&&has_roll);screenshot("death-roll-no-canvas.ppm");fail_canvas=false;
    assert(random_calls==roll_calls+1&&state.status.stabilized&&!state.status.successes&&!state.status.failures&&status_outcome==2);
    /* A missing/dead/stable/positive-HP registry cannot enter either roll path. */
    click("Eroe");
    for(unsigned blocked=0;blocked<5;blocked++){
        pd_demo(&state);state.hp=0;state.status.available=true;
        if(blocked==0)state.status.available=false;
        if(blocked==1)state.status.failures=3;
        if(blocked==2)state.status.stabilized=true;
        if(blocked==3)state.hp=1;
        if(blocked==4)state.status.successes=3;
        act(1501);effects_before=state;roll_calls=random_calls;roll_saves=save_calls;act(3100);act(1510);
        assert(page==DEATH_VIEW&&random_calls==roll_calls&&save_calls==roll_saves&&!memcmp(&state,&effects_before,sizeof state));
    }
    /* Historical pending write prevents a second roll before RNG; retry saves it once. */
    pd_demo(&state);state.hp=0;state.status.available=true;act(1501);click("Modifica");click("Vantaggio: no");click("Anteprima");
    fail_history=true;random_values[0]=random_values[1]=9;click("Conferma");click("Accelera");assert(history_pending&&state.status.successes==1);
    roll_calls=random_calls;click("Stato");click("Lancia d20 base");click("Conferma");assert(page==HISTORY_HOME&&history_pending&&random_calls==roll_calls&&state.status.successes==1);
    fail_history=false;click("Riprova");assert(!history_pending&&history.entries[0].kind==PD_H_DEATH&&history.entries[0].mode==1&&random_calls==roll_calls);
    puts("Death roll UI: explicit advantage/disadvantage/cancellation/bonus, preview/cancel, natural 1/20 priority, signed totals, stable/dead/legacy blocks, frozen write retry, undo, canvas failure and journal retry passed (synthetic)");
    /* v0.58: explicit external turn references and isolated timer expiry. */
    pd_demo(&state);state.reaction_spent=true;state.hp=32;
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=7,.tick=PD_TICK_END}));state.hp=42;
    effect_index=0;effect_removing=false;effect_draft=state.effects[0];page=EFFECT_DURATION;render();effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    click("Turno: mio >");assert(effect_draft.owner==1&&effect_draft.rounds==1);screenshot("external-turn-effect.ppm");
    click("Anteprima");assert(page==EFFECT_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("external-turn-effect-confirm.ppm");
    click("Conferma");assert(state.effects[0].owner==1&&state.effects[0].rounds==1&&state.hp==42);
    assert(pd_effect_set(&state,1,(pd_effect){.value=2,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START,.owner=2}));
    assert(pd_effect_set(&state,2,(pd_effect){.value=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START}));
    assert(pd_defence_grant_set(&state,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=1}));
    assert(pd_reduction_profile_set(&state,2,(pd_reduction_profile){.value=1,.types=8191,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_END,.owner=1}));
    /* The same subject selectors are available for defences and profiles. */
    defence_index=12;defence_slot=2;defence_temporary_draft=*pd_defence_grant(&state,12,2);page=DEFENCE_DURATION;render();
    click("Turno: A >");assert(defence_temporary_draft.owner==2&&defence_temporary_draft.rounds==1);screenshot("external-turn-defence.ppm");
    act(1877);assert(!defence_temporary_draft.owner); /* Switching clock removes external ownership. */
    profile_index=2;profile_draft=state.reductions[2];profile_number_rounds=true;profile_number_cost=false;page=PROFILE_NUMBER;render();
    click("Turno: A >");click("Turno: B >");assert(profile_draft.owner==3&&profile_draft.rounds==1);screenshot("external-turn-profile.ppm");
    act(2085);assert(page==PROFILE_DETAILS);act(2077);assert(!profile_draft.owner);
    /* A/B/C selection is read-only; ordinary start/end remain the player's own. */
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");assert(page==EXTERNAL_TURN_HOME&&turn_owner==1&&confirmation_open());screenshot("external-turn-home.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));shortcut(0);act(100);assert(page==EXTERNAL_TURN_HOME);
    click("B");click("C");click("A");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    click("Inizio turno A");assert(page==REACTION_CONFIRM);screenshot("external-turn-empty.ppm");click("Conferma");assert(page==EXTERNAL_TURN_HOME&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    click("Fine turno A");screenshot("external-turn-preview.ppm");click("Scadenze");assert(page==TURN_DETAILS);screenshot("external-turn-details.ppm");click("Riepilogo");click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state));
    click("Fine turno A");fail_save=true;click("Conferma");assert(page==REACTION_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("external-turn-save-error.ppm");fail_save=false;click("Conferma");
    assert(page==EXTERNAL_TURN_HOME&&state.hp==32&&pd_max_hp(&state)==32&&!state.effects[0].active&&state.effects[1].rounds==2&&state.effects[2].active&&state.reaction_spent);
    assert(!pd_defence_grant(&state,12,2)->active&&state.reductions[2].rounds==1&&random_calls==effects_calls);
    act(2122);assert(page==EXTERNAL_TURN_HOME&&state.reductions[2].rounds==1); /* No queued duplicate confirmation. */
    click("Indietro");click("Indietro");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");click("B");click("Inizio turno B");click("Conferma");assert(state.effects[1].rounds==1&&state.effects[2].active&&state.reaction_spent&&state.hp==42);
    click("Indietro");click("Inizio turno");click("Conferma");assert(!state.effects[2].active&&!state.reaction_spent&&state.effects[1].rounds==1&&state.effects[0].active);
    /* Backup/restore preserves references, including inactive definitions. */
    snapshot.state=state;pd_history_reset(&snapshot.history,state.history_epoch);size_t external_size;char *external_json=pd_backup_encode(&state,&snapshot.history,&external_size);assert(external_json);
    assert(pd_backup_decode(external_json,external_size,&snapshot)&&snapshot.state.effects[1].owner==2&&snapshot.state.reductions[2].owner==1);free(external_json);
    /* All 50 eligible timers use the selected external edge, including the last bit. */
    pd_demo(&state);state.reaction_spent=true;state.effect_count=8;
    for(unsigned i=0;i<8;i++)state.effects[i]=(pd_effect){.value=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=3};
    for(unsigned slot=0;slot<3;slot++)for(unsigned type=0;type<13;type++)assert(pd_defence_grant_set(&state,type,slot,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=3}));
    for(unsigned i=0;i<3;i++)assert(pd_reduction_profile_set(&state,i,(pd_reduction_profile){.value=1,.types=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=3}));
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");click("C");click("Fine turno C");click("Scadenze");for(unsigned i=0;i<16;i++)click(">");screenshot("external-turn-last.ppm");assert(turn_detail_page==16);click("Riepilogo");click("Conferma");assert(state.reaction_spent&&pd_armor(&state)==state.armor&&!state.effects[7].active&&!state.reductions[2].active);
    pd_demo(&state);state.armor=20;state.reaction_spent=true;
    assert(pd_effect_set(&state,0,(pd_effect){.value=20,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_START,.owner=1}));assert(pd_effect_set(&state,1,(pd_effect){.value=-20,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_START,.owner=2}));
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");click("Inizio turno A");effects_before=state;effects_saves=save_calls;act(2122);assert(page==REACTION_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));screenshot("external-turn-invalid.ppm");
    puts("External turn UI: subject editors, clock reset, three references, own/external reaction isolation, preview/details/cancel, save retry/undo, empty no-write, all 50 expiries and native backup passed (synthetic)");
    /* v0.59: names label stable references, never rebind or advance their timers. */
    pd_demo(&state);state.reaction_spent=true;state.hp=32;
    assert(pd_effect_set(&state,0,(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=1}));state.hp=42;
    assert(pd_defence_grant_set(&state,12,2,(pd_defence_temporary){.flags=1,.active=true,.timed=true,.rounds=1,.tick=PD_TICK_END,.owner=1}));
    assert(pd_reduction_profile_set(&state,2,(pd_reduction_profile){.value=1,.types=8191,.active=true,.timed=true,.rounds=2,.tick=PD_TICK_END,.owner=1}));
    pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");click("A: senza nome >");assert(page==TURN_NAME_EDIT&&confirmation_open());screenshot("turn-name-empty.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;unsigned name_history_saves=history_save_calls;
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));shortcut(0);act(100);assert(page==TURN_NAME_EDIT);
    type_turn_name("Ally A");assert(!strcmp(name_draft,"Ally A")&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);screenshot("turn-name-editor.ppm");
    click("Anteprima");assert(page==TURN_NAME_CONFIRM);wrapped_fits();screenshot("turn-name-confirm.ppm");click("Indietro");click("Cancella");type_turn_name("B");click("Indietro");assert(page==EXTERNAL_TURN_HOME&&!state.turn_names[0][0]&&save_calls==effects_saves);
    click("A: senza nome >");type_turn_name("Ally A");click("Anteprima");fail_save=true;click("Conferma");assert(page==TURN_NAME_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("turn-name-save-error.ppm");act(3304);assert(page==TURN_NAME_CONFIRM);
    fail_save=false;click("Conferma");assert(page==EXTERNAL_TURN_HOME);strcpy(effects_before.turn_names[0],"Ally A");assert(!memcmp(&state,&effects_before,sizeof state));
    effects_saves=save_calls;act(3307);assert(save_calls==effects_saves&&page==EXTERNAL_TURN_HOME);screenshot("turn-name-home.ppm");
    click("Fine turno A");wrapped_fits();screenshot("turn-name-preview.ppm");click("Indietro");click("Indietro");click("Indietro");act(100);click("Annulla");
    effects_before.turn_names[0][0]=0;memset(effects_before.turn_names[0],0,PD_TURN_NAME_BYTES);assert(!memcmp(&state,&effects_before,sizeof state));
    page=STATUS_HOME;render();click("Reazione");click("Altri turni");click("B");click("B: senza nome >");type_turn_name("WWWWWWWWWWWWWWWWWWWW");act(3400);assert(strlen(name_draft)==20);wrapped_fits();screenshot("turn-name-maximum.ppm");
    while(name_key_page+1<name_pages())click(">");screenshot("turn-name-last-page.ppm");click("Anteprima");wrapped_fits();screenshot("turn-name-maximum-confirm.ppm");click("Conferma");assert(strlen(state.turn_names[1])==20);
    click("C");click("C: senza nome >");type_turn_name("Mage-3");click("Anteprima");click("Conferma");assert(!strcmp(state.turn_names[2],"Mage-3"));
    click("C: Mage-3 >");click("Anteprima");effects_saves=save_calls;effects_before=previous;click("Conferma");assert(save_calls==effects_saves&&!memcmp(&previous,&effects_before,sizeof previous));
    click("C: Mage-3 >");while(name_draft[0])click("Cancella");type_turn_name("  ");click("Anteprima");assert(!name_draft[0]);screenshot("turn-name-clear.ppm");click("Conferma");assert(!state.turn_names[2][0]&&strlen(state.turn_names[1])==20);
    click("A");click("A: senza nome >");type_turn_name("The Mage");click("Anteprima");click("Conferma");click("Fine turno A");wrapped_fits();screenshot("turn-name-named-event.ppm");click("Conferma");
    assert(state.hp==32&&!state.effects[0].active&&state.reductions[2].rounds==1&&state.reaction_spent&&!strcmp(state.turn_names[0],"The Mage"));
    click("Indietro");click("Indietro");act(100);click("Annulla");assert(state.hp==42&&state.effects[0].active&&state.reductions[2].rounds==2&&!strcmp(state.turn_names[0],"The Mage"));
    snapshot.state=state;pd_history_reset(&snapshot.history,state.history_epoch);char *named_json=pd_backup_encode(&state,&snapshot.history,&external_size);assert(named_json&&pd_backup_decode(named_json,external_size,&snapshot));free(named_json);
    assert(!strcmp(snapshot.state.turn_names[0],"The Mage")&&strlen(snapshot.state.turn_names[1])==20&&snapshot.state.effects[0].owner==1&&random_calls==effects_calls&&history_save_calls==name_history_saves);
    assert(pd_turn_name_set(&state,1,"Changed"));assert(stage_backup(&snapshot));click("Annulla");assert(!strcmp(state.turn_names[0],"Changed"));
    assert(stage_backup(&snapshot));click("Ripristina");assert(page==HERO&&!strcmp(state.turn_names[0],"The Mage")&&strlen(state.turn_names[1])==20&&state.effects[0].owner==1&&state.hp==42);screenshot("turn-name-restored.ppm");
    puts("Turn name UI: all keyboard pages, case/digits/space, full-length wrapping, preview/cancel, failed write/retry, unchanged-save no-write, clear, undo, named timer preview and native backup passed (synthetic)");
    /* v0.60: explicitly choose exactly one alternative payment for each hit. */
    pd_demo(&state);state.hp=state.max_hp=30;state.temp_hp=3;state.defences.configured=8191;state.resources[0].maximum=state.resources[0].current=3;
    state.magic.available=true;state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Limited power",.kind=1,.maximum=3,.remaining=1};state.items[2].quantity=1;state.items[2].charges=(pd_charges){0,3,0};
    pd_reduction_profile alternative={.value=3,.types=8191,.stage=1,.active=true,.cost_kind=PD_COST_COUNTER,.cost=2};
    assert(pd_reduction_profile_set(&state,0,alternative));profile_index=0;profile_cost_row=0;profile_cost_editing=true;profile_draft=alternative;page=PROFILE_COST;render();
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;unsigned alternative_history=history_save_calls;
    click("Tutte >");assert(profile_draft.payment==PD_PAYMENT_CHOOSE&&!state.reductions[0].payment);click("Indietro");click("Anteprima");screenshot("alternative-incomplete.ppm");act(2088);assert(page==PROFILE_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    pd_profile_cost_set(&alternative,1,(pd_cost){PD_COST_POWER,0,2});pd_profile_cost_set(&alternative,2,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&state,0,alternative));
    profile_draft=alternative;profile_cost_row=0;profile_cost_editing=true;page=PROFILE_COST;render();click("Tutte >");screenshot("alternative-editor.ppm");click("Indietro");click("Anteprima");click("Dati >");click("Regola >");click("Durata >");screenshot("alternative-profile-confirm.ppm");
    effects_before=state;fail_save=true;click("Conferma");assert(page==PROFILE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.reductions[0].payment==PD_PAYMENT_CHOOSE);
    page=STATUS_HOME;render();act(1800);damage_profile=1;damage_plan.amount=5;click("Anteprima");assert(page==DAMAGE_CONFIRM&&!damage_payment_choice);screenshot("alternative-unresolved.ppm");
    effects_before=state;effects_saves=save_calls;act(1805);assert(save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));click("Spesa: da scegliere >");assert(page==PAYMENT_CHOICE&&confirmation_open());screenshot("alternative-choices.ppm");
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));shortcut(0);act(100);act(3503);act(3504);assert(page==PAYMENT_CHOICE&&!damage_payment_choice&&save_calls==effects_saves);
    click("1. Slancio");assert(page==DAMAGE_CONFIRM&&damage_payment_choice==1&&!memcmp(&state,&effects_before,sizeof state));screenshot("alternative-selected.ppm");act(3500);click("Indietro");assert(damage_payment_choice==1);
    fail_save=true;click("Conferma");assert(page==DAMAGE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("alternative-save-error.ppm");fail_save=false;click("Conferma");
    assert(page==DAMAGE_RESULT&&state.resources[0].current==1&&state.magic.powers[0].remaining==1&&!state.items[2].charges.current&&state.temp_hp==1&&state.hp==30);
    assert(incoming_event.payment_choice==1&&incoming_event.costs[0].selected&&!incoming_event.costs[1].selected&&incoming_event.costs[1].before==incoming_event.costs[1].after&&random_calls==effects_calls&&history_save_calls==alternative_history);
    screenshot("alternative-result.ppm");act(1805);assert(state.resources[0].current==1);click("Tipi");act(2050);act(2090);act(2105);assert(page==PROFILE_COST_IMPACT);screenshot("alternative-paid.ppm");click("Spesa 1/3 >");screenshot("alternative-not-chosen.ppm");
    state.magic.powers[0].remaining=0;state.resources[0].current=0;strcpy(state.resources[0].name,"Changed");state.reductions[0].payment=PD_PAYMENT_ALL;render();assert(damage_details.payment_choice==1&&damage_details.costs[0].before==3&&damage_details.costs[0].after==1&&!strcmp(damage_details.costs[0].name,"Slancio"));
    page=HERO;render();click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Explicitly changing choice is allowed before the hit; reaction remains common. */
    state.magic.powers[0].remaining=3;state.items[2].charges.current=3;state.reductions[0].reaction=true;state.reaction_spent=true;
    page=STATUS_HOME;render();act(1800);damage_profile=1;damage_plan.amount=5;click("Anteprima");act(3500);click("2. Limited power");assert(damage_payment_choice==2);screenshot("alternative-reaction-block.ppm");
    click("Gestisci reazione");click("Inizio turno");click("Conferma");click("Indietro");assert(page==DAMAGE_CONFIRM&&damage_payment_choice==2&&!state.reaction_spent);click("Conferma");assert(state.magic.powers[0].remaining==1&&state.resources[0].current==3&&state.items[2].charges.current==3&&state.reaction_spent);
    /* A long source name is clipped only in the compact picker, not the receipt. */
    pd_demo(&state);state.hp=state.max_hp=30;state.temp_hp=3;state.defences.configured=8191;state.resources[0].maximum=state.resources[0].current=3;memset(state.resources[0].name,'W',32);state.resources[0].name[32]=0;state.items[2].quantity=0;state.items[2].charges=(pd_charges){3,3,0};
    alternative=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.cost_kind=PD_COST_COUNTER,.cost=2,.payment=PD_PAYMENT_CHOOSE};pd_profile_cost_set(&alternative,2,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&state,0,alternative));
    page=STATUS_HOME;render();act(1800);damage_profile=1;damage_plan.amount=5;click("Anteprima");act(3500);screenshot("alternative-hole-long.ppm");act(3503);act(3504);assert(page==PAYMENT_CHOICE&&!damage_payment_choice);act(3502);assert(page==DAMAGE_CONFIRM&&damage_payment_choice==1);
    /* Concentration retry freezes payment and RNG together. */
    pd_demo(&state);state.hp=state.max_hp=30;state.temp_hp=3;state.defences.configured=8191;state.resources[0].maximum=3;state.resources[0].current=0;state.items[2].quantity=0;state.items[2].charges=(pd_charges){0,3,0};
    state.magic.available=true;state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Limited power",.kind=1,.maximum=3,.remaining=3};state.magic.powers[1]=(pd_power){.name="Concentration",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=2;
    alternative=(pd_reduction_profile){.value=3,.types=8191,.stage=1,.active=true,.cost_kind=PD_COST_COUNTER,.cost=2,.payment=PD_PAYMENT_CHOOSE,.concentration=2};pd_profile_cost_set(&alternative,1,(pd_cost){PD_COST_POWER,0,2});pd_profile_cost_set(&alternative,2,(pd_cost){PD_COST_ITEM,2,2});assert(pd_reduction_profile_set(&state,0,alternative));
    pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;page=STATUS_HOME;render();act(1800);damage_profile=1;damage_plan.amount=20;click("Anteprima");effects_calls=random_calls;act(1860);assert(page==DAMAGE_CONFIRM&&random_calls==effects_calls);act(3500);click("2. Limited power");click("Tira COS");assert(page==CONCENTRATION_SETUP);
    effects_before=state;random_values[random_calls%2]=0;fail_save=true;click("Tira e applica");assert(concentration_rolled&&random_calls==effects_calls+1&&!memcmp(&state,&effects_before,sizeof state));act(3500);assert(page==CONCENTRATION_SETUP);screenshot("alternative-concentration-retry.ppm");
    fail_save=false;click("Riprova");click("Accelera");assert(random_calls==effects_calls+1&&state.hp==16&&state.magic.powers[0].remaining==1&&!state.magic.concentration&&!state.reductions[0].active&&incoming_event.payment_choice==2&&incoming_event.costs[1].selected&&!incoming_event.costs[0].selected);
    click("Eroe");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Zero-HP resolution keeps the selected payment and never rolls here. */
    state.hp=0;state.magic.concentration=0;state.reductions[0].concentration=0;state.status.available=true;state.magic.powers[0].remaining=0;state.resources[0].current=3;state.items[2].quantity=0;state.reductions[0].active=true;
    page=STATUS_HOME;render();act(1800);damage_profile=1;damage_plan.amount=5;click("Anteprima");act(3500);click("1. Slancio");click("Risolvi registro morte");click("Colpo critico: da verificare >");click("Base 2014: da verificare");effects_calls=random_calls;click("Conferma");assert(state.resources[0].current==1&&!state.magic.powers[0].remaining&&state.status.failures==1&&state.temp_hp==1&&random_calls==effects_calls);screenshot("alternative-death-result.ppm");
    /* Each new impact starts without a payment choice. */
    click("Nuovo");assert(!damage_payment_choice&&!damage_profile);
    puts("Alternative cost UI: mode edit/validation, explicit selection, missing balances/no fallback, retry/undo, frozen receipts, reaction, long names/holes, CON frozen retry and zero-HP integration passed (synthetic)");
    /* v0.61: common table operations use the home vitals and bounded keypad. */
    pd_demo(&state);state.hp=12;state.temp_hp=3;state.effects[0]=(pd_effect){.active=true,.target=PD_EFFECT_MAX_HP,.value=10};state.effect_count=1;
    pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;
    page=HERO;render();tap(200,104);assert(page==HOME_VITALS);bounds();screenshot("practical-vitals.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;unsigned practical_history=history_save_calls;
    click("Cura");assert(page==NUMBER_ENTRY&&!number_value&&number_target==NUMBER_HEAL&&confirmation_open());
    for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);act(1);assert(page==NUMBER_ENTRY&&!stage_import(&snapshot.state)&&!stage_backup(NULL));
    click("2");click("7");assert(number_value==27&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);screenshot("practical-heal-keypad.ppm");
    click("Indietro");assert(page==HOME_VITALS&&!memcmp(&state,&effects_before,sizeof state));click("Cura");click("2");click("7");click("Anteprima");assert(page==VITAL_CONFIRM);screenshot("practical-heal-preview.ppm");
    act(100);shortcut(0);assert(page==VITAL_CONFIRM&&!stage_import(&snapshot.state)&&!stage_backup(NULL));fail_save=true;click("Conferma");assert(page==VITAL_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("practical-heal-error.ppm");
    fail_save=false;click("Conferma");assert(page==HERO&&state.hp==39&&state.temp_hp==3&&pd_max_hp(&state)==42&&feedback.active&&random_calls==effects_calls&&history_save_calls==practical_history);screenshot("practical-heal-result.ppm");
    click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    click("Temp.");click("1");click("Anteprima");assert(page==VITAL_CONFIRM);screenshot("practical-temp-kept.ppm");effects_saves=save_calls;act(3621);assert(page==VITAL_CONFIRM&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    click("Indietro");click("Azzera");click("1");click("2");click("Anteprima");screenshot("practical-temp-grant.ppm");click("Conferma");assert(page==HERO&&state.temp_hp==12&&state.hp==12);
    click("Temp.");click("Temp: assegna >");assert(vital_kind==PD_VITAL_TEMP_SET&&!number_value);screenshot("practical-temp-correction.ppm");click("Anteprima");assert(page==VITAL_CONFIRM);screenshot("practical-temp-clear.ppm");click("Conferma");assert(!state.temp_hp&&state.hp==12);click("Annulla");assert(state.temp_hp==12&&state.hp==12);
    /* Bounded keypad, full-heal cap, no write for cancelling or an unchanged result. */
    click("Cura");click("9");click("9");click("9");click("9");assert(number_value==999);click("Canc.");assert(number_value==99);click("Azzera");assert(!number_value);act(3606);assert(page==NUMBER_ENTRY);click("9");click("9");click("9");click("Anteprima");screenshot("practical-heal-capped.ppm");click("Conferma");assert(state.hp==42&&state.temp_hp==12);click("Annulla");assert(state.hp==12&&state.temp_hp==12);
    click("Danni");assert(page==DAMAGE_HOME&&damage_plan.amount==1);click("1 danno >");assert(page==NUMBER_ENTRY&&number_fresh&&number_target==NUMBER_DAMAGE);click("2");click("7");click("Usa numero");assert(page==DAMAGE_HOME&&damage_plan.amount==27&&state.hp==12);screenshot("practical-damage-amount.ppm");
    click("27 danni >");click("9");click("9");click("9");click("9");click("9");assert(number_value==9999);screenshot("practical-damage-maximum.ppm");click("Indietro");assert(damage_plan.amount==27);
    click("Componenti");click("Aggiungi");assert(page==DAMAGE_PART_EDIT);click("1 danni >");click("4");click("2");click("Usa numero");assert(page==DAMAGE_PART_EDIT&&damage_part_draft.amount==42);click("Usa parte");assert(damage_extra_count==1&&damage_extra[0].amount==42);click("Indietro");
    act(100);click("Stato");assert(page==STATUS_HOME&&state.hp==12&&state.temp_hp==12&&random_calls==effects_calls&&history_save_calls==practical_history);
    /* Temp HP never revive a dead character; the healing control remains blocked. */
    state.hp=0;state.temp_hp=0;state.status.available=true;state.status.failures=3;act(100);click("Temp.");click("5");click("Anteprima");click("Conferma");assert(page==HERO&&!state.hp&&state.temp_hp==5&&state.status.failures==3);
    tap(200,104);effects_saves=save_calls;act(3600);assert(page==HOME_VITALS&&save_calls==effects_saves);screenshot("practical-zero-pf.ppm");
    /* A failed potion write freezes its candidate/dice. No cancel, import,
       shortcut or duplicate button can reroll or consume twice. */
    pd_demo(&state);state.hp=12;state.temp_hp=7;act(100);act(501);click("Usa");effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;practical_history=history_save_calls;random_values[0]=random_values[1]=0;
    fail_save=true;click("Conferma");assert(page==ITEM_CONFIRM&&item_rolled&&random_calls==effects_calls+2&&!memcmp(&state,&effects_before,sizeof state));screenshot("practical-potion-retry.ppm");
    pd_state potion_candidate=hit_candidate;pd_pool potion_roll=item_result;act(91);act(100);act(88);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);assert(page==ITEM_CONFIRM&&item_rolled&&!stage_import(&snapshot.state)&&!stage_backup(NULL));
    random_values[0]=random_values[1]=3;click("Riprova");assert(random_calls==effects_calls+2&&!memcmp(&state,&effects_before,sizeof state)&&!memcmp(&hit_candidate,&potion_candidate,sizeof hit_candidate)&&!memcmp(&item_result,&potion_roll,sizeof item_result));
    fail_save=false;fail_canvas=true;click("Riprova");fail_canvas=false;assert(page==DICE&&has_roll&&!dice_animation.active&&!item_rolled&&random_calls==effects_calls+2&&state.hp==16&&state.temp_hp==7&&state.items[1].quantity==1&&history.count==1&&history_save_calls==practical_history+1);screenshot("practical-potion-result.ppm");
    act(89);assert(state.items[1].quantity==1&&random_calls==effects_calls+2&&history.count==1);click("Eroe");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    /* Ordinary consumables do not lock a random result that does not exist. */
    act(502);click("Usa");effects_before=state;effects_calls=random_calls;fail_save=true;click("Conferma");assert(!item_rolled&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Indietro");assert(page==ITEM_DETAIL);
    puts("Practical table UI: home vitals routes, bounded/cancellable keypad, healing/caps, temp grant/correction/clear/no-stacking, modal guards, failed save/undo, unchanged no-write, zero-HP guard, first/extra damage input and frozen potion/hit-die retry passed (synthetic)");
    /* v0.62: power telemetry never fabricates a percentage or changes game data. */
    pd_demo(&state);pd_history_reset(&history,state.history_epoch);persisted_history=history;history_pending=false;fail_history=false;history_quarantined=false;
    fake_power=(pd_power_info){.available=true,.battery=true,.percent_valid=true,.percent=64,.voltage_valid=true,.millivolts=3940};
    power_off_attempted=power_paused=false;power_wait_result=PD_POWER_WAIT;physical_keys=(pd_buttons){0};display_screen=(pd_screen){0};
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;practical_history=history_save_calls;
    page=HERO;render();power_service();uint32_t power_avatar_clock=avatar_started;lv_tick_inc(1);fake_power.millivolts--;power_service();assert(avatar_started==power_avatar_clock);screenshot("power-home-battery.ppm");click("Aspetto");click("Schermo");click("Alimentazione >");assert(page==POWER_HOME&&power_info.percent==64);screenshot("power-battery.ppm");
    assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls&&history_save_calls==practical_history);
    fake_power.vbus=fake_power.charging=true;power_service();screenshot("power-charging.ppm");act(3702);assert(page==POWER_HOME&&!power_off_calls);
    fake_power.battery=false;fake_power.percent_valid=fake_power.voltage_valid=fake_power.charging=false;power_service();screenshot("power-no-battery.ppm");
    fake_power.available=false;power_service();screenshot("power-unavailable.ppm");
    fake_power=(pd_power_info){.available=true,.battery=true,.percent_valid=true,.percent=19,.voltage_valid=true,.millivolts=3600};power_service();assert(pd_power_warning(&power_info,lv_tick_get())==1);screenshot("power-low.ppm");
    fake_power.percent=5;power_service();assert(pd_power_warning(&power_info,lv_tick_get())==2);screenshot("power-critical.ppm");page=HERO;render();screenshot("power-home-critical.ppm");
    page=POWER_HOME;power_info.sampled_at=lv_tick_get()-10001;render();screenshot("power-stale.ppm");assert(!pd_power_fresh(&power_info,lv_tick_get()));power_service();
    /* Pause is reversible and keeps RAM/undo. The wake gesture is swallowed. */
    fake_power.percent=64;fake_power.vbus=true;power_service();unsigned pauses_before=pause_calls;
    click("Pausa");assert(power_paused&&pause_calls==pauses_before+1&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    assert(!stage_import(&snapshot.state)&&!stage_backup(NULL));for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);assert(power_paused&&page==POWER_HOME);
    power_service();assert(power_paused&&!last_low_power);power_wait_result=PD_POWER_WAKE;fail_resume=true;power_service();assert(power_paused);fail_resume=false;power_service();assert(!power_paused&&display_screen.swallow_keys&&display_screen.swallow_touch&&page==POWER_HOME);
    screen_service(1,1);lv_tick_inc(800);screen_service(1,1);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(page==POWER_HOME&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);
    assert(!pd_screen_touch(&display_screen,true,true,lv_tick_get()));assert(!pd_screen_touch(&display_screen,true,false,lv_tick_get()));
    fail_pause=true;click("Pausa");assert(!power_paused);screenshot("power-pause-error.ppm");fail_pause=false;
    fake_power.vbus=false;power_service();power_wait_result=PD_POWER_WAIT;click("Pausa");power_service();assert(power_paused&&last_low_power);
    power_wait_result=PD_POWER_ERROR;power_service();assert(!power_paused&&strstr(power_notice,"errore sleep"));screenshot("power-sleep-error.ppm");
    power_wait_result=PD_POWER_WAIT;click("Pausa");fake_power.vbus=true;power_service();assert(!power_paused&&!strcmp(power_notice,"USB collegata"));
    /* Auto pause uses real inactivity, including at minimum brightness. */
    fake_power.vbus=false;power_service();state.dim_timeout=1;state.brightness=10;page=HERO;render();physical_keys=(pd_buttons){0};display_screen=(pd_screen){.activity=lv_tick_get()-300000};
    screen_service(0,0);power_service();assert(power_paused);power_wait_result=PD_POWER_WAKE;power_service();assert(!power_paused);
    for(unsigned blocked=0;blocked<6;blocked++){
        page=blocked==3?NUMBER_ENTRY:HERO;number_target=NUMBER_HEAL;vital_kind=PD_VITAL_HEAL;number_value=1;
        fake_power=(pd_power_info){.available=blocked!=2,.battery=blocked!=1,.vbus=blocked==0,.percent_valid=true,.percent=64};
        history_pending=blocked==4;state.dim_timeout=blocked==5?0:1;display_screen.touch_down=false;display_screen.activity=lv_tick_get()-300000;render();
        unsigned before_pause=pause_calls;power_service();assert(!power_paused&&pause_calls==before_pause);
    }
    history_pending=false;state.dim_timeout=0;state.brightness=50;effects_before=state;
    fake_power=(pd_power_info){.available=true,.battery=true,.percent_valid=true,.percent=64};page=POWER_HOME;power_notice="";power_service();render();
    history_pending=true;render();screenshot("power-pending-history.ppm");act(3702);assert(page==POWER_HOME);history_pending=false;render();
    click("Spegni");assert(page==POWER_OFF&&confirmation_open());screenshot("power-off-preview.ppm");effects_saves=save_calls;practical_history=history_save_calls;
    act(100);shortcut(0);assert(page==POWER_OFF&&!stage_import(&snapshot.state)&&!stage_backup(NULL));click("Indietro");assert(page==POWER_HOME&&save_calls==effects_saves&&history_save_calls==practical_history);
    click("Spegni");fake_power.vbus=true;act(3704);assert(!power_off_attempted&&!power_off_calls);fake_power.vbus=false;power_service();
    fail_storage_commit=true;click("Salva e spegni");assert(!power_off_attempted&&!power_off_calls&&history_save_calls==practical_history&&!memcmp(&state,&effects_before,sizeof state));screenshot("power-off-state-error.ppm");
    fail_storage_commit=false;fail_history=true;click("Salva e spegni");assert(!power_off_attempted&&!power_off_calls&&history_save_calls==practical_history+1&&!memcmp(&state,&effects_before,sizeof state));screenshot("power-off-history-error.ppm");
    fail_history=false;fail_power_off=true;click("Salva e spegni");assert(power_off_attempted&&!power_off_ok&&power_off_calls==1);screenshot("power-off-uncertain.ppm");
    effects_saves=save_calls;practical_history=history_save_calls;act(3704);act(3703);act(100);assert(power_off_calls==1&&save_calls==effects_saves&&history_save_calls==practical_history);
    power_off_attempted=false;fail_power_off=false;page=POWER_HOME;power_notice="";render();click("Spegni");click("Salva e spegni");assert(power_off_attempted&&power_off_ok&&power_off_calls==2&&save_calls==effects_saves+1&&history_save_calls==practical_history+1&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);screenshot("power-off-requested.ppm");
    act(3704);shortcut(1);assert(power_off_calls==2);power_off_attempted=false;page=HERO;fake_power=(pd_power_info){0};power_info=fake_power;
    puts("Power UI: estimated/unknown/absent/charging/low/critical/stale telemetry, unchanged game/history/RNG, reversible pause, held-input quarantine, USB wake, sleep/panel errors, auto-idle/guards, pending history, save failures and single explicit PMIC shutdown request passed (synthetic; no physical PMIC/sleep tested)");
    /* v0.63: search is view state; filtered positions retain original IDs. */
    pd_demo(&state);state.item_count=60;for(unsigned i=3;i<60;i++)state.items[i]=(pd_item){.name="Ordinary item",.quantity=1};
    strcpy(state.items[0].name,"Light sword");strcpy(state.items[31].name,"Light token +1");state.items[58]=(pd_item){.name="Light spent",.kind=PD_CONSUMABLE};state.items[59]=(pd_item){.name="Light potion",.quantity=2,.kind=PD_HEALING,.count=2,.sides=4,.healing_modifier=2};
    state.magic.available=true;state.magic.count=120;for(unsigned i=0;i<120;i++)state.magic.powers[i]=(pd_power){.name="Ordinary spell",.kind=i%2,.maximum=i%2?3:0,.remaining=i%2?3:0,.flags=PD_PREPARED};
    strcpy(state.magic.powers[0].name,"Light");strcpy(state.magic.powers[61].name,"Light step");strcpy(state.magic.powers[118].name,"Daylight");strcpy(state.magic.powers[119].name,"Light step");assert(pd_valid(&state));
    power_info=fake_power=(pd_power_info){0};history_pending=false;history_quarantined=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;
    item_query[0]=power_query[0]=0;inventory_page=19;magic_filter=2;magic_page=39;page=INVENTORY;render();
    effects_before=state;pd_state search_previous=previous;bool search_undo=undo_available;effects_saves=save_calls;effects_calls=random_calls;practical_history=history_save_calls;
    click("Cerca nello zaino");assert(page==ITEM_SEARCH&&confirmation_open()&&!search_draft[0]);screenshot("search-items-keyboard.ppm");
    type_search("light");assert(!strcmp(search_draft,"LIGHT")&&!item_query[0]);screenshot("search-items-draft.ppm");
    for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);act(83);assert(page==ITEM_SEARCH&&!stage_import(&snapshot.state)&&!stage_backup(NULL));
    click("Indietro");assert(page==INVENTORY&&inventory_page==19&&!item_query[0]);click("Cerca nello zaino");type_search(" light ");fail_save=true;click("Mostra");fail_save=false;render();
    assert(page==INVENTORY&&!inventory_page&&!strcmp(item_query,"LIGHT")&&save_calls==effects_saves&&random_calls==effects_calls&&history_save_calls==practical_history&&!memcmp(&state,&effects_before,sizeof state)&&undo_available==search_undo&&!memcmp(&previous,&search_previous,sizeof previous));screenshot("search-items-results.ppm");
    tap(233,176);assert(page==ITEM_DETAIL&&item_index==0);click("Zaino");assert(!strcmp(item_query,"LIGHT")&&!inventory_page);click(">");assert(inventory_page==1);screenshot("search-items-last.ppm");
    tap(233,176);assert(item_index==59&&page==ITEM_DETAIL);click("Usa");act(3800);assert(page==ITEM_CONFIRM);random_values[0]=random_values[1]=0;click("Conferma");click("Accelera");assert(state.items[59].quantity==1&&state.items[1].quantity==2&&state.hp==28&&random_calls==effects_calls+2);
    click("Eroe");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state)&&!strcmp(item_query,"LIGHT"));click("Zaino");assert(inventory_page==1);click("<");tap(233,274);assert(item_index==58&&page==ITEM_DETAIL);effects_saves=save_calls;act(88);assert(page==ITEM_DETAIL&&!state.items[58].quantity&&save_calls==effects_saves);click("Zaino");
    click("Cerca: LIGHT >");click("Azzera");type_search("zzzz");click("Mostra");assert(page==INVENTORY&&!inventory_page&&pd_catalog_count(&state,PD_CATALOG_ITEMS,item_query)==0);screenshot("search-items-empty.ppm");act(83);assert(!inventory_page);tap(233,326);click("Azzera");type_search("+1");screenshot("search-symbols.ppm");click("Mostra");tap(233,176);assert(item_index==31);click("Zaino");tap(233,326);click("Azzera");type_search("WWWWWWWWWWWWWWWWWWWW");act(3900);assert(strlen(search_draft)==20);wrapped_fits();screenshot("search-maximum.ppm");while(search_key_page+1<search_pages())click(">");screenshot("search-last-keyboard.ppm");click("Azzera");click("Mostra");assert(!item_query[0]&&!inventory_page&&pd_catalog_count(&state,PD_CATALOG_ITEMS,"")==60);
    click("Cerca nello zaino");type_search("light");click("Mostra");act(800);assert(page==MAGIC_LIST&&magic_page==39&&!power_query[0]);click("Cerca");assert(page==POWER_SEARCH);type_search("light");screenshot("search-powers-keyboard.ppm");click("Mostra");assert(!magic_page&&!strcmp(power_query,"LIGHT")&&visible_powers()==4);screenshot("search-powers-results.ppm");
    click("Magie");assert(visible_powers()==2&&visible_power(0)==0&&visible_power(1)==118&&!magic_page);screenshot("search-spells.ppm");click("Capacita");assert(visible_powers()==2&&visible_power(0)==61&&visible_power(1)==119);screenshot("search-features.ppm");
    tap(233,270);assert(power_index==119&&page==MAGIC_DETAIL);click("Usa");click("Conferma");assert(state.magic.powers[119].remaining==2&&state.magic.powers[61].remaining==3&&state.magic.powers[0].remaining==0);screenshot("search-use-original.ppm");
    click("Elenco");assert(!strcmp(power_query,"LIGHT")&&visible_power(1)==119);click("Tutte");click(">");assert(magic_page==1&&visible_power(3)==119);tap(233,222);assert(power_index==119);click("Elenco");
    click("Cerca*");click("Azzera");type_search("step");click("Mostra");assert(!strcmp(power_query,"STEP")&&!strcmp(item_query,"LIGHT")&&visible_powers()==2);screenshot("search-separate-queries.ppm");
    click("Cerca*");click("Azzera");type_search("nothing");click("Mostra");assert(!magic_page&&!visible_powers());screenshot("search-powers-empty.ppm");tap(233,382);click("Azzera");click("Mostra");assert(!power_query[0]&&visible_powers()==120&&!magic_page);
    /* Pausing a list preserves its query; import/restore reset queries only on confirmation. */
    act(80);assert(!strcmp(item_query,"LIGHT"));effects_saves=save_calls;effects_calls=random_calls;assert(power_begin_pause()&&power_paused);power_wait_result=PD_POWER_WAKE;power_service();assert(!power_paused&&!strcmp(item_query,"LIGHT")&&page==INVENTORY&&save_calls==effects_saves&&random_calls==effects_calls);
    pd_state search_import=state;assert(stage_import(&search_import));click("Annulla");assert(!strcmp(item_query,"LIGHT"));snapshot.state=state;snapshot.history=history;
    assert(stage_backup(&snapshot));click("Annulla");assert(!strcmp(item_query,"LIGHT"));assert(stage_backup(&snapshot));click("Ripristina");assert(page==HERO&&!item_query[0]&&!power_query[0]&&!inventory_page&&!magic_page&&state.item_count==60&&state.magic.count==120);screenshot("search-restored.ppm");
    act(80);click("Cerca nello zaino");type_search("light");click("Mostra");assert(stage_import(&search_import));click("Conferma");assert(page==HERO&&!item_query[0]&&!power_query[0]&&!inventory_page&&!magic_page);screenshot("search-imported.ppm");
    state.inventory_available=false;state.item_count=0;state.equipped=PD_NO_WEAPON;act(80);act(3800);assert(page==INVENTORY);state.magic.available=false;state.magic.count=0;act(800);act(3800);assert(page==MAGIC_LIST);
    puts("Catalog UI: both search keyboards, trim/case/clear/cancel/limit, four pages, zero results, 60/120 rows, original-ID potion/feature consumption, duplicate/depleted names, type intersections, RAM-only queries, no-write search, pause and import/native-restore reset passed (synthetic)");
    /* v0.64: local spell preparation, with original casts and bindings intact. */
    pd_demo(&state);state.magic.count=4;state.magic.powers[0]=(pd_power){.name="Scudo sintetico",.level=1,.flags=PD_USES_SLOT|PD_CONCENTRATES};
    state.magic.powers[1]=(pd_power){.name="Innato sintetico",.level=2,.flags=PD_PREPARED|PD_LIMITED,.maximum=2,.remaining=1};
    state.magic.powers[2]=(pd_power){.name="Trucchetto sintetico",.flags=PD_PREPARED};state.magic.powers[3]=(pd_power){.name="Capacita sintetica",.kind=1,.maximum=2,.remaining=2};
    state.magic.slots[0]=(pd_slot){2,3};state.magic.pact[0]=(pd_slot){1,1};assert(pd_valid(&state));
    power_info=fake_power=(pd_power_info){0};history_pending=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;item_query[0]=power_query[0]=0;magic_filter=2;magic_page=0;
    act(100);click("Menu");click("Magie");click("Scudo sintetico");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;practical_history=history_save_calls;
    act(831);assert(page==MAGIC_PREPARATION&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");click("Gestisci");assert(page==MAGIC_MANAGE);screenshot("prepare-manage-unprepared.ppm");click("Preparazione >");assert(page==MAGIC_PREPARATION&&preparation_target&&confirmation_open());screenshot("prepare-preview.ppm");
    for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);act(100);act(839);assert(page==MAGIC_PREPARATION&&!stage_import(&snapshot.state)&&!stage_backup(NULL));
    assert(save_calls==effects_saves&&random_calls==effects_calls&&history_save_calls==practical_history&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");assert(page==MAGIC_MANAGE&&!memcmp(&state,&effects_before,sizeof state));
    click("Preparazione >");fail_save=true;click("Conferma");assert(page==MAGIC_PREPARATION&&preparation_target&&!memcmp(&state,&effects_before,sizeof state));screenshot("prepare-save-error.ppm");fail_save=false;click("Conferma");
    assert(page==MAGIC_MANAGE&&(state.magic.powers[0].flags&PD_PREPARED)&&state.magic.slots[0].current==2&&state.magic.pact[0].current==1&&!state.magic.concentration&&random_calls==effects_calls&&history_save_calls==practical_history);screenshot("prepare-manage-prepared.ppm");
    click("Menu");click("Eroe");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    act(1000);click("Gestisci");click("Preparazione >");click("Conferma");click("Indietro");assert(page==MAGIC_DETAIL);screenshot("prepare-ready-to-cast.ppm");click("Lancia");click("Conferma");assert(state.magic.slots[0].current==1&&state.magic.pact[0].current==1&&state.magic.concentration==1);
    /* Removing preparation keeps an existing concentration and every binding. */
    state.effect_count=2;state.effects[0]=(pd_effect){.target=PD_EFFECT_MAX_HP,.value=10,.active=true,.concentration=1};state.effects[1]=(pd_effect){.target=PD_EFFECT_ARMOR,.value=2,.active=true,.concentration=1};state.hp=41;
    state.defences.temporary[0]=(pd_defence_temporary){.flags=1,.active=true,.concentration=1};state.reductions[0]=(pd_reduction_profile){.value=3,.types=8191,.active=true,.stage=1,.concentration=1};assert(pd_valid(&state));effects_before=state;
    click("Gestisci");click("Preparazione >");assert(!preparation_target);screenshot("prepare-remove-preview.ppm");click("Conferma");effects_before.magic.powers[0].flags&=~PD_PREPARED;assert(!memcmp(&state,&effects_before,sizeof state)&&state.hp==41&&pd_armor(&state)==18&&state.magic.concentration==1);screenshot("prepare-concentration-kept.ppm");
    pd_history_reset(&history,state.history_epoch);persisted_history=history;char *prepared_json=pd_backup_encode(&state,&history,&external_size);assert(prepared_json&&pd_backup_decode(prepared_json,external_size,&snapshot)&&!memcmp(&snapshot.state,&state,sizeof state));free(prepared_json);
    click("Indietro");effects_saves=save_calls;act(831);assert(page==MAGIC_PREPARATION&&save_calls==effects_saves&&state.magic.slots[0].current==1);click("Indietro");click("Menu");click("Eroe");click("Annulla");effects_before.magic.powers[0].flags|=PD_PREPARED;assert(!memcmp(&state,&effects_before,sizeof state));
    /* Innate manual uses remain reachable; preparation never refills them. */
    act(1001);click("Gestisci");screenshot("prepare-innate-manage.ppm");click("Preparazione >");click("Conferma");assert(!(state.magic.powers[1].flags&PD_PREPARED)&&state.magic.powers[1].remaining==1&&state.magic.concentration==1);click("Utilizzi manuali");click("+1");assert(state.magic.powers[1].remaining==2&&!(state.magic.powers[1].flags&PD_PREPARED));click("Indietro");click("Gestisci");click("Preparazione >");click("Conferma");assert(state.magic.powers[1].remaining==2&&(state.magic.powers[1].flags&PD_PREPARED));
    act(1002);click("Gestisci");click("Preparazione >");screenshot("prepare-cantrip.ppm");click("Conferma");assert(!(state.magic.powers[2].flags&PD_PREPARED)&&state.magic.slots[0].current==1&&state.magic.concentration==1);
    act(1003);assert(page==MAGIC_DETAIL);effects_saves=save_calls;act(4000);assert(page==MAGIC_DETAIL&&save_calls==effects_saves);click("Utilizzi");assert(page==MAGIC_RECOVER);click("Indietro");
    /* Last original row through an active name filter, including a wide name. */
    state.magic.count=120;for(unsigned i=4;i<120;i++)state.magic.powers[i]=(pd_power){.name="Other spell",.flags=PD_PREPARED};memset(state.magic.powers[119].name,'W',40);state.magic.powers[119].name[40]=0;state.magic.powers[119].flags=0;assert(pd_valid(&state));strcpy(power_query,"WWW");magic_filter=0;magic_page=0;act(800);assert(visible_powers()==1&&visible_power(0)==119);tap(233,222);assert(power_index==119);click("Gestisci");wrapped_fits();screenshot("prepare-last-long-name.ppm");click("Preparazione >");wrapped_fits();screenshot("prepare-long-preview.ppm");click("Conferma");assert((state.magic.powers[119].flags&PD_PREPARED)&&!(state.magic.powers[2].flags&PD_PREPARED));click("Indietro");click("Elenco");assert(!strcmp(power_query,"WWW")&&visible_power(0)==119);screenshot("prepare-search-kept.ppm");
    snapshot.state=state;snapshot.history=history;act(1119);click("Gestisci");click("Preparazione >");click("Conferma");assert(!(state.magic.powers[119].flags&PD_PREPARED));
    assert(stage_backup(&snapshot));click("Annulla");assert(!(state.magic.powers[119].flags&PD_PREPARED));assert(stage_backup(&snapshot));click("Ripristina");assert(state.magic.powers[119].flags&PD_PREPARED);screenshot("prepare-restored.ppm");
    pd_state unprepared_import=state;unprepared_import.magic.powers[119].flags&=~PD_PREPARED;assert(stage_import(&unprepared_import));click("Annulla");assert(state.magic.powers[119].flags&PD_PREPARED);assert(stage_import(&unprepared_import));click("Conferma");assert(!(state.magic.powers[119].flags&PD_PREPARED)&&random_calls==effects_calls);screenshot("prepare-imported.ppm");
    state.magic.available=false;state.magic.count=0;state.magic.concentration=0;state.effects[0].concentration=state.effects[1].concentration=state.defences.temporary[0].concentration=state.reductions[0].concentration=0;page=MAGIC_DETAIL;act(4000);assert(page==MAGIC_DETAIL);page=MAGIC_MANAGE;render();assert(page==MAGIC_LIST);
    puts("Preparation UI: manage/preview/cancel, guarded keys/import/pause, failed save/retry/undo, prepared cast gating, ordinary/pact/innate pools unchanged, concentration/bindings preserved, feature exclusion, cantrips, last filtered index/long name, native restore and USB import passed (synthetic)");
    /* v0.65: explicit local quantities without using/refilling the object. */
    pd_demo(&state);state.hp=12;state.temp_hp=7;state.items[1].charges=(pd_charges){1,5,2};state.items[0].thrown_range=20;state.combat.thrown=true;
    state.magic.count=1;state.magic.powers[0]=(pd_power){.name="Concentration",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=1;
    state.effect_count=1;state.effects[0]=(pd_effect){.active=true,.target=PD_EFFECT_ARMOR,.value=2,.concentration=1};assert(pd_valid(&state));
    power_info=fake_power=(pd_power_info){0};item_query[0]=power_query[0]=0;history_pending=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;
    act(501);screenshot("quantity-detail.ppm");effects_before=state;effects_calls=random_calls;effects_saves=save_calls;practical_history=history_save_calls;
    click("Quantita 2 >");assert(page==NUMBER_ENTRY&&number_target==NUMBER_ITEM&&number_value==2&&number_fresh);click("1");click("8");assert(number_value==18);screenshot("quantity-keypad.ppm");
    act(4102);assert(page==NUMBER_ENTRY&&!memcmp(&state,&effects_before,sizeof state));click("Anteprima");assert(page==ITEM_QUANTITY_CONFIRM&&confirmation_open());screenshot("quantity-preview.ppm");
    act(100);act(89);for(unsigned gesture=0;gesture<3;gesture++)shortcut(gesture);assert(page==ITEM_QUANTITY_CONFIRM&&!stage_import(&snapshot.state)&&!stage_backup(NULL));
    click("Indietro");assert(page==NUMBER_ENTRY&&number_value==18);click("Indietro");assert(page==ITEM_DETAIL&&save_calls==effects_saves&&!memcmp(&state,&effects_before,sizeof state));
    click("Quantita 2 >");click("0");click("Anteprima");fail_save=true;click("Conferma");assert(page==ITEM_QUANTITY_CONFIRM&&!number_value&&!memcmp(&state,&effects_before,sizeof state));screenshot("quantity-save-error.ppm");fail_save=false;click("Conferma");
    effects_before.items[1].quantity=0;assert(page==ITEM_DETAIL&&!memcmp(&state,&effects_before,sizeof state)&&state.items[1].charges.current==1&&state.hp==12&&state.temp_hp==7&&state.magic.concentration==1);screenshot("quantity-zero.ppm");
    /* Unchanged preview cannot write or replace the preceding game undo. */
    click("Quantita 0 >");click("Anteprima");effects_saves=save_calls;act(4102);assert(page==ITEM_QUANTITY_CONFIRM&&save_calls==effects_saves);screenshot("quantity-no-change.ppm");click("Indietro");click("Indietro");click("Menu");click("Eroe");click("Annulla");effects_before.items[1].quantity=2;assert(!memcmp(&state,&effects_before,sizeof state));
    act(501);click("Quantita 2 >");click("0");click("Anteprima");click("Conferma");click("Quantita 0 >");click("9");click("9");click("9");click("9");assert(number_value==999);click("Canc.");assert(number_value==99);click("Azzera");click("3");click("Anteprima");click("Conferma");assert(state.items[1].quantity==3&&state.items[1].charges.current==1&&state.hp==12&&state.magic.concentration==1);screenshot("quantity-restocked.ppm");
    assert(random_calls==effects_calls&&history_save_calls==practical_history);click("Menu");click("Eroe");click("Annulla");assert(!state.items[1].quantity&&state.items[1].charges.current==1);
    /* Zero on the selected weapon atomically clears equipment and thrown mode. */
    act(500);click("Quantita 1 >");click("0");click("Anteprima");screenshot("quantity-weapon-preview.ppm");click("Conferma");assert(state.equipped==PD_NO_WEAPON&&!state.combat.thrown&&!state.items[0].quantity&&!strike_ready);screenshot("quantity-weapon-zero.ppm");
    click("Quantita 0 >");click("1");click("Anteprima");click("Conferma");assert(state.items[0].quantity==1&&state.equipped==PD_NO_WEAPON&&!state.combat.thrown);screenshot("quantity-weapon-returned.ppm");click("Equipaggia");click("Conferma");assert(state.equipped==0);
    /* The filtered last row retains its original identity and bounded long name. */
    state.item_count=60;for(unsigned i=3;i<60;i++)state.items[i]=(pd_item){.name="Other item",.quantity=3};memset(state.items[59].name,'W',40);state.items[59].name[40]=0;state.items[59].quantity=0;state.items[59].charges=(pd_charges){1,5,2};assert(pd_valid(&state));
    strcpy(item_query,"WWW");inventory_page=0;act(80);tap(233,177);assert(item_index==59);wrapped_fits();screenshot("quantity-last-name.ppm");click("Quantita 0 >");click("9");click("9");click("9");click("Anteprima");wrapped_fits();screenshot("quantity-long-preview.ppm");click("Conferma");assert(state.items[59].quantity==999&&state.items[59].charges.current==1&&state.items[2].quantity==3);click("Zaino");assert(!strcmp(item_query,"WWW"));screenshot("quantity-search-kept.ppm");
    pd_history_reset(&history,state.history_epoch);persisted_history=history;char *quantity_json=pd_backup_encode(&state,&history,&external_size);assert(quantity_json&&pd_backup_decode(quantity_json,external_size,&snapshot)&&!memcmp(&snapshot.state,&state,sizeof state));free(quantity_json);
    act(559);click("Quantita 999 >");click("0");click("Anteprima");click("Conferma");assert(!state.items[59].quantity);assert(stage_backup(&snapshot));click("Annulla");assert(!state.items[59].quantity);assert(stage_backup(&snapshot));click("Ripristina");assert(state.items[59].quantity==999&&state.items[59].charges.current==1);screenshot("quantity-restored.ppm");
    pd_state quantity_import=state;quantity_import.items[59].quantity=0;assert(stage_import(&quantity_import));click("Annulla");assert(state.items[59].quantity==999);assert(stage_import(&quantity_import));click("Conferma");assert(!state.items[59].quantity&&state.items[59].charges.current==1&&random_calls==effects_calls);screenshot("quantity-imported.ppm");
    puts("Quantity UI: keypad/bounds, preview/cancel, failed save/retry/undo, unchanged no-write, manual restock without charges/HP/effects/RNG/history, selected weapon zero/re-equip, original filtered index 59/long name and native backup/import/restore passed (synthetic)");
    /* Personalization drafts never write or change a rule. Saved cosmetics survive HP undo. */
    pd_demo(&state);state.shortcuts[0]=14;undo_available=false;display_screen=(pd_screen){0};physical_keys=(pd_buttons){0};act(100);act(1);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    shortcut(0);assert(page==APPEARANCE);click("Modifica");assert(page==AVATAR_EDIT&&confirmation_open());screenshot("appearance-editor-skin.ppm");
    click("Media");click(">");click("Biondi");screenshot("appearance-editor-hair.ppm");click(">");click("Vino");click(">");click("Lunghi");
    assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);lv_tick_inc(3200);avatar_frame(NULL);pd_avatar_large(&appearance_draft,avatar_phase,still);assert(!memcmp(still,avatar_data,sizeof still));
    click("Scena");assert(page==SCENE_EDIT);click(">");screenshot("appearance-scene-ruins.ppm");click(">");screenshot("appearance-scene-port.ppm");assert(appearance_scene==2&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);click("Annulla");assert(page==APPEARANCE&&!state.scene&&!state.avatar.skin);
    click("Modifica");click("Scura");click(">");click("Scuri");click(">");click("Notte");click(">");click("Rasati");click("Scena");click(">");click(">");click("Anteprima");assert(page==APPEARANCE_CONFIRM);screenshot("appearance-confirm.ppm");
    act(100);shortcut(0);assert(page==APPEARANCE_CONFIRM&&!stage_import(&effects_before)&&!stage_backup(NULL));
    fail_save=true;click("Conferma");assert(page==APPEARANCE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("appearance-save-error.ppm");fail_save=false;click("Indietro");assert(page==SCENE_EDIT&&appearance_scene==2);click("Anteprima");click("Conferma");assert(page==APPEARANCE&&state.scene==2&&state.avatar.skin==2&&state.avatar.hair==2&&state.avatar.cloak==2&&state.avatar.style==2);
    effects_before.avatar=state.avatar;effects_before.scene=2;assert(!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);click("Indietro");screenshot("appearance-home-port.ppm");click("Annulla");assert(state.hp==24&&state.scene==2&&state.avatar.style==2);screenshot("appearance-undo-keeps-look.ppm");
    effects_before=state;shortcut(0);click("Modifica");physical_keys=(pd_buttons){0};probe_key_gesture(0,100);assert(page==AVATAR_EDIT&&appearance_keyboard&&appearance_focus==0);probe_key_gesture(1,100);assert(page==SCENE_EDIT);probe_key_gesture(1,900);assert(page==APPEARANCE&&!memcmp(&state,&effects_before,sizeof state));
    click("Modifica");click("Anteprima");effects_saves=save_calls;click("Conferma");assert(page==APPEARANCE&&save_calls==effects_saves);click("Indietro");
    snapshot.state=state;snapshot.history=history;snapshot.state.history_epoch=snapshot.history.epoch;state=snapshot.state;char *appearance_json=pd_backup_encode(&state,&history,&external_size);assert(appearance_json&&pd_backup_decode(appearance_json,external_size,&snapshot)&&snapshot.state.scene==2&&snapshot.state.avatar.style==2);free(appearance_json);
    assert(stage_import(&effects_before));assert(pending.scene==2);click("Annulla");assert(state.scene==2);state.scene=0;assert(stage_backup(&snapshot));click("Ripristina");assert(page==HERO&&state.scene==2&&state.shortcuts[0]==14);screenshot("appearance-restored.ppm");
    state.scene=1;act(100);screenshot("appearance-home-ruins.ppm");state.scene=0;act(100);screenshot("appearance-home-camp.ppm");
    /* Species/body remain a draft until save, preserve mechanics/undo, render all families. */
    act(1);effects_before=state;effects_saves=save_calls;shortcut(0);click("Modifica");
    click(">");click(">");click(">");click(">");assert(appearance_field==4);click("Altre");click("Gnomo");click(">");click("Femminile");wrapped_fits();screenshot("species-gnome-female-preview.ppm");
    assert(state.lineage==0&&state.body==0&&save_calls==effects_saves);click("Anteprima");screenshot("species-confirm.ppm");fail_save=true;click("Conferma");assert(page==APPEARANCE_CONFIRM&&state.lineage==0);fail_save=false;click("Conferma");assert(state.lineage==4&&state.body==1);
    effects_before.lineage=4;effects_before.body=1;assert(!memcmp(&effects_before,&state,sizeof state));click("Indietro");screenshot("species-gnome-female-home.ppm");click("Annulla");assert(state.hp==24&&state.lineage==4&&state.body==1);
    for(unsigned lineage=0;lineage<PD_SPECIES_COUNT;lineage++)for(unsigned body=0;body<2;body++){
        state.lineage=lineage;state.body=body;state.avatar=(pd_avatar){0};act(100);char filename[80];snprintf(filename,sizeof filename,"species-home-%02u-%u.ppm",lineage,body);screenshot(filename);
        shortcut(0);click("Modifica");click(">");assert(appearance_field==(pd_species_hair(lineage)?1u:2u));click("Annulla");click("Indietro");
    }
    /* All seven species groups expose only valid IDs, including the new Goblin row. */
    effects_before=state;shortcut(0);click("Modifica");while(appearance_field!=4)click(">");assert(appearance_group==6);screenshot("species-last-group.ppm");
    click("Lizardfolk");click("Altre");assert(appearance_group==0);for(unsigned group=0;group<5;group++)click("Altre");assert(appearance_group==5);click("Goblin");screenshot("species-goblin-preview.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));click("Indietro");
    pd_state imported_species=state;imported_species.lineage=16;imported_species.body=1;assert(stage_import(&imported_species));screenshot("species-import.ppm");click("Annulla");assert(state.lineage==20);assert(stage_import(&imported_species));click("Conferma");assert(state.lineage==16&&state.body==1);
    pd_history_reset(&history,state.history_epoch);char *species_json=pd_backup_encode(&state,&history,&external_size);assert(species_json&&pd_backup_decode(species_json,external_size,&snapshot)&&snapshot.state.lineage==16&&snapshot.state.body==1);free(species_json);
    puts("Appearance UI: staged colors/cut/scenes, cancel, preview, failed-save retry, no-change no-write, actual draft blink, cosmetic-only commit, HP undo isolation, local physical focus/back, retained key assignments, USB import scene preservation and native backup/restore passed (synthetic)");
    /* Full SD class look path through the production editor and home renderer. */
    pd_demo(&state);state.shortcuts[0]=14;undo_available=false;pd_board_assets_refresh();act(100);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    shortcut(0);click("Modifica");while(appearance_field!=6)click(">");wrapped_fits();screenshot("class-editor-original.ppm");click("Altre");click("Altre");click("Altre");click("Ladro");assert(appearance_outfit==9&&avatar_class_loaded&&state.outfit==0);wrapped_fits();screenshot("class-editor-rogue.ppm");
    lv_tick_inc(3200);unsigned class_reads=pd_looks_reads();avatar_frame(NULL);assert(avatar_phase==0&&pd_looks_reads()==class_reads);
    click("Anteprima");wrapped_fits();screenshot("class-confirm.ppm");fail_save=true;click("Conferma");assert(page==APPEARANCE_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(state.outfit==9&&save_calls==effects_saves+2);effects_before.outfit=9;assert(!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls);click("Indietro");screenshot("class-home-rogue.ppm");
    for(unsigned lineage=0;lineage<PD_SPECIES_COUNT;lineage++)for(unsigned body=0;body<2;body++)for(unsigned outfit=1;outfit<PD_OUTFITS;outfit++){
        state.lineage=lineage;state.body=body;state.outfit=outfit;act(100);assert(avatar_class_loaded&&avatar_image.header.w==128&&avatar_image.header.h==128);if(lineage==0||outfit==8||outfit==9){char filename[96];snprintf(filename,sizeof filename,"class-home-%02u-%u-%02u.ppm",lineage,body,outfit);screenshot(filename);}
    }
    pd_history_reset(&history,state.history_epoch);char *class_json=pd_backup_encode(&state,&history,&external_size);assert(class_json&&pd_backup_decode(class_json,external_size,&snapshot)&&snapshot.state.outfit==12);free(class_json);
    effects_before=state;pd_looks_set_file(NULL);act(100);assert(!avatar_class_loaded&&!memcmp(&state,&effects_before,sizeof state));pd_avatar_variant_large(&state.avatar,state.lineage,state.body,avatar_phase,still);assert(!memcmp(still,avatar_data,sizeof still));screenshot("class-missing-sd.ppm");shortcut(0);click("Modifica");assert(appearance_field==6);wrapped_fits();screenshot("class-missing-sd-editor.ppm");click("Rileggi");assert(avatar_class_loaded);click("Annulla");click("Indietro");
    shortcut(0);click("Modifica");click("Altre");assert(appearance_class_group==0);click("Originale");click("Anteprima");click("Conferma");effects_before.outfit=0;assert(!memcmp(&state,&effects_before,sizeof state));click("Indietro");
    puts("Class UI: all 504 SD looks in production home, hood captures, editor staging/cancel/preview, physical-compatible focus, failed-save retry, mechanics/RNG preservation, native backup, missing-SD fallback/explicit refresh and original appearance recovery passed (host files, no physical SD)");
    /* USB14's verified base defenses survive confirmation; unknown types stay unknown. */
    pd_demo(&state);act(100);effects_before=state;pd_state imported_defences=state;imported_defences.defences.configured=48;imported_defences.defences.resistance=32;imported_defences.defences.immunity=16;imported_defences.items[2].charges=(pd_charges){5,7,0};
    assert(stage_import(&imported_defences));assert(pending.defences.resistance==32);wrapped_fits();screenshot("beyond-defences-import.ppm");click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));assert(stage_import(&imported_defences));fail_save=true;click("Conferma");assert(!import_pending&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;assert(stage_import(&imported_defences));click("Conferma");assert(!import_pending&&state.defences.configured==48&&state.defences.resistance==32&&state.defences.immunity==16&&state.items[2].charges.current==5);assert(!(state.defences.configured&1));
    act(1800);screenshot("beyond-defences-device.ppm");act(100);
    puts("Beyond defenses UI: imported verified masks/charges, pending preview/cancel, failed save and confirmation preserve the old sheet until commit; unknown types remain unknown (synthetic, no USB hardware)");
    /* Explicit completion preserves known conditional bases and independent extras. */
    pd_demo(&state);state.hp=32;state.temp_hp=7;
    assert(pd_defence_set(&state,0,true,1)&&pd_defence_rule_set(&state,0,false,0,PD_RULE_NONMAGICAL));
    assert(pd_defence_set(&state,5,true,6));
    for(unsigned slot=0;slot<3;slot++)assert(pd_defence_grant_set(&state,12,slot,(pd_defence_temporary){.flags=slot+1,.active=true,.timed=true,.rounds=slot+1}));
    act(1800);click("Difese per tipo");screenshot("defence-setup-list.ppm");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    click("Completa");assert(page==DEFENCE_SETUP);bounds();wrapped_fits();screenshot("defence-setup-preview.ppm");
    for(unsigned i=0;i<3;i++){shortcut(i);act(100);act(1852);assert(page==DEFENCE_SETUP&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state));}
    click(">");click(">");click(">");bounds();wrapped_fits();assert(defence_setup_page==3);screenshot("defence-setup-last.ppm");
    act(3904);assert(defence_setup_page==3);click("Indietro");assert(page==DEFENCE_LIST&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);
    click("Completa");fail_save=true;click("Conferma");assert(page==DEFENCE_SETUP&&!memcmp(&state,&effects_before,sizeof state));screenshot("defence-setup-save-error.ppm");
    fail_save=false;click("Conferma");assert(page==DEFENCE_LIST&&state.defences.configured==8191&&random_calls==effects_calls&&save_calls==effects_saves+2);
    pd_state completed_defences=effects_before;completed_defences.defences.configured=8191;assert(!memcmp(&state,&completed_defences,sizeof state));screenshot("defence-setup-complete.ppm");
    effects_saves=save_calls;act(3900);assert(page==DEFENCE_LIST&&save_calls==effects_saves);act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    pd_demo(&state);act(1800);click("Difese per tipo");click("Completa");for(unsigned i=0;i<4;i++)click(">");bounds();wrapped_fits();assert(defence_setup_page==4);screenshot("defence-setup-thirteen.ppm");click("Conferma");assert(state.defences.configured==8191&&!state.defences.resistance&&!state.defences.vulnerability&&!state.defences.immunity);
    puts("Manual defence completion: explicit paged review, unknown bases only, conditional rules/all extras preserved, keys/import protected, cancel, save retry, no RNG, undo, 13-type bounds and native persistence in core tests passed; no physical USB");
    /* Manual source counters occupy existing generic slots, not consumable powers. */
    pd_demo(&state);page=HERO;pd_state manual_import=state;manual_import.resource_count=8;manual_import.magic.count=1;
    manual_import.magic.powers[0]=(pd_power){.name="Azione informativa",.description="Contatore manuale separato in Usi. Nessun consumo o recupero automatico.",.kind=1,.flags=PD_PASSIVE|PD_PREPARED};
    for(unsigned i=0;i<8;i++){snprintf(manual_import.resources[i].name,sizeof manual_import.resources[i].name,"Riserva manuale %u",i+1);manual_import.resources[i].maximum=i==7?999:50;manual_import.resources[i].current=i==7?999:35;}
    assert(pd_valid(&manual_import)&&stage_import(&manual_import));click("Conferma");assert(state.resource_count==8);act(103);resource_index=0;render();bounds();screenshot("manual-counter-first.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;fail_save=true;click("-1");assert(!memcmp(&state,&effects_before,sizeof state));screenshot("manual-counter-save-error.ppm");fail_save=false;click("-1");assert(state.resources[0].current==34&&save_calls==effects_saves+2&&random_calls==effects_calls);act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    act(103);for(unsigned i=0;i<7;i++)click("Prossimo");assert(resource_index==7);bounds();screenshot("manual-counter-last.ppm");click("-1");assert(state.resources[7].current==998);click("+1");assert(state.resources[7].current==999);click("Prossimo");assert(!resource_index);
    click("Magie");click("Capacita");tap(233,222);assert(page==MAGIC_DETAIL);screenshot("manual-counter-note.ppm");effects_before=state;act(831);assert(!memcmp(&state,&effects_before,sizeof state)&&state.resources[0].current==35);
    puts("Manual counters UI: 8 generic slots, exact source balances, passive ability separation, failed-save retry, undo, last slot/999 boundary and no RNG passed; synthetic, no physical USB");
    /* Paid creation consumes a selected slot only, with normal modal/save protections. */
    pd_demo(&state);state.hp=32;state.temp_hp=7;state.resource_count=2;state.resources[0]=(pd_resource){.name="Manual cannon",.current=35,.maximum=50};state.resources[1]=(pd_resource){.name="Manual infusion",.current=1,.maximum=4};
    state.magic.available=true;state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Cannone sintetico",.kind=1,.flags=PD_PREPARED,.maximum=1,.remaining=1,.recovery=2,.effect=3};state.magic.powers[1]=(pd_power){.name="Concentrazione presente",.flags=PD_PREPARED|PD_CONCENTRATES};state.magic.concentration=2;
    state.magic.slots[0]=(pd_slot){2,3};state.magic.pact[1]=(pd_slot){1,2};power_query[0]=0;magic_page=0;act(800);click("Capacita");tap(233,222);bounds();screenshot("cannon-free.ppm");effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    click("Crea");assert(page==MAGIC_CONFIRM);screenshot("cannon-free-confirm.ppm");for(unsigned i=0;i<3;i++)shortcut(i);assert(page==MAGIC_CONFIRM&&!stage_import(&imported_magic));click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);
    click("Crea");fail_save=true;click("Conferma");assert(page==MAGIC_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));screenshot("cannon-free-error.ppm");fail_save=false;click("Conferma");assert(page==MAGIC_DETAIL&&!state.magic.powers[0].remaining&&state.magic.slots[0].current==2&&state.magic.concentration==2);screenshot("cannon-paid.ppm");
    click("Slot 1 ordinario >");assert(cast_level==2&&cast_pact);effects_before=state;effects_saves=save_calls;click("Crea");screenshot("cannon-paid-confirm.ppm");click("Indietro");assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves);click("Crea");fail_save=true;click("Conferma");assert(!memcmp(&state,&effects_before,sizeof state));screenshot("cannon-paid-error.ppm");fail_save=false;click("Conferma");assert(page==MAGIC_DETAIL&&!state.magic.pact[1].current&&!state.magic.powers[0].remaining&&state.magic.slots[0].current==2&&state.magic.concentration==2&&state.resources[0].current==35&&state.resources[1].current==1&&random_calls==effects_calls);
    pd_state paid_cannon=state;act(832);assert(!memcmp(&state,&paid_cannon,sizeof state));screenshot("cannon-paid-result.ppm");act(100);click("Annulla");assert(!memcmp(&state,&effects_before,sizeof state));
    state.magic.concentration=0;state.hp=0;power_index=0;page=MAGIC_DETAIL;render();effects_before=state;act(831);assert(page==MAGIC_DETAIL&&!memcmp(&state,&effects_before,sizeof state));screenshot("cannon-zero-hp.ppm");state.hp=32;for(unsigned i=0;i<9;i++)state.magic.slots[i].current=state.magic.pact[i].current=0;render();effects_before=state;act(831);assert(!memcmp(&state,&effects_before,sizeof state));screenshot("cannon-no-slot.ppm");
    puts("Cannon UI: free then selected ordinary/pact cost, staged/cancel, failed-save retry, no double confirmation, protected keys/import, exact undo, manual counters/concentration retained, zero HP/no slot and no RNG passed; synthetic, no hardware");
    pd_demo(&state);page=HERO;act(1600);click("Wi-Fi >");assert(page==WIFI_HOME);bounds();screenshot("wifi-offline.ppm");click("Configura da telefono / PC");assert(wifi_requested==PD_WIFI_SETUP);
    wifi_mock=(pd_wifi_info){.setup=true,.setup_seconds=300,.setup_ssid="PocketDD-DEMO",.setup_password="SYNTHETIC-ONLY"};render();bounds();screenshot("wifi-provisioning.ppm");click("Spegni");assert(wifi_requested==PD_WIFI_DISABLE);
    wifi_mock=(pd_wifi_info){.state=PD_WIFI_CONNECTED,.configured=true,.ssid="Synthetic home network",.ip="192.168.1.23"};render();bounds();screenshot("wifi-connected.ppm");click("Dimentica");assert(page==WIFI_FORGET);effects_before=state;for(unsigned i=0;i<3;i++)shortcut(i);assert(page==WIFI_FORGET&&!stage_import(&imported_magic));bounds();screenshot("wifi-forget.ppm");click("Indietro");click("Dimentica");click("Conferma");assert(wifi_requested==PD_WIFI_FORGET&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");
    state.magic.available=true;state.magic.count=2;state.magic.powers[0]=(pd_power){.name="Movimento",.description="Passo: 30 ft. Volo: da verificare. Fotografia all importazione.",.kind=1,.flags=PD_PASSIVE|PD_PREPARED,.effect=4,.level=1};state.magic.powers[1]=(pd_power){.name="Iniziativa",.description="Bonus +4. Fotografia: ricontrollare dopo modifiche.",.kind=1,.flags=PD_PASSIVE|PD_PREPARED,.effect=5,.level=6,.attack_modifier=4};assert(pd_valid(&state));page=SHEET;sheet_tab=0;render();click("Profilo PG >");assert(page==REFERENCE_HOME);bounds();screenshot("reference-list.ppm");click("Movimento");assert(page==REFERENCE_TEXT);bounds();screenshot("reference-movement.ppm");click("Indietro");click("Iniziativa");bounds();screenshot("reference-initiative.ppm");effects_calls=random_calls;effects_before=state;click("Lancia");assert(page==DICE&&combat_roll==8&&random_calls==effects_calls+1&&result.bonus==4&&!memcmp(&state,&effects_before,sizeof state));act(16);bounds();screenshot("reference-initiative-result.ppm");click("Profilo");click("Iniziativa");state.roll_conditions=PD_FRIGHTENED;render();bounds();screenshot("reference-initiative-context.ppm");click("Contesto");click("Fonte paura in vista: ? >");click("Usa contesto");click("Lancia");assert(combat_roll==8);act(16);click("Profilo");click("Iniziativa");click("Contesto");click("Effetti tiro: applicati >");click("Usa contesto");effects_calls=random_calls;click("Lancia");assert(combat_roll==8&&random_calls==effects_calls+1&&result.bonus==4);act(16);act(100);
    /* Direct setup: disposable scan/secret draft, same real controls and bounds. */
    page=WIFI_HOME;wifi_mock=(pd_wifi_info){0};scan_mock=(pd_wifi_scan_info){.scanning=true};render();effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    click("Cerca reti >");assert(page==WIFI_LIST&&wifi_requested==PD_WIFI_SCAN);bounds();screenshot("wifi-scan.ppm");for(unsigned i=0;i<3;i++)shortcut(i);act(100);assert(page==WIFI_LIST&&!stage_import(&imported_magic));
    scan_mock=(pd_wifi_scan_info){.complete=true,.count=4,.networks={{.ssid="Casa sintetica",.rssi=-42,.security=PD_WIFI_PERSONAL},{.ssid="Rete ospiti",.rssi=-63,.security=PD_WIFI_OPEN},{.ssid="Rete aziendale",.rssi=-70,.security=PD_WIFI_UNSUPPORTED},{.ssid="Quarta rete con nome lungo",.rssi=-80,.security=PD_WIFI_PERSONAL}}};render();bounds();screenshot("wifi-networks.ppm");click(">");assert(wifi_list_page==1);bounds();click("<");act(5012);assert(page==WIFI_LIST);click("Casa sintetica");assert(page==WIFI_PASSWORD);bounds();screenshot("wifi-password.ppm");
    /* All 95 printable keys exist on the actual paged keyboard. */
    bool present[128]={0};for(unsigned mode=0;mode<4;mode++){act(5030+mode);for(unsigned pg=0;pg<wifi_key_pages();pg++){bounds();for(unsigned i=0;i<10;i++){unsigned index=wifi_key_page*10+i;if(index<strlen(wifi_keys[mode]))present[(unsigned char)wifi_keys[mode][index]]=true;}act(5020);}}
    for(unsigned c=32;c<=126;c++)assert(present[c]);
    act(5030);for(const char *c="Test 9!z";*c;c++)act(5100+(unsigned char)*c);assert(!strcmp(wifi_draft.password,"Test 9!z"));bounds();screenshot("wifi-password-filled.ppm");assert(!wifi_reveal);click("Avanti");assert(page==WIFI_JOIN_REVIEW&&!wifi_reveal);bounds();screenshot("wifi-join-review.ppm");for(unsigned i=0;i<3;i++)shortcut(i);assert(page==WIFI_JOIN_REVIEW&&!stage_import(&imported_magic)&&!join_calls);
    join_rejected=true;click("Collega");assert(page==WIFI_JOIN_REVIEW&&!join_calls&&wifi_draft.password[0]);join_rejected=false;click("Collega");assert(page==WIFI_HOME&&join_calls==1&&!strcmp(joined_mock.ssid,"Casa sintetica")&&!strcmp(joined_mock.password,"Test 9!z")&&!wifi_draft.password[0]&&!wifi_draft.ssid[0]);
    wifi_mock=(pd_wifi_info){.joining=true,.state=PD_WIFI_CONNECTING,.ssid="Casa sintetica"};render();bounds();screenshot("wifi-joining.ppm");wifi_mock=(pd_wifi_info){.state=PD_WIFI_CONNECTED,.configured=true,.setup_saved=true,.ssid="Casa sintetica",.ip="192.168.1.23"};render();bounds();screenshot("wifi-saved.ppm");wifi_mock=(pd_wifi_info){.state=PD_WIFI_FAILED,.setup_failed=true,.error=PD_WIFI_ERROR_NETWORK};render();bounds();screenshot("wifi-join-failed.ppm");wifi_mock=(pd_wifi_info){0};render();
    click("Cerca reti >");click("Rete ospiti");assert(page==WIFI_JOIN_REVIEW&&wifi_draft.open&&!wifi_draft.password[0]&&join_calls==1);bounds();screenshot("wifi-open-review.ppm");click("Indietro");assert(page==WIFI_LIST&&join_calls==1&&!wifi_draft.ssid[0]);
    click("Casa sintetica");for(unsigned i=0;i<64;i++)act(5100+'A');act(5100+'B');assert(strlen(wifi_draft.password)==64&&pd_wifi_credentials_valid(&wifi_draft));bounds();wifi_draft.password[0]='G';render();bounds();act(5024);assert(page==WIFI_PASSWORD&&!pd_wifi_credentials_valid(&wifi_draft));wifi_draft.password[0]='A';render();click("Cancella");assert(strlen(wifi_draft.password)==63);click("Annulla");assert(page==WIFI_LIST&&!wifi_draft.password[0]);click("Indietro");assert(page==WIFI_HOME&&wifi_requested==PD_WIFI_SCAN_CANCEL);
    /* Physical keys focus/activate/back locally instead of invoking configured shortcuts. */
    click("Cerca reti >");physical_keys=(pd_buttons){0};display_screen=(pd_screen){0};screen_service(0,0);lv_tick_inc(40);screen_service(0,0);screen_service(1,0);lv_tick_inc(40);screen_service(1,0);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(wifi_keyboard&&wifi_focus==0&&page==WIFI_LIST);bounds();
    for(unsigned k=0;k<2;k++){screen_service(1,0);lv_tick_inc(40);screen_service(1,0);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);}assert(wifi_focus==2);screen_service(0,1);lv_tick_inc(40);screen_service(0,1);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(page==WIFI_PASSWORD);screen_service(0,1);lv_tick_inc(40);screen_service(0,1);lv_tick_inc(801);screen_service(0,1);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(page==WIFI_LIST&&!wifi_draft.password[0]);
    click("Hotspot");assert(page==WIFI_HOME&&wifi_requested==PD_WIFI_SETUP);
    assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);memset(&joined_mock,0,sizeof joined_mock);
    puts("Direct Wi-Fi UI: async scan/list/paging, unsupported/open gates, 95 printable keys, masked/review drafts, 64-byte cap, cancel/retry/one join, protected import/shortcuts, physical focus, >=44 px targets and unchanged PG/RNG/NVS passed; RF not tested.");
    puts("Wi-Fi UI and reference/initiative: synthetic status only, no PG mutations, explicit forget guard, actual dice/history and current condition context passed; physical Wi-Fi still untested");
    pd_demo(&state);page=WIFI_HOME;wifi_mock=(pd_wifi_info){0};render();click("Ponte");assert(page==SYNC_HOME);bounds();screenshot("sync-offline.ppm");sync_mock=(pd_sync_info){.offer=true,.character_id="123456",.host="192.168.1.10",.fingerprint="0123456789abcdef0123456789abcdef"};page=SYNC_REVIEW;render();effects_before=state;bounds();screenshot("sync-review.ppm");for(unsigned i=0;i<3;i++)shortcut(i);assert(page==SYNC_REVIEW&&!stage_import(&imported_magic));sync_fail_save=true;click("Conferma");assert(page==SYNC_REVIEW&&sync_mock.offer&&!memcmp(&state,&effects_before,sizeof state)&&!strcmp(sync_notice,"Salvataggio non confermato"));bounds();screenshot("sync-save-error.ppm");sync_fail_save=false;click("Conferma");assert(page==SYNC_HOME&&!sync_mock.offer&&sync_mock.configured&&!memcmp(&state,&effects_before,sizeof state));sync_mock.state=PD_SYNC_CONFLICT;render();bounds();screenshot("sync-conflict.ppm");click("Rimuovi");assert(page==SYNC_FORGET);bounds();screenshot("sync-forget.ppm");click("Indietro");click("Rimuovi");click("Conferma");assert(page==SYNC_HOME&&!sync_mock.configured&&!memcmp(&state,&effects_before,sizeof state));
    puts("Sync UI: synthetic pairing/fingerprint, protected confirmation, failed-save retry, conflict, forget and unchanged character passed; no physical network or Beyond write tested");
    /* v0.80: count real pointer taps, not helper auto-navigation. */
    pd_demo(&state);display_screen=(pd_screen){0};power_paused=power_off_attempted=false;import_pending=false;fail_save=false;history_pending=false;
    state.magic.available=true;state.magic.count=6;state.magic.concentration=0;memset(state.magic.slots,0,sizeof state.magic.slots);memset(state.magic.pact,0,sizeof state.magic.pact);state.magic.slots[0]=(pd_slot){2,3};
    state.magic.powers[0]=(pd_power){.name="Scudo arcano",.flags=PD_PREPARED|PD_USES_SLOT,.level=1};state.magic.powers[1]=(pd_power){.name="Luce",.flags=PD_PREPARED};state.magic.powers[2]=(pd_power){.name="Passo velato",.flags=PD_PREPARED|PD_USES_SLOT,.level=2};state.magic.powers[3]=(pd_power){.name="Cura non preparata",.flags=PD_USES_SLOT,.level=1};state.magic.powers[4]=(pd_power){.name="Impulso eroico",.kind=1,.maximum=2,.remaining=1};state.magic.powers[5]=(pd_power){.name="Capacita esaurita",.kind=1,.maximum=1};
    ready_view=pouch_view=false;item_query[0]=power_query[0]=0;magic_filter=2;magic_page=inventory_page=0;pd_dice_idle(&dice_animation,1);combat_roll=0;page=HERO;feedback.active=false;render();
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    lv_indev_t *ux=lv_indev_create();lv_indev_set_type(ux,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(ux,probe_touch);lv_indev_set_display(ux,d);
    screenshot("ux080-home.ppm");probe_click(ux,115,266);assert(page==COMBAT&&save_calls==effects_saves&&random_calls==effects_calls);act(100);probe_click(ux,350,266);assert(page==MAGIC_LIST);screenshot("ux080-magic.ppm");probe_click(ux,320,49);assert(page==HERO&&save_calls==effects_saves&&random_calls==effects_calls);
    act(100);probe_click(ux,200,104);assert(page==HOME_VITALS);screenshot("ux080-vitals.ppm");probe_click(ux,155,229);assert(page==DAMAGE_HOME);assert(!memcmp(&state,&effects_before,sizeof state));
    act(100);probe_click(ux,233,375);assert(page==HOME_MENU);screenshot("ux080-menu.ppm");probe_click(ux,344,69);assert(page==DEVICE_SETTINGS);act(100);
    /* Keep long-press help distinct from release/click, even on a ready die. */
    act(14);effects_saves=save_calls;effects_calls=random_calls;probe_sample(ux,153,49,true,20);probe_sample(ux,153,49,true,650);screenshot("ux080-hint.ppm");probe_sample(ux,153,49,false,20);assert(page==DICE&&save_calls==effects_saves&&random_calls==effects_calls);lv_tick_inc(2100);lv_timer_handler();bounds();
    /* Search/filter intersections retain original IDs. Navigation never consumes. */
    state=effects_before;state.item_count=8;state.equipped=0;for(unsigned i=2;i<8;i++){state.items[i]=(pd_item){.name="Ration",.kind=PD_CONSUMABLE,.quantity=1};}state.items[1].quantity=0;inventory_page=0;act(80);probe_click(ux,359,327);assert(pouch_view&&page==INVENTORY);screenshot("ux080-pouch.ppm");
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    probe_sample(ux,292,224,true,20);for(int x=267;x>=142;x-=25)probe_sample(ux,x,224,true,20);probe_sample(ux,142,224,false,20);assert(page==INVENTORY&&inventory_page==1&&save_calls==effects_saves&&random_calls==effects_calls&&!memcmp(&state,&effects_before,sizeof state));screenshot("ux080-swipe.ppm");
    /* At the last page, a swipe must not become a tap on a row at release. */
    probe_sample(ux,292,224,true,20);for(int x=267;x>=142;x-=25)probe_sample(ux,x,224,true,20);probe_sample(ux,142,224,false,20);assert(page==INVENTORY&&inventory_page==1);
    probe_sample(ux,142,224,true,20);for(int x=167;x<=292;x+=25)probe_sample(ux,x,224,true,20);probe_sample(ux,292,224,false,20);assert(inventory_page==0&&page==INVENTORY);
    click("Consumabili");assert(!pouch_view);act(800);click("Pronte");assert(ready_view&&page==MAGIC_LIST);screenshot("ux080-ready.ppm");click("Pronte");assert(!ready_view);
    /* Swiping inside an irreversible confirmation never commits it. */
    state=effects_before;state.items[1].quantity=1;page=ITEM_CONFIRM;item_index=1;render();effects_saves=save_calls;effects_calls=random_calls;effects_before=state;
    probe_sample(ux,350,383,true,20);probe_sample(ux,325,383,true,20);probe_sample(ux,300,383,true,20);probe_sample(ux,275,383,true,20);probe_sample(ux,275,383,false,20);assert(page==ITEM_CONFIRM&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    lv_indev_delete(ux);ready_view=pouch_view=false;act(91);puts("Practical device UX: one-tap attack/magic, primary damage/cure, shared icons/help, real pointer swipes/edges without row activation or confirmation, filters and unchanged state/RNG/storage passed; no physical touch claim.");
    /* Direct key selection never changes the binding while paging/cancelling. */
    pd_demo(&state);page=HERO;fail_save=false;history_pending=false;import_pending=false;
    display_screen=(pd_screen){0};power_paused=power_off_attempted=false;
    act(850);click("PWR breve: Eroe");assert(page==KEY_PICKER);effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    screenshot("keys-direct-first.ppm");click("Pagina successiva");click("Pagina successiva");screenshot("keys-direct-last.ppm");
    for(unsigned i=0;i<3;i++)shortcut(i);
    assert(page==KEY_PICKER&&!stage_import(&imported_magic));click("Indietro");
    assert(page==KEY_SETTINGS&&!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    click("PWR breve: Eroe");click("Pagina successiva");click("Pagina successiva");fail_save=true;click("Aspetto");
    assert(page==KEY_PICKER&&!memcmp(&state,&effects_before,sizeof state));screenshot("keys-direct-error.ppm");fail_save=false;click("Aspetto");
    assert(page==KEY_SETTINGS&&state.shortcuts[0]==14&&state.shortcuts[1]==3&&state.shortcuts[2]==5);
    for(unsigned gesture=0;gesture<3;gesture++)for(unsigned choice=0;choice<15;choice++){
        effects_before=state;unsigned old_binding=state.shortcuts[gesture];effects_saves=save_calls;act(860+gesture);assert(page==KEY_PICKER);
        while(key_page>choice/6)click("Pagina precedente");while(key_page<choice/6)click("Pagina successiva");
        click(shortcut_names[choice]);assert(page==KEY_SETTINGS&&state.shortcuts[gesture]==choice);
        effects_before.shortcuts[gesture]=choice;assert(!memcmp(&state,&effects_before,sizeof state));
        assert(save_calls==effects_saves+(choice!=old_binding));
    }
    assert(random_calls==effects_calls);
    puts("Direct key picker: all 15 functions/three gestures, current-page selection, cancel/paging without writes, protected keys/import, save failure/retry and other fields preserved passed; synthetic input only");
    /* SD upload is presentation/asset work, never a PG or RNG transaction. */
    pd_demo(&state);page=HERO;import_pending=false;history_pending=false;power_paused=power_off_attempted=false;display_screen=(pd_screen){0};fail_save=false;
    effects_before=state;effects_saves=save_calls;effects_calls=random_calls;
    pd_asset_request sd_request={.kind=PD_ASSET_BEGIN,.size=PD_ASSET_BYTES};memset(sd_request.sha,'0',64);
    sd_present=false;asset_request(&sd_request);assert(!asset_running&&page==HERO&&strstr(last_reply,"ERROR sd"));
    sd_present=true;sd_verified=true;asset_request(&sd_request);assert(!asset_running&&page==AVATAR_EDIT&&appearance_field==6&&strstr(last_reply,"ASSET DONE"));click("Annulla");
    asset_return=HERO;page=ASSET_INSTALL;asset_running=true;asset_verifying=false;asset_progress=50;render();bounds();screenshot("sd-install-progress.ppm");
    for(unsigned i=0;i<3;i++)shortcut(i);assert(page==ASSET_INSTALL&&!stage_import(&imported_magic));
    click("Interrompi");assert(!asset_running&&page==HERO&&strstr(last_reply,"ERROR cancelled"));
    asset_running=true;page=ASSET_INSTALL;asset_activity=lv_tick_get();lv_tick_inc(45000);asset_service();assert(!asset_running&&page==HERO&&strstr(last_reply,"ERROR timeout"));
    assert(!memcmp(&state,&effects_before,sizeof state)&&save_calls==effects_saves&&random_calls==effects_calls);
    puts("SD installer UI: absent SD, verified already-installed result, direct class choice without save, progress, protected keys/import, cancel/timeout and unchanged PG/RNG/NVS passed; no physical SD claim");
    /* v0.85: synthetic device session, explicit profiles, no real character JSON. */
    pd_demo(&state);state.level=6;state.hp=35;state.max_hp=53;state.armor=16;memcpy(state.abilities,(uint8_t[]){15,10,16,8,8,17},6);
    state.items[0]=(pd_item){.name="Claymore sintetica",.kind=PD_GEAR,.quantity=1};state.items[1]=(pd_item){.name="Chain Mail sintetica",.kind=PD_GEAR,.quantity=1};state.items[2]=(pd_item){.name="Anello sintetico",.kind=PD_GEAR,.quantity=1};state.equipped=PD_NO_WEAPON;
    state.unarmored_rule=1;state.gear[1]=(pd_gear){.kind=PD_WEAR_HEAVY,.armor=16,.equipped=true};
    state.magic.count=2;state.magic.powers[0]=(pd_power){.name="True Strike",.description="Synthetic weapon cantrip",.effect=PD_TRUE_STRIKE_2024,.ability=5,.flags=PD_PREPARED};state.magic.powers[1]=(pd_power){.name="Healing Word",.level=1,.flags=PD_USES_SLOT};state.magic.slots[0]=(pd_slot){1,2};
    history_pending=history_pair_pending=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;item_query[0]=power_query[0]=0;ready_view=pouch_view=false;magic_filter=2;magic_page=inventory_page=0;display_screen=(pd_screen){0};physical_keys=(pd_buttons){0};wifi_keyboard=false;pd_dice_idle(&dice_animation,1);page=HERO;combat_roll=0;fail_save=false;render();
    assert(pd_valid(&state));effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    act(800);screenshot("ux085-spell-list.ppm");act(80);screenshot("ux085-inventory.ppm");act(100);
    act(81);assert(page==WEAPON_PICKER);screenshot("ux085-weapon-picker.ppm");click("Claymore sintetica");assert(page==WEAPON_SETUP);screenshot("ux085-weapon-profile.ppm");
    click("Spada: 1d8 >");click("Competenza: no");click("Att +");click("Dan +");click("Anteprima");assert(page==PLAY_CONFIRM&&scratch.items[0].count==2&&scratch.items[0].sides==6&&scratch.equipped==0);screenshot("ux085-weapon-confirm.ppm");
    for(unsigned i=0;i<3;i++)shortcut(i);act(100);assert(page==PLAY_CONFIRM&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls&&save_calls==effects_saves);
    fail_save=true;click("Conferma");assert(page==PLAY_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==ATTACK_MODE&&state.equipped==0&&state.items[0].attack_modifier==1&&state.items[0].damage_modifier==1);click("Attacco normale");
    click("Varianti");assert(page==ATTACK_MODE);screenshot("ux085-attack-mode.ppm");click("True Strike");pd_weapon_info ts;assert(attack_weapon_info(&ts)&&ts.attack_bonus==7&&ts.damage_bonus==4&&ts.radiant_dice==1);screenshot("ux085-true-strike.ppm");
    random_values[0]=random_values[1]=19;click("Lancia");assert(strike.weapon.true_strike&&strike.critical&&strike.roll.bonus==7);click("Accelera");random_values[0]=random_values[1]=0;effects_calls=random_calls;unsigned pair_saves=history_save_calls;fail_history=true;click("Danni");assert(random_calls==effects_calls+6&&damage_parts.weapon.count==4&&damage_parts.radiant.count==2&&damage_parts.total==10&&history_pending&&history_pair_pending&&history_save_calls==pair_saves+1);click("Accelera");screenshot("ux085-true-strike-damage.ppm");
    effects_calls=random_calls;click("Radianti");click("Accelera");assert(random_calls==effects_calls&&damage_view_radiant&&damage_parts.total==10);screenshot("ux085-radiant-dice.ppm");click("Storico");click("Riprova");assert(history_pending&&history_pair_pending&&random_calls==effects_calls);act(2609);assert(page==HISTORY_DETAIL);click("Radiante");screenshot("ux085-pending-radiant.ppm");assert(history_index==PD_HISTORY_LIMIT+1);fail_history=false;click("Riprova");assert(!history_pending&&!history_pair_pending&&history.count==3&&history.entries[0].sides==6&&history.entries[0].count==2&&history.entries[1].count==4&&random_calls==effects_calls);
    /* Direct prepare: cancel and save retry, without slot consumption. */
    act(1001);click("Prepara");assert(page==MAGIC_PREPARATION&&preparation_return==MAGIC_DETAIL);click("Conferma");assert(page==MAGIC_DETAIL&&(state.magic.powers[1].flags&PD_PREPARED)&&state.magic.slots[0].current==1);
    /* Existing source armor moves through real equip -> preview -> confirm. */
    act(501);click("Riponi");assert(page==PLAY_CONFIRM&&scratch.armor==10&&state.armor==16);screenshot("ux085-armor-off.ppm");click("Conferma");assert(state.armor==10&&state.hp==35);click("Equipaggia");click("Conferma");assert(state.armor==16);
    click("Configura");assert(page==ITEM_CONFIG_MENU);screenshot("ux085-item-menu.ppm");click("Armatura e bonus");assert(page==GEAR_SETUP);screenshot("ux085-gear-setup.ppm");
    for(unsigned f=0;f<19;f++){gear_field=f;render();bounds();wrapped_fits();}click("Indietro");click("Indietro");
    /* New item bonuses: explicit 'not already included', CON propagates max HP. */
    act(502);click("Configura");click("Armatura e bonus");gear_field=3;render();click("+ / Si");assert(gear_adopt==1);click("- / No");assert(gear_adopt==2);gear_field=7;render();click("+ / Si");click("+ / Si");click("Anteprima");assert(scratch.abilities[2]==18&&scratch.max_hp==59&&scratch.hp==35);screenshot("ux085-gear-bonus-preview.ppm");click("Conferma");assert(state.abilities[2]==18&&state.max_hp==59&&state.hp==35);click("Riponi");click("Conferma");assert(state.abilities[2]==16&&state.max_hp==53&&state.hp==35);
    /* Old imported note is only a proposal: the user explicitly chooses 2024. */
    state.magic.powers[0]=(pd_power){.name="True Strike",.kind=1,.flags=PD_PASSIVE,.description="Use your spellcasting ability for a weapon attack"};assert(pd_valid(&state));act(1000);click("Regole");assert(page==MAGIC_PROFILE);screenshot("ux085-magic-profile.ppm");click("Effetti manuali >");while(profile_ability!=5)act(6302);click("Anteprima");assert(scratch.magic.powers[0].effect==PD_TRUE_STRIKE_2024&&state.magic.powers[0].effect==0);click("Conferma");assert(state.magic.powers[0].effect==PD_TRUE_STRIKE_2024&&!state.magic.powers[0].kind);click("Attacca");assert(page==COMBAT&&attack_spell==0);
    /* Physical setup navigation is local and preserves configured shortcuts. */
    click("Varianti");effects_before=state;wifi_keyboard=false;physical_keys=(pd_buttons){0};display_screen=(pd_screen){0};screen_service(0,0);lv_tick_inc(40);screen_service(0,0);screen_service(1,0);lv_tick_inc(40);screen_service(1,0);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(wifi_keyboard&&page==ATTACK_MODE&&!memcmp(&state,&effects_before,sizeof state));act(6099);
    char *session_backup=pd_backup_encode(&state,&history,&external_size);assert(session_backup);free(session_backup);
    puts("Device session 0.85: weapon setup/save retry, direct prepare, armor/CON deltas, explicit old-note profile, True Strike casting ability/critical mixed dice, atomic journal pair/frozen retries, local physical focus and all seven new views passed (synthetic LVGL, no hardware claim)");

    /* General equipment, independent of any real character or class name. */
    pd_demo(&state);state.item_count=6;state.items[4]=(pd_item){.name="Anello di prova",.quantity=1};state.items[5]=(pd_item){.name="Stivali di prova",.quantity=1};state.items[3]=(pd_item){.name="Mantello di prova",.quantity=1};state.gear[3]=(pd_gear){.kind=PD_WEAR_ITEM,.equipped=true,.attunement=1,.ac=1,.spell_attack=2,.spell_dc=1,.saves=1,.ability={0,0,2,0,0,0}};state.unarmored_rule=1;state.hp=32;
    state.magic.count=6;for(unsigned i=0;i<6;i++)state.magic.powers[i]=(pd_power){.name="Magia di prova",.flags=PD_PREPARED,.ability=i};assert(pd_valid(&state));history_pending=history_pair_pending=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;fail_save=false;wifi_keyboard=false;display_screen=(pd_screen){0};pd_dice_idle(&dice_animation,1);page=HERO;render();effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
    inventory_page=1;item_query[0]=0;act(80);screenshot("ux086-equipment-icons.ppm");act(503);screenshot("ux086-attunement-inactive.ppm");click("Sintonia");assert(page==PLAY_CONFIRM&&scratch.gear[3].attunement==2&&scratch.armor==17&&scratch.max_hp==35&&scratch.hp==32);screenshot("ux086-attunement-preview.ppm");
    for(unsigned i=0;i<3;i++)shortcut(i);act(100);assert(page==PLAY_CONFIRM&&!stage_import(&imported_magic)&&!memcmp(&state,&effects_before,sizeof state)&&random_calls==effects_calls&&save_calls==effects_saves);click("Indietro");assert(page==ITEM_DETAIL&&!memcmp(&state,&effects_before,sizeof state));click("Sintonia");fail_save=true;click("Conferma");assert(page==PLAY_CONFIRM&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==ITEM_DETAIL&&state.gear[3].attunement==2&&state.max_hp==35&&state.hp==32&&state.magic.powers[0].attack_modifier==2);screenshot("ux086-attuned.ppm");
    click("Riponi");click("Conferma");assert(!state.gear[3].equipped&&state.gear[3].attunement==2&&state.armor==16&&state.max_hp==32);screenshot("ux086-attuned-unworn.ppm");click("Equipaggia");click("Conferma");assert(state.gear[3].equipped&&state.armor==17&&state.max_hp==35);
    state.hp=35;click("Sintonia");assert(scratch.hp==32&&scratch.max_hp==32);screenshot("ux086-end-attunement.ppm");click("Conferma");assert(state.hp==32&&state.max_hp==32&&state.gear[3].attunement==1&&state.abilities[2]==14&&state.training.save_bonus[2]==0&&state.magic.powers[0].attack_modifier==0&&random_calls==effects_calls);
    click("Configura");click("Armatura e bonus");gear_field=19;render();bounds();wrapped_fits();screenshot("ux086-attunement-editor.ppm");click("Indietro");click("Indietro");assert(page==ITEM_DETAIL);click("Sintonia");click("Conferma");click("Quantita 1 >");click("0");click("Anteprima");click("Conferma");assert(state.items[3].quantity==0&&!state.gear[3].equipped&&state.gear[3].attunement==1&&state.max_hp==32);screenshot("ux086-item-removed.ppm");
    puts("General item UI 0.86: independent equip/attune states, effective spell/AC/CON/HP preview, guarded confirm/cancel/save retry, worn-only bonuses, no healing, bounded editor and quantity-zero end passed; synthetic, no character-specific branch");

    /* Contextual weapon variants: generic identities, all casting abilities. */
    lv_indev_t *attack_touch=lv_indev_create();lv_indev_set_type(attack_touch,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(attack_touch,probe_touch);lv_indev_set_display(attack_touch,d);
    for(unsigned casting=0;casting<6;casting++){
        pd_demo(&state);state.level=6;state.magic.available=true;state.magic.count=4;state.magic.concentration=0;
        state.magic.powers[0]=(pd_power){.name="Fire Bolt",.flags=PD_PREPARED};
        state.magic.powers[1]=(pd_power){.name="True Strike (nota)",.kind=1,.flags=PD_PASSIVE,.description="Use your spellcasting ability for a weapon attack"};
        state.magic.powers[2]=(pd_power){.name="Colpo Accurato",.flags=PD_PREPARED,.ability=casting,.effect=PD_TRUE_STRIKE_2024};
        state.magic.powers[3]=(pd_power){.name="Altra origine",.ability=(casting+1)%6,.effect=PD_TRUE_STRIKE_2024};
        assert(pd_valid(&state));page=HERO;history_pending=history_pair_pending=false;pd_history_reset(&history,state.history_epoch);persisted_history=history;fail_save=false;wifi_keyboard=false;display_screen=(pd_screen){0};pd_dice_idle(&dice_animation,1);render();
        effects_before=state;effects_calls=random_calls;effects_saves=save_calls;
        assert(attack_power_count()==3&&attack_power_at(0)==2&&attack_power_at(1)==3&&attack_power_at(2)==1&&attack_power_at(3)==PD_POWERS);
        probe_click(attack_touch,115,266);assert(page==ATTACK_MODE&&attack_return==HERO);bounds();wrapped_fits();if(casting==5)screenshot("ux087-attack-choices.ppm");
        for(unsigned i=0;i<3;i++)shortcut(i);act(100);assert(page==ATTACK_MODE&&!stage_import(&imported_magic));
        click("Indietro");assert(page==HERO&&!memcmp(&state,&effects_before,sizeof state));
        act(81);click("Attacco normale");assert(page==COMBAT&&attack_spell<0);pd_weapon_info ordinary;assert(attack_weapon_info(&ordinary)&&!ordinary.true_strike);if(casting==5)screenshot("ux087-normal.ppm");
        click("Varianti");click("Colpo Accurato");assert(page==COMBAT&&attack_spell==2);assert(attack_weapon_info(&ts)&&ts.true_strike&&ts.base==pd_modifier(state.abilities[casting])&&ts.radiant_dice==1);if(casting==5)screenshot("ux087-weapon-spell.ppm");
        /* Even a previous result cannot supply damage after selecting a new mode. */
        strike_ready=damage_used=has_roll=true;click("Varianti");click("Attacco normale");assert(!strike_ready&&!damage_used&&!has_roll&&!dice_animation.active);
        click("Varianti");if(casting==5)screenshot("ux087-unprepared.ppm");click("Altra origine");assert(page==MAGIC_PREPARATION&&preparation_return==ATTACK_MODE);if(casting==5)screenshot("ux087-prepare.ppm");
        wifi_keyboard=false;physical_keys=(pd_buttons){0};display_screen=(pd_screen){0};screen_service(0,0);lv_tick_inc(40);screen_service(0,0);screen_service(1,0);lv_tick_inc(40);screen_service(1,0);screen_service(0,0);lv_tick_inc(40);screen_service(0,0);assert(wifi_keyboard&&page==MAGIC_PREPARATION&&wifi_target_count==2&&!memcmp(&state,&effects_before,sizeof state));
        click("Indietro");assert(page==ATTACK_MODE&&!memcmp(&state,&effects_before,sizeof state));click("Altra origine");fail_save=true;click("Conferma");assert(page==MAGIC_PREPARATION&&!memcmp(&state,&effects_before,sizeof state));fail_save=false;click("Conferma");assert(page==COMBAT&&attack_spell==3&&(state.magic.powers[3].flags&PD_PREPARED));
        assert(attack_weapon_info(&ts)&&ts.base==pd_modifier(state.abilities[(casting+1)%6])&&random_calls==effects_calls&&save_calls==effects_saves+2);assert(!memcmp(state.magic.slots,effects_before.magic.slots,sizeof state.magic.slots)&&!memcmp(state.magic.pact,effects_before.magic.pact,sizeof state.magic.pact));
        effects_before=state;effects_saves=save_calls;click("Varianti");click("True Strike: tipo dell arma");assert(attack_radiant);click("Colpo Accurato");assert(attack_weapon_info(&ts)&&ts.radiant_weapon);click("Varianti");click("True Strike: arma radiante");assert(!attack_radiant);
        click(">");assert(attack_mode_page==1);bounds();wrapped_fits();if(casting==5)screenshot("ux087-manual.ppm");click("True Strike (nota)");assert(page==MAGIC_PROFILE&&!memcmp(&state,&effects_before,sizeof state));click("Indietro");assert(page==MAGIC_DETAIL);
        /* Unrelated attack spells never acquire weapon rules; blocked choices cannot fire. */
        state.combat.raging=true;act(81);assert(page==ATTACK_MODE);bounds();wrapped_fits();if(casting==5)screenshot("ux087-raging.ppm");act(6411);assert(page==ATTACK_MODE&&attack_spell<0);state.combat.raging=false;state.hp=0;render();act(6412);assert(page==ATTACK_MODE&&attack_spell<0);state.hp=state.max_hp;
        assert(random_calls==effects_calls&&save_calls==effects_saves);state.magic.count=1;page=HERO;act(81);assert(page==COMBAT&&attack_spell<0); /* no redundant menu */
    }
    lv_indev_delete(attack_touch);
    puts("Contextual attacks 0.87: real touch entry, all six casting abilities, normal/prepared/prepare-now, source identity/order/paging, no unrelated spell automation, cancel/save failure/retry, blocked states, stale-damage invalidation, zero RNG/slot/extra-save changes passed; synthetic LVGL");

    unsigned covered=0;
    for(unsigned p=0;p<=SYNC_FORGET;p++){
        covered+=styled_pages[p];
        if(!styled_pages[p])fprintf(stderr,"Theme page not exercised: %u\n",p);
        if(p!=HERO&&p!=HOME_MENU&&p!=HOME_VITALS)assert(pd_theme_get(page_themes[p]));
    }
    printf("Styled production views exercised: %u/%u\n",covered,(unsigned)SYNC_FORGET+1);
    assert(covered==SYNC_FORGET+1);
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, animated single/double dice, RNG isolation, skip, disabled input, timer cleanup, circular touch bounds >=44px, rest previews/policies, hit dice, rollback, condition notes/poison, death saves, display preferences, idle/wake policy explicit CA/max-HP effects, round durations/expiry, typed defences/damage, concentration saves with frozen retry, explicit effect bindings/end/recast/caps, signed rest preview, temporary defences, mixed damage, conditional rules/source declarations/grouping/frozen results, three independent extras/combined drafts/high-bit timers, before/after reductions/full math/frozen retry, reusable profiles/shared budget/42 timers/outer drafts, atomic pool costs/reaction/explicit turn/frozen expense, ordinary/pact slot costs/fixed levels/no fallback/rest recovery own-turn timers/atomic reaction reset/50 expiries/17 detail pages elapsed minutes/hours/chronological caps/precision, separate item charges/manual correction/rest recovery/atomic costs/frozen receipt three independent expenses/all-or-none/frozen receipts explicit charge recharge/frozen retry/no-write zero/capped results four explicit roll conditions/partial context/automatic failure/Boolean cancellation/DEX saves, saved-state feedback/expiry/static mode/wrap and guarded physical buttons passed");
}
