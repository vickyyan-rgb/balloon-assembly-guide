'use strict';
const $=s=>document.querySelector(s);
const resource=path=>window.BUNDLED_RESOURCES?.[path]||path;
const downloadAttrs=path=>'href="'+esc(resource(path))+'" download="'+esc(path.split('/').pop())+'"';
const esc=t=>String(t).replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));
const all=chapters.flatMap((c,ci)=>c.steps.map((step,si)=>({...step,ci,si})));
const key='balloon-workbench-v1';
let saved={};
try{saved=JSON.parse(localStorage.getItem(key)||'{}')||{}}catch{}
const done=new Set((Array.isArray(saved.done)?saved.done:[]).filter(id=>all.some(s=>s.id===id)));
const kit=new Set(Array.isArray(saved.kit)?saved.kit:[]);
const connected=new Set(Array.isArray(saved.connected)?saved.connected:[]);
let current=Math.max(0,all.findIndex(s=>s.id===saved.step)),large=Boolean(saved.large),side=0;
function persist(){try{localStorage.setItem(key,JSON.stringify({done:[...done],kit:[...kit],connected:[...connected],step:all[current].id,large}))}catch{}}
const icon=(d)=>'<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.7" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">'+d+'</svg>';
const dlIcon=icon('<path d="M12 3v12m-5-5 5 5 5-5M4 16v4h16v-4"/>'),zoomIcon=icon('<circle cx="10" cy="10" r="6"/><path d="m14.5 14.5 5 5m-12-10h5m-2.5-2.5v5"/>');
const figures={
 kit:['assets/kit.webp','把两只盒子的零件分开','左图是一体腕盒与电机座，右图是紧凑声舱与抽屉。点开放大，可以对照每一层找零件。'],
 overview:['assets/overview.webp','腕带与吊舱','绿色腕盒收放绳子，紫色吊舱出声，独立灯环向气球发光。图中的绳长和气球大小为示意。'],
 wrist:['assets/wristbox.webp','腕端零件的位置','从侧口查看电池、主板和电机所在的三层。接线时按端子名称逐一核对。'],
 motor:['assets/motor.webp','N20 电机与卷筒','电机横放，卷筒对齐导眼，电线从静止的一侧引出。'],
 terminals:['assets/terminals.svg','正负分线端子','每个方框是一块五孔端子。上图表示分线的对应关系，实物以标记为准。'],
 button:['assets/button.webp','腕盒上的浅型按钮','大孔装按钮，小孔走气球绳。按钮的两个开关触点接 GPIO32 与 GND。'],
 wristbox:['assets/wristbox.webp','腕盒分层结构','从侧口装入零件。底仓放电池，中层放电路板，上层放电机和正负端子。'],
 strap:['assets/strap.webp','齐平底穿结构','腕带从底面进入，再从底面引出。底板只在左右两条侧肩上胶。'],
 balloon:['assets/balloonbox.webp','气球端的三个层次','电池在上，XIAO 与电容在中层，功放与喇叭在底层。灯环独立悬挂。'],
 ring:['assets/ring.webp','16 颗灯一起亮','认准 DI 或 DIN。数据线接入这一端，DO 是给下一只灯环的输出。'],
 audio:['assets/audio.svg','声音从主板送到喇叭','三条音频信号线连到功放。喇叭两端各接功放一个输出端。'],
 balloonbox:['assets/balloonbox.webp','紧凑声舱与出声口','由软兜承重。扬声器对着底面的三条声孔，兜带走两侧。']
};
function tag(el,on){if(on)el.setAttribute('aria-current','page');else el.removeAttribute('aria-current')}
function size(){document.body.classList.toggle('large',large);$('#zoomText').setAttribute('aria-pressed',large)}
$('#zoomText').onclick=()=>{large=!large;size();persist()};
function count(ci){return chapters[ci].steps.filter(s=>done.has(s.id)).length}
function showChapters(){
 $('#chapters').innerHTML=chapters.map((c,i)=>{const n=count(i),a=all[current].ci===i;
 return '<button class="chapter '+(n===c.steps.length?'complete':'')+'" data-chapter="'+i+'" '+(a?'aria-current="step"':'')+'><span class="num">'+(n===c.steps.length?'✓':String(i+1).padStart(2,'0'))+'</span><span><b>'+esc(c.name)+'</b><small>'+n+' / '+c.steps.length+' 步</small></span></button>'}).join('');
 $('#progressLabel').textContent='已完成 '+done.size+' / '+all.length+' 步';
 $('#progressBar').max=all.length;$('#progressBar').value=done.size;$('#progressBar').setAttribute('aria-label','安装进度');
 $('#start').textContent=done.size||current?'继续装配':'开始第一步';
}
function wireRows(wires,id){
 return wires.map(([a,b],i)=>{const color=/GND|负极|负端|BAT−|地线/.test(a+b)?'#495b69':/正极|正端|VIN|BAT\+|电源 \+/.test(a+b)?'#bf5846':/DIN|D3|DI |330/.test(a+b)?'#26876f':'#5c74bc';
 const mark=id+'-'+i,inside='<span>'+esc(a)+'</span><i aria-hidden="true" class="trace" style="--wire:'+color+'"></i><span>'+esc(b)+'</span>';
 return id?'<label class="wire-row tick-row"><input type="checkbox" aria-label="'+esc(a+' 接到 '+b)+'" data-wire="'+esc(mark)+'" '+(connected.has(mark)?'checked':'')+'>'+inside+'</label>':'<div class="wire-row">'+inside+'</div>'}).join('')
}
function setStep(ix,focus=true){
 current=Math.max(0,Math.min(ix,all.length-1));persist();
 const nextHash='#step-'+all[current].id;
 if(location.hash!==nextHash)location.hash=nextHash;else{renderRoute();if(focus)scrollStep()}
}
function chapterEntry(ci){const c=chapters[ci],step=c.steps.find(s=>!done.has(s.id))||c.steps[0];setStep(all.findIndex(s=>s.id===step.id))}
function scrollStep(){const card=$('#stepCard');card.focus({preventScroll:true});card.scrollIntoView({block:'start',behavior:'instant'})}
function stepHTML(){
 const s=all[current],c=chapters[s.ci],f=figures[s.figure]||figures.overview;
 const list=c.steps.map(st=>'<li><button data-goto="'+esc(st.id)+'" '+(st.id===s.id?'aria-current="step"':'')+'>'+esc(st.title)+(done.has(st.id)?' ✓':'')+'</button></li>').join('');
 const dl=s.download?'<a class="download-inline" '+downloadAttrs(s.download[0])+'>'+dlIcon+esc(s.download[1])+'</a>':'';
 $('#stepCard').innerHTML='<div class="step-head"><div class="meta"><span>'+esc(c.name)+' &nbsp; / &nbsp; 第 '+(s.si+1)+' 步，共 '+c.steps.length+' 步</span><button class="jump" id="openSteps" aria-expanded="false">本章步骤</button></div><div id="jumpBox" class="steplist" hidden><ol>'+list+'</ol></div><h2>'+esc(s.title)+'</h2><p class="action">'+esc(s.action)+'</p></div>'+
 '<div class="step-body"><figure class="illustration"><button class="art-btn" data-zoom="'+esc(s.figure)+'" aria-label="放大：'+esc(f[1])+'"><img src="'+resource(f[0])+'" alt="'+esc(f[1])+'"><span class="zoommark">'+zoomIcon+'放大</span></button><figcaption class="figure-caption"><strong>'+esc(f[1])+'</strong>'+esc(f[2])+'</figcaption></figure>'+
 '<ol class="steps-list">'+s.items.map(i=>'<li><span>'+esc(i)+'</span></li>').join('')+'</ol>'+
 (s.wires?'<section class="wirepanel"><h3>接好一行，勾一行</h3>'+wireRows(s.wires,s.id)+'</section>':'')+dl+
 '<div class="check"><b>做完看一眼</b><span>'+esc(s.check)+'</span></div><details class="help"><summary>我卡在这一步了</summary><p>'+esc(s.help)+'</p></details></div>'+
 '<div class="step-foot"><button id="prev" class="secondary-btn" '+(current===0?'disabled':'')+'>上一步</button><button id="next" class="primary">'+(current===all.length-1?'完成并查看记录':done.has(s.id)?'下一步':'确认完成，下一步')+'</button></div>';
 $('#openSteps').onclick=e=>{const box=$('#jumpBox');box.hidden=!box.hidden;e.currentTarget.setAttribute('aria-expanded',!box.hidden)};
 $('#prev').onclick=()=>setStep(current-1);
 $('#next').onclick=()=>{done.add(s.id);persist();if(current<all.length-1)setStep(current+1);else location.hash='#done'};
 showChapters()
}
function showWires(){
 $('#wireControls').innerHTML=['腕带电机','气球声光'].map((l,i)=>'<button data-side="'+i+'" aria-pressed="'+(i===side)+'">'+l+'</button>').join('');
 const steps=chapters[side?2:1].steps.filter(s=>s.wires);
 $('#wireTables').innerHTML=steps.map(s=>'<section class="table-card"><h3>'+esc(s.title)+'</h3><div>'+wireRows(s.wires)+'</div><button class="text-btn" data-goto="'+esc(s.id)+'">查看这一步的做法</button></section>').join('');
}
function showKit(){
 const refs=[...window.WRIST_SOURCES,...window.BALLOON_SOURCES];
 $('#kitContents').innerHTML='<div class="kit-grid">'+inventory.map(([name,...items],ci)=>'<section class="kit-card"><h3>'+name+'</h3>'+items.map((t,j)=>'<label><input type="checkbox" data-kit="'+ci+'-'+j+'" '+(kit.has(ci+'-'+j)?'checked':'')+'><span>'+t+'</span></label>').join('')+'</section>').join('')+'</div><h2 class="resource-title">程序与打印文件</h2><div class="dl-grid">'+downloads.map(([label,desc,url])=>'<a class="dl-card" '+downloadAttrs(url)+'><span><b>'+label+'</b><small>'+desc+'</small></span>'+dlIcon+'</a>').join('')+'</div><h2 class="resource-title">板卡与元件资料</h2><div class="sources">'+refs.map(([label,url])=>'<a href="'+esc(url)+'" target="_blank" rel="noreferrer">'+esc(label)+'</a>').join('')+'</div>'
}
function showDone(){
 const complete=done.size===all.length;
 $('#finishTitle').textContent=complete?'桌面联测完成':'你的装配记录';
 $('#finishMsg').textContent=complete?'两盒的接线、装盒和桌面联测都已勾选。可以重新打开任一步，核对线头或紧固件。':'已确认 '+done.size+' / '+all.length+' 步。点开下方章节，可接着完成未勾选的步骤。';
 $('#finishList').innerHTML=chapters.map((c,i)=>'<div class="finish-card"><span><b>'+c.name+'</b>　'+count(i)+' / '+c.steps.length+'</span><button data-chapter="'+i+'" class="text-btn">打开章节</button></div>').join('')
}
function renderRoute(initial=false){
 const h=location.hash,st=h.startsWith('#step-'),name=h==='#wires'?'wires':h==='#kit'?'kit':h==='#done'?'done':'home';
 ['home','wires','kit','done'].forEach(n=>$('#'+n+'view').hidden=n!==name);
 tag($('#nav-guide'),name==='home');tag($('#nav-wires'),name==='wires');tag($('#nav-kit'),name==='kit');
 if(name==='home'){
  if(st){const i=all.findIndex(s=>s.id===h.slice(6));if(i>=0)current=i;persist()}
  stepHTML();
  if(st||h==='#guide')requestAnimationFrame(()=>st?scrollStep():$('#guide').scrollIntoView({block:'start',behavior:'instant'}));
  else if(!initial)window.scrollTo({top:0,behavior:'instant'})
 }else{
  ({wires:showWires,kit:showKit,done:showDone})[name]();window.scrollTo({top:0,behavior:'instant'});$('#'+name+'view').focus({preventScroll:true})
 }
}
document.addEventListener('click',e=>{
 const button=e.target.closest('[data-chapter],[data-goto],[data-side],[data-zoom]');if(!button)return;
 if(button.hasAttribute('data-chapter'))chapterEntry(Number(button.dataset.chapter));
 if(button.hasAttribute('data-goto'))setStep(all.findIndex(s=>s.id===button.dataset.goto));
 if(button.hasAttribute('data-side')){side=Number(button.dataset.side);showWires()}
 if(button.hasAttribute('data-zoom')){
  const f=figures[button.dataset.zoom]||figures.overview;$('#dialogImg').src=resource(f[0]);$('#dialogImg').alt=f[1];$('#dialogTitle').textContent=f[1];$('#lightbox').showModal()
 }
});
document.addEventListener('change',e=>{for(const [name,set] of [['kit',kit],['wire',connected]])if(e.target.hasAttribute('data-'+name)){const id=e.target.dataset[name];e.target.checked?set.add(id):set.delete(id);persist()}});
$('#closeDialog').onclick=()=>$('#lightbox').close();
$('#lightbox').onclick=e=>{if(e.target===$('#lightbox'))$('#lightbox').close()};
$('#start').onclick=()=>setStep(current);
window.addEventListener('hashchange',()=>renderRoute());
size();renderRoute(true);
