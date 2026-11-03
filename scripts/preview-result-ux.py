#!/usr/bin/env python3
"""Actual LVGL result/spell captures with synthetic data; no physical display claim."""
from pathlib import Path
from PIL import Image, ImageDraw
root=Path(__file__).resolve().parents[1]
out=root/'firmware/tests/out/ui-build'
frames=[('ux097-critical','Critico: risultato e prossima azione'),
        ('ux097-damage','Danni misti: totale sempre riconoscibile'),
        ('ux097-spell','Magia: valore, costo e comando'),
        ('ux097-heal','Cura: PF effettivamente recuperati')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466)
 im.save(out/(name+'.png'));x,y=i%2*466,i//2*506
 draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux097-miss','ux097-radiant','ux097-free','ux097-item'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v097-result-ux.png')
print('Native result UX: synthetic fixed rolls, real production renderer')
