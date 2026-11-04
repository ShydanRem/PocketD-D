/* Final secondary views: draft-only pickers; existing commands commit state. */
static const char *const wallet_step_names[]={"1","10","100","1 mila","10 mila","100 mila","1 mln","10 mln","100 mln"};
static bool secondary_action(intptr_t action){
    if(import_pending)return false;
    if(page==RESOURCES){
        if(action==5750&&state.resource_count>1){resource_picker=true;resource_picker_page=resource_index/6;return true;}
        if(resource_picker){
            if(action==100||action==2300){resource_picker=false;return false;}
            if(action==5751)resource_picker=false;
            else if(action==5752&&resource_picker_page)resource_picker_page--;
            else if(action==5753&&(resource_picker_page+1)*6<state.resource_count)resource_picker_page++;
            else if(action>=5754&&action<5754+state.resource_count){resource_index=action-5754;resource_picker=false;}
            return true;
        }
    }
    if(page!=WALLET_EDIT||!wallet_panel)return false;
    if(wallet_panel==1){
        if(action==2429)wallet_panel=0;
        else if(action>=2420&&action<2429){wallet_step=action-2420;wallet_panel=0;}
    }else if(wallet_panel==2){
        if(action>=2440&&action<2450){unsigned digit=action-2440;if(wallet_number_fresh||wallet_number<=(PD_COIN_MAX-digit)/10){wallet_number=wallet_number_fresh?digit:wallet_number*10+digit;wallet_number_fresh=false;}}
        else if(action==2450){wallet_number/=10;wallet_number_fresh=false;}
        else if(action==2451){wallet_number=0;wallet_number_fresh=false;}
        else if(action==2452)wallet_panel=0;
        else if(action==2453){wallet_draft.coins[wallet_coin]=wallet_number;wallet_panel=0;}
    }
    return true;
}
static const lv_image_dsc_t *shortcut_icon(unsigned id){
    const pd_ui_icon_id icons[]={PD_UI_KEYS,PD_UI_HOME,PD_UI_SHEET,PD_UI_DICE,PD_UI_PACK,PD_UI_MAGIC,PD_UI_DICE,PD_UI_REST,PD_UI_HEART,PD_UI_GEAR,PD_UI_USES,PD_UI_TIME,PD_UI_ATTACK,PD_UI_HISTORY,PD_UI_ACCOUNT};
    return pd_ui_icon(icons[id<15?id:0]);
}
static void render_key_controls(void){
    char text[100];const char *gestures[]={"PWR breve","BOOT breve","BOOT lungo"};
    if(page==KEY_PICKER){
        label(gestures[key_gesture],83,111,300,true);snprintf(text,sizeof text,"Assegna comando | %u/3",key_page+1);connection_note(text,141,20,PD_MUTED);
        for(unsigned row=0;row<6;row++){
            unsigned i=key_page*6+row;if(i>=15)break;
            lv_obj_t *b=scene_control(shortcut_names[i],shortcut_icon(i),shortcut_names[i],i==state.shortcuts[key_gesture]?"Attuale":"Assegna",83+(row%2)*156,167+(row/2)*56,144,48,4810+i,true);rules_selected_style(b,i==state.shortcuts[key_gesture]);
        }
        connection_note("Tocca per salvare; Indietro annulla",335,20,PD_MUTED);
        icon_button("Pagina precedente",PD_UI_BACK,83,359,60,48,4800,key_page>0);button("Indietro",155,359,156,48,4802,true);icon_button("Pagina successiva",PD_UI_NEXT,323,359,60,48,4801,key_page<2);
    }else{
        label("Tasti configurabili",83,111,300,true);
        for(unsigned i=0;i<3;i++){snprintf(text,sizeof text,"%s: %s",gestures[i],shortcut_names[state.shortcuts[i]]);scene_control(text,shortcut_icon(state.shortcuts[i]),gestures[i],shortcut_names[state.shortcuts[i]],83,159+i*54,300,48,860+i,true);}
        connection_note(input_check_notice,325,32,PD_GOLD);button("Indietro",83,359,144,48,863,true);scene_control("Collaudo",pd_ui_icon(PD_UI_TOUCH),"Collaudo","Tocco e tasti",239,359,144,48,2900,true);
    }
}
static void render_resource_picker(void){
    unsigned pages=(state.resource_count+5)/6;if(!pages)pages=1;if(resource_picker_page>=pages)resource_picker_page=pages-1;
    char text[80];snprintf(text,sizeof text,"Contatori %u/%u",resource_picker_page+1,pages);label(text,139,111,188,false);
    button("<",83,99,48,44,5752,resource_picker_page>0);button(">",335,99,48,44,5753,resource_picker_page+1<pages);
    for(unsigned row=0;row<6;row++){
        unsigned i=resource_picker_page*6+row;if(i>=state.resource_count)break;const pd_resource *r=&state.resources[i];snprintf(text,sizeof text,"%u | %u/%u",i+1,r->current,r->maximum);
        lv_obj_t *b=scene_control(r->name,pd_ui_icon(PD_UI_USES),r->name,text,83+(row%2)*156,159+(row/2)*56,144,48,5754+i,true);rules_selected_style(b,i==resource_index);
    }
    connection_note("Scegli il contatore; nessun utilizzo speso",335,20,PD_MUTED);button("Indietro",83,359,300,48,5751,true);
}
static void render_wallet(void){
    char text[160];
    if(page==WALLET_HOME){
        snprintf(text,sizeof text,"Borsello %u/2",wallet_page+1);label(text,139,111,188,false);button("<",83,99,48,44,2401,state.wallet.available&&wallet_page>0);button(">",335,99,48,44,2402,state.wallet.available&&wallet_page<1);
        if(!state.wallet.available){
            connection_banner(PD_UI_GOLD,"Saldo non disponibile","Da impostare, senza inventare valori",PD_GOLD);
            scene_control("Configura saldo",pd_ui_icon(PD_UI_GOLD),"Imposta saldo","Bozza prima del salvataggio",83,247,300,64,2403,true);connection_note("Correzione manuale dei cinque tagli",335,20,PD_MUTED);
        }else{
            for(unsigned row=0;row<3;row++){unsigned i=wallet_page*3+row;if(i>=PD_COINS)break;char badge[40];snprintf(text,sizeof text,"%s: %u %s",coin_names[i],(unsigned)state.wallet.coins[i],coin_units[i]);snprintf(badge,sizeof badge,"%u %s",(unsigned)state.wallet.coins[i],coin_units[i]);scene_control(text,pd_ui_coin_icon(i),coin_names[i],badge,83,155+52*row,300,44,2500+i,true);}
            snprintf(text,sizeof text,"Equiv. rame: %llu MR",(unsigned long long)pd_wallet_copper(&state.wallet));connection_note(text,311,20,PD_GOLD);connection_note("Nessun cambio automatico",335,20,PD_MUTED);
        }
        lv_obj_t *undo=button("Annulla monete",83,359,144,48,2414,wallet_can_undo());lv_obj_set_style_text_font(undo,&lv_font_montserrat_16,0);button("Zaino",239,359,144,48,80,true);return;
    }
    if(page==WALLET_EDIT){
        if(wallet_panel==1){
            label("Scegli il passo",83,111,300,true);
            for(unsigned i=0;i<9;i++){lv_obj_t *b=button(wallet_step_names[i],83+(i%3)*104,163+(i/3)*56,92,48,2420+i,true);lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);rules_selected_style(b,i==wallet_step);}
            connection_note("Cambia solo il passo di + e -",335,20,PD_MUTED);button("Indietro",83,359,300,48,2429,true);return;
        }
        if(wallet_panel==2){
            snprintf(text,sizeof text,"Saldo %s",coin_names[wallet_coin]);label(text,83,111,300,true);snprintf(text,sizeof text,"%u | massimo %u",(unsigned)wallet_number,(unsigned)PD_COIN_MAX);small_label(text,83,145,300,wallet_number_fresh?PD_GOLD:PD_TEXT);
            const char *keys[]={"1","2","3","4","5","6","7","8","9","Azzera","0","Canc."};const unsigned actions[]={2441,2442,2443,2444,2445,2446,2447,2448,2449,2451,2440,2450};
            for(unsigned i=0;i<12;i++){bool enabled=i==9||i==11?(i==9||wallet_number>0):wallet_number_fresh||wallet_number<=(PD_COIN_MAX-(actions[i]-2440))/10;lv_obj_t *b=button(keys[i],83+(i%3)*104,167+(i/3)*48,92,44,actions[i],enabled);if(i==9||i==11)lv_obj_set_style_text_font(b,&lv_font_montserrat_16,0);}
            button("Annulla",83,359,144,48,2452,true);button("Usa numero",239,359,144,48,2453,true);return;
        }
        snprintf(text,sizeof text,"%s (%s)",coin_names[wallet_coin],coin_units[wallet_coin]);label(text,139,111,188,false);button("<",83,99,48,44,2404,wallet_coin>0);button(">",335,99,48,44,2405,wallet_coin+1<PD_COINS);
        snprintf(text,sizeof text,"%u",(unsigned)wallet_draft.coins[wallet_coin]);lv_obj_t *amount=scene_control("Inserisci saldo",pd_ui_coin_icon(wallet_coin),"Saldo",text,83,151,300,64,2415,true);lv_obj_t *value=lv_obj_get_child(amount,3);lv_obj_set_style_text_font(value,&pd_pixel_4,0);lv_obj_set_y(value,29);lv_obj_set_height(value,pd_pixel_4.line_height);
        if(wallet_origin.available)snprintf(text,sizeof text,"Prima: %u %s",(unsigned)wallet_origin.coins[wallet_coin],coin_units[wallet_coin]);else snprintf(text,sizeof text,"Prima: saldo non disponibile");connection_note(text,221,20,PD_MUTED);
        snprintf(text,sizeof text,"Passo: %u >",(unsigned)wallet_steps[wallet_step]);scene_control(text,pd_ui_coin_icon(wallet_coin),"Passo",wallet_step_names[wallet_step],83,247,300,44,2408,true);
        button("-",83,299,144,44,2406,wallet_draft.coins[wallet_coin]>=wallet_steps[wallet_step]);button("+",239,299,144,44,2407,wallet_draft.coins[wallet_coin]<=PD_COIN_MAX-wallet_steps[wallet_step]);button("Annulla",83,359,144,48,2409,true);button("Anteprima",239,359,144,48,2410,true);return;
    }
    label("Conferma borsello",83,111,300,true);
    for(unsigned i=0;i<PD_COINS;i++){
        lv_obj_t *box=decoration(83,153+i*36,300,32,0x122b38,2);lv_obj_set_style_pad_all(box,0,0);art(pd_ui_coin_icon(i),5,4,box);tile_label(box,coin_units[i],35,8,30,&lv_font_montserrat_14,PD_MUTED);
        if(wallet_origin.available)snprintf(text,sizeof text,"%u",(unsigned)wallet_origin.coins[i]);else strcpy(text,"?");tile_label(box,text,69,6,90,&lv_font_montserrat_16,PD_MUTED);art(pd_ui_icon(PD_UI_NEXT),166,4,box);
        snprintf(text,sizeof text,"%u",(unsigned)wallet_draft.coins[i]);unsigned color=!wallet_origin.available||wallet_origin.coins[i]!=wallet_draft.coins[i]?wallet_origin.available&&wallet_draft.coins[i]<wallet_origin.coins[i]?PD_DANGER:PD_GOLD:PD_TEXT;tile_label(box,text,197,6,98,&lv_font_montserrat_16,color);
    }
    connection_note(wallet_origin.available?"Salva tutti i tagli insieme":"Crea un saldo locale verificato",335,20,PD_MUTED);button("Indietro",83,359,144,48,2412,true);button("Conferma",239,359,144,48,2411,!wallet_same(&wallet_draft,&state.wallet)&&wallet_same(&wallet_origin,&state.wallet));
}
