#!/usr/bin/env python3
"""Native collection/setup captures: synthetic PGs, no account or hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux100-library','Raccolta: identita e copia attuale'),('ux100-open-pg','Cambio PG: avatar e valori'),('ux100-scene','Aspetto: bozza e scelta diretta'),('ux100-settings','Impostazioni: comandi visivi')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux100-fields','ux100-avatar','ux100-account','ux100-import','ux100-long-preview'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v100-collection-setup.png');print('Four native collection/setup views, synthetic data only')
