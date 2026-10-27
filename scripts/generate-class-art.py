#!/usr/bin/env python3
"""Normalize complete ImageGen class sprites; no procedural repainting.
Normal builds verify committed artifacts using only Python's standard library.
--export requires Pillow/numpy for segmentation and nearest-neighbour sampling.
"""
import argparse,hashlib,json,struct,zlib,zipfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
ART=ROOT/'firmware/assets/class-source'
PACK=ROOT/'public/assets/classes.pdc'
META=ROOT/'docs/design/classes/pack.json'
ZIP=ROOT/'public/assets/PocketDD-SD.zip'
C=21*2*12;FRAME=64*64*3

def check():
 b=PACK.read_bytes();meta=json.loads(META.read_text());assert len(b)==32+C*4+C*FRAME
 assert b[:4]==b'PDCL' and struct.unpack_from('<HBBHBBIII',b,4)==(1,64,64,21,2,12,FRAME,C,len(b))
 assert zlib.crc32(b[:28])==struct.unpack_from('<I',b,28)[0]
 assert zlib.crc32(b[32:32+C*4])==struct.unpack_from('<I',b,24)[0]
 for i in range(C):
  f=b[32+C*4+i*FRAME:32+C*4+(i+1)*FRAME];assert zlib.crc32(f)==struct.unpack_from('<I',b,32+i*4)[0]
  alpha=f[8192:];assert set(alpha)<={0,255} and 255 in alpha
 assert hashlib.sha256(b).hexdigest()==meta['sha256']
 assert meta['speciesCatalogSha256']==hashlib.sha256((ROOT/'src/avatar-species.json').read_bytes()).hexdigest()
 assert meta['classesCatalogSha256']==hashlib.sha256((ROOT/'src/avatar-classes.json').read_bytes()).hexdigest()
 for s in meta['sources']:assert hashlib.sha256((ART/(s['key']+'.png')).read_bytes()).hexdigest()==s['sha256']
 assert [x['id'] for x in json.loads((ROOT/'src/avatar-classes.json').read_text())]==list(range(13))
 with zipfile.ZipFile(ZIP) as z:
  assert z.testzip() is None and set(z.namelist())=={'pocketdd/classes.pdc','pocketdd/manifest.json','LEGGIMI.txt'}
  assert z.read('pocketdd/classes.pdc')==b and json.loads(z.read('pocketdd/manifest.json'))['sha256']==meta['sha256']
 print(f'Class looks: {C} complete RGB565A8 sprites, header/table/frame CRC, source SHA256, SD zip and stable IDs verified')

