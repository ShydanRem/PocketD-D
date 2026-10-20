"""Synthetic PC support-flow check. Does not touch the user's browser collection."""
import json
import argparse
from pathlib import Path
from playwright.sync_api import sync_playwright
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--url',default='http://127.0.0.1:5173')
args=parser.parse_args()

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    page=browser.new_page(viewport={'width':1280,'height':900})
    errors=[]
    page.on('pageerror',lambda e:errors.append(str(e)))
    page.goto(args.url,wait_until='networkidle')
    # Seed this disposable browser only, by using the UI once to store the demo.
    page.locator('[data-hp="-1"]').click()
    page.locator('[data-action="edit"]').click()
    page.locator('[data-avatar-preset="1"]').click()
    page.locator('#hero-editor [type="submit"]').click()
    before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
    page.locator('[data-action="device-export"]').click()
    dialog=page.locator('dialog')
    assert dialog.is_visible()
    assert 'Prove con competenze' in dialog.inner_text()
    assert 'inventario' in dialog.inner_text()
    assert 'PF 23/32' in dialog.inner_text()
    assert 'Mantello vino' in dialog.locator('[data-device-avatar]').inner_text()
    assert dialog.locator('.device-avatar-sprite rect').count()>50
    dialog.locator('[data-device-training] summary').click()
    assert dialog.locator('.device-training-table tbody tr').count()==30
    assert 'Prova DES' in dialog.locator('.device-training-table').inner_text()
    with page.expect_download() as info:
        page.locator('[data-device-download]').click()
    download=info.value
    packet=json.loads(Path(download.path()).read_text())
    assert download.suggested_filename=='pocketdd-device.json'
    assert packet['hp']==23 and packet['abilities']['DES']==16 and packet['armorClass']==16
    assert packet['resources']==[{'name':'Slancio','current':1,'maximum':1}]
    assert packet['format']=='pocketdd-device' and packet['version']==3
    assert packet['training']['skillRanks']['perception']==0
    assert packet['avatar']=={'skin':1,'hair':1,'cloak':1,'style':1}
    assert 'inventory' not in packet and 'importInfo' not in packet
    assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
    assert not errors, errors
    browser.close()
print('PC support: preview, omissions, download, unchanged collection passed (synthetic demo)')
