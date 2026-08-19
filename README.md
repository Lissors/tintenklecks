# Tintenklecks

Deutsch · [English](README.en.md)

Firmware für den **Waveshare ESP32-S3 PhotoPainter** (7,3″ Spectra-6 / E6, 480×800).
Zuschnitt, Verfahren und Beschriftung laufen im Browser auf dem Rahmen.
Die Galerie liegt auf der SD-Karte. Kein ESP-IDF, nur Arduino IDE.

© 2026 Ingo Lissors · Herkunft und Lizenzen: [CREDITS.txt](CREDITS.txt)

Sketch-Datei: `Bilderrahmen.ino` (Ordnername bleibt, sonst findet die Arduino IDE den Sketch nicht).

## Hardware

- Waveshare ESP32-S3 PhotoPainter, Spectra-6 / E6
- **Lautsprecher am Rahmen** (ES8311 + PA, sitzt auf der Platine — ohne ihn kein Ton)
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

## Bedienung

Im Browser die IP des Rahmens öffnen: Access Point [http://192.168.4.1](http://192.168.4.1), im Heimnetz die IP aus dem Router oder später [http://tintenklecks.local](http://tintenklecks.local). Lange Vorgänge (Anzeigen, Speichern, Wechsel) zeigen **Bitte warten…** — in der Zeit sind die Knöpfe gesperrt.

### Kopfzeile (alle Seiten)

Oben links **Tintenklecks**: auf Unterseiten ein Link zurück ins Hauptmenü, auf dem Menü nur der Name.

Rechts **Akku … %**, bei Ladung **· lädt**, oder **USB-Betrieb**. Daneben **zzz** — nur ohne USB sichtbar. Der Knopf schickt den Rahmen sofort in den Deep Sleep. Status-Abfragen im Browser halten den Rahmen nicht wach.

### Tasten am Gerät

| Taste | Wach | Aus dem Deep Sleep |
| --- | --- | --- |
| **KEY** | nächstes Zufallsbild (wie „Jetzt wechseln“) | wecken, Bild wechseln, wieder schlafen |
| **BOOT** | — | Web an, **kein** Bildwechsel |

### USB und Akku

USB: der Rahmen bleibt wach.

Akku und Wechsel **≥ 10 Minuten** oder **1× pro Tag**: ohne offenes Web direkt nach dem Bildwechsel Deep Sleep, mit Browser nach 60 Sekunden Pause. **5 Minuten**-Intervall: bleibt wach. Wecken: nächster Timer, KEY oder BOOT.

### Hauptmenü

![Hauptmenü](docs/menu.png)

Sechs Kacheln:

| Kachel | Seite |
| --- | --- |
| Live-Anzeige | aktuelles Bild im Holzrahmen, Text, blättern |
| Neues Bild | Studio: Foto, Zuschnitt, Verfahren, Beschriftung |
| Bilder | Galerie: anzeigen, bearbeiten, umbenennen, löschen |
| Rahmen | Uhr, Intervall oder täglich, Geburtstag/Sterbetag |
| System | Status, ntfy, Panel, Neustart, WLAN vergessen |
| WLAN-Setup | Heimnetz und AP-Passwort |

### Neues Bild (Studio)

![Studio](docs/studio.png)

Zwei Bühnen: links **Zuschnitt**, rechts **E6 Vorschau**. Das Foto in das gestrichelte Feld ziehen oder tippen (JPG, PNG, BMP). Mausrad zoomt, Ziehen auf der linken Bühne verschiebt den Ausschnitt.

**Gerendertes BMP an E6** umgeht das Studio: eine schon fertige 24-Bit-BMP in Panelgröße (480×800 oder 800×480) direkt aufs E-Paper.

Beim **Bearbeiten** aus der Galerie steht **Bearbeitung beenden · neues Bild**. Solange die Bearbeitung läuft, nimmt das Drop-Feld kein neues Foto.

#### Verfahren

**Methode Sierra** (Vorgabe bei neuen Bildern): Zoom und Ausschnitt zuerst, dann **In E6 konvertieren**. Ohne diesen Schritt weder Anzeigen noch Speichern. Schieber Vorbereitung/Dither und **Automatik** sind hier unsichtbar.

**Methode Lab**: Schieber unter **Vorbereitung** und **E-Paper / Dither**. **Automatik** sucht Werte zum aktuellen Zuschnitt. Alte Galeriebilder ohne gespeicherte Methode öffnen als Lab und behalten ihre Schieber.

Vorbereitung (nur Lab): Belichtung, Sättigung, S-Kurve, Lichter stauchen, Schatten, Mitte, Häkchen **Tonumfang ins Papier**.

E-Paper / Dither (nur Lab): Helligkeit, Kontrast, Wärme, Dither %, Algorithmus Atkinson / Floyd–Steinberg / Stucki / Ohne.

#### Format und Ausgabe

**Ausrichtung:** Portrait 480×800 oder Landscape 800×480. **Zoom** 10–400.

**Anzeigen am Rahmen** schreibt nur aufs Panel, nicht in die Galerie. **Speichern in Galerie** legt BMP, Zuschnitt und Vorschaubild auf der SD ab. Ändert man an einem gespeicherten Bild nur Text oder Person, speichert der Rahmen die Metadaten, ohne neu zu dithern.

#### Person

Name, Geburtsdatum, Sterbedatum (`TT.MM.JJJJ`). Häkchen **Name auf Bild**, **Geburt auf Bild**, **Tod auf Bild** und Schieber **Größe Name** / **Größe Daten**. Die Daten steuern in der Rahmeneinstellung den Geburtstag/Sterbetag. Name und Daten lassen sich auf dem Zuschnitt verschieben.

#### Bildbeschreibung

Freitext nur für die Live-Anzeige, nicht auf dem Panel.

#### Freier Text

Häkchen **Beschriftung anzeigen** (aus: Name, Daten und Freitexte unsichtbar). Felder: Text, Schrift Serif / Sans / Schnörkel, Fett, Größe, Drehung (−180° bis 180°), Farbe Weiß / Schwarz / Gelb / Rot / Blau / Grün, Ausrichtung Mitte / Links / Rechts.

**Freitext hinzufügen**, in der Liste auswählen, auf dem Zuschnitt verschieben. **Ausgewählten Text löschen** oder **Alle Freitexte löschen**.

### Bilder (Galerie)

![Galerie](docs/galerie.png)

Karten nach Personenname. Vorschau mit Beschriftung wie im Studio, darunter Name, `*` Geburt, `†` Tod, Beschreibung (Tipp klappt den ganzen Text auf), Dateiname.

| Knopf | Wirkung |
| --- | --- |
| Bearbeiten | Studio mit Zuschnitt und gespeicherten Werten |
| Anzeigen | dieses BMP aufs Panel |
| Umbenennen | Dateiname ohne `.bmp` |
| Löschen | BMP plus JSON, Vorschauen und zugehörigen Ton (Nachfrage) |

Fehlt die Vorschau: **Kein Vorschaubild · neu speichern**. Ohne Zuschnitt-Datei ein roter Rand und **Kein Zuschnitt**. Der Index baut sich nach dem Start im Hintergrund; die Seite wartet darauf (**Galerie-Index wird gebaut…**). Fehlende Vorschaubilder versucht die Seite selbst nachzuziehen.

### Live-Anzeige

![Live-Anzeige](docs/live.png)

Holzrahmen-Vorschau des gewählten Bildes samt Text auf dem Bild. Darunter Name, Daten, Bildbeschreibung, Freitexte. **‹** / **›** blättern, Zähler in der Mitte.

**Am Rahmen anzeigen** schickt genau dieses Bild aufs Panel. Hängt es schon, steht der Knopf **Am Rahmen (aktuell)** und ist gesperrt.

Leere Galerie: **Kein Bild**. Nur Thumb statt Zuschnitt: **Kleine Vorschau · Zuschnitt nicht geladen**.

### Rahmen

![Rahmen](docs/rahmen.png)

Zuerst die Uhr. **Suchen** filtert die Stadtliste. Stadt wählen, **Standort speichern** (Sommerzeit, danach NTP). Zeit: Feld **Datum & Zeit** und **Uhr setzen**, oder **Von diesem Gerät übernehmen**. Ohne gültige Chip-Zeit kein Tageswechsel.

**Wechsel:** Modus **Aus**, **Im Intervall** (5 / 10 / 30 / 60 Minuten) oder **1× pro Tag** (Uhrzeit, Vorgabe 08:00). **Speichern** merkt den Modus. **Jetzt wechseln** sofort.

Reihenfolge: Geburt oder Tod morgen oder übermorgen — bis zu drei Bilder übereinander. Sonst Bilder ohne diese Daten, ohne Zurücklegen, bis der Topf leer ist, dann neu mischen. USB bleibt wach. Akku schläft wie unter „USB und Akku“.

### System

![System](docs/system.png)

**Status:** Gerät, IP, Modus (AP oder Heimnetz), SD, Akku, Heap.

**ntfy:** Thema oder volle URL (leer = aus), Priorität Min / Niedrig / Normal / Hoch / Dringend. **Speichern**, **Probe senden**. Es geht nur eine Meldung pro Kalendertag bei Akku unter 10 %, keine Wachmeldung. App [ntfy](https://ntfy.sh/), dasselbe Thema abonnieren.

**Anzeige:** **Panel leeren (weiß)** — Bilder danach wieder über Galerie **Anzeigen**. **Akkuwarnung testen** legt „Akku < 10 %“ unten rechts auf das aktuelle Bild, unabhängig vom echten Stand (und sendet die ntfy-Probe, wenn ein Thema gesetzt ist).

**Wartung:** **Neustart** (spielt `neustart.wav`). **Jetzt aufräumen** entfernt Reste gelöschter Bilder (passiert sonst automatisch beim Start und nach jedem Löschen). **WLAN-Daten löschen & Neustart** vergisst das Heimnetz (Nachfrage), danach wieder Access Point.

### WLAN-Setup

![WLAN-Setup](docs/wlan.png)

**SSID (Heimnetz)**, **WLAN-Passwort**, **AP-Passwort (Hotspot)** — Auge zeigt oder verdeckt das Passwort. AP-Passwort mindestens 8 Zeichen (WPA2), Vorgabe `tintenklecks`. Gilt nach dem Neustart. Hotspot heißt **Tintenklecks**.

**Speichern & neu starten**. **Offline · Menü öffnen** bleibt ohne Speichern im Menü.
