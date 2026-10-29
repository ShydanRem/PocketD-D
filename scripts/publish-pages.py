#!/usr/bin/env python3
"""Publish only verified installer output to gh-pages; never changes repo visibility.

Without --push this prepares a reviewable directory. The first GitHub Pages
activation is a repository setting; pushing a branch alone does not prove a live site.
"""
import argparse,hashlib,json,re,shutil,subprocess,tempfile
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]
ALLOWED=re.compile(r'(?:index\.html|personalizzazione\.html|manifest\.json|assets/[\w.-]+\.(?:js|css|png|pdc)|firmware/(?:bootloader|partitions|app)\.bin|licenses/[\w.-]+\.txt|downloads/[\w.-]+\.md|downloads/(?:PocketDD-(?:PC|SD)\.zip|PocketDD-PC\.zip\.sha256|pc-package\.json|PocketDD-PC\.part[0-7]\.bin)|downloads/previews/[\w.-]+\.png)')
def git(*args,cwd=ROOT):
    return subprocess.check_output(['git',*args],cwd=cwd,text=True).strip()
def sha(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def validate(base,revision,version):
    manifest=json.loads((base/'manifest.json').read_text())
    if manifest.get('version')!=version or manifest.get('commit')!=revision:raise ValueError('Rebuild installer for the current source commit/version')
    if manifest.get('schema')!=1 or manifest.get('name')!='PocketD-D' or manifest.get('board')!='Waveshare ESP32-S3-Touch-AMOLED-1.75' or manifest.get('chip')!='ESP32-S3' or manifest.get('flash_size')!=0x1000000:raise ValueError('Unexpected firmware target')
    expected=[('bootloader',0),('partitions',0x8000),('app',0x10000)]
    if [(p['id'],p['offset']) for p in manifest['parts']]!=expected:raise ValueError('Unexpected flash regions')
    for part,limit in zip(manifest['parts'],[0x8000,0x9000,0x410000]):
        if not isinstance(part['size'],int) or part['size']<1 or part['offset']+((part['size']+4095)//4096)*4096>limit:raise ValueError('Unsafe firmware bounds')
        if part['path']!=f"firmware/{part['id']}.bin":raise ValueError('Unexpected firmware path')
        path=base/part['path']
        if path.stat().st_size!=part['size'] or sha(path)!=part['sha256']:raise ValueError('Firmware integrity failed')
    if sha(base/'assets/classes.pdc')!=sha(ROOT/'public/assets/classes.pdc'):raise ValueError('SD catalog integrity failed')
    package=json.loads((base/'downloads/pc-package.json').read_text());total=hashlib.sha256();size=0
    for i,part in enumerate(package['parts']):
        if part['path']!=f'PocketDD-PC.part{i}.bin':raise ValueError('Unexpected PC part path')
        path=base/'downloads'/part['path'];data=path.read_bytes()
        if len(data)!=part['size'] or sha(path)!=part['sha256']:raise ValueError('PC part integrity failed')
        size+=len(data);total.update(data)
    if size!=package['size'] or total.hexdigest()!=package['sha256']:raise ValueError('PC package integrity failed')
    selected=[]
    for path in sorted(base.rglob('*')):
        if path.is_symlink():raise ValueError('Symlinks cannot be published')
        if not path.is_file():continue
        name=path.relative_to(base).as_posix()
        if not ALLOWED.fullmatch(name):raise ValueError('Unexpected file in installer output: '+name)
        if name=='downloads/PocketDD-PC.zip':continue # Browser reconstructs the verified parts.
        if path.stat().st_size>16*1024*1024:raise ValueError('Hosting object exceeds 16 MiB')
        selected.append(name)
    if 'index.html' not in selected:raise ValueError('Missing web page')
    return selected
def prepare(base,out,revision,version):
    files=validate(base,revision,version)
    marker=out/'.pocketdd-pages-output'
    if out.exists() and any(out.iterdir()) and not marker.is_file():raise ValueError('Output must be empty or an owned Pages staging directory')
    if out.exists():shutil.rmtree(out)
    out.mkdir(parents=True);marker.write_text('Generated PocketD-D Pages output\n')
    for name in files:
        dest=out/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(base/name,dest)
    (out/'.nojekyll').write_text('')
    (out/'deployment.json').write_text(json.dumps({'version':version,'source_commit':revision,'files_sha256':{name:sha(out/name) for name in files}},indent=2)+'\n')
    return files+['.nojekyll','deployment.json']
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--output',type=Path,default=ROOT.parent/'PocketDD-GitHub-Pages');p.add_argument('--push',action='store_true');args=p.parse_args()
    revision=git('rev-parse','HEAD');version=json.loads((ROOT/'package.json').read_text())['version']
    if args.push and git('branch','--show-current')!='main':raise ValueError('Publish releases from main only')
    if args.push and git('status','--porcelain'):raise ValueError('Commit source changes before publishing')
    out=args.output.resolve()
    if out==ROOT or ROOT.is_relative_to(out) or out.is_relative_to(ROOT/'dist/install'):raise ValueError('Unsafe output directory')
    files=prepare(ROOT/'dist/install',out,revision,version)
    if not args.push:print(json.dumps({'prepared':str(out),'source_commit':revision,'files':len(files)}));return
    remote=git('remote','get-url','origin')
    if remote!='https://github.com/ShydanRem/PocketD-D.git':raise ValueError('Unexpected publishing repository')
    with tempfile.TemporaryDirectory(prefix='pocketdd-pages-git-') as tmp:
        checkout=Path(tmp);git('init','-q',cwd=checkout);git('remote','add','origin',remote,cwd=checkout)
        existing=git('ls-remote','--heads','origin','gh-pages',cwd=checkout)
        if existing:
            git('fetch','--depth=1','origin','gh-pages',cwd=checkout);git('checkout','-q','-B','gh-pages','FETCH_HEAD',cwd=checkout)
            for path in checkout.iterdir():
                if path.name=='.git':continue
                if path.is_dir():shutil.rmtree(path)
                else:path.unlink()
        else:git('checkout','-q','--orphan','gh-pages',cwd=checkout)
        for name in files:
            dest=checkout/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(out/name,dest)
        git('add','--all',cwd=checkout)
        if git('diff','--cached','--name-only',cwd=checkout):
            git('-c','user.name=PocketD-D build','-c','user.email=noreply@users.noreply.github.com','commit','-q','-m',f'Publish PocketD-D {version} from {revision[:7]}',cwd=checkout)
        commit=git('rev-parse','HEAD',cwd=checkout);git('push','origin','HEAD:refs/heads/gh-pages',cwd=checkout)
    print(json.dumps({'branch':'gh-pages','commit':commit,'source_commit':revision,'prepared':str(out),'activation':'GitHub Settings > Pages > Deploy from a branch > gh-pages / (root)'}))
if __name__=='__main__':main()
