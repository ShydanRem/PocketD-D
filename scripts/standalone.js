import {readFileSync,writeFileSync} from 'node:fs';
const read=path=>readFileSync(`dist/${path.replace(/^\//,'')}`,'utf8');
let html=read('index.html');
html=html.replace(/<script type="module" crossorigin src="([^"]+)"><\/script>/g,(_,path)=>`<script type="module">${read(path).replace(/<\/script/gi,'<\\/script')}</script>`);
html=html.replace(/<link rel="stylesheet" crossorigin href="([^"]+)">/g,(_,path)=>`<style>${read(path)}</style>`);
html=html.replace('</head>',`<script type="application/octet-stream" id="pdd-class-pack">${readFileSync('public/assets/classes.pdc').toString('base64')}</script></head>`);
writeFileSync('PocketDD-demo.html',html);
console.log('Aggiornato PocketDD-demo.html');
