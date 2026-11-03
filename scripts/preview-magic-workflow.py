#!/usr/bin/env python3
"""Production firmware views, synthetic spells/resources; no hardware claim."""
from pathlib import Path
from PIL import Image, ImageDraw

root = Path(__file__).resolve().parents[1]
out = root / 'firmware/tests/out/ui-build'
frames = [('ux095-preparation', 'Prepara piu magie in una sola operazione'),
          ('ux095-preparation-confirm', 'Riepilogo e unico salvataggio'),
          ('ux095-slot-pact', 'Slot patto: scelta diretta del livello'),
          ('ux095-slot-ordinary', 'Slot ordinari: saldi e livelli utilizzabili')]
preview = Image.new('RGB', (932, 1012), (7, 21, 30))
draw = ImageDraw.Draw(preview)
for i, (name, title) in enumerate(frames):
    im = Image.open(out / (name + '.ppm'))
    assert im.size == (466, 466)
    im.save(out / (name + '.png'))
    x, y = i % 2 * 466, i // 2 * 506
    draw.text((x + 12, y + 12), title + ' | sintetico', fill=(239, 219, 172))
    preview.paste(im, (x, y + 40))
preview.save(root / 'docs/previews/v095-magic-workflow.png')
print('Four production magic workflow views, synthetic spells only')
