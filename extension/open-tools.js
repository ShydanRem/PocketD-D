// Deduplicate only this extension's own UI; never navigate or focus a tab that
// the user has since taken elsewhere. No account/bridge data is returned.
const states=new WeakMap();
export function openPocketTools(api,view='tools'){
 if(!['tools','beyond'].includes(view))return Promise.reject(Error('Vista Pocket non valida'));
 let state=states.get(api);if(!state){state={};states.set(api,state);}if(state.opening)return view==='beyond'?state.opening.then(()=>openPocketTools(api,view)):state.opening;
 state.opening=(async()=>{if(!state.tab)try{const saved=await api.storage?.session?.get('pocketToolsTab');if(Number.isInteger(saved?.pocketToolsTab)&&saved.pocketToolsTab>0)state.tab=saved.pocketToolsTab;}catch{}const base=api.runtime.getURL('account.html'),url=base+(view==='beyond'?'#beyond':'');if(state.tab)try{const tab=await api.tabs.get(state.tab);if(tab.url===base||tab.url===base+'#beyond'){await api.tabs.update(state.tab,{active:true,...(view==='beyond'&&tab.url!==url?{url}:{})});return {ok:true};}}catch{}const tab=await api.tabs.create({url});state.tab=tab.id;try{await api.storage?.session?.set({pocketToolsTab:tab.id});}catch{}return {ok:true};})().finally(()=>state.opening=null);return state.opening;
}
