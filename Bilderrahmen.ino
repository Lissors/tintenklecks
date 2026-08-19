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
 * Erinnerung: Geburt/Tod/Besonderes im Studio (TT.MM.JJJJ) → Wechsel morgen/übermorgen,
 *        bis zu 3 Bilder übereinander. Uhr und Rahmen-Modus müssen stimmen.
 * Audio: /sound/*.wav — willkommen (jeder Reset/Reboot, nicht Sleep-Wake),
 *        dann wlan (STA ok) oder ap (kein WLAN / Connect fehlgeschlagen).
 *        neustart kurz vor jedem Software-Neustart.
 * Bilder: /pic/ BMPs + JSON + Thumb
 * Power: USB = kein Deep Sleep. Akku: nur bei Wechsel ≥ 10 min oder täglich,
 *        ohne Client direkt nach Bildwechsel, mit Client 60 s Inaktivität.
 *        Wecken: Timer (Bildwechsel), KEY (Bildwechsel, dann wieder schlafen)
 *        oder BOOT-Taste (Web an, kein Bildwechsel). ntfy nur Akku < 10 %, keine Wachmeldung.
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
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, HIGH);
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

  digitalWrite(PIN_LED_GREEN, (sdOk() && g_epdOk) ? LOW : HIGH);
  digitalWrite(PIN_LED_RED, HIGH);

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
