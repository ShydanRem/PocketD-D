#!/usr/bin/env python3
"""Native production renderer; all characters and rolls are synthetic."""
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('dual113-setup','Due armi: una configurazione per PG'),('dual113-attack','Secondaria: conferma e azione bonus'),('dual113-bonus','Bonus: saldo e correzione esplicita'),('graze113-result','Scalfire: danno sul mancato, senza dadi')]
canvas=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(canvas)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'));x,y=i%2*466,i//2*506
 draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));canvas.paste(im,(x,y+40))
canvas.save(root/'docs/previews/v113-dual-graze.png');print('Dual/Graze preview: production LVGL, synthetic character and input only.')
