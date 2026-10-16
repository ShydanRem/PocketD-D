// Build an authentic pre-inventory backup for migration regression tests.
export function legacyBackup(state,version=8) {
  const old={...state,version,swordEquipped:state.equippedWeaponId!==null,
    potions:state.inventory.find(item=>item.id==='starter-potion')?.quantity??0,
    torches:state.inventory.find(item=>item.id==='starter-torch')?.quantity??0,
    weaponReach:state.inventory.find(item=>item.id==='starter-sword')?.reach??1};
  delete old.inventory;delete old.equippedWeaponId;
  if(version<8)delete old.weaponReach;
  return old;
}
