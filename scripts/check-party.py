"""v0.12: real BroadcastChannel across three pages; run after npm run build."""
import json
import subprocess
import time
from pathlib import Path
from playwright.sync_api import sync_playwright, expect

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=Path('/tmp/pocketdd-v012-qa');OUTPUT.mkdir(exist_ok=True)
KEY='pocketdd-party-v1';LIBRARY='pocketdd-library-v1'
fixture=subprocess.check_output(['node','--input-type=module','-e',"""
import {initialState} from './src/model.js';
import {createLibrary,addCharacter,selectCharacter,serializeLibrary} from './src/library.js';
let lib=createLibrary(initialState(),'elian');
lib=addCharacter(lib,{...initialState(),name:'Mira'},'mira');
lib=addCharacter(lib,{...initialState(),name:'Nora'},'nora');
console.log(serializeLibrary(selectCharacter(lib,'elian')));
"""],cwd=ROOT,text=True).strip()

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    context=browser.new_context(viewport={'width':1440,'height':1100},accept_downloads=True)
    context.add_init_script('(raw=>{if(!localStorage.getItem("pocketdd-library-v1"))localStorage.setItem("pocketdd-library-v1",raw)})('+json.dumps(fixture)+')')
    context.route('**/*',lambda route:route.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if route.request.is_navigation_request() else route.abort())
    errors=[]
    def page(character):
        tab=context.new_page();tab.on('pageerror',lambda e:errors.append(str(e)));tab.on('dialog',lambda d:d.accept())
        tab.goto('http://pocket.test');tab.locator('#character-select').select_option(character)
        tab.locator('[data-action="party"]').click();return tab
    def saved(tab):return tab.evaluate('(key)=>JSON.parse(sessionStorage.getItem(key))',KEY)
    def state(tab):return saved(tab)['session']
    def actor(tab):return 'p_'+saved(tab)['clientId']
    def until(tab,predicate,timeout=8):
        deadline=time.monotonic()+timeout
        while time.monotonic()<deadline:
            if predicate():return
            tab.wait_for_timeout(100)
        assert predicate(),'Timed out waiting for party state'
    def wait_same(*tabs):
        expected=state(tabs[0])
        for tab in tabs[1:]:
            try:until(tab,lambda:state(tab)==expected)
            except Exception:
                (OUTPUT/'failure.json').write_text(json.dumps({'expected':expected,'actual':state(tab),'errors':errors},indent=2));raise
    def choose_initiative(tab,value):
        tab.locator('[name="initiative"]').fill(str(value));tab.locator('[data-party-form="initiative"] button').click()
        until(tab,lambda:not saved(tab)['pending'] and next(a for a in state(tab)['encounter']['participants'] if a['id']==actor(tab))['initiative']==value)
    dm=page('elian');dm.locator('[data-party-form="create"] button').click();code=dm.locator('[data-party-code]').inner_text()
    first=page('mira');first.locator('[name="code"]').fill(code);first.locator('[data-party-form="join"] button').click()
    expect(dm.locator('[data-party-actor]')).to_have_count(2)
    second=page('nora');second.locator('[name="code"]').fill(code);second.locator('[data-party-form="join"] button').click()
    expect(dm.locator('[data-party-actor]')).to_have_count(3);wait_same(dm,first,second)
    before=dm.evaluate('(key)=>localStorage.getItem(key)',LIBRARY)
    choose_initiative(dm,15);wait_same(dm,first,second)
    choose_initiative(first,20);wait_same(dm,first,second)
    choose_initiative(second,10);wait_same(dm,first,second)
    for tab,cell in [(dm,40),(first,42),(second,44)]:
        dm.locator('[name="placement"]').select_option(actor(tab));dm.locator(f'[data-party-cell="{cell}"]').click();wait_same(dm,first,second)
    dm.locator('[name="placement"]').select_option('wall');dm.locator('[data-party-cell="31"]').click();wait_same(dm,first,second)
    dm.locator('.party-npc summary').click();dm.locator('[name="npcName"]').fill('Sentinella');dm.locator('[data-party-form="npc"] button').click()
    expect(dm.locator('[data-party-actor]')).to_have_count(4);wait_same(dm,first,second)
    npc=next(a['id'] for a in state(dm)['encounter']['participants'] if a['name']=='Sentinella')
    dm.locator('[data-party="start"]').click();wait_same(dm,first,second)
    assert state(dm)['encounter']['activeId']==actor(first)
    expect(first.locator('[data-party="next"]')).to_have_count(0)
    expect(second.locator('[data-party-cell="43"]')).to_be_disabled()
    first.locator('[data-party-cell="43"]').click();wait_same(first,dm,second)
    assert state(dm)['encounter']['grid']['positions'][actor(first)]==43
    assert state(dm)['encounter']['grid']['spent']==1
    dm.locator(f'[data-party-hp="-1"][data-actor="{npc}"]').click();wait_same(dm,first,second)
    assert next(a for a in state(first)['encounter']['participants'] if a['id']==npc)['hp']==14

    # Offline player catches up without another roster entry, then reloads.
    first.locator('[data-party="pause"]').click();dm.locator('[data-party="next"]').click();dm.locator(f'[data-party-hp="-1"][data-actor="{npc}"]').click();wait_same(dm,second)
    expect(first.locator('[data-party-cell="41"]')).to_be_disabled()
    first.locator('[data-party="resume"]').click();wait_same(dm,first,second)
    original_id=saved(first)['clientId'];first.reload();first.locator('[data-action="party"]').click();wait_same(dm,first,second)
    assert saved(first)['clientId']==original_id;assert len(state(dm)['members'])==3

    # Close the coordinator's transport, observe unavailable state, then restore
    # its checkpoint by reloading that same tab. No election or state reset.
    checkpoint=state(dm);dm.locator('[data-party="pause"]').click()
    expect(first.locator('.party-status')).to_contain_text('sospeso',timeout=13000)
    expect(first.locator('[data-party-cell="41"]')).to_be_disabled()
    dm.reload();dm.locator('[data-action="party"]').click();wait_same(dm,first,second)
    assert state(dm)==checkpoint;expect(first.locator('.party-status')).to_contain_text('Collegato')
    assert dm.evaluate('(key)=>localStorage.getItem(key)',LIBRARY)==before,'Party changed personal character storage'

    # Export is a report, separate from personal backups.
    with dm.expect_download() as download:dm.locator('[data-party="export"]').click()
    assert json.loads(Path(download.value.path()).read_text())==state(dm)
    dm.locator('.party-layout').scroll_into_view_if_needed();dm.screenshot(path=str(OUTPUT/'party-coordinator.png'))
    for width in [390,320]:
        second.set_viewport_size({'width':width,'height':844})
        assert second.evaluate('document.documentElement.scrollWidth<=innerWidth')
        assert second.locator('.party-dialog').evaluate('el=>el.scrollWidth<=el.clientWidth')
        assert all(b.bounding_box()['width']>=44 and b.bounding_box()['height']>=44 for b in second.locator('[data-party-cell]').all())
        second.locator('.party-map-scroll').scroll_into_view_if_needed();second.locator('.party-map-scroll').evaluate('el=>el.scrollLeft=el.scrollWidth')
        assert second.locator('[data-party-cell="80"]').evaluate('el=>{const r=el.getBoundingClientRect();return r.right<=innerWidth}')
        second.screenshot(path=str(OUTPUT/f'party-{width}.png'))
    assert not errors,errors
    (OUTPUT/'result.json').write_text(json.dumps({'passed':True,'clients':3,'viewports':[1440,390,320],'jsErrors':errors,'checks':['shared initiative, turns, HP and grid','player turn ownership','offline catch-up','player reload identity','coordinator absence and reload','personal collection unchanged','export report','44px map controls and horizontal scroll']},indent=2))
    print('PASS: three real BroadcastChannel clients; shared initiative/turns/HP/map; player permissions; disconnect, catch-up and reload; coordinator absence and checkpoint; unchanged personal library; export; desktop/mobile 390/320; no JS errors.')
    browser.close()