def export():
 from PIL import Image,ImageDraw
 import numpy as np
 import importlib.util
 spec=importlib.util.spec_from_file_location('species_art',ROOT/'scripts/generate-species-art.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
 catalog=json.loads((ROOT/'src/avatar-species.json').read_text());frames=[];sources=[];tiles=[]
 for s in catalog:
  path=ART/(s['key']+'.png');im=Image.open(path).convert('RGBA');arr=np.asarray(im);a=arr[:,:,3]>=250;h,w=a.shape
  assert np.count_nonzero(a[0])<=5 and np.count_nonzero(a[-1])<=5,('Clipped outer row',s['key'])
  ys=mod.cuts(a.sum(1).tolist(),4,h)
  bounds=[]
  for row in range(4):
   xs=mod.cuts(a[ys[row]:ys[row+1]].sum(0).tolist(),6,w)
   for col in range(6):
    x0,x1=xs[col:col+2];y0,y1=ys[row:row+2];yy,xx=np.where(a[y0:y1,x0:x1]);assert len(xx)>100
    box=(x0+int(xx.min()),y0+int(yy.min()),x0+int(xx.max())+1,y0+int(yy.max())+1)
    xx2=x0+xx[yy>=yy.max()-max(2,(box[3]-box[1])//30)];anchor=(xx2.min()+xx2.max()+1)/2
    assert box[1]>0 and box[3]<h,('Sprite clipped',s['key'],row,col)
    bounds.append({'box':box,'anchor':float(anchor)})
  for body in range(2):
   poses=bounds[body*12:(body+1)*12]
   scale=min(min(62,s['height']+4)/max(p['box'][3]-p['box'][1] for p in poses),31/max(p['anchor']-p['box'][0] for p in poses),31/max(p['box'][2]-p['anchor'] for p in poses))
   for outfit,b in enumerate(poses,1):
    x0,y0,x1,y1=b['box'];width=max(1,round((x1-x0)*scale));height=max(1,round((y1-y0)*scale));left=round(32-(b['anchor']-x0)*scale);top=64-height
    assert left>=0 and left+width<=64 and top>=0,(s['key'],body,outfit,left,width,top)
    cut=im.crop(b['box']).resize((width,height),Image.Resampling.NEAREST);tile=Image.new('RGBA',(64,64));tile.paste(cut,(left,top))
    p=np.asarray(tile).copy();p[:,:,3]=np.where(p[:,:,3]>=128,255,0)
    rgb=p[:,:,:3].astype(np.uint16);v=((rgb[:,:,0]>>3)<<11)|((rgb[:,:,1]>>2)<<5)|(rgb[:,:,2]>>3);v[p[:,:,3]==0]=0
    frames.append(v.astype('<u2').tobytes()+p[:,:,3].tobytes());tiles.append(Image.fromarray(p))
  sources.append({'key':s['key'],'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'layout':'6 columns x 4 rows; masculine 12, feminine 12','bounds':bounds})
 table=b''.join(struct.pack('<I',zlib.crc32(f)) for f in frames);total=32+len(table)+C*FRAME
 header=struct.pack('<4sHBBHBBIIII',b'PDCL',1,64,64,21,2,12,FRAME,C,total,zlib.crc32(table));header+=struct.pack('<I',zlib.crc32(header))
 b=header+table+b''.join(frames);assert len(b)==total
 PACK.parent.mkdir(parents=True,exist_ok=True);PACK.write_bytes(b)
 meta={'schema':1,'format':'PDCL','size':total,'sha256':hashlib.sha256(b).hexdigest(),'sprites':C,'alphaTrim':250,'binaryAlpha':128,'speciesCatalogSha256':hashlib.sha256((ROOT/'src/avatar-species.json').read_bytes()).hexdigest(),'classesCatalogSha256':hashlib.sha256((ROOT/'src/avatar-classes.json').read_bytes()).hexdigest(),'poses':'neutral only; scene motion/reaction remain active','source':'ImageGen, original species references and corrected class concept board','sources':sources};META.write_text(json.dumps(meta,indent=2)+'\n')
 info={k:v for k,v in meta.items() if k!='sources'}
 with zipfile.ZipFile(ZIP,'w',zipfile.ZIP_DEFLATED) as z:
  def put(name,data):
   entry=zipfile.ZipInfo(name,(2026,10,27,0,0,0));entry.compress_type=zipfile.ZIP_DEFLATED;z.writestr(entry,data)
  put('pocketdd/classes.pdc',b);put('pocketdd/manifest.json',json.dumps(info,indent=2)+'\n');put('LEGGIMI.txt','PocketD-D 0.70 - Look delle classi\n\nEstrai la cartella pocketdd nella radice di una SD FAT32 gia preparata.\nNon formattare una scheda contenente dati per installare questo pacchetto.\nInserisci la SD a dispositivo spento e avvia PocketD-D.\nAspetto > Modifica > Classe: scegli il look, Anteprima, Conferma.\nRileggi riprova la lettura dopo aver inserito una scheda.\nSenza scheda o con file non valido viene mostrato lo stile originale.\nIl preset usa colori/taglio propri; le personalizzazioni originali restano salvate.\nNon modifica valori, inventario, salvataggi o Beyond.\n504 look: 12 classi x 21 specie x 2 silhouette. Un fotogramma neutro per look.\nArtificiere e sottoclassi non inclusi. Collaudo SD fisico ancora necessario.\n')
 # Pixel-grid audit sheets, never substituted for actual firmware screenshots.
 for body in range(2):
  out=Image.new('RGB',(12*132,21*148),'#10222c');d=ImageDraw.Draw(out)
  for species in range(21):
   for outfit in range(12):
    t=tiles[(species*2+body)*12+outfit].resize((128,128),Image.Resampling.NEAREST);out.paste(t,(outfit*132,species*148+20),t)
   d.text((4,species*148+4),catalog[species]['name'])
  out.save(ROOT/f'docs/design/classes/catalog-{body}.png')
 check()
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--export',action='store_true');args=p.parse_args();export() if args.export else check()
