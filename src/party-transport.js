import {PARTY_PROTOCOL,createParty,validateParty,applyPartyCommand,acceptPartySnapshot} from './party.js';

export const PARTY_CHECKPOINT='pocketdd-party-v1';
const validId=value=>typeof value==='string'&&/^[a-f0-9]{24}$/.test(value);
// BroadcastChannel is deliberately replaceable by a future device transport.
export class PartyConnection{
  constructor({newId,onChange=()=>{},storage,channelFactory=name=>new BroadcastChannel(name),now=()=>Date.now()}){
    if(storage===undefined)try{storage=globalThis.sessionStorage}catch{storage=null}
    Object.assign(this,{newId,onChange,storage,channelFactory,now});
    this.session=null;this.pending=null;this.channel=null;this.status='idle';this.message='';this.checkpointSaved=true;
    this.timer=setInterval(()=>this.tick(),2000);
  }
  get isCoordinator(){return this.clientId===this.coordinatorId}
  get connected(){return Boolean(this.channel)&&this.status==='connected'}
  emit(){this.onChange(this)}
  checkpoint(){
    try{this.storage.setItem(PARTY_CHECKPOINT,JSON.stringify({clientId:this.clientId,sessionId:this.sessionId,coordinatorId:this.coordinatorId,profile:this.profile,session:this.session,pending:this.pending,seq:this.seq,joinAttempted:this.joinAttempted}));this.checkpointSaved=true}
    catch{this.checkpointSaved=false}
  }
  restore(){
    try{
      const raw=this.storage.getItem(PARTY_CHECKPOINT);if(!raw)return;
      const saved=JSON.parse(raw);
      if(!validId(saved.clientId)||!validId(saved.sessionId)||(saved.coordinatorId!==null&&!validId(saved.coordinatorId))||!Number.isSafeInteger(saved.seq)||saved.seq<0)throw new Error('Checkpoint non valido');
      const session=saved.session?validateParty(saved.session):null;
      if(session&&(session.id!==saved.sessionId||session.coordinatorId!==saved.coordinatorId))throw new Error('Checkpoint incoerente');
      if(saved.clientId===saved.coordinatorId&&!session)throw new Error('Stato del coordinatore mancante');
      if(saved.pending&&(saved.pending.clientId!==saved.clientId||saved.pending.sessionId!==saved.sessionId||saved.pending.seq!==saved.seq))throw new Error('Comando in attesa non valido');
      Object.assign(this,{clientId:saved.clientId,sessionId:saved.sessionId,coordinatorId:saved.coordinatorId,profile:saved.profile,pending:saved.pending,seq:saved.seq,joinAttempted:Boolean(saved.joinAttempted),session});this.resume();
    }catch{this.message='Ripresa del party non disponibile. Puoi entrare nuovamente con il codice.';this.emit()}
  }
  create(profile){
    if(this.sessionId)throw new Error('Esci dal party corrente prima di crearne un altro');
    this.clientId=this.newId();this.sessionId=this.newId();this.coordinatorId=this.clientId;this.seq=0;this.profile=profile;
    this.joinAttempted=false;this.session=createParty(this.sessionId,this.clientId);this.resume();this.send('join',profile);
  }
  join(code,profile){
    if(this.sessionId)throw new Error('Esci dal party corrente prima di entrare in un altro');
    if(!validId(code))throw new Error('Inserisci il codice di 24 caratteri del coordinatore');
    this.clientId=this.newId();this.sessionId=code;this.coordinatorId=null;this.session=null;this.seq=0;this.profile=profile;this.joinAttempted=false;this.resume();
  }
  open(){
    this.channel=this.channelFactory(`pocketdd-party-v1-${this.sessionId}`);
    this.channel.onmessage=event=>this.receive(event.data);
  }
  resume(){
    if(!this.sessionId||this.channel)return;
    try{this.open()}catch{this.status='offline';this.message='Collegamento non disponibile: usa la demo servita via HTTP nello stesso browser.';this.emit();return}
    this.status=this.isCoordinator?'connected':'connecting';this.lastSeen=this.now();this.message='';this.checkpoint();this.emit();
    if(this.isCoordinator){if(this.pending)this.process(this.pending);this.publish()}
    else this.post({type:'hello',clientId:this.clientId});
  }
  pause(){this.channel?.close();this.channel=null;this.status='offline';this.message='Collegamento sospeso: nessuna modifica viene inviata.';this.emit()}
  leave(){
    this.channel?.close();this.channel=null;this.session=null;this.sessionId=null;this.coordinatorId=null;this.pending=null;this.status='idle';this.message='';
    try{this.storage.removeItem(PARTY_CHECKPOINT)}catch{}
    this.emit();
  }
  dispose(){clearInterval(this.timer);this.channel?.close()}
  post(message){this.channel?.postMessage({protocol:PARTY_PROTOCOL,sessionId:this.sessionId,...message})}
  publish(to=null,ack=null){this.post({type:'snapshot',from:this.clientId,to,session:this.session,ack})}
  send(action,payload={}){
    if(!this.connected)throw new Error('Attendi il coordinatore o riconnetti il collegamento');
    if(this.pending)throw new Error('Attendi la conferma del comando precedente');
    if(action==='join')this.joinAttempted=true;
    this.seq=(this.session?.receipts[this.clientId]?.seq??0)+1;
    this.pending={protocol:PARTY_PROTOCOL,sessionId:this.sessionId,clientId:this.clientId,seq:this.seq,baseRevision:this.session.revision,action,payload};
    this.message='In attesa di conferma…';this.checkpoint();this.emit();
    if(this.isCoordinator)this.process(this.pending);else this.post({type:'command',command:this.pending});
  }
  process(command){
    let error=null;
    try{const result=applyPartyCommand(this.session,command);this.session=result.session;error=result.error}
    catch(e){error=e.message}
    // Persist before acknowledging. If storage is blocked, the UI explicitly
    // marks the coordinator as temporary; the in-memory authority still works.
    if(command.clientId===this.clientId){this.pending=null;this.message=error??'Modifica confermata'}
    this.checkpoint();this.publish(command.clientId,{seq:command.seq,error});this.publish();this.emit();
  }
  receive(message){
    if(!message||message.protocol!==PARTY_PROTOCOL||message.sessionId!==this.sessionId)return;
    if(this.isCoordinator){
      if(message.type==='hello'&&validId(message.clientId))this.publish(message.clientId);
      if(message.type==='command'&&message.command)this.process(message.command);
      return;
    }
    if(message.type!=='snapshot'||(message.to&&message.to!==this.clientId)||message.from!==message.session?.coordinatorId)return;
    try{
      const previous=this.session;
      const next=acceptPartySnapshot(previous,message.session,this.sessionId,this.coordinatorId);
      if(next===previous)return;
      const changed=!previous||JSON.stringify(previous)!==JSON.stringify(next)||this.status!=='connected';
      this.session=next;this.coordinatorId=next.coordinatorId;this.lastSeen=this.now();this.status='connected';
      let confirmed=false;
      if(this.pending){
        const receipt=next.receipts[this.clientId];
        if((receipt?.seq??0)>=this.pending.seq){this.message=receipt.error??'Modifica confermata';this.pending=null;confirmed=true}
        else if(message.ack?.seq===this.pending.seq){this.message=message.ack.error??'Comando non confermato';this.pending=null;confirmed=true}
      }
      this.checkpoint();if(changed||confirmed)this.emit();
      if(!next.members.some(m=>m.clientId===this.clientId)&&!this.pending&&!this.joinAttempted)this.send('join',this.profile);
    }catch{/* Malformed or foreign snapshots never change the current party. */}
  }
  tick(){
    if(!this.channel)return;
    if(this.isCoordinator){this.publish();return}
    if(this.now()-this.lastSeen>7000&&this.status!=='offline'){this.status='offline';this.message='Coordinatore non raggiungibile. Lo stato resta visibile; in attesa del suo ritorno.';this.emit()}
    this.post({type:'hello',clientId:this.clientId});
    if(this.pending)this.post({type:'command',command:this.pending});
  }
}
