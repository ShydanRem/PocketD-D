#!/usr/bin/env python3
"""Production LVGL ammunition flow with synthetic characters, no hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ammo111-picker','Scegli la pila: unita per attacco'),('ammo111-ready','Arma e munizioni nello stesso flusso'),('ammo111-hit','Costo confermato insieme al d20'),('ammo111-empty','Ultima munizione: scegli come proseguire')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
    im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
    x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v111-ammunition.png')
print('Ammunition preview: four real LVGL views, synthetic data.')
