#!/usr/bin/env python3
"""Capture the real LVGL spell-roll flow with synthetic character data."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1]
out=root/'firmware/tests/out/ui-build'
frames=[('spell108-method','Scegli come risolvere la magia'),('spell108-formula','Formula esplicita per questo lancio'),('spell108-result','Dadi fissi e PF effettivamente recuperati'),('spell108-receipt','Costo e risultato gia salvati')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
    im=Image.open(out/(name+'.ppm'));assert im.size==(466,466)
    im.save(out/(name+'.png'));x,y=i%2*466,i//2*506
    draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(root/'docs/previews/v108-spell-roll.png')
print('Spell-roll preview: four real LVGL views, synthetic data; no hardware claim.')
