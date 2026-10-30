#!/usr/bin/env python3
"""Export actual firmware renderer captures; synthetic session, no generated UI."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux085-weapon-profile','Profilo arma'),('ux085-spell-list','Magie e disponibilita'),('ux085-inventory','Icone dello zaino'),('ux085-true-strike','Attacco con CAR'),('ux085-true-strike-damage','Arma + extra radiante'),('ux085-armor-off','Riponi armatura')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x=i%3*466;y=i//3*506;draw.text((x+16,y+12),title+' | renderer host, dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
target=ROOT/'docs/previews/v085-device-session.png';target.parent.mkdir(exist_ok=True);preview.save(target)
print('v0.85 preview: six actual native views, synthetic data, no physical hardware claim')
