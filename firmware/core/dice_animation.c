#include "dice_animation.h"
#include "visual.h"
#include "../assets/d20_atlas.h"
#include "../assets/poly_atlas.h"
#include <assert.h>

void pd_dice_idle(pd_dice_animation *a,unsigned count) {
    *a=(pd_dice_animation){.preview=true,.roll={.count=count==2?2:1,.dice={20,20},.natural=20}};
}
void pd_dice_start(pd_dice_animation *a,const pd_roll *r,uint32_t now) {
    assert(r->count==1 || r->count==2);
    *a=(pd_dice_animation){.roll=*r,.started=now,.active=true};
}
void pd_dice_pool_idle(pd_dice_animation *a,unsigned sides,unsigned count) {
    assert(pd_pool_sides(sides)&&count>=1&&count<=PD_POOL_MAX);
    *a=(pd_dice_animation){.pool_roll=true,.preview=true,.pool={.sides=sides,.count=count}};
    for(unsigned i=0;i<count;i++)a->pool.dice[i]=sides;
}
void pd_dice_pool_start(pd_dice_animation *a,const pd_pool *r,uint32_t now) {
    assert(pd_pool_sides(r->sides)&&r->count>=1&&r->count<=PD_POOL_MAX);
    *a=(pd_dice_animation){.pool_roll=true,.pool=*r,.started=now,.active=true};
}
unsigned pd_dice_count(const pd_dice_animation *a) { return a->pool_roll?a->pool.count:a->roll.count; }
unsigned pd_dice_sides(const pd_dice_animation *a) { return a->pool_roll?a->pool.sides:20; }
static bool twenty(const pd_dice_animation *a) { return !a->pool_roll&&a->roll.natural==20; }
static uint32_t flight(const pd_dice_animation *a) { return PD_DICE_FLIGHT_MS+(pd_dice_count(a)-1)*PD_DICE_STAGGER_MS; }
uint32_t pd_dice_duration(const pd_dice_animation *a) {
    return flight(a)+(twenty(a)?PD_DICE_SPARK_MS:0);
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
    unsigned count=pd_dice_count(a);assert(die<count);
    unsigned width=count>6?6:count==4?2:count>3?3:count;
    unsigned row=die/width,columns=count>6?(count-row*width<width?count-row*width:width):count>3?(row?count-width:width):count;
    unsigned column=die%width;
    int target=count>6?(PD_DICE_WIDTH-(int)columns*52)/2+26+column*52:columns==1?160:columns==2?(column?218:102):54+column*106;
    int rows=(count+width-1)/width;
    int base=count>6?(PD_DICE_HEIGHT-rows*40)/2+20+row*40:count>3?(row?96:32):68;
    int size=count>6?40:count>3?64:96;
    if (a->preview || a->elapsed>=PD_DICE_FLIGHT_MS+die*PD_DICE_STAGGER_MS)
        return (pd_dice_pose){.x=target,.y=base,.frame=0,.settled=true,.size=size};
    int t=(int)a->elapsed-(int)die*PD_DICE_STAGGER_MS;
    if (t<0) t=0;
    int progress=ease(t,PD_DICE_FLIGHT_MS),start=die?PD_DICE_WIDTH+50:-50;
    int height;
    if (t<540) height=4*30*t*(540-t)/(540*540);
    else if (t<880) { int b=t-540;height=4*15*b*(340-b)/(340*340); }
    else { int b=t-880;height=4*5*b*(220-b)/(220*220); }
    if(count>3)height=height/2;
    return (pd_dice_pose){.x=start+(target-start)*progress/1024,.y=base-height,.size=size,
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
static void digit(uint16_t *p,int x,int y,unsigned d,int unit,uint16_t c) {
    for(int j=0;j<5;j++)for(int i=0;i<3;i++)if(digits[d][j]&(1<<(2-i)))block(p,x+i*unit,y+j*unit,unit,unit,c);
}
static void number(uint16_t *p,int x,int y,unsigned n,int unit) {
    int start=x-(n>=10?7*unit/2:3*unit/2),gap=4*unit,shadow=unit/2;
    if(n>=10)digit(p,start+shadow,y+shadow,n/10,unit,color(0x26342e));
    digit(p,start+(n>=10?gap:0)+shadow,y+shadow,n%10,unit,color(0x26342e));
    if(n>=10)digit(p,start,y,n/10,unit,color(0xfff0c2));
    digit(p,start+(n>=10?gap:0),y,n%10,unit,color(0xfff0c2));
}
static const uint8_t *atlas(unsigned sides,int frame) {
    switch(sides) {
        case 4:return pd_d4_atlas[frame];case 6:return pd_d6_atlas[frame];
        case 8:return pd_d8_atlas[frame];case 10:return pd_d10_atlas[frame];
        case 12:return pd_d12_atlas[frame];default:return pd_d20_atlas[frame];
    }
}
static void star(uint16_t *p,int x,int y,int r,uint16_t c) {
    block(p,x-2,y-r,4,r*2+2,c);block(p,x-r,y-2,r*2+2,4,c);
    block(p,x-4,y-4,8,8,c);
}
void pd_dice_paint(const pd_dice_animation *a,uint16_t p[PD_DICE_PIXELS]) {
    uint16_t bg=color(PD_INK);
    for(int i=0;i<PD_DICE_PIXELS;i++)p[i]=bg;
    /* Stepped inlaid tabletop: pure presentation, independent of the roll. */
    for(int y=14;y<116;y+=2)for(int x=8;x<PD_DICE_WIDTH-8;x+=2){
        int inset=y<28?(28-y):y>102?(y-102):0;
        if(x<8+inset||x>=PD_DICE_WIDTH-8-inset)continue;
        block(p,x,y,2,2,color((x/16+y/12)%2?0x101e2b:0x12212e));
    }
    for(int x=24;x<PD_DICE_WIDTH-24;x+=8){block(p,x,117,4,2,color(PD_EDGE));block(p,x,12,4,2,color(0x293e4a));}
    for(int y=34;y<98;y+=8){block(p,10,y,2,4,color(0x354b50));block(p,PD_DICE_WIDTH-12,y,2,4,color(0x354b50));}
    unsigned selected=a->roll.dice[0]==a->roll.natural?0:1;
    unsigned count=pd_dice_count(a),sides=pd_dice_sides(a);
    for(unsigned d=0;d<count;d++) {
        pd_dice_pose pose=pd_dice_position(a,d);
        bool discarded=!a->pool_roll && !a->preview && !a->active && count==2 && d!=selected;
        const unsigned gold[]={0,0x080f10,0xffde8b,0x624526,0xa27338,0xd8a850,0xffd87c};
        const unsigned mint[]={0,0x080f10,0xc0e3c2,0x27473d,0x43846b,0x68b58a,0xa4d8ad};
        const unsigned violet[]={0,0x080f10,0xe2c9ec,0x49304f,0x815483,0xb47bb2,0xe2addc};
        const unsigned gray[]={0,0x0a1113,0x78928c,0x283c37,0x3c5750,0x5b766a,0x8ca697};
        uint16_t palette[7];for(int i=0;i<7;i++)palette[i]=color(discarded?gray[i]:(sides==6||sides==8)?mint[i]:(sides==4||sides==12)?violet[i]:gold[i]);
        int radius=pose.size/4-pose.height/3;
        int ground=pose.size<=64?(pose.y+pose.height+pose.size/2-4):111;
        for(int y=-4;y<=4;y+=2)for(int x=-radius;x<=radius;x+=2)
            if(x*x*16+y*y*radius*radius<=16*radius*radius)block(p,pose.x+x,ground+y,2,2,color(0x080e0c));
        const uint8_t *sprite=atlas(sides,pose.frame);
        int logical=pose.size/2;
        for(int y=0;y<logical;y++)for(int x=0;x<logical;x++) {
            unsigned offset=(y*PD_D20_SIZE/logical)*PD_D20_SIZE+x*PD_D20_SIZE/logical;
            uint8_t ink=offset&1?sprite[offset/2]&15:sprite[offset/2]>>4;
            if(ink)block(p,pose.x-pose.size/2+x*2,pose.y-pose.size/2+y*2,2,2,palette[ink]);
        }
        unsigned face=pose.settled?(a->pool_roll?a->pool.dice[d]:a->roll.dice[d]):1+(pose.frame*7+d*11)%sides;
        number(p,pose.x+(sides==6?pose.size/12:0),pose.y-(pose.size==64?6:8)+(sides==6?pose.size/24:0),face,pose.size==40?2:pose.size==64?3:4);
        if(!a->pool_roll && !a->active && !a->preview && d==selected) {
            /* A small gold chevron indicates the selected die, including ties. */
            for(int i=0;i<3;i++)block(p,pose.x-6+i*2,119+i*2,12-i*4,2,color(0xffde8b));
        }
    }
    if(!a->preview && twenty(a) && a->elapsed>=flight(a)) {
        pd_dice_pose pose=pd_dice_position(a,selected);
        int burst=a->active?(int)(a->elapsed-flight(a))*10/PD_DICE_SPARK_MS:8;
        star(p,pose.x-55-burst,pose.y-26,3,color(0xffde8b));
        star(p,pose.x+49+burst,pose.y-12,4,color(0xffe9ab));
        star(p,pose.x+24,pose.y-47-burst/2,2,color(0xa7c99c));
    }
}
