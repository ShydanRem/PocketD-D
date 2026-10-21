#!/usr/bin/env python3
"""Publish actual native LVGL captures. No illustrative pixels are added to screens."""
from pathlib import Path
import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument('build', type=Path)
args = parser.parse_args()
build = args.build.resolve()
for capture in sorted(build.glob('companion-*.ppm')):
    subprocess.run(['magick', str(capture), str(capture.with_suffix('.png'))], check=True)
subprocess.run(['magick', 'montage', *[str(build/f'companion-{name}.png') for name in ('home', 'menu', 'heal')],
                '-tile', '3x1', '-geometry', '+12+12', '-background', '#081219',
                str(build/'companion-preview.png')], check=True)
frames = sorted(build.glob('companion-heal-[0-9][0-9].png'))
assert len(frames) == 24, 'Run the native companion executable first'
subprocess.run(['magick', '-delay', '8', *map(str, frames), '-loop', '0',
                str(build/'companion-heal.gif')], check=True)
print(build/'companion-preview.png')
