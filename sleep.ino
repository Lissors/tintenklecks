// Idle → deep sleep; wake on slideshow timer, KEY (GPIO4) or BOOT (GPIO0)

#include "config.h"
#include "board.h"

#include <esp_sleep.h>
#include <esp_system.h>
#include <WiFi.h>
#include <driver/gpio.h>
#include <Preferences.h>
#include <math.h>
#include <time.h>

static const uint32_t POWER_CLIENT_IDLE_MS = 60UL * 1000UL;
static const int64_t POWER_MIN_SLEEP_SEC = 20;
static const uint32_t POWER_RADIO_IDLE_MS = 10UL * 60UL * 1000UL;

static const uint32_t SLP_MAGIC = 0x51EE0001;
static RTC_DATA_ATTR uint32_t g_slpMagic = 0;
static RTC_DATA_ATTR int64_t g_slpWant = 0;
static RTC_DATA_ATTR int64_t g_slpStart = 0;

static float g_cal = 1.0f;
static bool g_calLoaded = false;

static uint32_t g_lastClientMs = 0;
static bool g_clientSeen = false;
static int g_busyDepth = 0;

void powerNoteActivity() {
  g_lastClientMs = millis();
  g_clientSeen = true;
}

bool powerBusy() { return g_busyDepth > 0; }

bool powerClientHere() {
  return g_clientSeen && (millis() - g_lastClientMs) < POWER_CLIENT_IDLE_MS;
}

void powerNoteBusy(bool on) {
  if (on) {
    g_busyDepth++;
  } else if (g_busyDepth > 0) {
    g_busyDepth--;
  }
}

void ledsAfterWake() {
  gpio_deep_sleep_hold_dis();
  gpio_hold_dis((gpio_num_t)PIN_LED_GREEN);
  gpio_hold_dis((gpio_num_t)PIN_LED_RED);
  gpio_hold_dis((gpio_num_t)PIN_AUDIO_PA);
  gpio_reset_pin((gpio_num_t)PIN_AUDIO_PA);
  pinMode(PIN_AUDIO_PA, OUTPUT);
  digitalWrite(PIN_AUDIO_PA, LOW);
}

void ledsOff() {
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, HIGH);
}

static void ledsHoldOff() {
  ledsOff();
  gpio_hold_en((gpio_num_t)PIN_LED_GREEN);
  gpio_hold_en((gpio_num_t)PIN_LED_RED);
  gpio_deep_sleep_hold_en();
}

static void calLoad() {
  if (g_calLoaded) {
    return;
  }
  Preferences p;
  p.begin("sleep", true);
  uint32_t u = p.getUInt("cal", 1000000);
  p.end();
  g_cal = (float)u / 1000000.0f;
  if (g_cal < 0.97f) {
    g_cal = 0.97f;
  }
  if (g_cal > 1.03f) {
    g_cal = 1.03f;
  }
  g_calLoaded = true;
}

static void calSave() {
  Preferences p;
  p.begin("sleep", false);
  p.putUInt("cal", (uint32_t)(g_cal * 1000000.0f + 0.5f));
  p.end();
}

void powerSleepCalOnWake() {
  calLoad();
  if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_TIMER || g_slpMagic != SLP_MAGIC ||
      g_slpWant < 60) {
    g_slpMagic = 0;
    return;
  }
  time_t now = 0;
  if (!rtcEpoch(&now) || g_slpStart < 1700000000) {
    g_slpMagic = 0;
    return;
  }
  int64_t actual = (int64_t)now - g_slpStart;
  g_slpMagic = 0;
  if (actual < 30) {
    return;
  }
  float ratio = (float)g_slpWant / (float)actual;
  if (ratio < 0.90f || ratio > 1.10f) {
    Serial.printf("sleep cal skip want=%lld actual=%lld\n", (long long)g_slpWant,
                  (long long)actual);
    return;
  }
  float prev = g_cal;
  g_cal = g_cal * (0.6f + 0.4f * ratio);
  if (g_cal < 0.97f) {
    g_cal = 0.97f;
  }
  if (g_cal > 1.03f) {
    g_cal = 1.03f;
  }
  calSave();
  float tC = pmuTemperature();
  Serial.printf("sleep cal want=%lld actual=%lld  %.5f → %.5f", (long long)g_slpWant,
                (long long)actual, prev, g_cal);
  if (!isnan(tC)) {
    Serial.printf("  T=%.1fC", tC);
  }
  Serial.println();
}

