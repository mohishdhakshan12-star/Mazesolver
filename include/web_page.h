#ifndef WEB_PAGE_H
#define WEB_PAGE_H

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>MazeBot ESP32 Wireless Telemetry HUD</title>
  <link rel="preconnect" href="https://fonts.googleapis.com">
  <link href="https://fonts.googleapis.com/css2?family=JetBrains+Mono:wght@400;600;800&family=Outfit:wght@400;600;800&display=swap" rel="stylesheet">
  <style>
    :root {
      --bg: #0b0f19;
      --card-bg: rgba(18, 24, 38, 0.85);
      --card-border: rgba(56, 189, 248, 0.18);
      --accent-cyan: #38bdf8;
      --accent-green: #10b981;
      --accent-yellow: #f59e0b;
      --accent-red: #ef4444;
      --text-main: #f1f5f9;
      --text-muted: #94a3b8;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; }
    body {
      font-family: 'Outfit', sans-serif;
      background: radial-gradient(circle at 50% 10%, #1e293b 0%, #0b0f19 80%);
      color: var(--text-main);
      min-height: 100vh;
      padding: 16px;
      display: flex;
      flex-direction: column;
      align-items: center;
    }
    header {
      width: 100%;
      max-width: 950px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      padding: 12px 20px;
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 16px;
      backdrop-filter: blur(12px);
      margin-bottom: 16px;
      box-shadow: 0 8px 32px rgba(0,0,0,0.4);
    }
    .brand { display: flex; align-items: center; gap: 12px; }
    .pulse-dot {
      width: 12px; height: 12px; border-radius: 50%;
      background: var(--accent-green);
      box-shadow: 0 0 12px var(--accent-green);
      animation: pulse 1.6s infinite ease-in-out;
    }
    @keyframes pulse {
      0%, 100% { opacity: 1; transform: scale(1); }
      50% { opacity: 0.4; transform: scale(1.2); }
    }
    h1 { font-size: 1.35rem; font-weight: 800; letter-spacing: -0.5px; }
    .ip-badge {
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.85rem;
      padding: 6px 12px;
      border-radius: 8px;
      background: rgba(56, 189, 248, 0.1);
      color: var(--accent-cyan);
      border: 1px solid rgba(56, 189, 248, 0.3);
    }

    .main-grid {
      width: 100%;
      max-width: 950px;
      display: grid;
      grid-template-columns: 1.2fr 1fr;
      gap: 16px;
    }
    @media (max-width: 820px) {
      .main-grid { grid-template-columns: 1fr; }
    }

    .card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 16px;
      padding: 20px;
      backdrop-filter: blur(12px);
      box-shadow: 0 8px 32px rgba(0,0,0,0.4);
    }
    .card-title {
      font-size: 0.95rem;
      text-transform: uppercase;
      letter-spacing: 1.2px;
      color: var(--text-muted);
      font-weight: 600;
      margin-bottom: 16px;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }

    /* RADAR HUD */
    .radar-container {
      position: relative;
      width: 100%;
      height: 330px;
      display: flex;
      justify-content: center;
      align-items: center;
      background: radial-gradient(circle, rgba(56,189,248,0.04) 0%, transparent 70%);
      border-radius: 12px;
      overflow: hidden;
    }
    .grid-lines {
      position: absolute;
      width: 280px; height: 280px;
      border: 1px dashed rgba(56, 189, 248, 0.15);
      border-radius: 50%;
    }
    .grid-lines-inner {
      position: absolute;
      width: 160px; height: 160px;
      border: 1px dashed rgba(56, 189, 248, 0.2);
      border-radius: 50%;
    }

    /* BOT SILHOUETTE */
    .bot-body {
      position: absolute;
      bottom: 60px;
      width: 84px; height: 120px;
      background: linear-gradient(180deg, #1e293b, #0f172a);
      border: 2px solid var(--accent-cyan);
      border-radius: 14px 14px 8px 8px;
      box-shadow: 0 0 20px rgba(56, 189, 248, 0.25);
      display: flex;
      flex-direction: column;
      justify-content: center;
      align-items: center;
      z-index: 5;
    }
    .bot-arrow {
      color: var(--accent-cyan);
      font-size: 1.4rem;
      font-weight: 800;
      margin-bottom: 4px;
    }
    .bot-name {
      font-size: 0.7rem;
      font-weight: 700;
      letter-spacing: 1px;
      color: var(--text-muted);
    }
    .wheel {
      position: absolute;
      width: 12px; height: 32px;
      background: #475569;
      border-radius: 4px;
      border: 1px solid #64748b;
    }
    .wheel-l { left: -14px; top: 55px; }
    .wheel-r { right: -14px; top: 55px; }

    /* SENSOR BEAM PILLS */
    .sensor-node {
      position: absolute;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.78rem;
      font-weight: 700;
      padding: 6px 10px;
      border-radius: 20px;
      background: rgba(15, 23, 42, 0.95);
      border: 1.5px solid var(--accent-green);
      color: var(--accent-green);
      box-shadow: 0 0 10px rgba(16, 185, 129, 0.2);
      transition: all 0.15s ease;
      white-space: nowrap;
      z-index: 10;
    }
    #node-center { top: 22px; left: 50%; transform: translateX(-50%); }
    #node-fl     { top: 52px; left: 16%; }
    #node-fr     { top: 52px; right: 16%; }
    #node-left   { top: 135px; left: 6%; }
    #node-right  { top: 135px; right: 6%; }

    /* SENSOR METERS */
    .sensor-meter-list { display: flex; flex-direction: column; gap: 8px; margin-top: 14px; }
    .meter-row {
      display: flex;
      align-items: center;
      justify-content: space-between;
      gap: 10px;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.8rem;
    }
    .meter-bar-bg {
      flex: 1;
      height: 8px;
      background: rgba(255,255,255,0.08);
      border-radius: 4px;
      overflow: hidden;
    }
    .meter-bar-fill {
      height: 100%;
      width: 50%;
      background: var(--accent-green);
      border-radius: 4px;
      transition: width 0.1s linear, background 0.15s ease;
    }

    /* TELEMETRY STATUS */
    .telemetry-row {
      display: flex;
      justify-content: space-between;
      padding: 10px 14px;
      background: rgba(255, 255, 255, 0.03);
      border-radius: 8px;
      margin-bottom: 8px;
      font-family: 'JetBrains Mono', monospace;
      font-size: 0.85rem;
    }
    .state-badge {
      font-weight: 800;
      padding: 4px 10px;
      border-radius: 6px;
      background: rgba(56, 189, 248, 0.15);
      color: var(--accent-cyan);
    }

    /* HARDWARE BUTTON STATUS CARDS */
    .btn-indicator-grid {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 12px;
      margin-top: 14px;
    }
    .btn-indicator {
      display: flex;
      align-items: center;
      gap: 12px;
      padding: 12px 14px;
      border-radius: 12px;
      background: rgba(255, 255, 255, 0.04);
      border: 1px solid rgba(255, 255, 255, 0.1);
    }
    .btn-indicator.start-btn {
      border-color: rgba(16, 185, 129, 0.4);
      background: rgba(16, 185, 129, 0.08);
    }
    .btn-indicator.stop-btn {
      border-color: rgba(239, 68, 68, 0.4);
      background: rgba(239, 68, 68, 0.08);
    }
    .btn-badge {
      font-size: 1.2rem;
      width: 36px; height: 36px;
      border-radius: 10px;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: 800;
    }
    .start-btn .btn-badge {
      background: rgba(16, 185, 129, 0.2);
      color: var(--accent-green);
      box-shadow: 0 0 12px rgba(16, 185, 129, 0.3);
    }
    .stop-btn .btn-badge {
      background: rgba(239, 68, 68, 0.2);
      color: var(--accent-red);
      box-shadow: 0 0 12px rgba(239, 68, 68, 0.3);
    }
    .btn-indicator-text {
      display: flex;
      flex-direction: column;
    }
    .btn-indicator-title {
      font-weight: 700;
      font-size: 0.82rem;
      letter-spacing: 0.3px;
    }
    .btn-indicator-sub {
      font-size: 0.7rem;
      color: var(--text-muted);
      margin-top: 2px;
    }

    /* MANUAL PAD */
    .dpad-container {
      display: grid;
      grid-template-columns: repeat(3, 52px);
      grid-template-rows: repeat(3, 52px);
      gap: 8px;
      justify-content: center;
      margin-top: 16px;
    }
    .dpad-btn {
      background: rgba(255,255,255,0.06);
      border: 1px solid rgba(255,255,255,0.15);
      border-radius: 10px;
      color: white;
      font-size: 1.2rem;
      font-weight: bold;
      cursor: pointer;
      transition: all 0.1s ease;
    }
    .dpad-btn:active {
      background: var(--accent-cyan);
      color: #0b0f19;
      box-shadow: 0 0 14px var(--accent-cyan);
    }

    /* TUNING SLIDERS */
    .slider-row {
      margin-top: 14px;
    }
    .slider-header {
      display: flex;
      justify-content: space-between;
      font-size: 0.82rem;
      font-weight: 600;
      margin-bottom: 6px;
      color: var(--text-muted);
    }
    input[type=range] {
      width: 100%;
      height: 6px;
      accent-color: var(--accent-cyan);
      background: rgba(255,255,255,0.1);
      border-radius: 4px;
      cursor: pointer;
    }
  </style>
</head>
<body>

  <header>
    <div class="brand">
      <div class="pulse-dot" id="live-dot"></div>
      <div>
        <h1>MazeBot ESP32 HUD</h1>
        <p style="font-size:0.75rem; color:var(--text-muted);">5x Time-of-Flight Laser Array & Wall Solver</p>
      </div>
    </div>
    <div class="ip-badge" id="ip-display">Connecting...</div>
  </header>

  <div class="main-grid">
    <!-- LEFT COLUMN: SENSOR RADAR & GAUGES -->
    <div class="card">
      <div class="card-title">
        <span>ToF Laser Distance Array</span>
        <span style="font-size:0.75rem; color:var(--accent-cyan);" id="ping-rate">10 Hz</span>
      </div>

      <div class="radar-container">
        <div class="grid-lines"></div>
        <div class="grid-lines-inner"></div>

        <!-- 5 Sensor Pills -->
        <div class="sensor-node" id="node-center">FRONT: --- mm</div>
        <div class="sensor-node" id="node-fl">FL: --- mm</div>
        <div class="sensor-node" id="node-fr">FR: --- mm</div>
        <div class="sensor-node" id="node-left">LEFT: --- mm</div>
        <div class="sensor-node" id="node-right">RIGHT: --- mm</div>

        <!-- Bot Chassis -->
        <div class="bot-body">
          <div class="wheel wheel-l"></div>
          <div class="wheel wheel-r"></div>
          <div class="bot-arrow">&#9650;</div>
          <div class="bot-name">ESP32</div>
        </div>
      </div>

      <!-- Linear Meters -->
      <div class="sensor-meter-list">
        <div class="meter-row">
          <span style="width:75px;">Center:</span>
          <div class="meter-bar-bg"><div class="meter-bar-fill" id="bar-center"></div></div>
          <span id="val-center" style="width:65px; text-align:right;">--- mm</span>
        </div>
        <div class="meter-row">
          <span style="width:75px;">Front-L:</span>
          <div class="meter-bar-bg"><div class="meter-bar-fill" id="bar-fl"></div></div>
          <span id="val-fl" style="width:65px; text-align:right;">--- mm</span>
        </div>
        <div class="meter-row">
          <span style="width:75px;">Front-R:</span>
          <div class="meter-bar-bg"><div class="meter-bar-fill" id="bar-fr"></div></div>
          <span id="val-fr" style="width:65px; text-align:right;">--- mm</span>
        </div>
        <div class="meter-row">
          <span style="width:75px;">Left:</span>
          <div class="meter-bar-bg"><div class="meter-bar-fill" id="bar-left"></div></div>
          <span id="val-left" style="width:65px; text-align:right;">--- mm</span>
        </div>
        <div class="meter-row">
          <span style="width:75px;">Right:</span>
          <div class="meter-bar-bg"><div class="meter-bar-fill" id="bar-right"></div></div>
          <span id="val-right" style="width:65px; text-align:right;">--- mm</span>
        </div>
      </div>
    </div>

    <!-- RIGHT COLUMN: CONTROL & NAVIGATION -->
    <div style="display:flex; flex-direction:column; gap:16px;">
      
      <div class="card">
        <div class="card-title">Robot Controller & State</div>

        <div class="telemetry-row">
          <span>Operating State:</span>
          <span class="state-badge" id="tele-state">STOPPED</span>
        </div>
        <div class="telemetry-row">
          <span>Motor Left (PWM):</span>
          <span id="tele-pwma" style="color:var(--accent-cyan); font-weight:700;">0</span>
        </div>
        <div class="telemetry-row">
          <span>Motor Right (PWM):</span>
          <span id="tele-pwmb" style="color:var(--accent-cyan); font-weight:700;">0</span>
        </div>

        <div class="btn-indicator-grid">
          <div class="btn-indicator start-btn">
            <div class="btn-badge">&#9654;</div>
            <div class="btn-indicator-text">
              <span class="btn-indicator-title" style="color:var(--accent-green);">START BUTTON (GPIO 13)</span>
              <span class="btn-indicator-sub">Press on bot to start Entry-to-Exit run</span>
            </div>
          </div>
          <div class="btn-indicator stop-btn">
            <div class="btn-badge">&#9209;</div>
            <div class="btn-indicator-text">
              <span class="btn-indicator-title" style="color:var(--accent-red);">STOP BUTTON (GPIO 4)</span>
              <span class="btn-indicator-sub">Emergency stop anytime</span>
            </div>
          </div>
        </div>

        <div style="margin-top:12px; padding:10px 12px; background:rgba(56,189,248,0.06); border:1px dashed rgba(56,189,248,0.25); border-radius:10px; font-size:0.75rem; color:var(--text-muted); line-height:1.4;">
          🏁 <strong style="color:var(--text-main);">Entry-to-Exit Operation:</strong> Place bot at maze entry &rarr; press <strong style="color:var(--accent-green);">START</strong> on bot &rarr; solves maze &rarr; automatically halts upon reaching <strong style="color:var(--accent-cyan);">EXIT</strong>. Wireless start/stop is disabled for safety.
        </div>
      </div>

      <!-- TUNING CARD -->
      <div class="card">
        <div class="card-title">Live Navigation Tuning</div>

        <div class="slider-row">
          <div class="slider-header">
            <span>Base Drive Speed</span>
            <span id="lbl-speed">150</span>
          </div>
          <input type="range" id="rng-speed" min="70" max="255" value="150" onchange="tuneParam('speed', this.value)">
        </div>

        <div class="slider-row">
          <div class="slider-header">
            <span>Front Obstacle Threshold (mm)</span>
            <span id="lbl-front">140 mm</span>
          </div>
          <input type="range" id="rng-front" min="60" max="300" value="140" onchange="tuneParam('front', this.value)">
        </div>

        <div class="slider-row">
          <div class="slider-header">
            <span>Desired Side Wall Distance (mm)</span>
            <span id="lbl-side">110 mm</span>
          </div>
          <input type="range" id="rng-side" min="50" max="250" value="110" onchange="tuneParam('side', this.value)">
        </div>
      </div>

    </div>
  </div>

  <script>
    document.getElementById('ip-display').innerText = window.location.host || '192.168.4.1';

    function updatePill(nodeId, barId, valId, dist) {
      const node = document.getElementById(nodeId);
      const bar = document.getElementById(barId);
      const val = document.getElementById(valId);
      if (!node) return;

      if (dist < 0 || dist > 2000) {
        node.innerText = "OUT OF RANGE";
        node.style.borderColor = "var(--text-muted)";
        node.style.color = "var(--text-muted)";
        bar.style.width = "0%";
        val.innerText = ">2000 mm";
        return;
      }

      node.innerText = dist + " mm";
      val.innerText = dist + " mm";

      let pct = Math.min(100, Math.max(5, (dist / 400) * 100));
      bar.style.width = pct + "%";

      let color = "var(--accent-green)";
      if (dist < 100) {
        color = "var(--accent-red)";
      } else if (dist < 180) {
        color = "var(--accent-yellow)";
      }

      node.style.borderColor = color;
      node.style.color = color;
      bar.style.background = color;
    }

    async function fetchTelemetry() {
      try {
        const res = await fetch('/api/status');
        if (!res.ok) throw new Error();
        const d = await res.json();

        updatePill('node-center', 'bar-center', 'val-center', d.c);
        updatePill('node-fl',     'bar-fl',     'val-fl',     d.fl);
        updatePill('node-fr',     'bar-fr',     'val-fr',     d.fr);
        updatePill('node-left',   'bar-left',   'val-left',   d.l);
        updatePill('node-right',  'bar-right',  'val-right',  d.r);

        const stateBadge = document.getElementById('tele-state');
        stateBadge.innerText = d.state;
        if (d.state.indexOf('STOPPED') !== -1) {
          stateBadge.style.color = 'var(--accent-red)';
          stateBadge.style.background = 'rgba(239,68,68,0.15)';
        } else if (d.state.indexOf('EXIT') !== -1) {
          stateBadge.style.color = '#38bdf8';
          stateBadge.style.background = 'rgba(56,189,248,0.2)';
        } else {
          stateBadge.style.color = 'var(--accent-green)';
          stateBadge.style.background = 'rgba(16,185,129,0.15)';
        }

        document.getElementById('tele-pwma').innerText = d.pwma;
        document.getElementById('tele-pwmb').innerText = d.pwmb;
        document.getElementById('live-dot').style.background = 'var(--accent-green)';
      } catch (e) {
        document.getElementById('live-dot').style.background = 'var(--accent-red)';
      }
    }

    function tuneParam(param, val) {
      if (param === 'speed') document.getElementById('lbl-speed').innerText = val;
      if (param === 'front') document.getElementById('lbl-front').innerText = val + ' mm';
      if (param === 'side')  document.getElementById('lbl-side').innerText = val + ' mm';
      fetch(`/api/control?action=tune&param=${param}&val=${val}`);
    }

    setInterval(fetchTelemetry, 100);
  </script>
</body>
</html>
)rawliteral";

#endif // WEB_PAGE_H
