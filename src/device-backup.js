export const DEVICE_BACKUP_MAX=110000;
const fail=()=>{throw new Error('Backup dispositivo non valido o non supportato')};
export function crc32(bytes){let c=0xffffffff;for(const b of bytes){c^=b;for(let i=0;i<8;i++)c=(c>>>1)^((c&1)?0xedb88320:0)}return (c^0xffffffff)>>>0}
function envelope(raw){
  if(typeof raw!=='string'||!raw.length||raw.length>DEVICE_BACKUP_MAX)fail();
  // The native envelope has four scalar fields, no escaped keys or nested JSON.
  let at=0;const result={},seen=new Set(),space=()=>{while(/[ \t\r\n]/.test(raw[at]??'!'))at++};
  const token=c=>{space();if(raw[at++]!==c)fail()};
  const string=()=>{token('"');const start=at;while(at<raw.length&&raw[at]!=='"'){const c=raw.charCodeAt(at++);if(c<32||c>126||c===92)fail()}if(at===raw.length)fail();return raw.slice(start,at++)};
  token('{');for(let i=0;i<4;i++){if(i)token(',');const key=string();if(seen.has(key)||!['format','version','state','history'].includes(key))fail();seen.add(key);token(':');if(key==='version'){token('1');result.version=1}else result[key]=string()}
  token('}');space();if(at!==raw.length||result.format!=='pocketdd-device-backup')fail();return result;
}
function record(text,magic,max){
  if(typeof text!=='string'||!text.length||text.length>4*Math.ceil(max/3)||text.length%4||!/^[A-Za-z0-9+/]*={0,2}$/.test(text))fail();
  let decoded;try{decoded=atob(text)}catch{fail()}
  if(btoa(decoded)!==text)fail();
  const bytes=Uint8Array.from(decoded,c=>c.charCodeAt(0)),view=new DataView(bytes.buffer);
  if(bytes.length<8||bytes.length>max||!(Array.isArray(magic)?magic:[magic]).includes(decoded.slice(0,4))||crc32(bytes.subarray(0,-4))!==view.getUint32(bytes.length-4,true))fail();
  return {bytes,view};
}
function text(bytes,start,capacity,empty=false){let end=start;while(end<start+capacity&&bytes[end]){if(bytes[end]<32||bytes[end]>126)fail();end++}if(end===start+capacity||(!empty&&end===start))fail();return String.fromCharCode(...bytes.subarray(start,end))}
export function inspectDeviceBackup(raw){
  const packet=envelope(raw),s=record(packet.state,['PDDP','PDDQ','PDDR','PDDS','PDDT','PDDU'],75185),h=record(packet.history,'PDH1',2481),b=s.bytes,v=s.view;
  const identity=b[3]===85,scenery=b[3]===84||identity,named=b[3]===82||b[3]===83||scenery,trailer=(named?63:0)+(identity?3:scenery?1:0);
  const scene=scenery?b[b.length-(identity?7:5)]:0,lineage=identity?b[b.length-6]:0,body=identity?b[b.length-5]:0;if(scene>2||lineage>11||body>1)fail();
  if(b.length<8879+trailer||b[5123]>120||b.length!==8879+trailer+552*b[5123])fail();
  // PDD26 packs the local turn subject into the existing timer-selector bytes.
  const turnSubjects=[0,0,0],packed=b[3]===81||named;
  for(let i=0;i<50;i++){const value=b[8375+552*b[5123]+i],tick=packed?value&7:value,owner=packed?value>>>3:0;if(tick>4||owner>3||(owner&&tick!==1&&tick!==2))fail();if(owner)turnSubjects[owner-1]++}
  const offset=552*b[5123],name=text(b,8,49),baseHp=v.getUint16(59,true),baseArmor=b[64];let maxHp=baseHp,armor=baseArmor;
  const effects=b[7913+offset];if(effects>8)fail();
  for(let i=0;i<effects;i++){const at=7914+offset+4*i,target=b[at],delta=v.getUint16(at+1,true)-999,active=b[at+3];if(target>1||active>1||Math.abs(delta)>(target?999:20))fail();if(active){if(target)maxHp+=delta;else armor+=delta}}
  const hp=v.getUint16(57,true),tempHp=v.getUint16(61,true),level=b[63];
  if(level<1||level>20||baseHp<1||baseHp>999||maxHp<1||maxHp>999||hp>maxHp||tempHp>999||baseArmor<1||baseArmor>40||armor<1||armor>40||b[435]>60||b[71]>8)fail();
  const epoch=v.getUint32(b.length-8-trailer,true),walletAt=b.length-29-trailer,available=b[walletAt];if(available>1)fail();
  const wallet={available:Boolean(available)};['cp','sp','ep','gp','pp'].forEach((key,i)=>{const value=v.getUint32(walletAt+1+4*i,true);if(value>999999999||(!available&&value))fail();wallet[key]=value});
  const turnNames=named?[0,1,2].map(i=>{const at=b.length-67-(identity?3:scenery?1:0)+i*21,name=text(b,at,21,true);if(name!==name.trim()||b.subarray(at+name.length,at+21).some(x=>x))fail();return name}):['','',''];
  const payments=[0,1,2].map(i=>{const raw=b[8333+offset+i*10],payment=(b[3]===83||scenery)?raw>>>1:0,stage=(b[3]===83||scenery)?raw&1:raw;if(stage>1||payment>1)fail();return payment});
  const keys=Array.from(b.subarray(5161,5164)),brightness=b[7911+offset],dim=b[7912+offset];if(keys.some(k=>k>(scenery?14:13))||brightness<10||brightness>100||dim>3)fail();
  const hb=h.bytes,hv=h.view;if(hb.length<21)fail();const count=hb[16],sequence=hv.getUint32(12,true);
  if(count>20||hb.length!==21+123*count||hv.getUint32(8,true)!==epoch||sequence<count||(!count&&sequence))fail();
  const rolls=[];
  for(let i=0;i<count;i++){
    const at=17+123*i,number=hv.getUint32(at,true),kind=hb[at+4],mode=hb[at+5]-1,n=hb[at+6],sides=hb[at+7],floor=hb[at+8],bonus=hv.getUint16(at+9,true)-200,total=hv.getUint32(at+11,true)-200,dice=Array.from(hb.subarray(at+105,at+105+n));
    if(number!==sequence-i||kind>10||mode< -1||mode>1||n<1||n>18||![4,6,8,10,12,20].includes(sides)||floor>1||bonus< -200||bonus>200||dice.some(d=>d<1||d>sides)||hb.subarray(at+105+n,at+123).some(d=>d))fail();
    if(mode&&(sides!==20||n!==2||floor))fail();
    let expected=(mode?mode>0?Math.max(...dice):Math.min(...dice):dice.reduce((a,c)=>a+c,0))+bonus;if(floor)expected=Math.max(0,expected);if(total!==expected)fail();
    rolls.push({number,kind,mode,sides,dice,bonus,total,label:text(hb,at+15,41),note:text(hb,at+56,49,true)});
  }
  // This is an integrity check and a bounded preview. The device runs the full
  // native validators for all remaining rules before offering Restore.
  return {nativeValidation:false,packet,scene,lineage,body,name,level,hp,maxHp,baseHp,tempHp,armor,baseArmor,effects,items:b[435],powers:b[5123],resources:b[71],wallet,keys,brightness,dim,turnSubjects,turnNames,payments,rolls};
}
