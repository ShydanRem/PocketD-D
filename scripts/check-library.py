"""Browser verification of v0.9. Run after npm run build; requires Python Playwright and Chromium."""
import json
import subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright

ROOT = Path(__file__).resolve().parents[1]
OUTPUT = Path('/tmp/pocketdd-v09-qa')
OUTPUT.mkdir(exist_ok=True)
LIBRARY = 'pocketdd-library-v1'
LEGACY = 'pocketdd-demo-v1'
fixture = json.loads(subprocess.check_output(['node','--input-type=module','-e', """
import {initialState,serialize,recordRoll} from './src/model.js';
import {upsertPower,usePower} from './src/powers.js';
import {upsertParticipant,startEncounter,queueDamage,applyPendingDamage} from './src/encounter.js';
import {placeToken,moveActive} from './src/grid.js';
let s={...initialState(),name:'Mira',hp:7,potions:0,condition:true};
s=upsertPower(s,{id:'light',type:'spell',name:'Luce',description:'Nota',level:0,concentration:true});s=usePower(s,'light');
s=upsertParticipant(s,{id:'hero',initiative:12});s=upsertParticipant(s,{id:'enemy',name:'Guardiano',initiative:1,hp:20,maxHp:20,armorClass:12,side:'enemy'});
s=moveActive(startEncounter(placeToken(placeToken(s,'hero',40),'enemy',80)),41);
s=applyPendingDamage(queueDamage(s,'enemy',4,'hit'));s=recordRoll(s,'Prova',{dice:[10],bonus:2,total:12},'d20','normal',1000);
console.log(serialize(s));
"""],cwd=ROOT,text=True))
errors=[]
accept=[True]

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    def open_page(context=None):
        page=(context.new_page() if context else browser.new_page(viewport={'width':1440,'height':1000},accept_downloads=True))
        page.on('pageerror',lambda error:errors.append(str(error)))
        page.on('dialog',lambda dialog:dialog.accept() if accept[0] else dialog.dismiss())
        page.route('**/*',lambda route:route.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if route.request.is_navigation_request() else route.abort())
        page.goto('http://pocket.test')
        return page
    def seed(page,entries):
        page.evaluate('(entries)=>{localStorage.clear();for(const [key,value] of entries)localStorage.setItem(key,value)}',entries)
        page.reload()
    def raw(page):return page.evaluate('(key)=>localStorage.getItem(key)',LIBRARY)
    def data(page):return json.loads(raw(page))
    def hp(page):return page.locator('.health strong').inner_text()
    def choose(page,id):page.locator('#character-select').select_option(id)
    def manager(page):page.locator('[data-action="manage-characters"]').click()
    def close_manager(page):page.locator('[data-library-close]').click()
    def upload(page,id,payload,name='backup.json'):
        page.locator('#'+id).set_input_files({'name':name,'mimeType':'application/json','buffer':json.dumps(payload).encode()})
        page.wait_for_timeout(100)
    def exported(page,selector):
        with page.expect_download() as result:page.locator(selector).click()
        target=OUTPUT/result.value.suggested_filename;result.value.save_as(target)
        return target.read_text()

    page=open_page();old=json.dumps(fixture);seed(page,[[LEGACY,old]])
    assert data(page)['characters'][0]['state']==fixture
    assert page.evaluate('(key)=>localStorage.getItem(key)',LEGACY)==old
    assert hp(page)=='7 / 32'
    first=data(page)['activeId']
    page.locator('[data-action="new"]').click();page.locator('[name="name"]').fill('Nora <&>')
    page.get_by_role('button',name='CREA PERSONAGGIO',exact=True).click()
    second=data(page)['activeId'];assert second!=first and len(data(page)['characters'])==2
    assert hp(page)=='32 / 32' and page.locator('.hero h2 img').count()==0
    page.locator('[data-hp="-1"]').click();assert hp(page)=='31 / 32'
    choose(page,first);assert hp(page)=='7 / 32';assert page.locator('[data-action="undo"]').is_disabled()
    page.locator('[data-page="actions"]').click();page.locator('#attack-target').select_option('enemy')
    assert page.locator('[data-action="attack"]').is_disabled()
    page.locator('[data-action="override-attack"]').click()
    choose(page,second);choose(page,first);page.locator('[data-page="actions"]').click()
    assert page.locator('#attack-target').input_value()==''
    page.locator('#attack-target').select_option('enemy');assert page.locator('[data-action="attack"]').is_disabled()
    expected=json.loads(json.dumps(fixture));expected['encounter']['strike']=None
    assert data(page)['characters'][0]['state']==expected
    choose(page,second);page.reload();assert data(page)['activeId']==second and hp(page)=='31 / 32'

    manager(page);page.locator(f'[data-copy-character="{first}"]').click()
    duplicate=data(page)['activeId'];assert duplicate not in [first,second]
    close_manager(page);assert hp(page)=='7 / 32';page.locator('[data-hp="-1"]').click()
    choose(page,first);assert hp(page)=='7 / 32';choose(page,duplicate);assert hp(page)=='6 / 32'
    manager(page);accept[0]=False;page.locator(f'[data-delete-character="{duplicate}"]').click()
    assert len(data(page)['characters'])==3
    accept[0]=True;page.locator(f'[data-delete-character="{duplicate}"]').click()
    assert len(data(page)['characters'])==2 and data(page)['activeId']==first
    collection=json.loads(exported(page,'[data-action="export-library"]'));close_manager(page)
    # Importing a single v8 backup creates a new independent entry, even if the name already exists.
    upload(page,'backup-file',fixture);assert len(data(page)['characters'])==3
    imported=data(page)['activeId'];assert imported not in [first,second] and hp(page)=='7 / 32'
    before=raw(page);upload(page,'backup-file',{'version':8,'hp':-1});assert raw(page)==before
    invalid=json.loads(json.dumps(collection));invalid['characters'][0]['state']['hp']=-1
    upload(page,'library-file',invalid);assert raw(page)==before
    accept[0]=False;upload(page,'library-file',collection);assert raw(page)==before
    accept[0]=True;upload(page,'library-file',collection);assert data(page)==collection
    choose(page,second);page.locator('[data-hp="-1"]').click();assert hp(page)=='30 / 32'
    choose(page,first);assert hp(page)=='7 / 32'
    # During an asynchronous roll the character selector cannot redirect the result to another hero.
    page.locator('[data-page="dice"]').click();page.locator('[data-action="roll"]').click()
    assert page.locator('#character-select').is_disabled()
    page.locator('[data-action="roll"]').wait_for(state='visible');page.wait_for_timeout(700)
    assert not page.locator('#character-select').is_disabled()
    assert len(data(page)['characters'][0]['state']['history'])==2
    for width in [390,320]:
        page.set_viewport_size({'width':width,'height':844});manager(page)
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
        assert page.locator('dialog').evaluate('(el)=>el.scrollWidth<=el.clientWidth')
        page.screenshot(path=str(OUTPUT/f'characters-{width}.png'))
        close_manager(page);choose(page,second);choose(page,first)
    page.set_viewport_size({'width':1440,'height':1000});manager(page)
    page.screenshot(path=str(OUTPUT/'characters-desktop.png'));close_manager(page)

    # Corrupted collections are never replaced by the legacy slot or by ordinary game actions.
    broken=open_page();seed(broken,[[LIBRARY,'{corrotto'],[LEGACY,old]])
    assert broken.locator('.storage-warning').count()==1
    broken.locator('[data-hp="-1"]').click();assert raw(broken)=='{corrotto'
    recovered=exported(broken,'[data-action="recover-storage"]');assert recovered=='{corrotto'
    upload(broken,'library-file',collection);assert data(broken)==collection
    assert broken.locator('.storage-warning').count()==0

    # A full browser quota retains the last disk value, but the in-memory collection is exportable.
    quota=open_page();seed(quota,[[LIBRARY,json.dumps(collection)]])
    before=raw(quota)
    quota.evaluate("() => {Storage.prototype.setItem=function(){throw new DOMException('Full','QuotaExceededError')}}")
    quota.locator('[data-hp="-1"]').click();assert raw(quota)==before
    assert quota.locator('.storage-warning').count()==1
    manager(quota);in_memory=json.loads(exported(quota,'[data-action="export-library"]'))
    assert in_memory['characters'][0]['state']['hp']==6

    # Two tabs share storage; a stale tab keeps its own session without overwriting a newer saved one.
    context=browser.new_context(viewport={'width':1440,'height':1000},accept_downloads=True)
    a=open_page(context);b=open_page(context)
    a.locator('[data-hp="-1"]').click();saved=raw(a)
    b.locator('[data-hp="-1"]').click();assert raw(b)==saved and b.locator('.storage-warning').count()==1
    manager(b);assert json.loads(exported(b,'[data-action="export-library"]'))['characters'][0]['state']['hp']==23
    # Denied storage supports a temporary session and downloadable backup.
    denied=open_page();denied.add_init_script("(() => {Storage.prototype.getItem=function(){throw new DOMException('Denied','SecurityError')}})()")
    denied.reload();denied.locator('[data-hp="-1"]').click();assert hp(denied)=='23 / 32'
    manager(denied);assert json.loads(exported(denied,'[data-action="export-library"]'))['characters'][0]['state']['hp']==23
    assert not errors,errors
    browser.close()
print('PASS: v8 migration, independent characters/resources/history/combat, creation, duplication, selection/reload, deletion/confirmation, single and collection backup, invalid imports, roll isolation, corruption recovery, quota, denied storage, stale tabs, desktop/mobile 390/320; no JS errors.')
