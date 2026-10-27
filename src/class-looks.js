import {crc32} from './device-backup.js';
import {avatarOutfit} from './avatar-classes.js';
import {avatarIdentity,avatarSpecies} from './avatar-species.js';
let pack=null;
export function installClassLooks(input){
  const b=input instanceof Uint8Array?input:new Uint8Array(input),v=new DataView(b.buffer,b.byteOffset,b.byteLength),count=avatarSpecies.length*24,tableSize=count*4,frameSize=12288,total=32+tableSize+count*frameSize;
  if(b.length!==total||String.fromCharCode(...b.subarray(0,4))!=='PDCL'||v.getUint16(4,true)!==1||b[6]!==64||b[7]!==64||v.getUint16(8,true)!==avatarSpecies.length||b[10]!==2||b[11]!==12||v.getUint32(12,true)!==frameSize||v.getUint32(16,true)!==count||v.getUint32(20,true)!==total||v.getUint32(28,true)!==crc32(b.subarray(0,28))||v.getUint32(24,true)!==crc32(b.subarray(32,32+tableSize)))throw new Error('Pacchetto classi non valido');
  for(let i=0;i<count;i++)if(v.getUint32(32+i*4,true)!==crc32(b.subarray(32+tableSize+i*frameSize,32+tableSize+(i+1)*frameSize)))throw new Error('Sprite classe corrotto');
  pack=b;
}
export async function loadClassLooks(){
  try{
    const inline=globalThis.document?.getElementById('pdd-class-pack');
    if(inline)installClassLooks(Uint8Array.from(atob(inline.textContent.trim()),c=>c.charCodeAt(0)));
    else{const r=await fetch(new URL('assets/classes.pdc',globalThis.document?.baseURI??globalThis.location?.href??import.meta.url),{signal:AbortSignal.timeout(5000)});if(!r.ok)throw new Error('Asset assenti');installClassLooks(await r.arrayBuffer());}
    return true;
  }catch{pack=null;return false;}
}
export function classLooksAvailable(){return Boolean(pack);}
export function classLookPixels(avatar){
  const outfit=avatarOutfit(avatar);if(!outfit||!pack)return null;
  const {lineage,body}=avatarIdentity(avatar),index=(lineage*2+body)*12+outfit-1,start=32+avatarSpecies.length*24*4+index*12288,bytes=pack.subarray(start,start+12288),v=new DataView(bytes.buffer,bytes.byteOffset,bytes.byteLength);
  const rgba=new Uint8Array(64*64*4);for(let i=0;i<4096;i++){const c=v.getUint16(i*2,true);rgba[i*4]=Math.floor((c>>11)*255/31);rgba[i*4+1]=Math.floor(((c>>5)&63)*255/63);rgba[i*4+2]=Math.floor((c&31)*255/31);rgba[i*4+3]=bytes[8192+i];}
  return {width:64,height:64,rgba};
}
