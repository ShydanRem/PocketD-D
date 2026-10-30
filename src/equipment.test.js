import {test} from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup,weaponStats,describeRoll} from './model.js';
import {emptyExtended} from './extended.js';
import {emptyGear,equipGear,validateLoadout} from './equipment.js';
import {adjustQuantity,removeItem,upsertItem} from './inventory.js';
import {upsertPower,usePower} from './powers.js';
import {projectDevice} from './device-export.js';
import {beyondRichFixture} from './beyond-fixtures.js';
import {convertBeyond} from './beyond-conversion.js';
import {semanticGlyph,semanticIcon} from './semantic-icons.js';
export function equippedFixture(){
 const s=initialState();s.extended=emptyExtended();s.level=6;s.hp=35;s.maxHp=53;s.armorClass=16;s.abilities={FOR:15,DES:10,COS:16,INT:8,SAG:8,CAR:17};
 s.inventory.push({id:'chain',type:'gear',name:'Chain Mail',notes:'',quantity:1},{id:'ring',type:'gear',name:'Anello',notes:'',quantity:1},{id:'shield',type:'gear',name:'Shield',notes:'',quantity:1});
 s.extended.loadout={unarmored:1,items:[{...emptyGear('chain'),kind:4,armor:16,equipped:true},{...emptyGear('ring'),ability:[0,2,2,0,0,2],ac:1,attack:1,damage:2,spellAttack:2,spellDc:1,saves:1,checks:2,maxHp:4},{...emptyGear('shield'),kind:5,armor:2}]};
 s.powers=[{id:'spell',type:'spell',name:'Spell',description:'',level:1,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:true},{id:'ts',type:'spell',name:'True Strike',description:'',level:0,concentration:false,castingAbility:'CAR',attackModifier:0,saveModifier:0,prepared:true,usesSpellSlot:false,effect:'true-strike-2024'}];return s;
}
test('Loadout: adopted armor snapshot, independent shield, CON HP and linked numeric deltas without healing',()=>{
 const s=equippedFixture(),raw=serialize(s);let n=equipGear(s,'chain',false);assert.equal(n.armorClass,10);n=equipGear(n,'shield',true);assert.equal(n.armorClass,12);n=equipGear(n,'ring',true);
 assert.deepEqual(n.abilities,{FOR:15,DES:12,COS:18,INT:8,SAG:8,CAR:19});assert.equal(n.armorClass,14);assert.equal(n.maxHp,63);assert.equal(n.hp,35);assert.equal(weaponStats(n).attackBonus,weaponStats(s).attackBonus+1);assert.equal(weaponStats(n).damageBonus,weaponStats(s).damageBonus+2);
 assert.equal(n.powers[0].attackModifier,2);assert.equal(n.powers[0].saveModifier,1);assert.equal(n.powers[1].attackModifier,0);assert.equal(n.powers[1].saveModifier,0);assert.equal(describeRoll(n,{kind:'check',key:'DES'}).bonus,3);assert.equal(describeRoll(n,{kind:'save',key:'COS'}).bonus,5);assert.equal(serialize(s),raw);
 n={...n,hp:63};n=equipGear(n,'ring',false);assert.equal(n.hp,53);assert.equal(n.maxHp,53);assert.equal(n.armorClass,12);
});
test('Loadout: armor exclusivity, deletion/quantity/type conversion, bounds and unknown AC rollback',()=>{
 const s=equippedFixture();s.inventory.push({id:'leather',type:'gear',name:'Leather',notes:'',quantity:1});s.extended.loadout.items.push({...emptyGear('leather'),kind:2,armor:11});
 let n=equipGear(s,'leather',true);assert.equal(n.armorClass,11);assert.equal(n.extended.loadout.items[0].equipped,false);n=equipGear(n,'ring',true);const before=serialize(n);
 assert.equal(adjustQuantity(n,'ring',-1).maxHp,53);assert.equal(removeItem(n,'ring').maxHp,53);assert.equal(upsertItem(n,{id:'ring',name:'Ring',notes:'',type:'consumable',quantity:1}).maxHp,53);assert.equal(serialize(n),before);
 const unknown=structuredClone(s);unknown.extended.loadout.unarmored=0;assert.throws(()=>equipGear(unknown,'chain',false),/CA senza/);assert.equal(unknown.armorClass,16);
 s.abilities.COS=30;assert.throws(()=>equipGear(s,'ring',true),/Caratteristica/);assert.equal(s.abilities.COS,30);
 for(const bad of [{...s.extended.loadout,items:[{...emptyGear('chain'),equipped:true},{...emptyGear('chain')}]},{...s.extended.loadout,unarmored:4},{...s.extended.loadout,items:[{...emptyGear('chain'),ability:[11,0,0,0,0,0]}]}])assert.throws(()=>validateLoadout(bad));
});
test('Loadout: versioned PC21/USB18, no silent downgrade and new powers/weapons inherit only their proper bonuses',()=>{
 const n=equipGear(equippedFixture(),'ring',true),raw=serialize(n);assert.equal(JSON.parse(raw).version,21);const loaded=parseBackup(raw);assert.deepEqual(loaded.extended.loadout,n.extended.loadout);assert.equal(projectDevice(loaded).packet.version,18);assert.equal(projectDevice(loaded).packet.magic.powers[1].effect,6);assert.equal(projectDevice(loaded).packet.equipment.items.length,loaded.inventory.length);
 assert.throws(()=>parseBackup(raw.replace('"version": 21','"version": 20')));assert.throws(()=>usePower(n,'ts'),/Pocket/);
 const weapon=upsertItem(n,{...n.inventory[0],id:'fresh',attackModifier:0,damageModifier:0});assert.equal(weapon.inventory.at(-1).attackModifier,1);assert.equal(weapon.inventory.at(-1).damageModifier,2);
 const spell=upsertPower(n,{...n.powers[0],id:'fresh-spell',attackModifier:0,saveModifier:0});assert.equal(spell.powers.at(-1).attackModifier,2);assert.equal(spell.powers.at(-1).saveModifier,1);
 assert.equal(upsertPower(n,{...n.powers[1],id:'fresh-ts'}).powers.at(-1).attackModifier,0);
});
test('Beyond: 2024 True Strike identity and class caster, cantrip readiness, standard armor adopts without double CA',()=>{
 const p=beyondRichFixture();p.data.inventory.push({id:999,quantity:1,equipped:true,definition:{name:'Chain Mail',filterType:'Armor',armorTypeId:3,armorClass:16,grantedModifiers:[]}});
 const d={id:2619204,name:'True Strike',level:0,concentration:false,sources:[{sourceId:145,sourceType:1}],description:'Synthetic spell'};
 p.data.classSpells[0].spells.push({id:999,prepared:false,definition:d});const before=structuredClone(p),r=convertBeyond(p,'123456');assert.ok(r.canImport,r.conversion?.blockers?.join());const s=r.draft,ts=s.powers.find(p=>p.name==='True Strike');assert.equal(ts.effect,'true-strike-2024');assert.equal(ts.castingAbility,'INT');assert.equal(ts.prepared,true);assert.equal(s.extended.loadout.items.find(g=>g.kind===4).equipped,true);assert.equal(equipGear(s,s.extended.loadout.items.find(g=>g.kind===4).id,false).armorClass,s.armorClass-16+10+Math.floor((s.abilities.DES-10)/2)+Math.floor((s.abilities.COS-10)/2));assert.deepEqual(p,before);
 for(const change of [()=>d.id=999999,()=>{d.id=2619204;d.sources=[{sourceId:1,sourceType:1}]},()=>{d.sources=[{sourceId:145,sourceType:1}];d.concentration=true}]){change();const spell=convertBeyond(p,'123456').draft.powers.find(p=>p.name==='True Strike');assert.equal(spell.effect,undefined);}
});
test('Semantic icons: specific exceptions win, bilingual stable aliases, typed item priority and honest fallback',()=>{
 for(const [name,glyph] of [['True Strike','strike'],['Colpo Accurato','strike'],['Eldritch Blast','blast'],['Chill Touch','skull'],['Faerie Fire','eye'],['Sacred Flame','sun'],['Hellish Rebuke','fire'],['Mage Hand','hand'],['Darkness','dark']])assert.equal(semanticGlyph('spell',name),glyph);
 assert.equal(semanticGlyph('item','Claymore'),'sword');assert.equal(semanticGlyph('item','Unknown',{kind:4}),'armor');assert.equal(semanticGlyph('item','Unknown'),'gear');assert.equal(semanticGlyph('spell','Unknown'),'scroll');assert.match(semanticIcon('spell','True Strike'),/shape-rendering="crispEdges"/);assert.doesNotMatch(semanticIcon('item','<script>'),/script/);
});
