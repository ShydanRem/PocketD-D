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

# Explicit local modifier screens from the actual firmware UI.
effects=['effects-list','effect-editor','effect-confirm','effect-cap']
for name in effects+['effects-empty','effects-hero','effect-invalid','effects-last','effects-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in effects):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(effects):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'effects-preview.png');print(base/'effects-preview.png')

# Round-duration views from real host LVGL, with explicit confirmation.
durations=['duration-list','duration-editor','duration-round','duration-expired']
for name in durations+['duration-manual','duration-all-expired','duration-zero','duration-invalid']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in durations):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(durations):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'durations-preview.png');print(base/'durations-preview.png')

# Incoming damage and manually configured defences; actual host firmware frames.
damage=['damage-home','defence-editor','damage-preview','damage-result']
for name in damage+['damage-unknown','defence-confirm','damage-concentration','damage-immune','defences-list','damage-combined','damage-massive','damage-zero','defences-last','damage-maximum','defences-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in damage):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(damage):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'damage-overview.png');print(base/'damage-overview.png')

# Committed concentration checks, including frozen retry and capped-DC option.
concentration=['concentration-setup','concentration-success','concentration-save-error','concentration-failure']
for name in concentration+['concentration-extra','concentration-flight','concentration-summary','concentration-legacy','concentration-no-canvas','concentration-cap30','concentration-capped-result']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in concentration):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(concentration):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'concentration-preview.png');print(base/'concentration-preview.png')

# Explicit concentration bindings; these are host renders, not hardware captures.
links=['link-editor','link-confirm','link-stop','link-damage']
for name in links+['link-recast','link-rest','link-unavailable']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in links):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(links):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'links-preview.png');print(base/'links-preview.png')

# Additional timed defences and a shared round confirmation, rendered by host LVGL.
temporary=['temporary-editor','temporary-confirm','temporary-round-defences','temporary-result']
for name in temporary+['temporary-duration','temporary-link','temporary-list','temporary-round','temporary-round-last','temporary-round-full','temporary-stop-maximum','temporary-expiry','temporary-damage','temporary-stop','temporary-unknown','temporary-invalid','temporary-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in temporary):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(temporary):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'temporary-defences-preview.png');print(base/'temporary-defences-preview.png')

# A single mixed impact: composition, canonical type groups, total and committed result.
mixed=['mixed-parts','mixed-types','mixed-preview','mixed-result']
for name in mixed+['mixed-editor','mixed-rounding','mixed-unknown','mixed-impact','mixed-save-error','mixed-concentration','mixed-snapshot','mixed-parts-last','mixed-massive','mixed-types-last','mixed-immune']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in mixed):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(mixed):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'mixed-damage-preview.png');print(base/'mixed-damage-preview.png')

# Explicit defence predicates and partial source facts; host firmware renders.
conditional=['conditional-rule','conditional-review','conditional-source','conditional-groups']
for name in conditional+['conditional-list','conditional-unknown','conditional-source-unknown','conditional-source-silver','conditional-excluded','conditional-applied','conditional-snapshot','conditional-extra','conditional-confirm','conditional-review-extra','conditional-expiry','conditional-long','conditional-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in conditional):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(conditional):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'conditional-defences-preview.png');print(base/'conditional-defences-preview.png')

# Stable extra slots with separate drafts, timers and concentration bindings.
multiple=['multiple-picker','multiple-review','multiple-round','multiple-impact']
for name in multiple+['multiple-editor','multiple-rules','multiple-list','multiple-surviving','multiple-import','multiple-full-round','multiple-last']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in multiple):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(multiple):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'multiple-defences-preview.png');print(base/'multiple-defences-preview.png')

# Explicit pre-defence allocations and a single post-defence impact reduction.
reductions=['reduction-before','reduction-after','reduction-math','reduction-result']
for name in reductions+['reduction-preview','reduction-groups','reduction-mixed','reduction-unknown','reduction-zero','reduction-retry','reduction-snapshot','reduction-last','reduction-maximum-group','reduction-maximum','reduction-maximum-damage']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in reductions):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(reductions):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'reductions-preview.png');print(base/'reductions-preview.png')

# Persistent reduction profiles, explicit shared budgets and impact-time snapshots.
profiles=['profile-details','profile-allocation','profile-before-math','profile-impact']
for name in profiles+['profile-math','profile-list','profile-view','profile-confirm','profile-rule','profile-duration','profile-excluded','profile-snapshot','profile-round','profile-expiry','profile-after','profile-import','profile-last-round','profile-invalid','profile-long']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in profiles):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(profiles):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'reduction-profiles-preview.png');print(base/'reduction-profiles-preview.png')

# Resource costs and explicit reaction lifecycle, captured from the host firmware.
costs=['cost-editor','cost-preview','reaction-turn','cost-snapshot']
for name in costs+['cost-confirm','cost-view','cost-retry','cost-zero','cost-blocked','reaction-used','reaction-ready','cost-power','cost-long','cost-power-preview','reaction-external','cost-import','cost-wide','cost-wide-confirm','cost-wide-preview']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in costs):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(costs):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'profile-costs-preview.png');print(base/'profile-costs-preview.png')