static void powerEnterDeepSleep(int64_t secs) {
  audioPowerDown();
  pinMode(PIN_AUDIO_PA, OUTPUT);
  digitalWrite(PIN_AUDIO_PA, LOW);
  gpio_hold_en((gpio_num_t)PIN_AUDIO_PA);

  ledsHoldOff();
  epdPanelSleep();
  delay(100);
  pmuSleepRails();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);

  if (secs >= 0) {
    uint64_t sleepSec = (uint64_t)secs;
    if (sleepSec < (uint64_t)POWER_MIN_SLEEP_SEC) {
      sleepSec = (uint64_t)POWER_MIN_SLEEP_SEC;
    }
    calLoad();
    time_t start = 0;
    if (rtcEpoch(&start)) {
      g_slpStart = (int64_t)start;
      g_slpWant = (int64_t)sleepSec;
      g_slpMagic = SLP_MAGIC;
    } else {
      g_slpMagic = 0;
    }
    uint64_t sleepUs = (uint64_t)((double)sleepSec * (double)g_cal * 1000000.0);
    Serial.printf("Deep sleep %llu s cal=%.5f (timer + KEY + BOOT)\n",
                  (unsigned long long)sleepSec, g_cal);
    Serial.flush();
    esp_sleep_enable_timer_wakeup(sleepUs);
  } else {
    g_slpMagic = 0;
    Serial.println(F("Deep sleep (KEY + BOOT)"));
    Serial.flush();
  }
  // BOOT = GPIO0, active low
  esp_sleep_enable_ext0_wakeup((gpio_num_t)PIN_BOOT, 0);
  keyPrepareSleepWake();

  esp_deep_sleep_start();
}

void powerSleepNow() {
  if (pmuUsbPowered()) {
    return;
  }
  powerEnterDeepSleep(slideshowSecondsUntilNext());
}

void powerOnBoot() {
  if (esp_reset_reason() != ESP_RST_DEEPSLEEP) {
    Serial.println(F("Wake: power-on / reset"));
    powerNoteActivity();
    return;
  }
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
  switch (cause) {
    case ESP_SLEEP_WAKEUP_TIMER:
      Serial.println(F("Wake: timer → Bildwechsel"));
      if (sdOk()) {
        powerNoteBusy(true);
        slideshowOnTimer();
        powerNoteBusy(false);
      }
      break;
    case ESP_SLEEP_WAKEUP_EXT1:
      Serial.println(F("Wake: KEY → Bildwechsel"));
      if (sdOk()) {
        powerNoteBusy(true);
        slideshowForceNow();
        powerNoteBusy(false);
      }
      break;
    case ESP_SLEEP_WAKEUP_EXT0:
      Serial.println(F("Wake: BOOT — Web aktiv, kein Bildwechsel"));
      powerNoteActivity();
      break;
    default:
      Serial.println(F("Wake: power-on / reset"));
      powerNoteActivity();
      break;
  }
}

void powerLoop() {
  bool clientHere = g_clientSeen && (millis() - g_lastClientMs) < POWER_CLIENT_IDLE_MS;

  static bool radioAwake = false;
  static bool radioKnown = false;
  bool want = g_busyDepth > 0 || (g_clientSeen && (millis() - g_lastClientMs) < POWER_RADIO_IDLE_MS);
  if (!radioKnown || want != radioAwake) {
    radioKnown = true;
    radioAwake = want;
    WiFi.setSleep(!want);
  }

  // Deep sleep: USB stays up. Battery: after 60 s without a client, and only when
  // pictures change rarely (daily or interval ≥ 10 min). 5 min / off: stays up.
  bool usb = pmuUsbPowered();
  bool allow = slideshowSleepAllowed();
  if (usb || !allow || g_busyDepth > 0 || clientHere) {
    static uint32_t lastWhy = 0;
    if ((uint32_t)(millis() - lastWhy) >= 10000UL) {
      lastWhy = millis();
      Serial.printf("no sleep: usb=%d allow=%d busy=%d client=%d\n", usb ? 1 : 0, allow ? 1 : 0,
                    g_busyDepth, clientHere ? 1 : 0);
    }
    return;
  }
  int64_t secs = slideshowSecondsUntilNext();
  if (secs < 0) {
    static uint32_t lastWhy2 = 0;
    if ((uint32_t)(millis() - lastWhy2) >= 10000UL) {
      lastWhy2 = millis();
      Serial.println(F("no sleep: no clock / no next"));
    }
    return;  // no clock yet
  }
  powerEnterDeepSleep(secs);
}
