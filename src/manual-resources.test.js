import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondManualFixture as manualFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {initialState,parseBackup,serialize} from './model.js';
import {validateManualResources,emptyExtended} from './extended.js';
import {projectDevice} from './device-export.js';
import {usePower,rest} from './powers.js';
import {createLibrary,addCharacter,parseLibrary,serializeLibrary} from './library.js';
const cv=p=>convertBeyond(p,'123456');
test('Beyond manual action counters preserve balances separately from abilities, without automatic use/rest or false shared pools',()=>{
 const p=manualFixture(),before=structuredClone(p),r=cv(p);assert.ok(r.canImport);
 const s=r.draft;assert.equal(s.extended.manualResources.length,5);assert.equal(JSON.parse(serialize(s)).version,18);assert.deepEqual(p,before);
 for(let i=0;i<5;i++){const a=p.data.actions.class[i+1],m=s.extended.manualResources[i],power=s.powers.find(v=>v.name===a.name);assert.equal(m.current,a.limitedUse.maxUses-a.limitedUse.numberUsed);assert.equal(m.max,a.limitedUse.maxUses);assert.ok(power.passive);assert.equal(power.maxUses,0);assert.match(power.description,/Contatore manuale separato/);assert.throws(()=>usePower(s,power.id));}
 for(const kind of ['short','long'])assert.deepEqual(rest(s,kind).extended.manualResources,s.extended.manualResources);
 const ordinary=s.powers.find(v=>v.name==='Rage'),spent=usePower(s,ordinary.id);assert.equal(spent.powers.find(p=>p.id===ordinary.id).remaining,ordinary.remaining-1);assert.deepEqual(spent.extended.manualResources,s.extended.manualResources);
 assert.deepEqual(parseBackup(serialize(s)),s);assert.deepEqual(projectDevice(s).packet.resources,s.extended.manualResources.map(r=>({name:r.name,current:r.current,maximum:r.max})));assert.equal(projectDevice(s).packet.version,9);
});
test('Manual counter limits: 0/1/999 and zero used are exact, all eight reach device, ninth blocks instead of truncating',()=>{
 for(const max of [1,50,99,100,999])for(const used of [0,max]){const p=manualFixture(1);Object.assign(p.data.actions.class[1].limitedUse,{maxUses:max,numberUsed:used});const r=cv(p);assert.ok(r.canImport);assert.equal(r.draft.extended.manualResources[0].current,max-used);assert.equal(projectDevice(r.draft).packet.resources[0].maximum,max);}
 assert.equal(projectDevice(cv(manualFixture(8)).draft).packet.resources.length,8);assert.throws(()=>cv(manualFixture(9)),/8 contatori/);
 const p=manualFixture(2);p.data.actions.class[2].name=p.data.actions.class[1].name;const r=cv(p);assert.equal(r.draft.extended.manualResources.length,2);assert.notEqual(r.draft.extended.manualResources[0].id,r.draft.extended.manualResources[1].id);
});
test('Manual counters: formula uses final ability/proficiency, invalid or unknown linked resource fields never become invented pools',()=>{
 for(let stat=1;stat<=6;stat++)for(const operator of [1,2]){const p=manualFixture(1),l=p.data.actions.class[1].limitedUse;Object.assign(l,{maxUses:10,statModifierUsesId:stat,operator,useProficiencyBonus:true,proficiencyBonusOperator:1,numberUsed:0});const r=cv(p),ability=Object.values(r.draft.abilities)[stat-1],m=Math.floor((ability-10)/2),expected=(operator===1?10+m:10*m)+2;if(expected>0){assert.equal(r.draft.extended.manualResources[0].max,expected);}else assert.equal(r.draft.extended.manualResources,undefined);}
 for(const mutate of [l=>l.numberUsed=-1,l=>l.numberUsed=5,l=>l.maxUses=1000,l=>l.numberUsed='0',l=>l.maxUses='4',l=>l.minNumberConsumed=2,l=>l.sharedResourceId=42,l=>l.statModifierUsesId=7,l=>l.useProficiencyBonus='false',l=>l.useProficiencyBonus=1]){const p=manualFixture(1);mutate(p.data.actions.class[1].limitedUse);const r=cv(p);assert.ok(r.canImport);assert.equal(r.draft.extended.manualResources,undefined);assert.ok(r.draft.powers.find(p=>p.name==='Riserva manuale 1').passive);assert.ok(r.conversion.omissions.length);}
 const p=manualFixture(1);p.data.actions.class.push(structuredClone(p.data.actions.class[1]));assert.equal(cv(p).draft.extended.manualResources.length,1);p.data.actions.class[2].limitedUse.numberUsed++;assert.throws(()=>cv(p),/discordante/);
});
test('Manual counter backup v18 is strict, optional on old states, independent of skill maps/defences and preserves a mixed collection',()=>{
 const s=cv(manualFixture()).draft,b=JSON.parse(serialize(s));
 for(const version of [11,16,17,19])assert.throws(()=>parseBackup(JSON.stringify({...b,version})));delete b.extended.manualResources;assert.throws(()=>parseBackup(JSON.stringify(b)));
 for(const input of [null,{},Array(9).fill({}),[{id:'x',name:'x',max:4,current:5}],[{id:'x',name:'x',max:0,current:0}],[{id:'x',name:'x',max:4,current:0,extra:1}]])assert.throws(()=>validateManualResources(input));
 const local=initialState();assert.equal(JSON.parse(serialize(local)).version,11);assert.equal(projectDevice(local).packet.resources[0].name,'Slancio');
 s.extended.skillAbilities={intimidation:'FOR'};s.extended.defences={configured:32,resistance:32,vulnerability:0,immunity:0};assert.equal(projectDevice(s).packet.version,15);assert.deepEqual(parseBackup(serialize(s)),s);
 const l=createLibrary(),next=addCharacter(l,s,'manual');assert.deepEqual(parseLibrary(serializeLibrary(next)),next);assert.deepEqual(next.characters[0],l.characters[0]);
 assert.throws(()=>validateManualResources([{id:'x',name:'x',max:1,current:0},{id:'x',name:'y',max:1,current:1}]));
 const empty={...local,extended:{...emptyExtended(),manualResources:[]}};assert.equal(projectDevice(empty).packet.resources.length,0);assert.equal(JSON.parse(serialize(empty)).version,18);
});
