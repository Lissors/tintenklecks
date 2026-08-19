# Tintenklecks

Firmware für den **Waveshare ESP32-S3 PhotoPainter** (7,3″ Spectra-6 / E6 E-Paper).
Studio, Galerie und Anzeige laufen auf dem Rahmen — Arduino IDE, kein ESP-IDF.

© 2026 Ingo Lissors · Herkunft und Lizenzen: [CREDITS.txt](CREDITS.txt)

## Board (Arduino IDE)

- Board: **ESP32S3 Dev Module**
- USB CDC On Boot: **Enabled**
- Flash: **16 MB**
- PSRAM: **OPI PSRAM**

Sketch-Datei: `Bilderrahmen.ino` (Ordnername bleibt für die Arduino IDE).

## Nutzung

Ohne WLAN: Access Point **Tintenklecks**, Passwort `tintenklecks` → http://192.168.4.1  
Mit WLAN: die IP des Rahmens, später http://tintenklecks.local

Web: Menü, Studio, Galerie, Live, System, Rahmen.

## SD-Karte

- Bilder: `/pic/` (BMP, JSON, Vorschaubilder)
- Töne: `/sound/` (`willkommen.wav`, `wlan.wav`, `ap.wav`, `neustart.wav`)

Galeriebilder und Sprachclips gehören nicht zu dieser Firmware.
