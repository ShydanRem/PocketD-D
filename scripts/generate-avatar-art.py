#!/usr/bin/env python3
"""Export the generated avatar atlas as shared I8 masks and material palettes.

--export uses ImageMagick 7 for source slicing/sampling. Ordinary generation and
--check use the committed indexed masks; firmware builds need neither this tool
nor an image library. No per-frame scaling: one scale and a boot-center anchor.
"""
import argparse
from collections import Counter
import colorsys
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT/'firmware/assets/avatar-source/poses.png'
MASKS = ROOT/'src/device-avatar-masks.json'
W, H = 48, 60
SOURCE_BASE = (194, 87, 80)


def export_masks():
    size = subprocess.check_output(['magick', 'identify', '-format', '%wx%h', str(SOURCE)], text=True)
    assert size == '1448x1086', size
    rgba = subprocess.check_output(['magick', str(SOURCE), '-depth', '8', 'rgba:-'])
    frames, materials = [], []
    scale = 58/357
    # Measured empty rows separate the generated strips. Uniform thirds would
    # cut the first row's boots and contaminate the next with detached pixels.
    rows = [(0, 369), (369, 733), (733, 1086)]
    bounds = []
    for row, (top, bottom) in enumerate(rows):
        for col in range(4):
            points = [(x, y) for y in range(top, bottom) for x in range(col*362, (col+1)*362)
                      if rgba[(y*1448+x)*4+3] >= 128]
            x0, x1 = min(x for x,y in points), max(x for x,y in points)+1
            y0, y1 = min(y for x,y in points), max(y for x,y in points)+1
            boots = [x for x,y in points if y >= y1-12]
            anchor = (min(boots)+max(boots)+1)/2
            width, height = round((x1-x0)*scale), round((y1-y0)*scale)
            left, upper = round(27-(anchor-x0)*scale), 59-height
            assert left >= 0 and left+width <= W and upper >= 0
            pixels = subprocess.check_output(['magick', str(SOURCE), '-crop', f'{x1-x0}x{y1-y0}+{x0}+{y0}',
                '+repage', '-channel', 'A', '-threshold', '50%', '+channel', '-sample', f'{width}x{height}!',
                '-background', 'none', '-gravity', 'northwest', '-splice', f'{left}x{upper}',
                '-extent', f'{W}x{H}', '-depth', '8', 'rgba:-'])
            assert len(pixels) == W*H*4
            frame, types = [], []
            for i in range(W*H):
                r,g,b,a = pixels[4*i:4*i+4]; x,y = i%W,i//W
                hue,sat,value = colorsys.rgb_to_hsv(r/255,g/255,b/255)
                kind = -1
                if a < 128: kind = -2
                elif 0.36 < hue < 0.60 and sat > .25 and value > .12: kind = 2
                elif y < 33 and r > 155 and g > 80 and .025 < hue < .12 and .20 < sat < .75:
                    kind = 0
                elif (y < 30 or (row == 1 and y < 37 and (x < 18 or x > 37))) and r > g*1.12 and g > b*1.1 and value > .15:
                    kind = 1
                frame.append((r,g,b));types.append(kind)
            frames.append(frame);materials.append(types)
            bounds.append(dict(style=row, pose=col, source=[x0,y0,x1,y1], anchor=anchor,
                               destination=[left,upper,width,height]))
    # Fixed leather, gold, linen, outline and eye colors retain their own tones.
    fixed_counts = Counter(tuple(c//16*16+8 for c in rgb) for frame,types in zip(frames,materials)
                           for rgb,kind in zip(frame,types) if kind == -1)
    fixed = sorted(fixed_counts, key=lambda c:(-fixed_counts[c],c))[:192]
    palettes = [0]*49 + [(r<<16)|(g<<8)|b for r,g,b in fixed]
    palettes += [0]*(256-len(palettes))
    encoded = []
    for frame,types in zip(frames,materials):
        data=[]
        for rgb,kind in zip(frame,types):
            if kind == -2: index=0
            elif kind >= 0:
                lum=(54*rgb[0]+183*rgb[1]+19*rgb[2])//256
                index=1+16*kind+min(15,lum//16)
            else:
                index=49+min(range(len(fixed)),key=lambda i:sum((a-b)**2 for a,b in zip(rgb,fixed[i])))
            data.append(index)
        encoded.append([''.join(f'{v:02x}' for v in data[y*W:(y+1)*W]) for y in range(H)])
    masks=dict(width=W,height=H,format='I8',poseNames=['idle','blink','heal','hurt'],
               fixed=palettes,poses=[encoded[i:i+4] for i in range(0,12,4)],
               source_sha256=hashlib.sha256(SOURCE.read_bytes()).hexdigest(),
               normalization=dict(scale=scale,anchor=[27,59],frames=bounds))
    MASKS.write_text(json.dumps(masks,separators=(',',':'))+'\n')


def material_palettes():
    appearance=json.loads((ROOT/'src/avatar-palette.json').read_text())
    groups=[]
    for kind,key in enumerate(('skinColors','hairColors','cloakColors')):
        variants=[]
        for value in appearance[key]:
            if kind==2:value=value[0]
            base=int(value[1:],16);channels=[base>>16,base>>8&255,base&255]
            shades=[]
            for shade in range(16):
                lum=shade*16+8;anchor=SOURCE_BASE[kind]
                rgb=[c*lum//anchor if lum<=anchor else c+(255-c)*(lum-anchor)//(255-anchor) for c in channels]
                shades.append((rgb[0]<<16)|(rgb[1]<<8)|rgb[2])
            variants.append(shades)
        groups.append(variants)
    return groups


def generate(check):
    masks=json.loads(MASKS.read_text())
    assert masks['source_sha256']==hashlib.sha256(SOURCE.read_bytes()).hexdigest()
    assert masks['width']==W and masks['height']==H and masks['format']=='I8'
    materials=material_palettes()
    lines=['/* Generated by scripts/generate-avatar-art.py; source/provenance in avatar-source. */',
           '#ifndef PD_AVATAR_ATLAS_H', '#define PD_AVATAR_ATLAS_H',
           'static const uint32_t pd_avatar_materials[3][3][16]={']
    for group in materials:
        lines.append('{'+','.join('{'+','.join(hex(v) for v in shades)+'}' for shades in group)+'},')
    lines+=['};','static const uint32_t pd_avatar_fixed[256]={'+','.join(hex(v) for v in masks['fixed'])+'};',
            'static const uint8_t pd_avatar_masks[3][4][2880]={']
    for poses in masks['poses']:
        assert len(poses)==4
        lines.append('{')
        for pose in poses:
            assert len(pose)==H and all(len(row)==W*2 for row in pose)
            lines.append('{')
            lines.extend(','.join('0x'+row[i:i+2] for i in range(0,len(row),2))+',' for row in pose)
            lines.append('},')
        lines.append('},')
    lines+=['};','#endif']
    files=[(ROOT/'firmware/assets/avatar_atlas.h','\n'.join(lines)+'\n'),
           (ROOT/'src/device-avatar-colors.json',json.dumps(materials,separators=(',',':'))+'\n')]
    for path,content in files:
        if check:
            assert path.read_text()==content, f'Regenerate {path}'
        else:path.write_text(content)
    print('Avatar I8: 12 masks, 81 appearances, source hash and shared C/JS colors verified')


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--export',action='store_true')
    parser.add_argument('--check',action='store_true')
    args=parser.parse_args()
    if args.export:export_masks()
    generate(args.check)
