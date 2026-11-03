#!/usr/bin/env python3
"""Native firmware captures: synthetic PG, never a hardware photograph."""
from pathlib import Path
from PIL import Image, ImageDraw
root = Path(__file__).resolve().parents[1]
out = root / 'firmware/tests/out/ui-build'
frames = [('ux096-home', 'Home: avatar e azioni'),
          ('ux096-weapon-spell', 'Arma e magia nello stesso comando'),
          ('ux096-vitals', 'Vitalita: azioni principali'),
          ('ux096-inventory', 'Zaino: nome e stato')]
preview = Image.new('RGB', (932, 1012), (7, 21, 30))
draw = ImageDraw.Draw(preview)
for i, (name, title) in enumerate(frames):
    im = Image.open(out / (name + '.ppm'))
    assert im.size == (466, 466)
    im.save(out / (name + '.png'))
    x, y = i % 2 * 466, i // 2 * 506
    draw.text((x + 12, y + 12), title + ' | sintetico', fill=(239, 219, 172))
    preview.paste(im, (x, y + 40))
for name in ('ux096-attack', 'ux096-spells', 'ux096-corrections'):
    Image.open(out / (name + '.ppm')).save(out / (name + '.png'))
preview.save(root / 'docs/previews/v096-rpg-ux.png')
print('Native RPG UX: four production captures, synthetic PG, no hardware claim')
