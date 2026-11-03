#!/usr/bin/env python3
"""Production LVGL views with synthetic radio/account, never hardware evidence."""
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root / 'firmware/tests/out/ui-build'
frames = [('ux094-connect', 'Rete salvata e ripresa della richiesta'),
          ('ux094-retry', 'Riprova dello stesso PG, senza reinserirlo'),
          ('ux094-wifi', 'Configurazione Wi-Fi nel flusso Beyond'),
          ('ux094-relink', 'Accesso da ricollegare, distinto dalla rete')]
preview = Image.new('RGB', (932, 1012), (7, 21, 30))
draw = ImageDraw.Draw(preview)
for i, (name, title) in enumerate(frames):
    im = Image.open(out / (name + '.ppm'))
    assert im.size == (466, 466)
    im.save(out / (name + '.png'))
    x, y = i % 2 * 466, i // 2 * 506
    draw.text((x + 12, y + 12), title + ' | sintetico', fill=(239, 219, 172))
    preview.paste(im, (x, y + 40))
preview.save(root / 'docs/previews/v094-beyond-recovery.png')
print('Four production recovery views; synthetic account/network, no hardware claim')
