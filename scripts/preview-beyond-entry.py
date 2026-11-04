#!/usr/bin/env python3
"""Native Beyond entry: synthetic account, worker, state and input; no live access."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux106-choices','Account o ID pubblico'),('ux106-code','Codice e scelta della memoria'),('ux106-http-blocked','HTTP 200, conversione bloccata'),('ux106-preview','Valori prima della conferma')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux106-first-import','ux106-guide','ux106-relink-guide','ux106-temporary','ux106-catalog','ux106-converting','ux106-preview-wide','ux106-report-wide'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v106-beyond-entry.png');print('Four native Beyond-entry views, synthetic state/input only')
