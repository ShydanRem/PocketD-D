import speciesMasks from './device-species-masks.json' with {type:'json'};
import {avatarIdentity} from './avatar-species.js';
import materials from './device-avatar-colors.json' with {type:'json'};
import masks from './device-avatar-masks.json' with {type:'json'};

export function deviceAvatarPixels(avatar,pose=0){
  if(!avatar||!['skin','hair','cloak','style'].every(k=>Number.isInteger(avatar[k])&&avatar[k]>=0&&avatar[k]<=2)||!Number.isInteger(pose)||pose<0||pose>3)throw new Error('Avatar non valido');
  const {lineage,body}=avatarIdentity(avatar);
  if(lineage||body){
    const decode=frame=>{
      const bytes=Uint8Array.from(frame.bytes.match(/../g),v=>Number.parseInt(v,16)),pixels=new Uint8Array(48*60);let at=0,bit=0,remaining=frame.size;
      const read=()=>{if(!remaining--)throw new Error('Sprite incompleto');let node=0;while(node>=0){if(bit>=bytes.length*8||!speciesMasks.tree[node])throw new Error('Sprite non valido');node=speciesMasks.tree[node][(bytes[bit>>3]>>(7-(bit++&7)))&1];}return -node-1;};
      while(remaining){const token=read(),count=token<128?token+1:token-127;if(at+count>pixels.length)throw new Error('Sprite non valido');if(token<128){for(let i=0;i<count;i++)pixels[at++]=read();}else{const value=read();pixels.fill(value,at,at+count);at+=count;}}
      if(at!==pixels.length||Math.ceil(bit/8)!==bytes.length)throw new Error('Sprite incompleto');return pixels;
    };
    const poses=speciesMasks.profiles[lineage][body][avatar.style],frame=poses[pose],pixels=decode(frame);
    if(frame.xor){const base=decode(poses[0]);for(let i=0;i<pixels.length;i++)pixels[i]^=base[i];}
    const colors=[...speciesMasks.fixed];
    [avatar.skin,avatar.hair,avatar.cloak].forEach((choice,material)=>colors.splice(1+16*material,16,...speciesMasks.materials[lineage][material][choice]));
    return {width:48,height:60,colors,pixels};
  }
  const colors=[...masks.fixed];
  [avatar.skin,avatar.hair,avatar.cloak].forEach((choice,material)=>colors.splice(1+16*material,16,...materials[material][choice]));
  return {width:masks.width,height:masks.height,colors,pixels:Uint8Array.from(masks.poses[avatar.style][pose].join('').match(/../g),v=>Number.parseInt(v,16))};
}

// Same I8 masks and precomputed material shades as firmware, at its native grid.
export function deviceAvatarSvg(avatar,pose=0){
  const {width,height,colors,pixels}=deviceAvatarPixels(avatar,pose);
  let shapes='';
  for(let y=0;y<height;y++)for(let x=0;x<width;){
    const index=pixels[y*width+x];let end=x+1;
    while(end<width&&pixels[y*width+end]===index)end++;
    if(index)shapes+=`<rect x="${x}" y="${y}" width="${end-x}" height="1" fill="#${colors[index].toString(16).padStart(6,'0')}"/>`;
    x=end;
  }
  return `<svg class="device-avatar-sprite" viewBox="0 0 ${width} ${height}" role="img" aria-label="Avatar pixel art per il dispositivo" shape-rendering="crispEdges">${shapes}</svg>`;
}
