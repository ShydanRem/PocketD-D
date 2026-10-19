#include "dice_animation.h"
#include "../assets/d20_atlas.h"
#include <assert.h>

void pd_dice_idle(pd_dice_animation *a,unsigned count) {
    *a=(pd_dice_animation){.preview=true,.roll={.count=count==2?2:1,.dice={20,20},.natural=20}};
}
void pd_dice_start(pd_dice_animation *a,const pd_roll *r,uint32_t now) {
    assert(r->count==1 || r->count==2);
    *a=(pd_dice_animation){.roll=*r,.started=now,.active=true};
}
static uint32_t flight(const pd_dice_animation *a) { return PD_DICE_FLIGHT_MS+(a->roll.count==2?PD_DICE_STAGGER_MS:0); }
uint32_t pd_dice_duration(const pd_dice_animation *a) {
    return flight(a)+(a->roll.natural==20?PD_DICE_SPARK_MS:0);
}
bool pd_dice_tick(pd_dice_animation *a,uint32_t now) {
    if (!a->active) return false;
    uint32_t elapsed=now-a->started, duration=pd_dice_duration(a);
    a->elapsed=elapsed<duration?elapsed:duration;
    if (a->elapsed==duration) { a->active=false;return true; }
    return false;
}
void pd_dice_finish(pd_dice_animation *a) { a->elapsed=pd_dice_duration(a);a->active=false; }
static int ease(int t,int duration) {
    int64_t remaining=duration-t;
    return 1024-(remaining*remaining*remaining*1024)/((int64_t)duration*duration*duration);
}
pd_dice_pose pd_dice_position(const pd_dice_animation *a,unsigned die) {
    assert(die<a->roll.count);
    int target=a->roll.count==2?(die?218:102):160;
    if (a->preview || a->elapsed>=PD_DICE_FLIGHT_MS+die*PD_DICE_STAGGER_MS)
        return (pd_dice_pose){.x=target,.y=68,.frame=0,.settled=true};
    int t=(int)a->elapsed-(int)die*PD_DICE_STAGGER_MS;
    if (t<0) t=0;
    int progress=ease(t,PD_DICE_FLIGHT_MS),start=die?PD_DICE_WIDTH+50:-50;
    int height;
    if (t<540) height=4*30*t*(540-t)/(540*540);
    else if (t<880) { int b=t-540;height=4*15*b*(340-b)/(340*340); }
    else { int b=t-880;height=4*5*b*(220-b)/(220*220); }
    return (pd_dice_pose){.x=start+(target-start)*progress/1024,.y=68-height,
        .height=height,.frame=((die?4:3)*PD_D20_FRAMES*progress/1024)%PD_D20_FRAMES};
}
static uint16_t color(unsigned rgb) { return ((rgb>>8)&0xf800)|((rgb>>5)&0x7e0)|((rgb>>3)&31); }
static void dot(uint16_t *p,int x,int y,uint16_t c) {
    if ((unsigned)x<PD_DICE_WIDTH && (unsigned)y<PD_DICE_HEIGHT) p[y*PD_DICE_WIDTH+x]=c;
}
static void block(uint16_t *p,int x,int y,int w,int h,uint16_t c) {
    for(int j=0;j<h;j++)for(int i=0;i<w;i++)dot(p,x+i,y+j,c);
}
static const uint8_t digits[10][5]={
    {7,5,5,5,7},{2,6,2,2,7},{7,1,7,4,7},{7,1,7,1,7},{5,5,7,1,1},
    {7,4,7,1,7},{7,4,7,5,7},{7,1,1,1,1},{7,5,7,5,7},{7,5,7,1,7}
};
static void digit(uint16_t *p,int x,int y,unsigned d,uint16_t c) {
    for(int j=0;j<5;j++)for(int i=0;i<3;i++)if(digits[d][j]&(1<<(2-i)))block(p,x+i*4,y+j*4,4,4,c);
}
static void number(uint16_t *p,int x,int y,unsigned n) {
    int start=x-(n>=10?14:6);
    /* Block shadow keeps the natural roll legible over changing facets. */
    if(n>=10)digit(p,start+2,y+2,n/10,color(0x26342e));
    digit(p,start+(n>=10?16:0)+2,y+2,n%10,color(0x26342e));
    if(n>=10)digit(p,start,y,n/10,color(0xfff0c2));
    digit(p,start+(n>=10?16:0),y,n%10,color(0xfff0c2));
}
static void star(uint16_t *p,int x,int y,int r,uint16_t c) {
    block(p,x-2,y-r,4,r*2+2,c);block(p,x-r,y-2,r*2+2,4,c);
    block(p,x-4,y-4,8,8,c);
}
void pd_dice_paint(const pd_dice_animation *a,uint16_t p[PD_DICE_PIXELS]) {
    uint16_t bg=color(0x0d1916);
    for(int i=0;i<PD_DICE_PIXELS;i++)p[i]=bg;
    /* A restrained pixel tray; all positions are local to the circular safe area. */
    for(int x=24;x<PD_DICE_WIDTH-24;x+=8)block(p,x,117,4,2,color(0x20382e));
    unsigned selected=a->roll.dice[0]==a->roll.natural?0:1;
    for(unsigned d=0;d<a->roll.count;d++) {
        pd_dice_pose pose=pd_dice_position(a,d);
        bool discarded=!a->preview && !a->active && a->roll.count==2 && d!=selected;
        const unsigned gold[]={0,0x080f10,0xffde8b,0x624526,0xa27338,0xd8a850,0xffd87c};
        const unsigned gray[]={0,0x0a1113,0x78928c,0x283c37,0x3c5750,0x5b766a,0x8ca697};
        uint16_t palette[7];for(int i=0;i<7;i++)palette[i]=color(discarded?gray[i]:gold[i]);
        int radius=26-pose.height/3;
        for(int y=-4;y<=4;y+=2)for(int x=-radius;x<=radius;x+=2)
            if(x*x*16+y*y*radius*radius<=16*radius*radius)block(p,pose.x+x,111+y,2,2,color(0x080e0c));
        const uint8_t *sprite=pd_d20_atlas[pose.frame];
        for(int y=0;y<PD_D20_SIZE;y++)for(int x=0;x<PD_D20_SIZE;x++) {
            unsigned offset=y*PD_D20_SIZE+x;
            uint8_t ink=offset&1?sprite[offset/2]&15:sprite[offset/2]>>4;
            if(ink)block(p,pose.x-48+x*2,pose.y-48+y*2,2,2,palette[ink]);
        }
        unsigned face=pose.settled?a->roll.dice[d]:1+(pose.frame*7+d*11)%20;
        number(p,pose.x,pose.y-8,face);
        if(!a->active && !a->preview && d==selected) {
            /* A small gold chevron indicates the selected die, including ties. */
            for(int i=0;i<3;i++)block(p,pose.x-6+i*2,119+i*2,12-i*4,2,color(0xffde8b));
        }
    }
    if(!a->preview && a->roll.natural==20 && a->elapsed>=flight(a)) {
        pd_dice_pose pose=pd_dice_position(a,selected);
        int burst=a->active?(int)(a->elapsed-flight(a))*10/PD_DICE_SPARK_MS:8;
        star(p,pose.x-55-burst,pose.y-26,3,color(0xffde8b));
        star(p,pose.x+49+burst,pose.y-12,4,color(0xffe9ab));
        star(p,pose.x+24,pose.y-47-burst/2,2,color(0xa7c99c));
    }
}
