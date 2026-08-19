# Tintenklecks

[Deutsch](README.md) · English

Firmware for the **Waveshare ESP32-S3 PhotoPainter** (7.3″ Spectra-6 / E6, 480×800).
Crop, rendering and captions run in the browser on the frame.
The gallery lives on the SD card. No ESP-IDF — Arduino IDE only.

© 2026 Ingo Lissors · Origin and licenses: [CREDITS.txt](CREDITS.txt)

Sketch file: `Bilderrahmen.ino` (keep that folder name, or the Arduino IDE will not find the sketch).

The web UI is German. Button and field names below match the screen.

## Hardware

- Waveshare ESP32-S3 PhotoPainter, Spectra-6 / E6
- **Speaker on the frame** (ES8311 + PA, on the board — no sound without it)
- Micro-SD, **FAT32**
- USB-C on the frame (flashing and web)
- optional battery (AXP2101)

## Set up the SD card

Without a card the web starts, but pictures and sound are missing.

1. Format the card on a PC as **FAT32** (one partition).
2. Create two folders and copy the included WAV files:

```
sound/willkommen.wav
sound/wlan.wav
sound/ap.wav
sound/neustart.wav
pic/
```

The four WAV files are in the `sound/` folder of this download. Copy them into `sound/` on the card. Do not rename them.

`pic/` stays empty. Pictures are created in Studio on the frame. There are **no** bundled BMP, JPG or thumbnail files.

3. Insert the card, **then** flash the firmware.

| File | When |
| --- | --- |
| `willkommen.wav` | every reset/reboot (not after deep sleep) |
| `wlan.wav` | home Wi-Fi connected |
| `ap.wav` | no Wi-Fi, access point |
| `neustart.wav` | just before a software restart |

PCM WAV, 16-bit, mono or stereo. If a file is missing, that clip stays silent.

## Arduino IDE

