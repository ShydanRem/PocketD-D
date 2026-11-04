#!/usr/bin/env python3
"""Native network/backup UI: synthetic state, radio/USB doubles, no credentials."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux105-network','Rete: le scelte subito disponibili'),('ux105-bridge-uncertain','Ponte: esito ancora da verificare'),('ux105-restore','Ripristino: prima e dopo'),('ux105-restore-details','Contenuto e ambito della copia')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux105-network-details','ux105-network-error','ux105-bridge-off','ux105-bridge-details','ux105-hotspot','ux105-pair-wide','ux105-backup-home','ux105-backup-send','ux105-backup-details','ux105-restore-wide'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v105-connections-backup.png');print('Four native connection/backup views, synthetic state/input only')
