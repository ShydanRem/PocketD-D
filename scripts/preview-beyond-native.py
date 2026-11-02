#!/usr/bin/env python3
"""Production LVGL captures with synthetic PGs; not physical ESP32 evidence."""
from pathlib import Path
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[1]
out = ROOT / 'firmware/tests/out/ui-build'
frames = [('ux091-beyond-form', 'Beyond sul Pocket: ID pubblico'),
          ('ux091-beyond-busy', 'Lettura e conversione locali'),
          ('ux091-beyond-preview', 'Anteprima prima del salvataggio'),
          ('ux091-beyond-blocked', 'Problemi espliciti, niente import forzato')]
preview = Image.new('RGB', (932, 1012), (7, 21, 30))
draw = ImageDraw.Draw(preview)
for i, (name, title) in enumerate(frames):
    im = Image.open(out / (name + '.ppm'))
    assert im.size == (466, 466)
    im.save(out / (name + '.png'))
    x, y = i % 2 * 466, i // 2 * 506
    draw.text((x + 12, y + 12), title + ' | dati sintetici', fill=(239, 219, 172))
    preview.paste(im, (x, y + 40))
preview.save(ROOT / 'docs/previews/v091-beyond-native.png')
print('Four production firmware views exported; synthetic PGs, no hardware claim')