1. Install [Arduino IDE 2](https://www.arduino.cc/en/software).
2. File → Preferences → Additional boards manager URLs:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```

3. Tools → Board → Boards Manager: install **esp32** by Espressif Systems.
4. Sketch → Include Library → Manage Libraries: install **XPowersLib** (Lewis He).
5. Open the `Bilderrahmen` folder (`Bilderrahmen.ino`).
6. Set Tools as below, pick the port, Upload.

### Tools (ESP32S3 Dev Module)

These values must match. Wrong PSRAM or too small an app partition: boot loop, or the sketch will not fit.

| Menu | Setting |
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

If there is no “16M Flash (3MB APP/9.9MB FATFS)” entry, use **Huge APP (3MB No OTA/1MB SPIFFS)**. Data lives on the SD card, not in the flash filesystem. After changing the partition scheme, saved Wi-Fi may be gone — run WLAN-Setup once.

Use USB-C on the frame (native USB of the ESP32-S3). Press BOOT only if the upload does not start the chip by itself.

The Serial Monitor is optional. Closing the Arduino IDE often restarts the chip (USB CDC / DTR) — that is normal.

## First Wi-Fi

With no saved network: access point **Tintenklecks**, password `tintenklecks`.

In the browser: [http://192.168.4.1](http://192.168.4.1) — enter the home network, or only set the AP password, then Speichern & neu starten.

With Wi-Fi: the IP from the router, or later [http://tintenklecks.local](http://tintenklecks.local).

## Using the web UI

Open the frame’s IP in the browser: access point [http://192.168.4.1](http://192.168.4.1), on the home network the router IP, or later [http://tintenklecks.local](http://tintenklecks.local). Long jobs (show, save, switch) display **Bitte warten…** — buttons are locked until they finish.

### Header (every page)

Top left **Tintenklecks**: on subpages this is a link back to the main menu; on the menu it is only the name.

Right: **Akku … %**, while charging **· lädt**, or **USB-Betrieb**. Next to that **zzz** — visible only without USB. The button puts the frame into deep sleep immediately. Status polling in the browser does not keep the frame awake.

### Buttons on the device

| Button | Awake | From deep sleep |
| --- | --- | --- |
| **KEY** | next random picture (same as „Jetzt wechseln“) | wake, switch picture, sleep again |
| **BOOT** | — | web on, **no** picture change |

### USB and battery

USB: the frame stays awake.

Battery and change interval **≥ 10 minutes** or **1× per day**: without an open web page, deep sleep right after the picture change; with a browser, after 60 seconds idle. **5 minute** interval: stays awake. Wake: next timer, KEY or BOOT.

### Hauptmenü (main menu)

![Main menu](docs/menu.png)

Six tiles:

| Tile | Page |
| --- | --- |
| Live-Anzeige | current picture in the wooden frame, text, browse |
| Neues Bild | Studio: photo, crop, method, captions |
| Bilder | gallery: show, edit, rename, delete |
| Rahmen | clock, interval or daily, birthday/death day |
| System | status, ntfy, panel, restart, forget Wi-Fi |
| WLAN-Setup | home network and AP password |

### Neues Bild (Studio)

![Studio](docs/studio.png)

Two stages: left **Zuschnitt**, right **E6 Vorschau**. Drop a photo onto the dashed field, or tap (JPG, PNG, BMP). Mouse wheel zooms; drag on the left stage to pan the crop.

**Gerendertes BMP an E6** skips Studio: a finished 24-bit BMP at panel size (480×800 or 800×480) goes straight to the e-paper.

While **Bearbeiten** from the gallery, **Bearbeitung beenden · neues Bild** is shown. Until you end the edit, the drop field will not take a new photo.

#### Verfahren (method)

**Methode Sierra** (default for new pictures): zoom and crop first, then **In E6 konvertieren**. Without that step, neither show nor save. Preparation/dither sliders and **Automatik** are hidden here.

**Methode Lab**: sliders under **Vorbereitung** and **E-Paper / Dither**. **Automatik** searches values for the current crop. Older gallery pictures with no saved method open as Lab and keep their sliders.

Vorbereitung (Lab only): Belichtung, Sättigung, S-Kurve, Lichter stauchen, Schatten, Mitte, checkbox **Tonumfang ins Papier**.

E-Paper / Dither (Lab only): Helligkeit, Kontrast, Wärme, Dither %, algorithm Atkinson / Floyd–Steinberg / Stucki / Ohne.

#### Format and output

**Ausrichtung:** Portrait 480×800 or Landscape 800×480. **Zoom** 10–400.

**Anzeigen am Rahmen** writes to the panel only, not the gallery. **Speichern in Galerie** stores BMP, crop and thumbnail on the SD card. If you change only text or person data on a saved picture, the frame stores the metadata without dithering again.

#### Person

Name, birth date, death date (`TT.MM.JJJJ`). Checkboxes **Name auf Bild**, **Geburt auf Bild**, **Tod auf Bild** and sliders **Größe Name** / **Größe Daten**. Those dates drive birthday/death-day in the frame settings. Name and dates can be dragged on the crop.

#### Bildbeschreibung (picture description)

Free text for Live-Anzeige only, not on the panel.

#### Freier Text (free text)

Checkbox **Beschriftung anzeigen** (off: name, dates and free texts hidden). Fields: text, typeface Serif / Sans / Schnörkel, Fett, size, rotation (−180° to 180°), colour Weiß / Schwarz / Gelb / Rot / Blau / Grün, alignment Mitte / Links / Rechts.

**Freitext hinzufügen**, select in the list, drag on the crop. **Ausgewählten Text löschen** or **Alle Freitexte löschen**.

### Bilder (gallery)

![Gallery](docs/galerie.png)

Cards sorted by person name. Preview with captions as in Studio, then name, `*` birth, `†` death, description (tap to expand), filename.

| Button | Effect |
| --- | --- |
| Bearbeiten | Studio with crop and saved values |
| Anzeigen | this BMP on the panel |
| Umbenennen | filename without `.bmp` |
| Löschen | BMP plus JSON, thumbnails and related sound (asks first) |

Missing preview: **Kein Vorschaubild · neu speichern**. Without a crop file: red border and **Kein Zuschnitt**. The index is built in the background after start; the page waits (**Galerie-Index wird gebaut…**). The page tries to rebuild missing thumbnails itself.

### Live-Anzeige

![Live display](docs/live.png)

Wooden-frame preview of the chosen picture, including text on the image. Below: name, dates, description, free texts. **‹** / **›** browse, counter in the middle.

**Am Rahmen anzeigen** sends exactly this picture to the panel. If it is already showing, the button reads **Am Rahmen (aktuell)** and is disabled.

Empty gallery: **Kein Bild**. Thumbnail instead of crop: **Kleine Vorschau · Zuschnitt nicht geladen**.

### Rahmen (frame)

![Frame](docs/rahmen.png)

Clock first. **Suchen** filters the city list. Pick a city, **Standort speichern** (daylight saving, then NTP). Time: **Datum & Zeit** and **Uhr setzen**, or **Von diesem Gerät übernehmen**. Without a valid chip time, there is no daily switch.

**Wechsel:** mode **Aus**, **Im Intervall** (5 / 10 / 30 / 60 minutes) or **1× pro Tag** (time of day, default 08:00). **Speichern** keeps the mode. **Jetzt wechseln** immediately.

Order: birth or death tomorrow or the day after — up to three pictures stacked. Otherwise pictures without those dates, without replacement, until the pot is empty, then reshuffle. USB stays awake. Battery sleeps as under “USB and battery”.

### System

![System](docs/system.png)

**Status:** device, IP, mode (AP or home network), SD, battery, heap.

**ntfy:** topic or full URL (empty = off), priority Min / Niedrig / Normal / Hoch / Dringend. **Speichern**, **Probe senden**. One message per calendar day when battery is under 10 %, no wake-up message. App [ntfy](https://ntfy.sh/), subscribe to the same topic.

**Anzeige:** **Panel leeren (weiß)** — show pictures again from the gallery with **Anzeigen**. **Akkuwarnung testen** puts “Akku < 10 %” at the bottom right of the current picture, regardless of the real level (and sends the ntfy probe if a topic is set).

**Wartung:** **Neustart** (plays `neustart.wav`). **Jetzt aufräumen** removes leftovers of deleted pictures (otherwise automatic at start and after each delete). **WLAN-Daten löschen & Neustart** forgets the home network (asks first), then access point again.

### WLAN-Setup

![Wi-Fi setup](docs/wlan.png)

**SSID (Heimnetz)**, **WLAN-Passwort**, **AP-Passwort (Hotspot)** — the eye shows or hides the password. AP password at least 8 characters (WPA2), default `tintenklecks`. Takes effect after restart. The hotspot is named **Tintenklecks**.

**Speichern & neu starten**. **Offline · Menü öffnen** stays in the menu without saving.
