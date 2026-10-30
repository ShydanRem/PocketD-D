/* Device-local equipment/spell setup. No rule is granted by an icon or name. */
static lv_obj_t *wifi_button(const char *,int,int,int,int,int,bool);
static void wifi_mark_focus(void);
static bool play_editing(void){return page>=WEAPON_PICKER&&page<=HIT_SMITE;}
static unsigned picker_page,weapon_template,gear_field,play_preview_page,play_mode;
static pd_item weapon_draft;
static pd_gear gear_draft;
static unsigned gear_rule,gear_adopt;
static bool weapon_from_attack,profile_true;
static unsigned profile_on_hit;
static uint8_t profile_ability;
static int profile_attack,profile_damage;
static page_t play_return;
static const char *play_error="";
static int attack_spell=-1;
static bool attack_radiant;
static int pending_attack_spell=-1;
static page_t picker_return=HERO;
static unsigned attack_mode_page;
static page_t attack_return=HERO;
static int attack_cast_power=-1;
static unsigned smite_source,smite_cost;
static bool smite_pact,smite_fiend,smite_target;
static bool smite_cost_at(unsigned power,unsigned at,unsigned *slot,bool *pact){
 if(power>=state.magic.count||state.magic.powers[power].effect!=PD_DIVINE_SMITE_2024)return false;
 const pd_power *p=&state.magic.powers[power];
 if(!(p->flags&PD_USES_SLOT)){if(at||!pd_smite_prepare(&state,&strike,power,0,false,false,NULL,NULL))return false;*slot=0;*pact=false;return true;}
 for(unsigned k=0;k<2;k++)for(unsigned i=1;i<=9;i++)if((k?state.magic.pact:state.magic.slots)[i-1].current&&pd_smite_prepare(&state,&strike,power,i,k,false,NULL,NULL)){if(!at){*slot=i;*pact=k;return true;}at--;}
 return false;
}
static unsigned smite_at(unsigned at){for(unsigned i=0;i<state.magic.count;i++){unsigned slot;bool pact;if(smite_cost_at(i,0,&slot,&pact)){if(!at)return i;at--;}}return PD_POWERS;}
static void smite_begin(void){smite_source=smite_at(0);smite_cost_at(smite_source,0,&smite_cost,&smite_pact);smite_fiend=smite_target=smite_rolled=false;page=HIT_SMITE;wifi_focus=0;}
static bool attack_context_candidate(const pd_power *p);
static bool weapon_spell_candidate(const pd_power *p){return p->effect==PD_TRUE_STRIKE_2024||((strstr(p->name,"True Strike")||strstr(p->name,"Colpo Accurato"))&&(strstr(p->description,"spellcasting ability")||strstr(p->description,"caratteristica da incantatore")));}
static bool attack_context_candidate(const pd_power *p){return weapon_spell_candidate(p)||p->effect>=PD_HEX_2024;}
/* Ready, then unprepared, then explicit setup proposals. Never grant rules by name. */
static unsigned attack_power_group(const pd_power *p){return p->effect<PD_TRUE_STRIKE_2024?2:!(p->flags&PD_PREPARED);}
static unsigned attack_power_at(unsigned at){if(state.magic.available)for(unsigned group=0;group<3;group++)for(unsigned i=0;i<state.magic.count;i++)if(attack_context_candidate(&state.magic.powers[i])&&attack_power_group(&state.magic.powers[i])==group){if(!at)return i;at--;}return PD_POWERS;}
static unsigned attack_power_count(void){unsigned n=0;if(state.magic.available)for(unsigned i=0;i<state.magic.count;i++)if(attack_context_candidate(&state.magic.powers[i]))n++;return n;}
static bool attack_weapon_info(pd_weapon_info *out){return attack_spell>=0?pd_true_strike_stats(&state,(unsigned)attack_spell,attack_radiant,out):pd_weapon_stats(&state,state.equipped,out);}
static void attack_select(int power){
 attack_spell=power;page=COMBAT;combat_roll=1;pool_view=false;has_roll=false;strike_ready=false;damage_used=false;wifi_focus=0;
 pd_dice_idle(&dice_animation,pd_attack_mode(&state,mode)?2:1);
}
static void attack_choose(void){attack_mode_page=0;page=ATTACK_MODE;wifi_focus=0;}
static void attack_start(int power,bool choose){
 mode=0;roll_context=(pd_roll_context){0};ignore_effects=false;attack_select(power);
 if(choose&&attack_power_count())attack_choose();
}

