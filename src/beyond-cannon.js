// Bounded Tasha's variant, identified by source entities, not by the action name alone.
export function beyondCannon(a,f){
 const c=f?.owner,d=c?.definition,s=c?.subclassDefinition,l=a.limitedUse;
 const book=v=>Array.isArray(v?.sources)&&v.sources.some(x=>x?.sourceId===67&&x.sourceType===1);
 return f?.group==='class'&&d?.id===252717&&d.name==='Artificer'&&d.isHomebrew===false&&book(d)&&
  s?.id===254188&&s.name==='Artillerist'&&s.isHomebrew===false&&book(s)&&
  f.d.id===1138963&&f.d.name==='Eldritch Cannon'&&f.d.requiredLevel===3&&f.d.isHomebrew!==true&&book(f.d)&&
  Number.isInteger(c.level)&&c.level>=3&&c.level<=20&&String(a.id)==='1038966'&&a.componentId===1138963&&a.componentTypeId===12168134&&
  a.name==='Create Eldritch Cannon'&&a.activation?.activationType===1&&(a.activation.activationTime==null||a.activation.activationTime===1)&&
  l?.maxUses===1&&l.resetType===2&&Number.isInteger(l.numberUsed)&&l.numberUsed>=0&&l.numberUsed<=1&&
  l.statModifierUsesId==null&&!l.useProficiencyBonus&&l.resetDice==null&&
  (l.minNumberConsumed==null||l.minNumberConsumed===1)&&(l.maxNumberConsumed==null||l.maxNumberConsumed===1);
}
