#!/usr/bin/env python3
"""Export actual firmware renderer captures; synthetic, no personal JSON."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux088-context-spells','Effetti dalla scelta dell attacco'),('ux088-target','Conferma il bersaglio del colpo'),('ux088-total','Totale delle parti'),('ux088-necrotic','Extra di Hex congelato'),('ux088-force','Extra del Marchio: forza'),('ux088-pending','Storico: riprova senza rilanciare')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x=i%3*466;y=i//3*506;draw.text((x+16,y+12),title+' | dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(ROOT/'docs/previews/v088-on-hit-effects.png');print('Six actual firmware views exported; synthetic data, no physical hardware claim')
