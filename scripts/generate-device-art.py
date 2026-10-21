#!/usr/bin/env python3
"""Original PocketD-D pixel art, authored on an integer grid; no external assets.

Emit LVGL I4 images with a 16-color BGRA palette, scaled by integer replication.
The generated const assets stay in flash. No dependencies or personal data.
"""
from pathlib import Path
import argparse
import json
import math

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

    def polygon(self, points, c):
        for y in range(max(0, min(p[1] for p in points)), min(self.h, max(p[1] for p in points)+1)):
            crossings=[]
            for (x1,y1),(x2,y2) in zip(points,points[1:]+points[:1]):
                if (y1<=y<y2) or (y2<=y<y1):
                    crossings.append(x1+(y-y1)*(x2-x1)/(y2-y1))
            crossings.sort()
            for left,right in zip(crossings[::2],crossings[1::2]):
                self.rect(math.ceil(left),y,math.floor(right)-math.ceil(left)+1,1,c)

    def ellipse(self, cx, cy, rx, ry, c):
        for y in range(max(0,cy-ry),min(self.h,cy+ry+1)):
            for x in range(max(0,cx-rx),min(self.w,cx+rx+1)):
                if (x-cx)**2*ry**2+(y-cy)**2*rx**2<=rx**2*ry**2:
                    self.p[y][x]=c


# Deep blue distance, jade stone, warm torchlight. Quantized atmosphere, no blur.
HD_COLORS=[0x090e1a,0x101c30,0x1c3047,0x2c495d,
           0x3b6470,0x182f35,0x284e4c,0x488174,
           0x83b6a0,0xc6e1c5,0x554451,0x80645e,
           0xb58b65,0xe9b77c,0xffe3aa,0x0d1b24]


def hd_scene():
    a=Art(150,48)
    for y in range(48):
        a.rect(0,y,150,1,1 if y<15 else 2 if y<29 else 3)
    # Layered clouds, crescent, long silhouettes and a broken aqueduct.
    for x,y,w in [(4,5,32),(80,8,23),(111,2,35),(24,17,42)]:
        a.rect(x,y,w,1,2);a.rect(x+5,y+1,w-11,1,2)
    a.ellipse(110,8,7,7,4);a.ellipse(110,8,5,5,9);a.ellipse(107,6,5,5,1)
    for x,y in [(17,4),(55,6),(87,2),(135,12),(70,10)]:a.rect(x,y,1,1,8)
    for points,c in [([(0,31),(0,17),(12,10),(27,26),(43,17),(58,31)],2),
                     ([(49,32),(71,11),(91,30),(111,19),(132,31),(149,20),(149,39)],3),
                     ([(64,22),(71,11),(78,20),(73,18),(70,18)],4),
                     ([(0,32),(23,23),(44,31),(62,29),(90,35),(149,29),(149,48),(0,48)],5)]:a.polygon(points,c)
    a.rect(12,20,39,3,3)
    for x in [13,26,40]:
        a.rect(x,23,3,13,3);a.rect(x,23,3,1,4)
    # Distant haze with ordered dithering, not a runtime filter.
    for y in range(27,37):
        for x in range(150):
            if a.p[y][x] in (2,3,5) and (x+2*y)%7==0:a.p[y][x]=4 if y<31 else 6
    # Ruined portal; asymmetric masonry and foliage break the rectangular forms.
    for x in [40,103]:
        a.rect(x,13,6,27,5);a.rect(x,13,2,25,7);a.rect(x+2,14,3,24,6)
        a.rect(x-2,11,10,3,7);a.rect(x-1,11,8,1,8)
        for y in [17,24,31]:a.rect(x+1,y,4,1,3)
        a.rect(x-3,37,12,3,6);a.rect(x-2,37,10,1,8)
    a.rect(43,8,63,4,6);a.rect(45,8,59,1,8)
    a.rect(51,7,12,1,7);a.rect(78,7,15,1,7);a.rect(66,8,8,4,3)
    a.rect(68,9,3,3,8);a.rect(69,10,1,1,9)
    # Broken tiled dais, three depth planes and a central casting shadow.
    a.polygon([(24,38),(121,38),(137,46),(13,46)],3)
    a.polygon([(31,37),(117,37),(127,42),(22,42)],6)
    a.rect(31,37,86,1,8);a.rect(22,42,105,2,2)
    for x in range(29,127,12):
        a.rect(x,40,7,1,7);a.rect(x+3,43,8,1,4)
    a.ellipse(75,40,19,3,15)
    a.rect(60,41,30,1,5)
    # Foreground trees, blades and warm lantern pools.
    for x,top in [(4,10),(139,7)]:
        a.polygon([(x,top),(x+2,top),(x+5,44),(x-3,44)],15)
        for y,w in [(top+1,7),(top+8,11),(top+15,14)]:
            a.polygon([(x,y-5),(x-w,y+5),(x+w,y+5)],5)
            a.rect(x-w+2,y+4,w-2,1,6)
    for x in [32,117]:
        a.ellipse(x,39,10,2,10);a.ellipse(x,38,6,1,11)
        a.rect(x-1,28,2,11,11);a.rect(x-2,27,4,2,12)
        a.rect(x-1,24,2,4,13);a.rect(x,24,1,3,14);a.rect(x,23,1,1,14)
    for x,y in [(6,42),(15,41),(26,44),(131,43),(142,40)]:
        a.rect(x,y,1,4,6);a.rect(x-1,y,1,1,8);a.rect(x+1,y+1,1,1,7)
    for x,y in [(24,20),(125,18),(94,26),(17,34),(130,31)]:a.rect(x,y,1,1,13)
    return a


