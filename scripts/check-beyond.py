"""Exercise the local Beyond lab with synthetic responses, never the live service."""
import json
import select
import subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright, expect

ROOT=Path(__file__).resolve().parents[1]
OUTPUT=Path('/tmp/pocketdd-beyond-qa');OUTPUT.mkdir(exist_ok=True)
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBeyondLabServer} from './scripts/beyond-lab.js';
import {previewBeyond} from './src/beyond.js';
import {beyondFixture} from './src/beyond-fixtures.js';
const server=createBeyondLabServer({probe:async id=>{
  await new Promise(resolve=>setTimeout(resolve,500));
  if(id==='999')throw Object.assign(new Error('Rete di test non disponibile'),{code:'NETWORK_ERROR'});
  const source=beyondFixture();source.data.id=Number(id);source.data.name='Sonda <img src=x onerror=alert(1)>';
  return {...previewBeyond(source,id),testFixture:true,transport:{status:200,bytes:1234,elapsedMs:500,authenticated:false}};
}});
server.listen(0,'127.0.0.1',()=>console.log('http://127.0.0.1:'+server.address().port));
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
    assert select.select([server.stdout],[],[],10)[0],'Local fixture server did not start'
    address=server.stdout.readline().strip();assert address.startswith('http://127.0.0.1:'),address
    with sync_playwright() as p:
        browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
        context=browser.new_context(viewport={'width':1440,'height':1050},accept_downloads=True)
        context.route('**/*',lambda route:route.continue_() if route.request.url.startswith(address+'/') else route.abort())
        page=context.new_page();errors=[];dialogs=[]
        page.on('pageerror',lambda error:errors.append(str(error)))
        page.on('dialog',lambda dialog:(dialogs.append(dialog.message),dialog.dismiss()))
        page.goto(address);page.evaluate('localStorage.setItem("pocketdd-library-v1","unchanged-sentinel")')
        page.locator('#character').fill('https://www.dndbeyond.com/characters/123456');page.locator('#probe button').click()
        expect(page.locator('#probe button')).to_be_disabled()
        expect(page.locator('#result')).to_be_visible()
        expect(page.locator('#name')).to_have_text('Sonda <img src=x onerror=alert(1)>')
        expect(page.locator('#evidence')).to_contain_text('SIMULATA')
        expect(page.locator('#hp')).to_contain_text('19/24')
        expect(page.locator('#abilities dd')).to_have_count(6)
        expect(page.locator('#inventory')).to_contain_text('Lama di prova ×1')
        assert page.locator('img').count()==0 and not dialogs
        with page.expect_download() as download:page.locator('#download').click()
        report=json.loads(Path(download.value.path()).read_text());assert report['canImport'] is False and report['testFixture'] is True
        assert report['format']=='pocketdd-beyond-probe' and 'data' not in report
        for width in [1440,390,320]:
            page.set_viewport_size({'width':width,'height':1050 if width==1440 else 844})
            assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
            assert page.locator('#probe button').bounding_box()['height']>=44
            page.screenshot(path=str(OUTPUT/f'lab-{width}.png'),full_page=True)
        page.locator('#character').fill('https://evil.test/characters/123456');page.locator('#probe button').click()
        expect(page.locator('#status')).to_contain_text('INVALID_URL');expect(page.locator('#result')).to_be_hidden()
        page.locator('#character').fill('999');page.locator('#probe button').click()
        expect(page.locator('#status')).to_contain_text('NETWORK_ERROR');expect(page.locator('#result')).to_be_hidden()
        expect(page.locator('#probe button')).to_be_enabled()
        assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')=='unchanged-sentinel'
        assert not errors,errors
        (OUTPUT/'result.json').write_text(json.dumps({'passed':True,'evidence':'synthetic HTTP responses only; no live Beyond access','jsErrors':errors,'viewports':[1440,390,320],'checks':['form and pending state','partial preview and missing conversion warnings','safe text rendering','downloadable read-only report','bad URL and network error','personal storage unchanged']},indent=2))
        print('PASS: Beyond lab with synthetic HTTP responses; preview, report, text escaping, busy state, input/network errors, unchanged local storage, desktop/mobile 390/320; no JS errors. Live Beyond access is NOT verified.')
        browser.close()
finally:
    server.terminate()
    try:server.wait(timeout=5)
    except subprocess.TimeoutExpired:server.kill();server.wait()
