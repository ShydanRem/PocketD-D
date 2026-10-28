#!/usr/bin/env python3
"""Package actual LVGL customization captures. Synthetic PG; no hardware photograph."""
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'firmware/tests/out/ui-build'
names=['home-camp','home-ruins','home-port','editor-hair','scene-port','confirm']
gallery=Image.new('RGB',(490*3,490*2),'#07151e')
for i,name in enumerate(names):
 p=BASE/f'appearance-{name}.ppm';assert p.exists(),p
 im=Image.open(p).convert('RGB');assert im.size==(466,466);im.save(p.with_suffix('.png'))
 gallery.paste(im,(12+(i%3)*490,12+(i//3)*490))
for name in ['editor-skin','scene-ruins','save-error','undo-keeps-look','restored']:
 p=BASE/f'appearance-{name}.ppm';assert p.exists(),p;Image.open(p).save(p.with_suffix('.png'))
gallery.save(BASE/'appearance-preview.png')
print('Appearance gallery: six real LVGL host frames plus error/undo/restore captures; synthetic data, no hardware capture')

for path in BASE.glob('species-*.ppm'):Image.open(path).save(path.with_suffix('.png'))

for path in BASE.glob('class-*.ppm'):Image.open(path).save(path.with_suffix('.png'))

for path in BASE.glob('beyond-defences-*.ppm'):Image.open(path).save(path.with_suffix('.png'))

for path in BASE.glob('skill-ability-*.ppm'):Image.open(path).save(path.with_suffix('.png'))
