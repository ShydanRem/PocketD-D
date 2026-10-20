// Synthetic integration: PC model -> actual USB JSON parser -> actual C bonuses/modes.
import {spawnSync} from 'node:child_process';
import {writeFileSync} from 'node:fs';
import assert from 'node:assert/strict';
import {initialState,skills,abilityKeys,describeRoll,modifier,proficiency} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';
import {convertBeyond} from '../src/beyond-conversion.js';
import {beyondRichFixture} from '../src/beyond-fixtures.js';
const states=[];
for(let level=1;level<=20;level++){
  const s=initialState();s.level=level;s.extended=emptyExtended();s.condition=level%3===0;
  abilityKeys.forEach((key,i)=>{s.abilities[key]=1+(level*7+i*5)%30;s.saveProficiencies[key]=(i+level)%2===0;s.extended.saveBonuses[key]=i-3;s.extended.checkBonuses[key]=3-i});
  skills.forEach(({id},i)=>{s.skillRanks[id]=(i+level)%3;s.extended.skillBonuses[id]=(i+level)%61-30});
  s.extended.halfAbilityChecks=level%2===1;s.extended.halfSkills=skills.filter((_,i)=>i%2===level%2).map(k=>k.id);
  if(level%4===0){s.extended.blockedChecks=['check:FOR','skill:arcana','save:DES'];s.extended.combat.raging=true;s.extended.combat.rageBonus=2}
  states.push(s);
}
states.push(convertBeyond(beyondRichFixture(),'123456').draft);
const input='firmware/tests/out/training-input.jsonl';
writeFileSync(input,states.map(s=>serializeDevice(projectDevice(s).packet)).join('\n')+'\n');
const run=spawnSync('firmware/tests/out/checks-probe',[input],{encoding:'utf8',maxBuffer:4*1024*1024,timeout:15000});
assert.equal(run.status,0,run.stderr);const lines=run.stdout.trim().split('\n');assert.equal(lines.length,states.length);
let checked=0;
for(let n=0;n<states.length;n++){
  const s=states[n],rows=JSON.parse(lines[n]);assert.equal(rows.length,30);
  const contexts=[...abilityKeys.map(key=>({kind:'check',key})),...skills.map(({id})=>({kind:'skill',key:id})),...abilityKeys.map(key=>({kind:'save',key}))];
  rows.forEach((row,i)=>{
    const context=contexts[i];assert.equal(row.key,context.key);const skill=context.kind==='skill'?skills.find(k=>k.id===context.key):null;
    assert.equal(row.base,modifier(s.abilities[skill?.ability??context.key]));
    const rank=context.kind==='skill'?s.skillRanks[context.key]:context.kind==='save'?Number(s.saveProficiencies[context.key]):0;
    const half=context.kind==='check'?s.extended?.halfAbilityChecks:context.kind==='skill'&&!rank&&s.extended?.halfSkills.includes(context.key);
    assert.equal(row.trained,rank*proficiency(s.level)+(half?Math.floor(proficiency(s.level)/2):0));
    ['disadvantage','normal','advantage'].forEach((mode,j)=>{
      const expected=describeRoll({...s,mode},context);
      assert.equal(row.total,expected.bonus,`${n}:${context.kind}:${context.key}`);assert.equal(row.blocked,expected.blocked);
      assert.equal(row.modes[j],expected.mode==='advantage'?1:expected.mode==='disadvantage'?-1:0);checked++;
    });
  });
}
console.log(`PC -> USB -> C: ${checked} bonus/mode comparisons passed, levels 1..20 and synthetic Beyond conversion; no personal character read`);
