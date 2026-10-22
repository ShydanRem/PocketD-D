import test from 'node:test';
import assert from 'node:assert/strict';
import {createHash} from 'node:crypto';
import {deviceAvatarPixels,deviceAvatarSvg} from './device-avatar.js';

test('all 81 device appearances and four poses render distinctly without changing input',()=>{
 const seen=new Set();
 for(let skin=0;skin<3;skin++)for(let hair=0;hair<3;hair++)for(let cloak=0;cloak<3;cloak++)for(let style=0;style<3;style++)for(let pose=0;pose<4;pose++){
  const input=Object.freeze({skin,hair,cloak,style});
  const {width,height,colors,pixels}=deviceAvatarPixels(input,pose);
  assert.equal(width,48);assert.equal(height,60);assert.equal(pixels.length,width*height);
  const rgba=new Uint8Array(pixels.length*4);
  pixels.forEach((p,i)=>{const c=colors[p];rgba.set([c>>16,c>>8&255,c&255,p?255:0],i*4);});
  const digest=createHash('sha256').update(rgba).digest('hex');assert(!seen.has(digest));seen.add(digest);
 }
 assert.equal(seen.size,324);
});
test('appearance controls affect only their material while preserving leather, eyes and silhouettes',()=>{
 for(let style=0;style<3;style++)for(let pose=0;pose<4;pose++){
  const base={skin:0,hair:0,cloak:0,style},a=deviceAvatarPixels(base,pose);
  for(const [kind,key] of ['skin','hair','cloak'].entries()){
   const b=deviceAvatarPixels({...base,[key]:2},pose);assert.deepEqual(a.pixels,b.pixels);let changed=0;
   a.pixels.forEach(p=>{if(a.colors[p]!==b.colors[p]){assert(p>=1+16*kind&&p<=16+16*kind);changed++;}});
   assert(changed>10);
  }
 }
 const svg=deviceAvatarSvg({skin:1,hair:2,cloak:0,style:1});
 assert.match(svg,/viewBox="0 0 48 60"/);assert.match(svg,/shape-rendering="crispEdges"/);assert(!svg.includes('undefined'));
});
test('invalid device appearance and pose do not produce a misleading preview',()=>{
 const a={skin:0,hair:0,cloak:0,style:0};
 for(const key of Object.keys(a))for(const value of [-1,3,.5,'1',null,NaN])assert.throws(()=>deviceAvatarSvg({...a,[key]:value}));
 for(const pose of [-1,4,.5,'0',null])assert.throws(()=>deviceAvatarSvg(a,pose));
 assert.throws(()=>deviceAvatarSvg(null));
});
