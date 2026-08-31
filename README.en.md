# Tintenklecks

<img src="docs/icon.png" width="96" height="96" alt="Tintenklecks"/>

[Deutsch](README.md) · English

Firmware for the **Waveshare ESP32-S3 PhotoPainter** (7.3″ Spectra-6 / E6, 480×800).
The frame exists so **special days are not forgotten**: birthdays, death days and other anniversaries (wedding, first meeting …). Two pots: **Zufall** (random) and **Erinnerungen** (memories). Memories come up on their own when the day is **today, tomorrow or the day after** — one full picture; if several are due, an arrow at the bottom right. Crop, rendering and captions run in the browser on the frame. The gallery lives on the SD card. No ESP-IDF — Arduino IDE only.

© 2026 Ingo Lissors · Origin and licenses: [CREDITS.txt](CREDITS.txt) · [LICENSE](LICENSE)

Public-domain demo: [`examples/mona_lisa.bmp`](examples/mona_lisa.bmp) (Leonardo da Vinci, *Mona Lisa*).
Further demo: [`examples/peter.bmp`](examples/peter.bmp) (person, captions, bold script) — see [`examples/peter.SOURCE.md`](examples/peter.SOURCE.md).

Sketch file: `Bilderrahmen.ino` (keep that folder name, or the Arduino IDE will not find the sketch).

The web UI is German. Button and field names below match the screen.

## Reminders — birth, death, special date, captions

In Studio under **Bildart** choose **Normal** or **Erinnerung**. Normal stays in the random pot. Erinnerung leaves the random pot — whether or not a date is filled in yet. With **Geburtsdatum**, **Sterbedatum** or **Besonderes Datum** (`TT.MM.JJJJ`) it is tied to that anniversary: the timer shows it **on the day, tomorrow or the day after**, **one** full picture at a time. If several are due: one at random, **arrow bottom right**, **KEY** and every **3 hours** the next. Occasion and special date are **two separate lines** on the picture. The chip clock must be valid, and under Rahmen a change mode (**Im Intervall** or **1× pro Tag**) must be on.

**KEY** and **Jetzt wechseln** otherwise take only Zufall. Only when several memories are due does KEY step through those not yet shown; after the last, Zufall again. Old JSON without a `kind` field: a date present means Erinnerung, otherwise Zufall.

On the picture itself: name, `*` birth, `†` death, occasion and date on separate lines, plus free notes (e.g. “In Erinnerung”). The **Bildbeschreibung** appears only in Live-Anzeige, not on the panel.

![Captions in Studio](docs/beschriftung.png)

Name, dates, checkboxes **Name auf Bild** / **Geburt auf Bild** / **Tod auf Bild** / **Besonderes auf Bild**. The dates control *when* a memory appears, not whether it is random. Whether the text is drawn on the panel does not change that.

![Free text and notes](docs/hinweistexte.png)

Free texts, typeface, **Fett**, rotation, colour. The list shows name, birth, death, occasion, date and notes.

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
| `willkommen.wav` | every reset/reboot (not after waking from deep sleep) |
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

