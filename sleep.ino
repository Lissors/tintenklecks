// Idle → deep sleep; wake on slideshow timer, KEY (GPIO4) or BOOT (GPIO0)

#include "config.h"
#include "board.h"

#include <esp_sleep.h>
#include <WiFi.h>

static const uint32_t POWER_CLIENT_IDLE_MS = 60UL * 1000UL;
static const int64_t POWER_MIN_SLEEP_SEC = 20;
static const uint32_t POWER_RADIO_IDLE_MS = 10UL * 60UL * 1000UL;

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

static void powerEnterDeepSleep(int64_t secs) {
  epdPanelSleep();
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  delay(50);

  if (secs >= 0) {
    uint64_t sleepUs = (uint64_t)secs * 1000000ULL;
    if (sleepUs < (uint64_t)POWER_MIN_SLEEP_SEC * 1000000ULL) {
      sleepUs = (uint64_t)POWER_MIN_SLEEP_SEC * 1000000ULL;
    }
    Serial.printf("Deep sleep %llu s (timer + KEY + BOOT)\n",
                  (unsigned long long)(sleepUs / 1000000ULL));
    Serial.flush();
    esp_sleep_enable_timer_wakeup(sleepUs);
  } else {
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
  if (pmuUsbPowered() || !slideshowSleepAllowed() || g_busyDepth > 0 || clientHere) {
    return;
  }
  int64_t secs = slideshowSecondsUntilNext();
  if (secs < 0) {
    return;  // no clock yet
  }
  powerEnterDeepSleep(secs);
}
