import palette from './avatar-palette.json' with {type:'json'};
import masks from './device-avatar-masks.json' with {type:'json'};
const shade=color=>{
  const rgb=Number.parseInt(color.slice(1),16);
  return '#'+[rgb>>16&255,rgb>>8&255,rgb&255].map(n=>Math.floor(n*3/4).toString(16).padStart(2,'0')).join('');
};
// The PC export preview uses the firmware's original masks and exact colors.
export function deviceAvatarSvg(avatar,pose=0){
  if(!avatar||!['skin','hair','cloak','style'].every(k=>Number.isInteger(avatar[k])&&avatar[k]>=0&&avatar[k]<=2)||!Number.isInteger(pose)||pose<0||pose>3)throw new Error('Avatar non valido');
  const skin=palette.skinColors[avatar.skin],hair=palette.hairColors[avatar.hair],cloak=palette.cloakColors[avatar.cloak];
  const colors=[null,'#081711',shade(skin),skin,hair,shade(hair),cloak[2],cloak[0],cloak[1],'#b8c2a3','#647967','#d0ae68','#f1d590','#73543b','#352e28','#172621'];
  let pixels='';
  masks.poses[avatar.style][pose].forEach((row,y)=>{
    for(let x=0;x<row.length;){
      const pixel=row[x];let end=x+1;while(end<row.length&&row[end]===pixel)end++;
      if(pixel!=='0')pixels+=`<rect x="${x}" y="${y}" width="${end-x}" height="1" fill="${colors[Number.parseInt(pixel,16)]}"/>`;
      x=end;
    }
  });
  return `<svg class="device-avatar-sprite" viewBox="0 0 ${masks.width} ${masks.height}" role="img" aria-label="Avatar originale per il dispositivo" shape-rendering="crispEdges">${pixels}</svg>`;
}
