// Minimal self-signed RSA/SHA256 certificate generated with built-in Node crypto.
import {generateKeyPairSync,createPrivateKey,sign,randomBytes,X509Certificate} from 'node:crypto';
const buf=x=>Buffer.isBuffer(x)?x:Buffer.from(x),len=n=>n<128?Buffer.from([n]):n<256?Buffer.from([0x81,n]):Buffer.from([0x82,n>>8,n&255]);
const der=(tag,...parts)=>{const b=Buffer.concat(parts.map(buf));return Buffer.concat([Buffer.from([tag]),len(b.length),b]);};
const seq=(...p)=>der(0x30,...p),set=(...p)=>der(0x31,...p),oid=x=>der(6,Buffer.from(x,'hex')),str=x=>der(12,x),integer=x=>der(2,x),oct=x=>der(4,x);
const alg=seq(oid('2a864886f70d01010b'),der(5,Buffer.alloc(0)));
const pem=(label,b)=>`-----BEGIN ${label}-----\n${b.toString('base64').match(/.{1,64}/g).join('\n')}\n-----END ${label}-----\n`;
export function createBridgeCertificate(ip,now=new Date()){
 if(!/^\d{1,3}(\.\d{1,3}){3}$/.test(ip)||ip.split('.').some(x=>Number(x)>255))throw Error('Indirizzo IPv4 non valido');
 const keys=generateKeyPairSync('rsa',{modulusLength:2048,publicKeyEncoding:{type:'spki',format:'der'},privateKeyEncoding:{type:'pkcs8',format:'pem'}}),name=seq(set(seq(oid('550403'),str('PocketDD local bridge'))));
 const time=d=>{const t=d.toISOString().replace(/[-:T]/g,'').replace(/\.\d{3}Z$/,'Z');return d.getUTCFullYear()<2050?der(0x17,t.slice(2)):der(0x18,t)};
 const from=new Date(now.getTime()-86400000),until=new Date(now.getTime()+3650*86400000),serial=randomBytes(16);serial[0]&=0x7f;serial[0]|=1;
 const extensions=seq(seq(oid('551d13'),der(1,Buffer.from([255])),oct(seq(der(1,Buffer.from([255]))))),seq(oid('551d11'),oct(seq(der(0x87,Buffer.from(ip.split('.').map(Number)))))));
 const tbs=seq(der(0xa0,integer(Buffer.from([2]))),integer(serial),alg,name,seq(time(from),time(until)),name,keys.publicKey,der(0xa3,extensions));
 const cert=pem('CERTIFICATE',seq(tbs,alg,der(3,Buffer.concat([Buffer.from([0]),sign('sha256',tbs,createPrivateKey(keys.privateKey))]))));
 const parsed=new X509Certificate(cert);if(!parsed.verify(parsed.publicKey)||!parsed.checkIP(ip))throw Error('Certificato locale non verificabile');return {cert,key:keys.privateKey};
}
