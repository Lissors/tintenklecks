// AXP2101 — ALDO4 3.3V for EPD + battery status
// Library Manager: "XPowersLib" by Lewis He

#include "config.h"
#include "board.h"
#include <Wire.h>
#include <XPowersLib.h>
#include <Preferences.h>
#include <math.h>

static XPowersAXP2101 pmu;
static bool pmuOk = false;

static uint8_t pmuMaToOpt(int ma);

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
  pmu.enableVbusVoltageMeasure();
  pmu.enableTemperatureMeasure();
  pmu.setALDO4Voltage(3300);
  pmu.enableALDO4();
  pmuOk = true;
  {
    Preferences p;
    p.begin("pmu", true);
    uint16_t ma = p.getUShort("ichg", 0);
    p.end();
    if (ma >= 100) {
      uint8_t opt = pmuMaToOpt((int)ma);
      if (opt != 255) {
        pmu.setChargerConstantCurr(opt);
      }
    }
  }
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

static int pmuChgCurrMa(uint8_t opt) {
  static const int kMa[] = {0, -1, -1, -1, 100, 125, 150, 175, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
  if (opt > 16) {
    return -1;
  }
  return kMa[opt];
}

static uint8_t pmuMaToOpt(int ma) {
  switch (ma) {
    case 100:
      return XPOWERS_AXP2101_CHG_CUR_100MA;
    case 125:
      return XPOWERS_AXP2101_CHG_CUR_125MA;
    case 150:
      return XPOWERS_AXP2101_CHG_CUR_150MA;
    case 175:
      return XPOWERS_AXP2101_CHG_CUR_175MA;
    case 200:
      return XPOWERS_AXP2101_CHG_CUR_200MA;
    case 300:
      return XPOWERS_AXP2101_CHG_CUR_300MA;
    case 400:
      return XPOWERS_AXP2101_CHG_CUR_400MA;
    case 500:
      return XPOWERS_AXP2101_CHG_CUR_500MA;
    case 600:
      return XPOWERS_AXP2101_CHG_CUR_600MA;
    case 700:
      return XPOWERS_AXP2101_CHG_CUR_700MA;
    case 800:
      return XPOWERS_AXP2101_CHG_CUR_800MA;
    case 900:
      return XPOWERS_AXP2101_CHG_CUR_900MA;
    case 1000:
      return XPOWERS_AXP2101_CHG_CUR_1000MA;
    default:
      return 255;
  }
}

int pmuChargeMa() {
  if (!pmuOk) {
    return -1;
  }
  return pmuChgCurrMa(pmu.getChargerConstantCurr());
}

bool pmuSetChargeMa(int ma) {
  uint8_t opt = pmuMaToOpt(ma);
  if (opt == 255) {
    return false;
  }
  Preferences p;
  p.begin("pmu", false);
  p.putUShort("ichg", (uint16_t)ma);
  p.end();
  if (pmuOk) {
    pmu.setChargerConstantCurr(opt);
  }
  return true;
}

static const char *pmuChgStage(uint8_t st) {
  switch (st) {
    case XPOWERS_AXP2101_CHG_TRI_STATE:
      return "tri";
    case XPOWERS_AXP2101_CHG_PRE_STATE:
      return "Vorladen";
    case XPOWERS_AXP2101_CHG_CC_STATE:
      return "Konstantstrom";
    case XPOWERS_AXP2101_CHG_CV_STATE:
      return "Konstantspannung";
    case XPOWERS_AXP2101_CHG_DONE_STATE:
      return "fertig";
    case XPOWERS_AXP2101_CHG_STOP_STATE:
      return "aus";
    default:
      return "—";
  }
}

static const char *pmuPath() {
  if (pmu.isCharging()) {
    return "laden";
  }
  if (pmu.isDischarge()) {
    return "entladen";
  }
  if (pmu.isStandby()) {
    return "standby";
  }
  return "—";
}

static int pmuTargetMv(uint8_t opt) {
  switch (opt) {
    case XPOWERS_AXP2101_CHG_VOL_4V:
      return 4000;
    case XPOWERS_AXP2101_CHG_VOL_4V1:
      return 4100;
    case XPOWERS_AXP2101_CHG_VOL_4V2:
      return 4200;
    case XPOWERS_AXP2101_CHG_VOL_4V35:
      return 4350;
    case XPOWERS_AXP2101_CHG_VOL_4V4:
      return 4400;
    default:
      return -1;
  }
}

static int pmuVbusLimMa(uint8_t opt) {
  switch (opt) {
    case XPOWERS_AXP2101_VBUS_CUR_LIM_100MA:
      return 100;
    case XPOWERS_AXP2101_VBUS_CUR_LIM_500MA:
      return 500;
    case XPOWERS_AXP2101_VBUS_CUR_LIM_900MA:
      return 900;
    case XPOWERS_AXP2101_VBUS_CUR_LIM_1000MA:
      return 1000;
    case XPOWERS_AXP2101_VBUS_CUR_LIM_1500MA:
      return 1500;
    case XPOWERS_AXP2101_VBUS_CUR_LIM_2000MA:
      return 2000;
    default:
      return -1;
  }
}

void pmuAppendJson(String &j) {
  j += ",\"batt\":{";
  if (!pmuOk) {
    j += "\"ok\":false}";
    return;
  }
  const bool cell = pmu.isBatteryConnect();
  const int pct = cell ? (int)pmu.getBatteryPercent() : -1;
  const uint16_t mv = cell ? pmu.getBattVoltage() : 0;
  const bool usb = pmu.isVbusIn();
  const uint16_t vbus = usb ? pmu.getVbusVoltage() : 0;
  const uint16_t vsys = pmu.getSystemVoltage();
  const int ichg = pmuChgCurrMa(pmu.getChargerConstantCurr());
  const int vt = pmuTargetMv(pmu.getChargeTargetVoltage());
  const int iterm = (int)pmu.getChargerTerminationCurr() * 25;
  const int ilim = pmuVbusLimMa(pmu.getVbusCurrentLimit());
  const int warn = (int)pmu.getLowBatWarnThreshold() + 5;
  const int off = (int)pmu.getLowBatShutdownThreshold();
  const float temp = pmu.getTemperature();
  j += "\"ok\":true,\"cell\":";
  j += cell ? "true" : "false";
  j += ",\"pct\":";
  j += String(pct);
  j += ",\"v\":";
  j += cell ? String(mv / 1000.0f, 2) : "0";
  j += ",\"usb\":";
  j += usb ? "true" : "false";
  j += ",\"vbus\":";
  j += usb ? String(vbus / 1000.0f, 2) : "0";
  j += ",\"vsys\":";
  j += String(vsys / 1000.0f, 2);
  j += ",\"path\":\"";
  j += pmuPath();
  j += "\",\"chg\":\"";
  j += pmuChgStage((uint8_t)pmu.getChargerStatus());
  j += "\",\"ichg\":";
  j += String(ichg);
  j += ",\"vtarget\":";
  j += String(vt);
  j += ",\"iterm\":";
  j += String(iterm);
  j += ",\"ilim\":";
  j += String(ilim);
  j += ",\"temp\":";
  if (isnan(temp)) {
    j += "null";
  } else {
    j += String(temp, 1);
  }
  j += ",\"thermal\":";
  j += pmu.getThermalRegulationStatus() ? "true" : "false";
  j += ",\"limHit\":";
  j += pmu.getCurrentLimitStatus() ? "true" : "false";
  j += ",\"warnPct\":";
  j += String(warn);
  j += ",\"offPct\":";
  j += String(off);
  j += "}";
}
