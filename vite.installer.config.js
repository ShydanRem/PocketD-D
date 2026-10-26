import {defineConfig} from 'vite';
export default defineConfig({
  root:'web-install',base:'./',
  build:{outDir:'../dist/install',emptyOutDir:true,target:'es2022'},
});
