#!/usr/bin/env python3
"""Normalize original transparent ImageGen sheets and encode shared C/JS sprites.

ImageMagick only samples the source to the native pixel grid. Python assigns
material indices and encodes Huffman/PackBits/XOR data; it does not redraw the artwork.
Ordinary builds use committed data and require neither ImageGen nor ImageMagick.
"""
import argparse,colorsys,hashlib,heapq,json,struct,subprocess
from collections import Counter
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];W,H=48,60
CAT=ROOT/'src/avatar-species.json';OUT=ROOT/'src/device-species-masks.json'
def raw(path):
 w,h=map(int,subprocess.check_output(['magick','identify','-format','%w %h',str(path)],text=True).split())
 rgba=subprocess.check_output(['magick',str(path),'-depth','8','rgba:-']);assert len(rgba)==w*h*4
 return w,h,rgba
def cuts(occupied,count,length):
 gaps=[];at=0
 while at<length:
  if occupied[at]:at+=1;continue
  start=at
  while at<length and not occupied[at]:at+=1
  if start and at<length:gaps.append((start,at))
 result=[0]
 for n in range(1,count):
  ideal=length*n/count
  possible=[g for g in gaps if abs(sum(g)/2-ideal)<length/count*.42 and sum(g)/2>result[-1]]
  if not possible:
   window=range(round(ideal-length/count*.24),round(ideal+length/count*.24))
   at=min(window,key=lambda x:(occupied[x],abs(x-ideal)))
   assert occupied[at]<=6,('No safe cell separation',n,count,length,occupied[at])
   result.append(at);continue
  gap=min(possible,key=lambda g:abs(sum(g)/2-ideal));result.append((gap[0]+gap[1])//2)
 result.append(length);assert len(set(result))==count+1;return result
def sample(species):
 source=ROOT/'firmware/assets/species-source'/f'{species["key"]}.png';w,h,rgba=raw(source)
 ys=cuts([any(rgba[(y*w+x)*4+3]>=128 for x in range(w)) for y in range(h)],3,h)
 xs=[cuts([sum(rgba[(y*w+x)*4+3]>=128 for y in range(ys[row],ys[row+1])) for x in range(w)],8,w) for row in range(3)]
 bounds=[]
 for body in range(2):
  rows=[]
  for style in range(3):
   poses=[]
   for pose in range(4):
    col=body*4+pose
    points=[(x,y) for y in range(ys[style],ys[style+1]) for x in range(xs[style][col],xs[style][col+1]) if rgba[(y*w+x)*4+3]>=128]
    assert points,(species['key'],body,style,pose)
    x0=min(x for x,y in points);x1=max(x for x,y in points)+1;y0=min(y for x,y in points);y1=max(y for x,y in points)+1
    feet=[x for x,y in points if y>=y1-max(3,(y1-y0)//25)];anchor=(min(feet)+max(feet)+1)/2
    poses.append(dict(box=[x0,y0,x1,y1],anchor=anchor))
   rows.append(poses)
  bounds.append(rows)
 # Shared scale for every hairstyle/pose, separately for each body. Keep true short species.
 frames=[]
 for body,rows in enumerate(bounds):
  allposes=[p for row in rows for p in row]
  scale=min(species['height']/max(p['box'][3]-p['box'][1] for p in allposes),25/max(p['anchor']-p['box'][0] for p in allposes),20/max(p['box'][2]-p['anchor'] for p in allposes))
  for style,poses in enumerate(rows):
   for pose,b in enumerate(poses):
    x0,y0,x1,y1=b['box'];width=max(1,round((x1-x0)*scale));height=max(1,round((y1-y0)*scale));left=round(27-(b['anchor']-x0)*scale);top=60-height
    assert left>=1 and top>=0 and left+width<W,(species['key'],left,top,width,height)
    pixels=subprocess.check_output(['magick',str(source),'-crop',f'{x1-x0}x{y1-y0}+{x0}+{y0}','+repage','-channel','A','-threshold','50%','+channel','-sample',f'{width}x{height}!','-background','none','-gravity','northwest','-splice',f'{left}x{top}','-extent',f'{W}x{H}','-depth','8','rgba:-'])
    assert len(pixels)==W*H*4
    frames.append((body,style,pose,pixels,dict(source=b['box'],destination=[left,top,width,height])))
 return frames,hashlib.sha256(source.read_bytes()).hexdigest(),[xs,ys]
def material(species,rgb,x,y,box):
 r,g,b=rgb;h,s,v=colorsys.rgb_to_hsv(r/255,g/255,b/255);_,top,width,height=box;relative=(y-top)/max(1,height)
 identity=species['key'];head=relative<.56
 if identity=='tabaxi' and .07<h<.19 and s>.24 and v>.22 and (head or x<16 or x>35):return 0
 if identity=='kenku' and .52<h<.85 and .10<s<.65 and .10<v<.8:return 0
 if identity=='aarakocra' and head and s<.38 and v>.30:return 0
 if identity=='tortle' and .15<h<.35 and s>.15 and v>.20:return 0
 if .35<h<.61 and s>.24 and v>.10:return 2
 if identity in ('orc','half-orc') and .16<h<.35 and s>.13 and v>.13:return 0
 if identity=='tiefling' and .62<h<.88 and s>.13 and v>.18:return 0
 if identity=='dragonborn' and .015<h<.15 and s>.35 and v>.20:return 0
 if identity=='goliath' and (s<.27 or .52<h<.74) and .22<v<.87 and (head or x<16 or x>34):return 0
 if species['id']>=12:return -1
 if identity=='aasimar':
  if head and s<.25 and v>.60:return 1
  if .02<h<.13 and .18<s<.8 and v>.22 and (head or x<17 or x>35):return 0
 else:
  if r>150 and g>75 and .02<h<.13 and .18<s<.76 and (head or x<16 or x>35):return 0
 if head and r>g*1.10 and g>b*1.08 and v>.13:return 1
 return -1
def pack(data):
 out=bytearray();at=0
 while at<len(data):
  run=1
  while at+run<len(data) and data[at+run]==data[at] and run<128:run+=1
  if run>=3:out.extend([127+run,data[at]]);at+=run;continue
  start=at;at+=run
  while at<len(data) and at-start<128:
   run=1
   while at+run<len(data) and data[at+run]==data[at] and run<3:run+=1
   if run>=3:break
   at+=min(run,128-(at-start))
  out.append(at-start-1);out.extend(data[start:at])
 return bytes(out)
def unpack(data):
 out=bytearray();at=0
 while at<len(data):
  n=data[at];at+=1
  if n<128:out.extend(data[at:at+n+1]);at+=n+1
  else:out.extend([data[at]]*(n-127));at+=1
 return bytes(out)
def palettes(catalog):
 original=json.loads((ROOT/'src/device-avatar-colors.json').read_text());result=[]
 for species in catalog:
  groups=[]
  for group,colors in enumerate([species['skin'],species.get('hairColors',['#774323','#d6ae65','#302c32']),['#3f806c','#8b4550','#344960']]):
   variants=[]
   for text in colors:
    base=int(text[1:],16);channels=[base>>16,base>>8&255,base&255];anchor=[species.get('skinAnchor',180),87,80][group]
    variants.append([sum((max(0,min(255,c*lum//anchor if lum<=anchor else c+(255-c)*(lum-anchor)//(255-anchor)))<<shift) for c,shift in zip(channels,[16,8,0])) for lum in range(8,256,16)])
   groups.append(variants)
  # Keep the existing cloak choices exactly consistent with the original avatar.
  groups[2]=original[2];result.append(groups)
 return result
def huffman_tree(streams):
 counts=Counter(v for stream in streams for v in stream);heap=[(n,i,i) for i,n in counts.items()];heapq.heapify(heap);serial=256
 while len(heap)>1:
  a,_,left=heapq.heappop(heap);b,_,right=heapq.heappop(heap);heapq.heappush(heap,(a+b,serial,(left,right)));serial+=1
 tree=[];codes={}
 def walk(node,code):
  if isinstance(node,int):codes[node]=code;return -node-1
  at=len(tree);tree.append(None);left=walk(node[0],code+[0]);right=walk(node[1],code+[1]);tree[at]=[left,right];return at
 assert walk(heap[0][2],[])==0;return tree,codes
def huffman_pack(data,codes):
 out=bytearray();acc=bits=0
 for value in data:
  for bit in codes[value]:
   acc=(acc<<1)|bit;bits+=1
   if bits==8:out.append(acc);acc=bits=0
 if bits:out.append(acc<<(8-bits))
 return bytes(out)
def huffman_unpack(data,size,tree):
 out=bytearray();bit=0
 for _ in range(size):
  node=0
  while node>=0:
   assert bit<len(data)*8,'Truncated Huffman stream'
   node=tree[node][(data[bit//8]>>(7-bit%8))&1];bit+=1
  out.append(-node-1)
 assert (bit+7)//8==len(data) and not any((data[n//8]>>(7-n%8))&1 for n in range(bit,len(data)*8))
 return bytes(out)
def packed_frame(frame,data):
 encoded=bytes.fromhex(frame['bytes'])
 return huffman_unpack(encoded,frame['size'],data['tree']) if 'tree' in data else encoded
def export():
 catalog=json.loads(CAT.read_text());previous=json.loads(OUT.read_text()) if OUT.exists() else None
 # Existing masks, palette indices and exact RGB values are immutable during catalog extension.
 original_count=len(previous['profiles']) if previous else 0
 allframes=[];counts=Counter();metadata=previous['sources'].copy() if previous else []
 for species in catalog[original_count:]:
  frames,digest,grid=sample(species);typed=[]
  for body,style,pose,pixels,bounds in frames:
   if species['id']==0 and body==0:continue
   rgb=[];types=[]
   for i in range(W*H):
    r,g,b,a=pixels[i*4:i*4+4];color=(r,g,b);kind=-2 if a<128 else material(species,color,i%W,i//W,bounds['destination'])
    rgb.append(color);types.append(kind)
    if kind==-1:counts[tuple(c//16*16+8 for c in color)]+=1
   typed.append((body,style,pose,rgb,types,bounds))
  allframes.append(typed);metadata.append(dict(key=species['key'],sha256=digest,grid=grid));print('Sampled',species['key'],len(typed),'frames',flush=True)
 fixed_values=previous['fixed'].copy() if previous else [0]*256
 fixed=[(i,((c>>16)&255,(c>>8)&255,c&255)) for i,c in enumerate(fixed_values) if i>=49 and c]
 free=[i for i in range(49,256) if not fixed_values[i]]
 existing={c for _,c in fixed}
 for index,color in zip(free,sorted((c for c in counts if c not in existing),key=lambda c:(-counts[c],c))):
  r,g,b=color;fixed_values[index]=r<<16|g<<8|b;fixed.append((index,color))
 lookup={}
 def fixed_index(rgb):
  q=tuple(c//16*16+8 for c in rgb)
  if q not in lookup:lookup[q]=min(fixed,key=lambda v:sum((a-b)**2 for a,b in zip(q,v[1])))[0]
  return lookup[q]
 profiles=previous['profiles'].copy() if previous else [];normalization=previous['normalization'].copy() if previous else []
 for family in profiles:
  for body in family:
   for style in body:
    for frame in style:
     if frame:frame['packed']=packed_frame(frame,previous)
 for identity,frames in enumerate(allframes,original_count):
  profile=[[[None]*4 for _ in range(3)] for _ in range(2)]
  for body,style,pose,rgb,types,bounds in frames:
   pixels=bytes(0 if kind==-2 else 1+16*kind+min(15,(54*c[0]+183*c[1]+19*c[2])//4096) if kind>=0 else fixed_index(c) for c,kind in zip(rgb,types))
   encoded=pack(pixels);xor=False
   if pose:
    base=profile[body][style][0]['raw'];delta=bytes(a^b for a,b in zip(base,pixels));compressed=pack(delta)
    if len(compressed)<len(encoded):encoded=compressed;xor=True
   profile[body][style][pose]={'packed':encoded,'xor':xor,'raw':pixels};assert len(unpack(encoded))==W*H
   normalization.append(dict(lineage=identity,body=body,style=style,pose=pose,**bounds))
  profiles.append(profile)
 allposes=[f for family in profiles for body in family for style in body for f in style if f]
 tree,codes=huffman_tree([f['packed'] for f in allposes]);before=sum(len(f['packed']) for f in allposes)
 for frame in allposes:
  packed=frame.pop('packed');encoded=huffman_pack(packed,codes);assert huffman_unpack(encoded,len(packed),tree)==packed
  frame.pop('raw',None);frame.update(bytes=encoded.hex(),size=len(packed))
 materials=(previous['materials'] if previous else [])+palettes(catalog)[original_count:]
 data={'width':W,'height':H,'format':'Huffman-PackBits-I8-XOR','tree':tree,'fixed':fixed_values,'materials':materials,'profiles':profiles,'sources':metadata,'normalization':normalization}
 OUT.write_text(json.dumps(data,separators=(',',':'))+'\n');print('Huffman PackBits:',before,'bytes before entropy coding',flush=True);generate(False)
def generate(check):
 d=json.loads(OUT.read_text());catalog=json.loads(CAT.read_text());assert len(d['profiles'])==len(catalog) and [s['id'] for s in catalog]==list(range(len(catalog)))
 assert d['materials']==palettes(catalog),'Regenerate material palettes'
 for source in d['sources']:assert hashlib.sha256((ROOT/'firmware/assets/species-source'/f'{source["key"]}.png').read_bytes()).hexdigest()==source['sha256']
 blob=bytearray();frames=[];count=0
 for lineage in d['profiles']:
  bodies=[]
  for body in lineage:
   styles=[]
   for style in body:
    poses=[];base=None
    for pose in style:
     if pose is None:poses.append('{0,0,0,0}');continue
     b=bytes.fromhex(pose['bytes']);decoded=unpack(packed_frame(pose,d));assert len(decoded)==W*H
     if base is None:base=decoded;assert not pose['xor']
     poses.append('{'+f'{len(blob)},{pose["size"]},{len(b)},{int(pose["xor"])}'+'}');blob.extend(b);count+=1
    styles.append('{'+','.join(poses)+'}')
   bodies.append('{'+','.join(styles)+'}')
  frames.append('{'+','.join(bodies)+'}')
 lines=['/* Generated by generate-species-art.py. Original ImageGen PNGs retained. */','#pragma once','#include <stdint.h>','#include "../core/species_catalog.h"','typedef struct {uint32_t offset;uint16_t size,packed_size;uint8_t delta;} pd_species_frame;','static const char *const pd_species_names[]={' + ','.join(json.dumps(s['name']) for s in catalog)+'};','static const char *const pd_species_surface_names[]={'+','.join(json.dumps(v.get('materialName','Carnagione')) for v in catalog)+'};', 'static const uint8_t pd_species_has_hair[]={'+','.join('0' if s.get('hair') is False else '1' for s in catalog)+'};', 'static const char *const pd_species_hair_names[PD_SPECIES_COUNT][3]={'+','.join('{'+','.join(json.dumps(v) for v in s.get('hairNames',['Castani','Biondi','Scuri']))+'}' for s in catalog)+'};','static const char *const pd_species_skin_names[PD_SPECIES_COUNT][3]={'+','.join('{'+','.join(json.dumps(v) for v in s.get('skinNames',['Chiara','Media','Scura']))+'}' for s in catalog)+'};','static const char *const pd_species_style_names[PD_SPECIES_COUNT][3]={'+','.join('{'+','.join(json.dumps(v) for v in s.get('styles',['Corti','Lunghi','Rasati']))+'}' for s in catalog)+'};','static const uint32_t pd_species_fixed[256]={'+','.join(hex(v) for v in d['fixed'])+'};','static const uint32_t pd_species_materials[PD_SPECIES_COUNT][3][3][16]={']
 for lineage in d['materials']:lines.append('{'+','.join('{'+','.join('{'+','.join(hex(v) for v in shades)+'}' for shades in material)+'}' for material in lineage)+'},')
 lines+=['};','static const int16_t pd_species_huffman[][2]={'+','.join('{'+str(a)+','+str(b)+'}' for a,b in d['tree'])+'};','static const pd_species_frame pd_species_frames[PD_SPECIES_COUNT][2][3][4]={',',\n'.join(frames),'};','static const uint8_t pd_species_data[]={']
 for at in range(0,len(blob),24):lines.append(','.join(f'0x{x:02x}' for x in blob[at:at+24])+',')
 lines.append('};');content='\n'.join(lines)+'\n';path=ROOT/'firmware/assets/species_atlas.h'
 catalog_path=ROOT/'firmware/core/species_catalog.h';catalog_header=f'/* Generated from src/avatar-species.json. Stable IDs; append only. */\n#pragma once\n#define PD_SPECIES_COUNT {len(catalog)}\n'
 if check:
  assert path.read_text()==content,'Regenerate species_atlas.h'
  assert catalog_path.read_text()==catalog_header,'Regenerate species_catalog.h'
 else:path.write_text(content);catalog_path.write_text(catalog_header)
 print(f'Species: {count} frames, {len(blob)} compressed bytes, shared C/JS palettes and source hashes verified')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--export',action='store_true');p.add_argument('--check',action='store_true');a=p.parse_args()
 if a.export:export()
 else:generate(a.check)
