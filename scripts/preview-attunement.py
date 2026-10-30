#!/usr/bin/env python3
"""Actual native UI, synthetic item; no generated mockup or personal sheet."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux086-attunement-inactive','Equipaggiato, bonus inattivi'),('ux086-attunement-preview','Sintonia: anteprima dei valori'),('ux086-attuned','Oggetto sintonizzato'),('ux086-attuned-unworn','Riposto, sintonia conservata'),('ux086-end-attunement','Fine sintonia e limite PF'),('ux086-equipment-icons','Icone semantiche condivise')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x=i%3*466;y=i//3*506;draw.text((x+16,y+12),title+' | dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(ROOT/'docs/previews/v086-attunement.png');print('Six production firmware captures exported; synthetic, independent of any real character')
