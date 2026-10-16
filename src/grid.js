export const GRID_SIZE = 9;
const cellValid=cell=>Number.isInteger(cell)&&cell>=0&&cell<GRID_SIZE*GRID_SIZE;
export const emptyGrid=()=>({walls:[],positions:{},spent:0});
export function validateGrid(input,participants,activeId,round){
  if(!input||!Array.isArray(input.walls)||input.walls.length>81||!input.walls.every(cellValid)||new Set(input.walls).size!==input.walls.length||!input.positions||typeof input.positions!=='object'||Array.isArray(input.positions))throw new Error('Mappa non valida');
  const entries=Object.entries(input.positions);const ids=new Set(participants.map(item=>item.id));
  if(entries.some(([id,cell])=>!ids.has(id)||!cellValid(cell)||input.walls.includes(cell))||new Set(entries.map(([,cell])=>cell)).size!==entries.length)throw new Error('Posizioni non valide');
  const active=participants.find(item=>item.id===activeId);
  if(!Number.isInteger(input.spent)||input.spent<0||input.spent>(active?.speed??0)||(round===0&&input.spent!==0)||(input.spent>0&&!entries.some(([id])=>id===activeId)))throw new Error('Movimento non valido');
  return {walls:[...input.walls],positions:Object.fromEntries(entries),spent:input.spent};
}
function planning(state){if(state.encounter.round>0)throw new Error('Termina il combattimento prima di modificare la mappa')}
export function placeToken(state,id,cell){
  planning(state);
  if(!state.encounter.participants.some(item=>item.id===id)||!cellValid(cell))throw new Error('Posizione non valida');
  const grid=state.encounter.grid;
  if(grid.walls.includes(cell)||Object.entries(grid.positions).some(([other,position])=>other!==id&&position===cell))throw new Error('Casella occupata o bloccata');
  return {...state,encounter:{...state.encounter,strike:null,grid:{...grid,positions:{...grid.positions,[id]:cell}}}};
}
export function unplaceToken(state,id){
  planning(state);const grid=state.encounter.grid;
  return {...state,encounter:{...state.encounter,strike:null,grid:{...grid,positions:Object.fromEntries(Object.entries(grid.positions).filter(([other])=>other!==id))}}};
}
export function toggleWall(state,cell){
  planning(state);const grid=state.encounter.grid;
  if(!cellValid(cell)||Object.values(grid.positions).includes(cell))throw new Error('Casella occupata o non valida');
  const walls=grid.walls.includes(cell)?grid.walls.filter(other=>other!==cell):[...grid.walls,cell];
  return {...state,encounter:{...state.encounter,strike:null,grid:{...grid,walls}}};
}
export function reachableCells(state){
  const encounter=state.encounter,grid=encounter.grid;
  const actor=encounter.participants.find(item=>item.id===encounter.activeId);
  if(!encounter.round||!actor||(actor.id==='hero'?state.hp:actor.hp)===0)return new Map();
  const start=grid.positions[actor.id];
  if(!cellValid(start))return new Map();
  const blocked=new Set([...grid.walls,...Object.entries(grid.positions).filter(([id])=>id!==actor.id).map(([,cell])=>cell)]);
  const distances=new Map([[start,0]]),queue=[start],budget=actor.speed-grid.spent;
  for(let cursor=0;cursor<queue.length;cursor++){
    const cell=queue[cursor],distance=distances.get(cell);if(distance>=budget)continue;
    const x=cell%GRID_SIZE,y=Math.floor(cell/GRID_SIZE);
    for(let dy=-1;dy<=1;dy++)for(let dx=-1;dx<=1;dx++){
      if((dx===0&&dy===0)||x+dx<0||x+dx>=GRID_SIZE||y+dy<0||y+dy>=GRID_SIZE)continue;
      const next=(y+dy)*GRID_SIZE+x+dx;
      if(blocked.has(next)||distances.has(next))continue;
      // Regola del prototipo: niente tagli diagonali tra muri o pedine.
      if(dx&&dy&&(blocked.has(y*GRID_SIZE+x+dx)||blocked.has((y+dy)*GRID_SIZE+x)))continue;
      distances.set(next,distance+1);queue.push(next);
    }
  }
  return distances;
}
export function moveActive(state,cell){
  const encounter=state.encounter,grid=encounter.grid;
  if(!cellValid(cell)||!encounter.round||grid.positions[encounter.activeId]===cell)throw new Error('Scegli una destinazione valida');
  const cost=reachableCells(state).get(cell);
  if(cost===undefined)throw new Error('Casella non raggiungibile con il movimento rimasto');
  return {...state,encounter:{...encounter,strike:null,grid:{...grid,positions:{...grid.positions,[encounter.activeId]:cell},spent:grid.spent+cost}}};
}

