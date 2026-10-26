// Keep only transport metadata and recognized diagnostic fields, never raw lines,
// packet bodies, character names, memory dumps or arbitrary firmware text.
export function createUsbDiagnostics(operation){
  const started=Date.now(),data={version:1,operation,phase:'apertura porta',outcome:'unconfirmed',usbVendorId:null,usbProductId:null,usbVersion:null,protocol:null,helloAttempts:0,logLines:0,logBytes:0,romBanners:0,resets:[],lastLogTag:null,fault:null,backtrace:[]};
  return {
    data,
    port(info){for(const k of ['usbVendorId','usbProductId'])if(Number.isInteger(info?.[k])&&info[k]>=0&&info[k]<=65535)data[k]=info[k];},
    line(raw){
      const text=raw.replace(/\x1b\[[0-9;]*m/g,'');
      if(/[^\x20-\x7e]/.test(text))return false;
      const idf=/^[IWE] \([0-9]+\) ([A-Za-z0-9_. -]{1,48}):/.exec(text);
      const startup=/^(?:ESP-ROM:esp32s3-[0-9]+|Build:|rst:0x[0-9a-fA-F]+|Saved PC:0x[0-9a-fA-F]+|SPIWP:0x[0-9a-fA-F]+|mode:(?:DIO|QIO|DOUT|QOUT),|load:0x[0-9a-fA-F]+|entry 0x[0-9a-fA-F]+)/.test(text);
      const panic=/^(?:Guru Meditation Error:|Backtrace:|abort\(\) was called at PC |assert failed:|Stack smashing protect failure!|Brownout detector was triggered|Core [01] register dump:|(?:PC|PS|A[0-9]{1,2}|SAR|EXCCAUSE|EXCVADDR|LBEG|LEND|LCOUNT)\s*: 0x|ELF file SHA256:|Rebooting\.\.\.)/.test(text);
      if(!startup&&!idf&&!panic)return false;
      data.logLines++;data.logBytes+=raw.length+1;
      if(text.startsWith('ESP-ROM:'))data.romBanners++;
      const reset=/^rst:(0x[0-9a-fA-F]+) \(([A-Z0-9_ ]{1,48})\)/.exec(text);
      if(reset&&data.resets.length<8)data.resets.push({code:reset[1],reason:reset[2]});
      if(idf){data.lastLogTag=idf[1];if(idf[1]==='task_wdt')data.fault='watchdog';}
      if(text.startsWith('Guru Meditation Error:'))data.fault=['LoadProhibited','StoreProhibited','IllegalInstruction','Stack canary watchpoint triggered','Interrupt wdt timeout'].find(v=>text.includes(v))??'panic';
      if(text.startsWith('abort()'))data.fault='abort';
      if(text.startsWith('assert failed:'))data.fault='assert';
      if(text.startsWith('Stack smashing'))data.fault='stack';
      if(text.startsWith('Brownout'))data.fault='brownout';
      if(text.startsWith('Backtrace:'))data.backtrace=(text.match(/0x[0-9a-fA-F]{4,8}/g)??[]).slice(0,16);
      if(data.logLines>512||data.logBytes>32768)throw new Error('Limite dei log USB raggiunto. Apri Dettagli collegamento USB: il numero di righe non dimostra da solo un riavvio continuo. Nessun salvataggio confermato.');
      return true;
    },
    snapshot(){return structuredClone({...data,elapsedMs:Math.max(0,Date.now()-started)});}
  };
}
export function formatUsbDiagnostics(d){
  const hex=v=>v==null?'non disponibile':'0x'+v.toString(16).padStart(4,'0');
  const operations={check:'verifica collegamento, nessun PG inviato',import:'invio personaggio',receive:'ricezione backup',restore:'ripristino backup'};
  const result=d.outcome==='completed'?(d.operation==='check'?'collegamento verificato':'operazione confermata'):'non confermato';
  return `Diagnosi USB v${d.version}\nOperazione: ${operations[d.operation]??'collegamento'}\nEsito: ${result}\nFase: ${d.phase}\nPorta: VID ${hex(d.usbVendorId)} / PID ${hex(d.usbProductId)}\nProtocollo PocketD-D: ${d.usbVersion==null?'nessuna risposta READY':(d.protocol==='backup'?'backup ':'USB')+d.usbVersion}\nSaluti inviati: ${d.helloAttempts}\nLog: ${d.logLines} righe / ${d.logBytes} byte\nAnnunci ROM osservati: ${d.romBanners} (possono essere accumulati)\nReset: ${d.resets.map(r=>r.code+' '+r.reason).join('; ')||'non osservato'}\nUltimo componente log: ${d.lastLogTag??'non disponibile'}\nErrore firmware: ${d.fault??'non riconosciuto'}\nBacktrace: ${d.backtrace.join(' ')||'non disponibile'}\nDurata: ${d.elapsedMs} ms\nNessun JSON del PG o log integrale incluso.`;
}