Open the frame’s IP in the browser: access point [http://192.168.4.1](http://192.168.4.1), on the home network the router IP, or later [http://tintenklecks.local](http://tintenklecks.local). Long jobs (show, save, switch) display **Bitte warten…** — buttons are locked until they finish. Action buttons are tan; they darken briefly when pressed. **Zufall** / **Erinnerungen** in gallery and Live are switches. **Löschen** stays red.

### Header (every page)

Top left the **icon** and **Tintenklecks**: on subpages this is a link back to the main menu; on the menu it is only the name. The same icon is in the browser tab.

Right: **Akku … %**, while charging **· lädt**, or **USB-Betrieb**. Next to that, only without USB: **Wach bleiben** (frame will not sleep) and **zzz** (deep sleep immediately, even if Wach bleiben is on). An open tab (status every 15 s) keeps the frame awake.

### Buttons on the device

| Button | Awake | From deep sleep |
| --- | --- | --- |
| **KEY** | next picture (Zufall; if several memories are due, the next of those) | wake, switch, sleep again |
| **BOOT** | — | web on, **no** picture change |

### USB and battery

USB: the frame stays awake. Sound (welcome, WLAN, AP, restart) only on USB.

Battery and change interval **≥ 10 minutes** or **1× per day**: without an open web page, deep sleep right after the picture change; with a browser, after 60 seconds idle. **5 minute** interval or **Off**: stays awake. **Wach bleiben** in the header prevents sleep. Ping alone does not keep it awake; an open tab does. Wake: next timer, KEY or BOOT.

In sleep: unused AXP rails, panel (ALDO4), codec (ALDO3), speaker, sensor and SD off. The wake timer (ESP) is calibrated against the chip RTC (quartz) so the change time does not drift.

### Hauptmenü (main menu)

![Main menu](docs/menu.png)

Six tiles:

| Tile | Page |
| --- | --- |
| Live-Anzeige | current picture in the wooden frame, text, browse (Zufall / Erinnerungen) |
| Neues Bild | Studio: photo, crop, method, captions |
| Bilder | gallery: Zufall and Erinnerungen, show, edit, rename, delete |
| Rahmen | change, hang, pot; timer takes memories first |
| System | clock, status, ntfy, sound, backup, restart, forget Wi-Fi |
| WLAN-Setup | home network and AP password |

### Neues Bild (Studio)

![Studio](docs/studio.png)

Two stages: left **Zuschnitt**, right **E6 Vorschau**. Drop a photo onto the dashed field, or tap (JPG, PNG, BMP). Or **Blanko-Seite einfügen**: one of the six e-paper colours, size from the frame hang — a placeholder without a photo, with text and dates. Mouse wheel zooms; drag on the left stage to pan the crop — the picture follows the mouse (including vertically).

While **Bearbeiten** from the gallery, **Bearbeitung beenden** is shown. Until you end the edit, the drop field will not take a new photo. **E6 Vorschau** shows the stored picture unchanged, including text. Lab sliders and free texts come back as last saved. The picture is only re-rendered if crop or Lab values change.

#### Verfahren (method)

**Methode Sierra** (default for new pictures): zoom and crop first, then **In E6 konvertieren**. Without that step, neither show nor save. Preparation/dither sliders and **Automatik** are hidden here.

**Methode Lab**: sliders under **Vorbereitung** and **E-Paper / Dither**. **Automatik** searches values for the current crop. Older gallery pictures with no saved method open as Lab and keep their sliders.

Vorbereitung (Lab only): Belichtung, Sättigung, S-Kurve, Lichter stauchen, Schatten, Mitte, checkbox **Tonumfang ins Papier**.

E-Paper / Dither (Lab only): Helligkeit, Kontrast, Wärme, Dither %, algorithm Atkinson / Floyd–Steinberg / Stucki / Ohne.

#### Format and output

Hochkant 480×800 or Quer 800×480 is under Rahmen → Anzeige (**Rahmenlage**). New photos and blank pages follow that hang, loaded gallery pictures keep theirs. Existing BMPs are not converted. **Zoom** 10–400.

**Anzeigen am Rahmen** writes to the panel only, not the gallery. **Speichern in Galerie** stores BMP, crop and thumbnail on the SD card. If you change only text or person data on a saved picture, the frame stores the metadata without dithering again.

#### Bildart and person

![Captions in Studio](docs/beschriftung.png)

**Art:** **Normal** (random pot) or **Erinnerung** (out of the random pot). Date fields only for Erinnerung: birth date, death date, special date (`TT.MM.JJJJ`) and occasion (Hochzeitstag, Kennenlerntag …). On the picture two lines: occasion first, then the date. Checkboxes **Name auf Bild**, **Geburt auf Bild**, **Tod auf Bild**, **Besonderes auf Bild** and sliders **Größe Name** / **Größe Daten**. Name and dates can be dragged on the crop.

Without a matching anniversary (today / tomorrow / day after) a memory stays off the panel — the timer then falls back to Zufall. KEY and **Jetzt wechseln** do that too, except when several memories are due: those not yet shown, then Zufall again.

#### Bildbeschreibung (picture description)

Free text for Live-Anzeige only, not on the panel. There it sits under the wooden frame, with name and dates.

#### Freier Text (free text)

![Free text](docs/hinweistexte.png)

Checkbox **Beschriftung anzeigen** (off: name, dates and free texts hidden). Fields: text, typeface Serif / Sans / Schnörkel, Fett, size, rotation (−180° to 180°), colour Weiß / Schwarz / Gelb / Rot / Blau / Grün, alignment Mitte / Links / Rechts.

**Freitext hinzufügen**, select in the list, drag on the crop. **Ausgewählten Text löschen** or **Alle Freitexte löschen**. **Fett** applies to the selected text (name, dates or free text), including Schnörkel. Typical: “In Erinnerung”.

### Bilder (gallery)

![Gallery](docs/galerie.png)

Two tabs: **Zufall** and **Erinnerungen**. Below that **Suchen** (name, date, filename) — only after **Enter** or the **Suche** button; everything else is hidden. The **X** in the field clears the search and shows all pictures again. At the top: counts and the **Zufallstopf** (still *n* of *m*; if the pot is empty: next draw starts a new round). Cards sorted by person name. Preview with captions as in Studio, then name, `*` birth, `†` death, occasion and special date on separate lines, description (tap to expand), filename.

| Button | Effect |
| --- | --- |
| Bearbeiten | Studio: stored E6 picture, Lab values and text |
| Anzeigen | this BMP on the panel |
| Umbenennen | filename without `.bmp` |
| Löschen | BMP plus JSON, thumbnails and related sound (asks first). If that picture is on the panel, the next one is shown — same as KEY / Jetzt wechseln |

Missing preview: **Kein Vorschaubild · neu speichern**. Without a crop file: red border and **Kein Zuschnitt**. The index is a file on the SD card and is updated immediately on save, delete or rename — Zufall and Erinnerungen without a full rescan. Only if the file is missing (or after restore) is it rebuilt; then the page waits (**Galerie-Index wird gebaut…**). Leftovers without a picture (JPG/JSON without a BMP) are removed when the index is rebuilt. The page tries to rebuild missing thumbnails itself.

### Live-Anzeige

![Live display](docs/live.png)

Wooden-frame preview of the chosen picture, including text on the image (thumbnail; the crop is already in it). Below: name, dates, description, free texts, then a line **Nächste Erinnerung** (not inside the wooden frame). No filename — that is in the gallery. Tabs **Zufall** / **Erinnerungen**, **‹** / **›** browse, counter in the middle. Memories are sorted by name, as in the gallery. The mock-up follows the picture’s hang (Hochkant or Quer).

![Live with person](docs/live_person.png)

**Am Rahmen anzeigen** sends exactly this picture to the panel. If it is already showing, the button reads **Am Rahmen (aktuell)** and is disabled.

Empty list: **Kein Bild**.

### Rahmen (frame)

![Frame](docs/rahmen.png)

**Nächste Erinnerung** (today, tomorrow or the day after: date · name) and **Zufallstopf still n of m**. **Alle Bilder in die Auswahl** reshuffles the random pot (all Zufall pictures back in). **Index neu aufbauen** rescans the SD and sorts by name — save, delete and rename otherwise update the list immediately. Leftovers without a picture go away then too. Without a valid chip time, there is no daily switch.

**Wechsel:** mode **Aus**, **Im Intervall** (5 / 10 / 30 / 60 minutes) or **1× pro Tag** (time of day, default 08:00). **Speichern** keeps the mode. **Jetzt wechseln** immediately — Zufall only, no memories. **Aus** = no timer.

**Anzeige:** **Rahmenlage** Hochkant or Quer, **Lage speichern**. Applies to new pictures in Studio, Live and on the panel; existing pictures stay as they are. Hang the device to match.

**Timer (interval / clock / wake for a change):** if a memory’s birth, death or special date is **today, tomorrow or the day after**, **one** such picture comes first (full size, type as in Studio). Several due: arrow bottom right, next one every 3 hours (KEY likewise). Otherwise only Zufall, without replacement, until the pot is empty, then reshuffle. If there is no due memory and no Zufall picture, the panel stays. USB stays awake. Battery sleeps as under “USB and battery”.

On the panel itself there is no hint text: **battery under 10 %** as an icon bottom left, **more memories** as an arrow bottom right (white squares, black outline).

### System

![System](docs/system.png)

**Status:** device, IP, mode (AP or home network), SD, heap, NTP.

**Uhrzeit (Chip-RTC):** city in the dropdown, **Standort speichern** (daylight saving, then NTP). Time: **Datum & Zeit** and **Uhr setzen**, or **Von diesem Gerät übernehmen**. A hand-set clock is left alone for 10 minutes, then NTP fetches network time — only if a time server actually answered, not because the RTC already holds some valid time. NTP also runs on wake. Access point: no network time.

**Akku:** everything the AXP2101 reports — also on USB: cell present, percent, battery voltage, USB yes/no and USB voltage, system voltage, path (charge/discharge/standby), charge stage, charge-current and end-current setpoints, USB current limit, PMU temperature, thermal and input limiting, chip warning and shutdown thresholds. No measured current, no mAh. **Ladestrom** 100–1000 mA (chip default 300, maximum 1000) goes into the cell, not the ESP. USB must supply the frame plus charging.

**ntfy:** topic or full URL (empty = off), priority Min / Niedrig / Normal / Hoch / Dringend. **Speichern**, **Probe senden**. All notifications use the chosen priority. App [ntfy](https://ntfy.sh/).

**Ton:** USB only. Volume of the notice sounds 0–100, default 80. **Lautstärke speichern**. On battery the amplifier stays off.

**Sicherung:** **Sicherung als .txt** fetches `pic/` and `sound/` as one file (`tintenklecks-YYYY-MM-DD.txt`) — works in Chrome over HTTP. **Sicherung als .zip** the same, uncompressed; Chrome may block zip over HTTP, then use Firefox/Edge or the `.txt`. Leave USB plugged in. **Wiederherstellen:** `.txt` and uncompressed `.zip` in one go. Compressed zip still file by file. Same name overwrites, everything else stays. Firmware backup is the GitHub release.

**Wartung:** **Neustart** (plays `neustart.wav`). **WLAN-Daten löschen & Neustart** forgets the home network (asks first), then access point again.

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
