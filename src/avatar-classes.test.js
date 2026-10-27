import test from 'node:test';
import assert from 'node:assert/strict';
import {avatarClasses,avatarOutfit,suggestBeyondClass} from './avatar-classes.js';
import {avatarIdentity} from './avatar-species.js';
import {initialState,serialize,parseBackup} from './model.js';
import {projectDevice} from './device-export.js';
import {addBeyondAppearance} from './beyond-avatar.js';
test('Class presets: all 504 cosmetic choices persist as v15/USB13 while original formats remain unchanged',()=>{
 const original=initialState(),before=structuredClone(original);
 for(let lineage=0;lineage<21;lineage++)for(let body=0;body<2;body++)for(let outfit=1;outfit<13;outfit++){
  const s=structuredClone(original);Object.assign(s.avatar,{lineage,body,outfit});const backup=JSON.parse(serialize(s)),restored=parseBackup(JSON.stringify(backup)),{packet,warnings}=projectDevice(restored);
  assert.equal(backup.version,15);assert.equal(restored.avatar.outfit,outfit);assert.equal(packet.version,13);assert.deepEqual(packet.avatar,s.avatar);assert.ok(warnings.some(w=>w.includes('SD')));
  for(const version of [11,12,13,14])assert.throws(()=>parseBackup(JSON.stringify({...backup,version})));
  delete backup.avatar.outfit;assert.throws(()=>parseBackup(JSON.stringify(backup)));
 }
 assert.equal(JSON.parse(serialize(original)).version,11);assert.equal(projectDevice(original).packet.version,9);assert.deepEqual(original,before);
 for(const outfit of [-1,13,1.5,'1',null])assert.throws(()=>avatarOutfit({outfit}));
 assert.equal(avatarOutfit({}),0);assert.deepEqual(avatarIdentity({outfit:12}),{lineage:0,body:0});
});
test('Beyond class proposal: only explicit known single class; manual multiclass, custom and unsupported classes',()=>{
 for(const c of avatarClasses.slice(1))for(const name of c.aliases){const a=suggestBeyondClass([{definition:{name}}]);assert.equal(a.outfit,c.id);assert.equal(a.status,'matched');}
 for(const classes of [undefined,[],[{definition:{name:'Artificer'}}],[{definition:{name:'Rogue',isHomebrew:true}}],[{definition:{name:'Rogue'}},{definition:{name:'Wizard'}}]])assert.equal(suggestBeyondClass(classes).outfit,0);
 const base=initialState(),report={draft:structuredClone(base),conversion:{evidence:[]}};
 addBeyondAppearance(report,{race:{fullName:'Tabaxi'},gender:'female',classes:[{definition:{name:'Ranger'}}]});assert.equal(report.draft.avatar.outfit,8);assert.equal(report.draft.avatar.lineage,12);assert.equal(report.draft.avatar.body,1);
 const expected=parseBackup(serialize(base));report.draft.avatar=expected.avatar;assert.deepEqual(report.draft,expected);assert.ok(report.conversion.evidence.some(e=>e.includes('Solo estetica')));
});
