# Tintenklecks

[Deutsch](README.md) · English

Firmware for the **Waveshare ESP32-S3 PhotoPainter** (7.3″ Spectra-6 / E6, 480×800).
The frame exists so **special days are not forgotten**: birthdays, death days and other anniversaries (wedding, first meeting …). Two pots: **Zufall** (random) and **Erinnerungen** (memories). Memories come up on their own when the day is tomorrow or the day after — up to three pictures stacked. Crop, rendering and captions run in the browser on the frame. The gallery lives on the SD card. No ESP-IDF — Arduino IDE only.

© 2026 Ingo Lissors · Origin and licenses: [CREDITS.txt](CREDITS.txt) · [LICENSE](LICENSE)

Public-domain demo: [`examples/mona_lisa.bmp`](examples/mona_lisa.bmp) (Leonardo da Vinci, *Mona Lisa*).
Further demo: [`examples/peter.bmp`](examples/peter.bmp) (person, captions, bold script) — see [`examples/peter.SOURCE.md`](examples/peter.SOURCE.md).

Sketch file: `Bilderrahmen.ino` (keep that folder name, or the Arduino IDE will not find the sketch).

The web UI is German. Button and field names below match the screen.

## Reminders — birth, death, special date, captions

In Studio under **Bildart** choose **Normal** or **Erinnerung**. Normal stays in the random pot. Erinnerung leaves the random pot — whether or not a date is filled in yet. With **Geburtsdatum**, **Sterbedatum** or **Besonderes Datum** (`TT.MM.JJJJ`) it is tied to that anniversary: the timer shows it **tomorrow or the day after**, up to **three** stacked. A special date has an **Anlass** (e.g. Hochzeitstag, Kennenlerntag). The chip clock must be valid, and under Rahmen a change mode (**Im Intervall** or **1× pro Tag**) must be on.

**KEY** and **Jetzt wechseln** take only Zufall, never Erinnerungen. Old JSON without a `kind` field: a date present means Erinnerung, otherwise Zufall.

On the picture itself: name, `*` birth, `†` death, occasion plus date, plus free notes (e.g. “In Erinnerung”). The **Bildbeschreibung** appears only in Live-Anzeige, not on the panel.

![Captions in Studio](docs/beschriftung.png)

Name, dates, checkboxes **Name auf Bild** / **Geburt auf Bild** / **Tod auf Bild** / **Besonderes auf Bild**. The dates control *when* a memory appears, not whether it is random. Whether the text is drawn on the panel does not change that.

![Free text and notes](docs/hinweistexte.png)

Free texts, typeface, **Fett**, rotation, colour. The list shows name, birth, death, special date and notes.

![Live with person, dates and note](docs/live_person.png)

Live-Anzeige: picture with text, then name, dates, description and free texts below.

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

`pic/` stays empty unless you copy the demos. Pictures are otherwise created in Studio. [`examples/`](examples/) has ready gallery sets — copy BMP, JSON, `_src.jpg` and `_thumb.jpg` into `pic/`:

- `mona_lisa.*` — Leonardo’s *Mona Lisa* (public domain, Wikimedia). [`SOURCE`](examples/mona_lisa.SOURCE.md)
- `peter.*` — example with name, dates and bold script type. [`SOURCE`](examples/peter.SOURCE.md)

3. Insert the card, **then** flash the firmware.

| File | When |
| --- | --- |
| `willkommen.wav` | every reset/reboot (not after deep sleep) |
| `wlan.wav` | home Wi-Fi connected |
| `ap.wav` | no Wi-Fi, access point |
| `neustart.wav` | just before a software restart |

PCM WAV, 16-bit, mono or stereo. If a file is missing, that clip stays silent.

## Ready-made firmware (no Arduino)

