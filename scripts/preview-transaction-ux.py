#!/usr/bin/env python3
"""Production LVGL transaction views, synthetic PG; no physical hardware claim."""
from pathlib import Path
from PIL import Image, ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux098-backpack','Zaino: comando rapido per la dotazione'),
        ('ux098-equip','Equipaggiamento: confronto prima/dopo'),
        ('ux098-cast','Magia: costi e concentrazione'),
        ('ux098-rest','Riposo: recuperi e perdita dei bonus')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux098-equip-details','ux098-rest-policy','ux098-rest-details','ux098-cast-details','ux098-consume'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v098-transaction-ux.png')
print('Four native transaction views, synthetic state and simulated input')
