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
    # Distance is quieter and cooler; irregular silhouettes replace flat strips.
    for y in range(48):
        a.rect(0,y,150,1,1 if y<15 else 2 if y<30 else 3)
    for x,y,w in [(3,5,31),(78,8,24),(116,3,27)]:
        a.rect(x,y,w,1,2);a.rect(x+5,y+1,w-12,1,2)
    a.ellipse(111,9,8,8,3);a.ellipse(111,9,6,6,4)
    a.ellipse(111,9,5,5,9);a.ellipse(108,7,5,5,1)
    for x,y in [(17,4),(54,5),(87,2),(136,12),(72,10)]:a.rect(x,y,1,1,8)
    ridges=[([(0,29),(0,19),(8,12),(15,15),(20,22),(33,16),(48,29)],2),
            ([(45,32),(57,23),(62,24),(71,11),(78,20),(85,22),(94,30),
              (109,19),(119,22),(127,18),(149,28),(149,39)],3)]
    for points,c in ridges:a.polygon(points,c)
    a.polygon([(64,22),(71,11),(78,20),(74,18),(72,19),(70,16)],4)
    a.polygon([(104,25),(109,19),(118,23),(111,22),(110,23)],4)
    # Broken aqueduct, softened by two discontinuous ribbons of horizon mist.
    a.rect(10,22,42,2,3)
    for x in [11,24,38]:
        a.rect(x,24,3,10,3);a.rect(x,24,1,8,4)
        a.rect(x+2,24,6,1,2);a.rect(x+3,25,4,1,2)
    for y in range(28,34):
        for x in range(150):
            if a.p[y][x] in (2,3) and ((x//11+y)%4!=0) and (x+y)%3==0:a.p[y][x]=4
    a.polygon([(0,34),(22,27),(36,31),(52,29),(70,34),(95,30),(116,33),
               (149,27),(149,48),(0,48)],5)
    # Cut stone, chips, seams and moss; the portal is intentionally asymmetric.
    for x in [40,103]:
        a.polygon([(x,13),(x+5,13),(x+6,36),(x+9,39),(x-3,39),(x-1,35)],5)
        a.rect(x,14,2,23,7);a.rect(x+2,15,3,22,6)
        a.rect(x-2,11,10,3,6);a.rect(x-1,11,8,1,8)
        for y in [18,25,32]:
            a.rect(x+1,y,4,1,3);a.rect(x+3,y+1,2,1,5)
        a.rect(x+1,15,1,2,8);a.rect(x+1,22,1,2,8)
        a.rect(x-3,37,12,3,6);a.rect(x-2,37,10,1,7)
    a.polygon([(42,9),(49,8),(52,7),(64,8),(68,7),(74,8),(78,7),
               (92,8),(95,9),(107,9),(106,12),(94,12),(92,11),(81,12),
               (74,11),(68,12),(59,11),(48,12),(42,11)],6)
    for x in range(46,105,11):
        a.rect(x,9,6,1,8);a.rect(x+7,10,1,2,3)
    a.polygon([(67,8),(74,8),(75,11),(71,14),(67,11)],3)
    a.rect(70,9,2,3,8);a.rect(70,10,1,1,9)
    # Ivy descends over the ruin, leaving the center clear for the avatar.
    for x,y in [(39,12),(106,11),(99,9),(45,18)]:
        for j in range(8):
            xx=x+(j//3)%2
            a.rect(xx,y+j,1,2,6);a.rect(xx-1 if j%2 else xx+1,y+j,2,1,7)
            if j%3==0:a.rect(xx-1,y+j,1,1,8)
    # Perspective paving: broad contact shadow and uneven warm/cool edge lights.
    a.polygon([(24,38),(122,38),(140,47),(10,47)],3)
    a.polygon([(31,37),(116,37),(130,42),(21,42)],6)
    a.rect(32,37,25,1,8);a.rect(91,37,25,1,8)
    a.rect(22,42,108,1,2);a.rect(18,44,115,2,2)
    for row in range(3):
        for x in range(23-row*4,132+row*3,13):
            a.rect(x,39+row*3,9,1,4 if row else 7)
            a.rect(x+10,38+row*3,1,2,5)
    a.ellipse(75,40,20,3,15);a.ellipse(75,40,13,2,0)
    # Near foliage is a set of overlapping leaf clumps, with visible crooked limbs.
    for x,mirror in [(2,1),(146,-1)]:
        a.polygon([(x,3),(x+3*mirror,3),(x+5*mirror,44),
                   (x-3*mirror,44),(x+mirror,22)],15)
        a.polygon([(x+2*mirror,15),(x+17*mirror,7),(x+12*mirror,12),
                   (x+4*mirror,20)],15)
        for dx,dy,rx,ry in [(0,3,12,6),(12,5,10,4),(4,11,11,5),(9,18,7,3)]:
            cx=x+dx*mirror;a.ellipse(cx,dy,rx,ry,5)
            a.ellipse(cx-2*mirror,dy-1,max(2,rx-3),max(1,ry-2),6)
            for k in range(3):a.rect(cx+(k*3-4)*mirror,dy-2+k%2,2,1,7)
    # Lanterns and reflected light are static art; a small translucent halo is layered above.
    for x in [32,117]:
        a.ellipse(x,39,11,2,10);a.ellipse(x,38,7,1,11)
        a.rect(x-1,28,2,11,11);a.rect(x-2,27,4,2,12)
        a.rect(x-2,23,5,5,10);a.rect(x-1,24,3,3,13)
        a.rect(x,24,1,3,14);a.rect(x-2,23,5,1,12)
        a.rect(x,21,1,2,12)
        a.rect(x-5,40,3,1,12);a.rect(x+2,41,4,1,11)
    for x,y in [(6,43),(16,41),(27,45),(129,44),(140,41)]:
        a.rect(x,y,1,3,6);a.rect(x-1,y,1,1,8);a.rect(x+1,y+1,2,1,7)
        a.rect(x+2,y+1,1,1,8)
    for x,y in [(24,20),(125,18),(94,26),(17,34),(130,31)]:a.rect(x,y,1,1,13)
    return a


def lantern_halo():
    a=Art(20,20)
    # Baked falloff in an indexed alpha palette, still exact 2x pixel replication.
    for y in range(20):
        for x in range(20):
            radius=((x-9.5)**2+(y-9.5)**2)/91
            level=max(0,min(15,int((1-radius)*15)))
            a.p[y][x]=level
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

# Specific symbols describe explicit row kinds, never inferred names or rules.
ICONS.update({
    'sword':['.........cc.','........ccc.','.......ccc..','......ccc...',
             '.....ccc....','..8.ccc.....','...8cc......','...c88......',
             '..99..8.....','.99.........','.9..........','............'],
    'bundle':['............','...999999...','..9eeeeee9..','.9888888889.','.9ee88eee99.','.9ee88eee99.','.9888888889.','.9ee88eee99.','..9e88eee9..','...999999...','............','............'],
    'flask':['....8888....','.....99.....','.....cc.....','....cbbc....',
             '...cbbbcc...','..cbbbbbbc..','..cb88bbbc..','..cb8bbbbc..',
             '..cbbbbbbc..','...cccccc...','............','............'],
    'pouch':['....9999....','...988889...','...988889...','....9999....',
             '...9eeee9...','..9eeeeee9..','.9ee888eee9.','.9ee898eee9.',
             '.9ee888eee9.','..9eeeeee9..','...999999...','............'],
    'spell':['......b.....','.....bcb....','.....bcb....','..bbbcccbbb.',
             '...bccccc...','....bccc....','...bccccc...','..bbbcccbbb.',
             '.....bcb....','.....bcb....','......b.....','............'],
    'talent':['.....88.....','....8cc8....','...8cccc8...','..8cc33cc8..',
              '.8cc3773cc8.','.8c377773c8.','..8c3773c8..','...8c33c8...',
              '....8cc8....','.....88.....','............','............'],
    'heart':['............','..eee..eee..','.e888ee888e.','.e88888888e.',
             '.e88888888e.','..e888888e..','...e8888e...','....e88e....',
             '.....ee.....','............','............','............'],
    'shield':['..cccccccc..','..c777777c..','..c788887c..','..c787787c..',
              '..c787787c..','..c788887c..','...c7777c...','...c7777c...',
              '....c77c....','.....cc.....','............','............'],
    'recover':['....77......','..777777....','.77....77...','.7......7...',
               '.7......777.','.7.......7..','..7.........','...777777...',
               '............','............','............','............'],
})

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


def encode(name, art, scale, transparent=False, colors=COLORS, alphas=None):
    w, h = art.w*scale, art.h*scale
    data = []
    for i, c in enumerate(colors):
        data += [c & 255, c >> 8 & 255, c >> 16, alphas[i] if alphas is not None else 0 if transparent and i == 0 else 255]
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
    a=Art(24,30)
    lift=1 if pose==1 else 0
    lean=-1 if pose==1 else 1 if pose==2 else 0
    # Weighted cape silhouette, narrow legs and asymmetric folds. No weapon claim.
    a.polygon([(9,10),(16,10),(19,13-lift),(20,20),(21+lean,25),
               (17,27),(12,25),(7,27),(4+lean,25),(6,19),(6,14-lift)],1)
    a.polygon([(8,12),(16,11),(18,14),(18,21),(20+lean,24),
               (16,25),(12,23),(7,25),(6+lean,24),(8,18)],7)
    a.polygon([(16,12),(18,14),(18,21),(20+lean,24),(17,25),(15,22)],6)
    a.polygon([(8,13),(9,14),(8,21),(7+lean,24),(6+lean,24)],8)
    a.rect(17,20,1,3,7);a.rect(8+lean,23,2,1,8)
    a.rect(15,24,2,1,8)
    # Feet remain planted while the shoulders breathe one logical pixel.
    a.polygon([(9,22),(12,22),(12,25),(11,28),(8,28)],13)
    a.polygon([(14,22),(17,22),(17,28),(14,28),(13,25)],13)
    a.rect(8,25,2,3,14);a.rect(15,25,2,3,14)
    a.rect(7,28,5,2,1);a.rect(14,28,5,2,1)
    a.rect(8,28,3,1,11);a.rect(15,28,3,1,11)
    # Tunic, diagonal strap, collar and small clasp.
    a.polygon([(9,12-lift),(15,12-lift),(17,15-lift),(16,21),
               (13,23),(9,21),(8,15-lift)],9)
    a.polygon([(14,13-lift),(16,15-lift),(16,20),(14,21)],10)
    a.rect(10,16-lift,1,4,10);a.rect(11,15-lift,2,1,9)
    a.polygon([(8,12-lift),(10,12-lift),(16,20),(14,21)],11)
    a.rect(10,14-lift,1,2,12);a.rect(13,18,1,2,12)
    a.rect(9,21,7,1,13);a.rect(12,21,2,2,12)
    a.rect(12,21,1,1,11)
    # Rounded shoulders and hands, with reflected warm/cool edges.
    a.polygon([(7,13-lift),(9,14-lift),(8,18-lift),(6,19-lift),(5,17-lift)],7)
    a.rect(6,14-lift,1,3,8);a.rect(6,18-lift,2,3,2);a.rect(6,18-lift,1,2,3)
    a.polygon([(17,13-lift),(19,15-lift),(19,19-lift),(17,19-lift),(16,15-lift)],6)
    a.rect(17,18-lift,2,3,2);a.rect(17,18-lift,1,2,3)
    a.polygon([(9,11-lift),(12,10-lift),(16,11-lift),(17,13-lift),
               (14,14-lift),(10,13-lift),(8,14-lift)],8)
    a.rect(11,12-lift,2,2,11);a.rect(11,12-lift,1,1,12)
    # Rounded face, ears, brow, nose and jaw; hair silhouettes retain their indices.
    a.polygon([(10,1),(15,1),(18,3),(18,7),(16,10),(14,11),
               (10,10),(8,8),(7,4)],1)
    a.polygon([(10,3),(15,3),(17,5),(16,9),(14,10),(11,10),(9,8),(9,5)],3)
    a.rect(8,6,1,2,2);a.rect(17,6,1,2,2)
    a.rect(15,7,1,2,2);a.rect(11,9,4,1,2)
    a.rect(12,7,1,1,2);a.rect(13,8,1,1,2)
    if style in (0,1):
        a.polygon([(9,1),(15,1),(18,3),(17,5),(15,4),(14,3),
                   (12,4),(10,3),(9,6),(8,7),(7,4)],4)
        a.rect(10,2,4,1,5);a.rect(15,3,2,1,5)
        a.rect(8,4,1,2,5);a.rect(14,2,1,1,4)
        if style==1:
            a.polygon([(7,4),(9,4),(8,10),(9,13),(7,15),(6,13),(7,9)],4)
            a.polygon([(16,4),(18,4),(18,10),(20,13),(18,15),(16,11)],4)
            a.rect(7,8,1,4,5);a.rect(17,7,1,5,5);a.rect(18,12,1,1,5)
    else:
        a.rect(10,2,5,1,5);a.rect(9,3,2,1,4)
        a.rect(16,3,1,2,5);a.rect(8,4,1,2,5)
    for x in [10,14]:
        a.rect(x,5,2,1,2)
        a.rect(x,6,2 if pose==3 else 1,1 if pose==3 else 2,15)
    a.rect(12,9,2,1,2)
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
    output += encode('lantern_halo', lantern_halo(), 2, colors=[0xffcd86]*16, alphas=[i*5 for i in range(16)])
    output += encode('surround', surround(), 2)
    corner=Art(4,4);corner.sprite(0,0,['888.','89..','8...','....'],{'8':8,'9':9})
    output+=encode('panel_corner',corner,2,transparent=True)
    corner.p=[row[::-1] for row in corner.p[::-1]]
    output+=encode('panel_corner_end',corner,2,transparent=True)
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
