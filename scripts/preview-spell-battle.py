#!/usr/bin/env python3
"""Real LVGL captures, synthetic spell combat only; no hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('battle110-method','Sei azioni contestuali'),('battle110-critical','Attacco magia: critico'),('battle110-target','TS del bersaglio dichiarato'),('battle110-half','Meta del totale per difetto')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
    im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
    x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v110-spell-battle.png')
print('Spell battle preview: four real LVGL views, synthetic data.')
