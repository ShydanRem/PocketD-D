"""v0.10 inventory/weapon browser regression; run after npm run build."""
import json
import subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=Path('/tmp/pocketdd-v010-qa');OUTPUT.mkdir(exist_ok=True)
LIBRARY='pocketdd-library-v1'
legacy=json.loads(subprocess.check_output(['node','--input-type=module','-e',"""
import {initialState} from './src/model.js';
import {legacyBackup} from './src/test-fixtures.js';
import {upsertParticipant,startEncounter} from './src/encounter.js';
import {placeToken,toggleWall} from './src/grid.js';
let s=upsertParticipant(initialState(),{id:'hero',initiative:12});
s=upsertParticipant(s,{id:'enemy',name:'Guardiano',initiative:1,hp:100,maxHp:100,armorClass:1,side:'enemy'});
s=startEncounter(toggleWall(placeToken(placeToken(s,'hero',40),'enemy',42),41));
console.log(JSON.stringify({...legacyBackup(s,8),hp:31,potions:1,torches:0,swordEquipped:false,weaponReach:3}));
"""],cwd=ROOT,text=True))

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    page=browser.new_page(viewport={'width':1440,'height':1000},accept_downloads=True)
    errors=[];accept=[True]
    page.on('pageerror',lambda error:errors.append(str(error)))
    page.on('dialog',lambda dialog:dialog.accept() if accept[0] else dialog.dismiss())
    page.route('**/*',lambda route:route.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if route.request.is_navigation_request() else route.abort())
    page.goto('http://pocket.test')
    # The saved v0.9 collection contains a genuine v8 character.
    old_collection={'type':'pocketdd-library','version':1,'activeId':'legacy','characters':[{'id':'legacy','state':legacy}]}
    page.evaluate('(raw)=>localStorage.setItem("pocketdd-library-v1",raw)',json.dumps(old_collection));page.reload()
    def raw():return page.evaluate('(key)=>localStorage.getItem(key)',LIBRARY)
    def library():return json.loads(raw())
    def hero():
        data=library();return next(item['state'] for item in data['characters'] if item['id']==data['activeId'])
    def item(id):return next(entry for entry in hero()['inventory'] if entry['id']==id)
    def pack():page.locator('[data-page="pack"]').click()
    def manager():page.locator('[data-action="manage-inventory"]').first.click()
    def close():page.locator('[data-inventory-close]').click()
    def save():
        page.get_by_role('button',name='Salva oggetto',exact=True).click()
        assert page.locator('.inventory-manager .editor-error').inner_text()==''
    def add(name,type='gear',quantity=1,notes='Nota <img src=x>'):
        page.locator('[data-new-item]').click();page.locator('[name="type"]').select_option(type)
        page.locator('[name="item-name"]').fill(name);page.locator('[name="quantity"]').fill(str(quantity));page.locator('[name="notes"]').fill(notes)
    def randomness(values):
        page.evaluate('''values=>{const original=crypto.getRandomValues.bind(crypto);crypto.getRandomValues=array=>{if(array instanceof Uint32Array){if(!values.length)throw new Error('Random exhausted');array[0]=values.shift();return array}return original(array)}}''',values)
    def upload(id,payload):
        page.locator('#'+id).set_input_files({'name':'backup.json','mimeType':'application/json','buffer':json.dumps(payload).encode()});page.wait_for_timeout(100)
    def exported(selector):
        with page.expect_download() as download:page.locator(selector).click()
        path=OUTPUT/download.value.suggested_filename;download.value.save_as(path);return json.loads(path.read_text())

    pack();assert 'Pozione ×1' in page.locator('.pack-list').inner_text();assert 'Torcia ×0' in page.locator('.pack-list').inner_text()
    assert page.locator('[data-use-item="starter-torch"]').is_disabled()
    # The first save migrates the collection to v9 without replenishing resources.
    page.locator('[data-item-quantity="1"][data-item="starter-torch"]').click()
    assert hero()['version']==9 and item('starter-sword')['reach']==3 and hero()['equippedWeaponId'] is None
    randomness([3,3]);page.locator('[data-use-item="starter-potion"]').click()
    assert hero()['hp']==32 and item('starter-potion')['quantity']==0
    assert page.locator('[data-use-item="starter-potion"]').is_disabled()
    page.locator('[data-action="undo"]').click();assert hero()['hp']==31 and item('starter-potion')['quantity']==1
    manager();add('Corda <&>',quantity=2);save();assert page.locator('.inventory-manager img').count()==0
    rope=next(entry['id'] for entry in hero()['inventory'] if entry['name']=='Corda <&>')
    add('Razione','consumable');save();ration=next(entry['id'] for entry in hero()['inventory'] if entry['name']=='Razione')
    add('Rimedio','healing');page.locator('[name="diceCount"]').fill('1');page.locator('[name="dieSides"]').select_option('12');page.locator('[name="healingModifier"]').fill('4');save()
    heal=next(entry['id'] for entry in hero()['inventory'] if entry['name']=='Rimedio')
    add('Arco <&>','weapon');page.locator('[name="ability"]').select_option('DES');page.locator('[name="proficient"]').uncheck()
    for key,value in {'diceCount':'2','attackModifier':'1','damageModifier':'2','reach':'6','damageType':'Perforante'}.items():page.locator(f'[name="{key}"]').fill(value)
    page.locator('[name="dieSides"]').select_option('6');save()
    bow=next(entry['id'] for entry in hero()['inventory'] if entry['name']=='Arco <&>')
    page.locator(f'[data-manager-equip="{bow}"]').click();assert hero()['equippedWeaponId']==bow
    page.screenshot(path=str(OUTPUT/'inventory-desktop.png'));close()
    pack();assert page.locator('.inventory-card img').count()==0
    page.locator(f'[data-use-item="{ration}"]').click();assert item(ration)['quantity']==0 and hero()['hp']==31
    assert page.locator(f'[data-use-item="{ration}"]').is_disabled()
    page.locator('[data-action="undo"]').click();assert item(ration)['quantity']==1
    randomness([11]);page.locator(f'[data-use-item="{heal}"]').click();assert hero()['hp']==32 and item(heal)['quantity']==0
    page.locator('[data-action="undo"]').click();assert hero()['hp']==31 and item(heal)['quantity']==1

    page.locator('[data-page="actions"]').click();page.locator('#attack-target').select_option('enemy')
    assert 'Arco <&>' in page.locator('.actions h2').inner_text()
    assert 'Attacco +4' in page.locator('.equipment-status').inner_text() and '2d6+5' in page.locator('.equipment-status').inner_text()
    assert 'portata 6' in page.locator('.geometry-info').inner_text() and page.locator('[data-action="attack"]').is_disabled()
    randomness([19,5,4,3,2]);page.locator('[data-action="override-attack"]').click();page.locator('[data-action="attack"]').click()
    assert '4d6+5' in page.locator('[data-action="damage"]').inner_text();page.locator('[data-action="damage"]').click()
    assert hero()['encounter']['strike']['amount']==23
    page.locator('[data-action="apply-damage"]').click();assert hero()['encounter']['participants'][1]['hp']==77
    assert page.locator('[data-action="apply-damage"]').is_disabled()
    assert hero()['history'][0]['sides']==6 and hero()['history'][0]['dice']==[6,5,4,3]
    assert hero()['history'][0]['bonus']==5
    page.screenshot(path=str(OUTPUT/'weapon-desktop.png'))
    page.locator('[data-page="sheet"]').click();page.locator('[data-sheet-tab="history"]').click()
    assert '4d6' in page.locator('.history-list article').first.inner_text()
    page.locator('[data-page="actions"]').click()
    # Editing the active weapon invalidates prepared results; zero quantity automatically unequips it.
    manager();page.locator(f'[data-edit-item="{bow}"]').click();page.locator('[name="quantity"]').fill('0');save();close()
    assert hero()['equippedWeaponId'] is None and hero()['encounter']['strike'] is None
    assert page.locator('[data-action="attack"]').is_disabled()
    pack();page.locator('[data-action="undo"]').click();assert hero()['equippedWeaponId']==bow and item(bow)['quantity']==1
    manager();page.locator(f'[data-edit-item="{bow}"]').click();page.locator('[name="type"]').select_option('gear');save();close()
    assert hero()['equippedWeaponId'] is None and item(bow)['type']=='gear'
    pack();page.locator('[data-action="undo"]').click();assert hero()['equippedWeaponId']==bow
    manager();accept[0]=False;before=raw();page.locator(f'[data-delete-item="{bow}"]').click();assert raw()==before
    accept[0]=True;page.locator(f'[data-delete-item="{bow}"]').click();assert hero()['equippedWeaponId'] is None
    close();pack();page.locator('[data-action="undo"]').click();assert hero()['equippedWeaponId']==bow
    # Bad imported inventories leave the collection untouched.
    before=raw();invalid=hero();invalid['inventory'][0]['quantity']=-1;upload('backup-file',invalid);assert raw()==before
    single=exported('[data-action="export"]');assert single['version']==9 and single['equippedWeaponId']==bow
    page.reload();assert hero()['equippedWeaponId']==bow and item(ration)['quantity']==1
    page.locator('[data-action="manage-characters"]').click();collection=exported('[data-action="export-library"]');page.locator('[data-library-close]').click()
    # Add the legacy backup as another hero; changes to the custom inventory stay with the first one.
    upload('backup-file',legacy);second=library()['activeId'];assert second!='legacy'
    pack();assert len(hero()['inventory'])==3 and hero()['equippedWeaponId'] is None
    page.locator('#character-select').select_option('legacy');assert hero()['equippedWeaponId']==bow and item(rope)['quantity']==2
    upload('library-file',collection);assert library()==collection
    for width in [390,320]:
        page.set_viewport_size({'width':width,'height':844});manager();page.locator(f'[data-edit-item="{bow}"]').click()
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
        assert page.locator('dialog').evaluate('el=>el.scrollWidth<=el.clientWidth')
        page.screenshot(path=str(OUTPUT/f'inventory-editor-{width}.png'));close()
        pack();page.screenshot(path=str(OUTPUT/f'pack-{width}.png'))
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
    # Empty inventories remain valid and usable.
    empty=hero();empty['inventory']=[];empty['equippedWeaponId']=None;upload('backup-file',empty)
    pack();assert 'Inventario vuoto' in page.locator('.pack-list').inner_text()
    page.locator('[data-page="actions"]').click();assert page.locator('[data-action="attack"]').is_disabled()
    assert not errors,errors
    browser.close()
print('PASS: v0.9/v8 migration, item CRUD, notes escaping, quantities, equip/unequip, consumables/healing/undo, custom attack and critical dice, walls/DM override, single damage application, edit/delete invalidation, v9 and collection backups, reload, character isolation, empty inventory, desktop/mobile 390/320; no JS errors.')