// Vista ritagliata: le coordinate del gioco restano sempre sulla mappa 9 × 9.
export function viewportCells(size=9,origin={x:0,y:0}){
  if(![5,9].includes(size)||!Number.isInteger(origin.x)||!Number.isInteger(origin.y))throw new Error('Vista non valida');
  const x=Math.max(0,Math.min(GRID_SIZE-size,origin.x)),y=Math.max(0,Math.min(GRID_SIZE-size,origin.y));
  return Array.from({length:size*size},(_,index)=>(y+Math.floor(index/size))*GRID_SIZE+x+index%size);
}
export function centerViewport(cell,size=9){
  if(!cellValid(cell)||![5,9].includes(size))throw new Error('Centro vista non valido');
  const clamp=n=>Math.max(0,Math.min(GRID_SIZE-size,n));
  return {x:clamp(cell%GRID_SIZE-Math.floor(size/2)),y:clamp(Math.floor(cell/GRID_SIZE)-Math.floor(size/2))};
}
export function panViewport(origin,dx,dy,size=9){
  if(![5,9].includes(size)||![origin.x,origin.y,dx,dy].every(Number.isInteger))throw new Error('Spostamento vista non valido');
  const clamp=n=>Math.max(0,Math.min(GRID_SIZE-size,n));
  return {x:clamp(origin.x+dx),y:clamp(origin.y+dy)};
}
// Celle toccate dal segmento tra i centri, includendo entrambi i lati di un angolo.
export function sightCells(start,end){
  if(!cellValid(start)||!cellValid(end))throw new Error('Linea di vista non valida');
  let x=start%GRID_SIZE,y=Math.floor(start/GRID_SIZE);
  const endX=end%GRID_SIZE,endY=Math.floor(end/GRID_SIZE);
  const nx=Math.abs(endX-x),ny=Math.abs(endY-y),sx=Math.sign(endX-x),sy=Math.sign(endY-y);
  let ix=0,iy=0;const cells=new Set([start]);
  while(ix<nx||iy<ny){
    const decision=(1+2*ix)*ny-(1+2*iy)*nx;
    if(decision===0){cells.add(y*GRID_SIZE+x+sx);cells.add((y+sy)*GRID_SIZE+x);x+=sx;y+=sy;ix++;iy++}
    else if(decision<0){x+=sx;ix++}
    else {y+=sy;iy++}
    cells.add(y*GRID_SIZE+x);
  }
  return [...cells];
}
export function targetGeometry(state,targetId,reach=state.weaponReach??1){
  if(!Number.isInteger(reach)||reach<1||reach>9)throw new Error('Portata non valida');
  if(!state.encounter.participants.some(item=>item.id===targetId))throw new Error('Bersaglio non trovato');
  const positions=state.encounter.grid.positions,start=positions.hero,end=positions[targetId];
  if(!state.encounter.participants.some(item=>item.id==='hero')||!cellValid(start)||!cellValid(end))return {known:false,distance:null,inRange:null,visible:null,cells:[]};
  const distance=Math.max(Math.abs(start%GRID_SIZE-end%GRID_SIZE),Math.abs(Math.floor(start/GRID_SIZE)-Math.floor(end/GRID_SIZE)));
  const cells=sightCells(start,end);
  return {known:true,distance,inRange:distance<=reach,visible:!cells.some(cell=>state.encounter.grid.walls.includes(cell)),cells};
}
