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
    for name in ['hero','hero-maximum','hero-zero','hero-storage-error','hero-imported','appearance','appearance-still','avatar-preset-0','avatar-preset-1','avatar-preset-2','sheet','dice','dice-advantage','dice-disadvantage','dice-twenty','pool-setup','pool-picker','pool-configured','pool-six','pool-maximum','pool-negative','pool-zero','pool-d4','pool-d6','pool-d8','pool-d10','pool-d12','pool-d20','resources','confirm']:
        image=preview/(name+'.ppm')
        if image.is_file():out.write(image,folder+'firmware-host-preview/'+image.name)
    for name in ['reductions-preview.png','reduction-before.png','reduction-after.png','reduction-math.png','reduction-result.png','reduction-preview.png','reduction-groups.png','reduction-mixed.png','reduction-unknown.png','reduction-zero.png','reduction-retry.png','reduction-snapshot.png','reduction-last.png','reduction-maximum-group.png','reduction-maximum.png','reduction-maximum-damage.png','multiple-defences-preview.png','multiple-picker.png','multiple-review.png','multiple-round.png','multiple-impact.png','multiple-editor.png','multiple-rules.png','multiple-list.png','multiple-surviving.png','multiple-import.png','multiple-full-round.png','multiple-last.png','conditional-defences-preview.png','conditional-rule.png','conditional-review.png','conditional-source.png','conditional-groups.png','conditional-list.png','conditional-unknown.png','conditional-source-unknown.png','conditional-source-silver.png','conditional-excluded.png','conditional-applied.png','conditional-snapshot.png','conditional-extra.png','conditional-confirm.png','conditional-review-extra.png','conditional-expiry.png','conditional-long.png','conditional-import.png','mixed-damage-preview.png','mixed-parts.png','mixed-types.png','mixed-preview.png','mixed-result.png','mixed-editor.png','mixed-rounding.png','mixed-unknown.png','mixed-impact.png','mixed-save-error.png','mixed-concentration.png','mixed-snapshot.png','mixed-parts-last.png','mixed-massive.png','mixed-types-last.png','mixed-immune.png','temporary-defences-preview.png','temporary-editor.png','temporary-confirm.png','temporary-round-defences.png','temporary-result.png','temporary-duration.png','temporary-link.png','temporary-list.png','temporary-round.png','temporary-round-last.png','temporary-round-full.png','temporary-stop-maximum.png','temporary-expiry.png','temporary-damage.png','temporary-stop.png','temporary-unknown.png','temporary-invalid.png','temporary-import.png','links-preview.png','link-editor.png','link-confirm.png','link-stop.png','link-damage.png','link-recast.png','link-rest.png','link-unavailable.png','concentration-preview.png','concentration-setup.png','concentration-success.png','concentration-save-error.png','concentration-failure.png','concentration-extra.png','concentration-flight.png','concentration-summary.png','concentration-legacy.png','concentration-no-canvas.png','concentration-cap30.png','concentration-capped-result.png','damage-overview.png','damage-preview.png','damage-home.png','defence-editor.png','damage-result.png','damage-unknown.png','defence-confirm.png','damage-concentration.png','damage-immune.png','defences-list.png','damage-combined.png','damage-massive.png','damage-zero.png','defences-last.png','damage-maximum.png','defences-import.png','durations-preview.png','duration-list.png','duration-editor.png','duration-round.png','duration-expired.png','duration-manual.png','duration-all-expired.png','duration-zero.png','duration-invalid.png','effects-preview.png','effects-list.png','effect-editor.png','effect-confirm.png','effect-cap.png','effects-empty.png','effects-hero.png','effect-invalid.png','effects-last.png','effects-import.png','screen-settings.png','screen-configured.png','screen-error.png','status-preview.png','status-home.png','conditions.png','death-tracker.png','death-twenty.png','status-poison.png','condition-detail.png','condition-picker.png','death-confirm.png','death-result.png','death-edit.png','death-stable.png','death-three.png','death-legacy.png','condition-last.png','condition-long.png','rest-preview.png','rest-home.png','rest-long.png','rest-regain.png','hit-healing.png','rest-short.png','rest-settings.png','hit-dice.png','hit-confirm.png','rest-legacy.png','hit-last.png','dice-preview.gif','pool-preview.gif','device-preview.png','hero.png','dice.png','sheet.png','resources.png','avatar-preview.gif','avatar-presets.png','appearance.png','appearance-still.png','hero-imported.png','magic-preview.png','magic-list.png','magic-detail.png','magic-text.png','keys.png','magic-confirm.png','magic-status.png','magic-slots.png','magic-recovery.png','magic-last.png','magic-legacy.png','inventory-preview.png','inventory.png','weapon.png','combat.png','combat-detail.png','attack-critical.png','weapon-damage.png','attack-miss.png','item-confirm.png','healing.png','damage-eighteen.png','inventory-last.png','inventory-legacy.png','item-long-name.png','weapon-empty.png','skills-preview.png','skills-first.png','skills-last.png','saves.png','check-detail.png','check-blocked.png','skill-result.png','save-detail.png','skills-legacy.png','sheet-trained.png']:
        animation=preview/name
        if animation.is_file():out.write(animation,folder+'firmware-host-preview/'+name)
digest=hashlib.sha256(args.output.read_bytes()).hexdigest()
args.output.with_suffix(args.output.suffix+'.sha256').write_text(f'{digest}  {args.output.name}\n')
print(f'{args.output}: {args.output.stat().st_size} bytes\nSHA256 {digest}')
