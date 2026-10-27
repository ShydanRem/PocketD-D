import {DIE_SIDES} from './inventory.js';
const integer=(v,min,max)=>Number.isSafeInteger(v)&&v>=min&&v<=max;
const array=v=>Array.isArray(v)?v:[];
export function beyondItems(source,inventory,{warn,evidence}){
 let charges=0,consumables=0,healing=0,unresolved=0;
 const items=inventory.map((item,index)=>{
  const src=source[index],d=src.definition,l=src.limitedUse;
  const uncertain=message=>{unresolved++;warn(`${item.name}: ${message}`);};
  const consumable=d.isConsumable===true||l?.resetType==='Consumable';
  if(consumable){
   if(d.isConsumable===false||item.type==='weapon'||src.chargesUsed!=null&&src.chargesUsed!==0||l&&(l.resetType!=='Consumable'||!integer(l.maxUses,1,1)||!integer(l.numberUsed,0,0))){uncertain('consumabile con stato non riconciliato; oggetto conservato, uso manuale.');return item;}
   consumables++;let result={...item,type:'consumable'};
   if(d.filterType==='Potion'){
    const grants=array(d.grantedModifiers),g=grants[0],dice=g?.dice,bonuses=[dice?.fixedValue,g?.fixedValue,g?.value].filter(v=>v!=null),fixed=bonuses[0]??0;
    const parsed=typeof dice?.diceString==='string'?dice.diceString.trim().match(/^(\d+)d(\d+)(?:\s*([+-])\s*(\d+))?$/i):null;
    const consistent=!dice?.diceString||parsed&&Number(parsed[1])===dice.diceCount&&Number(parsed[2])===dice.diceValue&&(parsed[3]==='-'?-1:1)*Number(parsed[4]??0)===fixed;
    if(d.isHomebrew!==true&&d.canAttune!==true&&grants.length===1&&g.type==='bonus'&&g.subType==='hit-points'&&g.statId==null&&g.duration==null&&!g.restriction&&g.isGranted!==false&&g.requiresAttunement!==true&&(!g.tagConstraints||Array.isArray(g.tagConstraints)&&!g.tagConstraints.length)&&integer(dice?.diceCount,1,6)&&DIE_SIDES.includes(dice?.diceValue)&&(dice.diceMultiplier==null||dice.diceMultiplier===1)&&integer(fixed,0,30)&&bonuses.every(v=>v===fixed)&&consistent){
     result={...result,type:'healing',diceCount:dice.diceCount,dieSides:dice.diceValue,healingModifier:fixed};healing++;
    }else uncertain('pozione conservata come consumabile: cura o altri effetti non rappresentabili automaticamente.');
   }
   return result;
  }
  if(l==null){if(src.chargesUsed!=null&&src.chargesUsed!==0)uncertain('cariche usate senza massimo esplicito: saldo da configurare.');return item;}
  if(!l||typeof l!=='object'||Array.isArray(l)||Object.keys(l).some(k=>!['maxUses','numberUsed','resetType','resetTypeDescription','resetDice','minNumberConsumed','maxNumberConsumed'].includes(k))||!integer(l.maxUses,1,999)||item.quantity>1){uncertain('cariche incomplete o pila con saldi individuali non rappresentabili; nessun saldo inventato.');return item;}
  const used=l.numberUsed??src.chargesUsed;
  if(!integer(used,0,l.maxUses)||(src.chargesUsed!=null&&src.chargesUsed!==used)){uncertain('registri delle cariche mancanti o discordanti; saldo da verificare.');return item;}
  let recovery=l.resetType==='Short Rest'?'short':l.resetType==='Long Rest'?'long':'manual';
  if(l.resetTypeDescription!=null&&(typeof l.resetTypeDescription!=='string'||l.resetTypeDescription.trim())||l.resetDice!=null)recovery='manual';
  if(recovery==='manual')warn(`${item.name}: saldo cariche importato; recupero giornaliero, variabile o non riconosciuto da gestire manualmente.`);
  charges++;return {...item,charges:{max:l.maxUses,current:l.maxUses-used,recovery}};
 });
 if(charges||consumables)evidence.push(`Inventario operativo: ${charges} saldi cariche, ${consumables} consumabili, di cui ${healing} cure con formula esplicita. Quantità conservate; cariche e quantità sono distinte.`);
 return {items,charges,consumables,healing,unresolved};
}
