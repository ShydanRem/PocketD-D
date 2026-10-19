"""Synthetic browser verification of the actual built PocketD-D import flow.
Never contacts Beyond, never uses the user's browser profile or source JSON.
"""
import json
import select
import subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright, expect

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=Path('/tmp/pocketdd-beyond-import-qa');OUTPUT.mkdir(exist_ok=True)
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {convertBeyond} from './src/beyond-conversion.js';
import {beyondCoreFixture} from './src/beyond-fixtures.js';
import {createLibrary,addCharacter,serializeLibrary} from './src/library.js';
import {initialState} from './src/model.js';
const seed=addCharacter(createLibrary({...initialState(),name:'Prima scheda',hp:7},'first'),{...initialState(),name:'Seconda scheda',hp:11},'second');
const server=createBeyondLabServer({appRoot:new URL('./dist/',import.meta.url),probe:async id=>{
  await new Promise(resolve=>setTimeout(resolve,250));
  if(id==='999')throw Object.assign(new Error('Rete sintetica non disponibile'),{code:'NETWORK_ERROR'});
  const p=beyondCoreFixture();p.data.id=Number(id);p.data.name='Sonda <b>x</b>';
  p.data.inventory[0].definition.name='<img src=x onerror=alert(1)>';
  if(id==='222222')p.data.modifiers.feat.push({type:'set',subType:'strength-score',value:19});
  return {...convertBeyond(p,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:250,authenticated:false}};
}});
server.listen(0,'127.0.0.1',()=>console.log(JSON.stringify({url:'http://127.0.0.1:'+server.address().port,seed:serializeLibrary(seed)})));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
    assert select.select([server.stdout],[],[],10)[0],'Local server did not start'
    config=json.loads(server.stdout.readline());address=config['url'];seed=config['seed'];key='pocketdd-library-v1'
    with sync_playwright() as p:
        browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
        context=browser.new_context(viewport={'width':1440,'height':1050},accept_downloads=True)
        context.route('**/*',lambda r:r.continue_() if r.request.url.startswith(address+'/') else r.abort())
        page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
        page.goto(address);page.evaluate('([k,v])=>localStorage.setItem(k,v)',[key,seed]);page.reload()
        original=json.loads(seed)
        def raw():return page.evaluate('(k)=>localStorage.getItem(k)',key)
        def open_import():page.locator('[data-action=beyond-import]').click();expect(page.locator('.beyond-import')).to_be_visible()
        def read(value='123456'):
            page.locator('[name=character]').fill(value);page.locator('[data-read] button').click()
        def preview(value='123456'):
            open_import();read(value);expect(page.locator('[data-preview]')).to_be_visible()
        def accept():
            page.locator('[data-accept]').check();page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click()
        # HTTP/format errors and blocked conversion do not write.
        open_import();read('https://evil.test/characters/123456');expect(page.locator('[data-status]')).to_contain_text('INVALID_URL');assert raw()==seed
        read('999');expect(page.locator('[data-status]')).to_contain_text('NETWORK_ERROR');assert raw()==seed
        read('222222');expect(page.locator('[data-preview]')).to_be_visible();expect(page.locator('[data-accept]')).to_be_disabled();assert raw()==seed
        # Editing the URL while waiting invalidates both pending and previous results.
        read();expect(page.locator('[data-read] button')).to_be_disabled();page.locator('[name=character]').fill('999');page.wait_for_timeout(350)
        expect(page.locator('[data-preview]')).to_be_hidden();assert raw()==seed
        read();expect(page.locator('[data-preview]')).to_be_visible()
        expect(page.locator('[data-checks]')).to_contain_text('DATI SINTETICI');expect(page.locator('[data-name]')).to_have_text('Sonda <b>x</b> · livello 3')
        assert page.locator('.beyond-import img,.beyond-import h3 b').count()==0
        expect(page.locator('[data-values]')).to_contain_text('25/29');expect(page.locator('[data-create]')).to_be_disabled();assert raw()==seed
        for width in [1440,390,320]:
            page.set_viewport_size({'width':width,'height':1050 if width==1440 else 844})
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
            assert page.locator('.beyond-import').evaluate('(el)=>el.scrollWidth<=el.clientWidth')
            page.locator('.beyond-import').evaluate('(el)=>el.scrollTop=0')
            page.screenshot(path=str(OUTPUT/f'preview-{width}.png'))
        page.set_viewport_size({'width':1440,'height':1050})
        page.locator('[data-accept]').check();page.once('dialog',lambda d:d.dismiss());page.locator('[data-create]').click();assert raw()==seed
        page.once('dialog',lambda d:d.accept());page.locator('[data-create]').click();expect(page.locator('.beyond-import')).to_have_count(0)
        imported=json.loads(raw());assert len(imported['characters'])==3
        assert imported['characters'][:2]==original['characters'];new=imported['characters'][2]['state']
        assert new['abilities']['COS']==17 and new['maxHp']==29 and new['hp']==25 and new['armorClass']==14
        assert len(new['inventory'])==2 and new['equippedWeaponId'] is None and new['powers']==[]
        assert new['importInfo']['limitations'] and new['resolve']==0
        page.reload();expect(page.locator('.import-notice')).to_be_visible()
        page.locator('[data-action=beyond-info]').click();expect(page.locator('.beyond-import')).to_contain_text('salvezze');page.locator('.beyond-import button').click()
        page.locator('[data-action=manage-characters]').click()
        with page.expect_download() as download:page.locator('[data-action=export-library]').click()
        exported=json.loads(Path(download.value.path()).read_text());assert exported==imported
        page.locator('[data-library-close]').click()
        # Closing an inspected preview preserves the already saved collection.
        current=raw();preview();page.locator('[data-close]').click();assert raw()==current
        # A stale tab cannot overwrite a second tab's newer collection.
        preview();other=context.new_page();other.goto(address)
        changed=json.loads(current);changed['characters'][0]['state']['hp']=2
        changedraw=json.dumps(changed);other.evaluate('([k,v])=>localStorage.setItem(k,v)',[key,changedraw]);other.close()
        accept();expect(page.locator('[data-status]')).to_contain_text('non salvata');assert raw()==changedraw
        page.locator('[data-close]').click();page.reload()
        # Quota failure leaves the collection and the active hero unchanged.
        before=raw();preview();page.evaluate("""()=>{const original=Storage.prototype.setItem;Storage.prototype.setItem=function(k,v){if(k==='pocketdd-library-v1')throw new DOMException('quota','QuotaExceededError');return original.call(this,k,v)}}""")
        accept();expect(page.locator('[data-status]')).to_contain_text('non salvata');assert raw()==before
        page.reload()
        # Collection capacity is enforced before writing.
        full=json.loads(seed);full['characters']=[{**original['characters'][0],'id':f'hero-{i}'} for i in range(20)];full['activeId']='hero-0'
        fullraw=json.dumps(full);page.evaluate('([k,v])=>localStorage.setItem(k,v)',[key,fullraw]);page.reload();preview();accept()
        expect(page.locator('[data-status]')).to_contain_text('20 personaggi');assert raw()==fullraw
        # Corrupt storage is never replaced as a side effect of import.
        page.evaluate('(k)=>localStorage.setItem(k,"invalid-existing-data")',key);page.reload();preview();accept()
        expect(page.locator('[data-status]')).to_contain_text('recupero');assert raw()=='invalid-existing-data'
        assert not errors,errors
        (OUTPUT/'result.json').write_text(json.dumps({'passed':True,'evidence':'synthetic connector only; isolated browser collection','checks':['preview read-only','hostile text','conversion blockers','cancel','URL changes and pending races','new character once','old characters identical','backup and reload retain provenance','stale-tab conflict','quota failure','20 character limit','corrupt storage preserved','desktop/mobile 390/320'],'jsErrors':errors},indent=2))
        print('PASS: synthetic PC import, preview/confirm, old sheets preserved, reload/backup, conflicts/quota/limits, desktop/mobile; no JS errors. Not live Beyond evidence.')
        browser.close()
finally:
    server.terminate()
    try:server.wait(timeout=5)
    except subprocess.TimeoutExpired:server.kill();server.wait()
