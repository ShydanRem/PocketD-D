#!/usr/bin/env python3
"""Built PC UI: synthetic collection only; no Beyond, USB or personal storage."""
import json,subprocess
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
raw=subprocess.check_output(['node','--input-type=module','-e',"""
import {initialState} from './src/model.js';import {emptyExtended} from './src/extended.js';import {emptyGear} from './src/equipment.js';import {createLibrary,addCharacter,selectCharacter,serializeLibrary} from './src/library.js';
const s=initialState();s.name='Synthetic session';s.extended=emptyExtended();s.level=6;s.hp=20;s.maxHp=40;s.armorClass=16;s.abilities.DES=10;s.inventory.push({id:'chain',name:'Chain Mail',notes:'',type:'gear',quantity:1});s.extended.loadout={unarmored:1,items:[{...emptyGear('chain'),kind:4,armor:16,equipped:true}]};s.inventory.push({id:'relic',name:'Anello di prova',notes:'',type:'gear',quantity:1});s.extended.loadout.items.push({...emptyGear('relic'),equipped:true,attunement:1,ac:1,saves:1});s.powers=[{id:'ts',name:'True Strike',description:'Synthetic note',type:'spell',level:0,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:false,effect:'true-strike-2024'}];
let l=createLibrary(s,'subject');l=addCharacter(l,{...initialState(),name:'Preserved synthetic'},'preserved');console.log(serializeLibrary(selectCharacter(l,'subject')));
"""],cwd=ROOT,text=True)
with sync_playwright() as p:
 browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);page=browser.new_page(viewport={'width':1440,'height':1050});errors=[];page.on('pageerror',lambda e:errors.append(str(e)))
 page.route('**/*',lambda r:r.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if r.request.is_navigation_request() else r.abort())
 page.goto('http://pocket.test');page.evaluate('raw=>localStorage.setItem("pocketdd-library-v1",raw)',raw);page.reload()
 def library():return json.loads(page.evaluate('localStorage.getItem("pocketdd-library-v1")'))
 def active():return next(c['state'] for c in library()['characters'] if c['id']=='subject')
 other=next(c for c in library()['characters'] if c['id']=='preserved')
 page.locator('[data-go-page=pack]').click();expect(page.locator('[data-equip-gear=chain]')).to_have_text('Riponi');assert page.locator('.inventory-card .semantic-icon').count()==5
 page.locator('[data-equip-gear=chain]').click();assert active()['armorClass']==10 and active()['hp']==20 and active()['maxHp']==40
 page.locator('[data-equip-gear=chain]').click();assert active()['armorClass']==16
 page.locator('[data-attune-gear=relic]').click();assert active()['armorClass']==17 and active()['hp']==20;expect(page.locator('[data-attune-gear=relic]')).to_have_text('Termina sintonia');page.locator('[data-attune-gear=relic]').click();assert active()['armorClass']==16
 page.locator('[data-go-page=actions]').click();page.locator('[data-action-tab=powers]').click();expect(page.locator('[data-use-power=ts]')).to_be_disabled();expect(page.locator('.power-card')).to_contain_text('Attacco con arma e CAR');assert page.locator('.power-card .semantic-icon').count()==1
 for width in [1440,390,320]:
  page.set_viewport_size({'width':width,'height':1050});assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
 page.reload();assert active()['armorClass']==16;assert next(c for c in library()['characters'] if c['id']=='preserved')==other;assert not errors,errors
 browser.close()
print('PASS built PC UI: independent equip/attune gates change AC without healing, semantic SVG icons, True Strike routes clearly to Pocket, reload/other character preserved and desktop/mobile bounds; synthetic browser storage only.')
