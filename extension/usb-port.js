// Page-local port selection only. No identifiers in storage and no background
// scanning. Closing a transfer releases the OS port, retaining its permission.
let serialOwner=null,remembered=null,leased=false,firmware=null;
const listeners=new Set(),watched=new WeakSet();
export function usbPortState(){return {busy:leased,known:Boolean(remembered),firmware};}
function notify(){for(const fn of listeners)try{fn(usbPortState());}catch{/* A view must not strand the port lease. */}}
export function observeUsbPort(fn){listeners.add(fn);fn(usbPortState());return ()=>listeners.delete(fn);}
export function claimUsbPort(){if(leased)throw Error('Pocket occupato: termina l’operazione in corso.');leased=true;notify();let released=false;return ()=>{if(!released){released=true;leased=false;notify();}};}
export function forgetUsbPort(port){if(!port||port===remembered){remembered=null;serialOwner=null;firmware=null;notify();}}
export function rememberUsbPort(port,serial,version){if(port!==remembered)firmware=null;if(version)firmware=version;if(!watched.has(serial)){watched.add(serial);serial.addEventListener?.('disconnect',e=>{if(serial===serialOwner&&(e.target===remembered||e.port===remembered))forgetUsbPort(remembered);});}serialOwner=serial;remembered=port;notify();}
export function chooseUsbPort(serial,{chooseNew=false}={}){
 if(!serial?.requestPort)throw Error('Per collegare il Pocket usa Chrome o Edge su PC.');
 // requestPort is invoked in this stack, preserving the user's activation.
 return !chooseNew&&serial===serialOwner&&remembered?Promise.resolve(remembered):serial.requestPort();
}
