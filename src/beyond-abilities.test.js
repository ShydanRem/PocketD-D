import test from 'node:test';
import assert from 'node:assert/strict';
import {beyondCoreFixture} from './beyond-fixtures.js';
import {convertBeyond,convertBeyondCore} from './beyond-conversion.js';
import {parseBackup,serialize,describeRoll} from './model.js';
import {projectDevice} from './device-export.js';
const names=['strength','dexterity','constitution','intelligence','wisdom','charisma'],keys=['FOR','DES','COS','INT','SAG','CAR'];
const mod=(subType,type,value,extra={})=>({id:`${subType}-${type}`,subType,type,value,isGranted:true,restriction:'',...extra});
const source=()=>{const p=beyondCoreFixture();p.data.modifiers.race=[];return p;};
const convert=p=>convertBeyond(p,'123456');
function item(p,m,extra={}){p.data.modifiers.item.push(m);p.data.inventory.push({quantity:1,equipped:true,isAttuned:true,definition:{name:'Oggetto sonda',filterType:'Other Gear',canAttune:true,grantedModifiers:[structuredClone(m)]},...extra});return p.data.inventory.at(-1);}
test('Beyond scores: six explicit maximum targets, cap before custom bonus, floors and final overrides; source and backup preserved',()=>{
 for(let i=0;i<6;i++)for(const cap of [0,2,4,10])for(const bonus of [0,2])for(const floor of [0,19,25])for(const override of [null,8,30]){
  const p=source(),c=p.data;c.stats[i].value=18;c.bonusStats[i].value=bonus;c.overrideStats[i].value=override;
  c.modifiers.feat.push(mod(`${names[i]}-score`,'bonus',4));if(cap)c.modifiers.feat.push(mod('ability-score-maximum','bonus',cap,{statId:i+1}));if(floor)c.modifiers.feat.push(mod(`${names[i]}-score`,'set',floor));
  const before=structuredClone(p),r=convert(p),expected=override??Math.max(Math.min(20+cap,22)+bonus,floor);assert.ok(r.canImport,r.conversion.blockers.join(';'));assert.equal(r.draft.abilities[keys[i]],expected);assert.equal(r.abilities[keys[i]].maximum,20+cap);assert.equal(r.abilities[keys[i]].setFloor,floor);assert.deepEqual(p,before);assert.deepEqual(parseBackup(serialize(r.draft)),r.draft);assert.equal(projectDevice(r.draft).packet.abilities[keys[i]],expected);
 }
});
test('Beyond scores: item floor never lowers a higher score, largest floor wins and order is irrelevant',()=>{
 const p=source();p.data.stats[0].value=20;const m=mod('strength-score','set',19,{restriction:'if not already higher'}),row=item(p,m);
 assert.equal(convert(p).draft.abilities.FOR,20);p.data.stats[0].value=8;assert.equal(convert(p).draft.abilities.FOR,19);
 const m2=mod('strength-score','set',23,{id:'second-set'});item(p,m2);assert.equal(convert(p).draft.abilities.FOR,23);p.data.modifiers.item.reverse();assert.equal(convert(p).draft.abilities.FOR,23);
 row.equipped=false;p.data.inventory.at(-1).isAttuned=false;assert.equal(convert(p).draft.abilities.FOR,8);
});
test('Beyond scores: depletion, equipment, attunement and future class levels gate grants without modifying source inventory',()=>{
 for(const patch of [{quantity:0},{equipped:false},{isAttuned:false}]){const p=source();item(p,mod('constitution-score','set',19),patch);assert.equal(convert(p).draft.abilities.COS,15);assert.equal(convert(p).draft.maxHp,26);}
 const p=source();p.data.modifiers.class.push(mod('ability-score-maximum','bonus',4,{componentId:502,statId:1}));assert.equal(convert(p).abilities.FOR.maximum,20);p.data.modifiers.class.at(-1).componentId=500;assert.equal(convert(p).abilities.FOR.maximum,24);
});
test('Beyond scores: final CON feeds HP, DEX feeds AC and saves/skills use the converted scores through device export',()=>{
 const p=source();p.data.stats[1].value=18;p.data.stats[2].value=18;p.data.modifiers.feat.push(mod('dexterity-score','bonus',4),mod('constitution-score','bonus',4),mod('ability-score-maximum','bonus',4,{statId:2}),mod('ability-score-maximum','bonus',4,{statId:3,id:'con-cap'}));
 const r=convert(p),s=r.draft;assert.ok(r.canImport);assert.equal(s.maxHp,38);assert.equal(s.hp,34);assert.equal(s.armorClass,22);assert.equal(describeRoll(s,{kind:'save',key:'COS'}).bonus,6);assert.equal(describeRoll(s,{kind:'skill',key:'acrobatics'}).bonus,6);const d=projectDevice(s).packet;assert.equal(d.maxHp,38);assert.equal(d.hp,34);assert.equal(d.armorClass,22);
 p.data.overrideHitPoints=40;assert.equal(convert(p).draft.maxHp,40);assert.equal(convert(p).draft.hp,36);
});
test('Beyond scores: invalid caps, formulas, conflicts, conditions and unsupported maxima remain blockers even with an override',()=>{
 const bad=[mod('ability-score-maximum','bonus',2),mod('ability-score-maximum','bonus',2,{statId:7}),mod('ability-score-maximum','set',22,{statId:1}),mod('ability-score-maximum','bonus',11,{statId:1}),mod('ability-score-maximum','bonus',-2,{statId:1}),mod('ability-score-maximum','bonus',2,{statId:1,entityId:2}),mod('strength-score','bonus',2,{duration:1}),mod('strength-score','set',19,{statId:3}),mod('strength-score','set',31),mod('strength-score','set',19,{restriction:'while raging'}),mod('strength-score','set',19,{restriction:'If not already higher'}),mod('strength-score','bonus',2,{tagConstraints:[{}]}),mod('strength-score','bonus',2,{dice:{diceString:'1d4'}}),mod('strength-score','bonus',2,{entityId:2}),mod('strength-score','bonus',2,{fixedValue:3}),mod('strength-score','bonus',2,{isGranted:'yes'}),mod('strength-score-maximum','bonus',2)];
 for(const m of bad){const p=source();p.data.overrideStats[0].value=22;p.data.modifiers.feat.push(m);const r=convert(p);assert.equal(r.canImport,false,JSON.stringify(m));assert.equal(r.draft,null);assert.ok(r.conversion.blockers.length);}
 const p=source();p.data.modifiers.feat.push(mod('ability-score-maximum','bonus',8,{statId:1}),mod('ability-score-maximum','bonus',4,{statId:1,id:'extra-cap'}));assert.equal(convert(p).canImport,false);
});
test('Beyond scores: missing/discordant/duplicate item grants and hidden source requirements block inferred values',()=>{
 for(const mutate of [r=>r.definition.grantedModifiers=[],r=>r.definition.grantedModifiers[0].restriction='while raging',r=>r.definition.grantedModifiers[0].duration=1,r=>r.definition.grantedModifiers[0].tagConstraints=[{}],r=>r.definition.grantedModifiers[0].value=20,r=>r.definition.grantedModifiers[0].entityId=2,r=>r.definition.grantedModifiers[0].isGranted=false,r=>r.definition.grantedModifiers.push(structuredClone(r.definition.grantedModifiers[0]))]){
  const p=source(),row=item(p,mod('strength-score','set',19));mutate(row);assert.equal(convert(p).canImport,false);
 }
 const p=source(),row=item(p,mod('strength-score','set',19));p.data.inventory.push(structuredClone(row));assert.equal(convert(p).canImport,false);
});
test('Beyond scores: final values outside 1..30 and base above its reconciled cap cannot silently clamp into a successful import',()=>{
 const p=source();p.data.stats[0].value=21;assert.equal(convert(p).canImport,false);p.data.modifiers.feat.push(mod('ability-score-maximum','bonus',2,{statId:1}));assert.equal(convert(p).draft.abilities.FOR,21);p.data.bonusStats[0].value=10;assert.equal(convert(p).canImport,false);
 p.data.bonusStats[0].value=0;p.data.modifiers.feat.push(mod('strength-score','bonus',-100));assert.equal(convert(p).canImport,false);
});
test('Beyond scores: existing hard override keeps precedence, without adding new metadata to the local character schema',()=>{
 const p=source();p.data.overrideStats[0].value=22;const r=convertBeyondCore(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.abilities.FOR,22);assert.equal(JSON.parse(serialize(r.draft)).version,11);assert.equal(projectDevice(r.draft).packet.version,9);assert.equal(r.conversion.complete,false);
});

test('Beyond scores: HP uses validated source fields after CON reconciliation, even when the preliminary maximum was nonpositive',()=>{
 const p=source();p.data.stats[2].value=4;p.data.baseHitPoints=1;item(p,mod('constitution-score','set',19));const r=convert(p);assert.ok(r.canImport);assert.equal(r.draft.maxHp,13);assert.equal(r.draft.hp,9);
 p.data.baseHitPoints='1';assert.equal(convert(p).canImport,false);p.data.baseHitPoints=1;p.data.removedHitPoints=-1;assert.equal(convert(p).canImport,false);
});
