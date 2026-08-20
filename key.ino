// KEY (GPIO4): next picture while awake; from deep sleep: wake, switch, sleep
// Zufall, or next due memory when several share the window

#include "config.h"
#include "board.h"

#include <esp_sleep.h>
#include "driver/rtc_io.h"

static bool g_armed = false;
static uint32_t g_downMs = 0;

void keyBegin() {
  rtc_gpio_deinit((gpio_num_t)PIN_KEY);
  pinMode(PIN_KEY, INPUT_PULLUP);
  g_armed = digitalRead(PIN_KEY) == HIGH;
  g_downMs = 0;
}

void keyPrepareSleepWake() {
  pinMode(PIN_KEY, INPUT_PULLUP);
  uint32_t t0 = millis();
  while (digitalRead(PIN_KEY) == LOW && (millis() - t0) < 2000) {
    delay(10);
  }
  rtc_gpio_init((gpio_num_t)PIN_KEY);
  rtc_gpio_set_direction((gpio_num_t)PIN_KEY, RTC_GPIO_MODE_INPUT_ONLY);
  rtc_gpio_pullup_en((gpio_num_t)PIN_KEY);
  rtc_gpio_pulldown_dis((gpio_num_t)PIN_KEY);
  esp_sleep_enable_ext1_wakeup(1ULL << PIN_KEY, ESP_EXT1_WAKEUP_ALL_LOW);
}

void keyLoop() {
  if (powerBusy()) {
    return;
  }
  const bool down = digitalRead(PIN_KEY) == LOW;
  if (!down) {
    g_armed = true;
    g_downMs = 0;
    return;
  }
  if (!g_armed) {
    return;
  }
  if (g_downMs == 0) {
    g_downMs = millis();
    return;
  }
  if (millis() - g_downMs < 40) {
    return;
  }
  g_armed = false;
  g_downMs = 0;
  Serial.println(F("KEY → Bildwechsel"));
  powerNoteBusy(true);
  slideshowForceNow();
  powerNoteBusy(false);
}
