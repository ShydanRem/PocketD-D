#!/usr/bin/env python3
"""Native firmware captures with synthetic classes/conditions/timers, no hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux099-rest','Riposo: scegli direttamente la riserva'),('ux099-status','Stato: comandi visivi coerenti'),('ux099-effects','Effetti: valore, attivita e durata'),('ux099-expiry','Scadenza: conseguenze prima di confermare')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux099-hit-confirm','ux099-notes','ux099-roll-states','ux099-effect-confirm'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v099-session-states.png');print('Four native session/state views, synthetic data only')
