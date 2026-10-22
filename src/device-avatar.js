import materials from './device-avatar-colors.json' with {type:'json'};
import masks from './device-avatar-masks.json' with {type:'json'};

export function deviceAvatarPixels(avatar,pose=0){
  if(!avatar||!['skin','hair','cloak','style'].every(k=>Number.isInteger(avatar[k])&&avatar[k]>=0&&avatar[k]<=2)||!Number.isInteger(pose)||pose<0||pose>3)throw new Error('Avatar non valido');
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
