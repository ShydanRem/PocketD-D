/* Presentation of fixed outcomes. Rule/RNG/storage decisions stay in act(). */
static lv_obj_t *dice_command(const char *name,int x,int width,int action,bool enabled,bool primary){
    lv_obj_t *b=button(name,x,335,width,48,action,enabled);
    lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);
    if(primary)pd_theme_button(b,true);
    return b;
}
static void render_dice_game(void){
    lv_obj_t *root=lv_screen_active();char source[160],verdict[120],value[32];
    pd_check_resolution check_gate={0};pd_attack_resolution attack_gate={0};
    bool check_known=ability>=0&&pd_check_resolve(&state,check_kind,ability,mode,ignore_effects,roll_context,&check_gate);
    pd_attack_resolve(&state,mode,roll_context,ignore_effects,&attack_gate);
    pd_weapon_info available_weapon;
    bool allowed=combat_roll==1?attack_gate.allowed&&attack_weapon_info(&available_weapon):combat_roll>1||pool_view||ability<0||(check_known&&check_gate.allowed);
    int effective=combat_roll==1?attack_gate.mode:pool_view?0:ability<0?mode:check_gate.mode;
    if(!pool_view&&!has_roll&&!dice_animation.active)pd_dice_idle(&dice_animation,effective?2:1);
    if(!pd_dice_count(&dice_animation))pd_dice_idle(&dice_animation,mode?2:1);
    bool automatic_failure=!combat_roll&&!pool_view&&check_known&&check_gate.automatic_failure;
    if(!automatic_failure)dice_pixels=lv_malloc(PD_DICE_PIXELS*sizeof *dice_pixels);
    if(dice_pixels){
        dice_canvas=lv_canvas_create(root);lv_obj_set_pos(dice_canvas,73,199);
        lv_canvas_set_buffer(dice_canvas,dice_pixels,PD_DICE_WIDTH,PD_DICE_HEIGHT,LV_COLOR_FORMAT_RGB565);
        lv_obj_add_flag(dice_canvas,LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(dice_canvas,event,LV_EVENT_CLICKED,(void*)(intptr_t)16);
        pd_dice_paint(&dice_animation,dice_pixels);
        if(dice_animation.active)dice_timer=lv_timer_create(dice_frame,33,NULL);
    }else if(dice_animation.active){pd_dice_finish(&dice_animation);has_roll=true;}

    unsigned color=PD_TEXT;int total=pool_view?dice_animation.pool.total:result.total;
    bool mixed=combat_roll==2&&(strike.weapon.true_strike||damage_parts.extra.count||damage_parts.smite.count);
    const char *part=damage_view_smite_overflow?"Punizione 2":damage_view_smite?"Punizione":damage_view_extra?(damage_parts.extra_effect==PD_HEX_2024?"Necrotici":"Forza"):damage_view_radiant?"Radianti":"Arma";
    if(combat_roll==1){
        pd_weapon_info w={0};attack_weapon_info(&w);
        snprintf(source,sizeof source,"Attacco %+d | CA %u",strike_ready?strike.roll.bonus:w.attack_bonus,strike_ready?strike.armor:target_armor);
    }else if(combat_roll==2){
        if(mixed&&has_roll)snprintf(source,sizeof source,"%s %d | %ud%u%+d",part,dice_animation.pool.total,dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else if(mixed)snprintf(source,sizeof source,"%s | %ud%u%+d",part,dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        else snprintf(source,sizeof source,"Danni %ud%u%+d",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);
        total=mixed?damage_parts.total:dice_animation.pool.total;
    }else if(combat_roll==3||combat_roll==4){if(has_roll)snprintf(source,sizeof source,"Cura %ud%u%+d | PF %u/%u",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus,state.hp,pd_max_hp(&state));else snprintf(source,sizeof source,"Cura %ud%u%+d",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);}
    else if(combat_roll==7){if(has_roll)snprintf(source,sizeof source,"Ricarica %ud%u%+d | %u/%u",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus,recharge_after,recharge_maximum);else snprintf(source,sizeof source,"Ricarica %ud%u%+d",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus);}
    else if(combat_roll==8)snprintf(source,sizeof source,"Iniziativa %+d",result.bonus);
    else if(combat_roll==6)snprintf(source,sizeof source,"Concentrazione | CD %u",incoming_result.concentration_dc);
    else if(combat_roll==5)snprintf(source,sizeof source,"Salvezza morte %+d",result.bonus);
    else if(pool_view)snprintf(source,sizeof source,"%ud%u%+d%s",dice_animation.pool.count,dice_animation.pool.sides,dice_animation.pool.bonus,dice_animation.pool.damage?" | danni":"");
    else if(ability<0)snprintf(source,sizeof source,"d20 libero");else check_name(source,sizeof source,true);

    if(has_roll){
        snprintf(value,sizeof value,"%d",total);
        if(combat_roll==1){snprintf(verdict,sizeof verdict,"%s",strike.critical?"CRITICO!":strike.hits?"COLPITO":"MANCATO");color=strike.critical?PD_GOLD:strike.hits?PD_MINT:PD_DANGER;}
        else if(combat_roll==2){snprintf(verdict,sizeof verdict,"%s",mixed?"Danni totali":"Danni");color=PD_GOLD;}
        else if(combat_roll==3||combat_roll==4){snprintf(value,sizeof value,"+%u",healing_gained);snprintf(verdict,sizeof verdict,"PF recuperati");color=PD_MINT;}
        else if(combat_roll==7){snprintf(value,sizeof value,"+%u",recharge_gained);snprintf(verdict,sizeof verdict,"Cariche recuperate");color=PD_VIOLET;}
        else if(combat_roll==5){snprintf(verdict,sizeof verdict,"%s",(const char *const[]){result.natural==1?"1 naturale: due fallite":"Fallita","Riuscita","Stabilizzato","Tre fallimenti","20 naturale: riprendi 1 PF"}[status_outcome]);color=status_outcome==0||status_outcome==3?PD_DANGER:PD_MINT;}
        else if(combat_roll==6){snprintf(verdict,sizeof verdict,"Concentrazione %s",incoming_result.ends_concentration?"persa":"mantenuta");color=incoming_result.ends_concentration?PD_DANGER:PD_MINT;}
        else if(combat_roll==8)snprintf(verdict,sizeof verdict,"Iniziativa");
        else if(pool_view){snprintf(verdict,sizeof verdict,"%s",dice_animation.pool.damage?"Danni":"Totale");if(dice_animation.pool.damage&&dice_animation.pool.sum+dice_animation.pool.bonus<0)snprintf(source,sizeof source,"Somma %d | minimo 0",dice_animation.pool.sum+dice_animation.pool.bonus);}
        else {
            snprintf(verdict,sizeof verdict,"%s",result.natural==20?"20 naturale":result.natural==1?"1 naturale":"Totale");color=result.natural==20?PD_GOLD:result.natural==1?PD_DANGER:PD_TEXT;
            const char *kind=ability<0?"d20 libero":check_kind==PD_SAVE?pd_abilities[ability]:check_kind==PD_SKILL?pd_skill_names[ability]:pd_abilities[ability];
            const char *prefix=ability<0||check_kind==PD_SKILL?"":check_kind==PD_SAVE?"TS ":"Prova ";
            if(result.count==2){snprintf(source,sizeof source,"%s%s | %u/%u%+d",prefix,kind,result.dice[0],result.dice[1],result.bonus);snprintf(verdict,sizeof verdict,"%s | %s",result.natural==20?"20 naturale":result.natural==1?"1 naturale":"Totale",effective>0?"vantaggio":"svantaggio");}
            else snprintf(source,sizeof source,"%s%s | %u%+d",prefix,kind,result.natural,result.bonus);
        }
    }else{
        snprintf(value,sizeof value,"--");
        snprintf(verdict,sizeof verdict,"%s",!allowed?(automatic_failure?"Fallimento automatico":check_gate.needed||(combat_roll==1&&attack_gate.needed)?"Dichiara Contesto":"Da verificare: apri i dettagli"):dice_animation.active?"In volo...":effective>0?"Vantaggio":effective<0?"Svantaggio":"Pronto al lancio");
        color=allowed?PD_MUTED:PD_DANGER;
    }
    lv_obj_t *detail=NULL;
    if(!combat_roll&&!pool_view&&ability>=0){
        char check[80];check_name(check,sizeof check,true);detail=button(check,83,96,300,100,70,!dice_animation.active);
        lv_obj_add_flag(lv_obj_get_child(detail,0),LV_OBJ_FLAG_HIDDEN);lv_obj_set_width(lv_obj_get_child(detail,0),280);lv_label_set_long_mode(lv_obj_get_child(detail,0),LV_LABEL_LONG_DOT);
        lv_obj_set_style_bg_opa(detail,LV_OPA_TRANSP,0);lv_obj_set_style_border_width(detail,0,0);lv_obj_set_style_shadow_opa(detail,0,0);lv_obj_set_style_pad_all(detail,0,0);pd_ui_name(detail,"Dettagli della prova");
    }
    lv_obj_t *caption=small_label(source,83,100,300,PD_MUTED);
    lv_obj_t *number=label(value,83,124,300,true);label_font(number,&pd_pixel_6);lv_obj_set_style_text_color(number,lv_color_hex(color),0);
    lv_obj_t *meaning=small_label(verdict,83,177,300,color);if(combat_roll==1&&has_roll)label_font(meaning,&lv_font_montserrat_20);
    if(detail){lv_obj_set_parent(caption,detail);lv_obj_set_pos(caption,0,4);lv_obj_set_parent(number,detail);lv_obj_set_pos(number,0,28);lv_obj_set_parent(meaning,detail);lv_obj_set_pos(meaning,0,81);}
    if(has_roll&&((combat_roll==1&&strike.critical)||(!combat_roll&&!pool_view&&result.natural==20))){
        for(unsigned i=0;i<2;i++){int x=i?359:99;decoration(x,139,12,4,PD_GOLD,0);decoration(x+4,135,4,12,PD_GOLD,0);}
    }
    if(automatic_failure)small_label("Nessun dado",83,254,300,PD_MUTED);
    else if(!dice_pixels)small_label("Animazione non disponibile",83,254,300,PD_MUTED);

    if(combat_roll==1){
        if(!dice_animation.active&&!(has_roll&&strike_ready&&strike.hits&&!damage_used)){
            dice_command("Arma",83,144,85,true,false);dice_command("Nuovo",239,144,81,true,true);
        }else{
            icon_button("Arma",PD_UI_ATTACK,83,335,52,48,85,!dice_animation.active);
            dice_command(dice_animation.active?"Accelera":"Nuovo",143,92,dice_animation.active?16:81,true,false);
            dice_command("Danni",243,140,90,!dice_animation.active&&has_roll&&strike_ready&&strike.hits&&!damage_used,true);
        }
    }else if(combat_roll==2&&mixed){
        icon_button("Arma",PD_UI_ATTACK,83,335,52,48,85,!dice_animation.active);
        const char *next=damage_view_smite_overflow?"Dadi arma":damage_view_smite?(damage_parts.smite_overflow.count?"Altri d8":"Dadi arma"):damage_view_extra?(damage_parts.smite.count?"Punizione":"Dadi arma"):damage_view_radiant?(damage_parts.extra.count?"Extra":damage_parts.smite.count?"Punizione":"Dadi arma"):damage_parts.radiant.count?"Radianti":damage_parts.extra.count?"Extra":"Punizione";
        dice_command(dice_animation.active?"Accelera":next,143,164,dice_animation.active?16:6422,dice_animation.active||damage_parts.radiant.count||damage_parts.extra.count||damage_parts.smite.count,true);
        dice_command("Fine",315,68,100,!dice_animation.active,false);
    }else if(combat_roll==8){
        dice_command("Profilo",83,144,4600,!dice_animation.active,false);dice_command(dice_animation.active?"Accelera":"Fine",239,144,dice_animation.active?16:100,true,true);
    }else if(combat_roll==6||combat_roll==7){
        bool recharge=combat_roll==7;
        icon_button(recharge?"Zaino":"Stato",recharge?PD_UI_PACK:PD_UI_HEART,83,335,52,48,recharge?80:1500,!dice_animation.active);
        dice_command(dice_animation.active?"Accelera":"Riepilogo",143,164,dice_animation.active?16:recharge?2180:1809,true,true);dice_command("Fine",315,68,100,!dice_animation.active,false);
    }else if(combat_roll){
        const char *back=combat_roll==5?"Stato":combat_roll==4?"Riposo":combat_roll==3?"Zaino":"Arma";
        int action=combat_roll==5?1501:combat_roll==4?911:combat_roll==3?80:85;
        icon_button(back,combat_roll==5?PD_UI_HEART:combat_roll==4?PD_UI_REST:combat_roll==3?PD_UI_PACK:PD_UI_ATTACK,83,335,52,48,action,!dice_animation.active);
        dice_command(dice_animation.active?"Accelera":combat_roll>=4?"Altro":"Nuovo",143,164,dice_animation.active?16:combat_roll==5?1501:combat_roll==4?910:combat_roll==3?80:81,true,true);
        dice_command("Fine",315,68,combat_roll==4?918:100,!dice_animation.active,false);
    }else{
        dice_command(pool_view?"Prova":mode==-1?"Svant":mode==1?"Vant":"Base",83,60,pool_view?14:15,!dice_animation.active,false);
        dice_command(dice_animation.active?"Accelera":has_roll?"Rilancia":"Lancia",151,168,13,allowed,true);
        bool context=state.roll_conditions&&!pool_view&&ability>=0;
        lv_obj_t *more=icon_button(context?"Contesto":"Altri",context?PD_UI_SHEET:PD_UI_MENU,327,335,56,48,context?2230:40,!dice_animation.active);(void)more;
    }
    const char *names[]={"Eroe","Scheda","Storico","Usi"};const lv_image_dsc_t *icons[]={&theme_hero,&theme_sheet,&theme_notes,&theme_crystal};const int actions[]={100,101,2600,103};
    for(unsigned i=0;i<4;i++){lv_obj_t *b=button("",133+52*i,394,44,44,actions[i],!dice_animation.active);pd_ui_name(b,names[i]);art(icons[i],6,6,b);}
}
