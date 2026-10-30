#!/usr/bin/env python3
"""Actual native firmware views; synthetic characters, no personal JSON."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux089-attack-options','Punizione disponibile dopo il colpo'),('ux089-smite-cost','Scelta dello slot'),('ux089-smite-combo','Bersaglio e altro extra'),('ux089-save-retry','Risultato congelato: salva senza rilanci'),('ux089-total','Totale con arma, True Strike e Hex'),('ux089-large-smite','Critico: secondo gruppo di d8')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x=i%3*466;y=i//3*506;draw.text((x+16,y+12),title+' | dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(ROOT/'docs/previews/v089-divine-smite.png');print('Six actual firmware views exported; synthetic, no hardware claim')
