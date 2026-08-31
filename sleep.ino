// Idle → deep sleep; wake on slideshow timer, KEY (GPIO4) or BOOT (GPIO0)

#include "config.h"
#include "board.h"

#include <esp_sleep.h>
#include <esp_system.h>
#include <WiFi.h>
#include <driver/gpio.h>
#include "driver/rtc_io.h"
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

static esp_sleep_wakeup_cause_t g_wake = ESP_SLEEP_WAKEUP_UNDEFINED;
static bool g_wakeBootBtn = false;
static bool g_wakeKeyBtn = false;

void powerCaptureWake() {
  g_wake = esp_sleep_get_wakeup_cause();
  pinMode(PIN_BOOT, INPUT_PULLUP);
  pinMode(PIN_KEY, INPUT_PULLUP);
  delay(2);
  g_wakeBootBtn = digitalRead(PIN_BOOT) == LOW;
  g_wakeKeyBtn = digitalRead(PIN_KEY) == LOW;
}

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
  for (int i = 0; i <= 21; i++) {
    gpio_num_t pin = (gpio_num_t)i;
    if (!rtc_gpio_is_valid_gpio(pin)) {
      continue;
    }
    rtc_gpio_hold_dis(pin);
    rtc_gpio_deinit(pin);
  }
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

static void gpioFloatUnusedRtc() {
  for (int i = 0; i <= 21; i++) {
    gpio_num_t pin = (gpio_num_t)i;
    if (!rtc_gpio_is_valid_gpio(pin)) {
      continue;
    }
    if (pin == (gpio_num_t)PIN_BOOT || pin == (gpio_num_t)PIN_KEY ||
        pin == (gpio_num_t)PIN_AUDIO_PA || pin == (gpio_num_t)PIN_AXP_IRQ) {
      continue;
    }
    rtc_gpio_init(pin);
    rtc_gpio_pullup_dis(pin);
    rtc_gpio_pulldown_dis(pin);
    rtc_gpio_isolate(pin);
  }
}

static void powerEnterDeepSleep(int64_t secs) {
  audioPowerDown();
  shtc3Sleep();
  sdDeinit();
  pinMode(PIN_AUDIO_PA, OUTPUT);
  digitalWrite(PIN_AUDIO_PA, LOW);
  gpio_hold_en((gpio_num_t)PIN_AUDIO_PA);

  ledsHoldOff();
  epdPanelSleep();
  delay(100);
  gpioFloatUnusedRtc();
  keyPrepareSleepWake();
  gpio_hold_dis((gpio_num_t)PIN_BOOT);
  rtc_gpio_hold_dis((gpio_num_t)PIN_BOOT);
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
  gpio_hold_dis((gpio_num_t)PIN_BOOT);
  rtc_gpio_hold_dis((gpio_num_t)PIN_BOOT);

  esp_deep_sleep_start();
}

void powerSleepNow() {
  if (pmuUsbPowered() || powerStayAwake()) {
    return;
  }
  powerEnterDeepSleep(slideshowSecondsUntilNext());
}

void powerSleepForced() {
  if (pmuUsbPowered()) {
    return;
  }
  powerEnterDeepSleep(slideshowSecondsUntilNext());
}

static bool g_stay = false;
static bool g_stayKnown = false;

bool powerStayAwake() {
  if (!g_stayKnown) {
    Preferences p;
    p.begin("power", true);
    g_stay = p.getBool("stay", false);
    p.end();
    g_stayKnown = true;
  }
  return g_stay;
}

void powerSetStayAwake(bool on) {
  g_stay = on;
  g_stayKnown = true;
  Preferences p;
  p.begin("power", false);
  p.putBool("stay", on);
  p.end();
  if (on) {
    powerNoteActivity();
  }
}

void powerOnBoot() {
  uint64_t ext1 = 0;
  if (g_wake == ESP_SLEEP_WAKEUP_EXT1) {
    ext1 = esp_sleep_get_ext1_wakeup_status();
  }
  const bool bootBit = (ext1 & (1ULL << PIN_BOOT)) != 0;
  const bool keyBit = (ext1 & (1ULL << PIN_KEY)) != 0;
  Serial.printf("reset=%d wake=%d boot=%d key=%d ext1=0x%llx\n", (int)esp_reset_reason(),
                (int)g_wake, (g_wakeBootBtn || bootBit) ? 1 : 0,
                (g_wakeKeyBtn || keyBit) ? 1 : 0, (unsigned long long)ext1);

  const bool fromBoot = (g_wake == ESP_SLEEP_WAKEUP_EXT0) || g_wakeBootBtn || bootBit;
  const bool fromTimer = (g_wake == ESP_SLEEP_WAKEUP_TIMER) && !fromBoot;
  const bool fromKey = (g_wakeKeyBtn || keyBit) && !fromBoot;

  if (fromBoot) {
    Serial.println(F("Wake: BOOT — Web aktiv, kein Bildwechsel"));
    slideshowHoldDue();
    powerNoteActivity();
    return;
  }

  if (fromTimer || fromKey) {
    if (!sdOk()) {
      sdInit();
    }
    if (!sdOk()) {
      Serial.println(F("Wake: KEY/timer — SD missing, no picture"));
      return;
    }
    powerNoteBusy(true);
    if (fromTimer) {
      Serial.println(F("Wake: timer → Bildwechsel"));
      slideshowOnTimer();
    } else {
      Serial.println(F("Wake: KEY → Bildwechsel"));
      slideshowForceNow();
    }
    powerNoteBusy(false);
    ntfyBegin();
    ntfySendWake();
    ntfyBatteryWatch();
    powerSleepNow();
    return;
  }
  Serial.println(F("Wake: power-on / reset"));
  powerNoteActivity();
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
  bool stay = powerStayAwake();
  bool allow = slideshowSleepAllowed();
  if (usb || stay || !allow || g_busyDepth > 0 || clientHere) {
    static uint32_t lastWhy = 0;
    if ((uint32_t)(millis() - lastWhy) >= 10000UL) {
      lastWhy = millis();
      Serial.printf("no sleep: usb=%d stay=%d allow=%d busy=%d client=%d\n", usb ? 1 : 0,
                    stay ? 1 : 0, allow ? 1 : 0, g_busyDepth, clientHere ? 1 : 0);
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
