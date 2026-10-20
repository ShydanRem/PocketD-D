import palette from './avatar-palette.json' with {type:'json'};
// Original SVG pixel art. Firmware uses the same palette and appearance indices.
export const {skinColors,hairColors,cloakColors}=palette;
export const avatarPresets=[
  {name:'Custode',avatar:{skin:0,hair:0,cloak:0,style:0}},
  {name:'Viandante',avatar:{skin:1,hair:1,cloak:1,style:1}},
  {name:'Sentinella',avatar:{skin:2,hair:2,cloak:2,style:2}}
];
export function avatarDescription(avatar) {
  return `Mantello ${['bosco','vino','notte'][avatar.cloak]}, capelli ${['castani','biondi','scuri'][avatar.hair]} ${['corti','lunghi','rasati'][avatar.style]}, carnagione ${['chiara','media','scura'][avatar.skin]}`;
}

export function avatarSvg(avatar,{armed=true}={}) {
  const cloak=cloakColors[avatar.cloak],skin=skinColors[avatar.skin],hair=hairColors[avatar.hair];
  const hairShape=[
    'M10 7h3V5h9v2h2v7h-3v-4h-4v2h-5v4h-2z',
    'M10 7h3V5h9v2h2v13h-3V10h-4v2h-5v9H9V10h1z',
    'M12 7h10v2H12z'
  ][avatar.style];
  return `<svg class="sprite" viewBox="0 0 36 44" role="img" aria-label="Eroe originale in pixel art, mantello ${['bosco','vino','notte'][avatar.cloak]}${armed?', arma equipaggiata':''}"><g shape-rendering="crispEdges">
    <path fill="#081711" d="M7 40h23v2H7zM11 6h2V4h10v2h2v14h3v16h-3v5h-8v-3h-2v3H9v-7H7V23h3V17h1z"/>
    <path fill="${cloak[0]}" d="M10 19h15v7h3v11h-8v-4h-8v4H7V25h3z"/>
    <path fill="${cloak[2]}" d="M22 20h3v6h3v11h-7V26h-2z"/>
    <path fill="${cloak[1]}" d="M10 21h3v11h-3v3H9V24h1zM13 19h7v2h-7z"/>
    <path fill="#73543b" d="M13 30h5v8h-5zm7 0h4v8h-4z"/>
    <path fill="#352e28" d="M11 37h7v4h-8v-2h1zm9 0h5v2h2v2h-7z"/>
    <path fill="#b8c2a3" d="M13 22h9v8h-9z"/><path fill="#647967" d="M20 22h2v8h-2zM13 27h7v1h-7z"/>
    <path fill="#d0ae68" d="M12 21h11v2H12zM12 30h12v2H12z"/><path fill="#f1d590" d="M17 30h3v3h-3z"/>
    <path fill="${skin}" d="M12 9h11v10h-2v2h-7v-2h-2zM8 24h4v5H8zm16 0h4v5h-4z"/>
    <path fill="#794f3e" opacity=".35" d="M12 16h2v3h7v-2h2v2h-2v2h-7v-2h-2z"/>
    <path fill="${hair}" d="${hairShape}"/><path fill="#d5a266" opacity=".3" d="M13 6h8v1h-8z"/>
    <path fill="#172621" d="M14 13h2v3h-2zm6 0h2v3h-2z"/><path fill="#f7e4bb" d="M14 13h1v1h-1zm6 0h1v1h-1z"/>
    <path fill="#a56a4f" d="M16 18h4v1h-4z"/><path fill="#d0ae68" d="M14 20h7v2h-7z"/>
    ${armed?'<g class="avatar-weapon"><path fill="#10231b" d="M29 13h4v18h-4z"/><path fill="#b9d9d0" d="M30 14h2v16h-2z"/><path fill="#f1ecd1" d="M31 12h1v14h-1z"/><path fill="#d0ae68" d="M27 29h8v2h-8z"/><path fill="#79543a" d="M30 31h2v5h-2z"/><path fill="#edcd81" d="M30 36h2v2h-2z"/></g>':''}
  </g></svg>`;
}
