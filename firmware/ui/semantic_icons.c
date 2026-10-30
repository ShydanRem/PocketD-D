#include "semantic_icons.h"
#include "magic.h"
#include <string.h>
#include "semantic_icons.inc"
static unsigned lower(unsigned c){return c>='A'&&c<='Z'?c+32:c;}
static bool contains(const char *name,const char *alias){for(unsigned i=0;name[i];i++){unsigned j=0;while(alias[j]&&name[i+j]&&lower((unsigned char)name[i+j])==(unsigned char)alias[j])j++;if(!alias[j])return true;}return false;}
static const lv_image_dsc_t *find(unsigned family,const char *name){for(unsigned i=0;i<sizeof semantic_aliases/sizeof semantic_aliases[0];i++)if(semantic_aliases[i].family==family&&contains(name,semantic_aliases[i].alias))return semantic_aliases[i].image;return NULL;}
const lv_image_dsc_t *pd_item_icon(const pd_state *s,unsigned index){
 if(!s||index>=s->item_count)return &sem_gear;
 const pd_item *v=&s->items[index];if(v->kind==PD_HEALING)return &sem_heal;if(s->gear[index].kind==PD_WEAR_SHIELD)return &sem_shield;if(s->gear[index].kind>=PD_WEAR_LIGHT)return &sem_armor;
 const lv_image_dsc_t *im=find(0,v->name);return im?im:v->kind==PD_WEAPON?&sem_sword:v->kind==PD_CONSUMABLE?&sem_bottle:&sem_gear;
}
const lv_image_dsc_t *pd_power_icon(const pd_power *p){if(!p)return &sem_scroll;if(p->effect==PD_TRUE_STRIKE_2024)return &sem_strike;if(p->effect==1)return &sem_rage;if(p->effect==2)return &sem_heal;const lv_image_dsc_t *im=find(p->kind?2:1,p->name);return im?im:p->kind?&sem_feature:&sem_scroll;}

unsigned pd_item_setup_hint(const pd_state *s,unsigned index){if(!s||index>=s->item_count)return 0;const lv_image_dsc_t *im=find(0,s->items[index].name);return im==&sem_sword||im==&sem_axe||im==&sem_bow||im==&sem_hammer||im==&sem_staff?1:im==&sem_armor||im==&sem_shield?2:0;}
