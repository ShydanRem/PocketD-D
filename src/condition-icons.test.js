import test from 'node:test';
import assert from 'node:assert/strict';
import {semanticGlyph} from './semantic-icons.js';
test('Condition icons: all fifteen bilingual notes share explicit glyphs; unknown notes stay generic',()=>{
 const pairs=[['Accecato','Blinded','blind'],['Affascinato','Charmed','heart'],['Assordato','Deafened','deaf'],['Spossatezza','Exhaustion','tired'],['Spaventato','Frightened','fear'],['Afferrato','Grappled','hand'],['Incapacitato','Incapacitated','stun'],['Invisibile','Invisible','dark'],['Paralizzato','Paralyzed','chain'],['Pietrificato','Petrified','stone'],['Avvelenato','Poisoned','skull'],['Prono','Prone','prone'],['Trattenuto','Restrained','chain'],['Stordito','Stunned','stun'],['Privo di sensi','Unconscious','sleep']];
 for(const [it,en,glyph] of pairs){assert.equal(semanticGlyph('condition',it),glyph);assert.equal(semanticGlyph('condition',en),glyph);}
 assert.equal(semanticGlyph('condition','Custom annotation'),'scroll');
});
