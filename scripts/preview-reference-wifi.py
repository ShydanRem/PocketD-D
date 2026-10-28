#!/usr/bin/env python3
"""Package captures from the production LVGL renderer, with synthetic data only."""
from pathlib import Path
from PIL import Image,ImageDraw,ImageFont
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
names=['reference-list','reference-movement','reference-initiative','reference-initiative-result','wifi-offline','wifi-provisioning','wifi-connected','wifi-forget']
for name in names:
 image=Image.open(out/(name+'.ppm'));assert image.size==(466,466);image.save(out/(name+'.png'))
preview=Image.new('RGB',(3*466,2*506),(7,21,30));draw=ImageDraw.Draw(preview)
for i,name in enumerate(['reference-list','reference-movement','reference-initiative','reference-initiative-result','wifi-provisioning','wifi-connected']):
 x=(i%3)*466;y=(i//3)*506;draw.text((x+18,y+10),name+' (synthetic)',fill=(239,219,172));preview.paste(Image.open(out/(name+'.png')),(x,y+40))
target=ROOT/'docs/previews/v078-reference-wifi.png';target.parent.mkdir(exist_ok=True);preview.save(target)
print('Native screenshots exported: synthetic character and Wi-Fi credentials only; no physical hardware claim')
