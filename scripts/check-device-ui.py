"""Synthetic PC support-flow check. Does not touch the user's browser collection."""
import json
from pathlib import Path
from playwright.sync_api import sync_playwright

with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    page=browser.new_page(viewport={'width':1280,'height':900})
    errors=[]
    page.on('pageerror',lambda e:errors.append(str(e)))
    page.goto('http://127.0.0.1:5173',wait_until='networkidle')
    # Seed this disposable browser only, by using the UI once to store the demo.
    page.locator('[data-hp="-1"]').click()
    before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
    page.locator('[data-action="device-export"]').click()
    dialog=page.locator('dialog')
    assert dialog.is_visible()
    assert 'non applicano competenze' in dialog.inner_text()
    assert 'inventario' in dialog.inner_text()
    assert 'PF 23/32' in dialog.inner_text()
    with page.expect_download() as info:
        page.locator('[data-device-download]').click()
    download=info.value
    packet=json.loads(Path(download.path()).read_text())
    assert download.suggested_filename=='pocketdd-device.json'
    assert packet['hp']==23 and packet['abilities']['DES']==16 and packet['armorClass']==16
    assert packet['resources']==[{'name':'Slancio','current':1,'maximum':1}]
    assert packet['format']=='pocketdd-device' and packet['version']==1
    assert 'inventory' not in packet and 'importInfo' not in packet
    assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
    assert not errors, errors
    browser.close()
print('PC support: preview, omissions, download, unchanged collection passed (synthetic demo)')
