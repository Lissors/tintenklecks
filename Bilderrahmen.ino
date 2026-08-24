/*
 * Tintenklecks — Arduino IDE only (kein ESP-IDF)
 *
 * Board: ESP32S3 Dev Module
 *   USB CDC On Boot: Enabled
 *   Flash: 16MB · PSRAM: OPI PSRAM
 * Herkunft und Namensnennung: CREDITS.txt
 *   Waveshare PhotoPainter / Spectra-6, convert.exe als Ausgang.
 *   Lab-Linie (gemessene Tinten, CDR): Wei-Ning Huang (AZ), MIT —
 *   github.com/aitjcize/esp32-photoframe und epaper-image-convert.
 *   Sierra: eigenes Python-Rezept. Schrift: Dancing Script (OFL).
 * Library Manager: XPowersLib (Lewis He)
 *
 * Ohne WLAN → AP "Tintenklecks" / Passwort aus WLAN-Setup (Standard: tintenklecks)
 *   → http://192.168.4.1  (Captive Portal)
 * Mit WLAN → http://192.168.x.x  ·  http://tintenklecks.local
 *
 * Web: Menü, Studio, Galerie, Live, System, Rahmen
 * Erinnerung: Geburt/Tod/Besonderes im Studio (TT.MM.JJJJ) → Wechsel am Tag, morgen, übermorgen;
 *        ein Bild voll, mehrere nacheinander (KEY, alle 3 h). Uhr und Rahmen-Modus müssen stimmen.
 * Audio: /sound/*.wav — willkommen (jeder Reset/Reboot, nicht Sleep-Wake),
 *        dann wlan (STA ok) oder ap (kein WLAN / Connect fehlgeschlagen).
 *        neustart kurz vor jedem Software-Neustart.
 * OTA: Arduino IDE Netzwerk-Port „tintenklecks“ (nach erstem USB-Flash). Partition 16M / 3MB APP.
 * Bilder: /pic/ BMPs + JSON + Thumb
 * Power: USB = kein Deep Sleep. Akku: nur bei Wechsel ≥ 10 min oder täglich,
 *        ohne Client direkt nach Bildwechsel, mit Client 60 s Inaktivität.
 *        Wecken: Timer (Bildwechsel), KEY (Bildwechsel, dann wieder schlafen)
 *        oder BOOT-Taste (Web an, kein Bildwechsel). ntfy: Akkustand bei jedem
 *        Aufwachen und Warnung Akku < 10 % einmal am Tag, Priorität wie eingestellt.
 */

#include "config.h"
#include "board.h"

#include <WiFi.h>  // localIP in setup log
#include <esp_sleep.h>

static bool g_epdOk = false;

static void bootBlink(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(PIN_LED_GREEN, LOW);
    delay(80);
    digitalWrite(PIN_LED_GREEN, HIGH);
    delay(80);
  }
}

void setup() {
  ledsAfterWake();
  ledsOff();
  bootBlink(3);

  Serial.setRxBufferSize(4096);
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println(F("=== Tintenklecks boot ==="));
  Serial.flush();

  Serial.println(F("boot: pmu"));
  Serial.flush();
  pmuInit();

  Serial.println(F("boot: rtc"));
  Serial.flush();
  rtcInit();
  tzBegin();
  rtcApplyToSystem();

  Serial.println(F("boot: wifi"));
  Serial.flush();
  bool sta = wifiBringUp();

  Serial.println(F("boot: web"));
  Serial.flush();
  webBegin();
  bootBlink(2);

  Serial.println(F("boot: sd"));
  Serial.flush();
  sdInit();
  webAfterSdReady();

  Serial.println(F("boot: epd"));
  Serial.flush();
  g_epdOk = epdInit();

  slideshowBegin();
  powerOnBoot();
  keyBegin();
  ntfyBegin();
  {
    esp_sleep_wakeup_cause_t wake = esp_sleep_get_wakeup_cause();
    if (wake == ESP_SLEEP_WAKEUP_TIMER || wake == ESP_SLEEP_WAKEUP_EXT0 ||
        wake == ESP_SLEEP_WAKEUP_EXT1) {
      ntfySendWake();
    }
  }
  ntfyBatteryWatch();
  serialProtocolBegin();

  Serial.println(F("boot: ready"));
  Serial.flush();

  if (sta) {
    Serial.print(F("Mode: STA IP "));
    Serial.println(WiFi.localIP());
    Serial.println(F("mDNS in ~8s: http://tintenklecks.local"));
  } else {
    Serial.println(F("Mode: AP — http://192.168.4.1"));
  }

  ledsOff();

  // Reset / ESP.restart only — not timer, KEY, or BOOT wake.
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_UNDEFINED) {
    audioPlayClip("willkommen");
    audioWaitIdle(12000);
    if (sta) {
      audioPlayClip("wlan");
    } else {
      audioPlayClip("ap");
    }
  }
}

void loop() {
  webLoop();
  wifiLoop();
  serialProtocolLoop();
  slideshowLoop();
  keyLoop();
  ntfyBatteryWatch();
  powerLoop();
  delay(1);
}
