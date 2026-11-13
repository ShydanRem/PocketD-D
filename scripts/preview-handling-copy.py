#!/usr/bin/env python3
"""Actual LVGL frames with synthetic data; no physical hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('handling112-ready','Impugnatura rapida e munizioni salvate'),('handling112-shield','Scudo: modifica esplicita con nuova CA'),('source112-library','Stesso PG: confronta prima di importare'),('handling112-ammo','Ogni arma conserva la propria pila')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v112-handling-copy.png')
print('Handling/copy preview: four production LVGL views, synthetic data.')
