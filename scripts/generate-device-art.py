#!/usr/bin/env python3
"""Original PocketD-D pixel art, authored on an integer grid; no external assets.

Emit LVGL I4 images with a 16-color BGRA palette, scaled by integer replication.
The generated const assets stay in flash. No dependencies or personal data.
"""
from pathlib import Path
import argparse
import json

ROOT = Path(__file__).resolve().parents[1]
COLORS = [0x0b101c, 0x111b2c, 0x1b2940, 0x293850,
          0x32495b, 0x254d52, 0x448776, 0x76dfc5,
          0xf2cc81, 0xb77c50, 0x5b466b, 0x9f799f,
          0xf0eee5, 0x748aa3, 0xc89675, 0x151621]


class Art:
    def __init__(self, w, h):
        self.w, self.h = w, h
        self.p = [[0] * w for _ in range(h)]

    def rect(self, x, y, w, h, c):
        for yy in range(max(0, y), min(self.h, y+h)):
            for xx in range(max(0, x), min(self.w, x+w)):
                self.p[yy][xx] = c

    def sprite(self, x, y, rows, palette):
        for yy, row in enumerate(rows):
            for xx, c in enumerate(row):
                if c != '.':
                    self.rect(x+xx, y+yy, 1, 1, palette[c])


def scene():
    a = Art(150, 36)
    # Layered night landscape, moon, ruined gate, pines and a lit stone plinth.
    a.rect(8, 0, 134, 30, 1)
    a.rect(3, 6, 144, 24, 1)
    a.rect(0, 14, 150, 16, 1)
    for x, y in [(17, 5), (39, 10), (94, 3), (129, 16), (105, 13)]:
        a.rect(x, y, 1, 1, 13)
    a.rect(115, 2, 7, 7, 8)
    a.rect(113, 4, 11, 3, 8)
    a.rect(113, 2, 7, 5, 1)
    for x, y, w in [(0, 22, 32), (24, 17, 21), (90, 19, 26), (111, 13, 39)]:
        a.rect(x, y, w, 30-y, 2)
        a.rect(x+4, y-3, w-8, 3, 2)
    for x, y in [(9, 14), (33, 22), (136, 12), (124, 23)]:
        a.rect(x, y, 2, 17, 4)
        for tier in range(3):
            a.rect(x-2-tier*2, y+2+tier*4, 6+tier*4, 3, 5)
            a.rect(x-1-tier*2, y+2+tier*4, 2+tier, 1, 6)
    a.rect(101, 17, 3, 14, 3)
    a.rect(111, 17, 3, 14, 3)
    a.rect(101, 15, 13, 3, 4)
    a.rect(103, 15, 3, 1, 13)
    a.rect(0, 30, 150, 3, 2)
    a.rect(8, 33, 134, 2, 1)
    a.rect(42, 29, 65, 3, 4)
    a.rect(48, 32, 53, 2, 3)
    a.rect(54, 34, 41, 2, 2)
    a.rect(53, 29, 39, 1, 6)
    a.rect(63, 29, 21, 1, 7)
    # The personalized sprite is a separate, animated image layered over this scene.
    a.rect(91, 23, 3, 5, 9)
    a.rect(92, 24, 1, 2, 8)
    a.rect(91, 29, 4, 1, 6)
    for x, y in [(22, 29), (45, 27), (116, 30), (130, 28)]:
        a.rect(x, y, 1, 2, 6)
        a.rect(x+1, y-1, 1, 1, 7)
    return a


ICONS = {
    'hero': ['....7777....', '...777777...', '...77cc77...', '....cccc....',
             '....7777....', '..77788777..', '..77788777..', '.7777887777.',
             '.7777777777.', '....3333....', '...33..33...', '...33..33...'],
    'sheet': ['..88888888..', '..8cccccc8..', '..8c3333c8..', '..8cccccc8..',
              '..8c3333c8..', '..8cccccc8..', '..8c3333c8..', '..8cccccc8..',
              '..8c3333c8..', '..8cccccc8..', '..88888888..', '............'],
    'dice': ['.....88.....', '...888888...', '..88....88..', '.88..88..88.',
             '.8..8..8..8.', '.8.8....8.8.', '.8.8....8.8.', '.8..8888..8.',
             '..8......8..', '...88..88...', '.....88.....', '............'],
    'uses': ['....bbbb....', '.....bb.....', '.....cc.....', '....b77b....',
             '...b7777b...', '..b777777b..', '..b777777b..', '..b777777b..',
             '..b777777b..', '...bbbbbb...', '............', '............'],
}


