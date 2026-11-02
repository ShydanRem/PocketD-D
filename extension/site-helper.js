// No credentials or device data cross this bridge. Only presence and opening a
// fixed extension page; main-world messages are treated as untrusted input.
(()=>{
 const u=new URL(location.href);if(window!==top||!(u.protocol==='https:'&&u.hostname==='shydanrem.github.io'&&u.pathname.startsWith('/PocketD-D/')||u.protocol==='http:'&&u.hostname==='127.0.0.1'&&(u.pathname==='/'||u.pathname.startsWith('/install/')||u.pathname.startsWith('/PocketD-D/'))))return;
 window.addEventListener('message',event=>{
  const m=event.data;if(event.source!==window||event.origin!==location.origin||!m||m.type!=='pocketdd-helper-request'||!/^\w{16,64}$/.test(m.id)||!['ping','open'].includes(m.action))return;
  const reply=value=>window.postMessage({type:'pocketdd-helper-response',id:m.id,...value},location.origin);
  try{if(m.action==='ping')reply({installed:true,version:chrome.runtime.getManifest().version});else chrome.runtime.sendMessage({type:'open-pocket-tools'},result=>{if(chrome.runtime.lastError)reply({opened:false});else reply({opened:result?.ok===true});});}catch{/* An old content script after extension reload cannot impersonate a new one. */}
 });
})();
