#!/usr/bin/env python3
"""Actual MV3 worker/content/popup against local synthetic Beyond; no account or remote writes."""
import json, select, subprocess, tempfile, sys
from pathlib import Path
from playwright.sync_api import sync_playwright, expect
ROOT = Path(__file__).resolve().parents[1]
fixture = r"""
import {createBridge} from './scripts/bridge-server.js';import {mkdtempSync,rmSync} from 'node:fs';import {tmpdir} from 'node:os';import {join} from 'node:path';import {createInterface} from 'node:readline';
const dir=mkdtempSync(join(tmpdir(),'pdd-ext-')),b=createBridge({directory:dir,ip:'127.0.0.1',test:true,devicePort:0,adminPort:0,verifyHP:async()=>({hp:18,max:30,temp:0})});await b.listen();
const binding=b.store.addBinding('123456');let snapshot={version:1,boot:'0123456789abcdef',counter:1,epoch:1,name:'Sonda estensione',vitals:{hp:20,max:30,temp:5},sequence:0,rolls:[]};b.store.observe(binding.id,snapshot);console.log(JSON.stringify({url:'http://127.0.0.1:'+b.admin.address().port,id:binding.id}));
createInterface({input:process.stdin}).on('line',line=>{try{const op=JSON.parse(line);if(op.type==='approve')b.store.approve(binding.id);if(op.type==='roll'){snapshot={...snapshot,counter:2,sequence:1,rolls:[{sequence:1,kind:2,label:'Tiro sintetico',sides:20,dice:[8,17],mode:1,bonus:3,total:20,floor:false}]};b.store.observe(binding.id,snapshot)}if(op.type==='hp'){snapshot={...snapshot,counter:3,vitals:{hp:18,max:30,temp:0}};b.store.observe(binding.id,snapshot)}if(op.type==='heartbeat')b.store.observe(binding.id,snapshot);console.log(JSON.stringify({ok:true,...b.store.publicState()}));}catch(e){console.log(JSON.stringify({error:e.message}))}});
process.on('SIGTERM',async()=>{await b.close();rmSync(dir,{recursive:true,force:true});process.exit(0)});
"""
server = subprocess.Popen(['node','--input-type=module','-e',fixture], cwd=ROOT, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
def read():
    assert select.select([server.stdout],[],[],15)[0], 'Fixture did not reply'
    result=json.loads(server.stdout.readline());assert 'error' not in result,result
    return result
def command(kind):
    server.stdin.write(json.dumps({'type':kind})+'\n');server.stdin.flush();return read()
try:
    ready=read()
    with tempfile.TemporaryDirectory(prefix='pdd-chrome-') as profile, sync_playwright() as p:
        ext=str(ROOT/'extension')
        ctx=p.chromium.launch_persistent_context(profile,executable_path='/usr/lib/chromium/chromium',headless=True,ignore_default_args=['--disable-extensions'],args=['--no-sandbox','--enable-unsafe-extension-debugging'])
        ctx.browser.new_browser_cdp_session().send('Extensions.loadUnpacked',{'path':ext})
        worker=ctx.service_workers[0] if ctx.service_workers else ctx.wait_for_event('serviceworker')
        extension_id=worker.url.split('/')[2]
        page=ctx.new_page();page.goto(ready['url']);page.locator('#extension').click();expect(page.locator('#code')).to_contain_text(__import__('re').compile(r'\d{9}'));code=page.locator('#code').inner_text()
        popup=ctx.new_page();popup.goto(f'chrome-extension://{extension_id}/popup.html');popup.locator('[name=url]').fill(ready['url']);popup.locator('[name=code]').fill(code);popup.locator('button').click();expect(popup.locator('#status')).to_contain_text('Estensione abbinata')
        target='https://www.dndbeyond.com/characters/123456'
        html='''<html><div class=ct-health-manager><label>Current Hit Points<input aria-label="Current Hit Points" value=20></label><div class=ct-health-manager__hp-max>30</div><label>Temporary Hit Points<input aria-label="Temporary Hit Points" value=5></label></div><script>window.sent=[];window.callback=null;window[Symbol.for('@dndbeyond/message-broker-lib')]={userId:'7',gameId:'42',subscribe:f=>window.callback=f,dispatch:m=>{window.sent.push(m);setTimeout(()=>window.callback(m),10)}};</script></html>'''
        # Fulfil this origin entirely locally. Never contact Beyond or authenticate.
        ctx.route('https://www.dndbeyond.com/**',lambda r:r.fulfill(status=200,body=html,content_type='text/html') if r.request.url==target else r.abort())
        beyond=ctx.new_page();beyond.goto(target)
        for _ in range(12):
            page.wait_for_timeout(500)
            try: command('approve');break
            except AssertionError:
                if _==11:raise
        command('roll');beyond.wait_for_function('window.sent.length===1',timeout=10000)
        for _ in range(12):
            state=command('heartbeat')
            if state['events'][0]['status']=='observed':break
            page.wait_for_timeout(500)
        assert state['events'][0]['status']=='observed',state
        result=beyond.evaluate('window.sent[0].data.rolls[0]');assert result['result']['total']==20 and [d['dieValue'] for d in result['diceNotation']['set'][0]['dice']]==[8,17]
        command('hp')
        for _ in range(20):
            page.wait_for_timeout(500);state=command('heartbeat')
            if state['events'][-1]['status']=='verified':break
        assert state['events'][-1]['status']=='verified',state
        assert beyond.locator('[aria-label="Current Hit Points"]').input_value()=='18'
        assert beyond.locator('[aria-label="Temporary Hit Points"]').input_value()=='0'
        # Same tab and a second tab must not dispatch a concluded event again.
        second=ctx.new_page();second.goto(target);page.wait_for_timeout(3500);command('heartbeat')
        assert beyond.evaluate('window.sent.length')==1 and second.evaluate('window.sent.length')==0
        receipts=worker.evaluate('async()=> (await chrome.storage.local.get("receipts")).receipts');assert len(receipts)==2 and all(not r['pending'] for r in receipts.values())
        ctx.close()
    print('PASS actual MV3 popup/pairing, service worker, content heartbeat, MAIN-world adapter, exact frozen dice, durable receipts, verified synthetic HP and two-tab deduplication. Synthetic origin/readback only; no authenticated Beyond or ESP32 proof.')
except Exception as error:
    if 'disabled by the administrator' in str(error):
        print('BLOCKED: this managed Chromium policy prohibits unpacked extensions. No loaded MV3 integration test passed; do not bypass the policy. Use worker API doubles and the page-adapter test separately.');sys.exit(2)
    raise
finally:
    server.terminate()
    try:server.wait(timeout=5)
    except subprocess.TimeoutExpired:server.kill();server.wait()
