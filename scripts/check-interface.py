"""v0.11 browser navigation, avatar and circular-layout verification after npm run build."""
import json
import subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=Path('/tmp/pocketdd-v011-qa');OUTPUT.mkdir(exist_ok=True)
KEY='pocketdd-library-v1'
fixture=json.loads(subprocess.check_output(['node','--input-type=module','-e',"""
import {initialState,recordRoll} from './src/model.js';
import {upsertItem} from './src/inventory.js';
import {upsertPower} from './src/powers.js';
import {upsertParticipant,startEncounter} from './src/encounter.js';
import {placeToken} from './src/grid.js';
import {createLibrary,addCharacter,selectCharacter,serializeLibrary} from './src/library.js';
let s={...initialState(),name:'Mira',title:'Custode dei sentieri',condition:true};
for(let i=0;i<12;i++)s=upsertItem(s,{id:`gear-${i}`,type:'gear',name:`Oggetto ${i}`,notes:'Note del viaggio, da consultare al tavolo.',quantity:1});
for(let i=0;i<8;i++)s=upsertPower(s,{id:`spell-${i}`,type:'spell',name:`Scintilla ${i}`,description:'Note personali per questa magia. Effetti e requisiti concordati con il DM.',level:0,concentration:false});
for(let i=0;i<20;i++)s=recordRoll(s,'Prova di Furtività',{dice:[10],bonus:3,total:13},'d20','normal',1000+i);
s=upsertParticipant(s,{id:'hero',initiative:12});s=upsertParticipant(s,{id:'enemy',name:'Guardiano',initiative:1,hp:20,maxHp:20,armorClass:12,side:'enemy'});
s=startEncounter(placeToken(placeToken(s,'hero',40),'enemy',44));
let library=createLibrary(s,'mira');library=addCharacter(library,{...initialState(),name:'Nora',equippedWeaponId:null},'nora');
console.log(serializeLibrary(selectCharacter(library,'mira')));
"""],cwd=ROOT,text=True))

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    page=browser.new_page(viewport={'width':1440,'height':1050},accept_downloads=True)
    errors=[];layout_failures=[];page.on('pageerror',lambda error:errors.append(str(error)))
    page.on('dialog',lambda dialog:dialog.accept())
    page.route('**/*',lambda route:route.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if route.request.is_navigation_request() else route.abort())
    page.goto('http://pocket.test');page.evaluate('(raw)=>localStorage.setItem("pocketdd-library-v1",raw)',json.dumps(fixture));page.reload()
    def raw():return page.evaluate('(key)=>localStorage.getItem(key)',KEY)
    def state():
        data=json.loads(raw());return next(item['state'] for item in data['characters'] if item['id']==data['activeId'])
    def go(id):page.locator(f'[data-go-page="{id}"]').click()
    def current():return page.locator('.screen nav [aria-current="page"]').get_attribute('data-page')
    def assert_clear(selectors):
        nav=page.locator('.screen nav').bounding_box()
        for selector in selectors:
            for element in page.locator(selector).all():
                box=element.bounding_box()
                if box and box['y']+box['height']>nav['y']-2:layout_failures.append(('menu overlap',page.viewport_size,current(),selector,box,nav))
    def assert_nav_hit_targets():
        for button in page.locator('.screen nav button').all():
            assert button.evaluate('''button=>{const r=button.getBoundingClientRect();return [[r.x+r.width/2,r.y+r.height/2],[r.x+3,r.y+3],[r.right-3,r.bottom-3]].every(([x,y])=>document.elementFromPoint(x,y)?.closest('button')===button)}'''),button.get_attribute('data-page')
    def swipe(target,dx,dy=0):
        page.dispatch_event(target,'pointerdown',{'pointerId':4,'clientX':200,'clientY':300})
        page.dispatch_event(target,'pointerup',{'pointerId':4,'clientX':200+dx,'clientY':300+dy})

    before=raw()
    for width in [1440,390,320]:
        page.set_viewport_size({'width':width,'height':1050 if width==1440 else 844})
        for id in ['hero','sheet','dice','actions','pack']:
            go(id);assert current()==id
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
            page.locator('.screen').scroll_into_view_if_needed()
            assert_nav_hit_targets()
            if id=='hero':assert_clear(['.hero .hp-actions','.hero .minor-actions'])
            if id=='dice':assert_clear(['[data-action="roll"]'])
            if id=='pack':assert_clear(['.pack-list','.pack>.minor-actions'])
            if id=='sheet':assert_clear(['.stat-grid','.sheet-foot'])
            if id=='actions':assert_clear(['.weapon-content'])
        for tab in ['powers','encounter','grid']:
            go('actions');page.locator(f'[data-action-tab="{tab}"]').click()
            assert_clear(['.powers-scroll','.encounter-scroll','.grid-play-area','.actions>.minor-actions'])
            if tab=='grid':
                if page.locator('.tactical-grid>button').count()==81:page.locator('[data-action="zoom-grid"]').click()
                assert page.locator('.tactical-grid>button').count()==25
                for button in page.locator('.map-camera button').all():
                    assert button.evaluate('b=>{const r=b.getBoundingClientRect();return document.elementFromPoint(r.x+r.width/2,r.y+r.height/2)?.closest("button")===b}')
                swipe('.tactical-grid',-100);assert current()=='actions'
                page.screenshot(path=str(OUTPUT/f'map-{width}.png'))
        go('hero');page.screenshot(path=str(OUTPUT/f'hero-{width}.png'))
        assert all(button.bounding_box()['height']>=44 for button in page.locator('.quick-navigation button').all())
        if width<600:
            page.evaluate('scrollTo(0,0)');box=page.locator('.quick-navigation').bounding_box();assert box['y']+box['height']<844
        assert raw()==before,'Navigation changed character data'
    assert not layout_failures,layout_failures

    # Scroll positions belong to their view and survive both navigation and state re-rendering.
    go('sheet');page.locator('[data-sheet-tab="skills"]').click();page.locator('.sheet-scroll').evaluate('el=>el.scrollTop=135')
    page.locator('[data-sheet-tab="saves"]').click();assert page.locator('.sheet-scroll').evaluate('el=>el.scrollTop')==0
    page.locator('[data-sheet-tab="skills"]').click();assert page.locator('.sheet-scroll').evaluate('el=>el.scrollTop')==135
    go('hero');go('sheet');assert page.locator('.sheet-scroll').evaluate('el=>el.scrollTop')==135
    go('pack');page.locator('.pack-list').evaluate('el=>el.scrollTop=140')
    page.locator('[data-item-quantity="1"][data-item="starter-potion"]').click()
    assert page.locator('.pack-list').evaluate('el=>el.scrollTop')>=100
    go('hero');go('pack');assert page.locator('.pack-list').evaluate('el=>el.scrollTop')>=100

    # Keyboard and pointer gestures are explicit and cannot take over dialogs or map controls.
    go('hero');page.locator('.screen').focus();page.keyboard.press('ArrowRight');assert current()=='sheet'
    page.keyboard.press('ArrowLeft');assert current()=='hero'
    swipe('.scene',-100);assert current()=='sheet';swipe('.page.sheet',100);assert current()=='hero'
    swipe('.scene',0,100);assert current()=='hero'
    go('dice');page.locator('.screen').focus();page.keyboard.press('Space');assert page.locator('#character-select').is_disabled()
    assert page.locator('[data-go-page="hero"]').is_disabled()
    page.wait_for_timeout(750);assert not page.locator('[data-go-page="hero"]').is_disabled()
    page.locator('.screen').focus();page.keyboard.press('Home');assert current()=='hero'
    page.locator('[data-action="edit"]').click();page.locator('[name="name"]').focus();page.keyboard.press('ArrowRight');assert current()=='hero'
    # Presets and swatches update only the draft until Save.
    old_avatar=state()['avatar'];saved=raw()
    page.locator('[data-avatar-preset="1"]').click();assert page.locator('[name="cloak"]:checked').input_value()=='1'
    assert raw()==saved
    page.locator('[data-avatar-reset]').click();assert page.locator('[name="cloak"]:checked').input_value()==str(old_avatar['cloak'])
    page.locator('[data-avatar-preset="2"]').click();page.locator('[name="skin"][value="0"]').check()
    page.locator('[name="name"]').fill('Mira <&>');assert page.locator('.avatar-caption').inner_text()=='Mira <&>'
    page.screenshot(path=str(OUTPUT/'avatar-editor-320.png'))
    page.keyboard.press('Escape');page.locator('dialog').wait_for(state='detached');assert raw()==saved
    page.locator('[data-action="edit"]').click();page.locator('[data-avatar-preset="2"]').click();page.locator('[name="skin"][value="0"]').check()
    page.get_by_role('button',name='SALVA PERSONAGGIO',exact=True).click()
    assert state()['avatar']=={'skin':0,'hair':2,'cloak':2,'style':2}
    page.reload();assert state()['avatar']=={'skin':0,'hair':2,'cloak':2,'style':2}
    assert state()['inventory']==fixture['characters'][0]['state']['inventory'][:1]+[{**fixture['characters'][0]['state']['inventory'][1],'quantity':3}]+fixture['characters'][0]['state']['inventory'][2:]
    go('pack');page.locator('[data-equip-weapon="starter-sword"]').click();go('hero');assert page.locator('.hero .avatar-weapon').count()==0
    page.locator('[data-action="edit"]').click();assert page.locator('.editor-preview .avatar-weapon').count()==0
    page.keyboard.press('Escape');page.locator('dialog').wait_for(state='detached')
    go('pack');page.locator('[data-equip-weapon="starter-sword"]').click();go('hero');assert page.locator('.hero .avatar-weapon').count()==1
    page.locator('[data-action="manage-characters"]').click();assert page.locator('.character-card').first.locator('.avatar-weapon').count()==1;assert page.locator('.character-card').nth(1).locator('.avatar-weapon').count()==0
    page.locator('[data-library-close]').click()

    page.set_viewport_size({'width':1440,'height':1050});saved=raw();page.locator('[data-action="focus-view"]').click()
    assert page.locator('body').get_attribute('class')=='focus-mode';assert page.locator('.intro').is_hidden()
    assert raw()==saved;page.screenshot(path=str(OUTPUT/'focus-desktop.png'))
    page.locator('[data-action="focus-view"]').click();assert page.locator('.intro').is_visible()
    page.emulate_media(reduced_motion='reduce');assert page.locator('.hero .sprite').evaluate('el=>getComputedStyle(el).animationName')=='none'
    assert not errors,errors
    browser.close()
print('PASS: circular hit targets and content separation, five sections desktop/mobile 390/320, quick navigation >=44px, first-screen mobile play, view scroll memory, keyboard/swipe guards, roll isolation, avatar presets/swatches/preview/save/cancel, equipment depiction, focus view, reduced motion, schema v9 unchanged; no JS errors.')
