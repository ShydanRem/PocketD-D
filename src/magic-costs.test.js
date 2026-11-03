import test from 'node:test';
import assert from 'node:assert/strict';
import {initialState} from './model.js';
import {availableSlots,usePower} from './powers.js';
import {emptyExtended} from './extended.js';

test('Slot picker: lowest level across ordinary/pact, explicit distinct payments and no expense on lookup',()=>{
  const s=initialState(),power={id:'costs',name:'Synthetic cost',type:'spell',description:'',level:2,concentration:false,castingAbility:'INT',prepared:true};
  s.extended=emptyExtended();s.powers=[power];s.slots=Array.from({length:9},(_,i)=>({max:3,current:i===1||i===8?2:0}));
  s.extended.pactSlots=Array.from({length:9},(_,i)=>({max:2,current:i===1||i===2?1:0}));
  const before=structuredClone(s),choices=availableSlots(s,power);
  assert.deepEqual(choices.map(x=>x.value),['2','pact:2','pact:3','9']);assert.deepEqual(s,before);
  for(const choice of choices){const paid=usePower(s,power.id,choice.value),pact=choice.value.startsWith('pact:'),level=Number(choice.value.replace('pact:',''));
    for(let i=0;i<9;i++){
      assert.equal(paid.slots[i].current,s.slots[i].current-(!pact&&i===level-1?1:0));
      assert.equal(paid.extended.pactSlots[i].current,s.extended.pactSlots[i].current-(pact&&i===level-1?1:0));
    }
  }
  assert.deepEqual(s,before);
});
test('Slot choices preserve free cantrips/innate uses and first cannon creation',()=>{
  const s=initialState();s.extended={pactSlots:Array.from({length:9},()=>({max:0,current:0}))};s.slots[4]={max:1,current:1};s.extended.pactSlots[0]={max:2,current:1};
  for(const power of [{level:0},{level:1,usesSpellSlot:false},{type:'feature',effect:'eldritch-cannon',remaining:1}])assert.deepEqual(availableSlots(s,power),[]);
  assert.equal(availableSlots(s,{type:'feature',effect:'eldritch-cannon',remaining:0})[0].value,'pact:1');
});
