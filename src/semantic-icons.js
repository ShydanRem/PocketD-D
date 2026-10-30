import catalog from './semantic-icons.json' with {type:'json'};
// The same explicit, ordered aliases build the native firmware sprites.
export function semanticGlyph(family,name,{kind,effect,healing=false}={}){
 if(family==='item'){if(healing)return 'heal';if(kind===5)return 'shield';if(kind>=2)return 'armor';}
 if(effect==='hex-2024')return 'skull';if(effect==='hunters-mark-2024')return 'eye';
 if(effect==='divine-smite-2024')return 'sun';if(effect==='true-strike-2024')return 'strike';if(effect==='rage')return 'rage';if(effect==='relentless-endurance')return 'heal';
 const value=String(name).toLowerCase();return catalog.aliases[family]?.find(([,names])=>names.some(n=>value.includes(n)))?.[0]??(family==='item'?'gear':family==='feature'?'feature':'scroll');
}
export function semanticIcon(family,name,options){
 const id=semanticGlyph(family,name,options),g=catalog.glyphs[id];
 const pixels=g.rows.flatMap((row,y)=>[...row].flatMap((cell,x)=>cell==='#'?[`<rect x="${x}" y="${y}" width="1" height="1"/>`]:[])).join('');
 return `<svg class="semantic-icon" aria-hidden="true" viewBox="0 0 12 12" shape-rendering="crispEdges" fill="#${g.color}">${pixels}</svg>`;
}
