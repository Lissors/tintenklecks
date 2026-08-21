#pragma once
#include <Arduino.h>

static const char PAGE_SETUP[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Tintenklecks · WLAN</title>
<style>
:root{--bg:#1a1612;--line:#56483c;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888}
body{font-family:system-ui,sans-serif;margin:0;background:var(--bg);color:var(--txt);min-height:100vh}
.top{display:flex;justify-content:space-between;align-items:center;padding:.85rem 1.1rem;border-bottom:1px solid var(--line);gap:.75rem;flex-wrap:wrap}
.brand a{color:var(--txt);text-decoration:none;font-weight:700}
.wrap{display:flex;align-items:center;justify-content:center;padding:1.5rem 0 2rem}
.card{max-width:420px;width:92%;background:#2a231c;border-radius:12px;padding:1.5rem}
h1{font-size:1.4rem;margin:0 0 .35rem}
p{opacity:.85;line-height:1.45;font-size:.92rem}
label{display:block;margin:.8rem 0 .3rem;font-size:.75rem;letter-spacing:.06em;text-transform:uppercase;opacity:.7}
input{width:100%;box-sizing:border-box;padding:.65rem .75rem;border-radius:8px;border:1px solid #56483c;background:#1a1612;color:#f3ebe3;font:inherit}
.pw{display:flex;gap:.4rem;align-items:stretch}
.pw input{flex:1;min-width:0}
.eye{margin:0;width:2.6rem;padding:.45rem;flex:none;display:flex;align-items:center;justify-content:center;border-radius:8px;border:1px solid #56483c;background:#1a1612;color:#c4966e;cursor:pointer}
.eye svg{width:1.25rem;height:1.25rem;display:block;fill:none;stroke:currentColor;stroke-width:2;stroke-linecap:round;stroke-linejoin:round}
.eye .eye-off{display:none}
.eye[aria-pressed="true"] .eye-on{display:none}
.eye[aria-pressed="true"] .eye-off{display:block}
button.pri{margin-top:1rem;width:100%;padding:.75rem;border:0;border-radius:8px;background:#c4966e;color:#1a1612;font-weight:700}
.hint{margin-top:1rem;font-size:.8rem;opacity:.65}
a{color:#c4966e}
</style></head><body>
<div class="top"><div class="brand"><a href="/menu">Tintenklecks</a></div></div>
<div class="wrap"><div class="card">
<h1>WLAN</h1>
<p>Heimnetz eintragen oder AP-Passwort festlegen. Offline im Menü bleiben.</p>
<form method="POST" action="/wifi" id="wifiForm">
<label>SSID (Heimnetz)</label><input id="ssid" name="ssid" maxlength="32" autocomplete="off" spellcheck="false"/>
<label>WLAN-Passwort</label>
<div class="pw">
  <input id="pass" name="pass" type="password" maxlength="64" autocomplete="off"/>
  <button type="button" class="eye" data-for="pass" aria-label="Passwort zeigen" aria-pressed="false">
    <svg class="eye-on" viewBox="0 0 24 24" aria-hidden="true"><path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/></svg>
    <svg class="eye-off" viewBox="0 0 24 24" aria-hidden="true"><path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/><path d="M4 4l16 16"/></svg>
  </button>
</div>
<label>AP-Passwort (Hotspot)</label>
<div class="pw">
  <input id="apPass" name="apPass" type="password" maxlength="63" minlength="8" autocomplete="off"/>
  <button type="button" class="eye" data-for="apPass" aria-label="Passwort zeigen" aria-pressed="false">
    <svg class="eye-on" viewBox="0 0 24 24" aria-hidden="true"><path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/></svg>
    <svg class="eye-off" viewBox="0 0 24 24" aria-hidden="true"><path d="M2 12s3.5-7 10-7 10 7 10 7-3.5 7-10 7S2 12 2 12z"/><circle cx="12" cy="12" r="3"/><path d="M4 4l16 16"/></svg>
  </button>
</div>
<button class="pri" type="submit">Speichern &amp; neu starten</button>
</form>
<form method="POST" action="/offline"><button type="submit" style="margin-top:.65rem;width:100%;padding:.75rem;border:0;border-radius:8px;background:#56483c;color:#f3ebe3;font-weight:700">Offline · Menü öffnen</button></form>
<p class="hint">AP-Passwort mindestens 8 Zeichen (WPA2). Gilt nach dem Neustart. Hotspot heißt Tintenklecks. Später <a href="http://tintenklecks.local">tintenklecks.local</a></p>
</div></div>
<script>
document.querySelectorAll('.eye').forEach(function(btn){
  btn.onclick=function(){
    const el=document.getElementById(btn.getAttribute('data-for'));
    if(!el) return;
    const show=el.type==='password';
    el.type=show?'text':'password';
    btn.setAttribute('aria-pressed', show?'true':'false');
  };
});
(async function(){
  try{
    const w=await (await fetch('/api/wifi')).json();
    const s=document.getElementById('ssid');
    const p=document.getElementById('pass');
    const a=document.getElementById('apPass');
    if(s) s.value=w.ssid||'';
    if(p) p.value=w.pass||'';
    if(a) a.value=w.apPass||'';
  }catch(e){}
})();
</script>
</body></html>
)HTML";

static const char PAGE_MENU[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Tintenklecks</title>
<style>
:root{--bg:#14110f;--panel:#241e18;--line:#4a3f35;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt);min-height:100vh}
.top{display:flex;justify-content:space-between;align-items:center;padding:.85rem 1.1rem;border-bottom:1px solid var(--line);gap:.75rem;flex-wrap:wrap}
.brand{font-weight:700;font-size:1.05rem}.meta{font-size:.8rem;color:var(--dim);display:flex;gap:1rem;align-items:center}
.bat{font-variant-numeric:tabular-nums}
.zzz{margin:0;padding:.2rem .5rem;border-radius:6px;border:1px solid var(--line);background:#1a1612;color:var(--dim);font:inherit;font-size:.78rem;font-weight:700;letter-spacing:.08em;cursor:pointer}
main{max-width:720px;margin:0 auto;padding:1.5rem 1rem}
h1{font-size:1.35rem;margin:0 0 .35rem}
.lead{color:var(--dim);margin:0 0 1.25rem;line-height:1.45}
.grid{display:grid;grid-template-columns:1fr 1fr;gap:.75rem}
@media(max-width:560px){.grid{grid-template-columns:1fr}}
a.tile{display:block;padding:1.15rem 1rem;background:var(--panel);border:1px solid var(--line);border-radius:12px;color:var(--txt);text-decoration:none}
a.tile strong{display:block;font-size:1.05rem;margin-bottom:.25rem}
a.tile span{font-size:.82rem;color:var(--dim);line-height:1.35}
footer{text-align:center;padding:2rem 1rem 1.5rem;font-size:.75rem;color:var(--dim)}
</style></head><body>
<div class="top"><div class="brand">Tintenklecks</div><div class="meta"><button type="button" class="zzz" id="btnZzz">zzz</button><span class="bat" id="bat">Akku …</span></div></div>
<main>
<h1>Hauptmenü</h1>
<p class="lead">Bild anlegen, Galerie verwalten, Live ansehen oder System prüfen.</p>
<div class="grid">
<a class="tile" href="/live"><strong>Live-Anzeige</strong><span>Rahmen mit aktuellem Bild, Text, Blättern</span></a>
<a class="tile" href="/studio"><strong>Neues Bild</strong><span>Zuschnitt, Stil, Beschriftung, Anzeigen</span></a>
<a class="tile" href="/gallery"><strong>Bilder</strong><span>Anzeigen, bearbeiten, löschen</span></a>
<a class="tile" href="/frame"><strong>Rahmen</strong><span>Automatischer Bildwechsel</span></a>
<a class="tile" href="/system"><strong>System</strong><span>Akku, Panel, Neustart, WLAN</span></a>
<a class="tile" href="/setup"><strong>WLAN-Setup</strong><span>Zugang speichern oder ändern</span></a>
</div>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
async function refreshStatus(){
  if(window.__zzz) return;
  try{
    const s=await (await fetch('/api/status')).json();
    const bat=document.getElementById('bat');
    if(s.usb) bat.textContent='USB-Betrieb';
    else if(s.battery<0) bat.textContent='Akku —';
    else bat.textContent='Akku '+s.battery+'%'+(s.charging?' · lädt':'');
    const z=document.getElementById('btnZzz');
    if(z) z.hidden=!!s.usb;
    document.getElementById('foot').textContent=s.copyright||'© 2026 Ingo Lissors';
  }catch(e){ document.getElementById('bat').textContent='Akku ?'; }
}
document.getElementById('btnZzz').onclick=()=>{
  if(window.__zzz) return;
  window.__zzz=1;
  (async()=>{
    for(let i=0;i<8;i++){
      try{
        const r=await fetch('/api/sleep',{method:'POST',cache:'no-store'});
        const t=await r.text();
        if(r.ok && t.indexOf('USB')<0) return;
        if(t.indexOf('USB')>=0) break;
      }catch(e){}
      await new Promise(res=>setTimeout(res,300));
    }
    window.__zzz=0;
  })();
};
refreshStatus(); setInterval(refreshStatus,15000);
</script>
</body></html>
)HTML";

static const char PAGE_GALLERY[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Bilder · Tintenklecks</title>
<link rel="preconnect" href="https://fonts.googleapis.com"/>
<link href="https://fonts.googleapis.com/css2?family=Dancing+Script:wght@400;700&display=swap" rel="stylesheet"/>
<style>
:root{--bg:#14110f;--panel:#241e18;--line:#4a3f35;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt)}
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
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;font-weight:700;flex:none}
main{max-width:960px;margin:0 auto;padding:1rem}
h1{font-size:1.2rem;margin:0 0 .5rem}
h2.sec{font-size:.72rem;letter-spacing:.1em;text-transform:uppercase;color:var(--dim);margin:.35rem 0 .6rem}
.tabs{display:flex;gap:.45rem;margin:0 0 1rem}
.tabs button{flex:1;padding:.55rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit;font-weight:700;cursor:pointer}
.tabs button.on{background:var(--acc);color:#1a1612;border:0}
.list{display:grid;grid-template-columns:repeat(auto-fill,144px);justify-content:center;gap:1rem}
.card{position:relative;background:var(--panel);border:1px solid var(--line);border-radius:12px;overflow:hidden;display:flex;flex-direction:column}
.card.e6raw{border-color:#c45}
.card img{width:100%;aspect-ratio:3/5;object-fit:cover;background:#0c0a09;display:block}
.card.land img,.card.land .ph{aspect-ratio:5/3}
.thumb-wrap{position:relative}
.onpic{position:absolute;inset:0;pointer-events:none;overflow:hidden}
.onpic-lab{position:absolute;white-space:nowrap;line-height:1.15;text-shadow:0 0 3px #000,0 1px 2px #000;transform-origin:center center}
.e6tag{position:absolute;top:.45rem;left:.45rem;z-index:1;background:#8a4030;color:#f3ebe3;font-size:.68rem;font-weight:700;padding:.2rem .45rem;border-radius:6px;pointer-events:none}
.ph{width:100%;aspect-ratio:3/5;background:#0c0a09;display:flex;align-items:center;justify-content:center;color:var(--dim);font-size:.8rem}
.info{padding:.75rem;flex:1}
.info .name{font-weight:700;font-size:.95rem;margin:0 0 .25rem}
.info .dates{font-size:.8rem;color:var(--dim);line-height:1.4;margin:0 0 .35rem;min-height:2.4em}
.info .desc{
  font-size:.8rem;color:var(--dim);line-height:1.4;margin:0 0 .4rem;
  cursor:pointer;word-break:break-word;
  display:-webkit-box;-webkit-box-orient:vertical;-webkit-line-clamp:3;
  overflow:hidden;
}
.info .desc.open{display:block;-webkit-line-clamp:unset;overflow:visible}
.info .file{font-size:.7rem;color:var(--dim);word-break:break-all}
.actions{display:flex;flex-wrap:wrap;gap:.35rem;padding:0 .75rem .75rem}
button{font:inherit;cursor:pointer;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);padding:.4rem .55rem;font-size:.78rem}
button.pri{background:var(--acc);color:#1a1612;border:0;font-weight:700}
button.danger{border-color:#8a4030;color:#e8a090}
.status{font-size:.85rem;color:var(--dim);margin-bottom:1rem}
footer{text-align:center;padding:1.5rem;font-size:.75rem;color:var(--dim)}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div class="busy-msg" id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu">Tintenklecks</a></div>
  <div class="head-right"><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<h1>Bilder</h1>
<p class="status" id="status">Lade…</p>
<div class="tabs">
  <button type="button" id="tabNormal" class="on">Zufall</button>
  <button type="button" id="tabMemory">Erinnerungen</button>
</div>
<div id="secNormal">
<h2 class="sec">Zufall</h2>
<div class="list" id="listNormal"></div>
</div>
<div id="secMemory" hidden>
<h2 class="sec">Erinnerungen</h2>
<div class="list" id="listMemory"></div>
</div>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
let uiBusy=false;
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
function isMemoryMeta(m){
  if(!m) return false;
  if(m.kind==='memory') return true;
  if(m.kind==='normal') return false;
  return !!(String(m.birth||'').trim()||String(m.death||'').trim()||String(m.special||'').trim());
}
async function refreshStatus(){
  if(window.__zzz) return;
  try{
    const s=await (await fetch('/api/status')).json();
    const bat=document.getElementById('bat');
    if(s.usb) bat.textContent='USB-Betrieb';
    else if(s.battery<0) bat.textContent='Akku —';
    else bat.textContent='Akku '+s.battery+'%'+(s.charging?' · lädt':'');
    const z=document.getElementById('btnZzz');
    if(z) z.hidden=!!s.usb;
    document.getElementById('foot').textContent=s.copyright||'© 2026 Ingo Lissors';
    if(typeof s.potLeft==='number') window.__potLeft=s.potLeft;
    if(typeof s.potTotal==='number') window.__potTotal=s.potTotal;
    refreshCounts();
  }catch(e){}
}
function specialLine(m){
  const dt=(m&&m.special||'').trim();
  if(!dt) return '';
  const k=(m.specialKind||'').trim();
  return k?(k+' '+dt):dt;
}
function metaLines(m, fallbackName){
  if(!m || (!m.name && !m.birth && !m.death && !m.special && !(m.description||m.beschreibung))){
    return '<p class="name">'+esc(fallbackName||'—')+'</p><p class="dates"> </p>';
  }
  const lines=[];
  if(m.name) lines.push('<p class="name">'+esc(m.name)+'</p>');
  else lines.push('<p class="name">'+esc(fallbackName||'—')+'</p>');
  const d=[];
  if(m.birth) d.push('* '+esc(m.birth));
  if(m.death) d.push('† '+esc(m.death));
  const sp=specialLine(m);
  if(sp) d.push(esc(sp));
  lines.push('<p class="dates">'+(d.length?d.join('<br/>'):' ')+'</p>');
  const desc=(m.description||m.beschreibung||'').trim();
  if(desc){
    lines.push('<p class="desc" title="Tippen für ganzen Text">'+esc(desc)+'</p>');
  }
  return lines.join('');
}
function fontCss(font){
  if(font==='sans-serif') return "system-ui,sans-serif";
  if(font==='script') return "'Dancing Script','Segoe Script','Brush Script MT',cursive";
  return "Georgia,'Times New Roman',serif";
}
function onPicLabels(m){
  if(!m) return [];
  if(typeof m.captionVisible==='boolean' && !m.captionVisible) return [];
  if(Array.isArray(m.labels) && m.labels.length) return m.labels;
  const out=[];
  if(m.name && m.showName!==false) out.push({role:'name',text:m.name,x:0.5,y:0.80,size:m.sizeName||36,color:'#FFFFFF',font:'serif',align:'center',bold:false,rotate:0});
  if(m.birth && m.showBirth!==false) out.push({role:'birth',text:'* '+m.birth,x:0.5,y:0.88,size:m.sizeDates||22,color:'#FFFFFF',font:'serif',align:'center',bold:false,rotate:0});
  if(m.death && m.showDeath!==false) out.push({role:'death',text:'\u2020 '+m.death,x:0.5,y:0.94,size:m.sizeDates||22,color:'#FFFFFF',font:'serif',align:'center',bold:false,rotate:0});
  const sp=specialLine(m);
  if(sp && m.showSpecial!==false) out.push({role:'special',text:sp,x:0.5,y:0.70,size:m.sizeDates||22,color:'#FFFFFF',font:'serif',align:'center',bold:false,rotate:0});
  return out;
}
function onPicHtml(m){
  const labs=onPicLabels(m);
  if(!labs.length) return '';
  const panelW=(m && m.orient==='landscape')?800:480;
  const scale=144/panelW;
  const bits=[];
  for(let i=0;i<labs.length;i++){
    const L=labs[i];
    if(!(L.text||'').trim()) continue;
    const x=((L.x!=null)?L.x:0.5)*100;
    const y=((L.y!=null)?L.y:0.9)*100;
    const rot=+(L.rotate||0);
    const align=L.align||'center';
    const tx=align==='left'?'0%':(align==='right'?'-100%':'-50%');
    const size=Math.max(7, Math.round((L.size||28)*scale));
    const weight=L.bold?'700':'400';
    const col=String(L.color||'#FFFFFF').replace(/[^#A-Fa-f0-9]/g,'');
    bits.push('<span class="onpic-lab" style="color:'+col+';left:'+x+'%;top:'+y+'%;transform:translate('+tx+',-50%) rotate('+rot+'deg);font-weight:'+weight+';font-size:'+size+'px;font-family:'+fontCss(L.font)+'">'+esc(L.text)+'</span>');
  }
  if(!bits.length) return '';
  return '<div class="onpic">'+bits.join('')+'</div>';
}
function setOnPic(card, m){
  if(!card) return;
  card.classList.toggle('land', !!(m && m.orient==='landscape'));
  const wrap=card.querySelector('.thumb-wrap');
  if(!wrap) return;
  const old=wrap.querySelector('.onpic');
  const html=onPicHtml(m);
  if(old) old.remove();
  if(html) wrap.insertAdjacentHTML('beforeend', html);
}
function thumbPlaceholder(txt){
  return '<div class="ph">'+(txt||'Kein Vorschaubild<br/><small>neu speichern</small>')+'</div>';
}
function thumbImgHtml(file){
  return '<img loading="lazy" decoding="async" src="/api/thumb?name='+encodeURIComponent(file)+'&r='+Date.now()+'" alt="" data-thumb="1"/>';
}
function thumbHtml(it){
  const pic=(it.ct||it.thumb||it.src)?thumbImgHtml(it.file):thumbPlaceholder();
  return '<div class="thumb-wrap">'+pic+onPicHtml(it.meta)+'</div>';
}
let galleryCount=0;
const fixQ=[];
let fixRunning=false;
let fixDone=0, fixSkip=0, fixFail=0, fixTotal=0, fixStart=0;
function dropFromFixQ(name){
  for(let i=fixQ.length-1;i>=0;i--) if(fixQ[i].name===name) fixQ.splice(i,1);
}
function fixEta(){
  const done=fixDone+fixSkip+fixFail;
  if(!fixStart || done<3) return '';
  const leftMs=((Date.now()-fixStart)/done)*(fixTotal-done);
  const min=Math.round(leftMs/60000);
  return min>=1 ? ' · noch ca. '+min+' min' : ' · fast fertig';
}
function refreshGalleryStatus(){
  const status=document.getElementById('status');
  if(!status || !galleryCount) return;
  let t=galleryCount+' Bild(er)';
  if(fixRunning || fixQ.length){
    t+=' · Vorschau '+(fixDone+fixSkip+fixFail+1)+'/'+fixTotal+' wird erstellt…'+fixEta();
  } else if(fixDone || fixSkip || fixFail){
    t+=' · '+fixDone+' Vorschau(en) erstellt';
    if(fixSkip) t+=' · '+fixSkip+' ohne Zuschnitt (Studio neu speichern)';
    if(fixFail) t+=' · '+fixFail+' fehlgeschlagen';
  }
  status.textContent=t;
}
function markNoCrop(card, txt){
  if(!card) return;
  card.classList.add('e6raw');
  let tag=card.querySelector('.e6tag');
  if(!tag){
    tag=document.createElement('div');
    tag.className='e6tag';
    card.insertBefore(tag, card.firstChild);
  }
  tag.textContent=txt||'Kein Zuschnitt';
}
function loadImg(url){
  return new Promise(function(resolve,reject){
    const im=new Image();
    im.onload=function(){ resolve(im); };
    im.onerror=function(){ reject(new Error('img')); };
    im.src=url;
  });
}
function make240Jpeg(img, meta){
  const orient=(meta && meta.orient==='landscape')?'landscape':'portrait';
  const tw=orient==='landscape'?800:480, th=orient==='landscape'?480:800;
  const zoom=(meta && meta.zoom!=null)?+meta.zoom:100;
  const panX=(meta && typeof meta.panX==='number')?meta.panX:0.5;
  const panY=(meta && typeof meta.panY==='number')?meta.panY:0.5;
  const z=Math.max(tw/img.naturalWidth, th/img.naturalHeight)*(zoom/100);
  const dw=img.naturalWidth*z, dh=img.naturalHeight*z;
  const scale=240/Math.max(tw,th);
  const cw=Math.max(1,Math.round(tw*scale)), ch=Math.max(1,Math.round(th*scale));
  const c=document.createElement('canvas');
  c.width=cw; c.height=ch;
  const ctx=c.getContext('2d');
  ctx.fillStyle='#111';
  ctx.fillRect(0,0,cw,ch);
  ctx.drawImage(img, (tw-dw)*panX*scale, (th-dh)*panY*scale, dw*scale, dh*scale);
  return new Promise(function(resolve){ c.toBlob(function(b){ resolve(b); }, 'image/jpeg', 0.72); });
}
async function uploadCleanThumb(name, blob){
  const tfd=new FormData();
  tfd.append('file', blob, String(name).replace(/\.bmp$/i,'')+'_thumb.jpg');
  const r=await fetch('/api/upload-thumb?kind=clean&name='+encodeURIComponent(name),{method:'POST',body:tfd});
  if(!r.ok) throw new Error(await r.text());
}
function showThumbBlob(card, blob){
  if(!card || !blob) return;
  const url=URL.createObjectURL(blob);
  let img=card.querySelector('img[data-thumb]');
  if(!img){
    img=document.createElement('img');
    img.setAttribute('data-thumb','1');
    img.decoding='async';
    img.alt='';
    const ph=card.querySelector('.ph');
    if(ph) ph.replaceWith(img);
    else card.insertBefore(img, card.firstChild);
  }
  img.addEventListener('load', function(){ URL.revokeObjectURL(url); }, {once:true});
  img.src=url;
}
async function runFixQ(){
  if(fixRunning) return;
  fixRunning=true;
  fixStart=Date.now();
  refreshGalleryStatus();
  while(fixQ.length){
    if(!fixStart) fixStart=Date.now();
    const job=fixQ.shift();
    const enc=encodeURIComponent(job.name);
    let img=null;
    if(job.src){
      try{ img=await loadImg('/api/src?name='+enc); }catch(e){ img=null; }
    }
    if(!img){
      fixSkip++;
      markNoCrop(job.card, 'Kein Zuschnitt');
      refreshGalleryStatus();
      continue;
    }
    try{
      let meta=null;
      try{ meta=await (await fetch('/api/meta?name='+enc)).json(); }catch(e){ meta=null; }
      const blob=await make240Jpeg(img, meta);
      if(!blob) throw new Error('jpeg');
      await uploadCleanThumb(job.name, blob);
      fixDone++;
      showThumbBlob(job.card, blob);
    }catch(e){
      fixFail++;
      markNoCrop(job.card);
    }
    refreshGalleryStatus();
  }
  fixRunning=false;
  refreshGalleryStatus();
}
function enqueueFix(name, card, hasSrc){
  if(!name || !hasSrc) return;
  for(let i=0;i<fixQ.length;i++) if(fixQ[i].name===name) return;
  fixQ.push({name:name, card:card, src:true});
  fixTotal=fixQ.length;
  runFixQ();
}
function bindThumbFallback(root){
  (root||document).querySelectorAll('img[data-thumb]').forEach(function(img){
    img.addEventListener('error', function(){
      const card=img.closest('.card');
      const n=+(img.getAttribute('data-try')||0);
      if(n<1 && card){
        img.setAttribute('data-try','1');
        img.src='/api/thumb?name='+encodeURIComponent(card.getAttribute('data-file'))+'&r='+Date.now();
        return;
      }
      const d=document.createElement('div');
      d.className='ph';
      d.innerHTML='Kein Vorschaubild<br/><small>neu speichern</small>';
      img.replaceWith(d);
      if(card && card.getAttribute('data-src')==='1') enqueueFix(card.getAttribute('data-file'), card, true);
    });
  });
}
function cardHtml(it){
  const f=esc(it.file);
  const land=(it.meta&&it.meta.orient==='landscape')?' land':'';
  return '<article class="card'+land+'" data-file="'+f+'" data-src="'+(it.src?1:0)+'">'+
    thumbHtml(it)+
    '<div class="info">'+metaLines(it.meta||(it.name?{name:it.name}:null), it.file.replace(/\.bmp$/i,''))+'<p class="file">'+f+'</p></div>'+
    '<div class="actions">'+
    '<button class="pri" data-act="edit">Bearbeiten</button>'+
    '<button class="pri" data-act="show">Anzeigen</button>'+
    '<button data-act="rename">Umbenennen</button>'+
    '<button class="danger" data-act="del">Löschen</button>'+
    '</div></article>';
}
function placeCard(card, m){
  const mem=document.getElementById('listMemory');
  const nor=document.getElementById('listNormal');
  if(!card||!mem||!nor) return;
  const dest=isMemoryMeta(m)?mem:nor;
  if(card.parentNode!==dest) dest.appendChild(card);
}
function refreshCounts(){
  const status=document.getElementById('status');
  const nN=document.getElementById('listNormal').querySelectorAll('.card').length;
  const nM=document.getElementById('listMemory').querySelectorAll('.card').length;
  let pot='';
  const t=window.__potTotal|0;
  if(typeof window.__potLeft==='number' && t>0){
    if(window.__potLeft<1) pot=' · Topf: neue Runde ('+t+')';
    else pot=' · Topf noch '+window.__potLeft+' von '+t;
  }
  if(status) status.textContent=nN+' Zufall · '+nM+' Erinnerung'+pot;
}
async function loadMetaCard(card){
  if(!card || card.dataset.metaLoaded) return;
  card.dataset.metaLoaded='1';
  const name=card.getAttribute('data-file');
  try{
    const m=await (await fetch('/api/meta?name='+encodeURIComponent(name))).json();
    const info=card.querySelector('.info');
    if(info){
      const fileEl=info.querySelector('.file');
      info.innerHTML=metaLines(m, name.replace(/\.bmp$/i,''))+(fileEl?fileEl.outerHTML:'');
    }
    setOnPic(card, m);
    placeCard(card, m);
  }catch(e){}
}
async function showGalleryPile(p){
  document.getElementById('tabNormal').classList.toggle('on', p==='normal');
  document.getElementById('tabMemory').classList.toggle('on', p==='memory');
  document.getElementById('secNormal').hidden=p!=='normal';
  document.getElementById('secMemory').hidden=p!=='memory';
  refreshCounts();
}
async function loadList(){
  const status=document.getElementById('status');
  const nor=document.getElementById('listNormal');
  const mem=document.getElementById('listMemory');
  try{
    let items=[];
    galleryCount=0;
    fixQ.length=0;
    fixDone=0; fixSkip=0; fixFail=0; fixTotal=0; fixStart=0;
    for(let i=0;i<90;i++){
      items=await (await fetch('/api/list')).json();
      if(Array.isArray(items) && items.length) break;
      status.textContent='Galerie-Index wird gebaut… ('+(i+1)+'/90)';
      await new Promise(r=>setTimeout(r,2000));
    }
    if(!items.length){ status.textContent='Keine Bilder (oder Index noch leer)'; nor.innerHTML=''; mem.innerHTML=''; return; }
    galleryCount=items.length;
    nor.innerHTML=''; mem.innerHTML='';
    items.forEach(function(it){
      const html=cardHtml(it);
      const wrap=document.createElement('div');
      wrap.innerHTML=html;
      const card=wrap.firstChild;
      placeCard(card, it.meta||{name:it.name, kind:it.kind});
    });
    status.textContent=items.length+' Bild(er)';
    bindThumbFallback(nor);
    bindThumbFallback(mem);
    const io=new IntersectionObserver((entries)=>{
      entries.forEach(function(ent){
        if(!ent.isIntersecting) return;
        const card=ent.target;
        io.unobserve(card);
        loadMetaCard(card).then(refreshCounts);
      });
    },{rootMargin:'100px'});
    nor.querySelectorAll('.card').forEach(c=>io.observe(c));
    mem.querySelectorAll('.card').forEach(c=>io.observe(c));
    refreshCounts();
  }catch(e){ status.textContent='Fehler: '+e; }
}
document.getElementById('tabNormal').onclick=function(){ showGalleryPile('normal'); };
document.getElementById('tabMemory').onclick=function(){ showGalleryPile('memory'); };
document.querySelector('main').addEventListener('click', async e=>{
  const desc=e.target.closest('.desc');
  if(desc){
    desc.classList.toggle('open');
    return;
  }
  const btn=e.target.closest('button[data-act]'); if(!btn) return;
  if(uiBusy) return;
  const card=btn.closest('.card'); const name=card.getAttribute('data-file');
  const act=btn.getAttribute('data-act');
  const status=document.getElementById('status');
  if(act==='edit'){
    location.href='/studio?edit='+encodeURIComponent(name);
    return;
  }
  if(act==='show'){
    if(!setBusy(true,'Anzeigen…')) return;
    try{
      const r=await fetch('/api/show',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+encodeURIComponent(name)});
      status.textContent=await r.text();
    }catch(err){ status.textContent=String(err); }
    finally{ setBusy(false); }
  } else if(act==='rename'){
    const base=name.replace(/\.bmp$/i,'');
    const neu=prompt('Neuer Dateiname (ohne .bmp):', base);
    if(!neu) return;
    const to=neu.replace(/\.bmp$/i,'')+'.bmp';
    if(!setBusy(true,'Umbenennen…')) return;
    dropFromFixQ(name);
    try{
      const r=await fetch('/api/rename',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+encodeURIComponent(name)+'&to='+encodeURIComponent(to)});
      status.textContent=await r.text();
      if(r.ok) await loadList();
    }catch(err){ status.textContent=String(err); }
    finally{ setBusy(false); }
  } else if(act==='del'){
    if(!confirm('"'+name+'" inkl. JSON, Vorschauen und Ton löschen?')) return;
    if(!setBusy(true,'Löschen…')) return;
    dropFromFixQ(name);
    try{
      const r=await fetch('/api/delete',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'name='+encodeURIComponent(name)});
      status.textContent=await r.text();
      if(r.ok) await loadList();
    }catch(err){ status.textContent=String(err); }
    finally{ setBusy(false); }
  }
});
document.getElementById('btnZzz').onclick=()=>{
  if(window.__zzz) return;
  window.__zzz=1;
  (async()=>{
    for(let i=0;i<8;i++){
      try{
        const r=await fetch('/api/sleep',{method:'POST',cache:'no-store'});
        const t=await r.text();
        if(r.ok && t.indexOf('USB')<0) return;
        if(t.indexOf('USB')>=0) break;
      }catch(e){}
      await new Promise(res=>setTimeout(res,300));
    }
    window.__zzz=0;
  })();
};
refreshStatus(); setInterval(refreshStatus,15000); loadList();
</script>
</body></html>
)HTML";

static const char PAGE_SYSTEM[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>System · Tintenklecks</title>
<style>
:root{--bg:#14110f;--panel:#241e18;--line:#4a3f35;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888}
*{box-sizing:border-box}body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt)}
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
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;flex:none}
main{max-width:520px;margin:0 auto;padding:1.25rem 1rem}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:1rem;margin-bottom:1rem}
h1{font-size:1.2rem;margin:0 0 1rem}h2{font-size:.72rem;letter-spacing:.1em;text-transform:uppercase;color:var(--dim);margin:0 0 .75rem}
.row{display:flex;justify-content:space-between;gap:1rem;padding:.35rem 0;font-size:.9rem;border-bottom:1px solid #2e2720}
.row:last-child{border:0}.row span{color:var(--dim)}
label.field{display:block;margin:.55rem 0;font-size:.8rem;color:var(--dim)}
label.field input,label.field select{width:100%;margin-top:.3rem;box-sizing:border-box;padding:.55rem .65rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit}
button{width:100%;margin-top:.55rem;padding:.7rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit;font-weight:700;cursor:pointer}
button.pri{background:var(--acc);color:#1a1612;border:0}
a.pri{display:block;width:100%;margin-top:.55rem;padding:.7rem;border-radius:8px;background:var(--acc);color:#1a1612;font:inherit;font-weight:700;text-align:center;text-decoration:none}
a.sec{display:block;width:100%;margin-top:.55rem;padding:.7rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit;font-weight:700;text-align:center;text-decoration:none;box-sizing:border-box}
button.danger{border-color:#8a4030;color:#e8a090}
.status{font-size:.85rem;color:var(--dim);min-height:1.2em;margin-top:.5rem}
.hint{font-size:.78rem;color:var(--dim);line-height:1.4;margin:.5rem 0 0}
footer{text-align:center;padding:1.5rem;font-size:.75rem;color:var(--dim)}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu">Tintenklecks</a></div>
  <div class="head-right"><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<h1>System</h1>
<div class="panel">
<h2>Status</h2>
<div class="row"><span>Gerät</span><b id="dev">—</b></div>
<div class="row"><span>IP</span><b id="ip">—</b></div>
<div class="row"><span>Modus</span><b id="mode">—</b></div>
<div class="row"><span>SD</span><b id="sd">—</b></div>
<div class="row"><span>Heap</span><b id="heap">—</b></div>
<div class="row"><span>NTP</span><b id="ntp">—</b></div>
</div>
<div class="panel">
<h2>Akku</h2>
<div class="row"><span>PMU</span><b id="bOk">—</b></div>
<div class="row"><span>Zelle</span><b id="bCell">—</b></div>
<div class="row"><span>Stand</span><b id="bPct">—</b></div>
<div class="row"><span>Akkuspannung</span><b id="bV">—</b></div>
<div class="row"><span>USB</span><b id="bUsb">—</b></div>
<div class="row"><span>USB-Spannung</span><b id="bVbus">—</b></div>
<div class="row"><span>Systemspannung</span><b id="bVsys">—</b></div>
<div class="row"><span>Pfad</span><b id="bPath">—</b></div>
<div class="row"><span>Ladestufe</span><b id="bChg">—</b></div>
<div class="row"><span>Ladestrom (Vorgabe)</span><b id="bIchg">—</b></div>
<div class="row"><span>Ladeziel</span><b id="bVt">—</b></div>
<div class="row"><span>Ende-Strom (Vorgabe)</span><b id="bIterm">—</b></div>
<div class="row"><span>USB-Limit</span><b id="bIlim">—</b></div>
<div class="row"><span>PMU-Temperatur</span><b id="bTemp">—</b></div>
<div class="row"><span>Wärmebegrenzung</span><b id="bTh">—</b></div>
<div class="row"><span>Eingang begrenzt</span><b id="bLimHit">—</b></div>
<div class="row"><span>Chip-Warnung unter</span><b id="bWarn">—</b></div>
<div class="row"><span>Chip-Abschaltung unter</span><b id="bOff">—</b></div>
<label class="field">Ladestrom
  <select id="ichg">
    <option value="100">100 mA</option>
    <option value="125">125 mA</option>
    <option value="150">150 mA</option>
    <option value="175">175 mA</option>
    <option value="200">200 mA</option>
    <option value="300" selected>300 mA</option>
    <option value="400">400 mA</option>
    <option value="500">500 mA</option>
    <option value="600">600 mA</option>
    <option value="700">700 mA</option>
    <option value="800">800 mA</option>
    <option value="900">900 mA</option>
    <option value="1000">1000 mA</option>
  </select>
</label>
<button class="pri" id="btnChgSave">Ladestrom speichern</button>
<p class="hint">Geht in die Zelle, nicht in den ESP. Chip-Default 300&nbsp;mA, Maximum 1000&nbsp;mA. USB muss Rahmen plus Laden tragen — der ESP wird davon nicht überlastet. Kein gemessener Strom, keine mAh — Prozent ist Schätzung. Ende-Strom ist Vorgabe des Chips, nicht der Ist-Wert.</p>
<p class="status" id="chgStatus"></p>
</div>
<div class="panel">
<h2>ntfy</h2>
<label class="field">Thema oder URL (leer = aus)
  <input id="ntfyTopic" type="text" autocomplete="off" spellcheck="false" placeholder="z. B. tintenklecks-akku"/>
</label>
<label class="field">Priorität
  <select id="ntfyPrio">
    <option value="min">Min</option>
    <option value="low">Niedrig</option>
    <option value="default">Normal</option>
    <option value="high" selected>Hoch</option>
    <option value="urgent">Dringend</option>
  </select>
</label>
<button class="pri" id="btnNtfySave">Speichern</button>
<button id="btnNtfyTest">Probe senden</button>
<p class="hint">Gleiches Thema, zwei Nachrichten. Warnung: „Akku … %“, unter 10&nbsp;%, einmal am Tag. Aufwachen: „Aufgewacht, Akku … %“ (Timer, KEY, BOOT), stumm. App: ntfy, Thema abonnieren.</p>
<p class="status" id="ntfyStatus"></p>
</div>
<div class="panel">
<h2>Ton</h2>
<label class="field">Lautstärke Hinweise <span id="volLbl">80</span>
  <input id="vol" type="range" min="0" max="100" value="80"/>
</label>
<button class="pri" id="btnVolSave">Lautstärke speichern</button>
<p class="hint">Default 80. Willkommen, WLAN, AP, Neustart.</p>
<p class="status" id="volStatus"></p>
</div>
<div class="panel">
<h2>Anzeige</h2>
<label class="field">Rahmenlage
  <select id="hang">
    <option value="portrait" selected>Hochkant</option>
    <option value="landscape">Quer</option>
  </select>
</label>
<button class="pri" id="btnHangSave">Lage speichern</button>
<p class="hint">Gilt für neue Bilder in Studio, Live und am Panel. Vorhandene Bilder bleiben unverändert.</p>
<p class="status" id="hangStatus"></p>
<button class="pri" id="btnBlank">Panel leeren (weiß)</button>
<button id="btnBattWarn">Akkuwarnung testen</button>
<p class="status">Leert das E-Paper. Bilder erneut über Galerie „Anzeigen“. Test: „Akku &lt; 10 %“ unten rechts auf dem aktuellen Bild, unabhängig vom echten Stand.</p>
</div>
<div class="panel">
<h2>Sicherung</h2>
<p class="hint">Bilder und Töne als eine Datei vom Rahmen. <code>.txt</code> geht in Chrome über HTTP. <code>.zip</code> ebenfalls in einem Rutsch — Chrome kann Zip über HTTP sperren, dann Firefox/Edge oder die <code>.txt</code>. USB stecken. Wiederherstellen von <code>.txt</code> und unkomprimiertem <code>.zip</code> in einem Rutsch. Firmware bleibt das GitHub-Release.</p>
<a class="pri" id="btnBackup" href="/api/backup">Sicherung als .txt</a>
<a class="sec" id="btnBackupZip" href="/api/backup-zip">Sicherung als .zip</a>
<label class="field">Wiederherstellen
  <input id="restoreZip" type="file" accept=".txt,.tkbak,.zip,text/plain,application/octet-stream,application/zip"/>
</label>
<button id="btnRestore">Sicherung auf den Rahmen spielen</button>
<p class="hint">Gleiche Namen werden überschrieben, andere Dateien bleiben. Danach baut die Galerie neu auf.</p>
<p class="status" id="backupStatus"></p>
</div>
<div class="panel">
<h2>Wartung</h2>
<button id="btnReboot">Neustart</button>
<button id="btnOrphans">Jetzt aufräumen</button>
<p class="status">Löschreste werden automatisch beim Start und nach jedem Löschen entfernt — der Knopf erzwingt es nur sofort.</p>
<button class="danger" id="btnWifi">WLAN-Daten löschen &amp; Neustart</button>
<p class="status" id="status"></p>
</div>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
let uiBusy=false;
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
async function refreshStatus(){
  if(window.__zzz) return;
  try{
    const s=await (await fetch('/api/status')).json();
    document.getElementById('dev').textContent=s.device||'—';
    document.getElementById('ip').textContent=s.ip||'—';
    document.getElementById('mode').textContent=s.ap?'AP':'STA';
    document.getElementById('sd').textContent=s.sd?'OK':'fehlt';
    document.getElementById('heap').textContent=(s.heap||0)+' B';
    const ntp=document.getElementById('ntp');
    if(ntp){
      if(s.ntp==='hold') ntp.textContent='Pause nach Handzeit, noch '+Math.max(1,Math.ceil((s.ntpHoldSec||0)/60))+' min';
      else if(s.ntp==='ok'){
        const at=(s.ntpAt||'').replace('T',' ');
        ntp.textContent='ja'+(at.length>=16?', '+at.slice(11,16):'');
      }
      else if(s.ntp==='fail') ntp.textContent='fehlgeschlagen, RTC gilt';
      else if(s.ntp==='ap') ntp.textContent='— (Access Point)';
      else ntp.textContent='—';
    }
    (function(){
      const set=function(id,t){ const e=document.getElementById(id); if(e) e.textContent=t; };
      const b=s.batt;
      if(!b || !b.ok){
        set('bOk','fehlt');
        ['bCell','bPct','bV','bUsb','bVbus','bVsys','bPath','bChg','bIchg','bVt','bIterm','bIlim','bTemp','bTh','bLimHit','bWarn','bOff'].forEach(function(id){ set(id,'—'); });
        return;
      }
      set('bOk','AXP2101');
      set('bCell', b.cell?'ja':'nein');
      set('bPct', (typeof b.pct==='number' && b.pct>=0)? (b.pct+' %') : '—');
      set('bV', (b.cell && b.v)? (b.v+' V') : '—');
      set('bUsb', b.usb?'ja':'nein');
      set('bVbus', b.usb && b.vbus? (b.vbus+' V') : '—');
      set('bVsys', b.vsys? (b.vsys+' V') : '—');
      set('bPath', b.path||'—');
      set('bChg', b.chg||'—');
      set('bIchg', (typeof b.ichg==='number' && b.ichg>=0)? (b.ichg+' mA') : '—');
      set('bVt', (typeof b.vtarget==='number' && b.vtarget>0)? ((b.vtarget/1000).toFixed(2)+' V') : '—');
      set('bIterm', (typeof b.iterm==='number' && b.iterm>=0)? (b.iterm+' mA') : '—');
      set('bIlim', (typeof b.ilim==='number' && b.ilim>=0)? (b.ilim+' mA') : '—');
      set('bTemp', (typeof b.temp==='number')? (b.temp+' °C') : '—');
      set('bTh', b.thermal?'ja':'nein');
      set('bLimHit', b.limHit?'ja':'nein');
      set('bWarn', (typeof b.warnPct==='number')? (b.warnPct+' %') : '—');
      set('bOff', (typeof b.offPct==='number')? (b.offPct+' %') : '—');
    })();
    const bat=document.getElementById('bat');
    if(s.usb) bat.textContent='USB-Betrieb';
    else if(s.battery<0) bat.textContent='Akku —';
    else bat.textContent='Akku '+s.battery+'%'+(s.charging?' · lädt':'');
    const z=document.getElementById('btnZzz');
    if(z) z.hidden=!!s.usb;
    document.getElementById('foot').textContent=s.copyright||'© 2026 Ingo Lissors';
    const hang=document.getElementById('hang');
    if(hang && (s.hang==='portrait'||s.hang==='landscape') && document.activeElement!==hang) hang.value=s.hang;
    const vol=document.getElementById('vol');
    if(vol && typeof s.vol==='number' && document.activeElement!==vol){
      vol.value=String(s.vol);
      const lb=document.getElementById('volLbl');
      if(lb) lb.textContent=String(s.vol);
    }
    const ichg=document.getElementById('ichg');
    if(ichg && s.batt && typeof s.batt.ichg==='number' && s.batt.ichg>0 && document.activeElement!==ichg){
      ichg.value=String(s.batt.ichg);
    }
  }catch(e){ document.getElementById('status').textContent=String(e); }
  try{
    const n=await (await fetch('/api/ntfy')).json();
    const el=document.getElementById('ntfyTopic');
    if(el && document.activeElement!==el) el.value=n.topic||'';
    const pr=document.getElementById('ntfyPrio');
    if(pr && n.prio) pr.value=n.prio;
  }catch(e){}
}
document.getElementById('btnZzz').onclick=()=>{
  if(window.__zzz) return;
  window.__zzz=1;
  (async()=>{
    for(let i=0;i<8;i++){
      try{
        const r=await fetch('/api/sleep',{method:'POST',cache:'no-store'});
        const t=await r.text();
        if(r.ok && t.indexOf('USB')<0) return;
        if(t.indexOf('USB')>=0) break;
      }catch(e){}
      await new Promise(res=>setTimeout(res,300));
    }
    window.__zzz=0;
  })();
};
document.getElementById('btnHangSave').onclick=async()=>{
  if(!setBusy(true,'Lage…')) return;
  try{
    const hang=document.getElementById('hang').value;
    const r=await fetch('/api/hang',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'hang='+encodeURIComponent(hang)});
    document.getElementById('hangStatus').textContent=r.ok?'gespeichert':await r.text();
  }catch(e){ document.getElementById('hangStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('vol').oninput=function(){
  const lb=document.getElementById('volLbl');
  if(lb) lb.textContent=this.value;
};
document.getElementById('btnVolSave').onclick=async()=>{
  if(!setBusy(true,'Lautstärke…')) return;
  try{
    const vol=document.getElementById('vol').value;
    const r=await fetch('/api/volume',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'vol='+encodeURIComponent(vol)});
    document.getElementById('volStatus').textContent=r.ok?'gespeichert':await r.text();
  }catch(e){ document.getElementById('volStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnChgSave').onclick=async()=>{
  if(!setBusy(true,'Ladestrom…')) return;
  try{
    const ichg=document.getElementById('ichg').value;
    const r=await fetch('/api/charge',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'ichg='+encodeURIComponent(ichg)});
    document.getElementById('chgStatus').textContent=r.ok?'gespeichert':await r.text();
    if(r.ok) await refreshStatus();
  }catch(e){ document.getElementById('chgStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnNtfySave').onclick=async()=>{
  if(!setBusy(true,'ntfy…')) return;
  try{
    const topic=document.getElementById('ntfyTopic').value.trim();
    const prio=document.getElementById('ntfyPrio').value;
    const r=await fetch('/api/ntfy',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'topic='+encodeURIComponent(topic)+'&prio='+encodeURIComponent(prio)});
    document.getElementById('ntfyStatus').textContent=r.ok?'gespeichert':await r.text();
  }catch(e){ document.getElementById('ntfyStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnNtfyTest').onclick=async()=>{
  if(!setBusy(true,'ntfy…')) return;
  try{
    const topic=document.getElementById('ntfyTopic').value.trim();
    const prio=document.getElementById('ntfyPrio').value;
    const s=await fetch('/api/ntfy',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'topic='+encodeURIComponent(topic)+'&prio='+encodeURIComponent(prio)});
    if(!s.ok){ document.getElementById('ntfyStatus').textContent=await s.text(); return; }
    const r=await fetch('/api/ntfy-test',{method:'POST'});
    document.getElementById('ntfyStatus').textContent=r.ok?'Probe gesendet':await r.text();
  }catch(e){ document.getElementById('ntfyStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnBlank').onclick=async()=>{
  if(!setBusy(true,'Panel…')) return;
  try{
    const r=await fetch('/api/display',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'blank=1'});
    document.getElementById('status').textContent=await r.text();
  }finally{ setBusy(false); }
};
document.getElementById('btnBattWarn').onclick=async()=>{
  if(!setBusy(true,'Warnung…')) return;
  try{
    const r=await fetch('/api/batt-warn',{method:'POST'});
    document.getElementById('status').textContent=await r.text();
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnReboot').onclick=async()=>{
  if(uiBusy) return;
  if(!confirm('Gerät neu starten?')) return;
  if(!setBusy(true,'Neustart…')) return;
  await fetch('/api/reboot',{method:'POST'});
};
document.getElementById('btnOrphans').onclick=async()=>{
  if(uiBusy) return;
  if(!confirm('Reste gelöschter Bilder sofort entfernen? Löscht JPG/PNG/_src/_thumb/JSON/WAV in /pic, zu denen keine .bmp mehr existiert.')) return;
  if(!setBusy(true,'Aufräumen…')) return;
  try{
    const r=await fetch('/api/cleanup-orphans',{method:'POST'});
    document.getElementById('status').textContent=await r.text();
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnWifi').onclick=async()=>{
  if(uiBusy) return;
  if(!confirm('Gespeicherte WLAN-Daten löschen und neu starten?')) return;
  if(!setBusy(true,'Lösche WLAN…')) return;
  await fetch('/api/wifi-clear',{method:'POST'});
};
function concat(parts){
  let n=0;
  for(const p of parts) n+=p.length;
  const o=new Uint8Array(n); let at=0;
  for(const p of parts){ o.set(p,at); at+=p.length; }
  return o;
}
function unwrapBackup(u8){
  if(u8.length>=6 && u8[0]===0x54 && u8[1]===0x4b && u8[2]===0x42 && u8[3]===0x41 && u8[4]===0x4b && u8[5]===0x31){
    return u8.subarray(6);
  }
  return u8;
}
function u16at(u8,i){ return u8[i]| (u8[i+1]<<8); }
function u32at(u8,i){ return (u8[i]|(u8[i+1]<<8)|(u8[i+2]<<16)|(u8[i+3]<<24))>>>0; }
function findEocd(u8){
  const min=Math.max(0, u8.length-22-65535);
  for(let i=u8.length-22;i>=min;i--){
    if(u32at(u8,i)===0x06054b50) return i;
  }
  return -1;
}
async function inflateRaw(u8){
  const ds=new DecompressionStream('deflate-raw');
  const w=ds.writable.getWriter();
  await w.write(u8); await w.close();
  const r=ds.readable.getReader();
  const parts=[];
  for(;;){ const {value,done}=await r.read(); if(done) break; parts.push(value); }
  return concat(parts);
}
function backupRel(name){
  name=String(name||'').replace(/\\/g,'/');
  const m=name.match(/(?:^|\/)(pic|sound)\/([^/]+)$/i);
  if(!m) return '';
  return m[1].toLowerCase()+'/'+m[2];
}
function parseTkbak2(u8){
  const dv=new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
  let i=6;
  const out=[];
  while(i+8<=u8.length){
    const nl=dv.getUint32(i,true); i+=4;
    if(nl<1||nl>80||i+nl+4>u8.length) throw new Error('Sicherung beschädigt');
    const name=new TextDecoder().decode(u8.subarray(i,i+nl)); i+=nl;
    const sz=dv.getUint32(i,true); i+=4;
    if(i+sz>u8.length) throw new Error('Sicherung unvollständig: '+name);
    const rel=backupRel(name);
    const data=u8.slice(i,i+sz); i+=sz;
    if(rel) out.push({name:rel, data});
  }
  return out;
}
async function unzipBackup(u8){
  u8=unwrapBackup(u8);
  const eocd=findEocd(u8);
  if(eocd<0) throw new Error('kein Zip');
  const n=u16at(u8,eocd+10);
  let off=u32at(u8,eocd+16);
  const out=[];
  for(let i=0;i<n;i++){
    if(u32at(u8,off)!==0x02014b50) throw new Error('Zip beschädigt');
    const method=u16at(u8,off+10);
    const comp=u32at(u8,off+20);
    const uncomp=u32at(u8,off+24);
    const nl=u16at(u8,off+28), xl=u16at(u8,off+30), cl=u16at(u8,off+32);
    const local=u32at(u8,off+42);
    const name=new TextDecoder().decode(u8.slice(off+46, off+46+nl));
    off+=46+nl+xl+cl;
    if(name.endsWith('/')) continue;
    const rel=backupRel(name);
    if(!rel) continue;
    const lnl=u16at(u8,local+26), lxl=u16at(u8,local+28);
    const data=u8.slice(local+30+lnl+lxl, local+30+lnl+lxl+comp);
    let raw=data;
    if(method===8) raw=await inflateRaw(data);
    else if(method!==0) throw new Error('Zip-Methode '+method+' nicht lesbar');
    if(uncomp && raw.length!==uncomp) throw new Error('Zip-Größe passt nicht: '+rel);
    out.push({name:rel, data:raw});
  }
  return out;
}
async function readBackup(u8){
  if(u8.length>=6 && u8[0]===0x54 && u8[1]===0x4b && u8[2]===0x42 && u8[3]===0x41 && u8[4]===0x4b && u8[5]===0x32){
    return parseTkbak2(u8);
  }
  return unzipBackup(u8);
}
document.getElementById('btnBackup').onclick=()=>{
  const st=document.getElementById('backupStatus');
  if(st) st.textContent='Download läuft… USB stecken lassen.';
};
document.getElementById('btnBackupZip').onclick=()=>{
  const st=document.getElementById('backupStatus');
  if(st) st.textContent='Zip-Download… USB stecken. Chrome über HTTP kann Zip sperren — dann .txt oder anderen Browser.';
};
document.getElementById('btnRestore').onclick=async()=>{
  const inp=document.getElementById('restoreZip');
  const file=inp && inp.files && inp.files[0];
  if(!file){ document.getElementById('backupStatus').textContent='Bitte Sicherung wählen'; return; }
  if(!setBusy(true,'Wiederherstellen…')) return;
  const st=document.getElementById('backupStatus');
  try{
    const mag=new Uint8Array(await file.slice(0,6).arrayBuffer());
    const isTk=mag.length>=6 && mag[0]===0x54 && mag[1]===0x4b && mag[2]===0x42 && mag[3]===0x41 && mag[4]===0x4b && mag[5]===0x32;
    const isZip=mag.length>=4 && mag[0]===0x50 && mag[1]===0x4b && (mag[2]===0x03||mag[2]===0x05) && (mag[3]===0x04||mag[3]===0x06);
    if(isTk||isZip){
      try{
      const text=await new Promise(function(resolve,reject){
        const fd=new FormData();
        fd.append('file', file, file.name|| (isZip?'sicherung.zip':'sicherung.txt'));
        const x=new XMLHttpRequest();
        x.timeout=0;
        x.upload.onprogress=function(e){
          if(!e.lengthComputable) return;
          const pct=Math.floor(e.loaded*100/e.total);
          st.textContent='Sende '+pct+' % · '+(e.loaded/1048576).toFixed(0)+' MB';
          const m=document.getElementById('busyMsg');
          if(m) m.textContent='Wiederherstellen '+pct+' %';
        };
        x.onload=function(){
          if(x.status>=200 && x.status<300) resolve(x.responseText);
          else reject(new Error(x.responseText||('Fehler '+x.status)));
        };
        x.onerror=function(){ reject(new Error('Netzwerkfehler')); };
        x.open('POST','/api/restore');
        x.send(fd);
      });
      st.textContent=text||'fertig';
      return;
      }catch(e){
        if(isTk) throw e;
        st.textContent='Zip komprimiert, Datei für Datei…';
      }
    }
    const entries=await readBackup(new Uint8Array(await file.arrayBuffer()));
    if(!entries.length){ st.textContent='In der Datei keine pic/ oder sound/ Dateien'; return; }
    const msg=document.getElementById('busyMsg');
    for(let i=0;i<entries.length;i++){
      if(msg) msg.textContent='Wiederherstellen '+(i+1)+'/'+entries.length+'…';
      st.textContent='Schreibe '+entries[i].name+' ('+(i+1)+'/'+entries.length+')';
      const fd=new FormData();
      fd.append('file', new Blob([entries[i].data]), entries[i].name.split('/').pop());
      const r=await fetch('/api/restore-file?path='+encodeURIComponent(entries[i].name), {method:'POST', body:fd});
      if(!r.ok) throw new Error(entries[i].name+': '+await r.text());
    }
    await fetch('/api/list-rebuild',{method:'POST'}).catch(()=>{});
    st.textContent=entries.length+' Dateien geschrieben';
  }catch(e){ st.textContent=String(e); }
  finally{ setBusy(false); }
};
refreshStatus(); setInterval(refreshStatus,10000);
</script>
</body></html>
)HTML";

#include "html_frame.h"
#include "html_live.h"
#include "html_studio.h"
