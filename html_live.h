#pragma once

static const char PAGE_LIVE[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Live · Tintenklecks</title>
<style>
:root{--bg:#14110f;--panel:#241e18;--line:#4a3f35;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888;--mat:#f2efe8;--frame:9.5%}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt);min-height:100vh}
body.busy,body.busy *{cursor:wait !important}
body.busy button{pointer-events:none;opacity:.55}
.busy-ov{position:fixed;inset:0;background:rgba(10,8,6,.55);z-index:200;display:none;align-items:center;justify-content:center;flex-direction:column;gap:.75rem}
.busy-ov.on{display:flex}
.hour{width:44px;height:44px;border:3px solid var(--acc);border-top-color:transparent;border-radius:50%;animation:spin .75s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
.top{display:flex;justify-content:space-between;align-items:center;padding:.85rem 1.1rem;border-bottom:1px solid var(--line);gap:.75rem;flex-wrap:wrap}
.brand a{color:var(--txt);text-decoration:none;font-weight:700}.nav a{color:var(--acc);margin-left:.75rem;font-size:.85rem;text-decoration:none}
.bat{font-size:.8rem;color:var(--dim)}
.head-right{display:flex;align-items:center;gap:.5rem}
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;font-weight:700;flex:none;border-radius:6px;border:1px solid var(--line);background:#1a1612;color:var(--dim);cursor:pointer;font:inherit}
main{max-width:440px;margin:0 auto;padding:1.25rem 1rem 2rem}
h1{font-size:1.2rem;margin:0 0 1rem}
.stage{display:flex;justify-content:center;margin:0 0 1.1rem}
.device{
  width:min(78vw,300px);
  aspect-ratio:154/214;
  position:relative;
  box-shadow:0 18px 40px rgba(0,0,0,.45);
  background:#5a3518;
  overflow:hidden;
}
.rail{
  position:absolute;z-index:1;pointer-events:none;
  background-color:#7a4a24;
  background-image:
    repeating-linear-gradient(0deg,
      rgba(255,230,180,.14) 0 1px,
      transparent 1px 3px,
      rgba(40,18,6,.18) 3px 4px,
      transparent 4px 9px),
    repeating-linear-gradient(0deg,
      transparent 0 11px,
      rgba(120,70,30,.22) 11px 12px,
      transparent 12px 28px),
    linear-gradient(90deg,#b07a42 0%,#8b5a2b 42%,#6e4320 78%,#4e2f14 100%);
}
.rail.top,.rail.bottom{left:0;right:0;height:var(--frame)}
.rail.left,.rail.right{top:0;bottom:0;width:var(--frame)}
.rail.top{top:0;clip-path:polygon(0 0,100% 0,calc(100% - var(--frame)) 100%,var(--frame) 100%)}
.rail.bottom{
  bottom:0;
  clip-path:polygon(var(--frame) 0,calc(100% - var(--frame)) 0,100% 100%,0 100%);
  background-image:
    repeating-linear-gradient(0deg,
      rgba(255,230,180,.14) 0 1px,
      transparent 1px 3px,
      rgba(40,18,6,.18) 3px 4px,
      transparent 4px 9px),
    repeating-linear-gradient(0deg,
      transparent 0 11px,
      rgba(120,70,30,.22) 11px 12px,
      transparent 12px 28px),
    linear-gradient(90deg,#4e2f14 0%,#6e4320 22%,#8b5a2b 58%,#b07a42 100%);
}
.rail.left{
  left:0;
  background-image:
    repeating-linear-gradient(90deg,
      rgba(255,230,180,.14) 0 1px,
      transparent 1px 3px,
      rgba(40,18,6,.18) 3px 4px,
      transparent 4px 9px),
    repeating-linear-gradient(90deg,
      transparent 0 11px,
      rgba(120,70,30,.22) 11px 12px,
      transparent 12px 28px),
    linear-gradient(180deg,#b07a42 0%,#8b5a2b 42%,#6e4320 78%,#4e2f14 100%);
  clip-path:polygon(0 0,100% var(--frame),100% calc(100% - var(--frame)),0 100%);
}
.rail.right{
  right:0;
  background-image:
    repeating-linear-gradient(90deg,
      rgba(255,230,180,.14) 0 1px,
      transparent 1px 3px,
      rgba(40,18,6,.18) 3px 4px,
      transparent 4px 9px),
    repeating-linear-gradient(90deg,
      transparent 0 11px,
      rgba(120,70,30,.22) 11px 12px,
      transparent 12px 28px),
    linear-gradient(180deg,#4e2f14 0%,#6e4320 22%,#8b5a2b 58%,#b07a42 100%);
  clip-path:polygon(0 var(--frame),100% 0,100% 100%,0 calc(100% - var(--frame)));
}
.mat{
  position:absolute;inset:var(--frame);z-index:2;
  background:var(--mat);
  padding:14px;
  display:flex;align-items:center;justify-content:center;
}
.screen{
  height:100%;
  width:auto;
  aspect-ratio:480/800;
  max-width:100%;
  background:transparent;overflow:hidden;position:relative;
  container-type:size;
}
.screen img{width:100%;height:100%;object-fit:cover;display:block;background:transparent}
.screen .empty{position:absolute;inset:0;display:flex;align-items:center;justify-content:center;color:#888;font-size:.78rem;text-align:center;padding:1rem;line-height:1.4;background:#0c0a09}
.screen .fbnote{position:absolute;left:0;right:0;bottom:0;z-index:3;background:rgba(10,8,6,.72);color:#f3ebe3;font-size:.66rem;text-align:center;padding:.3rem .4rem;line-height:1.25}
.lab-layer{position:absolute;inset:0;pointer-events:none;overflow:hidden}
.lab-layer .lab{
  position:absolute;white-space:nowrap;line-height:1.15;
  transform:translate(-50%,-50%);
  text-shadow:0 0 2px rgba(0,0,0,.35);
}
.caps{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:1rem;margin-bottom:1rem}
.caps .title{font-weight:700;font-size:1.15rem;margin:0 0 .35rem;line-height:1.3}
.caps .dates{font-size:.9rem;color:var(--dim);margin:0 0 .65rem;line-height:1.45}
.caps .desc{font-size:.92rem;line-height:1.5;margin:0;white-space:pre-wrap;word-break:break-word}
.caps .labs{margin:.75rem 0 0;padding:0;list-style:none}
.caps .labs li{font-size:.85rem;color:var(--dim);padding:.25rem 0;border-top:1px solid #2e2720}
.caps .labs li:first-child{border-top:0;padding-top:0}
.caps .file{margin:.75rem 0 0;font-size:.7rem;color:var(--dim);word-break:break-all}
.caps .muted{color:var(--dim);font-size:.88rem;margin:0}
.navrow{display:flex;align-items:center;justify-content:center;gap:1rem;margin-bottom:.75rem}
.navrow button{
  width:64px;height:64px;border-radius:50%;border:1px solid var(--line);background:var(--panel);color:var(--txt);
  font-size:1.6rem;line-height:1;cursor:pointer;font-weight:700;
}
.navrow button:disabled{opacity:.35;cursor:default}
.navrow .pos{font-size:.85rem;color:var(--dim);min-width:4.5rem;text-align:center;font-variant-numeric:tabular-nums}
button.pri{
  width:100%;padding:.75rem;border:0;border-radius:10px;background:var(--acc);color:#1a1612;
  font:inherit;font-weight:700;cursor:pointer;
}
button.pri:disabled{opacity:.45;cursor:default}
.status{font-size:.82rem;color:var(--dim);text-align:center;min-height:1.2em;margin-top:.85rem}
footer{text-align:center;padding:1rem;font-size:.75rem;color:var(--dim)}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu">Tintenklecks</a></div>
  <div class="head-right"><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<h1>Live-Anzeige</h1>
<div class="stage">
  <div class="device" aria-label="Waveshare PhotoPainter">
    <div class="rail top"></div>
    <div class="rail right"></div>
    <div class="rail bottom"></div>
    <div class="rail left"></div>
    <div class="mat"><div class="screen" id="screen"><div class="empty">Kein Bild</div></div></div>
  </div>
</div>
<section class="caps" id="caps" aria-live="polite">
  <p class="muted">Lade…</p>
</section>
<div class="navrow">
  <button type="button" id="btnPrev" aria-label="Vorheriges Bild">‹</button>
  <div class="pos" id="pos">—</div>
  <button type="button" id="btnNext" aria-label="Nächstes Bild">›</button>
</div>
<button class="pri" type="button" id="btnShow" disabled>Am Rahmen anzeigen</button>
<p class="status" id="status"></p>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
let items=[], idx=0, uiBusy=false, panelFile='';
function setBusy(on,msg){
  if(on && uiBusy) return false;
  uiBusy=!!on;
  document.body.classList.toggle('busy',uiBusy);
  const ov=document.getElementById('busyOv');
  if(ov) ov.classList.toggle('on',uiBusy);
  const m=document.getElementById('busyMsg');
  if(m && msg) m.textContent=msg;
  return true;
}
function esc(s){return String(s||'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#39;'}[c]));}
function primaryFile(last){
  if(!last) return '';
  return String(last).split(',')[0].trim();
}
function findIndex(file){
  if(!file) return 0;
  const i=items.findIndex(it=>it.file===file);
  return i>=0?i:0;
}
function renderCaps(it){
  const caps=document.getElementById('caps');
  if(!it){
    caps.innerHTML='<p class="muted">Keine Bilder in /pic/</p>';
    return;
  }
  const m=it.meta||{};
  const parts=[];
  const baseName=String(it.file||'').replace(/\.bmp$/i,'');
  parts.push('<p class="title">'+(m.name?esc(m.name):esc(baseName))+'</p>');
  const d=[];
  if(m.birth) d.push('* '+esc(m.birth));
  if(m.death) d.push('† '+esc(m.death));
  if(d.length) parts.push('<p class="dates">'+d.join('<br/>')+'</p>');
  const desc=(m.description||m.beschreibung||'').trim();
  if(desc) parts.push('<p class="desc">'+esc(desc)+'</p>');
  const labs=Array.isArray(m.labels)?m.labels:[];
  const texts=labs.filter(function(L){
    if(!L||!L.text) return false;
    const role=L.role||'';
    if(role==='name'||role==='birth'||role==='death') return false;
    return String(L.text).trim().length>0;
  }).map(function(L){ return String(L.text).trim(); });
  if(texts.length){
    parts.push('<ul class="labs">'+texts.map(t=>'<li>'+esc(t)+'</li>').join('')+'</ul>');
  }
  if(!m.name && !d.length && !desc && !texts.length){
    parts.push('<p class="muted">Keine Beschriftung</p>');
  }
  parts.push('<p class="file">'+esc(it.file)+'</p>');
  caps.innerHTML=parts.join('');
}
function fontCss(font){
  if(font==='sans-serif') return 'system-ui,sans-serif';
  if(font==='script') return '"Dancing Script","Segoe Script","Brush Script MT",cursive';
  return 'Georgia,"Times New Roman",serif';
}
function renderLabelOverlay(screen, m){
  const show=m && m.captionVisible!==false;
  const labs=Array.isArray(m&&m.labels)?m.labels:[];
  if(!show || !labs.length) return;
  const layer=document.createElement('div');
  layer.className='lab-layer';
  labs.forEach(function(L){
    if(!L || !String(L.text||'').trim()) return;
    const el=document.createElement('div');
    el.className='lab';
    el.textContent=L.text;
    const x=((L.x!=null)?L.x:0.5)*100;
    const y=((L.y!=null)?L.y:0.9)*100;
    const rot=+(L.rotate||0);
    const size=+(L.size||28);
    const align=L.align||'center';
    let tx='-50%', origin='center center';
    if(align==='left'){ tx='0%'; origin='left center'; }
    else if(align==='right'){ tx='-100%'; origin='right center'; }
    el.style.left=x+'%';
    el.style.top=y+'%';
    el.style.color=L.color||'#fff';
    el.style.fontFamily=fontCss(L.font);
    el.style.fontWeight=L.bold?'700':'400';
    el.style.fontSize='calc(100cqh * '+size+' / 800)';
    el.style.transform='translate('+tx+', -50%) rotate('+rot+'deg)';
    el.style.transformOrigin=origin;
    layer.appendChild(el);
  });
  if(layer.childNodes.length) screen.appendChild(layer);
}
function noteFallback(screen){
  if(screen.querySelector('.fbnote')) return;
  const d=document.createElement('div');
  d.className='fbnote';
  d.textContent='Kleine Vorschau · Zuschnitt nicht geladen';
  screen.appendChild(d);
}
function renderScreen(it){
  const screen=document.getElementById('screen');
  if(!it){
    screen.innerHTML='<div class="empty">Kein Bild</div>';
    return;
  }
  const m=it.meta||{};
  const f=encodeURIComponent(it.file);
  screen.innerHTML='';
  const img=document.createElement('img');
  img.alt='';
  const urls=['/api/src?name='+f,'/api/thumb?name='+f];
  let u=0, retried=false;
  img.onload=function(){ renderLabelOverlay(screen, m); if(u>0) noteFallback(screen); };
  img.onerror=function(){
    if(!retried){ retried=true; setTimeout(function(){ img.src=urls[u]+'&r='+Date.now(); }, 700); return; }
    u++; retried=false;
    if(u<urls.length){ img.src=urls[u]; return; }
    screen.innerHTML='<div class="empty">Bild nicht geladen<br/><small>Gerät belegt oder kein Vorschaubild</small></div>';
  };
  img.src=urls[0];
  screen.appendChild(img);
}
function updateNav(){
  const n=items.length;
  document.getElementById('pos').textContent=n?(idx+1)+' / '+n:'—';
  document.getElementById('btnPrev').disabled=!n;
  document.getElementById('btnNext').disabled=!n;
  const it=n?items[idx]:null;
  const btn=document.getElementById('btnShow');
  btn.disabled=!it;
  const same=it && panelFile && it.file===panelFile;
  btn.textContent=same?'Am Rahmen (aktuell)':'Am Rahmen anzeigen';
}
function showLocal(i){
  if(!items.length){ idx=0; renderScreen(null); renderCaps(null); updateNav(); return; }
  idx=((i%items.length)+items.length)%items.length;
  const it=items[idx];
  renderScreen(it);
  renderCaps(it);
  updateNav();
  const st=document.getElementById('status');
  if(panelFile && it.file===panelFile) st.textContent='Am Rahmen: '+panelFile;
  else if(panelFile) st.textContent='Vorschau · am Rahmen: '+panelFile;
  else st.textContent='Vorschau';
  if(!it._metaTried){
    it._metaTried=true;
    fetch('/api/meta?name='+encodeURIComponent(it.file)).then(r=>r.json()).then(m=>{
      if(m && typeof m==='object') it.meta=m;
      if(items[idx]===it){ renderCaps(it); renderScreen(it); updateNav(); }
    }).catch(()=>{});
  }
}
async function showOnPanel(){
  if(!items.length) return;
  const it=items[idx];
  if(!it) return;
  if(!setBusy(true,'Anzeigen…')) return;
  const st=document.getElementById('status');
  st.textContent='';
  try{
    const r=await fetch('/api/show',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+encodeURIComponent(it.file)});
    const t=(await r.text()).trim();
    if(r.ok){
      panelFile=it.file;
      updateNav();
      st.textContent='Am Rahmen: '+panelFile;
    }else{
      st.textContent=t||('Fehler '+r.status);
    }
  }catch(e){ st.textContent=String(e); }
  finally{ setBusy(false); }
}
async function refreshStatus(){
  try{
    const s=await (await fetch('/api/status')).json();
    const bat=document.getElementById('bat');
    if(s.usb) bat.textContent='USB-Betrieb';
    else if(s.battery<0) bat.textContent='Akku —';
    else bat.textContent='Akku '+s.battery+'%'+(s.charging?' · lädt':'');
    const z=document.getElementById('btnZzz');
    if(z) z.hidden=!!s.usb;
    document.getElementById('foot').textContent=s.copyright||'© 2026 Ingo Lissors';
  }catch(e){}
}
document.getElementById('btnZzz').onclick=()=>{ fetch('/api/sleep',{method:'POST'}).catch(()=>{}); };
async function load(){
  try{
    let last='';
    try{
      const f=await (await fetch('/api/frame')).json();
      last=primaryFile(f.last||'');
    }catch(e){}
    panelFile=last;
    if(last){
      items=[{file:last, thumb:true}];
      idx=0;
      showLocal(0);
    }
    let names=[];
    for(let attempt=0; attempt<20; attempt++){
      try{
        names=await (await fetch('/api/list')).json();
        if(Array.isArray(names) && names.length) break;
      }catch(e){}
      await new Promise(r=>setTimeout(r,1500));
    }
    if(Array.isArray(names) && names.length){
      items=names;
      idx=findIndex(last);
      showLocal(idx);
    }else if(!last){
      items=[]; showLocal(0);
    }
  }catch(e){
    document.getElementById('caps').innerHTML='<p class="muted">Fehler: '+esc(e)+'</p>';
  }
}
document.getElementById('btnPrev').onclick=()=>showLocal(idx-1);
document.getElementById('btnNext').onclick=()=>showLocal(idx+1);
document.getElementById('btnShow').onclick=()=>showOnPanel();
refreshStatus(); setInterval(refreshStatus,15000); load();
</script>
</body></html>
)HTML";
