#!/usr/bin/env python3
"""Actual LVGL production views, synthetic native PGs; no hardware claim."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux090-library','Raccolta locale sulla SD'),('ux090-open-pg','Anteprima prima del cambio'),('ux090-no-sd','Scheda singola senza SD'),('ux090-reboot-required','Commit incerto: copie conservate')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x=i%2*466;y=i//2*506;draw.text((x+16,y+12),title+' | dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(ROOT/'docs/previews/v090-local-library.png');print('Four actual firmware views exported; synthetic, no hardware claim')
