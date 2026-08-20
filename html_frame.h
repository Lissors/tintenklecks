#pragma once

static const char PAGE_FRAME[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
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
.brand a{color:var(--txt);text-decoration:none;font-weight:700}.nav a{color:var(--acc);margin-left:.75rem;font-size:.85rem;text-decoration:none}
.bat{font-size:.8rem;color:var(--dim)}
.head-right{display:flex;align-items:center;gap:.5rem}
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;flex:none}
main{max-width:520px;margin:0 auto;padding:1.25rem 1rem}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:1rem;margin-bottom:1rem}
h1{font-size:1.2rem;margin:0 0 .35rem}h2{font-size:.72rem;letter-spacing:.1em;text-transform:uppercase;color:var(--dim);margin:0 0 .75rem}
.lead{color:var(--dim);font-size:.88rem;line-height:1.45;margin:0 0 1rem}
label.field{display:block;margin:.55rem 0;font-size:.8rem;color:var(--dim)}
label.field select,label.field input{width:100%;margin-top:.3rem;box-sizing:border-box;padding:.55rem .65rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit}
button{width:100%;margin-top:.55rem;padding:.7rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit;font-weight:700;cursor:pointer}
button.pri{background:var(--acc);color:#1a1612;border:0}
.status{font-size:.85rem;color:var(--dim);min-height:1.2em;margin-top:.65rem;line-height:1.4}
.hint{font-size:.78rem;color:var(--dim);line-height:1.4;margin:.5rem 0 0}
footer{text-align:center;padding:1.5rem;font-size:.75rem;color:var(--dim)}
#dailyBox,#intervalBox{display:none}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu">Tintenklecks</a></div>
  <div class="head-right"><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<h1>Rahmeneinstellung</h1>
<p class="lead">Automatischer Wechsel. Erinnerungen (am Tag selbst, morgen oder übermorgen): ein Bild voll. Mehrere am selben Tag: eine zufällig, Hinweis unten rechts, KEY und alle 3&nbsp;Stunden die nächste — KEY nach der Runde Zufall. KEY und Jetzt wechseln sonst nur Zufall. Sonst Zufall ohne Datum, ohne Zurücklegen. USB: bleibt wach. Akku, ab 10&nbsp;Min Intervall oder 1×/Tag: ohne Client direkt nach Bildwechsel, mit Client nach 60&nbsp;s Inaktivität Deep Sleep — Aufwachen beim nächsten Wechsel, per KEY (wechseln, wieder schlafen) oder per BOOT (Web, kein Wechsel). 5&nbsp;Min: bleibt wach.</p>
<div class="panel">
<h2>Uhrzeit (Chip-RTC)</h2>
<label class="field">Datum &amp; Zeit
  <input id="clock" type="datetime-local"/>
</label>
<label class="field">Suchen
  <input id="tzFilter" type="search" placeholder="Stadt…" autocomplete="off"/>
</label>
<label class="field">Stadt
  <select id="tzCity" size="8"></select>
</label>
<p class="hint">Die Stadt setzt die Zeitzone, inklusive Sommer- und Winterzeit.</p>
<button id="btnTz" type="button">Standort speichern</button>
<button id="btnTime" type="button">Uhr setzen</button>
<button id="btnPhone" type="button">Von diesem Gerät übernehmen</button>
<p class="hint" id="clockHint">—</p>
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
function applyFrame(f){
  document.getElementById('mode').value=String(f.mode||0);
  document.getElementById('intervalMin').value=String(f.intervalMin||5);
  const hh=String(f.dailyHour??8).padStart(2,'0');
  const mm=String(f.dailyMin??0).padStart(2,'0');
  document.getElementById('dailyTime').value=hh+':'+mm;
  document.getElementById('hint').textContent=
    'Zeit: '+(f.now||'—')+' · RTC '+(f.rtc?'OK':'fehlt')+' · sync '+(f.timeOk?'ja':'nein')+(f.last?' · zuletzt: '+f.last:'');
  document.getElementById('clockHint').textContent=f.now?('Aktuell: '+f.now.replace('T',' ')):'Keine gültige Chip-Zeit — bitte setzen';
  if(f.now){
    document.getElementById('clock').value=f.now;
  }
  syncUi();
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
async function loadTz(){
  try{
    const sel=document.getElementById('tzCity');
    const [cities,cur]=await Promise.all([
      (await fetch('/api/tz-cities')).json(),
      (await fetch('/api/tz')).json()
    ]);
    cities.sort((a,b)=>a.n.localeCompare(b.n,'de'));
    sel.innerHTML='';
    cities.forEach(c=>{
      const o=document.createElement('option');
      o.value=c.n; o.dataset.posix=c.p; o.textContent=c.n;
      if(c.n===cur.city) o.selected=true;
      sel.appendChild(o);
    });
    if(cur.city) document.getElementById('tzFilter').value=cur.city;
  }catch(e){}
}
document.getElementById('tzFilter').oninput=()=>{
  const q=document.getElementById('tzFilter').value.trim().toLowerCase();
  const sel=document.getElementById('tzCity');
  for(const o of sel.options){ o.hidden=q && o.textContent.toLowerCase().indexOf(q)<0; }
};
document.getElementById('tzCity').onchange=()=>{
  const sel=document.getElementById('tzCity');
  const o=sel.options[sel.selectedIndex];
  if(o) document.getElementById('tzFilter').value=o.textContent;
};
document.getElementById('btnTz').onclick=async()=>{
  const sel=document.getElementById('tzCity');
  const o=sel.options[sel.selectedIndex];
  if(!o) return;
  setBusy(true,'Standort…');
  try{
    const r=await fetch('/api/tz',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'city='+encodeURIComponent(o.value)+'&posix='+encodeURIComponent(o.dataset.posix||'')});
    const t=await r.text();
    document.getElementById('clockHint').textContent=r.ok?'Standort gespeichert · NTP an':'Fehler: '+t;
    if(r.ok) await loadFrame();
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
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
document.getElementById('btnTime').onclick=async()=>{
  const v=document.getElementById('clock').value;
  if(!v){ document.getElementById('status').textContent='Bitte Datum/Zeit wählen'; return; }
  setBusy(true,'Setze Uhr…');
  try{
    const r=await fetch('/api/time',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'iso='+encodeURIComponent(v)});
    const t=await r.text();
    if(!r.ok){ document.getElementById('status').textContent=t; return; }
    document.getElementById('status').textContent='Uhr gesetzt';
    applyFrame(JSON.parse(t));
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
document.getElementById('btnPhone').onclick=async()=>{
  setBusy(true,'Setze Uhr…');
  try{
    const r=await fetch('/api/time',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:'epoch='+Math.floor(Date.now()/1000)});
    const t=await r.text();
    if(!r.ok){ document.getElementById('status').textContent=t; return; }
    document.getElementById('status').textContent='Uhr vom Gerät übernommen (Standort-Zeit)';
    applyFrame(JSON.parse(t));
  }catch(e){ document.getElementById('status').textContent=String(e); }
  finally{ setBusy(false); }
};
refreshStatus(); setInterval(refreshStatus,15000); loadFrame(); loadTz();
</script>
</body></html>
)HTML";
