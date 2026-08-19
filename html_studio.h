#pragma once

static const char PAGE_STUDIO[] PROGMEM = R"HTML(
<!DOCTYPE html><html lang="de"><head>
<meta charset="utf-8"/><meta name="viewport" content="width=device-width,initial-scale=1"/>
<title>Studio · Tintenklecks</title>
<link rel="preconnect" href="https://fonts.googleapis.com"/>
<link href="https://fonts.googleapis.com/css2?family=Dancing+Script:wght@400;700&display=swap" rel="stylesheet"/>
<style>
:root{--bg:#14110f;--panel:#241e18;--line:#4a3f35;--acc:#c4966e;--txt:#f3ebe3;--dim:#a89888}
*{box-sizing:border-box}
body{margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--txt)}
body.busy,body.busy *{cursor:wait !important}
body.busy button{pointer-events:none;opacity:.55}
.busy-ov{position:fixed;inset:0;background:rgba(10,8,6,.55);z-index:200;display:none;align-items:center;justify-content:center;flex-direction:column;gap:.75rem}
.busy-ov.on{display:flex}
.hour{width:44px;height:44px;border:3px solid var(--acc);border-top-color:transparent;border-radius:50%;animation:spin .75s linear infinite}
@keyframes spin{to{transform:rotate(360deg)}}
.busy-msg{color:var(--txt);font-size:.9rem}
.top{display:flex;justify-content:space-between;align-items:center;padding:.75rem 1rem;border-bottom:1px solid var(--line);gap:.75rem;flex-wrap:wrap}
.brand a{color:var(--txt);text-decoration:none;font-weight:700}.nav a{color:var(--acc);margin-left:.7rem;font-size:.82rem;text-decoration:none}
.bat{font-size:.78rem;color:var(--dim)}
.head-right{display:flex;align-items:center;gap:.5rem}
button.zzz{width:auto;margin:0;padding:.2rem .5rem;font-size:.78rem;letter-spacing:.08em;font-weight:700;flex:none;border-radius:6px;border:1px solid var(--line);background:#1a1612;color:var(--dim);cursor:pointer;font:inherit}
main{display:grid;grid-template-columns:minmax(260px,1fr) 360px;gap:1rem;padding:1rem;max-width:1280px;margin:0 auto}
@media(max-width:980px){main{grid-template-columns:1fr}}
.panel{background:var(--panel);border:1px solid var(--line);border-radius:12px;padding:1rem}
.stage-wrap{display:flex;gap:.75rem;flex-wrap:wrap;justify-content:center}
.stage{position:relative;background:#0c0a09;border-radius:10px;overflow:hidden;max-height:68vh;touch-action:none;outline:2px solid #3d6ea8}
.stage.land{aspect-ratio:800/480;width:min(100%,720px)}
.stage.port{aspect-ratio:480/800;width:min(100%,360px)}
canvas{width:100%;height:100%;display:block}
.drop{border:1px dashed var(--line);border-radius:10px;padding:1rem;text-align:center;cursor:pointer;color:var(--dim);margin-bottom:.75rem}
.drop.drag{border-color:var(--acc);background:#3a2e24;color:var(--txt)}
h2{margin:0 0 .55rem;font-size:.72rem;letter-spacing:.1em;text-transform:uppercase;color:var(--dim)}
label.row{display:flex;justify-content:space-between;align-items:center;gap:.5rem;margin:.45rem 0;font-size:.84rem;color:var(--dim)}
label.row input[type=range],label.row select{width:52%}
label.row .val{flex:none;width:2.4rem;text-align:right;color:var(--txt);font-variant-numeric:tabular-nums}
label.field{display:block;margin:.45rem 0;font-size:.78rem;color:var(--dim)}
label.field input,label.field select,label.field textarea{width:100%;margin-top:.25rem;box-sizing:border-box;padding:.5rem .6rem;border-radius:8px;border:1px solid var(--line);background:#1a1612;color:var(--txt);font:inherit}
label.field textarea{min-height:5.5rem;resize:vertical;line-height:1.4}
.lab-list{margin:.45rem 0 .6rem;display:flex;flex-direction:column;gap:.35rem}
.lab-list .lab-item{display:flex;align-items:center;gap:.4rem;padding:.4rem .5rem;border:1px solid var(--line);border-radius:8px;background:#1a1612;font-size:.78rem}
.lab-list .lab-item.active{border-color:var(--acc)}
.lab-list .lab-item span{flex:1;min-width:0;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;color:var(--dim);cursor:pointer}
.lab-list .lab-item button{flex:none;padding:.25rem .45rem;border-radius:6px;border:1px solid var(--line);background:transparent;color:var(--txt);cursor:pointer;font:inherit;font-size:.72rem}
.lab-list .lab-item button:hover{border-color:#c45;color:#f88}
.lab-empty{font-size:.75rem;color:var(--dim);margin:.35rem 0}
button.primary,button.ghost{width:100%;margin-top:.45rem;padding:.7rem;border-radius:8px;border:0;cursor:pointer;font-weight:700;font:inherit}
button.primary{background:var(--acc);color:#1a1612}
button.ghost{background:transparent;border:1px solid var(--line);color:var(--txt)}
.status{font-size:.8rem;color:var(--dim);min-height:1.2em;margin-top:.55rem;line-height:1.35}
footer{text-align:center;padding:1rem;font-size:.75rem;color:var(--dim)}
</style></head><body>
<div class="busy-ov" id="busyOv"><div class="hour"></div><div class="busy-msg" id="busyMsg">Bitte warten…</div></div>
<div class="top">
  <div class="brand"><a href="/menu">Tintenklecks</a></div>
  <div class="head-right"><button type="button" class="zzz" id="btnZzz">zzz</button><div class="bat" id="bat">Akku …</div></div>
</div>
<main>
<section class="panel">
  <div class="stage-wrap">
    <div><h2>Zuschnitt</h2><div id="stage" class="stage port"><canvas id="crop"></canvas></div></div>
    <div><h2>E6 Vorschau</h2><div id="stageOut" class="stage port"><canvas id="out"></canvas></div></div>
  </div>
</section>
<aside class="panel">
  <div class="drop" id="drop"><span id="dropTitle">Foto hierher · JPG/PNG/BMP</span><br/><small id="dropSub">oder tippen zum Auswählen</small></div>
  <input id="file" type="file" accept="image/*" hidden/>
  <button class="ghost" id="btnNewPic" type="button" hidden>Bearbeitung beenden · neues Bild</button>
  <button class="ghost" id="btnRawE6" type="button">Gerendertes BMP an E6</button>
  <input id="fileRawE6" type="file" accept=".bmp,image/bmp" hidden/>
  <h2>Verfahren</h2>
  <label class="row">Methode
    <select id="paintMode">
      <option value="sierra2" selected>Sierra</option>
      <option value="saved">Lab</option>
    </select>
  </label>
  <button class="primary" id="btnSierra2" type="button">In E6 konvertieren</button>
  <button class="primary" id="btnTuneStyle" type="button" hidden>Automatik</button>
  <h2>Format</h2>
  <label class="row">Ausrichtung
    <select id="orient"><option value="portrait" selected>Portrait 480×800</option><option value="landscape">Landscape 800×480</option></select>
  </label>
  <label class="row">Zoom <input id="zoom" type="range" min="10" max="400" value="100"/><span class="val" data-for="zoom">100</span></label>
  <button class="primary" id="btnShow">Anzeigen am Rahmen</button>
  <button class="primary" id="btnSave">Speichern in Galerie</button>
  <p class="status" id="status">Anzeigen = nur Panel. Speichern = Galerie.</p>
  <h2>Person</h2>
  <label class="field">Name <input id="metaName" type="text" maxlength="80" placeholder="Name"/></label>
  <label class="field">Geburtsdatum <input id="metaBirth" type="text" maxlength="32" placeholder="TT.MM.JJJJ"/></label>
  <label class="field">Sterbedatum <input id="metaDeath" type="text" maxlength="32" placeholder="TT.MM.JJJJ"/></label>
  <label class="row">Name auf Bild <input id="showName" type="checkbox" checked/></label>
  <label class="row">Geburt auf Bild <input id="showBirth" type="checkbox" checked/></label>
  <label class="row">Tod auf Bild <input id="showDeath" type="checkbox" checked/></label>
  <label class="row">Größe Name <input id="sizeName" type="range" min="16" max="80" value="36"/></label>
  <label class="row">Größe Daten <input id="sizeDates" type="range" min="12" max="56" value="22"/></label>
  <h2>Bildbeschreibung</h2>
  <label class="field">Text (Live-Anzeige)
    <textarea id="metaDesc" maxlength="2000" placeholder="Kurzbeschreibung des Bildes, z. B. Personen, Ort, Anlass…"></textarea>
  </label>
  <h2>Freier Text</h2>
  <label class="row">Beschriftung anzeigen <input id="capVisible" type="checkbox" checked/></label>
  <label class="field">Text <input id="labText" type="text" maxlength="120" placeholder="z. B. In Erinnerung"/></label>
  <label class="field">Schrift
    <select id="labFont">
      <option value="serif">Serif</option>
      <option value="sans-serif">Sans</option>
      <option value="script">Schnörkel</option>
    </select>
  </label>
  <label class="row">Fett <input id="labBold" type="checkbox"/></label>
  <label class="row">Größe Freitext <input id="labSize" type="range" min="14" max="72" value="28"/></label>
  <label class="row">Drehung <input id="labRot" type="range" min="-180" max="180" value="0" step="1"/><input id="labRotNum" type="number" min="-180" max="180" value="0" style="width:4.2rem;margin-left:.35rem"/><span style="margin-left:.2rem">°</span></label>
  <label class="field">Farbe
    <select id="labColor">
      <option value="#FFFFFF">Weiß</option>
      <option value="#000000">Schwarz</option>
      <option value="#FFFF00">Gelb</option>
      <option value="#FF0000">Rot</option>
      <option value="#0000FF">Blau</option>
      <option value="#00FF00">Grün</option>
    </select>
  </label>
  <label class="field">Ausrichtung
    <select id="labAlign"><option value="center">Mitte</option><option value="left">Links</option><option value="right">Rechts</option></select>
  </label>
  <button class="ghost" id="btnAddLab" type="button">Freitext hinzufügen</button>
  <button class="ghost" id="btnDelLab" type="button">Ausgewählten Text löschen</button>
  <button class="ghost" id="btnClearLab" type="button">Alle Freitexte löschen</button>
  <div class="lab-list" id="labList"><p class="lab-empty">Keine Labels</p></div>
  <div id="blockPrep" hidden>
  <h2>Vorbereitung</h2>
  <label class="row">Belichtung <input id="prepExp" type="range" min="0.5" max="2" step="0.05" value="1"/><span class="val" data-for="prepExp">1.00</span></label>
  <label class="row">Sättigung <input id="prepSat" type="range" min="0.5" max="2" step="0.05" value="1"/><span class="val" data-for="prepSat">1.00</span></label>
  <label class="row">S-Kurve <input id="prepScurve" type="range" min="0" max="1" step="0.05" value="0"/><span class="val" data-for="prepScurve">0.00</span></label>
  <label class="row">Lichter stauchen <input id="prepHi" type="range" min="0.5" max="5" step="0.1" value="1.5"/><span class="val" data-for="prepHi">1.50</span></label>
  <label class="row">Schatten <input id="prepShadow" type="range" min="0" max="1" step="0.05" value="0"/><span class="val" data-for="prepShadow">0.00</span></label>
  <label class="row">Mitte <input id="prepMid" type="range" min="0.3" max="0.7" step="0.05" value="0.5"/><span class="val" data-for="prepMid">0.50</span></label>
  <label class="row">Tonumfang ins Papier <input id="prepCdr" type="checkbox" checked/></label>
  <h2>E-Paper / Dither</h2>
  <label class="row">Helligkeit <input id="bright" type="range" min="0" max="100" value="48"/><span class="val" data-for="bright">48</span></label>
  <label class="row">Kontrast <input id="contrast" type="range" min="0" max="100" value="62"/><span class="val" data-for="contrast">62</span></label>
  <label class="row">Wärme <input id="warmth" type="range" min="0" max="100" value="46"/><span class="val" data-for="warmth">46</span></label>
  <label class="row">Dither % <input id="dither" type="range" min="0" max="100" value="100"/><span class="val" data-for="dither">100</span></label>
  <label class="row">Algorithmus
    <select id="algo">
      <option value="atkinson" selected>Atkinson</option>
      <option value="floyd">Floyd–Steinberg</option>
      <option value="stucki">Stucki</option>
      <option value="none">Ohne</option>
    </select>
  </label>
  </div>
</aside>
</main>
<footer id="foot">© 2026 Ingo Lissors</footer>
<script>
const MEASURED=[
  {m:[2,2,2],d:[0,0,0]},{m:[190,200,200],d:[255,255,255]},{m:[205,202,0],d:[255,255,0]},
  {m:[135,19,0],d:[255,0,0]},{m:[5,64,158],d:[0,0,255]},{m:[39,102,60],d:[0,255,0]}
];
const STYLE_DEF={bright:48,contrast:62,warmth:46,dither:100,algo:'atkinson'};
const SPEC6=[
  {m:[34,34,38],d:[0,0,0]},
  {m:[227,223,214],d:[255,255,255]},
  {m:[46,104,77],d:[0,255,0]},
  {m:[37,60,110],d:[0,0,255]},
  {m:[158,42,43],d:[255,0,0]},
  {m:[232,185,49],d:[255,255,0]}
];
let styleId='portrait', img=null, panX=0.5, panY=0.5, drag=false, lx=0, ly=0, t=null;
let busy=false;
let labels=[], dragLab=-1, selectedLab=-1, labDragMode=false, lastDither=null, lastCrop=null, uiBusy=false, busyDepth=0;
let lastDitherSig='';
let loadedPicSig='';
let pictureDirty=false;
let editName=null;
let srcOriginal=null;
let srcNeedsRewrite=false;
let sandboxRawFile=null;
let sierra2Done=false;
let sierra2Sig='';
let specLut=null;
let imgSeq=0, tunedSig=null, tunePromise=null, tuning=false;
let appliedSeq=-1;
const crop=document.getElementById('crop'), out=document.getElementById('out');
const cctx=crop.getContext('2d',{willReadFrequently:true});
const octx=out.getContext('2d',{willReadFrequently:true});
const stage=document.getElementById('stage'), stageOut=document.getElementById('stageOut');
const status=document.getElementById('status');
const drop=document.getElementById('drop'), file=document.getElementById('file');
const dropTitle=document.getElementById('dropTitle'), dropSub=document.getElementById('dropSub'), btnNewPic=document.getElementById('btnNewPic');
const orient=document.getElementById('orient'), zoom=document.getElementById('zoom');
const bright=document.getElementById('bright'), contrast=document.getElementById('contrast');
const warmth=document.getElementById('warmth'), dither=document.getElementById('dither'), algo=document.getElementById('algo');
const prepExp=document.getElementById('prepExp'), prepSat=document.getElementById('prepSat');
const prepScurve=document.getElementById('prepScurve'), prepHi=document.getElementById('prepHi');
const prepShadow=document.getElementById('prepShadow'), prepMid=document.getElementById('prepMid');
const prepCdr=document.getElementById('prepCdr');
const paintMode=document.getElementById('paintMode');
const capVisible=document.getElementById('capVisible');
const metaName=document.getElementById('metaName'), metaBirth=document.getElementById('metaBirth'), metaDeath=document.getElementById('metaDeath');
const metaDesc=document.getElementById('metaDesc');
const showName=document.getElementById('showName'), showBirth=document.getElementById('showBirth'), showDeath=document.getElementById('showDeath');
const sizeName=document.getElementById('sizeName'), sizeDates=document.getElementById('sizeDates');

function clamp(v,a,b){return v<a?a:v>b?b:v}
function rgbToXyz(r,g,b){
  r/=255; g/=255; b/=255;
  r=r>0.04045?Math.pow((r+0.055)/1.055,2.4):r/12.92;
  g=g>0.04045?Math.pow((g+0.055)/1.055,2.4):g/12.92;
  b=b>0.04045?Math.pow((b+0.055)/1.055,2.4):b/12.92;
  return [100*(r*0.4124564+g*0.3575761+b*0.1804375),
          100*(r*0.2126729+g*0.7151522+b*0.072175),
          100*(r*0.0193339+g*0.119192+b*0.9503041)];
}
function xyzToLab(x,y,z){
  x/=95.047; y/=100; z/=108.883;
  const f=function(t){return t>0.008856?Math.pow(t,1/3):7.787*t+16/116;};
  x=f(x); y=f(y); z=f(z);
  return [116*y-16, 500*(x-y), 200*(y-z)];
}
function rgbToLab(r,g,b){ const xyz=rgbToXyz(r,g,b); return xyzToLab(xyz[0],xyz[1],xyz[2]); }
function labToXyz(L,a,b){
  let y=(L+16)/116, x=a/500+y, z=y-b/200;
  const inv=function(t){return t>0.206897?t*t*t:(t-16/116)/7.787;};
  return [inv(x)*95.047, inv(y)*100, inv(z)*108.883];
}
function xyzToRgb(x,y,z){
  x/=100; y/=100; z/=100;
  let r=x*3.2404542+y*-1.5371385+z*-0.4985314;
  let g=x*-0.969266+y*1.8760108+z*0.041556;
  let b=x*0.0556434+y*-0.2040259+z*1.0572252;
  const enc=function(c){ c=c>0.0031308?1.055*Math.pow(c,1/2.4)-0.055:12.92*c; return clamp(Math.round(c*255),0,255); };
  return [enc(r),enc(g),enc(b)];
}
function labToRgb(L,a,b){ const xyz=labToXyz(L,a,b); return xyzToRgb(xyz[0],xyz[1],xyz[2]); }
let panelLab=null;
function panelRange(){
  if(panelLab) return panelLab;
  const bl=rgbToLab(MEASURED[0].m[0],MEASURED[0].m[1],MEASURED[0].m[2])[0];
  const wl=rgbToLab(MEASURED[1].m[0],MEASURED[1].m[1],MEASURED[1].m[2])[0];
  panelLab={bl:bl, span:wl-bl};
  return panelLab;
}
function applyExposure(data, exposure){
  if(exposure===1) return;
  for(let i=0;i<data.length;i+=4){
    data[i]=Math.min(255, Math.round(data[i]*exposure));
    data[i+1]=Math.min(255, Math.round(data[i+1]*exposure));
    data[i+2]=Math.min(255, Math.round(data[i+2]*exposure));
  }
}
function applySatPrep(data, saturation){
  if(saturation===1) return;
  for(let i=0;i<data.length;i+=4){
    const r=data[i], g=data[i+1], b=data[i+2];
    const max=Math.max(r,g,b)/255, min=Math.min(r,g,b)/255, l=(max+min)/2;
    if(max===min) continue;
    const d=max-min;
    const s=l>0.5?d/(2-max-min):d/(max+min);
    let h;
    if(max===r/255) h=((g/255-b/255)/d+(g<b?6:0))/6;
    else if(max===g/255) h=((b/255-r/255)/d+2)/6;
    else h=((r/255-g/255)/d+4)/6;
    const newS=clamp(s*saturation,0,1);
    const c=(1-Math.abs(2*l-1))*newS;
    const x=c*(1-Math.abs(((h*6)%2)-1));
    const m=l-c/2;
    const sec=Math.floor(h*6);
    let rp,gp,bp;
    if(sec===0){rp=c;gp=x;bp=0;}
    else if(sec===1){rp=x;gp=c;bp=0;}
    else if(sec===2){rp=0;gp=c;bp=x;}
    else if(sec===3){rp=0;gp=x;bp=c;}
    else if(sec===4){rp=x;gp=0;bp=c;}
    else {rp=c;gp=0;bp=x;}
    data[i]=Math.round((rp+m)*255);
    data[i+1]=Math.round((gp+m)*255);
    data[i+2]=Math.round((bp+m)*255);
  }
}
function applyScurveTonemap(data, strength, shadowBoost, highlightCompress, midpoint){
  if(strength===0) return;
  for(let i=0;i<data.length;i+=4){
    for(let c=0;c<3;c++){
      const n=data[i+c]/255;
      let result;
      if(n<=midpoint){
        result=Math.pow(n/midpoint, 1-strength*shadowBoost)*midpoint;
      } else {
        result=midpoint+Math.pow((n-midpoint)/(1-midpoint), 1+strength*highlightCompress)*(1-midpoint);
      }
      data[i+c]=Math.round(clamp(result,0,1)*255);
    }
  }
}
function applyCdr(data){
  const rng=panelRange(), bl=rng.bl, span=rng.span;
  for(let i=0;i<data.length;i+=4){
    const lab=rgbToLab(data[i],data[i+1],data[i+2]);
    const rgb=labToRgb(bl+(lab[0]/100)*span, lab[1], lab[2]);
    data[i]=rgb[0]; data[i+1]=rgb[1]; data[i+2]=rgb[2];
  }
}
function applyInternetPrep(data){
  applyExposure(data, +prepExp.value);
  applySatPrep(data, +prepSat.value);
  applyScurveTonemap(data, +prepScurve.value, +prepShadow.value, +prepHi.value, +prepMid.value);
  if(prepCdr.checked) applyCdr(data);
}
function target(){return orient.value==='portrait'?[480,800]:[800,480]}
function syncStage(noRender){
  const [w,h]=target(); crop.width=out.width=w; crop.height=out.height=h;
  const port=w<h;
  stage.classList.toggle('port',port); stage.classList.toggle('land',!port);
  stageOut.classList.toggle('port',port); stageOut.classList.toggle('land',!port);
  if(!noRender) schedule();
}
function coverBase(){ if(!img) return 1; const [tw,th]=target(); return Math.max(tw/img.width, th/img.height); }
function containBase(){ if(!img) return 1; const [tw,th]=target(); return Math.min(tw/img.width, th/img.height); }
function zoomFitPct(){
  const cov=coverBase(), con=containBase();
  if(!cov) return 100;
  return clamp(Math.round(100*con/cov), 10, 100);
}
function drawCrop(){
  const [tw,th]=target(); cctx.fillStyle='#111'; cctx.fillRect(0,0,tw,th);
  if(!img) return;
  const z=coverBase()*(+zoom.value/100), dw=img.width*z, dh=img.height*z;
  cctx.imageSmoothingEnabled=true;
  cctx.drawImage(img,(tw-dw)*panX,(th-dh)*panY,dw,dh);
}
function applyStylePixels(data){
  const br=((+bright.value)-50)/50, ct=0.5+(+contrast.value)/100, warm=((+warmth.value)-50)/50;
  const sat=1.05;
  for(let i=0;i<data.length;i+=4){
    let r=data[i],g=data[i+1],b=data[i+2];
    let y=0.299*r+0.587*g+0.114*b;
    const y2=y+(255-y)*Math.max(0,br)+y*Math.min(0,br);
    r+=y2-y; g+=y2-y; b+=y2-y;
    r=(r-128)*ct+128; g=(g-128)*ct+128; b=(b-128)*ct+128;
    r+=warm*18; b-=warm*18;
    y=0.299*r+0.587*g+0.114*b;
    r=y+(r-y)*sat; g=y+(g-y)*sat; b=y+(b-y)*sat;
    data[i]=clamp(r,0,255); data[i+1]=clamp(g,0,255); data[i+2]=clamp(b,0,255);
  }
}
function isRose(r,g,b){
  const mx=Math.max(r,g,b), mn=Math.min(r,g,b), chroma=mx-mn;
  if(chroma<12 || chroma>120) return false;
  const y=0.299*r+0.587*g+0.114*b;
  if(y<22 || y>232) return false;
  if(r<g+4 || r<b) return false;
  if(g>b+42) return false;
  return true;
}
function protectRose(data){
  for(let i=0;i<data.length;i+=4){
    const r=data[i], g=data[i+1], b=data[i+2];
    if(!isRose(r,g,b)) continue;
    if(b<=g-8) continue;
    const k=Math.max(0, b-g)*0.75 + Math.max(0, 8-(g-b))*0.15;
    data[i]=clamp(r+k*0.15,0,255);
    data[i+2]=clamp(b-k,0,255);
  }
}
let inkLab=null;
function ensureInkLab(){
  if(!inkLab) inkLab=MEASURED.map(function(m){ return rgbToLab(m.m[0],m.m[1],m.m[2]); });
  return inkLab;
}
function luma(r,g,b){ return 0.299*r+0.587*g+0.114*b; }
function isFlatBlack(r,g,b){
  if(luma(r,g,b)>=38) return false;
  return (Math.max(r,g,b)-Math.min(r,g,b))<14;
}
function rgbHue(r,g,b){
  const mx=Math.max(r,g,b), mn=Math.min(r,g,b), df=mx-mn;
  if(df<1) return 0;
  let h;
  if(mx===r) h=((g-b)/df+(g<b?6:0));
  else if(mx===g) h=(b-r)/df+2;
  else h=(r-g)/df+4;
  return h*60;
}
function inkMask(r,g,b){
  if(isFlatBlack(r,g,b)) return [true,false,false,false,false,false];
  const mask=[true,true,false,false,false,false];
  const mx=Math.max(r,g,b), mn=Math.min(r,g,b);
  if(mx-mn<14) return mask;
  const h=rgbHue(r,g,b);
  for(let i=2;i<6;i++){
    const m=MEASURED[i].m, hi=rgbHue(m[0],m[1],m[2]);
    let dh=Math.abs(h-hi); if(dh>180) dh=360-dh;
    if(dh<=48) mask[i]=true;
  }
  return mask;
}
function projectToInks(r,g,b, mask){
  let n=0, i;
  for(i=0;i<6;i++) if(mask[i]) n++;
  if(!n) return [r,g,b];
  const w=new Float32Array(6);
  for(i=0;i<6;i++) if(mask[i]) w[i]=1/n;
  for(let it=0;it<40;it++){
    const p=[0,0,0];
    for(i=0;i<6;i++){
      if(w[i]<=0) continue;
      const m=MEASURED[i].m;
      p[0]+=w[i]*m[0]; p[1]+=w[i]*m[1]; p[2]+=w[i]*m[2];
    }
    let sum=0;
    for(i=0;i<6;i++){
      if(!mask[i]){ w[i]=0; continue; }
      const m=MEASURED[i].m;
      const gdt=(m[0]*(p[0]-r)+m[1]*(p[1]-g)+m[2]*(p[2]-b))/65025;
      w[i]=Math.max(0, w[i]-0.18*gdt);
      sum+=w[i];
    }
    if(sum<1e-8){
      for(i=0;i<6;i++) if(mask[i]){ w[i]=1/n; sum+=w[i]; }
    }
    for(i=0;i<6;i++) w[i]/=sum;
  }
  const o=[0,0,0];
  for(i=0;i<6;i++){
    if(w[i]<=0) continue;
    const m=MEASURED[i].m;
    o[0]+=w[i]*m[0]; o[1]+=w[i]*m[1]; o[2]+=w[i]*m[2];
  }
  return o;
}
function flattenColorRegions(imgData){
  const w=imgData.width, h=imgData.height, d=imgData.data, np=w*h;
  const lab=new Float32Array(np*3);
  for(let p=0;p<np;p++){
    const j=p*4, L=rgbToLab(d[j],d[j+1],d[j+2]), o=p*3;
    lab[o]=L[0]; lab[o+1]=L[1]; lab[o+2]=L[2];
  }
  const labels=new Int32Array(np);
  for(let i=0;i<np;i++) labels[i]=-1;
  const qx=new Int32Array(np), qy=new Int32Array(np);
  const DE2=5*5, nbs=[[1,0],[-1,0],[0,1],[0,-1]];
  let nreg=0;
  const sums=[];
  for(let y=0;y<h;y++){
    for(let x=0;x<w;x++){
      const s=y*w+x;
      if(labels[s]>=0) continue;
      const p0=s*3, L0=lab[p0], a0=lab[p0+1], b0=lab[p0+2];
      let head=0, tail=0;
      qx[0]=x; qy[0]=y; tail=1;
      labels[s]=nreg;
      let sr=0, sg=0, sb=0, cnt=0;
      while(head<tail){
        const cx=qx[head], cy=qy[head++];
        const j=(cy*w+cx)*4;
        sr+=d[j]; sg+=d[j+1]; sb+=d[j+2]; cnt++;
        for(let k=0;k<4;k++){
          const nx=cx+nbs[k][0], ny=cy+nbs[k][1];
          if(nx<0||ny<0||nx>=w||ny>=h) continue;
          const npx=ny*w+nx;
          if(labels[npx]>=0) continue;
          const lp=npx*3, dL=lab[lp]-L0, da=lab[lp+1]-a0, db=lab[lp+2]-b0;
          if(dL*dL+da*da+db*db>DE2) continue;
          labels[npx]=nreg;
          qx[tail]=nx; qy[tail]=ny; tail++;
        }
      }
      sums.push({r:sr,g:sg,b:sb,n:cnt});
      nreg++;
    }
  }
  const adj=new Array(nreg);
  for(let i=0;i<nreg;i++) adj[i]={};
  for(let y=0;y<h;y++){
    for(let x=0;x<w;x++){
      const a=labels[y*w+x];
      if(x+1<w){
        const b=labels[y*w+x+1];
        if(a!==b){ adj[a][b]=1; adj[b][a]=1; }
      }
      if(y+1<h){
        const b=labels[(y+1)*w+x];
        if(a!==b){ adj[a][b]=1; adj[b][a]=1; }
      }
    }
  }
  const MIN=4, remap=new Int32Array(nreg);
  for(let i=0;i<nreg;i++) remap[i]=i;
  for(let i=0;i<nreg;i++){
    if(sums[i].n>=MIN) continue;
    const keys=Object.keys(adj[i]);
    if(!keys.length) continue;
    const Li=rgbToLab(sums[i].r/sums[i].n, sums[i].g/sums[i].n, sums[i].b/sums[i].n);
    let best=-1, bestD=1e18;
    for(let k=0;k<keys.length;k++){
      const j=+keys[k];
      const Lj=rgbToLab(sums[j].r/sums[j].n, sums[j].g/sums[j].n, sums[j].b/sums[j].n);
      const dd=(Li[0]-Lj[0])*(Li[0]-Lj[0])+(Li[1]-Lj[1])*(Li[1]-Lj[1])+(Li[2]-Lj[2])*(Li[2]-Lj[2]);
      if(dd<bestD){ bestD=dd; best=j; }
    }
    if(best>=0) remap[i]=best;
  }
  for(let i=0;i<nreg;i++){
    let j=i, guard=0;
    while(remap[j]!==j && guard++<nreg) j=remap[j];
    remap[i]=j;
  }
  const col=new Array(nreg);
  const acc=new Float32Array(nreg*3), cnt=new Int32Array(nreg);
  for(let p=0;p<np;p++){
    const r=remap[labels[p]];
    labels[p]=r;
    const j=p*4;
    acc[r*3]+=d[j]; acc[r*3+1]+=d[j+1]; acc[r*3+2]+=d[j+2];
    cnt[r]++;
  }
  for(let i=0;i<nreg;i++){
    if(!cnt[i]) continue;
    const mr=acc[i*3]/cnt[i], mg=acc[i*3+1]/cnt[i], mb=acc[i*3+2]/cnt[i];
    col[i]=projectToInks(mr,mg,mb, inkMask(mr,mg,mb));
  }
  for(let p=0;p<np;p++){
    const c=col[labels[p]], j=p*4;
    d[j]=c[0]; d[j+1]=c[1]; d[j+2]=c[2];
  }
  return labels;
}
function nearest(r,g,b, mask){
  const lab=rgbToLab(clamp(r,0,255),clamp(g,0,255),clamp(b,0,255));
  const P=ensureInkLab();
  let best=1e18, idx=1, any=false;
  for(let i=0;i<P.length;i++){
    if(mask && !mask[i]) continue;
    any=true;
    const dL=lab[0]-P[i][0], da=lab[1]-P[i][1], db=lab[2]-P[i][2], dist=dL*dL+da*da+db*db;
    if(dist<best){best=dist; idx=i;}
  }
  if(!any) return nearest(r,g,b, null);
  return idx;
}
function ditherDiffuse(imgData, strength, kernel, serpentine, regions){
  const w=imgData.width,h=imgData.height,d=imgData.data,f=strength/100;
  const err=new Float32Array(w*h*3);
  for(let y=0;y<h;y++){
    const L=!serpentine||y%2===0;
    for(let xi=0;xi<w;xi++){
      const x=L?xi:w-1-xi, i=(y*w+x)*4, e=(y*w+x)*3, rid=regions?regions[y*w+x]:-1;
      const sr=d[i], sg=d[i+1], sb=d[i+2];
      if(isFlatBlack(sr,sg,sb)){
        d[i]=0; d[i+1]=0; d[i+2]=0;
        continue;
      }
      const mask=regions?inkMask(sr,sg,sb):null;
      const wr=clamp(sr+err[e],0,255), wg=clamp(sg+err[e+1],0,255), wb=clamp(sb+err[e+2],0,255);
      const ni=nearest(wr,wg,wb, mask), m=MEASURED[ni];
      d[i]=m.d[0]; d[i+1]=m.d[1]; d[i+2]=m.d[2];
      const er=(wr-m.m[0])*f, eg=(wg-m.m[1])*f, eb=(wb-m.m[2])*f;
      for(let k=0;k<kernel.length;k++){
        const dx=kernel[k][0], dy=kernel[k][1], fr=kernel[k][2];
        const xx=x+(L?dx:-dx), yy=y+dy;
        if(xx<0||xx>=w||yy<0||yy>=h) continue;
        if(regions && regions[yy*w+xx]!==rid) continue;
        const q=(yy*w+xx)*3;
        err[q]+=er*fr; err[q+1]+=eg*fr; err[q+2]+=eb*fr;
      }
    }
  }
}
function ditherFS(imgData, strength, regions){
  ditherDiffuse(imgData, strength, [[1,0,7/16],[-1,1,3/16],[0,1,5/16],[1,1,1/16]], true, regions);
}
function ditherAtkinson(imgData, strength, regions){
  const q=1/8;
  ditherDiffuse(imgData, strength, [[1,0,q],[2,0,q],[-1,1,q],[0,1,q],[1,1,q],[0,2,q]], false, regions);
}
function ditherStucki(imgData, strength, regions){
  ditherDiffuse(imgData, strength, [[1,0,8/42],[2,0,4/42],[-2,1,2/42],[-1,1,4/42],[0,1,8/42],[1,1,4/42],[2,1,2/42],[-2,2,1/42],[-1,2,2/42],[0,2,4/42],[1,2,2/42],[2,2,1/42]], true, regions);
}
function posterOnly(imgData){
  const d=imgData.data, abmalen=paintMode.value==='abmalen';
  for(let i=0;i<d.length;i+=4){
    if(isFlatBlack(d[i],d[i+1],d[i+2])){ d[i]=0; d[i+1]=0; d[i+2]=0; continue; }
    const mask=abmalen?inkMask(d[i],d[i+1],d[i+2]):null;
    const m=MEASURED[nearest(d[i],d[i+1],d[i+2], mask)];
    d[i]=m.d[0]; d[i+1]=m.d[1]; d[i+2]=m.d[2];
  }
}
function hexToRgb(hex){
  const h=hex.replace('#','');
  return [parseInt(h.slice(0,2),16),parseInt(h.slice(2,4),16),parseInt(h.slice(4,6),16)];
}
function fontCss(font){
  if(font==='sans-serif') return 'system-ui,sans-serif';
  if(font==='script') return '"Dancing Script","Segoe Script","Brush Script MT","Apple Chancery",cursive';
  return 'Georgia,"Times New Roman",serif';
}
function labelFont(L){
  const weight=L.bold?'700 ':'400 ';
  return weight+(L.size||28)+'px '+fontCss(L.font);
}
function measureLabelBox(ctx,L,w,h){
  ctx.font=labelFont(L);
  ctx.textAlign=L.align||'center';
  ctx.textBaseline='middle';
  const text=L.text||'';
  const tw=Math.max(8, ctx.measureText(text).width);
  const th=(L.size||28)*1.25;
  const cx=(L.x||0.5)*w, cy=(L.y||0.9)*h;
  let left=cx;
  if((L.align||'center')==='center') left=cx-tw/2;
  else if((L.align||'center')==='right') left=cx-tw;
  return {cx:cx, cy:cy, x:left-8, y:cy-th/2-6, w:tw+16, h:th+12};
}
function drawLabels(ctx,w,h,withFrames){
  if(!capVisible.checked || !labels.length) return;
  for(let i=0;i<labels.length;i++){
    const L=labels[i];
    const rgb=hexToRgb(L.color||'#FFFFFF');
    const m=MEASURED[nearest(rgb[0],rgb[1],rgb[2])].d;
    const px=(L.x||0.5)*w, py=(L.y||0.9)*h;
    const rot=((L.rotate||0)*Math.PI)/180;
    ctx.save();
    ctx.translate(px, py);
    ctx.rotate(rot);
    ctx.font=labelFont(L);
    ctx.textAlign=L.align||'center';
    ctx.textBaseline='middle';
    if(withFrames){
      const box=measureLabelBox(ctx,L,w,h);
      const lx=box.x-box.cx, ly=box.y-box.cy;
      ctx.strokeStyle=(i===selectedLab || i===dragLab)?'#c4966e':'#ffffff';
      ctx.lineWidth=2;
      ctx.setLineDash([6,4]);
      ctx.strokeRect(lx, ly, box.w, box.h);
      ctx.setLineDash([]);
    }
    ctx.fillStyle='rgb('+m[0]+','+m[1]+','+m[2]+')';
    const text=L.text||'';
    if(L.bold){
      ctx.strokeStyle=ctx.fillStyle;
      ctx.lineJoin='round';
      ctx.lineWidth=Math.max(1.2, (L.size||28)*0.07);
      ctx.strokeText(text, 0, 0);
    }
    ctx.fillText(text, 0, 0);
    ctx.restore();
  }
}
function roleTitle(role){
  if(role==='name') return 'Name';
  if(role==='birth') return 'Geburt';
  if(role==='death') return 'Tod';
  return 'Text';
}
function selectLab(i){
  selectedLab=(i>=0 && i<labels.length)?i:-1;
  if(selectedLab>=0){
    const L=labels[selectedLab];
    if(!L.role || L.role==='custom') document.getElementById('labText').value=L.text||'';
    document.getElementById('labSize').value=L.size||28;
    document.getElementById('labFont').value=L.font||'serif';
    document.getElementById('labColor').value=L.color||'#FFFFFF';
    document.getElementById('labAlign').value=L.align||'center';
    document.getElementById('labBold').checked=!!L.bold;
    document.getElementById('labRot').value=L.rotate||0;
    document.getElementById('labRotNum').value=L.rotate||0;
  }
  updateLabList();
  schedule();
}
function deleteLabAt(i){
  if(i<0 || i>=labels.length) return;
  const L=labels[i];
  if(L.role==='name') showName.checked=false;
  if(L.role==='birth') showBirth.checked=false;
  if(L.role==='death') showDeath.checked=false;
  labels.splice(i,1);
  if(selectedLab===i) selectedLab=-1;
  else if(selectedLab>i) selectedLab--;
  updateLabList();
  schedule();
}
function updateLabList(){
  const el=document.getElementById('labList');
  if(!labels.length){
    el.innerHTML='<p class="lab-empty">Keine Labels</p>';
    return;
  }
  el.innerHTML='';
  labels.forEach(function(L,i){
    const row=document.createElement('div');
    row.className='lab-item'+(i===selectedLab?' active':'');
    const span=document.createElement('span');
    span.title='Auswählen';
    span.textContent=roleTitle(L.role)+': '+(L.text||'(leer)')+(L.bold?' · Fett':'')+((L.rotate||0)?' · '+L.rotate+'°':'');
    span.onclick=function(){ if(!uiBusy) selectLab(i); };
    const del=document.createElement('button');
    del.type='button'; del.textContent='Löschen';
    del.onclick=function(e){ e.stopPropagation(); if(!uiBusy) deleteLabAt(i); };
    row.appendChild(span); row.appendChild(del);
    el.appendChild(row);
  });
}
function upsertRoleLabel(role, enabled, text, size, yDefault){
  const idx=labels.findIndex(function(L){return L.role===role;});
  if(!enabled || !text){
    if(idx>=0){
      labels.splice(idx,1);
      if(selectedLab===idx) selectedLab=-1;
      else if(selectedLab>idx) selectedLab--;
    }
    return;
  }
  const font=document.getElementById('labFont').value;
  const color=document.getElementById('labColor').value;
  const align=document.getElementById('labAlign').value;
  const bold=!!document.getElementById('labBold').checked;
  const rotate=+document.getElementById('labRot').value||0;
  if(idx>=0){
    labels[idx].text=text;
    labels[idx].size=size;
    if(selectedLab===idx){
      labels[idx].font=font;
      labels[idx].color=color;
      labels[idx].align=align;
      labels[idx].bold=bold;
      labels[idx].rotate=rotate;
    }
  } else {
    labels.push({role:role, text:text, x:0.5, y:yDefault, size:size, color:color, font:font, align:align, bold:bold, rotate:rotate});
  }
}
function syncPersonLabels(){
  const n=metaName.value.trim();
  const b=metaBirth.value.trim();
  const d=metaDeath.value.trim();
  upsertRoleLabel('name', showName.checked, n, +sizeName.value, 0.80);
  upsertRoleLabel('birth', showBirth.checked, b?('* '+b):'', +sizeDates.value, 0.88);
  upsertRoleLabel('death', showDeath.checked, d?('\u2020 '+d):'', +sizeDates.value, 0.94);
  updateLabList();
}
function buildMeta(){
  return {
    name: metaName.value.trim(),
    birth: metaBirth.value.trim(),
    death: metaDeath.value.trim(),
    description: metaDesc.value.trim(),
    showName: !!showName.checked,
    showBirth: !!showBirth.checked,
    showDeath: !!showDeath.checked,
    sizeName: +sizeName.value,
    sizeDates: +sizeDates.value,
    captionVisible: !!capVisible.checked,
    orient: orient.value,
    styleId: styleId,
    bright: +bright.value,
    contrast: +contrast.value,
    warmth: +warmth.value,
    dither: +dither.value,
    algo: algo.value,
    prepExp: +prepExp.value,
    prepSat: +prepSat.value,
    prepScurve: +prepScurve.value,
    prepHi: +prepHi.value,
    prepShadow: +prepShadow.value,
    prepMid: +prepMid.value,
    prepCdr: !!prepCdr.checked,
    paintMode: paintMode.value,
    zoom: +zoom.value,
    panX: panX,
    panY: panY,
    labels: labels.map(function(L){
      return {role:L.role||'custom',text:L.text,x:L.x,y:L.y,size:L.size,color:L.color,font:L.font,align:L.align,bold:!!L.bold,rotate:+(L.rotate||0)};
    })
  };
}
function specLutIndex(r,g,b){
  const s=64;
  const ri=Math.min(s-1, Math.max(0, (r*(s/256))|0));
  const gi=Math.min(s-1, Math.max(0, (g*(s/256))|0));
  const bi=Math.min(s-1, Math.max(0, (b*(s/256))|0));
  return specLut[(ri*s+gi)*s+bi];
}
async function buildSpecLut(){
  if(specLut) return;
  const s=64, lut=new Uint8Array(s*s*s);
  const palLab=SPEC6.map(function(p){ return rgbToLab(p.m[0],p.m[1],p.m[2]); });
  for(let ri=0;ri<s;ri++){
    if((ri%4)===0) await new Promise(function(r){ setTimeout(r,0); });
    const r=ri*(255/(s-1));
    for(let gi=0;gi<s;gi++){
      const g=gi*(255/(s-1));
      for(let bi=0;bi<s;bi++){
        const b=bi*(255/(s-1));
        const lab=rgbToLab(r,g,b);
        let best=1e18, idx=1;
        for(let k=0;k<6;k++){
          const dL=(lab[0]-palLab[k][0])*0.90;
          const da=lab[1]-palLab[k][1];
          const db=lab[2]-palLab[k][2];
          const dist=dL*dL+da*da+db*db;
          if(dist<best){ best=dist; idx=k; }
        }
        lut[(ri*s+gi)*s+bi]=idx;
      }
    }
  }
  specLut=lut;
}
function mulberry32(a){
  return function(){
    a|=0; a=a+0x6D2B79F5|0;
    let t=Math.imul(a^a>>>15, 1|a);
    t=t+Math.imul(t^t>>>7, 61|t)^t;
    return ((t^t>>>14)>>>0)/4294967296;
  };
}
async function ditherSierra2Spec(imgData, statusEl){
  await buildSpecLut();
  const w=imgData.width, h=imgData.height, d=imgData.data;
  const curr=new Float32Array(w*h*3);
  for(let p=0,i=0;i<d.length;i+=4,p+=3){
    curr[p]=d[i]; curr[p+1]=d[i+1]; curr[p+2]=d[i+2];
  }
  const rng=mulberry32(42);
  const noise=new Float32Array(w*h*3);
  for(let i=0;i<noise.length;i++) noise[i]=rng()*2.4-1.2;
  for(let y=0;y<h;y++){
    if(statusEl && (y%40)===0){
      statusEl.textContent='Sierra · Zeile '+y+'/'+h;
      await new Promise(function(r){ setTimeout(r,0); });
    }
    const rtl=(y%2)===1;
    const dir=rtl?-1:1;
    for(let xi=0;xi<w;xi++){
      const x=rtl?w-1-xi:xi;
      const p=(y*w+x)*3;
      const oldR=clamp(curr[p]+noise[p],0,255);
      const oldG=clamp(curr[p+1]+noise[p+1],0,255);
      const oldB=clamp(curr[p+2]+noise[p+2],0,255);
      const idx=specLutIndex(oldR,oldG,oldB);
      const pal=SPEC6[idx];
      const pix=(y*w+x)*4;
      d[pix]=pal.d[0]; d[pix+1]=pal.d[1]; d[pix+2]=pal.d[2]; d[pix+3]=255;
      let er=oldR-pal.m[0], eg=oldG-pal.m[1], eb=oldB-pal.m[2];
      const lum=0.299*oldR+0.587*oldG+0.114*oldB;
      if(lum>185){ er*=0.93; eg*=0.93; eb*=0.93; }
      const nbrs=rtl
        ?[[x-1,y,4],[x-2,y,3],[x+2,y+1,1],[x+1,y+1,2],[x,y+1,3],[x-1,y+1,2],[x-2,y+1,1]]
        :[[x+1,y,4],[x+2,y,3],[x-2,y+1,1],[x-1,y+1,2],[x,y+1,3],[x+1,y+1,2],[x+2,y+1,1]];
      for(let n=0;n<nbrs.length;n++){
        const nx=nbrs[n][0], ny=nbrs[n][1], wf=nbrs[n][2]/16;
        if(nx<0||nx>=w||ny<0||ny>=h) continue;
        const q=(ny*w+nx)*3;
        curr[q]+=er*wf; curr[q+1]+=eg*wf; curr[q+2]+=eb*wf;
      }
    }
  }
}
async function renderOut(){
  if(!img) return;
  if(sandboxRawFile){
    const wh=target(), w=wh[0], h=wh[1];
    cctx.fillStyle='#111'; cctx.fillRect(0,0,w,h);
    octx.fillStyle='#111'; octx.fillRect(0,0,w,h);
    cctx.imageSmoothingEnabled=false;
    octx.imageSmoothingEnabled=false;
    cctx.drawImage(img,0,0,w,h);
    octx.drawImage(img,0,0,w,h);
    lastCrop=cctx.getImageData(0,0,w,h);
    lastDither=octx.getImageData(0,0,w,h);
    status.textContent='E6-BMP unverändert · '+sandboxRawFile.name;
    return;
  }
  drawCrop();
  const wh=target(), w=wh[0], h=wh[1];
  lastCrop=cctx.getImageData(0,0,w,h);
  if(isSierra2()){
    const sig=cropSig();
    if(sierra2Done && sierra2Sig && sierra2Sig!==sig) sierra2Done=false;
    drawLabels(cctx,w,h,true);
    if(!sierra2Done){
      octx.putImageData(lastCrop,0,0);
      lastDither=null;
      lastDitherSig='';
      status.textContent='Sierra · Zuschnitt bereit — noch nicht konvertiert';
      return;
    }
    if(lastDither && sierra2Sig===sig){
      octx.putImageData(lastDither,0,0);
      drawLabels(octx,w,h,true);
      return;
    }
    const id=new ImageData(new Uint8ClampedArray(lastCrop.data), w, h);
    const msg=document.getElementById('busyMsg');
    await ditherSierra2Spec(id, msg);
    octx.putImageData(id,0,0);
    lastDither=octx.getImageData(0,0,w,h);
    lastDitherSig=picSig();
    drawLabels(octx,w,h,true);
    sierra2Sig=sig;
    status.textContent='Sierra · E6-Zuordnung';
    return;
  }
  if(lastDither && lastDitherSig===picSig()){
    octx.putImageData(lastDither,0,0);
    drawLabels(octx,w,h,true);
    drawLabels(cctx,w,h,true);
    return;
  }
  const id=cctx.getImageData(0,0,w,h);
  applyInternetPrep(id.data);
  applyStylePixels(id.data);
  const abmalen=paintMode.value==='abmalen';
  const regions=abmalen?flattenColorRegions(id):null;
  const strength=+dither.value, a=algo.value;
  if(a==='none'||strength===0) posterOnly(id);
  else if(a==='floyd') ditherFS(id,strength,regions);
  else if(a==='stucki') ditherStucki(id,strength,regions);
  else ditherAtkinson(id,strength,regions);
  octx.putImageData(id,0,0);
  lastDither=octx.getImageData(0,0,w,h);
  lastDitherSig=picSig();
  drawLabels(octx,w,h,true);
  drawLabels(cctx,w,h,true);
  status.textContent='OK · Lab · '+a+' · H'+bright.value+' K'+contrast.value+' W'+warmth.value;
}
function schedule(msg){
  clearTimeout(t);
  t=setTimeout(async function(){
    if(busy) return;
    busy=true;
    if(msg) setBusy(true, msg);
    try{ await renderOut(); } finally{ busy=false; if(msg) setBusy(false); }
  }, 60);
}
function syncSliderVals(){
  Array.prototype.forEach.call(document.querySelectorAll('.val[data-for]'), function(el){
    const src=document.getElementById(el.getAttribute('data-for'));
    if(!src) return;
    el.textContent=(src.step && +src.step<1)?(+src.value).toFixed(2):src.value;
  });
}
function readPrep(){
  return {
    prepExp:+prepExp.value, prepSat:+prepSat.value, prepScurve:+prepScurve.value,
    prepHi:+prepHi.value, prepShadow:+prepShadow.value, prepMid:+prepMid.value,
    prepCdr:!!prepCdr.checked
  };
}
function writePrep(p){
  if(!p) return;
  if(p.prepExp!=null) prepExp.value=p.prepExp;
  if(p.prepSat!=null) prepSat.value=p.prepSat;
  if(p.prepScurve!=null) prepScurve.value=p.prepScurve;
  if(p.prepHi!=null) prepHi.value=p.prepHi;
  if(p.prepShadow!=null) prepShadow.value=p.prepShadow;
  if(p.prepMid!=null) prepMid.value=p.prepMid;
  if(typeof p.prepCdr==='boolean') prepCdr.checked=p.prepCdr;
  syncSliderVals();
}
function applyPrepDefaults(){
  writePrep({prepExp:1, prepSat:1, prepScurve:0, prepHi:1.5, prepShadow:0, prepMid:0.5, prepCdr:true});
}
function applyStyleDefaults(st){
  if(!bright) return;
  st=st||STYLE_DEF;
  bright.value=st.bright; contrast.value=st.contrast; warmth.value=st.warmth;
  dither.value=st.dither; algo.value=st.algo;
  syncSliderVals();
}
function isSierra2(){ return paintMode.value==='sierra2'; }
function syncVerfahrenUi(){
  const s2=isSierra2();
  const saved=paintMode.value==='saved';
  const prep=document.getElementById('blockPrep');
  const btnS=document.getElementById('btnSierra2');
  const btnT=document.getElementById('btnTuneStyle');
  if(prep) prep.hidden=s2;
  if(btnS) btnS.hidden=!s2;
  if(btnT) btnT.hidden=!saved;
}
function syncEditUi(){
  dropTitle.textContent=editName?('Bearbeitung: '+editName):(isSierra2()?'Foto hierher · dann In E6 konvertieren':'Foto hierher · JPG/PNG/BMP');
  dropSub.textContent=editName?'Kein neues Foto — erst „Bearbeitung beenden“':(isSierra2()?'Sierra, kein Automatik-Start':'oder tippen zum Auswählen');
  btnNewPic.hidden=!editName;
}
function readBmpWh(file){
  return file.slice(0, 26).arrayBuffer().then(function(buf){
    const v=new DataView(buf);
    if(v.byteLength<26 || v.getUint8(0)!==0x42 || v.getUint8(1)!==0x4D) return null;
    return {w:Math.abs(v.getInt32(18,true)), h:Math.abs(v.getInt32(22,true))};
  });
}
function isPanelBmp(wh){
  return !!wh && ((wh.w===480 && wh.h===800) || (wh.w===800 && wh.h===480));
}
async function postRawBmp(file){
  const fd=new FormData();
  fd.append('file', file, '_preview.bmp');
  const r=await fetch('/api/show-bmp',{method:'POST',body:fd});
  if(!r.ok) throw new Error(await r.text());
}
async function sendRawPanelBmp(file, wh){
  if(uiBusy) return;
  setBusy(true, 'BMP an E6…');
  try{
    sandboxRawFile=file;
    srcOriginal=null;
    editName=null;
    orient.value=(wh.w===800 && wh.h===480)?'landscape':'portrait';
    zoom.value=100; panX=panY=0.5;
    labels=[]; selectedLab=-1; updateLabList();
    const url=URL.createObjectURL(file);
    await new Promise(function(resolve,reject){
      const im=new Image();
      im.onload=function(){
        img=im; imgSeq++; tunedSig=cropSig(); appliedSeq=imgSeq;
        syncStage(true);
        renderOut().then(resolve).catch(resolve);
      };
      im.onerror=function(){ reject(new Error('BMP konnte nicht gelesen werden')); };
      im.src=url;
    });
    URL.revokeObjectURL(url);
    await postRawBmp(file);
    status.textContent=file.name+' · unverändert an E6 (kein Dither)';
  }catch(e){
    sandboxRawFile=null;
    status.textContent=String(e.message||e);
  }finally{ setBusy(false); }
}
function pickFile(f){
  if(!f) return;
  loadFile(f);
}
function loadFile(f){
  if(!f) return;
  sandboxRawFile=null;
  if(editName){ file.value=''; status.textContent='Bearbeitung: '+editName+' — neues Foto erst nach „Bearbeitung beenden“'; return; }
  status.textContent='Lade '+f.name+'…';
  const url=URL.createObjectURL(f); const im=new Image();
  im.onload=function(){
    img=im; srcOriginal=f; editName=null; metaName.value=metaBirth.value=metaDeath.value=metaDesc.value='';
    labels=[]; selectedLab=-1; updateLabList(); zoom.value=zoomFitPct(); panX=panY=0.5;
    styleId='portrait';
    applyPrepDefaults();
    if(isSierra2()){
      sierra2Done=false; sierra2Sig='';
      imgSeq++; tunedSig=null; appliedSeq=-1;
      URL.revokeObjectURL(url);
      syncStage();
      status.textContent=f.name+' · Sierra — ganzes Bild, Zoom/schieben, dann konvertieren';
      return;
    }
    applyStyleDefaults(STYLE_DEF);
    imgSeq++; tunedSig=null; appliedSeq=-1;
    syncStage(true); URL.revokeObjectURL(url);
    status.textContent=f.name+' · Automatik…';
    tunePromise=runAutotune().catch(function(e){ status.textContent='Automatik: '+(e.message||e); });
  };
  im.onerror=function(){ srcOriginal=null; status.textContent='Bild konnte nicht geladen werden'; URL.revokeObjectURL(url); };
  im.src=url;
}
function canvasPos(e, el){
  const r=el.getBoundingClientRect();
  return {x:(e.clientX-r.left)/r.width, y:(e.clientY-r.top)/r.height};
}
function hitLabel(nx,ny){
  const wh=target(), w=wh[0], h=wh[1];
  const px=nx*w, py=ny*h;
  for(let i=labels.length-1;i>=0;i--){
    const L=labels[i];
    const box=measureLabelBox(octx, labels[i], w, h);
    const ang=-((L.rotate||0)*Math.PI)/180;
    const dx=px-box.cx, dy=py-box.cy;
    const lx=dx*Math.cos(ang)-dy*Math.sin(ang);
    const ly=dx*Math.sin(ang)+dy*Math.cos(ang);
    const left=box.x-box.cx, top=box.y-box.cy;
    if(lx>=left && lx<=left+box.w && ly>=top && ly<=top+box.h) return i;
  }
  return -1;
}

drop.addEventListener('click',function(){file.click();});
file.addEventListener('change',function(e){ const f=e.target.files && e.target.files[0]; if(f) pickFile(f); });
['dragenter','dragover'].forEach(function(ev){drop.addEventListener(ev,function(e){e.preventDefault(); e.stopPropagation(); drop.classList.add('drag');});});
['dragleave','drop'].forEach(function(ev){drop.addEventListener(ev,function(e){e.preventDefault(); e.stopPropagation(); drop.classList.remove('drag');});});
drop.addEventListener('drop',function(e){
  e.preventDefault();
  const f=e.dataTransfer && e.dataTransfer.files && e.dataTransfer.files[0];
  if(f) pickFile(f);
});
btnNewPic.onclick=function(){
  if(uiBusy) return;
  img=null; srcOriginal=null; srcNeedsRewrite=false; sandboxRawFile=null; sierra2Done=false; sierra2Sig=''; editName=null; loadedPicSig=''; pictureDirty=false; lastDitherSig=''; paintMode.value='sierra2'; syncVerfahrenUi(); metaName.value=metaBirth.value=metaDeath.value=metaDesc.value=''; labels=[]; selectedLab=-1; updateLabList(); zoom.value=100; panX=panY=0.5; imgSeq++; tunedSig=null; tunePromise=null; lastDither=null; syncEditUi(); syncStage(); status.textContent='Neues Bild · Foto wählen';
};

document.getElementById('btnAddLab').onclick=function(){
  if(uiBusy) return;
  const text=document.getElementById('labText').value.trim()||'Text';
  labels.push({
    role:'custom', text:text, x:0.5, y:0.72,
    size:+document.getElementById('labSize').value,
    color:document.getElementById('labColor').value,
    font:document.getElementById('labFont').value,
    align:document.getElementById('labAlign').value,
    bold:!!document.getElementById('labBold').checked,
    rotate:+document.getElementById('labRot').value||0
  });
  selectedLab=labels.length-1;
  updateLabList(); schedule();
};
document.getElementById('btnDelLab').onclick=function(){
  if(uiBusy) return;
  if(selectedLab<0){ status.textContent='Kein Text ausgewählt'; return; }
  deleteLabAt(selectedLab);
};
document.getElementById('btnClearLab').onclick=function(){
  if(uiBusy) return;
  labels=labels.filter(function(L){return L.role==='name'||L.role==='birth'||L.role==='death';});
  selectedLab=-1;
  updateLabList(); schedule();
};
capVisible.addEventListener('change', schedule);
['metaName','metaBirth','metaDeath','showName','showBirth','showDeath','sizeName','sizeDates'].forEach(function(id){
  document.getElementById(id).addEventListener('input', function(){ syncPersonLabels(); updateLabList(); schedule(); });
  document.getElementById(id).addEventListener('change', function(){ syncPersonLabels(); updateLabList(); schedule(); });
});
function applyControlsToSelected(){
  if(selectedLab<0 || selectedLab>=labels.length) return;
  const L=labels[selectedLab];
  if(!L.role || L.role==='custom'){
    const t=document.getElementById('labText').value.trim();
    if(t) L.text=t;
  }
  L.size=+document.getElementById('labSize').value;
  L.color=document.getElementById('labColor').value;
  L.font=document.getElementById('labFont').value;
  L.align=document.getElementById('labAlign').value;
  L.bold=!!document.getElementById('labBold').checked;
  L.rotate=+document.getElementById('labRot').value||0;
  document.getElementById('labRotNum').value=L.rotate;
  updateLabList();
  schedule();
}
function setRotation(deg){
  deg=Math.max(-180, Math.min(180, Math.round(+deg||0)));
  document.getElementById('labRot').value=deg;
  document.getElementById('labRotNum').value=deg;
  if(selectedLab>=0) applyControlsToSelected();
  else {
    syncPersonLabels();
    const customs=labels.filter(function(L){return !L.role||L.role==='custom';});
    if(customs.length) customs[customs.length-1].rotate=deg;
    updateLabList(); schedule();
  }
}
['labSize','labColor','labFont','labAlign','labBold','labText'].forEach(function(id){
  const el=document.getElementById(id);
  el.addEventListener('input', function(){
    if(selectedLab>=0){ applyControlsToSelected(); return; }
    syncPersonLabels();
    const customs=labels.filter(function(L){return !L.role||L.role==='custom';});
    if(customs.length){
      const L=customs[customs.length-1];
      L.size=+document.getElementById('labSize').value;
      L.color=document.getElementById('labColor').value;
      L.font=document.getElementById('labFont').value;
      L.align=document.getElementById('labAlign').value;
      L.bold=!!document.getElementById('labBold').checked;
      L.rotate=+document.getElementById('labRot').value||0;
    }
    updateLabList(); schedule();
  });
});
document.getElementById('labRot').addEventListener('input', function(){ setRotation(this.value); });
document.getElementById('labRotNum').addEventListener('input', function(){ setRotation(this.value); });
document.getElementById('labRotNum').addEventListener('change', function(){ setRotation(this.value); });

[orient,zoom,bright,contrast,warmth,dither,algo,prepExp,prepSat,prepScurve,prepHi,prepShadow,prepMid].forEach(function(el){
  el.addEventListener('input', function(){ pictureDirty=true; syncSliderVals(); schedule(); });
});
prepCdr.addEventListener('change', function(){ pictureDirty=true; schedule(); });
paintMode.addEventListener('change', function(){
  pictureDirty=true;
  sierra2Done=false; sierra2Sig=''; lastDither=null; lastDitherSig='';
  syncVerfahrenUi();
  syncEditUi();
  schedule(isSierra2()?'Sierra · Zuschnitt':'Verfahren…');
});
syncSliderVals();
orient.addEventListener('change', function(){ pictureDirty=true; syncStage(); });
stage.addEventListener('wheel',function(e){e.preventDefault(); pictureDirty=true; zoom.value=clamp(+zoom.value+(e.deltaY<0?2:-2),10,400); syncSliderVals(); schedule();},{passive:false});
stage.addEventListener('pointerdown',function(e){drag=true; labDragMode=false; lx=e.clientX; ly=e.clientY; stage.setPointerCapture(e.pointerId);});
stage.addEventListener('pointerup',function(){drag=false;});
stage.addEventListener('pointermove',function(e){
  if(!drag||!img) return;
  const wh=target(), tw=wh[0], th=wh[1]; const z=coverBase()*(+zoom.value/100); const dw=img.width*z, dh=img.height*z;
  panX=clamp(panX-(e.clientX-lx)/Math.max(1,Math.abs(tw-dw)||tw),0,1);
  panY=clamp(panY+(e.clientY-ly)/Math.max(1,Math.abs(th-dh)||th),0,1);
  lx=e.clientX; ly=e.clientY; pictureDirty=true; drawCrop(); schedule();
});

stageOut.addEventListener('pointerdown',function(e){
  const p=canvasPos(e, stageOut);
  dragLab=hitLabel(p.x,p.y);
  if(dragLab>=0){
    selectedLab=dragLab;
    selectLab(selectedLab);
    labDragMode=true;
    stageOut.setPointerCapture(e.pointerId);
    e.preventDefault();
  }
});
stageOut.addEventListener('pointermove',function(e){
  if(!labDragMode||dragLab<0) return;
  const p=canvasPos(e, stageOut);
  labels[dragLab].x=clamp(p.x,0.02,0.98);
  labels[dragLab].y=clamp(p.y,0.02,0.98);
  schedule();
});
stageOut.addEventListener('pointerup',function(){ labDragMode=false; dragLab=-1; updateLabList(); });

function toBmp(){
  const w=out.width,h=out.height;
  const tmp=document.createElement('canvas'); tmp.width=w; tmp.height=h;
  const tctx=tmp.getContext('2d');
  if(lastDither){
    tctx.putImageData(lastDither,0,0);
    drawLabels(tctx,w,h,false);
  } else {
    tctx.drawImage(out,0,0);
  }
  const data=tctx.getImageData(0,0,w,h).data;
  const row=((w*3+3)&~3), size=54+row*h;
  const buf=new ArrayBuffer(size), v=new DataView(buf), u=new Uint8Array(buf);
  u[0]=0x42;u[1]=0x4D; v.setUint32(2,size,true); v.setUint32(10,54,true); v.setUint32(14,40,true);
  v.setInt32(18,w,true); v.setInt32(22,h,true); v.setUint16(26,1,true); v.setUint16(28,24,true);
  for(let y=0;y<h;y++){ const sy=h-1-y; let o=54+y*row; for(let x=0;x<w;x++){ const i=(sy*w+x)*4; u[o++]=data[i+2]; u[o++]=data[i+1]; u[o++]=data[i]; } }
  return new Blob([buf],{type:'image/bmp'});
}
function makeJpegBlob(maxEdge, quality){
  return new Promise(function(resolve){
    drawCrop();
    const wh=target(), w=wh[0], h=wh[1];
    const scale=Math.min(1, maxEdge/Math.max(w,h));
    const tw=Math.max(1,Math.round(w*scale)), th=Math.max(1,Math.round(h*scale));
    const c=document.createElement('canvas'); c.width=tw; c.height=th;
    c.getContext('2d').drawImage(crop,0,0,tw,th);
    c.toBlob(function(b){ resolve(b); }, 'image/jpeg', quality);
  });
}
const SRC_MAX_EDGE=2000;
function makeSrcJpeg(quality){
  return new Promise(function(resolve){
    if(!img || !img.width){ resolve(null); return; }
    const scale=Math.min(1, SRC_MAX_EDGE/Math.max(img.width, img.height));
    const c=document.createElement('canvas');
    c.width=Math.max(1, Math.round(img.width*scale));
    c.height=Math.max(1, Math.round(img.height*scale));
    c.getContext('2d').drawImage(img,0,0,c.width,c.height);
    c.toBlob(function(b){ resolve(b); }, 'image/jpeg', quality);
  });
}
function isJpegFile(f){
  if(!f) return false;
  const n=(f.name||'').toLowerCase();
  const t=(f.type||'').toLowerCase();
  return t==='image/jpeg' || n.endsWith('.jpg') || n.endsWith('.jpeg');
}
async function srcBlobForSave(){
  if(!img || !img.width) return null;
  const edge=Math.max(img.width, img.height);
  if(srcOriginal && isJpegFile(srcOriginal) && edge<=SRC_MAX_EDGE) return srcOriginal;
  return makeSrcJpeg(0.88);
}
function setBusy(on, msg){
  if(on){
    busyDepth++;
    if(msg){
      const m=document.getElementById('busyMsg');
      if(m) m.textContent=msg;
    }
  } else {
    busyDepth=Math.max(0, busyDepth-1);
  }
  uiBusy=busyDepth>0;
  document.body.classList.toggle('busy', uiBusy);
  const ov=document.getElementById('busyOv');
  if(ov) ov.classList.toggle('on', uiBusy);
  return true;
}
async function uploadJpeg(kind, blob, fname){
  if(!blob) return;
  const suffix=kind==='src'?'_src.jpg':'_thumb.jpg';
  const tfd=new FormData();
  tfd.append('file', blob, baseNameNoExt(fname)+suffix);
  const tr=await fetch('/api/upload-thumb?name='+encodeURIComponent(fname)+'&kind='+kind,{method:'POST',body:tfd});
  if(!tr.ok) throw new Error(await tr.text());
}
function baseNameNoExt(n){ return String(n||'').replace(/\.(bmp|jpg|jpeg|json)$/i,''); }
async function saveMetaOnly(fname){
  syncPersonLabels();
  const body='json='+encodeURIComponent(JSON.stringify(buildMeta()));
  const r=await fetch('/api/meta?name='+encodeURIComponent(fname),{
    method:'POST',
    headers:{'Content-Type':'application/x-www-form-urlencoded'},
    body:body
  });
  const msg=await r.text();
  if(!r.ok) throw new Error(msg);
  return msg;
}
async function send(show){
  if(busyDepth>0 && busy) return;
  if(sandboxRawFile){
    setBusy(true, show?'Anzeigen…':'Speichern…');
    try{
      if(show){
        await postRawBmp(sandboxRawFile);
        status.textContent='Am Rahmen · BMP unverändert';
        return;
      }
      const fname=editName || ('studio_'+Date.now()+'.bmp');
      const fd=new FormData();
      fd.append('file', sandboxRawFile, fname);
      fd.append('show','0');
      fd.append('meta', JSON.stringify(buildMeta()));
      const r=await fetch('/api/upload',{method:'POST',body:fd});
      const msg=await r.text();
      if(!r.ok) throw new Error(msg);
      status.textContent=msg+' · gespeichert';
      window.location.href='/gallery';
    }catch(e){ status.textContent=String(e.message||e); }
    finally{ setBusy(false); }
    return;
  }
  if(!img){ status.textContent='Kein Bild'; return; }
  if(editName && !show && !pictureDirty && picSig()===loadedPicSig){
    setBusy(true,'Text speichern…');
    try{
      await saveMetaOnly(editName);
      status.textContent='Text gespeichert · Bild unverändert';
      window.location.href='/gallery';
    }catch(e){ status.textContent=String(e.message||e); }
    finally{ setBusy(false); }
    return;
  }
  if(isSierra2() && !sierra2Done){ status.textContent='Zuerst In E6 konvertieren'; return; }
  if(tunePromise) await tunePromise;
  setBusy(true, show?'Anzeigen…':'Speichern…');
  try{
    syncPersonLabels();
    busy=true; try{ await renderOut(); } finally{ busy=false; }
    const bmp=toBmp();
    if(show){
      const fd=new FormData();
      fd.append('file', bmp, '_preview.bmp');
      const r=await fetch('/api/show-bmp',{method:'POST',body:fd});
      if(!r.ok) throw new Error(await r.text());
      status.textContent='Am Rahmen angezeigt · nicht gespeichert';
      return;
    }
    const fname=editName || ('studio_'+Date.now()+'.bmp');
    const meta=JSON.stringify(buildMeta());
    const fd=new FormData();
    fd.append('file', bmp, fname);
    fd.append('show','0');
    fd.append('meta', meta);
    const r=await fetch('/api/upload',{method:'POST',body:fd});
    let msg=await r.text();
    if(!r.ok) throw new Error(msg);
    const clean=await makeJpegBlob(240, 0.72);
    if(!clean) throw new Error('Vorschau fehlgeschlagen');
    await uploadJpeg('clean', clean, fname);
    if(!editName || srcNeedsRewrite){
      const src=await srcBlobForSave();
      if(!src) throw new Error('Original fehlt');
      await uploadJpeg('src', src, fname);
      srcNeedsRewrite=false;
    }
    status.textContent=msg+' · gespeichert';
    window.location.href='/gallery';
  }catch(e){ status.textContent=String(e); }
  finally{ setBusy(false); }
}
function applyMeta(m){
  if(!m) return;
  metaName.value=m.name||'';
  metaBirth.value=m.birth||'';
  metaDeath.value=m.death||'';
  metaDesc.value=m.description||m.beschreibung||'';
  if(typeof m.showName==='boolean') showName.checked=m.showName;
  if(typeof m.showBirth==='boolean') showBirth.checked=m.showBirth;
  if(typeof m.showDeath==='boolean') showDeath.checked=m.showDeath;
  if(m.sizeName) sizeName.value=m.sizeName;
  if(m.sizeDates) sizeDates.value=m.sizeDates;
  if(typeof m.captionVisible==='boolean') capVisible.checked=m.captionVisible;
  if(m.orient){ orient.value=m.orient; }
  styleId='portrait';
  if(m.bright!=null) bright.value=m.bright;
  if(m.contrast!=null) contrast.value=m.contrast;
  if(m.warmth!=null) warmth.value=m.warmth;
  if(m.dither!=null) dither.value=m.dither;
  if(m.algo) algo.value=m.algo;
  if(m.paintMode==='sierra2'||m.paintMode==='saved') paintMode.value=m.paintMode;
  else paintMode.value='saved';
  syncVerfahrenUi();
  writePrep(m);
  if(m.zoom!=null) zoom.value=m.zoom;
  if(typeof m.panX==='number') panX=m.panX;
  if(typeof m.panY==='number') panY=m.panY;
  if(Array.isArray(m.labels) && m.labels.length){
    labels=m.labels.map(function(L){
      return {role:L.role||'custom',text:L.text,x:L.x,y:L.y,size:L.size,color:L.color,font:L.font,align:L.align,bold:!!L.bold,rotate:+(L.rotate||0)};
    });
  } else {
    labels=[];
  }
  syncPersonLabels();
  selectedLab=-1;
  updateLabList();
  syncSliderVals();
}
function loadImgUrl(url){
  return new Promise(function(resolve,reject){
    const im=new Image();
    im.onload=function(){ resolve(im); };
    im.onerror=function(){ reject(new Error('load')); };
    im.src=url;
  });
}
async function loadEdit(name){
  setBusy(true,'Bild laden…');
  try{
    editName=name; srcOriginal=null; srcNeedsRewrite=false; syncEditUi();
    status.textContent='Lade '+name+'…';
    let meta=null;
    try{
      const raw=await (await fetch('/api/meta?name='+encodeURIComponent(name))).text();
      meta=JSON.parse(raw);
    }catch(e){ meta=null; }
    if(meta && typeof meta==='object') applyMeta(meta);
    else { paintMode.value='saved'; syncVerfahrenUi(); syncPersonLabels(); }
    const enc=encodeURIComponent(name);
    let srcSize=0;
    try{
      const info=await (await fetch('/api/src?name='+enc+'&info=1')).json();
      srcSize=+(info && info.size)||0;
    }catch(e){ srcSize=0; }
    let srcUrl='/api/src?name='+enc+'&t='+Date.now();
    let note='';
    if(srcSize<32 || srcSize>6000000){
      srcUrl='/api/pic?name='+enc+'&t='+Date.now();
      srcNeedsRewrite=true;
      note=srcSize>6000000
        ? ('Original '+Math.round(srcSize/1048576)+' MB — ESP würde hängen. Panel-BMP geladen. Neu speichern ersetzt das Original.')
        : 'Kein Original — Panel-BMP geladen. Neu speichern legt ein Original an.';
    }
    const im=await loadImgUrl(srcUrl);
    img=im;
    if(!(meta && meta.zoom!=null)) zoom.value=zoomFitPct();
    if(!(meta && typeof meta.panX==='number')){ panX=0.5; panY=0.5; }
    imgSeq++;
    sierra2Done=false; sierra2Sig=''; lastDither=null; lastDitherSig='';
    pictureDirty=false;
    syncEditUi();
    syncStage();
    tunedSig=cropSig();
    loadedPicSig=picSig();
    schedule('Wiederhergestellt…');
    status.textContent=note || ('Bearbeiten: '+name);
  }catch(e){ status.textContent=String(e.message||e); editName=null; syncEditUi(); }
  finally{ setBusy(false); }
}
function cropSig(){
  return [imgSeq, orient.value, zoom.value,
          Math.round(panX*1000), Math.round(panY*1000)].join('|');
}
function picSig(){
  return cropSig()+'|'+[paintMode.value, bright.value, contrast.value, warmth.value, dither.value, algo.value,
    prepExp.value, prepSat.value, prepScurve.value, prepHi.value, prepShadow.value, prepMid.value,
    prepCdr.checked?1:0].join('|');
}
function inkOfDevice(r,g,b){
  if(r>200 && g>200 && b>200) return 1;
  if(r>200 && g>200) return 2;
  if(r>200) return 3;
  if(b>200) return 4;
  if(g>200) return 5;
  return 0;
}
function scoreTune(outId, cropId){
  const o=outId.data, c=cropId.data;
  let e=0, np=0, nRose=0, blueRose=0;
  for(let i=0;i<o.length;i+=16){
    const cr=c[i], cg=c[i+1], cb=c[i+2];
    const mi=inkOfDevice(o[i],o[i+1],o[i+2]);
    const m=MEASURED[mi].m;
    e+=(cr-m[0])*(cr-m[0])+(cg-m[1])*(cg-m[1])+(cb-m[2])*(cb-m[2]);
    np++;
    if(isRose(cr,cg,cb)){
      nRose++;
      if(mi===4) blueRose++;
    }
  }
  const rmse=Math.sqrt(e/Math.max(1,np));
  const rosePen=nRose?(blueRose/nRose)*120:0;
  return -(rmse+rosePen);
}
async function runAutotune(){
  if(!img){ status.textContent='Kein Bild'; return; }
  if(tuning){ status.textContent='Automatik läuft noch…'; return; }
  if(isSierra2()){ status.textContent='Sierra: In E6 konvertieren'; return; }
  tuning=true;
  pictureDirty=true;
  setBusy(true, 'Automatik…');
  try{
    applyPrepDefaults();
    applyStyleDefaults(STYLE_DEF);
    const algos=['atkinson','floyd','stucki'];
    const warms=[42,46,50,54,58];
    const sats=[0.85,0.95,1.00,1.10,1.20];
    const total=algos.length+warms.length*sats.length;
    let best=null, bestScore=-1e18, done=0;
    const msg=document.getElementById('busyMsg');
    async function tryCand(c){
      if(c.algo) algo.value=c.algo;
      if(c.warm!=null) warmth.value=c.warm;
      if(c.sat!=null) prepSat.value=c.sat;
      syncSliderVals();
      done++;
      if(msg) msg.textContent='Automatik '+done+'/'+total+' · '+(algo.value==='floyd'?'Floyd':algo.value==='stucki'?'Stucki':'Atkinson')+'…';
      await new Promise(function(r){ setTimeout(r,0); });
      await renderOut();
      const sc=scoreTune(lastDither, lastCrop);
      if(sc>bestScore){
        bestScore=sc;
        best={algo:algo.value, warm:+warmth.value, sat:+prepSat.value};
      }
    }
    for(let i=0;i<algos.length;i++) await tryCand({algo:algos[i]});
    algo.value=best.algo;
    for(let wi=0;wi<warms.length;wi++){
      for(let si=0;si<sats.length;si++){
        await tryCand({warm:warms[wi], sat:sats[si]});
      }
    }
    algo.value=best.algo;
    warmth.value=best.warm;
    prepSat.value=best.sat;
    syncSliderVals();
    appliedSeq=imgSeq;
    tunedSig=cropSig();
    await renderOut();
    const names={atkinson:'Atkinson',floyd:'Floyd',stucki:'Stucki'};
    status.textContent='Automatik OK · '+(names[best.algo]||best.algo)+' · W'+best.warm+' · Sat '+Number(best.sat).toFixed(2);
  }catch(e){
    status.textContent='Automatik: '+e.message;
  }finally{ tuning=false; setBusy(false); }
}
document.getElementById('btnTuneStyle').onclick=function(){
  tunePromise=runAutotune().catch(function(e){ status.textContent='Automatik: '+(e.message||e); });
};
document.getElementById('btnShow').onclick=function(){send(true);};
document.getElementById('btnSave').onclick=function(){send(false);};
if(document.fonts && document.fonts.load){
  Promise.all([
    document.fonts.load('400 28px "Dancing Script"'),
    document.fonts.load('700 28px "Dancing Script"')
  ]).then(function(){ schedule(); }).catch(function(){});
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
refreshStatus(); setInterval(refreshStatus,15000);
syncPersonLabels();
syncVerfahrenUi();
syncEditUi();
(function(){
  const btnRaw=document.getElementById('btnRawE6');
  const fileRaw=document.getElementById('fileRawE6');
  if(btnRaw && fileRaw){
    btnRaw.onclick=function(){ fileRaw.click(); };
    fileRaw.addEventListener('change', function(e){
      const f=e.target.files && e.target.files[0];
      if(!f) return;
      readBmpWh(f).then(function(wh){
        if(isPanelBmp(wh)) sendRawPanelBmp(f, wh);
        else status.textContent='Nur 480×800 oder 800×480 BMP';
      }).catch(function(){ status.textContent='BMP konnte nicht gelesen werden'; });
      fileRaw.value='';
    });
  }
  const btnS=document.getElementById('btnSierra2');
  if(btnS){
    btnS.onclick=async function(){
      if(!img){ status.textContent='Kein Bild'; return; }
      if(uiBusy) return;
      if(!isSierra2()){ paintMode.value='sierra2'; syncVerfahrenUi(); }
      sierra2Done=true;
      pictureDirty=true;
      lastDither=null;
      sierra2Sig='';
      setBusy(true, 'LUT + Sierra…');
      try{ await renderOut(); }
      catch(e){ status.textContent=String(e.message||e); sierra2Done=false; }
      finally{ setBusy(false); }
    };
  }
})();
(function(){
  const q=new URLSearchParams(location.search);
  const ed=q.get('edit');
  if(ed) loadEdit(ed);
  else syncStage();
})();
</script>
</body></html>
)HTML";
