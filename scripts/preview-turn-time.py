#!/usr/bin/env python3
"""Native explicit turn/time controls: synthetic data/input, no hardware clock claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux103-turns','Turni: azioni e reazione'),('ux103-subjects','Altri turni: riferimenti con nome'),('ux103-time','Tempo: quantita rapide e minuti'),('ux103-expiry','Scadenze: conseguenze prima del salvataggio')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux103-other-preview','ux103-turn-details','ux103-start-preview','ux103-reaction','ux103-time-preview','ux103-time-details'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v103-turn-time.png');print('Four native explicit turn/time views, synthetic data/input only')
