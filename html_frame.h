#pragma once

static const char PAGE_FRAME[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<link rel="icon" type="image/png" href="/favicon.png"/><link rel="apple-touch-icon" href="/favicon.png"/>
<title>Rahmen · Tintenklecks</title>
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
.brand,.brand a{display:inline-flex;align-items:center;gap:.45rem;color:var(--txt);text-decoration:none;font-weight:700}.brand img{width:1.55rem;height:1.55rem;border-radius:.4rem;flex:none}.nav a{color:var(--acc);margin-left:.75rem;font-size:.85rem;text-decoration:none}
.bat{font-size:.8rem;color:var(--dim)}
.head-right{display:flex;align-items:center;gap:.5rem}
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;flex:none;border-radius:6px;border:1px solid var(--line);background:#1a1612;color:var(--dim);cursor:pointer;font:inherit}
.stay{display:inline-flex;align-items:center;gap:.4rem;margin:0;position:relative;font:inherit;font-size:.78rem;color:var(--dim);cursor:pointer;user-select:none;flex:none;white-space:nowrap}
.stay[hidden]{display:none !important}
.stay input{position:absolute;opacity:0;pointer-events:none}
.stay-sw{width:1.9rem;height:1.05rem;border-radius:999px;background:#1a1612;border:1px solid var(--line);position:relative;flex:none}
.stay-sw:before{content:'';position:absolute;top:1px;left:1px;width:.8rem;height:.8rem;border-radius:50%;background:var(--dim)}
.stay input:checked+.stay-sw{background:var(--acc);border-color:var(--acc)}
.stay input:checked+.stay-sw:before{left:auto;right:1px;background:#1a1612}
main{max-width:520px;margin:0 auto;padding:1.25rem 1rem}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:1rem;margin-bottom:1rem}
h1{font-size:1.2rem;margin:0 0 .35rem}h2{font-size:.72rem;letter-spacing:.1em;text-transform:uppercase;color:var(--dim);margin:0 0 .75rem}
.lead{color:var(--dim);font-size:.88rem;line-height:1.45;margin:0 0 1rem}
label.field{display:block;margin:.55rem 0;font-size:.8rem;color:var(--dim)}
label.field select,label.field input{width:100%;margin-top:.3rem;box-sizing:border-box;padding:.55rem .65rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit}
button{width:100%;margin-top:.55rem;padding:.7rem;border-radius:8px;border:0;background:var(--acc);color:#1a1612;font:inherit;font-weight:700;cursor:pointer}
button.pri{background:var(--acc);color:#1a1612;border:0}
button:not(.zzz):active:not(:disabled){filter:brightness(.78)}
.status{font-size:.85rem;color:var(--dim);min-height:1.2em;margin-top:.65rem;line-height:1.4}
.hint{font-size:.78rem;color:var(--dim);line-height:1.4;margin:.5rem 0 0}
footer{text-align:center;padding:1.5rem;font-size:.75rem;color:var(--dim)}
#dailyBox,#intervalBox{display:none}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu"><img src="/favicon.png" width="28" height="28" alt=""/>Tintenklecks</a></div>
  <div class="head-right"><label class="stay" id="btnWach"><input type="checkbox" id="chkWach"/><span class="stay-sw"></span>Wach bleiben</label><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<h1>Rahmeneinstellung</h1>
<p class="lead">Automatischer Wechsel. Erinnerungen (am Tag selbst, morgen oder übermorgen): ein Bild voll. Mehrere am selben Tag: eine zufällig, Pfeil unten rechts, KEY und alle 3&nbsp;Stunden die nächste — KEY nach der Runde Zufall. KEY und Jetzt wechseln sonst nur Zufall. Sonst Zufall ohne Datum, ohne Zurücklegen. USB: bleibt wach. Akku, ab 10&nbsp;Min Intervall oder 1×/Tag: ohne Client direkt nach Bildwechsel, mit Client nach 60&nbsp;s Inaktivität Deep Sleep — Aufwachen beim nächsten Wechsel, per KEY (wechseln, wieder schlafen) oder per BOOT (Web, kein Wechsel). 5&nbsp;Min: bleibt wach.</p>
<div class="panel">
<p class="hint" id="memNext">—</p>
<p class="hint" id="potLine">—</p>
<button id="btnPot" type="button">Alle Bilder in die Auswahl</button>
</div>
<div class="panel">
<h2>Wechsel</h2>
<label class="field">Modus
  <select id="mode">
    <option value="0">Aus</option>
    <option value="1">Im Intervall</option>
    <option value="2">1× pro Tag</option>
  </select>
</label>
<label class="field" id="intervalBox">Alle …
  <select id="intervalMin">
    <option value="5" selected>5 Minuten</option>
    <option value="10">10 Minuten</option>
    <option value="30">30 Minuten</option>
    <option value="60">60 Minuten</option>
  </select>
</label>
<div id="dailyBox">
  <label class="field">Uhrzeit
    <input id="dailyTime" type="time" value="08:00"/>
  </label>
</div>
<button class="pri" id="btnSave" type="button">Speichern</button>
<button id="btnNow" type="button">Jetzt wechseln</button>
<p class="status" id="status"></p>
<p class="hint" id="hint">—</p>
</div>
<div class="panel">
<h2>Anzeige</h2>
<label class="field">Rahmenlage
  <select id="hang">
    <option value="portrait" selected>Hochkant</option>
    <option value="landscape">Quer</option>
  </select>
</label>
<button class="pri" id="btnHangSave" type="button">Lage speichern</button>
<p class="hint">Gilt für neue Bilder in Studio, Live und am Panel. Vorhandene Bilder bleiben unverändert.</p>
<p class="status" id="hangStatus"></p>
</div>
<div class="panel">
<h2>Index</h2>
<p class="hint">Scannt die SD neu und sortiert alle Bilder nach Namen. Speichern, Löschen und Umbenennen ändern die Liste sofort — dieser Knopf nur, wenn Reihenfolge oder Index nicht stimmen.</p>
<button id="btnRebuild" type="button">Index neu aufbauen</button>
</div>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
let busyDepth=0;
function setBusy(on,msg){
  if(on){ busyDepth++; if(msg){ const m=document.getElementById('busyMsg'); if(m) m.textContent=msg; } }
  else busyDepth=Math.max(0,busyDepth-1);
  const onB=busyDepth>0;
  document.body.classList.toggle('busy',onB);
  const ov=document.getElementById('busyOv');
  if(ov) ov.classList.toggle('on',onB);
}
function syncUi(){
  const mode=document.getElementById('mode').value;
  document.getElementById('intervalBox').style.display=mode==='1'?'block':'none';
  document.getElementById('dailyBox').style.display=mode==='2'?'block':'none';
}
document.getElementById('mode').onchange=syncUi;
function memLine(f){
  if(!f.timeOk) return 'Nächste Erinnerung: — (keine Chip-Zeit)';
  const m=f.memoryNext;
  if(!m) return 'Keine Erinnerung in den nächsten zwei Tagen';
  let s='Nächste Erinnerung: '+(m.when||'')+' · '+(m.name||m.file||'');
  if((m.count||0)>1) s+=' · '+(m.count-1)+' weitere';
  return s;
}
function applyPot(f){
  const pot=document.getElementById('potLine');
  if(!pot) return;
  const t=f.potTotal|0;
  if(typeof f.potLeft!=='number' || t<1) pot.textContent='Zufallstopf: —';
  else if(f.potLeft<1) pot.textContent='Zufallstopf: nächster Zug neue Runde ('+t+' Bilder)';
  else pot.textContent='Zufallstopf noch '+f.potLeft+' von '+t;
}
function applyFrame(f){
  document.getElementById('mode').value=String(f.mode||0);
  document.getElementById('intervalMin').value=String(f.intervalMin||5);
  const hh=String(f.dailyHour??8).padStart(2,'0');
  const mm=String(f.dailyMin??0).padStart(2,'0');
  document.getElementById('dailyTime').value=hh+':'+mm;
  document.getElementById('hint').textContent=
    'Zeit: '+(f.now||'—')+' · RTC '+(f.rtc?'OK':'fehlt')+' · sync '+(f.timeOk?'ja':'nein')+(f.last?' · zuletzt: '+f.last:'');
  const mem=document.getElementById('memNext');
  if(mem) mem.textContent=memLine(f);
  applyPot(f);
  syncUi();
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
    const w=document.getElementById('btnWach');
    const c=document.getElementById('chkWach');
    if(w) w.hidden=!!s.usb;
    if(c) c.checked=!!s.stayAwake;
    document.getElementById('foot').textContent=s.copyright||'© 2026 Ingo Lissors';
    const hang=document.getElementById('hang');
    if(hang && (s.hang==='portrait'||s.hang==='landscape') && document.activeElement!==hang) hang.value=s.hang;
    const mem=document.getElementById('memNext');
    if(mem && ('memoryNext' in s || 'timeOk' in s)) mem.textContent=memLine(s);
    if(typeof s.potLeft==='number' || typeof s.potTotal==='number') applyPot(s);
  }catch(e){}
}
document.getElementById('chkWach').onchange=()=>{
  if(window.__zzz){ const c=document.getElementById('chkWach'); if(c) c.checked=!c.checked; return; }
  const on=document.getElementById('chkWach').checked;
  fetch('/api/stayawake',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'on='+(on?'1':'0')})
    .catch(function(){});
};
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
async function loadFrame(){
  setBusy(true,'Laden…');
  try{
    const f=await (await fetch('/api/frame')).json();
    applyFrame(f);
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
}
document.getElementById('btnSave').onclick=async()=>{
  setBusy(true,'Speichern…');
  try{
    const mode=document.getElementById('mode').value;
    const intervalMin=document.getElementById('intervalMin').value;
    const t=document.getElementById('dailyTime').value||'08:00';
    const parts=t.split(':');
    const body='mode='+encodeURIComponent(mode)+'&intervalMin='+encodeURIComponent(intervalMin)+'&dailyHour='+encodeURIComponent(parts[0]||'8')+'&dailyMin='+encodeURIComponent(parts[1]||'0');
    const r=await fetch('/api/frame',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body});
    const f=await r.json();
    document.getElementById('status').textContent=r.ok?'Gespeichert.':'Fehler';
    applyFrame(f);
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnNow').onclick=async()=>{
  setBusy(true,'Wechsle Bild…');
  try{
    const r=await fetch('/api/frame-now',{method:'POST'});
    document.getElementById('status').textContent=await r.text();
    await loadFrame();
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnHangSave').onclick=async()=>{
  setBusy(true,'Lage…');
  try{
    const hang=document.getElementById('hang').value;
    const r=await fetch('/api/hang',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'hang='+encodeURIComponent(hang)});
    document.getElementById('hangStatus').textContent=r.ok?'gespeichert':await r.text();
  }catch(e){ document.getElementById('hangStatus').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnPot').onclick=async()=>{
  setBusy(true,'Topf…');
  try{
    const f=await (await fetch('/api/frame-pot',{method:'POST'})).json();
    applyFrame(f);
    document.getElementById('status').textContent='Alle Zufallsbilder wieder in der Auswahl';
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnRebuild').onclick=async()=>{
  setBusy(true,'Index…');
  try{
    const r=await fetch('/api/list-rebuild',{method:'POST'});
    const t=await r.text();
    if(!r.ok){ document.getElementById('status').textContent=t||'Fehler'; return; }
    let n=0, done=false;
    for(let i=0;i<90;i++){
      await new Promise(res=>setTimeout(res, i===0?300:2000));
      const s=await (await fetch('/api/status')).json();
      if(s.listBuilding) continue;
      const items=await (await fetch('/api/list')).json();
      n=Array.isArray(items)?items.length:0;
      done=true;
      break;
    }
    document.getElementById('status').textContent=done?('Index neu aufgebaut · '+n+' Bilder'):'Index wird noch gebaut';
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
refreshStatus(); setInterval(refreshStatus,15000); loadFrame();
</script>
</body></html>
)HTML";