def surround():
    a=Art(233,233)
    # A restrained stone/metal cabinet only at the edge, leaving text area quiet.
    for y in range(233):
        for x in range(233):
            d=(x-116)**2+(y-116)**2
            if 107**2<d<113**2:a.p[y][x]=1 if (x//4+y//4)%5 else 2
            if 112**2<d<114**2:a.p[y][x]=3
            if 114**2<d<115**2:a.p[y][x]=1
    for side in [0,1]:
        x=23 if not side else 206
        for y in [66,101,136,171]:
            a.rect(x,y,4,12,2);a.rect(x+1,y+2,2,8,3)
            a.rect(x+1,y+5,2,2,6)
    for cx,cy in [(116,12),(116,220)]:
        a.polygon([(cx,cy-5),(cx+5,cy),(cx,cy+5),(cx-5,cy)],9)
        a.polygon([(cx,cy-3),(cx+3,cy),(cx,cy+3),(cx-3,cy)],8)
        a.rect(cx,cy-1,1,3,12)
    return a


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

PROPS={
    'grimoire':[
        '................','....888888888...','...89999999998..','..89cccccccca8..',
        '..8c33333333a8..','..8c3bbbb3c3a8..','..8c3b77b3c3a8..','..8c3b77b3c3a8..',
        '..8c3bbbb3c3a8..','..8c33333333a8..','..8ccccccccca8..','..899999999998..',
        '..833333333338..','...8888888888...','................','................'],
    'satchel':[
        '.....999999.....','....9cccccc9....','....9c9999c9....','...9999999999...',
        '..9eeeeeeeeee9..','..9ecccccccce9..','..9e99999999e9..','..9e9cccccc9e9..',
        '..9e9c8888c9e9..','..9e9c8998c9e9..','..9e9c8888c9e9..','..9e9cccccc9e9..',
        '..9e99999999e9..','...9999999999...','....33333333....','................'],
    'crystal':[
        '.......b........','......bcb.......','.....bccbb......','....bccbbbb.....',
        '...bccbbbaab....','...bcb7bbaab....','...bc77bbaab....','...bc77bbaab....',
        '...bc77bbaab....','....b77baab.....','.....b7bab......','......bab.......',
        '....8888888.....','...833333338....','....8888888.....','................'],
    'campfire':[
        '........8.......','.......88.......','.......8c.......','.....88cc8......',
        '.....8ccc88.....','....88cccc8.....','...88cccccc8....','...8cc888cc8....',
        '...8c88888c8....','....8888888.....','.....88888......','..99933333999...',
        '..9e999999e99...','...99e99e99.....','....999999......','................']}


def encode(name, art, scale, transparent=False, colors=COLORS):
    w, h = art.w*scale, art.h*scale
    data = []
    for i, c in enumerate(colors):
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
    # Cloth folds, reflected warm trim and cool rim light, still no equipment claim.
    a.rect(9,14,1,6,12);a.rect(10,15,1,4,9)
    a.rect(15,14,1,5,10);a.rect(16,12,1,8,8)
    a.rect(6,21,1,3,6);a.rect(8,23,1,2,8)
    a.rect(10,24,2,1,6);a.rect(16,24,2,1,6)
    a.rect(8,28,3,1,11);a.rect(15,28,3,1,11)
    a.rect(8,5,1,3,2);a.rect(11,8,2,1,3)
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


def pixel_font_header():
    # Original 5x7 figures; only used for numeric HUD and the fixed brand.
    rows={
        '0':[14,17,19,21,25,17,14], '1':[4,12,4,4,4,4,14],
        '2':[14,17,1,2,4,8,31], '3':[30,1,1,14,1,1,30],
        '4':[2,6,10,18,31,2,2], '5':[31,16,16,30,1,1,30],
        '6':[14,16,16,30,17,17,14], '7':[31,1,2,4,8,8,8],
        '8':[14,17,17,14,17,17,14], '9':[14,17,17,15,1,1,14],
        'P':[30,17,17,30,16,16,16], 'O':[14,17,17,17,17,17,14],
        'C':[14,17,16,16,16,17,14], 'K':[17,18,20,24,20,18,17],
        'E':[31,16,16,30,16,16,31], 'T':[31,4,4,4,4,4,4],
        'D':[30,17,17,17,17,17,30], '-':[0,0,0,31,0,0,0],
        '+':[0,4,4,31,4,4,0], '/':[1,1,2,4,8,16,16], ' ':[0]*7}
    keys=sorted(rows)
    out='/* Original pixel lettering, generated by scripts/generate-device-art.py. */\n#ifndef PD_PIXEL_FONT_H\n#define PD_PIXEL_FONT_H\n'
    for scale in [2,4]:
        bitmap=[];glyphs=['{0}']
        for char in keys:
            width=2 if char==' ' else 5
            advance=(width+1)*scale
            bits=[(rows[char][y//scale]>>(4-x//scale))&1 for y in range(7*scale) for x in range(width*scale)]
            bits += [0]*((-len(bits))%8)
            packed=[sum(bits[i+j]<<(7-j) for j in range(8)) for i in range(0,len(bits),8)]
            glyphs.append(f'{{.bitmap_index={len(bitmap)},.adv_w={advance*16},.box_w={width*scale},.box_h={7*scale},.ofs_y=0}}')
            bitmap+=packed
        prefix=f'pd_pixel_{scale}'
        out+=f'static const uint8_t {prefix}_bitmap[]={{'+','.join(f'0x{b:02x}' for b in bitmap)+'};\n'
        out+=f'static const lv_font_fmt_txt_glyph_dsc_t {prefix}_glyphs[]={{'+','.join(glyphs)+'};\n'
        out+=f'static const uint16_t {prefix}_unicode[]={{'+','.join(str(ord(c)-32) for c in keys)+'};\n'
        out+=f'static const lv_font_fmt_txt_cmap_t {prefix}_map[]={{'+'{.range_start=32,.range_length=53,.glyph_id_start=1,.unicode_list='+prefix+'_unicode,.list_length='+str(len(keys))+',.type=LV_FONT_FMT_TXT_CMAP_SPARSE_TINY}};\n'
        out+=f'static const lv_font_fmt_txt_dsc_t {prefix}_dsc={{.glyph_bitmap={prefix}_bitmap,.glyph_dsc={prefix}_glyphs,.cmaps={prefix}_map,.cmap_num=1,.bpp=1}};\n'
        out+=f'static const lv_font_t {prefix}={{.get_glyph_dsc=lv_font_get_glyph_dsc_fmt_txt,.get_glyph_bitmap=lv_font_get_bitmap_fmt_txt,.line_height={8*scale},.base_line=0,.dsc=&{prefix}_dsc}};\n'
    return out+'#endif\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--check', action='store_true')
    args = parser.parse_args()
    output = '/* Original pixel art. Generated by scripts/generate-device-art.py. */\n#ifndef PD_DEVICE_ART_H\n#define PD_DEVICE_ART_H\n'
    output += encode('scene', scene(), 2)
    output += encode('diorama', hd_scene(), 2, colors=HD_COLORS)
    output += encode('surround', surround(), 2)
    for name, rows in ICONS.items():
        art = Art(12, 12)
        art.sprite(0, 0, rows, {c: int(c, 16) for c in '0123456789abcdef'})
        output += encode(name, art, 2, transparent=True)
    for name,rows in PROPS.items():
        prop=Art(16,16);prop.sprite(0,0,rows,{c:int(c,16) for c in '0123456789abcdef'})
        output+=encode(name,prop,2,transparent=True)
    output += '#endif\n'
    masks={'width':24,'height':30,'poses':[
        [[''.join(f'{pixel:x}' for pixel in row) for row in avatar(style,pose).p] for pose in range(4)]
        for style in range(3)]}
    files=[(ROOT/'firmware/assets/device_art.h',output),
           (ROOT/'firmware/assets/avatar_atlas.h',avatar_header()),
           (ROOT/'firmware/assets/pixel_font.h',pixel_font_header()),
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