For the frame you only need **USB-C**, the file `tintenklecks-merged.bin` from [Releases](https://github.com/Lissors/tintenklecks/releases), then the SD card as above.

1. Insert the SD card (`sound/` folder, empty `pic/` or the demos).
2. [Latest release](https://github.com/Lissors/tintenklecks/releases/latest) → download `tintenklecks-merged.bin`.
3. Chrome or Edge: [Adafruit WebSerial ESPTool](https://adafruit.github.io/Adafruit_WebSerial_ESPTool/). USB-C on the frame, **Connect**, pick the file, offset **0x0**, **Program**. No port: hold BOOT, plug in USB, release.
4. Access point **Tintenklecks** / password `tintenklecks` → [http://192.168.4.1](http://192.168.4.1) for the home network.

No Arduino IDE. Pictures reach the card in Studio.

The maintainer builds the `.bin` in the Arduino IDE (below). If there is no release file yet, skip this section and flash with Arduino.

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

If there is no “16M Flash (3MB APP/9.9MB FATFS)” entry, use **Huge APP (3MB No OTA/1MB SPIFFS)**. Then there is **no OTA**. Data lives on the SD card, not in the flash filesystem. After changing the partition scheme, saved Wi-Fi may be gone — run WLAN-Setup once.

Use USB-C on the frame (native USB of the ESP32-S3). Press BOOT only if the upload does not start the chip by itself.

**Later without USB (Arduino users only):** flash this sketch once over USB. Frame on the home network and **awake** (USB plugged in, or not in deep sleep). In the Arduino IDE under **Port** pick **tintenklecks** (network), then Upload as usual. Deep sleep: no network port — BOOT or USB, then try again.

**`.bin` for others (Releases):** Tools as above, then Sketch → **Export Compiled Binary**. In the sketch folder (or under `build/`) find **`Bilderrahmen.ino.merged.bin`** — bootloader + partitions + app in one file. Rename to `tintenklecks-merged.bin` and attach it on GitHub → Releases. Do not ship the small `.ino.bin` alone (an empty chip will not boot without the bootloader).

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
| **KEY** | next random picture (same as „Jetzt wechseln“), no memories | wake, random picture, sleep again |
| **BOOT** | — | web on, **no** picture change |

### USB and battery

USB: the frame stays awake.

Battery and change interval **≥ 10 minutes** or **1× per day**: without an open web page, deep sleep right after the picture change; with a browser, after 60 seconds idle. **5 minute** interval: stays awake. Wake: next timer, KEY or BOOT.

### Hauptmenü (main menu)

![Main menu](docs/menu.png)

Six tiles:

| Tile | Page |
| --- | --- |
| Live-Anzeige | current picture in the wooden frame, text, browse (Zufall / Erinnerungen) |
| Neues Bild | Studio: photo, crop, method, captions |
| Bilder | gallery: Zufall and Erinnerungen, show, edit, rename, delete |
| Rahmen | clock, interval or daily; timer takes memories first |
| System | status, ntfy, hang, panel, restart, forget Wi-Fi |
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

**Lage (System):** Hochkant 480×800 or Quer 800×480 — under System → Anzeige (**Rahmenlage**); Studio only displays it. New photos follow that hang, loaded gallery pictures keep theirs. Existing BMPs are not converted. **Zoom** 10–400.

**Anzeigen am Rahmen** writes to the panel only, not the gallery. **Speichern in Galerie** stores BMP, crop and thumbnail on the SD card. If you change only text or person data on a saved picture, the frame stores the metadata without dithering again.

#### Bildart and person

![Captions in Studio](docs/beschriftung.png)

**Art:** **Normal** (random pot) or **Erinnerung** (out of the random pot). Date fields only for Erinnerung: birth date, death date, special date (`TT.MM.JJJJ`) and occasion (Hochzeitstag, Kennenlerntag …). Checkboxes **Name auf Bild**, **Geburt auf Bild**, **Tod auf Bild**, **Besonderes auf Bild** and sliders **Größe Name** / **Größe Daten**. Name and dates can be dragged on the crop.

Without a matching anniversary (tomorrow / day after) a memory stays off the panel — the timer then falls back to Zufall. KEY and **Jetzt wechseln** always do that.

#### Bildbeschreibung (picture description)

Free text for Live-Anzeige only, not on the panel. There it sits under the wooden frame, with name and dates.

#### Freier Text (free text)

![Free text](docs/hinweistexte.png)

Checkbox **Beschriftung anzeigen** (off: name, dates and free texts hidden). Fields: text, typeface Serif / Sans / Schnörkel, Fett, size, rotation (−180° to 180°), colour Weiß / Schwarz / Gelb / Rot / Blau / Grün, alignment Mitte / Links / Rechts.

**Freitext hinzufügen**, select in the list, drag on the crop. **Ausgewählten Text löschen** or **Alle Freitexte löschen**. **Fett** applies to the selected text (name, dates or free text), including Schnörkel. Typical: “In Erinnerung”.

### Bilder (gallery)

![Gallery](docs/galerie.png)

Two tabs: **Zufall** and **Erinnerungen**. Cards sorted by person name. Preview with captions as in Studio, then name, `*` birth, `†` death, occasion plus special date, description (tap to expand), filename.

| Button | Effect |
| --- | --- |
| Bearbeiten | Studio with crop and saved values |
| Anzeigen | this BMP on the panel |
| Umbenennen | filename without `.bmp` |
| Löschen | BMP plus JSON, thumbnails and related sound (asks first) |

Missing preview: **Kein Vorschaubild · neu speichern**. Without a crop file: red border and **Kein Zuschnitt**. The index is built in the background after start; the page waits (**Galerie-Index wird gebaut…**). The page tries to rebuild missing thumbnails itself.

### Live-Anzeige

![Live display](docs/live.png)

Wooden-frame preview of the chosen picture, including text on the image (thumbnail; the crop is already in it). Below: name, dates, description, free texts. Tabs **Zufall** / **Erinnerungen**, **‹** / **›** browse, counter in the middle. The mock-up follows the picture’s hang (Hochkant or Quer).

![Live with person](docs/live_person.png)

**Am Rahmen anzeigen** sends exactly this picture to the panel. If it is already showing, the button reads **Am Rahmen (aktuell)** and is disabled.

Empty list: **Kein Bild**.

### Rahmen (frame)

![Frame](docs/rahmen.png)

Clock first. **Suchen** filters the city list. Pick a city, **Standort speichern** (daylight saving, then NTP). Time: **Datum & Zeit** and **Uhr setzen**, or **Von diesem Gerät übernehmen**. Without a valid chip time, there is no daily switch.

**Wechsel:** mode **Aus**, **Im Intervall** (5 / 10 / 30 / 60 minutes) or **1× pro Tag** (time of day, default 08:00). **Speichern** keeps the mode. **Jetzt wechseln** immediately — Zufall only, no memories. **Aus** = no timer.

**Timer (interval / clock / wake for a change):** if a memory’s birth, death or special date is **tomorrow or the day after**, those pictures come first — up to three stacked. Otherwise only Zufall, without replacement, until the pot is empty, then reshuffle. If there is no due memory and no Zufall picture, the panel stays. USB stays awake. Battery sleeps as under “USB and battery”.

### System

![System](docs/system.png)

**Status:** device, IP, mode (AP or home network), SD, battery, heap.

**ntfy:** topic or full URL (empty = off), priority Min / Niedrig / Normal / Hoch / Dringend. **Speichern**, **Probe senden**. One message per calendar day when battery is under 10 %, no wake-up message. App [ntfy](https://ntfy.sh/), subscribe to the same topic.

**Anzeige:** **Rahmenlage** Hochkant or Quer, **Lage speichern**. Applies to new pictures in Studio, Live and on the panel; existing pictures stay as they are. Hang the device to match. **Panel leeren (weiß)** — show pictures again from the gallery with **Anzeigen**. **Akkuwarnung testen** puts “Akku < 10 %” at the bottom right of the current picture, regardless of the real level (and sends the ntfy probe if a topic is set).

**Wartung:** **Neustart** (plays `neustart.wav`). **Jetzt aufräumen** removes leftovers of deleted pictures (otherwise automatic at start and after each delete). **WLAN-Daten löschen & Neustart** forgets the home network (asks first), then access point again.

### WLAN-Setup

![Wi-Fi setup](docs/wlan.png)

**SSID (Heimnetz)**, **WLAN-Passwort**, **AP-Passwort (Hotspot)** — the eye shows or hides the password. AP password at least 8 characters (WPA2), default `tintenklecks`. Takes effect after restart. The hotspot is named **Tintenklecks**.

**Speichern & neu starten**. **Offline · Menü öffnen** stays in the menu without saving.

## License

**Firmware & UI:** see [`LICENSE`](LICENSE) — **non-commercial**. Third-party origin: [CREDITS.txt](CREDITS.txt).

| Allowed | Not allowed |
|---------|-------------|
| Download; exact unmodified copies | **Commercial use** (sell, paid service, commercial product) |
| Flash and use privately (your own frame) | Shipping a commercial product based on this code |
| | Redistributing modifications; relicense / remove notices |

Goal: others may look at and copy the project, and use it privately — they may **not** commercialize it.

Demo images only where marked otherwise: [`examples/mona_lisa.bmp`](examples/mona_lisa.bmp) (public domain, Wikimedia) and [`examples/peter.bmp`](examples/peter.bmp) (example from the copyright holder). See the `SOURCE.md` next to the files.

### Content policy (for this repo)

- Do **not** commit other people’s private photos or modern copyrighted images.
- Do **not** put further gallery BMPs from the SD card into git, except the marked demos.
