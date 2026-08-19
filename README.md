# Tintenklecks

Firmware für den **Waveshare ESP32-S3 PhotoPainter** (7,3″ Spectra-6 / E6, 480×800).
Zuschnitt, Verfahren und Beschriftung laufen im Browser auf dem Rahmen.
Die Galerie liegt auf der SD-Karte. Kein ESP-IDF, nur Arduino IDE.

© 2026 Ingo Lissors · Herkunft und Lizenzen: [CREDITS.txt](CREDITS.txt)

Sketch-Datei: `Bilderrahmen.ino` (Ordnername bleibt, sonst findet die Arduino IDE den Sketch nicht).

## So sieht es aus

Hauptmenü, Galerie und Live-Anzeige im Browser:

![Hauptmenü](docs/menu.png)

![Galerie](docs/galerie.png)

![Live-Anzeige](docs/live.png)

## Hardware

- Waveshare ESP32-S3 PhotoPainter, Spectra-6 / E6
- Micro-SD, **FAT32**
- USB-C am Rahmen (Flashen und Web)
- optional Akku (AXP2101)

## SD-Karte einrichten

Ohne Karte startet das Web, Bilder und Ton fehlen.

1. Karte am PC als **FAT32** formatieren (eine Partition).
2. Zwei Ordner anlegen und die mitgelieferten WAV-Dateien kopieren:

```
sound/willkommen.wav
sound/wlan.wav
sound/ap.wav
sound/neustart.wav
pic/
```

Die vier WAV-Dateien liegen im Ordner `sound/` dieses Downloads. Nach `sound/` auf der Karte kopieren, Namen nicht ändern.

`pic/` bleibt leer. Bilder entstehen erst im Studio auf dem Rahmen. Es gibt **keine** mitgelieferten BMP, JPG oder Vorschaubilder.

3. Karte in den Rahmen, **dann** Firmware flashen.

| Datei | Wann |
| --- | --- |
| `willkommen.wav` | jeder Reset/Reboot (nicht nach Deep Sleep) |
| `wlan.wav` | Heimnetz verbunden |
| `ap.wav` | kein WLAN, Access Point |
| `neustart.wav` | kurz vor Software-Neustart |

PCM-WAV, 16 Bit, Mono oder Stereo. Fehlt eine Datei, bleibt dieser Clip stumm.

## Arduino IDE

1. [Arduino IDE 2](https://www.arduino.cc/en/software) installieren.
2. Datei → Einstellungen → Zusätzliche Boardverwalter-URLs:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

3. Werkzeuge → Board → Boardverwalter: **esp32** von Espressif Systems installieren.
4. Sketch → Bibliothek einbinden → Bibliotheken verwalten: **XPowersLib** (Lewis He) installieren.
5. Ordner `Bilderrahmen` öffnen (Datei `Bilderrahmen.ino`).
6. Werkzeuge wie unten setzen, Port wählen, Hochladen.

### Werkzeuge (ESP32S3 Dev Module)

Diese Werte müssen stimmen. Falsches PSRAM oder zu kleine App-Partition: Bootschleife oder der Sketch passt nicht.

| Menü | Einstellung |
| --- | --- |
| Board | **ESP32S3 Dev Module** |
| USB CDC On Boot | **Enabled** |
| USB Mode | **Hardware CDC and JTAG** |
| USB DFU On Boot | Disabled |
| CPU Frequency | 240 MHz (WiFi/BT) |
| Flash Mode | QIO 80 MHz |
| Flash Size | **16MB (128Mb)** |
| Partition Scheme | **16M Flash (3MB APP/9.9MB FATFS)** |
| PSRAM | **OPI PSRAM** |
| Upload Mode | UART0 / Hardware CDC |
| Upload Speed | 921600 |
| Arduino Runs On | Core 1 |
| Events Run On | Core 1 |
| Erase All Flash Before Sketch Upload | Disabled |

Fehlt der Eintrag „16M Flash (3MB APP/9.9MB FATFS)“, nimm **Huge APP (3MB No OTA/1MB SPIFFS)**. Die Daten liegen auf der SD, nicht im Flash-Dateisystem. Nach Wechsel des Partitionsschemas können WLAN-Daten weg sein — dann einmal WLAN-Setup.

USB-C am Rahmen wählen (nativer USB des ESP32-S3). BOOT-Taste nur, wenn der Upload den Chip nicht selbst startet.

Der Serial Monitor ist optional. Schließen der Arduino IDE startet den Chip oft neu (USB CDC / DTR) — das ist normal.

## Erstes WLAN

Ohne gespeichertes Netz: Access Point **Tintenklecks**, Passwort `tintenklecks`.

Im Browser: [http://192.168.4.1](http://192.168.4.1) — Heimnetz eintragen oder nur AP-Passwort setzen, Speichern & neu starten.

Mit WLAN: IP aus dem Router oder später [http://tintenklecks.local](http://tintenklecks.local).

## Web

| Seite | Zweck |
| --- | --- |
| Menü | Einstieg |
| Neues Bild (Studio) | Foto, Zuschnitt, Sierra oder Lab, Beschriftung, Anzeigen oder Speichern |
| Bilder | Galerie: bearbeiten, am Rahmen zeigen, umbenennen, löschen |
| Live-Anzeige | aktuelles Bild im Holzrahmen, blättern, am Gerät zeigen |
| Rahmen | Intervall oder täglich, Geburtstag/Sterbetag |
| System | Akku, ntfy, Neustart |
| WLAN-Setup | Netz oder AP-Passwort |

USB: der Rahmen bleibt wach. Akku: Deep Sleep nur bei Wechsel ≥ 10 min oder 1×/Tag, ohne Browser nach dem Bildwechsel, mit Browser nach 60 s Pause. Wecken: Timer, KEY (nächstes Bild) oder BOOT (Web, kein Bildwechsel).
