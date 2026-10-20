#!/usr/bin/env python3
"""Export actual host-LVGL firmware frames as PNGs and a four-screen contact sheet.
Requires Pillow. Run npm run firmware:test first. This is not a hardware capture.
"""
from pathlib import Path
from PIL import Image

base = Path(__file__).resolve().parents[1]/'firmware/tests/out/ui-build'
names = ['hero', 'dice', 'sheet', 'resources']
gallery = Image.new('RGB', (466*2, 466*2), 'black')
for i, name in enumerate(names):
    source = base/(name+'.ppm')
    if not source.is_file():
        raise SystemExit('Run npm run firmware:test first')
    frame = Image.open(source).convert('RGB')
    frame.save(base/(name+'.png'))
    gallery.paste(frame, ((i % 2)*466, (i // 2)*466))
output = base/'device-preview.png'
gallery.save(output)
print(output)
for name in ['appearance','appearance-still','hero-imported']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
frames=[Image.open(p).convert('RGB') for p in sorted(base.glob('avatar-frame-*.ppm'))]
if frames:
    frames[0].save(base/'avatar-preview.gif',save_all=True,append_images=frames[1:],duration=200,loop=0,disposal=2,optimize=True)
    print(base/'avatar-preview.gif')
presets=Image.new('RGB',(466*3,466),'black')
for i in range(3):
    source=base/f'avatar-preset-{i}.ppm'
    if not source.is_file():break
    presets.paste(Image.open(source),(i*466,0))
else:
    presets.save(base/'avatar-presets.png');print(base/'avatar-presets.png')
checks=['skills-first','saves','check-detail','skill-result']
for name in checks+['skills-last','check-blocked','save-detail','skills-legacy','sheet-trained']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in checks):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(checks):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'skills-preview.png');print(base/'skills-preview.png')
combat=['inventory','combat-detail','attack-critical','damage-eighteen']
for name in combat+['combat','weapon','weapon-damage','attack-miss','item-confirm','healing','inventory-last','inventory-legacy','item-long-name','weapon-empty']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in combat):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(combat):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'inventory-preview.png');print(base/'inventory-preview.png')

magic=['magic-list','magic-detail','magic-text','keys']
for name in magic+['magic-confirm','magic-status','magic-slots','magic-recovery','magic-last','magic-legacy']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in magic):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(magic):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'magic-preview.png');print(base/'magic-preview.png')

rests=['rest-home','rest-long','rest-regain','hit-healing']
for name in rests+['rest-short','rest-settings','hit-dice','hit-confirm','rest-legacy','hit-last']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in rests):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(rests):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'rest-preview.png');print(base/'rest-preview.png')

status=['status-home','conditions','death-tracker','death-twenty']
for name in status+['status-poison','condition-detail','condition-picker','death-confirm','death-result','death-edit','death-stable','death-three','death-legacy','condition-last','condition-long']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in status):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(status):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'status-preview.png');print(base/'status-preview.png')

for name in ['screen-settings','screen-configured','screen-error']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
