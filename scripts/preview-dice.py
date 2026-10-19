#!/usr/bin/env python3
"""Encode actual host-LVGL test frames as a circular GIF. Requires Pillow.
No board capture: this is a visual preview of the compiled C UI on the host.
Run npm run firmware:test first. Does not alter any game state or source assets.
"""
import argparse
from pathlib import Path
from PIL import Image

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--pool-only',action='store_true',help='Preview only the multi-die throw')
args=parser.parse_args()
root=Path(__file__).resolve().parents[1]/'firmware/tests/out/ui-build'
frames=[]
patterns=['pool-frame-*.ppm'] if args.pool_only else ['dice-frame-*.ppm','dice-double-*.ppm','pool-frame-*.ppm']
for pattern in patterns:
    paths=sorted(root.glob(pattern))
    if not paths:raise SystemExit('Run npm run firmware:test to produce frames')
    for path in paths:
        frame=Image.open(path).convert('RGB')
        # The test framebuffer already masks pixels outside the physical circle.
        frames.append(frame)
    for _ in range(12):frames.append(frames[-1].copy())
output=root/('pool-preview.gif' if args.pool_only else 'dice-preview.gif')
frames[0].save(output,save_all=True,append_images=frames[1:],duration=66,loop=0,disposal=2,optimize=True)
print(output)
