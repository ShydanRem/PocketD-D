import {test} from 'node:test';import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup,describeRoll,weaponStats} from './model.js';
import {emptyExtended} from './extended.js';
import {emptyGear,equipGear,attuneGear} from './equipment.js';
import {adjustQuantity,removeItem} from './inventory.js';
import {convertBeyondFull} from './beyond-conversion.js';
import {beyondRichFixture} from './beyond-fixtures.js';
import {projectDevice} from './device-export.js';
import {spellStats} from './powers.js';
import {semanticGlyph} from './semantic-icons.js';
const grant=(id,subType,value,more={})=>({id,type:'bonus',subType,value,isGranted:true,requiresAttunement:false,restriction:'',...more});
function source({worn=true,attuned=true,requires=true}={}){
 const p=beyondRichFixture(),c=p.data;c.conditions=[];c.temporaryHitPoints=0;
 const grants=[grant('equip-ac','armor-class',1),grant('equip-save','saving-throws',1),grant('equip-spell','spell-attacks',2),grant('equip-dc','spell-save-dc',1)];
 c.inventory.push({id:800,quantity:1,equipped:worn,isAttuned:attuned,definition:{id:801,name:'Anonymous relic',filterType:'Other Gear',canAttune:requires,grantedModifiers:grants}});
 if(worn&&(!requires||attuned))c.modifiers.item.push(...structuredClone(grants));
 return p;
}
test('General import: worn and attuned are distinct for any class, active source numbers adopted exactly once',()=>{
 for(const worn of [false,true])for(const attuned of [false,true])for(const stat of [1,2,3,4,5,6]){
  const p=source({worn,attuned}),cl=p.data.classes[0];cl.definition.name=`Anonymous class ${stat}`;cl.definition.spellCastingAbilityId=stat;
  const before=structuredClone(p),r=convertBeyondFull(p,'123456');assert.ok(r.canImport,r.conversion.blockers.join());const s=r.draft,g=s.extended.loadout.items.find(g=>g.id==='beyond-item-4');assert.ok(g);assert.equal(g.equipped,worn);assert.equal(g.attunement,attuned?2:1);assert.equal(g.ac,1);assert.equal(g.spellAttack,2);
  const base=convertBeyondFull(source({worn:false,attuned:false}),'123456').draft;
  assert.equal(s.armorClass,base.armorClass+(worn&&attuned?1:0));assert.equal(s.extended.saveBonuses.FOR,worn&&attuned?1:0);assert.equal(s.powers.find(p=>p.type==='spell').attackModifier,worn&&attuned?2:0);
  let n=equipGear(attuneGear(s,g.id,false),g.id,true);assert.equal(n.armorClass,base.armorClass);n=attuneGear(n,g.id,true);assert.equal(n.armorClass,base.armorClass+1);assert.equal(n.powers.find(p=>p.type==='spell').attackModifier,2);assert.deepEqual(p,before);
 }
});
test('General import: permanent bonuses work without any armor; recognized stat deltas respect caps/overrides',()=>{
 for(const index of [0,1,2,3,4,5]){
  const p=source({requires:false}),g=p.data.inventory.at(-1).definition.grantedModifiers;
  const bonus=grant('score',`${['strength','dexterity','constitution','intelligence','wisdom','charisma'][index]}-score`,2);g.push(bonus);p.data.modifiers.item.push(bonus);
  const r=convertBeyondFull(p,'123456');assert.ok(r.canImport);const s=r.draft,item=s.extended.loadout.items.find(g=>g.id==='beyond-item-4');assert.ok(item);assert.equal(item.ability[index],2);const n=equipGear(s,item.id,false);assert.equal(s.abilities[['FOR','DES','COS','INT','SAG','CAR'][index]]-n.abilities[['FOR','DES','COS','INT','SAG','CAR'][index]],2);assert.equal(n.hp,Math.min(s.hp,n.maxHp));
  p.data.overrideStats[index].value=18;const overridden=convertBeyondFull(p,'123456');assert.ok(overridden.canImport);assert.equal(overridden.draft.extended.loadout,undefined);assert.ok(overridden.warnings.some(t=>/profilo automatico/.test(t)));
 }
 const p=source({requires:false});p.data.stats[0].value=20;const g=grant('capped','strength-score',2);p.data.inventory.at(-1).definition.grantedModifiers.push(g);p.data.modifiers.item.push(g);const r=convertBeyondFull(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.extended.loadout,undefined);
});
test('General import: conditional, ambiguous, unsupported and discrepant item properties stay explicitly manual',()=>{
 for(const mutate of [g=>g.restriction='while hidden',g=>g.duration=60,g=>g.dice={diceCount:1,diceValue:4},g=>g.type='set',g=>g.subType='fire',g=>g.value=99,g=>g.entityId=2]){
  const p=source({worn:false});mutate(p.data.inventory.at(-1).definition.grantedModifiers[1]);const r=convertBeyondFull(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.extended.loadout,undefined);assert.ok(r.warnings.some(t=>t.includes('profilo automatico non applicabile')));
 }
 const p=source();p.data.inventory.at(-1).definition.grantedModifiers[1].value=2;const r=convertBeyondFull(p,'123456');assert.ok(r.canImport);assert.equal(r.draft.extended.loadout,undefined);
});
test('Attunement state: versioned transport, native-independent HP caps, quantity/removal, unchanged source and no downgrade',()=>{
 const r=convertBeyondFull(source(),'123456'),s=r.draft,g=s.extended.loadout.items[0];assert.equal(JSON.parse(serialize(s)).version,22);assert.deepEqual(parseBackup(serialize(s)).extended.loadout,s.extended.loadout);const p=projectDevice(s).packet;assert.equal(p.version,19);assert.equal(p.equipment.items[3].attunement,2);assert.ok(p.equipment.items.every(g=>Number.isInteger(g.attunement)));
 assert.throws(()=>parseBackup(serialize(s).replace('"version": 22','"version": 21')));const n=adjustQuantity(s,g.id,-1);assert.equal(n.extended.loadout.items[0].attunement,1);assert.equal(n.extended.loadout.items[0].equipped,false);assert.throws(()=>attuneGear(n,g.id,true));assert.ok(!removeItem(s,g.id).extended.loadout.items.some(x=>x.id===g.id));assert.equal(s.inventory[3].quantity,1);
});
test('Equipment icon selection recognizes jewelry, clothing and foci consistently',()=>{
 for(const [name,id] of [['Ring of protection','ring'],['Anello','ring'],['Cloak','cloak'],['Mantello','cloak'],['Staff','staff'],['Bastone','staff'],['Boots','boots'],['Stivali','boots'],['Amulet','amulet'],['Collana','amulet']])assert.equal(semanticGlyph('item',name),id);
});

test('Modifier-scoped attunement, inactive future score caps and HP overrides are handled without class-name shortcuts',()=>{
 const p=source({requires:false,attuned:false});p.data.modifiers.item=[];for(const g of p.data.inventory.at(-1).definition.grantedModifiers)g.requiresAttunement=true;
 const r=convertBeyondFull(p,'123456');assert.ok(r.canImport,r.conversion.blockers.join());assert.equal(r.draft.extended.loadout.items[0].attunement,1);
 const hp=source({requires:false});hp.data.overrideHitPoints=100;const g=grant('flat-hp','hit-points',5);hp.data.inventory.at(-1).definition.grantedModifiers.push(g);hp.data.modifiers.item.push(g);const h=convertBeyondFull(hp,'123456');assert.ok(h.canImport);assert.equal(h.draft.maxHp,100);assert.equal(h.draft.extended.loadout,undefined);
 const cap=source({requires:false,worn:false});cap.data.stats[0].value=20;cap.data.inventory.at(-1).definition.grantedModifiers.push(grant('future-cap','strength-score',2));const c=convertBeyondFull(cap,'123456');assert.ok(c.canImport);assert.equal(c.draft.extended.loadout,undefined);
});
