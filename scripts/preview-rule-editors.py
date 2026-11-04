#!/usr/bin/env python3
"""Native draft editor captures, synthetic state and input; no hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux101-effect','Effetti: scegli il valore da modificare'),('ux101-defences','Difese: proprieta e stato distinti'),('ux101-timer-picker','Timer: sei scelte dirette'),('ux101-hours','Durata: valori rapidi e precisione')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux101-extra','ux101-rules','ux101-defence-confirm','ux101-defence-details','ux101-profile'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v101-rule-editors.png');print('Four native draft editor views, synthetic state/input only')
