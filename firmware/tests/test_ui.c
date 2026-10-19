/* Actual firmware UI and LVGL rendered on host. No ESP peripherals simulated as verified. */
#define PD_UI_TEST 1
#include "../main/main.c"
#include <stdlib.h>
#include <math.h>
static bool fail_save;
uint32_t pd_board_random(void *unused) { return 9; }
bool pd_storage_save(const pd_state *s) { return !fail_save; }
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
static void bounds(void) {
    lv_obj_t *root=lv_screen_active();lv_obj_update_layout(root);
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *o=lv_obj_get_child(root,i);if(!lv_obj_check_type(o,&lv_button_class))continue;
        lv_area_t a;lv_obj_get_coords(o,&a);
        assert(a.y2-a.y1+1>=44);
        int xs[]={a.x1,a.x2},ys[]={a.y1,a.y2};
        for(int x=0;x<2;x++)for(int y=0;y<2;y++)assert((xs[x]-233)*(xs[x]-233)+(ys[y]-233)*(ys[y]-233)<=233*233);
        lv_obj_t *l=lv_obj_get_child(o,0);lv_area_t text;lv_obj_get_coords(l,&text);
        if(!(text.x1>=a.x1&&text.x2<=a.x2&&text.y1>=a.y1&&text.y2<=a.y2)) { fprintf(stderr,"Overflow %s: button %d,%d,%d,%d text %d,%d,%d,%d\n",lv_label_get_text(l),a.x1,a.y1,a.x2,a.y2,text.x1,text.y1,text.x2,text.y2); abort(); }
    }
}
static void click(const char *name) {
    lv_obj_t *root=lv_screen_active();
    for(unsigned i=0;i<lv_obj_get_child_count(root);i++) {
        lv_obj_t *o=lv_obj_get_child(root,i);
        if(lv_obj_check_type(o,&lv_button_class)&&!strcmp(lv_label_get_text(lv_obj_get_child(o,0)),name)) {
            assert(!lv_obj_has_state(o,LV_STATE_DISABLED));lv_obj_send_event(o,LV_EVENT_CLICKED,NULL);bounds();return;
        }
    }
    assert(!"button not found");
}
int main(void) {
    lv_init();lv_display_t *d=lv_display_create(466,466);static uint16_t buffer[466*40];
    lv_display_set_buffers(d,buffer,NULL,sizeof buffer,LV_DISPLAY_RENDER_MODE_PARTIAL);lv_display_set_flush_cb(d,paint);
    pd_demo(&state);render();bounds();screenshot("hero.ppm");
    click("-1 PF");assert(state.hp==23&&undo_available);click("Annulla modifica");assert(state.hp==24&&!undo_available);
    fail_save=true;click("-1 PF");assert(state.hp==24);fail_save=false;
    click("Scheda");screenshot("sheet.ppm");click("DES 16\n+3");
    click("Lancia d20");assert(result.total==13&&result.count==1);screenshot("dice.ppm");
    click("Vant");click("Lancia d20");assert(result.total==13&&result.count==2);
    click("Usi");click("-1");assert(state.resources[0].current==0);click("+1");assert(state.resources[0].current==1);screenshot("resources.ppm");
    pending=state;strcpy(pending.name,"Nuovo eroe");pending.hp=6;import_pending=true;render();bounds();screenshot("confirm.ppm");
    click("Annulla");assert(state.hp==24&&!import_pending);
    import_pending=true;render();click("Conferma");assert(state.hp==6&&!import_pending&&!undo_available);
    puts("LVGL host: navigation, base d20, HP/undo, counters, confirm/cancel, failed-save rollback, circular touch bounds >=44px passed");
}
