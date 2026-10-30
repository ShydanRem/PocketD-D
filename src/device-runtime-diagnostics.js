// Exact public firmware schema. Arbitrary fields/text are rejected, never exported.
export function validateRuntimeDiagnostics(input){
  const fail=()=>{throw Error('Diagnosi firmware non valida o non supportata. Nessun dato della scheda inviato.');};
  const fields=(o,keys)=>{if(!o||Array.isArray(o)||typeof o!=='object'||Object.keys(o).sort().join(' ')!==keys.split(' ').sort().join(' '))fail();};
  const counts=(o,keys,max=0xffffffff)=>{for(const key of keys.split(' '))if(!Number.isSafeInteger(o[key])||o[key]<0||o[key]>max)fail();};
  const flags=(o,keys)=>{for(const key of keys.split(' '))if(typeof o[key]!=='boolean')fail();};
  const d=input;fields(d,'format version board_target firmware idf uptime_ms reset_reason formats memory stack io storage screen');
  if(d.format!=='pocketdd-diagnostics'||d.version!==1||d.board_target!=='ESP32-S3-Touch-AMOLED-1.75')fail();
  for(const key of ['firmware','idf'])if(typeof d[key]!=='string'||!/^[A-Za-z0-9._+\-]{1,48}$/.test(d[key]))fail();
  counts(d,'uptime_ms',Number.MAX_SAFE_INTEGER);counts(d,'reset_reason');
  fields(d.formats,'usb state history backup');counts(d.formats,'usb state history backup');
  if(d.formats.history!==2||d.formats.backup!==1||![[9,25],[10,30],[11,31],[12,32],[13,33],[14,33],[15,34],[16,35],[17,36],[18,37],[19,38]].some(([u,s])=>d.formats.usb===u&&d.formats.state===s))fail();
  fields(d.memory,'internal psram');fields(d.memory.internal,'free minimum_free largest_block');counts(d.memory.internal,'free minimum_free largest_block');
  fields(d.memory.psram,'total free minimum_free largest_block');counts(d.memory.psram,'total free minimum_free largest_block');
  fields(d.stack,'ui_min_bytes usb_min_bytes');counts(d.stack,'ui_min_bytes usb_min_bytes');
  fields(d.io,'lvgl_ticks lvgl_max_us flushes_submitted touch_samples touch_errors touch_contacts pwr_read_errors pwr_presses boot_presses pwr_available');
  counts(d.io,'lvgl_ticks lvgl_max_us flushes_submitted touch_samples touch_errors touch_contacts pwr_read_errors pwr_presses boot_presses');flags(d.io,'pwr_available');
  fields(d.storage,'state_writable history_available history_pending history_matches state_valid');flags(d.storage,'state_writable history_available history_pending history_matches state_valid');
  fields(d.screen,'dimmed failed busy');flags(d.screen,'dimmed failed busy');
  return structuredClone(d);
}
