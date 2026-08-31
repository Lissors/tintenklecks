// KEY (GPIO4): next picture while awake; from deep sleep: wake, switch, sleep
// Zufall, or next due memory when several share the window

#include "config.h"
#include "board.h"

#include <esp_sleep.h>
#include "driver/rtc_io.h"
#include <driver/gpio.h>

static bool g_armed = false;
static uint32_t g_downMs = 0;
static uint32_t g_ignoreUntil = 0;

void keyBegin() {
  rtc_gpio_deinit((gpio_num_t)PIN_KEY);
  pinMode(PIN_KEY, INPUT_PULLUP);
  uint32_t t0 = millis();
  while (digitalRead(PIN_KEY) == LOW && (millis() - t0) < 2000) {
    delay(10);
  }
  delay(30);
  g_armed = digitalRead(PIN_KEY) == HIGH;
  g_downMs = 0;
  g_ignoreUntil = millis() + 400;
}

void keyPrepareSleepWake() {
  // GPIO0 is a strapping pin: global deep-sleep hold latches it high, BOOT
  // then cannot pull it down. Keep RTC IO domain on so EXT1 sees the pad.
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

  pinMode(PIN_KEY, INPUT_PULLUP);
  gpio_reset_pin((gpio_num_t)PIN_BOOT);
  pinMode(PIN_BOOT, INPUT_PULLUP);
  uint32_t t0 = millis();
  while ((digitalRead(PIN_KEY) == LOW || digitalRead(PIN_BOOT) == LOW) &&
         (millis() - t0) < 2000) {
    delay(10);
  }
  const gpio_num_t pins[] = {(gpio_num_t)PIN_BOOT, (gpio_num_t)PIN_KEY};
  for (int i = 0; i < 2; i++) {
    gpio_hold_dis(pins[i]);
    rtc_gpio_init(pins[i]);
    rtc_gpio_set_direction(pins[i], RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pullup_en(pins[i]);
    rtc_gpio_pulldown_dis(pins[i]);
    rtc_gpio_hold_dis(pins[i]);
  }
  esp_sleep_enable_ext1_wakeup((1ULL << PIN_BOOT) | (1ULL << PIN_KEY),
                               ESP_EXT1_WAKEUP_ANY_LOW);
}

void keyLoop() {
  if (powerBusy() || (int32_t)(millis() - g_ignoreUntil) < 0) {
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
