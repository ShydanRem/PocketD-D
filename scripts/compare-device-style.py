#!/usr/bin/env python3
"""Compare current host-LVGL captures with the approved v0.45 release.
Requires Pillow 10.1+ and fresh firmware:test captures; never opens character data.
"""
import argparse
import io
import json
from pathlib import Path
import zipfile
from PIL import Image, ImageDraw, ImageFont

root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('baseline_release',type=Path)
parser.add_argument('--output',type=Path,default=root/'firmware/tests/out/ui-build/pixel-polish-comparison.png')
args=parser.parse_args()
approved_commit='4afcf0b5cbbd99cd138a903c50aa6e484b010bbb'
current=json.loads((root/'package.json').read_text())['version']
frames=root/'firmware/tests/out/ui-build'
canvas=Image.new('RGB',(932,972),'#0b101c')
draw=ImageDraw.Draw(canvas)
font=ImageFont.load_default(size=18)
draw.text((233,10),'v0.45 - base approvata',font=font,fill='#a5b5c6',anchor='mt')
draw.text((699,10),f'v{current} - affinamento',font=font,fill='#f2cc81',anchor='mt')
with zipfile.ZipFile(args.baseline_release) as archive:
    infos=[name for name in archive.namelist() if name.endswith('/firmware-binaries/release-info.json')]
    if len(infos)!=1:raise SystemExit('Missing unique baseline release metadata')
    info=json.loads(archive.read(infos[0]))
    if info.get('version')!='0.45.0' or info.get('commit')!=approved_commit:
        raise SystemExit('Baseline must be the approved v0.45.0 commit 4afcf0b')
    prefix=infos[0].removesuffix('firmware-binaries/release-info.json')
    for row,name in enumerate(['hero','appearance']):
        old=Image.open(io.BytesIO(archive.read(prefix+'firmware-host-preview/'+name+'.png'))).convert('RGB')
        new=Image.open(frames/(name+'.ppm')).convert('RGB')
        if old.size!=(466,466) or new.size!=(466,466):raise SystemExit('Expected native 466x466 frames')
        canvas.paste(old,(0,40+row*466));canvas.paste(new,(466,40+row*466))
args.output.parent.mkdir(parents=True,exist_ok=True)
canvas.save(args.output)
print(args.output)
