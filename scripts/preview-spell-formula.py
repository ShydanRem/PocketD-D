#!/usr/bin/env python3
"""Real LVGL captures, synthetic saved formula and character only."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('profile109-editor','Formula salvata con la magia'),('profile109-scaling','Aumento esplicito dei dadi'),('profile109-ready','Formula risolta con slot patto 3'),('profile109-result','Un risultato, costo e cura insieme')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
    im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
    x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v109-spell-formulas.png')
print('Saved spell formula preview: four real LVGL views, synthetic data, no hardware claim.')
