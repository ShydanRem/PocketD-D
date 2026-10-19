#!/usr/bin/env python3
"""Export actual host-LVGL firmware frames as PNGs and a four-screen contact sheet.
Requires Pillow. Run npm run firmware:test first. This is not a hardware capture.
"""
from pathlib import Path
from PIL import Image

base = Path(__file__).resolve().parents[1]/'firmware/tests/out/ui-build'
names = ['hero', 'dice', 'sheet', 'resources']
gallery = Image.new('RGB', (466*2, 466*2), 'black')
for i, name in enumerate(names):
    source = base/(name+'.ppm')
    if not source.is_file():
        raise SystemExit('Run npm run firmware:test first')
    frame = Image.open(source).convert('RGB')
    frame.save(base/(name+'.png'))
    gallery.paste(frame, ((i % 2)*466, (i // 2)*466))
output = base/'device-preview.png'
gallery.save(output)
print(output)
