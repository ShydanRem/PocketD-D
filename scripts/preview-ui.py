#!/usr/bin/env python3
"""Package every actual firmware view captured by the host LVGL regression suite.
No artwork/concept is substituted for a missing capture. Requires Pillow.
"""
import html
import json
from pathlib import Path
import re
from PIL import Image

ROOT=Path(__file__).resolve().parents[1]
BASE=ROOT/'firmware/tests/out/ui-build'
source=(ROOT/'firmware/main/main.c').read_text()
names=[x.strip() for x in re.search(r'typedef enum \{([^}]+)\} page_t;',source).group(1).split(',')]
themes=dict(re.findall(r'\[(\w+)\]=PD_THEME_(\w+)',source))
records=[]
for i,name in enumerate(names):
    path=BASE/f'ux-page-{i:02d}.ppm'
    if not path.is_file():raise SystemExit(f'Missing production capture: {name}')
    Image.open(path).save(path.with_suffix('.png'))
    records.append(dict(page=i,name=name,theme=themes[name],image=path.with_suffix('.png').name))
manifest=dict(version='0.50.0',hardware_capture=False,synthetic_character=True,views=records)
(BASE/'ui-views.json').write_text(json.dumps(manifest,indent=2)+'\n')
cards=''.join(f'<figure><a href="{r["image"]}"><img loading="lazy" src="{r["image"]}" width="466" height="466" alt="{html.escape(r["name"])}"></a><figcaption>{r["page"]:02d} · {html.escape(r["name"])} · {r["theme"]}</figcaption></figure>' for r in records)
(BASE/'ui-catalog.html').write_text('''<!doctype html><html lang="it"><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>PocketD-D — verifica delle viste v0.50</title><style>body{background:#081c27;color:#ffefd2;font:16px system-ui;margin:24px}main{display:grid;grid-template-columns:repeat(auto-fit,minmax(240px,1fr));gap:16px}figure{margin:0;background:#102631;padding:8px;border-radius:12px}img{display:block;width:100%;height:auto}figcaption{padding:8px;overflow-wrap:anywhere}a{color:inherit}</style><h1>PocketD-D v0.50 — 88 viste</h1><p>Catture del codice firmware LVGL eseguito su PC con dati sintetici. Non sono fotografie del dispositivo. Aprire un'immagine per la risoluzione originale.</p><main>'''+cards+'</main></html>')
gallery=Image.new('RGB',(466*3,466*2),'black')
for i,name in enumerate(('inventory','magic-list','rest-home','sheet','screen-settings','item-confirm')):
    frame=Image.open(BASE/f'{name}.ppm').convert('RGB')
    gallery.paste(frame,(i%3*466,i//3*466))
gallery.save(BASE/'ui-overview.png')
all_views=Image.new('RGB',(233*8,233*11),'black')
for i,r in enumerate(records):
    frame=Image.open(BASE/r['image']).convert('RGB').resize((233,233),Image.Resampling.NEAREST)
    all_views.paste(frame,(i%8*233,i//8*233))
all_views.save(BASE/'ui-all-views.png')
frames=[Image.open(BASE/f'ux-transition-{i:02d}.ppm').convert('RGB') for i in range(5)]
frames[0].save(BASE/'ui-transition.gif',save_all=True,append_images=frames[1:],duration=[40,40,40,40,1000],loop=0,disposal=2,optimize=True)
print(f'UI: {len(records)} production views, overview, offline catalog and transition exported')
