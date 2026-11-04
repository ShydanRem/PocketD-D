#!/usr/bin/env python3
"""Secondary UX captures and a complete native-view contact sheet, synthetic only."""
import hashlib,json,math,re,subprocess,zipfile
from pathlib import Path
from PIL import Image,ImageDraw
root=Path(__file__).resolve().parents[1];out=root/'firmware/tests/out/ui-build'
frames=[('ux107-wallet','Borsello: i cinque metalli'),('ux107-wallet-preview','Prima e dopo, senza cambio automatico'),('ux107-keys','Comandi e tasti configurabili'),('ux107-input-home','Collaudo: tocchi e gesti osservati')]
preview=Image.new('RGB',(932,1012),(7,21,30));draw=ImageDraw.Draw(preview)
for i,(name,title) in enumerate(frames):
 im=Image.open(out/(name+'.ppm'));assert im.size==(466,466);im.save(out/(name+'.png'))
 x,y=i%2*466,i//2*506;draw.text((x+12,y+12),title+' | sintetico',fill=(239,219,172));preview.paste(im,(x,y+40))
for name in ('ux107-wallet-edit','ux107-wallet-number','ux107-wallet-steps','ux107-wallet-max','ux107-key-picker','ux107-input-keys','ux107-resources','ux107-resource-wide','ux107-heading'):
 Image.open(out/(name+'.ppm')).save(out/(name+'.png'))
preview.save(root/'docs/previews/v107-restyle-finish.png')
declarations=re.search(r'typedef enum \{([^}]+)\} page_t;', (root/'firmware/main/main.c').read_text()).group(1).split(',')
assert all('=' not in x for x in declarations),'Explicit page IDs need an explicit capture map'
names=[x.strip() for x in declarations];captures=[];sheets=[]
for start in range(0,len(names),39):
 subset=names[start:start+39];sheet=Image.new('RGB',(936,math.ceil(len(subset)/6)*186),(7,21,30));labels=ImageDraw.Draw(sheet)
 for offset,name in enumerate(subset):
  index=start+offset;source=out/f'ux-page-{index:02d}.ppm';im=Image.open(source);assert im.size==(466,466)
  x,y=offset%6*156,offset//6*186;caption=f'{index:03d} {name}'
  labels.text((x+4,y+2),caption[:25],fill=(239,219,172));labels.text((x+4,y+14),caption[25:],fill=(239,219,172));sheet.paste(im.resize((152,152),Image.Resampling.LANCZOS),(x+2,y+32))
  captures.append(dict(index=index,page=name,file=source.name,sha256=hashlib.sha256(source.read_bytes()).hexdigest()))
 path=root.parent/f'PocketDD-UI-Audit-v0.107-{len(sheets)+1}.png';sheet.save(path);sheets.append(str(path))
manifest=dict(version=json.loads((root/'package.json').read_text())['version'],source_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=root,text=True).strip(),working_tree_dirty=bool(subprocess.check_output(['git','status','--porcelain'],cwd=root,text=True).strip()),synthetic=True,hardware_verified=False,view_count=len(captures),original_captures_included=False,scope='First native capture per page ID; scenario tests cover additional states. This is not aesthetic approval or physical usability verification.',contact_sheets=sheets,captures=captures)
(root.parent/'PocketDD-UI-Audit-v0.107.json').write_text(json.dumps(manifest,indent=2)+'\n')
print(f'{len(captures)} native page captures mapped; four contact sheets outside the PC ZIP; synthetic data, no hardware claim')

with zipfile.ZipFile(root.parent/'PocketDD-UI-Audit-v0.107.zip','w',zipfile.ZIP_DEFLATED) as archive:
 for path in sheets:archive.write(path,Path(path).name)
 archive.write(root.parent/'PocketDD-UI-Audit-v0.107.json','PocketDD-UI-Audit-v0.107.json')
 checklist=root/'docs/RESTYLE-CHECK.md'
 if checklist.exists():archive.write(checklist,'RESTYLE-CHECK.md')
 archive.writestr('LEGGIMI.txt','Quattro tavole con le 153 prime catture per ID pagina. Dati e input sintetici. Non prova usabilita fisica o approvazione estetica. Gli originali PPM restano in firmware/tests/out/ui-build; il JSON include nomi e SHA256 per riprodurre il confronto.\n')
