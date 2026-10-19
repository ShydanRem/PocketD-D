#!/usr/bin/env python3
"""Encode actual host-LVGL test frames as a circular GIF. Requires Pillow.
No board capture: this is a visual preview of the compiled C UI on the host.
Run npm run firmware:test first. Does not alter any game state or source assets.
"""
from pathlib import Path
from PIL import Image

root=Path(__file__).resolve().parents[1]/'firmware/tests/out/ui-build'
frames=[]
for pattern in ['dice-frame-*.ppm','dice-double-*.ppm']:
    paths=sorted(root.glob(pattern))
    if not paths:raise SystemExit('Run npm run firmware:test to produce frames')
    for path in paths:
        frame=Image.open(path).convert('RGB')
        # The test framebuffer already masks pixels outside the physical circle.
        frames.append(frame)
    for _ in range(12):frames.append(frames[-1].copy())
frames[0].save(root/'dice-preview.gif',save_all=True,append_images=frames[1:],duration=66,loop=0,disposal=2,optimize=True)
print(root/'dice-preview.gif')