def encode(name, art, scale, transparent=False):
    w, h = art.w*scale, art.h*scale
    data = []
    for i, c in enumerate(COLORS):
        data += [c & 255, c >> 8 & 255, c >> 16, 0 if transparent and i == 0 else 255]
    for y in range(h):
        row = [art.p[y//scale][x//scale] for x in range(w)]
        data += [(row[x] << 4) | row[x+1] for x in range(0, w, 2)]
    body = '\n'.join('    '+','.join(f'0x{b:02x}' for b in data[i:i+24])+',' for i in range(0, len(data), 24))
    return f'''_Alignas(4) static const uint8_t pd_art_{name}_data[] = {{
{body}
}};
static const lv_image_dsc_t pd_art_{name} = {{
    .header = {{.magic=LV_IMAGE_HEADER_MAGIC, .cf=LV_COLOR_FORMAT_I4,
               .w={w}, .h={h}, .stride={w//2}}},
    .data_size=sizeof pd_art_{name}_data, .data=pd_art_{name}_data
}};
'''


def avatar(style, pose):
    a = Art(24, 30)
    # Integer-grid silhouette, cloth, cuirass and boots. No weapon/equipment claim.
    a.rect(7, 10, 11, 15, 1)
    a.rect(5, 13, 15, 11, 1)
    a.rect(4, 20, 17, 6, 1)
    a.rect(6, 12, 13, 13, 7)
    a.rect(5, 21, 15, 4, 7)
    a.rect(16, 13, 3, 12, 6)
    a.rect(6, 15, 2, 9, 8)
    a.rect(8, 11, 8, 2, 8)
    if pose == 1:
        a.rect(3, 23, 3, 3, 1)
        a.rect(4, 23, 2, 2, 8)
        a.rect(6, 22, 1, 3, 7)
    elif pose == 2:
        a.rect(19, 22, 3, 3, 1)
        a.rect(19, 23, 2, 1, 7)
        a.rect(7, 22, 1, 3, 8)
    a.rect(8, 23, 3, 5, 13)
    a.rect(14, 23, 3, 5, 13)
    a.rect(7, 28, 5, 2, 14)
    a.rect(14, 28, 5, 2, 14)
    a.rect(9, 13, 7, 8, 9)
    a.rect(14, 13, 2, 8, 10)
    a.rect(8, 12, 9, 1, 11)
    a.rect(8, 21, 9, 2, 11)
    a.rect(11, 21, 3, 3, 12)
    a.rect(5, 16, 3, 4, 3)
    a.rect(18, 16, 3, 4, 2)
    # Head, face and three hair silhouettes matching the PC's appearance indices.
    a.rect(7, 1, 11, 10, 1)
    a.rect(8, 2, 9, 8, 3)
    a.rect(9, 10, 7, 2, 2)
    a.rect(10, 10, 4, 2, 3)
    a.rect(16, 5, 1, 5, 2)
    if style in (0, 1):
        a.rect(8, 1, 9, 3, 4)
        a.rect(7, 3, 3, 4, 4)
        a.rect(15, 3, 3, 3, 4)
        a.rect(8, 2, 6, 1, 5)
        a.rect(10, 4, 3, 1, 4)
        if style == 1:
            a.rect(7, 6, 2, 7, 4)
            a.rect(17, 5, 2, 9, 4)
            a.rect(18, 8, 1, 4, 5)
    else:
        a.rect(8, 2, 9, 1, 4)
        a.rect(8, 3, 2, 1, 5)
        a.rect(16, 3, 1, 2, 4)
    for x in [10, 14]:
        a.rect(x, 7 if pose == 3 else 6, 2 if pose == 3 else 1, 1 if pose == 3 else 2, 15)
    a.rect(11, 9, 3, 1, 2)
    return a


def avatar_header():
    palette = json.loads((ROOT/'src/avatar-palette.json').read_text())
    out = '/* Original avatar masks. Generated by scripts/generate-device-art.py. */\n#ifndef PD_AVATAR_ATLAS_H\n#define PD_AVATAR_ATLAS_H\n'
    for name in ['skin', 'hair']:
        colors = ','.join('0x'+c[1:] for c in palette[name+'Colors'])
        out += f'static const uint32_t pd_avatar_{name}[3]={{{colors}}};\n'
    rows = [','.join('0x'+c[1:] for c in row) for row in palette['cloakColors']]
    out += 'static const uint32_t pd_avatar_cloak[3][3]={' + ','.join('{'+r+'}' for r in rows) + '};\n'
    out += 'static const uint8_t pd_avatar_masks[3][4][1440]={\n'
    for style in range(3):
        out += ' {\n'
        for pose in range(4):
            a = avatar(style, pose)
            data = []
            for y in range(60):
                row = [a.p[y//2][x//2] for x in range(48)]
                data += [(row[x]<<4)|row[x+1] for x in range(0,48,2)]
            out += '  {\n' + '\n'.join('   '+','.join(f'0x{b:02x}' for b in data[i:i+24])+',' for i in range(0,len(data),24)) + '\n  },\n'
        out += ' },\n'
    return out+'};\n#endif\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    output = '/* Original pixel art. Generated by scripts/generate-device-art.py. */\n#ifndef PD_DEVICE_ART_H\n#define PD_DEVICE_ART_H\n'
    output += encode('scene', scene(), 2)
    for name, rows in ICONS.items():
        art = Art(12, 12)
        art.sprite(0, 0, rows, {c: int(c, 16) for c in '0123456789abcdef'})
        output += encode(name, art, 2, transparent=True)
    output += '#endif\n'
    masks={'width':24,'height':30,'poses':[
        [[''.join(f'{pixel:x}' for pixel in row) for row in avatar(style,pose).p] for pose in range(4)]
        for style in range(3)]}
    files=[(ROOT/'firmware/assets/device_art.h',output),
           (ROOT/'firmware/assets/avatar_atlas.h',avatar_header()),
           (ROOT/'src/device-avatar-masks.json',json.dumps(masks,separators=(',',':'))+'\n')]
    for path, content in files:
        if args.check:
            if not path.is_file() or path.read_text() != content:
                raise SystemExit('Device art differs: run scripts/generate-device-art.py')
        else:
            path.write_text(content)
        print(f'{path}: original art verified' if args.check else path)


if __name__ == '__main__':
    main()
