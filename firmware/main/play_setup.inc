/* Device-local equipment/spell setup. No rule is granted by an icon or name. */
static lv_obj_t *wifi_button(const char *,int,int,int,int,int,bool);
static void wifi_mark_focus(void);
static bool play_editing(void){return page>=WEAPON_PICKER&&page<=ATTACK_MODE;}
static unsigned picker_page,weapon_template,gear_field,play_preview_page,play_mode;
static pd_item weapon_draft;
static pd_gear gear_draft;
static unsigned gear_rule,gear_adopt;
static bool weapon_from_attack,profile_true;
static uint8_t profile_ability;
static int profile_attack,profile_damage;
static page_t play_return;
static const char *play_error="";
static int attack_spell=-1;
static bool attack_radiant;
static int pending_attack_spell=-1;
static page_t picker_return=HERO;
static unsigned attack_mode_page;
static bool weapon_spell_candidate(const pd_power *p){return p->effect==PD_TRUE_STRIKE_2024||((strstr(p->name,"True Strike")||strstr(p->name,"Colpo Accurato"))&&(strstr(p->description,"spellcasting ability")||strstr(p->description,"caratteristica da incantatore")));}
static unsigned attack_power_at(unsigned at){for(unsigned i=0;i<state.magic.count;i++)if(weapon_spell_candidate(&state.magic.powers[i])){if(!at)return i;at--;}return PD_POWERS;}
static unsigned attack_power_count(void){unsigned n=0;for(unsigned i=0;i<state.magic.count;i++)if(weapon_spell_candidate(&state.magic.powers[i]))n++;return n;}
static bool attack_weapon_info(pd_weapon_info *out){return attack_spell>=0?pd_true_strike_stats(&state,(unsigned)attack_spell,attack_radiant,out):pd_weapon_stats(&state,state.equipped,out);}

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
 const pd_power *p=&state.magic.powers[power_index];profile_true=p->effect==PD_TRUE_STRIKE_2024;profile_ability=p->ability;profile_attack=profile_true?p->attack_modifier:0;profile_damage=profile_true?p->save_modifier:0;
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
 if(profile_true){if(p->level||p->maximum||(p->flags&PD_LIMITED)||scratch.magic.concentration==power_index+1)return false;p->kind=0;p->effect=PD_TRUE_STRIKE_2024;p->level=0;p->flags=(p->flags&PD_EXCERPT)|PD_PREPARED;p->maximum=p->remaining=p->recovery=0;p->ability=profile_ability;p->attack_modifier=profile_attack;p->save_modifier=profile_damage;}
 else if(p->effect==PD_TRUE_STRIKE_2024){p->effect=0;p->attack_modifier=p->save_modifier=0;}
 return pd_valid(&scratch);
}
static void play_preview(unsigned mode,page_t back){play_mode=mode;play_return=back;play_preview_page=0;page=PLAY_CONFIRM;wifi_focus=0;}
static unsigned play_changes(char lines[32][96]){
 unsigned n=0;
 if(play_mode==4)snprintf(lines[n++],96,"%s",scratch.gear[item_index].attunement==2?"Attiva sintonia":"Termina sintonia");
 else if(play_mode==3)snprintf(lines[n++],96,"Profilo: %s",profile_true?"True Strike 2024":"effetti manuali");
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
 return n;
}
static bool play_action(intptr_t a){
 play_error="";
 if(a==6099){if(page==WEAPON_PICKER){page=picker_return;pending_attack_spell=-1;}else if(page==PLAY_CONFIRM)page=play_return;else if(page==MAGIC_PROFILE)page=MAGIC_DETAIL;else if(page==WEAPON_SETUP)page=weapon_from_attack?WEAPON_PICKER:ITEM_CONFIG_MENU;else if(page==GEAR_SETUP)page=ITEM_CONFIG_MENU;else if(page==ATTACK_MODE)page=COMBAT;else page=ITEM_DETAIL;wifi_focus=0;return true;}
 if(a==6500&&page==ITEM_DETAIL&&state.gear[item_index].attunement){play_preview(4,ITEM_DETAIL);return true;}
 if(a==6090&&page==ITEM_DETAIL){page=ITEM_CONFIG_MENU;return true;}
 if(a==84&&page==ITEM_DETAIL){if(state.items[item_index].kind==PD_GEAR&&pd_gear_unconfigured(&state,item_index)&&pd_item_setup_hint(&state,item_index)){page=ITEM_CONFIG_MENU;return true;}play_preview(0,ITEM_DETAIL);return true;}
 if(a==6402&&page==COMBAT){attack_mode_page=0;page=ATTACK_MODE;return true;}
 if(a==6403&&page==COMBAT){picker_page=0;weapon_from_attack=true;pending_attack_spell=attack_spell;picker_return=COMBAT;page=WEAPON_PICKER;return true;}
 if(page==ATTACK_MODE){
  if(a==6400&&attack_mode_page)attack_mode_page--;else if(a==6401&&(attack_mode_page+1)*2<attack_power_count())attack_mode_page++;
  else if(a==6410){attack_spell=-1;page=COMBAT;}
  else if(a==6420)attack_radiant=!attack_radiant;
  else if(a==6411||a==6412){unsigned i=attack_power_at(attack_mode_page*2+a-6411);if(i<PD_POWERS){power_index=i;if(state.magic.powers[i].effect!=PD_TRUE_STRIKE_2024)profile_setup_begin();else if(!(state.magic.powers[i].flags&PD_PREPARED))page=MAGIC_DETAIL;else{attack_spell=i;page=COMBAT;}}}
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
  if(a==6301&&!state.magic.powers[power_index].level&&!state.magic.powers[power_index].maximum)profile_true=!profile_true;
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
    if(play_mode==3)page=MAGIC_DETAIL;else if((play_mode==1||play_return==WEAPON_PICKER)&&weapon_from_attack){page=COMBAT;combat_roll=1;pool_view=false;has_roll=false;mode=0;attack_spell=pending_attack_spell;pending_attack_spell=-1;strike_ready=false;damage_used=false;roll_context=(pd_roll_context){0};ignore_effects=false;}
    else page=ITEM_DETAIL;
   }else play_error="Salvataggio non confermato: riprova";
  }return true;
 }
 return false;
}
static void setup_name(const char *name,int y){lv_obj_t *l=small_label(name,83,y,300,PD_GOLD);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_set_height(l,24);}
static void render_play_setup(void){
 char text[160];wifi_target_count=0;
 if(page==WEAPON_PICKER){
  unsigned n=picker_count(),pages=(n+2)/3;if(!pages)pages=1;if(picker_page>=pages)picker_page=pages-1;snprintf(text,sizeof text,"Scegli arma %u/%u",picker_page+1,pages);label(text,139,111,188,false);
  wifi_button("<",83,99,48,44,6200,picker_page>0);wifi_button(">",335,99,48,44,6201,picker_page+1<pages);
  for(unsigned row=0;row<3;row++){unsigned i=picker_at(picker_page*3+row);if(i>=PD_ITEMS)break;lv_obj_t *b=wifi_button(state.items[i].name,83,155+row*52,300,44,6210+row,true);lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,278);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);}
  if(!n)small_label("Nessun oggetto: aggiungilo dal PC",83,211,300,PD_GOLD);
  small_label("Se manca il profilo, configura dadi e bonus",83,319,300,PD_MUTED);
  wifi_button("Indietro",83,359,144,48,6099,true);wifi_button("Zaino",239,359,144,48,6202,true);
 }else if(page==ATTACK_MODE){
  unsigned n=attack_power_count(),pages=(n+1)/2;if(!pages)pages=1;if(attack_mode_page>=pages)attack_mode_page=pages-1;
  label("Tipo di attacco",139,111,188,false);wifi_button("<",83,99,48,44,6400,attack_mode_page>0);wifi_button(">",335,99,48,44,6401,attack_mode_page+1<pages);
  wifi_button("Attacco normale",83,151,300,44,6410,true);
  for(unsigned row=0;row<2;row++){unsigned i=attack_power_at(attack_mode_page*2+row);if(i>=PD_POWERS)break;snprintf(text,sizeof text,"%s%s",state.magic.powers[i].name,state.magic.powers[i].effect==PD_TRUE_STRIKE_2024?"":" · configura");lv_obj_t *b=wifi_button(text,83,203+row*52,300,44,6411+row,true);lv_obj_t *l=lv_obj_get_child(b,0);lv_obj_set_width(l,278);lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);lv_obj_center(l);}
  if(!n)small_label("Nessun trucco con arma configurato",83,227,300,PD_MUTED);
  wifi_button(attack_radiant?"Danno arma: radiante":"Danno arma: tipo normale",83,307,300,44,6420,n>0);wifi_button("Indietro",83,359,300,48,6099,true);
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
  wifi_button(profile_true?"True Strike 2024 >":"Effetti manuali >",83,191,300,44,6301,!state.magic.powers[power_index].level&&!state.magic.powers[power_index].maximum);
  snprintf(text,sizeof text,"Caratteristica: %s",pd_abilities[profile_ability]);wifi_button(text,83,243,300,44,6302,profile_true);
  snprintf(text,sizeof text,"Bonus att %+d | danni %+d",profile_attack,profile_damage);small_label(text,83,291,300,PD_GOLD);
  const char *names[]={"Att -","Att +","Dan -","Dan +"};for(unsigned i=0;i<4;i++)wifi_button(names[i],83+i*78,313,66,44,6303+i,profile_true);
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
