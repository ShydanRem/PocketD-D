#!/usr/bin/env python3
"""Cache the unchanged official Windows x64 Node runtime, pinned by SHA256."""
import argparse,hashlib,io,json,struct,urllib.request,zipfile
from pathlib import Path
root=Path(__file__).resolve().parents[1]
info=json.loads((root/'scripts/windows-runtime.json').read_text())
cache=root/'.pocketdd-runtime'/f'node-{info["version"]}-{info["platform"]}'
p=argparse.ArgumentParser();p.add_argument('--verify',action='store_true');args=p.parse_args()
digest=lambda data:hashlib.sha256(data).hexdigest()
if not args.verify:
    with urllib.request.urlopen(info['source'],timeout=180) as r:data=r.read(100_000_000)
    assert digest(data)==info['archive_sha256'],'Official archive SHA256 mismatch'
    with zipfile.ZipFile(io.BytesIO(data)) as z:
        prefix=f'node-{info["version"]}-{info["platform"]}/'
        files={name:z.read(prefix+name) for name in ['node.exe','LICENSE']}
    assert digest(files['node.exe'])==info['node_sha256'] and digest(files['LICENSE'])==info['license_sha256']
    cache.mkdir(parents=True,exist_ok=True)
    for name,data in files.items():(cache/name).write_bytes(data)
    (cache/'runtime-info.json').write_text(json.dumps(info,indent=2)+'\n')
node=(cache/'node.exe').read_bytes();license=(cache/'LICENSE').read_bytes()
assert digest(node)==info['node_sha256'] and digest(license)==info['license_sha256']
assert node[:2]==b'MZ';offset=struct.unpack_from('<I',node,0x3c)[0]
assert node[offset:offset+4]==b'PE\0\0' and struct.unpack_from('<H',node,offset+4)[0]==0x8664
assert json.loads((cache/'runtime-info.json').read_text())==info
print(f'Official Node {info["version"]} Windows x64: pinned archive/executable/license, PE architecture checked. Windows execution not tested on Linux.')
