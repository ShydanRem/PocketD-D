#!/usr/bin/env python3
"""Account page UI against synthetic routes and Chrome permission API doubles.

No account session, authenticated Beyond call or ESP32 access. This does not
replace a loaded MV3 permission/cookie test in the user's real browser.
"""
import base64, http.server, json, threading, time
from functools import partial
from pathlib import Path
from playwright.sync_api import sync_playwright, expect

ROOT=Path(__file__).resolve().parents[1]
class Handler(http.server.SimpleHTTPRequestHandler):
    def log_message(self,*args): pass
server=http.server.ThreadingHTTPServer(('127.0.0.1',0),partial(Handler,directory=str(ROOT/'extension')))
threading.Thread(target=server.serve_forever,daemon=True).start()
try:
    with sync_playwright() as p:
        browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
        context=browser.new_context(viewport={'width':900,'height':1000})
        context.add_init_script('''
          window.allowCheck=false;window.permissionCalls=0;window.removals=0;
          window.chrome={permissions:{request:async()=>{window.permissionCalls++;return window.allowCheck},remove:async()=>{window.removals++;return true}}};
          const original=window.fetch.bind(window);
          window.fetch=(url,options)=>window.holdCheck&&String(url).includes('auth-service.')
            ?new Promise((_,reject)=>options.signal.addEventListener('abort',()=>reject(options.signal.reason),{once:true}))
            :original(url,options);
        ''')
        calls=[];mode={'value':'ok'}
        def route(request):
            url=request.request.url
            calls.append(url)
            headers={'Content-Type':'application/json','Access-Control-Allow-Origin':f'http://127.0.0.1:{server.server_port}',
                     'Access-Control-Allow-Credentials':'true','Access-Control-Allow-Headers':'authorization,content-type',
                     'Access-Control-Allow-Methods':'POST,GET,OPTIONS'}
            if request.request.method=='OPTIONS':request.fulfill(status=204,headers=headers,body='');return
            if 'auth-service.' in url:
                assert request.request.method=='POST'
                payload={'http://schemas.xmlsoap.org/ws/2005/05/identity/claims/nameidentifier':'123456','exp':int(time.time())+120}
                encoded=base64.urlsafe_b64encode(json.dumps(payload).encode()).decode().rstrip('=')
                body={'token':'e30.'+encoded+'.synthetic_signature'}
            else:
                assert url=='https://character-service.dndbeyond.com/character/v5/characters/list?userId=123456'
                assert request.request.headers['authorization'].startswith('Bearer e30.')
                assert 'cookie' not in request.request.headers
                body={'success':True,'data':{'characters':[{'id':71,'name':'<img src=x onerror=alert(1)>','level':6,'raceName':'Elfo','classDescription':'Mago'}]}}
                if mode['value']=='partial':body['pagination']={'total':2}
                if mode['value']=='format':request.fulfill(status=200,headers={**headers,'Content-Type':'text/html'},body='<h1>Login</h1>');return
                if mode['value']=='outage':request.fulfill(status=503,headers=headers,body=json.dumps({'token':'synthetic secret never displayed'}));return
                if mode['value']=='empty':body['data']['characters']=[]
            request.fulfill(status=200,headers=headers,body=json.dumps(body))
        # Abort any other remote origin: the page must not contact a bridge or cloud service.
        local=f'http://127.0.0.1:{server.server_port}/'
        def bounded_route(request):
            if request.request.url.startswith(local):request.continue_()
            elif request.request.url.startswith(('https://auth-service.dndbeyond.com/','https://character-service.dndbeyond.com/')):route(request)
            else:raise AssertionError('Unexpected remote request')
        context.route('**/*',bounded_route)
        page=context.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
        page.goto(f'http://127.0.0.1:{server.server_port}/account.html')
        page.locator('#check').click();expect(page.locator('#status')).to_contain_text('Permessi non concessi');assert not calls
        page.evaluate('allowCheck=true');page.locator('#check').click()
        expect(page.locator('#status')).to_contain_text('Login ed elenco letti')
        assert page.locator('#checks dd').all_text_contents()==['OK']*6
        expect(page.locator('#characters')).to_contain_text('<img src=x onerror=alert(1)>');assert page.locator('#characters img').count()==0
        expect(page.locator('#completeness')).to_contain_text('completezza deve essere confermata')
        assert page.evaluate('localStorage.length')==0
        assert 'synthetic_signature' not in page.locator('body').inner_text() and '123456' not in page.locator('body').inner_text()
        for width in (900,390,320):
            page.set_viewport_size({'width':width,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
        page.locator('#clear').click();expect(page.locator('#catalog')).to_be_hidden();assert page.locator('#characters li').count()==0
        for value,expected in [('partial','Elenco parziale'),('empty','0 personaggi letti')]:
            mode['value']=value;page.locator('#check').click();expect(page.locator('#completeness')).to_contain_text(expected)
        for value,expected in [('format','Formato della risposta'),('outage','HTTP 503')]:
            mode['value']=value;page.locator('#check').click();expect(page.locator('#status')).to_contain_text(expected)
            expect(page.locator('#catalog')).to_be_hidden();assert page.locator('#characters li').count()==0
            assert 'synthetic secret' not in page.locator('body').inner_text()
        page.evaluate('holdCheck=true');page.locator('#check').click();expect(page.locator('#check')).to_be_disabled()
        page.locator('#clear').click();expect(page.locator('#status')).to_contain_text('Risultato cancellato')
        expect(page.locator('#check')).to_be_enabled();page.wait_for_timeout(100);expect(page.locator('#status')).to_contain_text('Risultato cancellato')
        page.locator('#revoke').click();expect(page.locator('#status')).to_contain_text('Permessi di verifica rimossi');assert page.evaluate('removals')==1
        assert not errors,errors
        browser.close()
    print('PASS account page: denied permissions/no requests, projected synthetic catalogue, 6 checks, XSS text rendering, empty/partial, HTTP-vs-format, clear/cancel, revoke, 900/390/320px. Chrome API doubles and synthetic HTTPS routes only; no actual login, MV3 permission/cookies, PG writes or ESP32.')
finally:server.shutdown();server.server_close()
