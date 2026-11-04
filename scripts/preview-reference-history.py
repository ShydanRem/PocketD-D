#!/usr/bin/env python3
"""Native reference/history UI captures; synthetic state and input only."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux104-reference','Profilo: consulta o cerca'),('ux104-filtered-history','Registro: filtri e risultati'),('ux104-roll','Un risultato, le facce originali'),('ux104-pending-detail','Tiro da registrare: riprova')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux104-search','ux104-filtered-profile','ux104-initiative','ux104-history','ux104-wide-roll','ux104-pending','ux104-discard'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v104-reference-history.png');print('Four native reference/history views, synthetic data/input only')
