#!/usr/bin/env python3
"""Production LVGL captures; synthetic characters, dice and hardware inputs."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('extra114-nick','Nick: Light dentro l azione Attacco'),('extra114-profile','Talento 2024: configurazione esplicita'),('extra114-cleave','Cleave: altro bersaglio, costo separato'),('extra114-topple','Topple: CD vera, esito dichiarato dal DM')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x,y=i%2*466,i//2*506
 draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v114-extra-masteries.png');print('Extra/mastery gallery: four native production views, synthetic inputs only.')
