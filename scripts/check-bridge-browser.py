#!/usr/bin/env python3
"""Production page adapter and bridge UI on synthetic local pages. Never authenticates/writes Beyond."""
import json,subprocess,select
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
server=subprocess.Popen(['node','--input-type=module','-e',"""
import {createBridge} from './scripts/bridge-server.js';import {mkdtempSync,rmSync} from 'node:fs';import {tmpdir} from 'node:os';import {join} from 'node:path';
const dir=mkdtempSync(join(tmpdir(),'pdd-ui-bridge-')),b=createBridge({directory:dir,ip:'127.0.0.1',test:true,devicePort:0,adminPort:0});await b.listen();console.log('http://127.0.0.1:'+b.admin.address().port);process.on('SIGTERM',async()=>{await b.close();rmSync(dir,{recursive:true,force:true});process.exit(0)});
"""],cwd=ROOT,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
 assert select.select([server.stdout],[],[],10)[0];url=server.stdout.readline().strip();assert url.startswith('http://127.0.0.1:')
 with sync_playwright() as p:
  browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);ctx=browser.new_context(viewport={'width':1280,'height':1000});page=ctx.new_page();errors=[];page.on('pageerror',lambda e:errors.append(str(e)));page.goto(url);page.locator('#extension').click();expect(page.locator('#code')).to_contain_text(__import__('re').compile(r'\d{9}'));page.locator('[name=characterId]').fill('https://www.dndbeyond.com/characters/123456');page.locator('#bind button').click();expect(page.locator('#bundle')).not_to_have_value('');bundle=json.loads(page.locator('#bundle').input_value());assert bundle['version']==1 and len(bundle['token'])==64 and 'CERTIFICATE' in bundle['certificate'];assert 'PG' not in bundle
  for w in [320,390,1280]:
   page.set_viewport_size({'width':w,'height':1000});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
  # No remote request: this URL is fulfilled with an entirely synthetic page.
  target='https://www.dndbeyond.com/characters/123456';ctx.route(target,lambda r:r.fulfill(status=200,body='''<html><div class=ct-health-manager><label>Current Hit Points<input aria-label="Current Hit Points" value=20></label><div class=ct-health-manager__hp-max>30</div><label>Temporary Hit Points<input aria-label="Temporary Hit Points" value=5></label></div></html>''',content_type='text/html'));fake=ctx.new_page();fake.goto(target)
  source=(ROOT/'extension/page-adapter.js').read_text().replace('export async function pageAdapter','async function pageAdapter');fake.add_script_tag(content=source+';window.testAdapter=pageAdapter;');snapshot=fake.evaluate("testAdapter({type:'read'})");assert snapshot=={'characterId':'123456','vitals':{'hp':20,'max':30,'temp':5}}
  event={'id':'fixture-roll','remoteId':'00000000-0000-4000-8000-000000000001','characterId':'123456','scope':'user','kind':'roll','roll':{'label':'Tiro sintetico'}}
  roll={'diceNotation':{'constant':3,'set':[{'count':2,'dieType':'d20','operation':2,'dice':[{'dieType':'d20','dieValue':8},{'dieType':'d20','dieValue':17}]}]},'result':{'constant':3,'total':20,'values':[17]}}
  fake.evaluate("""()=>{window.sent=[];window.callback=null;window[Symbol.for('@dndbeyond/message-broker-lib')]={userId:'7',gameId:'42',subscribe:f=>window.callback=f,dispatch:m=>{window.sent.push(m);setTimeout(()=>window.callback(m),10)}};}""")
  result=fake.evaluate('(op)=>testAdapter(op)',{'type':'apply','event':event,'roll':roll});assert result['outcome']=='broker-observed';sent=fake.evaluate('window.sent');assert len(sent)==1 and sent[0]['data']['rolls'][0]['result']['total']==20
  fake.evaluate('(op)=>testAdapter(op)',{'type':'apply','event':event,'roll':roll});assert fake.evaluate('window.sent.length')==1
  op={'type':'apply','event':{'kind':'hp','characterId':'123456','before':snapshot['vitals'],'after':{'hp':18,'max':30,'temp':0}}};assert fake.evaluate('(op)=>testAdapter(op)',op)['outcome']=='dom-updated';assert fake.evaluate("testAdapter({type:'read'})")['vitals']==op['event']['after']
  assert fake.evaluate('(op)=>testAdapter(op)',op)['outcome']=='conflict';op['event']['before']=op['event']['after'];op['event']['after']={'hp':18,'max':31,'temp':0};assert fake.evaluate('(op)=>testAdapter(op)',op)['outcome']=='unsupported'
  fake.evaluate("document.querySelector('.ct-health-manager').remove()");assert fake.evaluate("testAdapter({type:'read'})") is None
  assert not errors,errors;browser.close()
 print('PASS bridge browser: local pairing/code/UI, mobile, actual page adapter on synthetic Beyond DOM/broker, exact frozen faces/result, single dispatch, HP/current/temp controls, conflict and unsupported layout/max. No authenticated Beyond request or physical ESP32.')
finally:
 server.terminate()
 try:server.wait(timeout=5)
 except subprocess.TimeoutExpired:server.kill();server.wait()
