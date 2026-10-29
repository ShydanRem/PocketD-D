#!/usr/bin/env python3
"""Compiled PC preparation workflow; synthetic browser collection, no USB/Beyond IO."""
from pathlib import Path
from playwright.sync_api import sync_playwright,expect
ROOT=Path(__file__).resolve().parents[1]
with sync_playwright() as p:
    browser=p.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox'])
    page=browser.new_page(viewport={'width':1280,'height':1000});errors=[]
    page.on('pageerror',lambda e:errors.append(str(e)))
    page.route('**/*',lambda r:r.fulfill(status=200,content_type='text/html',body=(ROOT/'PocketDD-demo.html').read_text()) if r.request.is_navigation_request() else r.abort())
    page.goto('http://pocket.test');page.locator('[data-action=beyond-import]').wait_for()
    before=page.evaluate('localStorage.getItem("pocketdd-library-v1")')
    tools=page.locator('.support-tools');expect(tools.locator('h3')).to_have_text(['1 · Importa e personalizza','2 · Verifica e trasferisci','3 · Conserva e collega'])
    for action in ['beyond-import','edit','manage-inventory','manage-powers','device-export','device-backup','export','import']:
        expect(tools.locator(f'[data-action="{action}"]')).to_have_count(1)
    for width in [1280,390,320]:
        page.set_viewport_size({'width':width,'height':1000});tools.scroll_into_view_if_needed()
        assert page.evaluate('document.documentElement.scrollWidth<=innerWidth')
        expect(tools.locator('[data-action=device-export]')).to_be_visible()
        expect(tools.locator('[data-action=party]')).not_to_be_visible()
    secondary=tools.locator(':scope > details');secondary.locator('summary').click();expect(secondary.locator('[data-action=party]')).to_be_visible();secondary.locator('summary').click()
    bridge=tools.locator('details').filter(has=page.locator('summary',has_text='Ponte Beyond sperimentale'));bridge.locator('summary').click();expect(bridge.locator('a')).to_have_attribute('href','http://127.0.0.1:8787/')
    page.locator('[data-action=beyond-import]').click();expect(page.locator('.beyond-import')).to_be_visible()
    assert page.evaluate('localStorage.getItem("pocketdd-library-v1")')==before
    assert not errors,errors
    browser.close()
print('PASS PC preparation: import/check/transfer/backup actions unique and visible, secondary prototypes folded, bridge optional, mobile widths, import opens without modifying the synthetic collection. No USB or Beyond request.')
