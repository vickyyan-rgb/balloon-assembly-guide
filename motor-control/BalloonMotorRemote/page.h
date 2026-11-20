#pragma once
static const char page[] PROGMEM = R"HTML(<!doctype html>
<html lang="zh-CN"><meta charset="utf-8"><meta name="viewport" content="width=device-width, initial-scale=1">
<title>气球腕带 · 电机遥控</title>
<style>
*{box-sizing:border-box}body{margin:0;background:#eaf2ef;color:#173934;font:17px system-ui,sans-serif}
main{max-width:530px;margin:5vh auto;padding:26px}header{display:flex;align-items:center;justify-content:space-between}
h1{font-size:30px;letter-spacing:-1px}#status{font-size:15px}section{background:#fff;border-radius:22px;padding:25px;margin:16px 0}
label{display:flex;align-items:baseline;justify-content:space-between;font-weight:650}output{font-size:23px;font-variant-numeric:tabular-nums}
input{width:100%;accent-color:#107968;margin:18px 0 4px;height:32px}.row{display:flex;gap:12px}
button{font:inherit;font-weight:650;cursor:pointer;border:0;border-radius:16px;padding:16px;flex:1;color:#123d34;background:#ffd28d}
button small{display:block;font-size:13px;margin-top:5px}#up{background:#a3dfc9}#stop{background:#24453e;color:white;width:100%;margin-top:12px}
button:disabled{opacity:.48}p{font-size:14px;line-height:1.6}
</style>
<main><header><h1>腕带电机遥控</h1><span id="status" role="status">连接中…</span></header>
<section><label for="speed">速度 · PWM <output id="sv">40%</output></label>
<input id="speed" type="range" min="1" max="100" value="40">
<label for="duration" style="margin-top:22px">每次转动（0–25 秒） <output id="tv">0.5 秒</output></label>
<input id="duration" type="range" min="0" max="25000" step="100" value="500"></section>
<p>100% 为全占空比。3 V 电机只在额定 3.0 V 电机电源下测试满速。</p><div class="row"><button id="down" disabled>↺ 降落<small>逆时针</small></button>
<button id="up" disabled>↻ 升起<small>顺时针</small></button></div>
<button id="stop">■ 立即停止</button><p id="message" role="status">按一次方向键，电机按设定时长运行后自动停止。</p></main>
<script>
const el=id=>document.getElementById(id);
let sending=false,online=false,dir=0,refreshId=0;
function draw(){el('down').disabled=el('up').disabled=sending||!online||dir!==0;
el('status').textContent=!online?'未连接':dir===1?'降落中':dir===-1?'升起中':'已连接 · 待机';}
el('speed').oninput=()=>el('sv').textContent=el('speed').value+'%';
el('duration').oninput=()=>el('tv').textContent=(el('duration').value/1000).toFixed(1)+' 秒';
async function state(){const id=++refreshId; const controller=new AbortController(); const timer=setTimeout(()=>controller.abort(),2200); try{const r=await fetch('/state',{cache:'no-store',signal:controller.signal});
if(!r.ok)throw Error();const value=await r.json();if(id!==refreshId)return;dir=value;online=true;}
catch{if(id!==refreshId)return;online=false;}finally{clearTimeout(timer);}draw();}
async function action(which){sending=true;draw();try{const stop=which==='stop';
const ms=el('duration').value; const r=await fetch(stop?'/stop':'/move',{method:'POST',body:stop?null:new URLSearchParams({dir:which,speed:el('speed').value,ms})});
el('message').textContent=r.ok?(stop?'已发出停止指令。':ms==='0'?'时长为 0 秒，电机保持停止。':'指令已接收，到时自动停止。'):'请先松开实体按钮，待电机停止后再试。';}
catch{el('message').textContent='连接中断，请检查腕带 Wi-Fi。';}
finally{await state();sending=false;draw();}}
for(const k of ['down','up','stop'])el(k).onclick=()=>action(k);
async function poll(){await state();setTimeout(poll,700);} poll();
</script></html>)HTML";

