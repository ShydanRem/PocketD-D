import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState,serialize,parseBackup} from './model.js';
import {emptyExtended} from './extended.js';
import {projectDevice} from './device-export.js';
import {validateSpellFormula,resolveSpellFormula,emptySpellFormula} from './spell-formula.js';
import {validatePower} from './powers.js';
import {beyondSpellFormula} from './beyond-spell-formula.js';
import {formulaDefinition,formulaState} from './spell-formula-fixtures.js';
test('Spell formulas: portable v25/USB22, exact isolated backup, ability and slot scaling without expense',()=>{
  const s=formulaState(),before=structuredClone(s),r=parseBackup(serialize(s));assert.equal(JSON.parse(serialize(s)).version,25);assert.deepEqual(r.powers[0].formula,s.powers[0].formula);assert.equal(projectDevice(s).packet.version,22);assert.deepEqual(projectDevice(s).packet.magic.powers[0].formula,s.powers[0].formula);
  for(let level=1;level<=9;level++)for(const pact of [false,true])assert.deepEqual(resolveSpellFormula(s,s.powers[0],level,pact),{kind:2,count:2*level,sides:8,bonus:4});
  assert.deepEqual(s,before);const low=JSON.parse(serialize(s));low.version=24;assert.throws(()=>parseBackup(JSON.stringify(low)));delete s.powers[0].formula;assert.notEqual(JSON.parse(serialize(s)).version,25);assert.notEqual(projectDevice(s).packet.version,22);
});
test('Spell formulas: character thresholds, finite slot-free base, overflow rejection and strict shapes',()=>{
  const s=formulaState(),p=s.powers[0];p.level=0;p.usesSpellSlot=false;p.formula={kind:1,count:1,sides:10,ability:0,scaling:2,step:1,source:0,bonus:0};
  for(let level=1;level<=20;level++){s.level=level;assert.equal(resolveSpellFormula(s,p).count,1+Number(level>=5)+Number(level>=11)+Number(level>=17));}
  assert.throws(()=>resolveSpellFormula(s,p,1));assert.throws(()=>resolveSpellFormula(s,p,0,true));
  p.formula.count=18;assert.throws(()=>resolveSpellFormula(s,p));p.formula.count=1;p.formula.ability=1;p.formula.bonus=60;assert.throws(()=>resolveSpellFormula(s,p));
  const good=formulaState().powers[0],f=good.formula;
  for(const patch of [{kind:3},{count:0},{sides:7},{ability:2},{source:2},{step:0},{bonus:61},{bonus:-16},{scaling:3},{extra:true},{count:NaN}])assert.throws(()=>validateSpellFormula({...f,...patch},good));
  assert.throws(()=>validatePower({...good,type:'feature'},true));assert.throws(()=>validatePower({...good,effect:'hex-2024',concentration:true},true));assert.deepEqual(validateSpellFormula(emptySpellFormula(),{}),emptySpellFormula());assert.throws(()=>validateSpellFormula({...emptySpellFormula(),bonus:1},good));
});
test('Beyond formula identities: structured base + full scaling, no name/text inference, mismatches stay manual',()=>{
  for(const id of [2619079,2618890,2618967]){
    const d=formulaDefinition(id),before=structuredClone(d),notes=[];const f=beyondSpellFormula(d,{evidence:t=>notes.push(t)});assert.equal(f.source,1);assert.equal(f.kind,id===2619079?2:1);assert.equal(notes.length,1);assert.deepEqual(d,before);
    for(const mutate of [d=>d.id++,d=>d.isHomebrew=true,d=>d.sources[0].sourceId=1,d=>d.modifiers[0].die.diceString='99d20',d=>d.modifiers[0].restriction='Conditional',d=>d.modifiers[0].usePrimaryStat=!d.modifiers[0].usePrimaryStat,d=>d.atHigherLevels.special.push({}),d=>d.modifiers[0].atHigherLevels.higherLevelDefinitions.pop(),d=>d.modifiers[0].atHigherLevels.higherLevelDefinitions[0].dice.diceCount++,d=>d.modifiers.push({...d.modifiers[0]}),d=>d.modifiers[0].requiresAttunement='yes',d=>d.modifiers[0].bonusTypes.push(1),d=>d.duration.durationType='Concentration']){const bad=structuredClone(d);mutate(bad);assert.equal(beyondSpellFormula(bad),undefined);}
  }
  const d=formulaDefinition();d.id=12;d.name='Cure Wounds';d.description='2d8 + casting modifier';assert.equal(beyondSpellFormula(d),undefined);
});
