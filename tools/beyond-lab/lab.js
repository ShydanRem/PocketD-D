let report=null;
const $=selector=>document.querySelector(selector);
const list=(selector,values)=>{const root=$(selector);root.replaceChildren();for(const value of values){const li=document.createElement('li');li.textContent=value;root.append(li)}};
$('#probe').addEventListener('submit',async event=>{
  event.preventDefault();report=null;$('#result').hidden=true;
  const button=$('#probe button');button.disabled=true;$('#status').textContent='Lettura in corso…';
  try{
    const response=await fetch('/api/probe',{method:'POST',headers:{'Content-Type':'application/json'},credentials:'omit',body:JSON.stringify({character:$('#character').value})});
    const result=await response.json();
    if(!response.ok||!result.ok)throw new Error(`${result.error?.code??'ERROR'} · ${result.error?.message??'Prova non riuscita'}`);
    report=result.report;
    $('#name').textContent=report.character.name;
    $('#metadata').textContent=`Livello ${report.character.level} · ID ${report.character.id} · HTTP ${report.transport.status} · ${report.transport.bytes} byte · ${report.transport.elapsedMs} ms`;
    $('#evidence').textContent=report.testFixture?'RISPOSTA SIMULATA DI TEST: non dimostra accesso a Beyond.':'Risposta ricevuta dal connettore HTTP. La corrispondenza con la scheda va verificata.';
    $('#abilities').replaceChildren();
    for(const [key,value] of Object.entries(report.abilities)){const dt=document.createElement('dt'),dd=document.createElement('dd');dt.textContent=key;dd.textContent=String(value.preview);$('#abilities').append(dt,dd)}
    $('#hp').textContent=report.hp?`${report.hp.previewCurrent}/${report.hp.previewMax} PF · temporanei sorgente: ${report.hp.temporary}`:'PF non determinabili con questa prova.';
    list('#inventory',report.inventory.items.map(item=>`${item.name} ×${item.quantity??'?'}${item.equipped?' · equipaggiato':''}`));
    list('#warnings',report.warnings);$('#result').hidden=false;
    $('#status').textContent='Lettura completata. Anteprima da verificare; nessuna scheda modificata.';
  }catch(error){$('#status').textContent=error.message}
  finally{button.disabled=false}
});
$('#download').addEventListener('click',()=>{
  if(!report)return;
  const url=URL.createObjectURL(new Blob([JSON.stringify(report,null,2)],{type:'application/json'}));
  const link=document.createElement('a');link.href=url;link.download='PocketDD-prova-Beyond.json';link.click();setTimeout(()=>URL.revokeObjectURL(url),1000);
});
