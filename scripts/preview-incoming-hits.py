#!/usr/bin/env python3
"""Incoming-hit production captures: synthetic PGs, no hardware/account claim."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux102-types','Tipo di danno: scelta diretta'),('ux102-parts','Parti del colpo: icone e quantita'),('ux102-hit','Prima di confermare: danni, PF, costi'),('ux102-costs','Spese: saldi prima e dopo')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux102-reduce','ux102-sources'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v102-incoming-hits.png');print('Four native incoming-hit views, synthetic PG/input only')
