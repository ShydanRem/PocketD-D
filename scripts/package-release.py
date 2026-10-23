#!/usr/bin/env python3
"""Archive tracked sources, built PC app, firmware binaries and third-party licenses.
Never include browser state, source-character JSON, .git, credentials or .deps.
"""
import argparse
import hashlib
import io
import json
from pathlib import Path
import subprocess
import tarfile
import zipfile

root=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('output',type=Path)
args=parser.parse_args()
def git(*params):
    return subprocess.check_output(['git','-C',str(root),*params])
if git('status','--porcelain').strip():
    raise SystemExit('Commit the source changes before packaging')
revision=git('rev-parse','HEAD').decode().strip()
version=json.loads((root/'package.json').read_text())['version']
build=root/'firmware/build'
project=json.loads((build/'project_description.json').read_text())
if project['project_version']!=version or project['target']!='esp32s3':
    raise SystemExit('Firmware build does not match the release version/target')
folder=f'PocketD-D-v{version}-{revision[:7]}/'
binary_files=['pocketdd.bin','bootloader/bootloader.bin','partition_table/partition-table.bin','flash_args','flasher_args.json']
for name in binary_files:
    if not (build/name).is_file():raise SystemExit(f'Missing {name}')
if not (root/'dist/index.html').is_file():raise SystemExit('Run npm run build first')
args.output.parent.mkdir(parents=True,exist_ok=True)
with zipfile.ZipFile(args.output,'w',zipfile.ZIP_DEFLATED) as out:
    source=tarfile.open(fileobj=io.BytesIO(git('archive','--format=tar','HEAD')))
    for member in source:
        if member.isfile():out.writestr(folder+member.name,source.extractfile(member).read())
    for path in (root/'dist').rglob('*'):
        if path.is_file():out.write(path,folder+str(path.relative_to(root)))
    hashes={}
    for name in binary_files:
        data=(build/name).read_bytes()
        out.writestr(folder+'firmware-binaries/'+name,data)
        hashes[name]=hashlib.sha256(data).hexdigest()
    # Collect dependency and IDF license/notice files only; no source data or settings.
    dep_root=root/'firmware/.deps'
    license_roots=[('ESP-IDF',Path(project['idf_path']))]+[(p.name,p) for p in dep_root.iterdir() if p.is_dir()]
    for label,directory in license_roots:
        for path in directory.rglob('*'):
            if not path.is_file() or '.git' in path.parts:continue
            if not path.name.lower().startswith(('license','licence','notice','copying','copyright')):continue
            if path.suffix.lower() in ['.c','.h','.py','.cmake']:continue
            out.write(path,folder+'firmware-binaries/licenses/'+label+'/'+str(path.relative_to(directory)))
    info={'version':version,'commit':revision,'board':'ESP32-S3-Touch-AMOLED-1.75 (not 1.75C)',
          'idf':subprocess.check_output(['git','-C',project['idf_path'],'describe','--tags','--always'],text=True).strip(),'target':project['target'],'hardware_tested':False,'files_sha256':hashes,
          'notes':'Compiled and host-tested. Requires bundled partition layout (pdd_state); legacy NVS retained. Physical display/touch/buttons/USB/power tests pending. See docs/FIRMWARE.md.'}
    out.writestr(folder+'firmware-binaries/release-info.json',json.dumps(info,indent=2)+'\n')
    preview=root/'firmware/tests/out/ui-build'
    for name in ['home-menu','home-vitals','home-no-weapon','home-import-error','home-imported','home-effects','home-vitals-effects','home-keys','home-zero','feedback-heal','feedback-damage','fantasy-inventory','fantasy-magic','feedback-temporary','feedback-static','feedback-save-error','feedback-spend','feedback-recover','feedback-expired','roll-conditions-editor', 'roll-conditions-context-unknown', 'roll-conditions-auto-fail', 'roll-conditions-dex-save', 'roll-conditions-confirm', 'roll-conditions-blind-block', 'roll-conditions-auto-detail', 'roll-conditions-no-sight', 'roll-conditions-exclude', 'roll-conditions-fear-visible', 'roll-conditions-fear-roll', 'roll-conditions-attack', 'roll-conditions-cancelled', 'roll-conditions-attack-detail', 'roll-conditions-import','recharge-setup', 'recharge-confirm', 'recharge-result', 'recharge-receipt', 'recharge-retry', 'recharge-flight', 'recharge-full', 'recharge-zero', 'recharge-maximum-setup', 'recharge-maximum-result', 'recharge-maximum-receipt', 'recharge-unavailable', 'recharge-no-canvas','multi-cost-editor-third', 'multi-cost-confirm-third', 'multi-cost-preview-third', 'multi-cost-missing-second', 'multi-cost-editor-second', 'multi-cost-confirm-first', 'multi-cost-view', 'multi-cost-preview-first', 'multi-cost-preview-second', 'multi-cost-retry', 'multi-cost-receipt', 'multi-cost-blocked', 'multi-cost-long-name', 'multi-cost-import','charges-item','charges-edit','charges-confirm','charges-retry','charges-unavailable','charges-long-name','charges-cost-editor','charges-cost-preview','charges-cost-receipt','charges-cost-blocked','charges-rest-short','charges-rest-long','hero','hero-maximum','hero-zero','hero-storage-error','hero-imported','appearance','appearance-still','avatar-preset-0','avatar-preset-1','avatar-preset-2','sheet','dice','dice-advantage','dice-disadvantage','dice-twenty','pool-setup','pool-picker','pool-configured','pool-six','pool-maximum','pool-negative','pool-zero','pool-d4','pool-d6','pool-d8','pool-d10','pool-d12','pool-d20','resources','confirm']:
        image=preview/(name+'.ppm')
        if image.is_file():out.write(image,folder+'firmware-host-preview/'+image.name)
    for name in ['home-integration-preview.png','home-menu.png','home-vitals.png','home-no-weapon.png','home-import-error.png','home-imported.png','home-effects.png','home-vitals-effects.png','home-keys.png','home-zero.png','fantasy-feedback-preview.png','feedback-preview.gif','feedback-heal.png','feedback-damage.png','fantasy-inventory.png','fantasy-magic.png','feedback-temporary.png','feedback-static.png','feedback-save-error.png','feedback-spend.png','feedback-recover.png','feedback-expired.png', 'roll-conditions-preview.png', 'roll-conditions-editor.png', 'roll-conditions-context-unknown.png', 'roll-conditions-auto-fail.png', 'roll-conditions-dex-save.png', 'roll-conditions-confirm.png', 'roll-conditions-blind-block.png', 'roll-conditions-auto-detail.png', 'roll-conditions-no-sight.png', 'roll-conditions-exclude.png', 'roll-conditions-fear-visible.png', 'roll-conditions-fear-roll.png', 'roll-conditions-attack.png', 'roll-conditions-cancelled.png', 'roll-conditions-attack-detail.png', 'roll-conditions-import.png','item-recharge-preview.png', 'recharge-setup.png', 'recharge-confirm.png', 'recharge-result.png', 'recharge-receipt.png', 'recharge-retry.png', 'recharge-flight.png', 'recharge-full.png', 'recharge-zero.png', 'recharge-maximum-setup.png', 'recharge-maximum-result.png', 'recharge-maximum-receipt.png', 'recharge-unavailable.png', 'recharge-no-canvas.png','multiple-costs-preview.png', 'multi-cost-editor-third.png', 'multi-cost-confirm-third.png', 'multi-cost-preview-third.png', 'multi-cost-missing-second.png', 'multi-cost-editor-second.png', 'multi-cost-confirm-first.png', 'multi-cost-view.png', 'multi-cost-preview-first.png', 'multi-cost-preview-second.png', 'multi-cost-retry.png', 'multi-cost-receipt.png', 'multi-cost-blocked.png', 'multi-cost-long-name.png', 'multi-cost-import.png','item-charges-preview.png','charges-item.png','charges-edit.png','charges-confirm.png','charges-retry.png','charges-unavailable.png','charges-long-name.png','charges-cost-editor.png','charges-cost-preview.png','charges-cost-receipt.png','charges-cost-blocked.png','charges-rest-short.png','charges-rest-long.png','elapsed-time-preview.png','time-home.png','time-defence-hours.png','time-cap-preview.png','time-details.png','time-effect-editor.png','time-profile-confirm.png','time-preview.png','time-maximum.png','time-invalid.png','time-wide-details.png','time-last-details.png','time-import.png','turn-durations-preview.png','turn-effect-editor.png','turn-start-preview.png','turn-end-details.png','turn-last-details.png','turn-effect-confirm.png','turn-defence-editor.png','turn-profile-editor.png','turn-profile-confirm.png','turn-home.png','turn-start-details.png','turn-start-applied.png','turn-end-preview.png','turn-invalid.png','turn-import.png','turn-pending-impact.png','slot-costs-preview.png','slot-cost-editor.png','slot-cost-preview.png','slot-cost-pact.png','slot-cost-short-rest.png','slot-cost-confirm.png','slot-cost-view.png','slot-cost-retry.png','slot-cost-snapshot.png','slot-cost-ninth.png','slot-cost-blocked.png','slot-cost-empty.png','slot-cost-zero.png','slot-cost-long-rest.png','slot-cost-import.png','profile-costs-preview.png','cost-editor.png','cost-preview.png','reaction-turn.png','cost-snapshot.png','cost-confirm.png','cost-view.png','cost-retry.png','cost-zero.png','cost-blocked.png','reaction-used.png','reaction-ready.png','cost-power.png','cost-long.png','cost-power-preview.png','reaction-external.png','cost-import.png','cost-wide.png','cost-wide-confirm.png','cost-wide-preview.png','reduction-profiles-preview.png','profile-details.png','profile-allocation.png','profile-before-math.png','profile-math.png','profile-impact.png','profile-list.png','profile-view.png','profile-confirm.png','profile-rule.png','profile-duration.png','profile-excluded.png','profile-snapshot.png','profile-round.png','profile-expiry.png','profile-after.png','profile-import.png','profile-last-round.png','profile-invalid.png','profile-long.png','reductions-preview.png','reduction-before.png','reduction-after.png','reduction-math.png','reduction-result.png','reduction-preview.png','reduction-groups.png','reduction-mixed.png','reduction-unknown.png','reduction-zero.png','reduction-retry.png','reduction-snapshot.png','reduction-last.png','reduction-maximum-group.png','reduction-maximum.png','reduction-maximum-damage.png','multiple-defences-preview.png','multiple-picker.png','multiple-review.png','multiple-round.png','multiple-impact.png','multiple-editor.png','multiple-rules.png','multiple-list.png','multiple-surviving.png','multiple-import.png','multiple-full-round.png','multiple-last.png','conditional-defences-preview.png','conditional-rule.png','conditional-review.png','conditional-source.png','conditional-groups.png','conditional-list.png','conditional-unknown.png','conditional-source-unknown.png','conditional-source-silver.png','conditional-excluded.png','conditional-applied.png','conditional-snapshot.png','conditional-extra.png','conditional-confirm.png','conditional-review-extra.png','conditional-expiry.png','conditional-long.png','conditional-import.png','mixed-damage-preview.png','mixed-parts.png','mixed-types.png','mixed-preview.png','mixed-result.png','mixed-editor.png','mixed-rounding.png','mixed-unknown.png','mixed-impact.png','mixed-save-error.png','mixed-concentration.png','mixed-snapshot.png','mixed-parts-last.png','mixed-massive.png','mixed-types-last.png','mixed-immune.png','temporary-defences-preview.png','temporary-editor.png','temporary-confirm.png','temporary-round-defences.png','temporary-result.png','temporary-duration.png','temporary-link.png','temporary-list.png','temporary-round.png','temporary-round-last.png','temporary-round-full.png','temporary-stop-maximum.png','temporary-expiry.png','temporary-damage.png','temporary-stop.png','temporary-unknown.png','temporary-invalid.png','temporary-import.png','links-preview.png','link-editor.png','link-confirm.png','link-stop.png','link-damage.png','link-recast.png','link-rest.png','link-unavailable.png','concentration-preview.png','concentration-setup.png','concentration-success.png','concentration-save-error.png','concentration-failure.png','concentration-extra.png','concentration-flight.png','concentration-summary.png','concentration-legacy.png','concentration-no-canvas.png','concentration-cap30.png','concentration-capped-result.png','damage-overview.png','damage-preview.png','damage-home.png','defence-editor.png','damage-result.png','damage-unknown.png','defence-confirm.png','damage-concentration.png','damage-immune.png','defences-list.png','damage-combined.png','damage-massive.png','damage-zero.png','defences-last.png','damage-maximum.png','defences-import.png','durations-preview.png','duration-list.png','duration-editor.png','duration-round.png','duration-expired.png','duration-manual.png','duration-all-expired.png','duration-zero.png','duration-invalid.png','effects-preview.png','effects-list.png','effect-editor.png','effect-confirm.png','effect-cap.png','effects-empty.png','effects-hero.png','effect-invalid.png','effects-last.png','effects-import.png','screen-settings.png','screen-configured.png','screen-error.png','status-preview.png','status-home.png','conditions.png','death-tracker.png','death-twenty.png','status-poison.png','condition-detail.png','condition-picker.png','death-confirm.png','death-result.png','death-edit.png','death-stable.png','death-three.png','death-legacy.png','condition-last.png','condition-long.png','rest-preview.png','rest-home.png','rest-long.png','rest-regain.png','hit-healing.png','rest-short.png','rest-settings.png','hit-dice.png','hit-confirm.png','rest-legacy.png','hit-last.png','dice-preview.gif','pool-preview.gif','device-preview.png','hero.png','dice.png','sheet.png','resources.png','avatar-preview.gif','avatar-presets.png','appearance.png','appearance-still.png','hero-imported.png','magic-preview.png','magic-list.png','magic-detail.png','magic-text.png','keys.png','magic-confirm.png','magic-status.png','magic-slots.png','magic-recovery.png','magic-last.png','magic-legacy.png','inventory-preview.png','inventory.png','weapon.png','combat.png','combat-detail.png','attack-critical.png','weapon-damage.png','attack-miss.png','item-confirm.png','healing.png','damage-eighteen.png','inventory-last.png','inventory-legacy.png','item-long-name.png','weapon-empty.png','skills-preview.png','skills-first.png','skills-last.png','saves.png','check-detail.png','check-blocked.png','skill-result.png','save-detail.png','skills-legacy.png','sheet-trained.png']:
        animation=preview/name
        if animation.is_file():out.write(animation,folder+'firmware-host-preview/'+name)
    # Illustrated avatar production captures (synthetic character data).
    avatar_previews=['avatar-illustrated-preview.png','avatar-reactions.gif']
    avatar_previews += [f'avatar-live-{style}-{pose}.{ext}' for style in range(3)
                        for pose in ('idle','blink','heal','hurt') for ext in ('png','ppm')]
    for name in avatar_previews:
        image=preview/name
        if not image.is_file():raise SystemExit(f'Missing avatar preview: {name}')
        out.write(image,folder+'firmware-host-preview/'+name)
    # Complete device UI review catalog; all entries are native host captures.
    ui_manifest=json.loads((preview/'ui-views.json').read_text())
    ui_files=['ui-views.json','ui-catalog.html','ui-overview.png','ui-all-views.png','ui-transition.gif']
    ui_files += [r['image'] for r in ui_manifest['views']]
    ui_files += ['wallet-preview.png']+[f'wallet-{name}.png' for name in
                 ('unknown','zero','edit','confirm','save-error','home','gold','maximum','import','confirm-zero')]
    ui_files += ['history-preview.png']+[f'history-{name}.png' for name in
                 ('empty','detail','unsaved-roll','pending','unsaved-detail','discard','eighteen','last-dice','full','oldest','key','import-empty')]
    ui_files += ['backup-preview.png']+[f'backup-{name}.png' for name in
                 ('home','send','send-error','sent','timeout','restore','history-error','state-error','restored')]
    ui_files += ['input-check-preview.png']+[f'input-check-{name}.png' for name in
                 ('entry','home','touch','touch-2','touch-3','touch-4','touch-5','touch-done','keys','keys-done','unavailable','maximum','timeout')]
    ui_files += ['death-damage-preview.png']+[f'death-damage-{name}.png' for name in
                 ('choice','save-error','result','critical','stable','instant','instant-result',
                  'blocked-0','blocked-1','blocked-2','blocked-3','maximum')]
    ui_files += ['death-roll-preview-gallery.png']+[f'death-roll-{name}.png' for name in
                 ('setup','cancelled','preview-cancelled','preview','save-error',
                  'advantage','disadvantage','minimum','twenty','maximum','one','no-canvas')]
    ui_files += ['external-turn-gallery.png']+[f'external-turn-{name}.png' for name in
                 ('effect','effect-confirm','defence','profile','home','empty',
                  'preview','details','save-error','last','invalid')]
    ui_files += ['turn-name-gallery.png']+[f'turn-name-{name}.png' for name in
                 ('empty','editor','confirm','save-error','home','preview','maximum',
                  'last-page','maximum-confirm','clear','named-event','restored')]
    ui_files += ['alternative-costs-gallery.png']+[f'alternative-{name}.png' for name in
                 ('incomplete','editor','profile-confirm','unresolved','choices','selected',
                  'save-error','result','paid','not-chosen','reaction-block','hole-long',
                  'concentration-retry','death-result')]
    for name in ui_files:
        image=preview/name
        if not image.is_file():raise SystemExit(f'Missing UI capture: {name}')
        out.write(image,folder+'firmware-host-preview/'+name)
digest=hashlib.sha256(args.output.read_bytes()).hexdigest()
args.output.with_suffix(args.output.suffix+'.sha256').write_text(f'{digest}  {args.output.name}\n')
print(f'{args.output}: {args.output.stat().st_size} bytes\nSHA256 {digest}')
