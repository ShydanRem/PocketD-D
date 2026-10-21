/* Native LVGL proof of layout/input/lifetime. Synthetic core fixture only.
 * The optional target is deliberately disconnected from production NVS/USB.
 */
#include "companion.h"
#include "pocketdd.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pd_state state, previous;
static companion_view view;
static unsigned mutations;
static bool fail_save, undo_available;
static companion_action last_action;
static uint16_t pixels[466*466];

static void paint(lv_display_t *display,const lv_area_t *area,uint8_t *bytes) {
    uint16_t *p=(uint16_t*)bytes;
    for(int y=area->y1;y<=area->y2;y++)for(int x=area->x1;x<=area->x2;x++)pixels[y*466+x]=*p++;
    lv_display_flush_ready(display);
}
static void screenshot(const char *name) {
    lv_obj_update_layout(lv_screen_active());lv_obj_invalidate(lv_screen_active());lv_refr_now(NULL);
    FILE *f=fopen(name,"wb");assert(f);fprintf(f,"P6\n466 466\n255\n");
    for(int y=0;y<466;y++)for(int x=0;x<466;x++) {
        uint16_t c=pixels[y*466+x];unsigned char rgb[]={((c>>11)&31)*255/31,((c>>5)&63)*255/63,(c&31)*255/31};
        if((x-233)*(x-233)+(y-233)*(y-233)>233*233)memset(rgb,0,3);
        assert(fwrite(rgb,1,3,f)==3);
    }
    assert(!fclose(f));
}
static void bounds(void) {
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);if(!lv_obj_check_type(b,&lv_button_class))continue;
        lv_area_t a;lv_obj_get_coords(b,&a);
        assert(a.x2-a.x1+1>=44&&a.y2-a.y1+1>=44);
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2};
        for(unsigned x=0;x<2;x++)for(unsigned y=0;y<2;y++)assert((xs[x]-233)*(xs[x]-233)+(ys[y]-233)*(ys[y]-233)<=233*233);
        for(unsigned j=i+1;j<lv_obj_get_child_count(root);j++) {
            lv_obj_t *other=lv_obj_get_child(root,j);if(!lv_obj_check_type(other,&lv_button_class))continue;
            lv_area_t c;lv_obj_get_coords(other,&c);
            assert(a.x2<c.x1||a.x1>c.x2||a.y2<c.y1||a.y1>c.y2);
        }
        for(unsigned j=0;j<lv_obj_get_child_count(b);j++) {
            lv_obj_t *label=lv_obj_get_child(b,j);if(!lv_obj_check_type(label,&lv_label_class))continue;
            lv_area_t c;lv_obj_get_coords(label,&c);
            assert(c.x1>=a.x1&&c.x2<=a.x2&&c.y1>=a.y1&&c.y2<=a.y2);
        }
    }
}
static void show(void);
static void apply_hp(int delta) {
    pd_state next=state;pd_hp(&next,delta);
    view.applied_hp=view.applied_temporary=0;
    if(memcmp(&next,&state,sizeof state)==0)return;
    if(fail_save){view.storage_error=true;return;}
    previous=state;undo_available=true;mutations++;
    view.applied_hp=(int)next.hp-state.hp;view.applied_temporary=(int)next.temp_hp-state.temp_hp;
    state=next;view.storage_error=false;
}
static void action(companion_action selected,void *context) {
    assert(context==&state);last_action=selected;
    view.applied_hp=view.applied_temporary=0;
    switch(selected) {
    case COMPANION_OPEN_MENU:view.page=COMPANION_MENU;break;
    case COMPANION_OPEN_HOME:view.page=COMPANION_HOME;break;
    case COMPANION_OPEN_VITALS:view.page=COMPANION_VITALS;break;
    case COMPANION_MINUS:apply_hp(-1);if(!view.storage_error)view.page=COMPANION_HOME;break;
    case COMPANION_PLUS:apply_hp(1);if(!view.storage_error)view.page=COMPANION_HOME;break;
    case COMPANION_UNDO:if(undo_available&&!fail_save){state=previous;undo_available=false;mutations++;view.page=COMPANION_HOME;}break;
    default:break; /* Six route events are verified, destinations are still production UI. */
    }
    show();
}
static void show(void) {
    view.name=state.name;view.level=state.level;view.hp=state.hp;view.maximum=pd_max_hp(&state);
    view.temporary=state.temp_hp;view.armor=pd_armor(&state);view.can_undo=undo_available;
    view.can_hurt=state.hp||state.temp_hp;
    view.can_heal=state.hp<pd_max_hp(&state)&&(state.hp||!state.status.available||state.status.failures<3);
    companion_show(lv_screen_active(),&view,action,&state);bounds();
}
static void tap(int x,int y) {
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *b=lv_obj_get_child(root,i);if(!lv_obj_check_type(b,&lv_button_class))continue;
        lv_area_t a;lv_obj_get_coords(b,&a);
        if(x>=a.x1&&x<=a.x2&&y>=a.y1&&y<=a.y2){lv_obj_send_event(b,LV_EVENT_CLICKED,NULL);return;}
    }
    assert(!"Missing touch target");
}
int main(void) {
    lv_init();lv_display_t *display=lv_display_create(466,466);static uint16_t draw_buffer[466*40];
    lv_display_set_buffers(display,draw_buffer,NULL,sizeof draw_buffer,LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(display,paint);pd_demo(&state);
    view=(companion_view){.page=COMPANION_HOME,.motion=true};show();screenshot("companion-home.ppm");
    pd_state original=state;unsigned before=mutations;
    for(unsigned i=0;i<20;i++){lv_tick_inc(80);lv_timer_handler();}
    assert(!memcmp(&state,&original,sizeof state)&&mutations==before);
    tap(233,375);assert(view.page==COMPANION_MENU);screenshot("companion-menu.ppm");
    const companion_action routes[]={COMPANION_ATTACK,COMPANION_DICE,COMPANION_MAGIC,COMPANION_PACK,COMPANION_REST,COMPANION_SHEET};
    for(unsigned i=0;i<6;i++){tap(125+(i%3)*108,158+(i/3)*120);assert(last_action==routes[i]&&mutations==before);}
    companion_focus_step(1);companion_focus_step(1);assert(companion_focus_index()==1);
    screenshot("companion-focus.ppm");companion_activate();assert(last_action==COMPANION_DICE);
    tap(233,389);assert(view.page==COMPANION_HOME);tap(210,105);assert(view.page==COMPANION_VITALS);
    screenshot("companion-vitals.ppm");tap(302,251);assert(state.hp==25&&view.applied_hp==1&&view.page==COMPANION_HOME);
    tap(210,105);tap(230,326);assert(state.hp==24&&!undo_available);
    apply_hp(4);show();assert(state.hp==28&&view.applied_hp==4);screenshot("companion-heal.ppm");
    for(unsigned frame=0;frame<24;frame++){char name[48];snprintf(name,sizeof name,"companion-heal-%02u.ppm",frame);screenshot(name);lv_tick_inc(80);lv_timer_handler();}
    screenshot("companion-expired.ppm");assert(state.hp==28);
    /* Failed storage cannot update displayed values or celebrate a requested delta. */
    tap(210,105);fail_save=true;before=mutations;tap(150,251);
    assert(view.page==COMPANION_VITALS&&state.hp==28&&!view.applied_hp&&view.storage_error&&mutations==before);
    screenshot("companion-storage-error.ppm");fail_save=false;view.storage_error=false;
    /* Damage absorbed by temporary HP must show temporary loss, not a false HP loss. */
    state.temp_hp=3;apply_hp(-1);view.page=COMPANION_HOME;show();
    assert(state.hp==28&&state.temp_hp==2&&view.applied_hp==0&&view.applied_temporary==-1);
    screenshot("companion-temporary.ppm");
    companion_pause(true);before=mutations;companion_focus_step(1);companion_activate();tap(233,375);
    assert(mutations==before&&view.page==COMPANION_HOME);lv_tick_inc(2000);lv_timer_handler();companion_pause(false);
    screenshot("companion-wake.ppm");
    /* A static preference must keep the same pixels, expire feedback, and do no writes. */
    view.motion=false;apply_hp(1);show();screenshot("companion-static.ppm");
    static uint16_t static_frame[466*466];memcpy(static_frame,pixels,sizeof pixels);
    before=mutations;lv_tick_inc(400);lv_timer_handler();screenshot("companion-static-400.ppm");
    assert(!memcmp(static_frame,pixels,sizeof pixels)&&mutations==before);
    lv_tick_inc(1400);lv_timer_handler();screenshot("companion-static-expired.ppm");
    assert(memcmp(static_frame,pixels,sizeof pixels)&&mutations==before);
    /* Disabled events and configurable-focus hooks never bypass availability. */
    state.hp=pd_max_hp(&state);state.temp_hp=0;view.page=COMPANION_VITALS;show();before=mutations;
    tap(300,251);assert(mutations==before&&view.page==COMPANION_VITALS);
    companion_focus_step(-1);assert(companion_focus_index()==3);companion_activate();assert(view.page==COMPANION_HOME);
    memset(state.name,'W',PD_NAME_BYTES-1);state.name[PD_NAME_BYTES-1]=0;
    state.hp=state.max_hp=999;state.temp_hp=999;state.armor=40;state.level=20;
    view.applied_hp=view.applied_temporary=0;show();screenshot("companion-maximum.ppm");
    state.hp=state.temp_hp=0;show();screenshot("companion-zero.ppm");
    /* Repeated replacement/close exercises timer lifetime; sanitizers cover ownership. */
    for(unsigned i=0;i<80;i++){view.page=i%3;view.motion=true;show();lv_tick_inc(100);lv_timer_handler();}
    companion_close();lv_obj_clean(lv_screen_active());lv_tick_inc(4000);lv_timer_handler();
    lv_deinit();puts("Companion native prototype: touch, focus, core HP, failures, bounds, lifecycle passed");
    return 0;
}