# Fixed-level ordinary/pact costs; actual LVGL host frames, not hardware captures.
slots=['slot-cost-editor','slot-cost-preview','slot-cost-pact','slot-cost-short-rest']
for name in slots+['slot-cost-confirm','slot-cost-view','slot-cost-retry','slot-cost-snapshot','slot-cost-ninth','slot-cost-blocked','slot-cost-empty','slot-cost-zero','slot-cost-long-rest','slot-cost-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in slots):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(slots):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'slot-costs-preview.png');print(base/'slot-costs-preview.png')

# Explicit own-turn expiry, including atomic reaction/HP changes; host LVGL only.
turns=['turn-effect-editor','turn-start-preview','turn-end-details','turn-last-details']
for name in turns+['turn-effect-confirm','turn-defence-editor','turn-profile-editor','turn-profile-confirm','turn-home','turn-start-details','turn-start-applied','turn-end-preview','turn-invalid','turn-import','turn-pending-impact']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in turns):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(turns):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'turn-durations-preview.png');print(base/'turn-durations-preview.png')

# Explicit elapsed game time with minute precision; actual host LVGL frames.
time=['time-home','time-defence-hours','time-cap-preview','time-details']
for name in time+['time-effect-editor','time-profile-confirm','time-preview','time-maximum','time-invalid','time-wide-details','time-last-details','time-import']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in time):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(time):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'elapsed-time-preview.png');print(base/'elapsed-time-preview.png')

charges=['charges-edit','charges-confirm','charges-cost-preview','charges-rest-short']
charge_details=['charges-item','charges-retry','charges-unavailable','charges-long-name','charges-cost-editor','charges-cost-receipt','charges-cost-blocked','charges-rest-long']
for name in charges+charge_details:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in charges):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(charges):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'item-charges-preview.png');print(base/'item-charges-preview.png')

multi=['multi-cost-editor-third','multi-cost-confirm-third','multi-cost-preview-third','multi-cost-missing-second']
multi_details=['multi-cost-editor-second','multi-cost-confirm-first','multi-cost-view','multi-cost-preview-first','multi-cost-preview-second','multi-cost-retry','multi-cost-receipt','multi-cost-blocked','multi-cost-long-name','multi-cost-import']
for name in multi+multi_details:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in multi):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(multi):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'multiple-costs-preview.png');print(base/'multiple-costs-preview.png')

recharge=['recharge-setup','recharge-confirm','recharge-result','recharge-receipt']
recharge_details=['recharge-retry','recharge-flight','recharge-full','recharge-zero','recharge-maximum-setup','recharge-maximum-result','recharge-maximum-receipt','recharge-unavailable','recharge-no-canvas']
for name in recharge+recharge_details:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in recharge):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(recharge):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'item-recharge-preview.png');print(base/'item-recharge-preview.png')

conditions=['roll-conditions-editor','roll-conditions-context-unknown','roll-conditions-auto-fail','roll-conditions-dex-save']
condition_details=['roll-conditions-confirm','roll-conditions-blind-block','roll-conditions-auto-detail','roll-conditions-no-sight','roll-conditions-exclude','roll-conditions-fear-visible','roll-conditions-fear-roll','roll-conditions-attack','roll-conditions-cancelled','roll-conditions-attack-detail','roll-conditions-import']
for name in conditions+condition_details:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in conditions):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(conditions):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'roll-conditions-preview.png');print(base/'roll-conditions-preview.png')


feedback_names=['feedback-heal','feedback-damage','fantasy-inventory','fantasy-magic']
feedback_details=['feedback-temporary','feedback-static','feedback-save-error','feedback-spend','feedback-recover','feedback-expired']
for name in feedback_names+feedback_details:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in feedback_names):
    sheet=Image.new('RGB',(932,932),'black')
    for i,name in enumerate(feedback_names):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'fantasy-feedback-preview.png');print(base/'fantasy-feedback-preview.png')
frames=[Image.open(p).convert('RGB') for p in sorted(base.glob('feedback-frame-*.ppm'))]
expired=base/'feedback-expired.ppm'
if frames and expired.is_file():
    frames += [frames[-1].copy() for _ in range(20)]
    frames += [Image.open(expired).convert('RGB') for _ in range(8)]
    frames[0].save(base/'feedback-preview.gif',save_all=True,append_images=frames[1:],duration=66,loop=0,disposal=2,optimize=True)
    print(base/'feedback-preview.gif')

# Production v0.48 views; separate from the standalone companion demonstration.
home_views=['hero','home-menu','home-vitals','feedback-heal']
for name in home_views+['home-no-weapon','home-import-error','home-imported','home-effects','home-vitals-effects','home-keys','home-zero']:
    source=base/(name+'.ppm')
    if source.is_file():Image.open(source).save(base/(name+'.png'))
if all((base/(name+'.ppm')).is_file() for name in home_views):
    sheet=Image.new('RGB',(466*2,466*2),'black')
    for i,name in enumerate(home_views):sheet.paste(Image.open(base/(name+'.ppm')),((i%2)*466,(i//2)*466))
    sheet.save(base/'home-integration-preview.png');print(base/'home-integration-preview.png')