static const struct {const char *name,*damage;uint8_t count,sides,ability,flags,reach,thrown;} weapon_templates[]={
 {"Spada","Tagliente",1,8,0,2,1,0},{"Spadone","Tagliente",2,6,0,10,1,0},{"Stocco","Perforante",1,8,1,2,1,0},{"Pugnale","Perforante",1,4,1,2,1,4},{"Ascia bipenne","Tagliente",1,12,0,10,1,0},{"Arco corto","Perforante",1,6,1,6,16,0},{"Mazza","Contundente",1,6,0,2,1,0},{"Lancia","Perforante",1,6,0,2,1,4}
};
static bool weapon_candidate(unsigned index){return index<state.item_count&&state.items[index].quantity&&(state.items[index].kind==PD_WEAPON||(state.items[index].kind==PD_GEAR&&state.gear[index].kind<=PD_WEAR_ITEM));}
static unsigned picker_count(void){unsigned n=0;for(unsigned i=0;i<state.item_count;i++)if(weapon_candidate(i))n++;return n;}
static unsigned picker_at(unsigned at){for(unsigned i=0;i<state.item_count;i++)if(weapon_candidate(i)){if(!at)return i;at--;}return PD_ITEMS;}
static void weapon_setup_begin(bool from_attack){
 weapon_from_attack=from_attack;weapon_draft=state.items[item_index];weapon_template=99;int a,d;pd_gear_weapon_bonus(&state,&a,&d);
 if(weapon_draft.kind==PD_WEAPON){weapon_draft.attack_modifier-=a;weapon_draft.damage_modifier-=d;}
 else{weapon_draft.kind=PD_WEAPON;weapon_draft.ability=0;weapon_draft.count=1;weapon_draft.sides=8;weapon_draft.flags=2;weapon_draft.reach=1;weapon_draft.thrown_range=0;weapon_draft.attack_modifier=weapon_draft.damage_modifier=weapon_draft.healing_modifier=0;strcpy(weapon_draft.damage_type,"Tagliente");weapon_template=0;}
 page=WEAPON_SETUP;wifi_focus=0;
}
static void profile_setup_begin(void){
 const pd_power *p=&state.magic.powers[power_index];profile_true=p->effect==PD_TRUE_STRIKE_2024;profile_on_hit=p->effect>=PD_HEX_2024?p->effect:0;profile_ability=p->ability;profile_attack=profile_true?p->attack_modifier:0;profile_damage=profile_true?p->save_modifier:0;
 if(p->kind){unsigned counts[6]={0};for(unsigned i=0;i<state.magic.count;i++)if(!state.magic.powers[i].kind)counts[state.magic.powers[i].ability]++;for(unsigned i=0;i<6;i++)if(counts[i]>counts[profile_ability])profile_ability=i;}
 page=MAGIC_PROFILE;wifi_focus=0;
}
static bool play_candidate(void){
 scratch=state;
 if(play_mode==0){return state.items[item_index].kind==PD_WEAPON?pd_equip(&scratch,play_return!=WEAPON_PICKER&&state.equipped==item_index?PD_NO_WEAPON:item_index):pd_gear_equip(&scratch,item_index,!state.gear[item_index].equipped);}
 if(play_mode==4)return pd_gear_attune(&scratch,item_index,state.gear[item_index].attunement!=2);
 if(play_mode==1){
  if(state.gear[item_index].kind&&!pd_gear_configure(&scratch,item_index,(pd_gear){0},state.unarmored_rule,false))return false;
  pd_item v=weapon_draft;int a,d;pd_gear_weapon_bonus(&scratch,&a,&d);int attack=v.attack_modifier+a,damage=v.damage_modifier+d;if(attack< -10||attack>10||damage< -10||damage>10)return false;v.attack_modifier=attack;v.damage_modifier=damage;scratch.items[item_index]=v;
  return pd_equip(&scratch,item_index)&&pd_valid(&scratch);
 }
 if(play_mode==2){if(!gear_adopt)return false;return pd_gear_configure(&scratch,item_index,gear_draft,gear_rule,gear_adopt==1);}
 pd_power *p=&scratch.magic.powers[power_index];
 if(p->effect!=(profile_true?PD_TRUE_STRIKE_2024:profile_on_hit)&&scratch.magic.concentration==power_index+1)return false;
 if(profile_true){if(p->level||p->maximum||(p->flags&PD_LIMITED)||scratch.magic.concentration==power_index+1)return false;p->kind=0;p->effect=PD_TRUE_STRIKE_2024;p->level=0;p->flags=(p->flags&PD_EXCERPT)|PD_PREPARED;p->maximum=p->remaining=p->recovery=0;p->ability=profile_ability;p->attack_modifier=profile_attack;p->save_modifier=profile_damage;}
 else if(profile_on_hit){if(p->kind||p->level!=1||(profile_on_hit==PD_DIVINE_SMITE_2024?!!(p->flags&PD_CONCENTRATES):!(p->flags&PD_CONCENTRATES)))return false;p->effect=profile_on_hit;}
 else if(p->effect>=PD_TRUE_STRIKE_2024){p->effect=0;p->attack_modifier=p->save_modifier=0;}
 return pd_valid(&scratch);
}
static void play_preview(unsigned mode,page_t back){play_mode=mode;play_return=back;play_preview_page=0;page=PLAY_CONFIRM;wifi_focus=0;}
static unsigned play_changes(char lines[32][96]){
 unsigned n=0;
 if(play_mode==4)snprintf(lines[n++],96,"%s",scratch.gear[item_index].attunement==2?"Attiva sintonia":"Termina sintonia");
 else if(play_mode==3)snprintf(lines[n++],96,"Profilo: %s",profile_true?"True Strike 2024":profile_on_hit==PD_DIVINE_SMITE_2024?"Punizione 2024":profile_on_hit==PD_HEX_2024?"Hex 2024":profile_on_hit?"Marchio 2024":"effetti manuali");
 else snprintf(lines[n++],96,"%s",play_mode==1?"Salva arma ed equipaggia":scratch.gear[item_index].equipped?"Oggetto equipaggiato":"Oggetto riposto");
 if(scratch.armor!=state.armor)snprintf(lines[n++],96,"CA base: %u -> %u",state.armor,scratch.armor);
 if(scratch.max_hp!=state.max_hp)snprintf(lines[n++],96,"PF massimi: %u -> %u",state.max_hp,scratch.max_hp);
 if(scratch.hp!=state.hp)snprintf(lines[n++],96,"PF attuali: %u -> %u",state.hp,scratch.hp);
 for(unsigned i=0;i<6;i++)if(scratch.abilities[i]!=state.abilities[i])snprintf(lines[n++],96,"%s: %u -> %u",pd_abilities[i],state.abilities[i],scratch.abilities[i]);
 pd_weapon_info old,new;if(pd_weapon_stats(&scratch,scratch.equipped,&new)){
  if(pd_weapon_stats(&state,state.equipped,&old)){if(old.attack_bonus!=new.attack_bonus)snprintf(lines[n++],96,"Tiro per colpire: %+d -> %+d",old.attack_bonus,new.attack_bonus);if(old.damage_bonus!=new.damage_bonus)snprintf(lines[n++],96,"Bonus danni: %+d -> %+d",old.damage_bonus,new.damage_bonus);}
  else{snprintf(lines[n++],96,"Attacco: %+d",new.attack_bonus);snprintf(lines[n++],96,"Danni: %ud%u %+d",new.count,new.sides,new.damage_bonus);}
 }
 if(play_mode==2){const pd_gear *old=&state.gear[item_index];if(old->spell_attack!=gear_draft.spell_attack||old->spell_dc!=gear_draft.spell_dc)snprintf(lines[n++],96,"Bonus magie: att %+d / CD %+d",gear_draft.spell_attack,gear_draft.spell_dc);if(old->saves!=gear_draft.saves||old->checks!=gear_draft.checks)snprintf(lines[n++],96,"Bonus tiri: prove %+d / TS %+d",gear_draft.checks,gear_draft.saves);}
 if(play_mode!=3){
  const pd_gear *g=&scratch.gear[item_index];
  if(g->equipped&&g->attunement==1)snprintf(lines[n++],96,"Senza sintonia: bonus inattivi");
  unsigned seen=0;for(unsigned i=0;i<state.magic.count;i++)if(!state.magic.powers[i].kind&&state.magic.powers[i].effect!=PD_TRUE_STRIKE_2024&&!(seen&(1u<<state.magic.powers[i].ability))){int a,dc,b,bc;if(pd_magic_stats(&state,i,&a,&dc)&&pd_magic_stats(&scratch,i,&b,&bc)&&(a!=b||dc!=bc)){snprintf(lines[n++],96,"Magie %s: att %+d -> %+d",pd_abilities[scratch.magic.powers[i].ability],a,b);snprintf(lines[n++],96,"CD magie: %d -> %d",dc,bc);seen|=1u<<state.magic.powers[i].ability;}}
  int checks=scratch.training.check_bonus[0]-state.training.check_bonus[0],saves=scratch.training.save_bonus[0]-state.training.save_bonus[0];if(checks||saves)snprintf(lines[n++],96,"Extra tiri: prove %+d / TS %+d",checks,saves);
  if(play_mode==4){unsigned total=0;for(unsigned i=0;i<scratch.item_count;i++)total+=scratch.gear[i].attunement==2;snprintf(lines[n++],96,"Oggetti in sintonia: %u",total);}
 }
 if(play_mode==3&&profile_true){pd_weapon_info w;if(pd_true_strike_stats(&scratch,power_index,false,&w)){snprintf(lines[n++],96,"%s: att %+d / danni %+d",pd_abilities[profile_ability],w.attack_bonus,w.damage_bonus);snprintf(lines[n++],96,"Extra: %ud6 radianti",w.radiant_dice);}else snprintf(lines[n++],96,"Scegli un arma per usare la magia");}
 if(play_mode==3&&profile_on_hit==PD_DIVINE_SMITE_2024){snprintf(lines[n++],96,"Dopo il colpo: 2d8 + slot/bersaglio");snprintf(lines[n++],96,"Radianti; richiede azione bonus");}
 else if(play_mode==3&&profile_on_hit){snprintf(lines[n++],96,"Extra: 1d6 %s",profile_on_hit==PD_HEX_2024?"necrotici":"forza");snprintf(lines[n++],96,"Concentrazione e bersaglio richiesti");snprintf(lines[n++],96,"Tempi e potenziamenti: manuali");}
 return n;
}
static bool play_action(intptr_t a){
 play_error="";
 if(page==HIT_SMITE){
  if(smite_rolled&&a!=6479)return true;
  if(a==6099)page=DICE;
  else if(a==6480)attack_damage_finish(smite_target);
  else if(a==6470){unsigned at=0;while(smite_at(at)<PD_POWERS&&smite_at(at)!=smite_source)at++;unsigned next=smite_at(at+1);smite_source=next<PD_POWERS?next:smite_at(0);smite_cost_at(smite_source,0,&smite_cost,&smite_pact);}
  else if(a==6471){unsigned at=0,slot;bool pact;while(smite_cost_at(smite_source,at,&slot,&pact)&&(slot!=smite_cost||pact!=smite_pact))at++;if(!smite_cost_at(smite_source,at+1,&smite_cost,&smite_pact))smite_cost_at(smite_source,0,&smite_cost,&smite_pact);}
  else if(a==6472)smite_fiend=!smite_fiend;
  else if(a==6473&&pd_on_hit_spell(&state)>=0)smite_target=!smite_target;
  else if(a==6479&&!attack_smite_finish(smite_source,smite_cost,smite_pact,smite_fiend,smite_target))play_error="Salvataggio non confermato: riprova";
  return true;
 }
 if(a==6099){if(page==HIT_EFFECTS)page=DICE;else if(page==WEAPON_PICKER){page=picker_return;pending_attack_spell=-1;}else if(page==PLAY_CONFIRM)page=play_return;else if(page==MAGIC_PROFILE)page=MAGIC_DETAIL;else if(page==WEAPON_SETUP)page=weapon_from_attack?WEAPON_PICKER:ITEM_CONFIG_MENU;else if(page==GEAR_SETUP)page=ITEM_CONFIG_MENU;else if(page==ATTACK_MODE)page=attack_return;else page=ITEM_DETAIL;wifi_focus=0;return true;}
 if(page==HIT_EFFECTS){if(a==6460)attack_damage_finish(false);else if(a==6461)attack_damage_finish(true);return true;}
 if(a==6500&&page==ITEM_DETAIL&&state.gear[item_index].attunement){play_preview(4,ITEM_DETAIL);return true;}
 if(a==6090&&page==ITEM_DETAIL){page=ITEM_CONFIG_MENU;return true;}
 if(a==84&&page==ITEM_DETAIL){if(state.items[item_index].kind==PD_GEAR&&pd_gear_unconfigured(&state,item_index)&&pd_item_setup_hint(&state,item_index)){page=ITEM_CONFIG_MENU;return true;}play_preview(0,ITEM_DETAIL);return true;}
 if(a==6402&&page==COMBAT){attack_return=COMBAT;attack_choose();return true;}
 if(a==6403&&page==COMBAT){attack_return=COMBAT;picker_page=0;weapon_from_attack=true;pending_attack_spell=attack_spell;picker_return=COMBAT;page=WEAPON_PICKER;return true;}
 if(page==ATTACK_MODE){
  if(a==6400&&attack_mode_page)attack_mode_page--;else if(a==6401&&(attack_mode_page+1)*2<attack_power_count())attack_mode_page++;
  else if(a==6410){pd_weapon_info w;if(pd_weapon_stats(&state,state.equipped,&w))attack_select(-1);}
  else if(a==6420){attack_radiant=!attack_radiant;strike_ready=damage_used=has_roll=false;pd_dice_idle(&dice_animation,1);}
  else if(a==6411||a==6412){unsigned i=attack_power_at(attack_mode_page*2+a-6411);if(i<PD_POWERS){
   power_index=i;const pd_power *p=&state.magic.powers[i];
   if(p->effect==PD_DIVINE_SMITE_2024){if(!state.hp||state.combat.raging)return true;if(!(p->flags&PD_PREPARED)){preparation_return=MAGIC_DETAIL;preparation_target=true;page=MAGIC_PREPARATION;}else attack_select(attack_spell);}
   else if(p->effect>=PD_HEX_2024){if(!state.hp||state.combat.raging)return true;if(pd_on_hit_spell(&state)==(int)i)attack_select(attack_spell);else{attack_cast_power=i;page=MAGIC_DETAIL;cast_level=0;cast_pact=false;}}
   else if(p->effect!=PD_TRUE_STRIKE_2024)profile_setup_begin();
   else if(state.hp&&!state.combat.raging){pd_weapon_info w;
    if(!(p->flags&PD_PREPARED)){preparation_return=ATTACK_MODE;preparation_target=true;page=MAGIC_PREPARATION;}
    else if(pd_true_strike_stats(&state,i,attack_radiant,&w))attack_select(i);
   }
  }}
  return true;
 }
 if(page==WEAPON_PICKER){
  if(a==6200&&picker_page)picker_page--;else if(a==6201&&(picker_page+1)*3<picker_count())picker_page++;
  else if(a>=6210&&a<=6212){unsigned i=picker_at(picker_page*3+a-6210);if(i<PD_ITEMS){item_index=i;if(state.items[i].kind==PD_WEAPON){play_preview(0,WEAPON_PICKER);weapon_from_attack=true;}else weapon_setup_begin(true);}}
  else if(a==6202)page=INVENTORY;
  return true;
 }
 if(a==6000&&(page==ITEM_CONFIG_MENU||page==ITEM_DETAIL)){weapon_setup_begin(false);return true;}
 if(a==6100&&(page==ITEM_CONFIG_MENU||page==ITEM_DETAIL)){gear_draft=state.gear[item_index];if(!gear_draft.kind){gear_draft.kind=PD_WEAR_ITEM;gear_draft.equipped=true;}gear_rule=state.unarmored_rule?state.unarmored_rule:1;gear_adopt=pd_gear_unconfigured(&state,item_index)?0:2;gear_field=0;page=GEAR_SETUP;return true;}
 if(a==6300&&(page==MAGIC_MANAGE||page==MAGIC_DETAIL)){profile_setup_begin();return true;}
 if(page==WEAPON_SETUP){
  if(a==6002){weapon_template=weapon_template==99?0:(weapon_template+1)%(sizeof weapon_templates/sizeof weapon_templates[0]);const unsigned i=weapon_template;weapon_draft.count=weapon_templates[i].count;weapon_draft.sides=weapon_templates[i].sides;weapon_draft.ability=weapon_templates[i].ability;weapon_draft.flags=(weapon_draft.flags&1)|weapon_templates[i].flags;weapon_draft.reach=weapon_templates[i].reach;weapon_draft.thrown_range=weapon_templates[i].thrown;snprintf(weapon_draft.damage_type,sizeof weapon_draft.damage_type,"%s",weapon_templates[i].damage);}
  else if(a==6004)weapon_draft.ability=(weapon_draft.ability+1)%6;else if(a==6005)weapon_draft.flags^=1;
  else if(a==6006&&weapon_draft.attack_modifier> -10)weapon_draft.attack_modifier--;else if(a==6007&&weapon_draft.attack_modifier<10)weapon_draft.attack_modifier++;
  else if(a==6008&&weapon_draft.damage_modifier> -10)weapon_draft.damage_modifier--;else if(a==6009&&weapon_draft.damage_modifier<10)weapon_draft.damage_modifier++;
  else if(a==6010)play_preview(1,WEAPON_SETUP);
  return true;
 }
 if(page==GEAR_SETUP){
  if(a==6101&&gear_field)gear_field--;else if(a==6102&&gear_field<19)gear_field++;
  else if(a==6106)gear_field=gear_field<5?5:0;
  else if(a==6105)play_preview(2,GEAR_SETUP);
  else if(a==6103||a==6104){int d=a==6103?-1:1;
   if(gear_field==0){int kind=gear_draft.kind+d;if(kind>=1&&kind<=5){gear_draft.kind=kind;gear_draft.armor=(int[]){0,0,11,14,16,2}[kind];}}
   else if(gear_field==1&&gear_draft.kind>=2){int n=gear_draft.armor+d;if(n>=1&&n<=(gear_draft.kind==5?10:30))gear_draft.armor=n;}
   else if(gear_field==2)gear_draft.equipped=d>0;
   else if(gear_field==3&&pd_gear_unconfigured(&state,item_index))gear_adopt=d>0?1:2;
   else if(gear_field==4){int n=gear_rule+d;if(n>=0&&n<=3)gear_rule=n;}
   else if(gear_field>=5&&gear_field<=10){int n=gear_draft.ability[gear_field-5]+d;if(n>= -10&&n<=10)gear_draft.ability[gear_field-5]=n;}
   else if(gear_field>=11&&gear_field<=17){int8_t *fields[]={&gear_draft.ac,&gear_draft.attack,&gear_draft.damage,&gear_draft.spell_attack,&gear_draft.spell_dc,&gear_draft.saves,&gear_draft.checks};int n=*fields[gear_field-11]+d;if(n>= -10&&n<=10)*fields[gear_field-11]=n;}
   else if(gear_field==19){int n=gear_draft.attunement+d;if(n>=0&&n<=2)gear_draft.attunement=n;}
   else if(gear_field==18){int n=gear_draft.max_hp+d;if(n>= -999&&n<=999)gear_draft.max_hp=n;}
  }return true;
 }
 if(page==MAGIC_PROFILE){
  if(a==6301){const pd_power *p=&state.magic.powers[power_index];if(!p->level&&!p->maximum)profile_true=!profile_true;else if(!p->kind&&p->level==1){if(p->flags&PD_CONCENTRATES)profile_on_hit=!profile_on_hit?PD_HEX_2024:profile_on_hit==PD_HEX_2024?PD_HUNTERS_MARK_2024:0;else profile_on_hit=profile_on_hit?0:PD_DIVINE_SMITE_2024;}}
  else if(a==6302)profile_ability=(profile_ability+1)%6;
  else if(a==6303&&profile_attack> -15)profile_attack--;else if(a==6304&&profile_attack<30)profile_attack++;
  else if(a==6305&&profile_damage> -15)profile_damage--;else if(a==6306&&profile_damage<30)profile_damage++;
  else if(a==6307)play_preview(3,MAGIC_PROFILE);
  return true;
 }
 if(page==PLAY_CONFIRM){
  if(a==6096&&play_preview_page)play_preview_page--;else if(a==6097)play_preview_page++;
  else if(a==6098&&play_candidate()){
   if(pd_storage_save(&scratch)){previous=state;undo_available=true;state=scratch;invalidate_strike();wifi_focus=0;
    if(play_mode==3)page=MAGIC_DETAIL;else if((play_mode==1||play_return==WEAPON_PICKER)&&weapon_from_attack){int selected=pending_attack_spell;pending_attack_spell=-1;attack_start(selected,selected<0);}
    else page=ITEM_DETAIL;
   }else play_error="Salvataggio non confermato: riprova";
  }return true;
 }
 return false;
}
static void setup_name(const char *name,int y){lv_obj_t *l=small_label(name,83,y,300,PD_GOLD);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_height(l,24);}
static void attack_card(const char *name,const char *detail,const lv_image_dsc_t *icon,int y,int action,bool enabled,bool selected){
 lv_obj_t *b=wifi_button(name,83,y,300,48,action,enabled);art(icon,10,14,b);
 lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_style_text_font(l,&lv_font_montserrat_16,0);lv_obj_set_size(l,244,20);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_align(l,LV_ALIGN_TOP_LEFT);lv_obj_set_pos(l,45,4);
 l=lv_label_create(b);lv_label_set_text(l,detail);lv_obj_set_style_text_font(l,&lv_font_montserrat_14,0);lv_obj_set_style_text_color(l,lv_color_hex(enabled?PD_GOLD:PD_MUTED),0);lv_obj_set_size(l,244,18);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_pos(l,45,26);
 if(selected)lv_obj_set_style_border_color(b,lv_color_hex(PD_GOLD),0);
}
static void render_play_setup(void){
 char text[160];wifi_target_count=0;
 if(page==WEAPON_PICKER){
  unsigned n=picker_count(),pages=(n+2)/3;if(!pages)pages=1;if(picker_page>=pages)picker_page=pages-1;snprintf(text,sizeof text,"Scegli arma %u/%u",picker_page+1,pages);label(text,139,111,188,false);
  wifi_button("<",83,99,48,44,6200,picker_page>0);wifi_button(">",335,99,48,44,6201,picker_page+1<pages);
  for(unsigned row=0;row<3;row++){unsigned i=picker_at(picker_page*3+row);if(i>=PD_ITEMS)break;lv_obj_t *b=wifi_button(state.items[i].name,83,155+row*52,300,44,6210+row,true);lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,278);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);}
  if(!n)small_label("Nessun oggetto: aggiungilo dal PC",83,211,300,PD_GOLD);
  small_label("Se manca il profilo, configura dadi e bonus",83,319,300,PD_MUTED);
  wifi_button("Indietro",83,359,144,48,6099,true);wifi_button("Zaino",239,359,144,48,6202,true);
 }else if(page==HIT_SMITE){
  unsigned dice=0;bool valid=pd_smite_prepare(&state,&strike,smite_source,smite_cost,smite_pact,smite_fiend,NULL,&dice);const pd_power *p=smite_source<state.magic.count?&state.magic.powers[smite_source]:NULL;
  if(smite_rolled)snprintf(text,sizeof text,"Risultato da salvare");else snprintf(text,sizeof text,"%ud8 rad | azione bonus",dice);label(text,83,111,300,false);
  snprintf(text,sizeof text,"%s",p?p->name:"Punizione non disponibile");lv_obj_t *b=wifi_button(text,83,147,300,44,6470,!smite_rolled);lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,280);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);
  if(smite_cost)snprintf(text,sizeof text,"Slot %u %s%s >",smite_cost,smite_pact?"patto":"ordinario",p&&(p->flags&PD_LIMITED)?" + uso":"");else snprintf(text,sizeof text,"Uso innato: %u >",p?p->remaining:0);
  wifi_button(text,83,197,300,44,6471,!smite_rolled);
  wifi_button(smite_fiend?"Immondo / Non morto: si":"Immondo / Non morto: no",83,247,300,44,6472,!smite_rolled);
  int rider=pd_on_hit_spell(&state);if(rider>=0)wifi_button(smite_target?"Extra bersaglio: si":"Extra bersaglio: no",83,297,300,44,6473,!smite_rolled);
  else{snprintf(text,sizeof text,smite_rolled?"Danni %d congelati: salva per continuare":"Punizione: %ud8 | azione bonus",smite_rolled?smite_frozen.total:dice);small_label(text,83,311,300,smite_rolled?PD_GOLD:PD_TEXT);}
  wifi_button("Indietro",83,359,92,48,6099,!smite_rolled);wifi_button("Senza",187,359,92,48,6480,!smite_rolled);wifi_button(smite_rolled?"Riprova":"Usa e tira",291,359,92,48,6479,valid);
 }else if(page==HIT_EFFECTS){
  int index=pd_on_hit_spell(&state);const pd_power *p=index>=0?&state.magic.powers[index]:NULL;
  label("Bersaglio dell effetto?",83,111,300,false);setup_name(p?p->name:"Concentrazione terminata",155);
  small_label("Questo colpo e sul bersaglio",83,195,300,PD_TEXT);small_label("che hai maledetto o marchiato?",83,219,300,PD_TEXT);
  snprintf(text,sizeof text,"Extra: %ud6 %s%s",strike.critical?2:1,p&&p->effect==PD_HEX_2024?"necrotici":"forza",strike.critical?" (critico)":"");small_label(text,83,255,300,PD_GOLD);
  wifi_button("No: danni base",83,291,144,48,6460,true);wifi_button("Si: con extra",239,291,144,48,6461,p!=NULL);
  wifi_button("Indietro",83,359,300,48,6099,true);
 }else if(page==ATTACK_MODE){
  unsigned n=attack_power_count(),pages=(n+1)/2;if(!pages)pages=1;if(attack_mode_page>=pages)attack_mode_page=pages-1;
  label("Come attacchi?",139,111,188,false);wifi_button("<",83,99,48,44,6400,attack_mode_page>0);wifi_button(">",335,99,48,44,6401,attack_mode_page+1<pages);
  pd_weapon_info w;bool weapon=pd_weapon_stats(&state,state.equipped,&w);
  if(weapon)snprintf(text,sizeof text,"%s %+d | %ud%u%+d",pd_abilities[state.items[state.equipped].ability],w.attack_bonus,w.count,w.sides,w.damage_bonus);else snprintf(text,sizeof text,"Scegli prima un arma");
  attack_card("Attacco normale",text,pd_item_icon(&state,state.equipped),147,6410,weapon,attack_spell<0);
  bool supported=false;for(unsigned i=0;i<state.magic.count;i++)supported|=state.magic.available&&state.magic.powers[i].effect==PD_TRUE_STRIKE_2024;
  for(unsigned row=0;row<2;row++){unsigned i=attack_power_at(attack_mode_page*2+row);if(i>=PD_POWERS)break;const pd_power *p=&state.magic.powers[i];bool enabled=true;
   if(p->effect>=PD_HEX_2024&&(!state.hp||state.combat.raging)){snprintf(text,sizeof text,"%s",!state.hp?"Non disponibile a zero PF":"Non disponibile durante Ira");enabled=false;}
   else if(p->effect==PD_DIVINE_SMITE_2024)snprintf(text,sizeof text,"%s",p->flags&PD_PREPARED?"Dopo un colpo: usa dai Danni":"Da preparare: tocca qui");
   else if(p->effect>=PD_HEX_2024)snprintf(text,sizeof text,"%s",pd_on_hit_spell(&state)==(int)i?"Attiva: extra dopo il colpo":"Lancia prima dell attacco");
   else if(p->effect!=PD_TRUE_STRIKE_2024)snprintf(text,sizeof text,"Regole da configurare");
   else if(!state.hp){snprintf(text,sizeof text,"Non disponibile a zero PF");enabled=false;}
   else if(state.combat.raging){snprintf(text,sizeof text,"Non disponibile durante Ira");enabled=false;}
   else if(!(p->flags&PD_PREPARED))snprintf(text,sizeof text,"Da preparare: tocca qui");
   else if(pd_true_strike_stats(&state,i,attack_radiant,&w)){if(w.radiant_dice)snprintf(text,sizeof text,"%s %+d | %ud%u%+d +%ud6 rad",pd_abilities[p->ability],w.attack_bonus,w.count,w.sides,w.damage_bonus,w.radiant_dice);else snprintf(text,sizeof text,"%s %+d | %ud%u%+d",pd_abilities[p->ability],w.attack_bonus,w.count,w.sides,w.damage_bonus);}
   else{snprintf(text,sizeof text,"Verifica il profilo dell arma");enabled=false;}
   attack_card(p->name,text,pd_power_icon(p),201+row*54,6411+row,enabled,attack_spell==(int)i);
  }
  if(!n)small_label("Nessuna variante con arma configurata",83,225,300,PD_MUTED);
  wifi_button(attack_radiant?"True Strike: arma radiante":"True Strike: tipo dell arma",83,307,300,44,6420,supported);wifi_button("Indietro",83,359,300,48,6099,true);
 }else if(page==ITEM_CONFIG_MENU){
  label("Configura oggetto",83,111,300,false);setup_name(state.items[item_index].name,153);
  wifi_button("Arma: dadi e tiri",83,203,300,44,6000,true);wifi_button("Armatura e bonus",83,255,300,44,6100,state.items[item_index].kind==PD_GEAR);
  small_label("Confronta i valori con la tua scheda",83,319,300,PD_MUTED);wifi_button("Indietro",83,359,300,48,6099,true);
 }else if(page==WEAPON_SETUP){
  label("Profilo arma",83,111,300,false);setup_name(state.items[item_index].name,141);
  snprintf(text,sizeof text,"%s: %ud%u >",weapon_template==99?"Attuale":weapon_templates[weapon_template].name,weapon_draft.count,weapon_draft.sides);wifi_button(text,83,175,300,44,6002,true);
  snprintf(text,sizeof text,"Caratt. %s",pd_abilities[weapon_draft.ability]);wifi_button(text,83,227,144,44,6004,true);wifi_button(weapon_draft.flags&1?"Competenza: si":"Competenza: no",239,227,144,44,6005,true);
  snprintf(text,sizeof text,"Bonus attacco %+d | danni %+d",weapon_draft.attack_modifier,weapon_draft.damage_modifier);small_label(text,83,277,300,PD_TEXT);
  const char *names[]={"Att -","Att +","Dan -","Dan +"};for(unsigned i=0;i<4;i++)wifi_button(names[i],83+i*78,303,66,44,6006+i,true);
  wifi_button("Indietro",83,359,144,48,6099,true);wifi_button("Anteprima",239,359,144,48,6010,true);
 }else if(page==GEAR_SETUP){
  static const char *const fields[]={"Tipo di oggetto","CA base armatura/scudo","Oggetto equipaggiato","Bonus gia nei valori importati?","CA senza armatura","Bonus FOR","Bonus DES","Bonus COS","Bonus INT","Bonus SAG","Bonus CAR","Bonus CA","Bonus attacchi con arma","Bonus danni con arma","Bonus attacchi magici","Bonus CD magie","Bonus tiri salvezza","Bonus prove e abilita","Bonus PF massimi (oltre COS)","Sintonia dell oggetto"};
  snprintf(text,sizeof text,"Profilo %u/20",gear_field+1);label(text,139,111,188,false);wifi_button("<",83,99,48,44,6101,gear_field>0);wifi_button(">",335,99,48,44,6102,gear_field<19);setup_name(state.items[item_index].name,153);label(fields[gear_field],83,203,300,false);
  if(gear_field==0)snprintf(text,sizeof text,"%s",(const char *const[]){"","Oggetto indossato","Armatura leggera","Armatura media","Armatura pesante","Scudo"}[gear_draft.kind]);
  else if(gear_field==1)snprintf(text,sizeof text,"%u",gear_draft.armor);else if(gear_field==2)snprintf(text,sizeof text,"%s",gear_draft.equipped?"Si":"No");
  else if(gear_field==3)snprintf(text,sizeof text,"%s",!pd_gear_unconfigured(&state,item_index)?"Delta automatico":gear_adopt==1?"Si: sono gia inclusi":gear_adopt==2?"No: applica i bonus":"Scegli Si oppure No");
  else if(gear_field==4)snprintf(text,sizeof text,"%s",(const char *const[]){"Manuale","10 + DES","10 + DES + COS","10 + DES + SAG (senza scudo)"}[gear_rule]);
  else if(gear_field==19)snprintf(text,sizeof text,"%s",(const char *const[]){"Non richiesta","Richiesta: non sintonizzato","In sintonia"}[gear_draft.attunement]);
  else if(gear_field<=10)snprintf(text,sizeof text,"%+d",gear_draft.ability[gear_field-5]);else if(gear_field==18)snprintf(text,sizeof text,"%+d",gear_draft.max_hp);
  else{const int values[]={gear_draft.ac,gear_draft.attack,gear_draft.damage,gear_draft.spell_attack,gear_draft.spell_dc,gear_draft.saves,gear_draft.checks};snprintf(text,sizeof text,"%+d",values[gear_field-11]);}small_label(text,83,247,300,PD_GOLD);
  wifi_button("- / No",83,281,144,44,6103,true);wifi_button("+ / Si",239,281,144,44,6104,true);small_label(!gear_adopt?"Scegli se i bonus sono gia inclusi (4/20)":"Proprieta esplicite: verifica con il DM",83,331,300,PD_MUTED);
  wifi_button("Indietro",83,359,92,48,6099,true);wifi_button(gear_field<5?"Bonus >":"Base",187,359,92,48,6106,true);wifi_button("Anteprima",291,359,92,48,6105,gear_adopt>0);
 }else if(page==MAGIC_PROFILE){
  label("Regole della magia",83,111,300,false);setup_name(state.magic.powers[power_index].name,151);
  const pd_power *p=&state.magic.powers[power_index];
  wifi_button(profile_true?"True Strike 2024 >":profile_on_hit==PD_DIVINE_SMITE_2024?"Punizione 2024 >":profile_on_hit==PD_HEX_2024?"Hex 2024 >":profile_on_hit?"Marchio 2024 >":"Effetti manuali >",83,191,300,44,6301,(!p->level&&!p->maximum)||(!p->kind&&p->level==1));
  if(!p->kind&&p->level==1){small_label(profile_on_hit==PD_DIVINE_SMITE_2024?"D8 radianti dopo un colpo valido":profile_on_hit?"D6 sul bersaglio in concentrazione":"Scegli solo la regola che possiedi",83,253,300,PD_TEXT);small_label(profile_on_hit==PD_DIVINE_SMITE_2024?"Slot/uso e azione bonus richiesti":"Tempi e proprieta speciali: manuali",83,299,300,PD_GOLD);}
  else{snprintf(text,sizeof text,"Caratteristica: %s",pd_abilities[profile_ability]);wifi_button(text,83,243,300,44,6302,profile_true);
   snprintf(text,sizeof text,"Bonus att %+d | danni %+d",profile_attack,profile_damage);small_label(text,83,291,300,PD_GOLD);
   const char *names[]={"Att -","Att +","Dan -","Dan +"};for(unsigned i=0;i<4;i++)wifi_button(names[i],83+i*78,313,66,44,6303+i,profile_true);}
  wifi_button("Indietro",83,359,144,48,6099,true);wifi_button("Anteprima",239,359,144,48,6307,true);
 }else if(page==PLAY_CONFIRM){
  bool okay=play_candidate();char lines[32][96];unsigned n=okay?play_changes(lines):0,pages=n?(n+2)/3:1;if(play_preview_page>=pages)play_preview_page=pages-1;
  snprintf(text,sizeof text,"Conferma %u/%u",play_preview_page+1,pages);label(text,139,111,188,false);wifi_button("<",83,99,48,44,6096,play_preview_page>0);wifi_button(">",335,99,48,44,6097,play_preview_page+1<pages);
  for(unsigned i=0;i<3;i++){unsigned at=play_preview_page*3+i;if(at>=n)break;small_label(lines[at],83,173+i*46,300,PD_TEXT);}
  if(!okay){small_label("Valori non applicabili",83,201,300,PD_GOLD);small_label("Verifica profilo, bonus e formula CA",83,249,300,PD_TEXT);}
  small_label(play_error[0]?play_error:play_mode==4?"Sintonia: verifica i requisiti":play_mode==2&&gear_adopt==1?"Gia inclusi: registro senza raddoppiare":"Applico solo alla conferma",83,319,300,PD_GOLD);
  wifi_button("Indietro",83,359,144,48,6099,true);wifi_button("Conferma",239,359,144,48,6098,okay);
 }
 wifi_mark_focus();
}
