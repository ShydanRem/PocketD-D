import test from 'node:test';
import assert from 'node:assert/strict';
import {avatarIdentity,avatarSpecies,suggestBeyondAvatar} from './avatar-species.js';
import {initialState,parseBackup,serialize} from './model.js';
import {projectDevice} from './device-export.js';
import {deviceAvatarPixels} from './device-avatar.js';
import {addBeyondAppearance} from './beyond-avatar.js';
test('Species: exact bilingual visual aliases, ambiguous and homebrew sources, explicit gender only',()=>{
 for(const species of avatarSpecies)for(const alias of species.aliases){const a=suggestBeyondAvatar({fullName:alias},'Female');assert.equal(a.status,'matched');assert.equal(a.lineage,species.id);assert.equal(a.body,1);assert.equal(a.bodySource,'explicit');}
 for(const name of ['Tabaxi','Half-Elf pirate homebrew','Human variant monster','',null]){const a=suggestBeyondAvatar({fullName:name},'Unknown');assert.equal(a.status,'manual');assert.equal(a.bodySource,'manual');assert.equal(a.lineage,0);}
 assert.equal(suggestBeyondAvatar({fullName:'Dwarf',baseName:'Elf'}).status,'ambiguous');assert.equal(suggestBeyondAvatar({fullName:'Elf',isHomebrew:true}).status,'manual');assert.equal(suggestBeyondAvatar({fullName:'Elfo (2024)'},'femmina').body,1);
 assert.deepEqual(avatarIdentity({}),{lineage:0,body:0});for(const a of [{lineage:12},{lineage:-1},{body:2},{body:null},{lineage:'1'}])assert.throws(()=>avatarIdentity(a));
});
test('Species: all visual choices persist in v12/USB10; old avatar stays v11/USB9; invalid/future versions reject',()=>{
 const original=initialState(),before=structuredClone(original);assert.equal(JSON.parse(serialize(original)).version,11);assert.equal(projectDevice(original).packet.version,9);
 for(let lineage=0;lineage<12;lineage++)for(let body=0;body<2;body++){
  const state=structuredClone(original);Object.assign(state.avatar,{lineage,body});const raw=serialize(state),restored=parseBackup(raw),{packet}=projectDevice(restored);
  assert.deepEqual(avatarIdentity(restored.avatar),{lineage,body});assert.equal(packet.version,lineage||body?10:9);
  if(lineage||body){assert.equal(JSON.parse(raw).version,12);assert.equal(packet.avatar.lineage,lineage);assert.equal(packet.avatar.body,body);assert.throws(()=>parseBackup(JSON.stringify({...JSON.parse(raw),version:11})));}
  const image=deviceAvatarPixels(restored.avatar);assert.equal(image.pixels.length,2880);assert.ok(image.pixels.some(v=>v));
 }
 assert.deepEqual(original,before);assert.throws(()=>parseBackup(JSON.stringify({...JSON.parse(serialize(original)),version:13})));
});
test('Species: proposal/override change only avatar, blocked report never creates a draft',()=>{
 const original=initialState(),report={draft:structuredClone(original),conversion:{evidence:[]}};
 addBeyondAppearance(report,{race:{fullName:'Forest Gnome'},gender:'female'});assert.deepEqual(avatarIdentity(report.draft.avatar),{lineage:4,body:1});
 const expected=parseBackup(serialize(original)),converted=structuredClone(report.draft);converted.avatar=expected.avatar;delete converted.version;delete expected.version;assert.deepEqual(converted,expected);
 const blocked={draft:null,conversion:{evidence:[]}};addBeyondAppearance(blocked,{race:{fullName:'Orc'}});assert.equal(blocked.draft,null);
});
