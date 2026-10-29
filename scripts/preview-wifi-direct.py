#!/usr/bin/env python3
"""Export real LVGL host captures with disposable network names and masked passwords."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('wifi-scan','Ricerca reti'),('wifi-networks','Scegli la rete'),('wifi-password-filled','Password nascosta'),('wifi-join-review','Prova e salva'),('wifi-saved','Conferma del salvataggio'),('wifi-join-failed','Connessione non riuscita')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 image=Image.open(out/(name+'.ppm'));assert image.size==(466,466);image.save(out/(name+'.png'))
 x=i%3*466;y=i//3*506;draw.text((x+18,y+12),title+' | prova host',fill=(239,219,172));preview.paste(image,(x,y+40))
target=ROOT/'docs/previews/v084-wifi-direct.png';preview.save(target)
print('Wi-Fi direct: real renderer, synthetic networks, masked secret; physical RF not tested')
