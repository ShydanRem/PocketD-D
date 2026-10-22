import {readFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {deviceAvatarPixels} from '../src/device-avatar.js';
const native=readFileSync('firmware/tests/out/avatar-hashes.txt','utf8').trim().split('\n').map(Number);
function crc32(bytes){let crc=0xffffffff;for(const byte of bytes){crc^=byte;for(let bit=0;bit<8;bit++)crc=(crc>>>1)^(crc&1?0xedb88320:0);}return (crc^0xffffffff)>>>0;}
let n=0;
for(let skin=0;skin<3;skin++)for(let hair=0;hair<3;hair++)for(let cloak=0;cloak<3;cloak++)for(let style=0;style<3;style++)for(let pose=0;pose<4;pose++){
 const {colors,pixels}=deviceAvatarPixels({skin,hair,cloak,style},pose);
 const bytes=new Uint8Array(1024+pixels.length);
 colors.forEach((color,i)=>bytes.set([color&255,color>>8&255,color>>16,i?255:0],i*4));bytes.set(pixels,1024);
 assert.equal(crc32(bytes),native[n++],`${skin}/${hair}/${cloak}/${style}, pose ${pose}`);
}
assert.equal(n,native.length);
console.log(`Avatar: ${n} exact C/JS I8 buffer comparisons passed`);
