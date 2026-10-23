// Original synthetic character for the first physical session. No browser storage.
import {readFileSync} from 'node:fs';
import {spawnSync} from 'node:child_process';
import {fileURLToPath} from 'node:url';
import assert from 'node:assert/strict';
import {initialState,describeRoll} from '../src/model.js';
import {emptyExtended} from '../src/extended.js';
import {projectDevice,serializeDevice} from '../src/device-export.js';

const state=initialState();state.name='Elian Collaudo';state.extended=emptyExtended();
state.extended.tempHp=3;state.extended.currenciesAvailable=true;state.extended.currencies.gp=12;
state.extended.hitDice=[{name:'Classe di prova',die:8,max:3,used:1}];
state.skillRanks.athletics=1;state.saveProficiencies.COS=true;
state.slots[0]={current:2,max:3};state.extended.pactSlots[0]={current:1,max:1};
state.powers=[
  {id:'test-light',type:'spell',name:'Luce di prova',description:'Magia originale di collaudo. Consuma uno slot e avvia la concentrazione; nessun effetto numerico automatico.',level:1,concentration:true,prepared:true,usesSpellSlot:true,castingAbility:'INT',attackModifier:0,saveModifier:0},
  {id:'test-step',type:'feature',name:'Passo di prova',description:'Capacita originale di collaudo. Consuma un uso e recupera al riposo breve; nessun movimento automatico.',maxUses:2,remaining:1,recovery:'short'}
];
const wire=serializeDevice(projectDevice(state).packet)+'\n';
if(process.argv.length===2){process.stdout.write(wire);}
else if(process.argv.length===3&&process.argv[2]==='--check'){
  const fixture=new URL('../firmware/tests/fixtures/hardware-session.usb9.synthetic.json',import.meta.url);
  assert.equal(readFileSync(fixture,'utf8'),wire,'Regenerate the synthetic hardware session fixture');
  const probe=fileURLToPath(new URL('../firmware/tests/out/checks-probe',import.meta.url));
  const result=spawnSync(probe,[fileURLToPath(fixture)],{encoding:'utf8'});
  assert.equal(result.status,0,result.stderr||String(result.error));
  const rows=JSON.parse(result.stdout),kinds=['check','skill','save'];assert.equal(rows.length,30);
  for(const row of rows){const expected=describeRoll(state,{kind:kinds[row.kind],key:row.key});assert.equal(row.total,expected.bonus);assert.equal(row.blocked,expected.blocked);}
  console.log('Synthetic hardware session: committed USB9 fixture matches the PC projector; actual C parser validates all fields and 30 check bonuses match');
}else{throw new Error('Usage: node scripts/hardware-session.js [--check]');}
