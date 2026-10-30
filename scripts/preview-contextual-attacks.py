#!/usr/bin/env python3
"""Actual native UI captures, synthetic characters, no personal source data."""
from pathlib import Path
from PIL import Image,ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux087-attack-choices','Attacco normale o magia'),('ux087-normal','Attacco ordinario'),('ux087-weapon-spell','Variante con caratteristica magica'),('ux087-prepare','Preparazione senza lasciare il flusso'),('ux087-raging','Motivo del blocco visibile'),('ux087-manual','Regole da configurare esplicitamente')]
preview=Image.new('RGB',(1398,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x=i%3*466;y=i//3*506;draw.text((x+16,y+12),title+' | dati sintetici',fill=(239,219,172));preview.paste(im,(x,y+40))
preview.save(ROOT/'docs/previews/v087-contextual-attacks.png');print('Six production firmware captures exported; synthetic, no real character data')
