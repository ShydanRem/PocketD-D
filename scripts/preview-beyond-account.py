#!/usr/bin/env python3
"""Actual firmware UI; synthetic account and codes, no hardware claim."""
from pathlib import Path
from PIL import Image, ImageDraw
ROOT=Path(__file__).resolve().parents[1];out=ROOT/'firmware/tests/out/ui-build'
frames=[('ux092-pair','Conferma del codice e memoria opzionale'),('ux092-account','Elenco account sul Pocket'),('ux092-forget','Scollegamento separato dai PG')]
preview=Image.new('RGB',(1398,506),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));draw.text((466*i+12,12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(466*i,40))
preview.save(ROOT/'docs/previews/v092-beyond-account.png')
print('Three production account views; synthetic PG/session/code only')
