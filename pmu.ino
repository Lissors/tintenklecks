// AXP2101 — ALDO4 3.3V for EPD + battery status
// Library Manager: "XPowersLib" by Lewis He

#include "config.h"
#include "board.h"
#include <Wire.h>
#include <XPowersLib.h>

static XPowersAXP2101 pmu;
static bool pmuOk = false;

bool pmuInit() {
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(100);
  if (!pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL)) {
    Serial.println(F("WARN AXP2101 not found — EPD may stay blank"));
    pmuOk = false;
    return false;
  }
  pmu.enableSystemVoltageMeasure();
  pmu.enableBattDetection();
  pmu.enableBattVoltageMeasure();
  pmu.setALDO4Voltage(3300);
  pmu.enableALDO4();
  pmuOk = true;
  Serial.println(F("AXP2101 OK — ALDO4 EPD (Audio ALDO3 deferred)"));
  return true;
}

bool pmuEnableAudioRail() {
  if (!pmuOk) {
    return false;
  }
  pmu.setALDO3Voltage(3300);
  pmu.enableALDO3();
  delay(100);
  return true;
}

bool pmuReady() {
  return pmuOk;
}

int pmuBatteryPercent() {
  if (!pmuOk || !pmu.isBatteryConnect()) {
    return -1;
  }
  return (int)pmu.getBatteryPercent();
}

bool pmuCharging() {
  if (!pmuOk) {
    return false;
  }
  return pmu.isCharging();
}

bool pmuUsbPowered() {
  if (!pmuOk) {
    return true;
  }
  return pmu.isVbusIn();
}

float pmuBattVoltage() {
  if (!pmuOk || !pmu.isBatteryConnect()) {
    return 0.0f;
  }
  return pmu.getBattVoltage() / 1000.0f;
}
