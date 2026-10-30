#!/usr/bin/env python3
"""Original semantic pixel glyphs, one shared alias catalogue; never grants rules."""
import argparse,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];source=json.loads((ROOT/'src/semantic-icons.json').read_text());lines=['/* Generated semantic icons: cosmetic classification only. */']
for name,g in source['glyphs'].items():
 rows=g['rows'];assert len(rows)==12 and all(len(r)==12 and set(r)<=set('.#') for r in rows),name
 data=bytearray();alpha=bytearray();value=int(g['color'],16)
 for y in range(24):
  c=value if y<14 else 0xffefd2;r,gc,b=c>>16,(c>>8)&255,c&255
  for x in range(24):data+=(((r>>3)<<11)|((gc>>2)<<5)|(b>>3)).to_bytes(2,'little');alpha.append(255 if rows[y//2][x//2]=='#' else 0)
 data+=alpha;lines.append(f'_Alignas(4) static const uint8_t sem_{name}_pixels[]={{');lines+=[' '+','.join(map(str,data[i:i+32]))+',' for i in range(0,len(data),32)]
 lines+=['};',f'static const lv_image_dsc_t sem_{name}={{.header={{.magic=LV_IMAGE_HEADER_MAGIC,.cf=LV_COLOR_FORMAT_RGB565A8,.w=24,.h=24,.stride=48}},.data_size=sizeof sem_{name}_pixels,.data=sem_{name}_pixels}};']
lines.append('typedef struct {unsigned family;const char *alias;const lv_image_dsc_t *image;} semantic_alias;\nstatic const semantic_alias semantic_aliases[]={')
for family,items in source['aliases'].items():
 for name,aliases in items:
  for alias in aliases:lines.append('{'+str(['item','spell','feature'].index(family))+','+json.dumps(alias)+',&sem_'+name+'},')
lines.append('};');data='\n'.join(lines)+'\n';out=ROOT/'firmware/ui/semantic_icons.inc'
p=argparse.ArgumentParser();p.add_argument('--check',action='store_true');args=p.parse_args()
if args.check:assert out.read_text()==data,'Regenerate semantic glyphs'
else:out.write_text(data)
print(f'Semantic icons: {len(source["glyphs"])} original masks and stable bilingual aliases verified; cosmetic only')
