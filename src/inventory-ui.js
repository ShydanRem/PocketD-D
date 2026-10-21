import {MAX_ITEMS,DIE_SIDES,upsertItem,removeItem,equipWeapon} from './inventory.js';

export function openInventoryManager({getState,commit,newId,escapeHtml,abilityKeys}) {
  const dialog=document.createElement('dialog');dialog.className='editor-dialog inventory-manager';
  let editing=null;
  const labels={gear:'Oggetto',consumable:'Consumabile',healing:'Cura',weapon:'Arma'};
  const number=(label,name,value,min,max)=>`<label>${label}<input name="${name}" type="number" min="${min}" max="${max}" value="${value}" required></label>`;
  function draw(){
    const state=getState();
    dialog.innerHTML=`<div class="editor-heading"><div><div class="eyebrow">APP DI SUPPORTO · ${state.inventory.length}/${MAX_ITEMS} OGGETTI</div><h2>Inventario e armi</h2></div><button data-inventory-close aria-label="Chiudi inventario">×</button></div><p class="editor-intro">Quantità e regole sono manuali. Equipaggia una sola arma; i bonus scelti valgono per quest'arma. Armature, munizioni, peso e proprietà speciali restano al DM.</p><button data-new-item class="rest-button" ${state.inventory.length>=MAX_ITEMS?'disabled':''}>+ Aggiungi oggetto o arma</button><div class="manager-list inventory-manager-list">${state.inventory.map(item=>`<div><span>${escapeHtml(item.name)} ×${item.quantity}<small>${labels[item.type]}${item.charges?` · Cariche ${item.charges.current}/${item.charges.max}`:''}${state.equippedWeaponId===item.id?' · Equipaggiata':''}</small></span>${item.type==='weapon'?`<button data-manager-equip="${item.id}" ${item.quantity===0?'disabled':''}>${state.equippedWeaponId===item.id?'Riponi':'Equipaggia'}</button>`:''}<button data-edit-item="${item.id}">Modifica</button><button data-delete-item="${item.id}">Elimina</button></div>`).join('')||'<p class="editor-intro">Inventario vuoto. Aggiungi il primo oggetto.</p>'}</div><div id="item-form-container"></div><p class="editor-error" role="alert"></p><p class="manager-status" role="status"></p>`;
  }
  function drawForm(){
    const item=editing;
    dialog.querySelector('#item-form-container').innerHTML=`<form id="item-editor"><fieldset><legend>${getState().inventory.some(entry=>entry.id===item.id)?'Modifica oggetto':'Nuovo oggetto'}</legend><label>Tipo<select name="type">${Object.entries(labels).map(([type,label])=>`<option value="${type}" ${item.type===type?'selected':''}>${label}</option>`).join('')}</select></label><div class="inventory-form-grid"><label>Nome<input name="item-name" maxlength="40" value="${escapeHtml(item.name)}" required></label>${number('Quantità','quantity',item.quantity,0,999)}</div><label>Note<textarea name="notes" maxlength="400" rows="3">${escapeHtml(item.notes)}</textarea></label><label class="checkbox-label"><input name="charges-enabled" type="checkbox" ${item.charges?'checked':''}>Gestisci cariche</label><div data-charge-fields><div class="inventory-form-grid">${number('Cariche attuali','charge-current',item.charges?.current??0,0,999)}${number('Cariche massime','charge-max',item.charges?.max??1,1,999)}<label>Recupero<select name="charge-recovery">${Object.entries({manual:'Manuale',short:'Riposo breve o lungo',long:'Riposo lungo'}).map(([value,label])=>`<option value="${value}" ${value===(item.charges?.recovery??'manual')?'selected':''}>${label}</option>`).join('')}</select></label></div><p class="editor-intro">Un saldo condiviso per questa voce, indipendente dalla quantità. Per copie con cariche separate crea voci distinte. Il recupero al riposo riempie il saldo; regole speciali e ricariche casuali restano manuali.</p></div><div data-dice-fields class="inventory-form-grid">${number('Numero dadi','diceCount',item.diceCount??1,1,6)}<label>Facce<select name="dieSides">${DIE_SIDES.map(sides=>`<option value="${sides}" ${(item.dieSides??6)===sides?'selected':''}>d${sides}</option>`).join('')}</select></label></div><div data-weapon-fields><div class="inventory-form-grid"><label>Caratteristica<select name="ability">${abilityKeys.map(key=>`<option ${key===(item.ability??'FOR')?'selected':''}>${key}</option>`).join('')}</select></label>${number('Portata (caselle)','reach',item.reach??1,1,getState().extended?120:9)}${number('Bonus extra attacco','attackModifier',item.attackModifier??0,-10,10)}${number('Bonus extra danni','damageModifier',item.damageModifier??0,-10,10)}</div><label class="checkbox-label"><input name="proficient" type="checkbox" ${(item.proficient??true)?'checked':''}>Aggiungi competenza all’attacco</label><label class="checkbox-label"><input name="addAbilityToDamage" type="checkbox" ${(item.addAbilityToDamage??true)?'checked':''}>Aggiungi caratteristica ai danni</label><label>Tipo di danno (nota)<input name="damageType" maxlength="24" value="${escapeHtml(item.damageType??'')}"></label><p class="editor-intro">Il critico raddoppia il numero di dadi. I bonus restano singoli. Portata e linea di vista usano la griglia semplificata.</p></div><div data-healing-fields>${number('Bonus cura','healingModifier',item.healingModifier??0,0,30)}<p class="editor-intro">Usare la cura consuma un oggetto. A PF pieni non consuma nulla.</p></div><div class="editor-controls"><button type="button" data-cancel-item>Annulla modifica</button><button type="submit" class="roll-button">Salva oggetto</button></div></fieldset></form>`;
    toggleFields();
  }
  function toggleFields(){
    const form=dialog.querySelector('#item-editor');if(!form)return;
    const type=form.elements.type.value;
    for(const [selector,active] of [['[data-charge-fields]',form.elements['charges-enabled'].checked],['[data-dice-fields]',['weapon','healing'].includes(type)],['[data-weapon-fields]',type==='weapon'],['[data-healing-fields]',type==='healing']]){
      const block=form.querySelector(selector);block.hidden=!active;block.querySelectorAll('input,select').forEach(input=>input.disabled=!active);
    }
  }
  dialog.addEventListener('click',event=>{
    const button=event.target.closest('button');if(!button||button.disabled)return;
    try{
      if(button.hasAttribute('data-inventory-close')){dialog.close();return}
      if(button.hasAttribute('data-new-item')){editing={id:newId(),type:'gear',name:'',notes:'',quantity:1};drawForm();dialog.querySelector('[name="item-name"]').focus()}
      if(button.dataset.editItem){editing=structuredClone(getState().inventory.find(item=>item.id===button.dataset.editItem));drawForm();dialog.querySelector('#item-editor').scrollIntoView({block:'nearest'})}
      if(button.hasAttribute('data-cancel-item')){editing=null;dialog.querySelector('#item-form-container').innerHTML=''}
      if(button.dataset.managerEquip){const state=getState();commit(equipWeapon(state,state.equippedWeaponId===button.dataset.managerEquip?null:button.dataset.managerEquip));editing=null;draw()}
      if(button.dataset.deleteItem){
        const item=getState().inventory.find(entry=>entry.id===button.dataset.deleteItem);
        if(!confirm(`Eliminare ${item.name} dall’inventario?`))return;
        commit(removeItem(getState(),item.id));editing=null;draw();
      }
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('change',event=>{if(['type','charges-enabled'].includes(event.target.name))toggleFields()});
  dialog.addEventListener('submit',event=>{
    event.preventDefault();const data=new FormData(event.target);
    try{
      const item={...editing,id:editing.id,type:data.get('type'),name:data.get('item-name').trim(),notes:data.get('notes'),quantity:Number(data.get('quantity')),
        diceCount:Number(data.get('diceCount')),dieSides:Number(data.get('dieSides')),ability:data.get('ability'),proficient:data.has('proficient'),
        addAbilityToDamage:data.has('addAbilityToDamage'),attackModifier:Number(data.get('attackModifier')),damageModifier:Number(data.get('damageModifier')),
        reach:Number(data.get('reach')),damageType:data.get('damageType'),healingModifier:Number(data.get('healingModifier'))};
      if(data.has('charges-enabled'))item.charges={current:Number(data.get('charge-current')),max:Number(data.get('charge-max')),recovery:data.get('charge-recovery')};
      else delete item.charges;
      commit(upsertItem(getState(),item));editing=null;draw();dialog.querySelector('.manager-status').textContent='Oggetto salvato';
    }catch(error){dialog.querySelector('.editor-error').textContent=error.message}
  });
  dialog.addEventListener('close',()=>dialog.remove());draw();document.body.append(dialog);dialog.showModal();
}
