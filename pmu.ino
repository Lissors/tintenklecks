// AXP2101 — ALDO4 3.3V for EPD + battery status
// Library Manager: "XPowersLib" by Lewis He

#include "config.h"
#include "board.h"
#include <Wire.h>
#include <XPowersLib.h>
#include <Preferences.h>
#include <math.h>
#include <string.h>
#include <driver/gpio.h>

static XPowersAXP2101 pmu;
static bool pmuOk = false;

static uint8_t pmuMaToOpt(int ma);

struct PmuSnap {
  bool ready;
  bool cell;
  int pct;
  uint16_t mv;
  bool usb;
  bool charging;
  bool discharge;
  bool standby;
  uint16_t vbus;
  uint16_t vsys;
  int ichg;
  int vt;
  int iterm;
  int ilim;
  int warn;
  int off;
  float temp;
  bool thermal;
  bool limHit;
  uint8_t chgSt;
};

static PmuSnap g_pmuSnap;
static uint32_t g_pmuSnapMs = 0;
static bool g_pmuSnapOk = false;

static void pmuFillSnap();
static void pmuEnsureSnap();
static int pmuTargetMv(uint8_t opt);
static int pmuVbusLimMa(uint8_t opt);

static void i2cBusRecover() {
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, OUTPUT);
  for (int i = 0; i < 16; i++) {
    digitalWrite(I2C_SCL, HIGH);
    delayMicroseconds(5);
    digitalWrite(I2C_SCL, LOW);
    delayMicroseconds(5);
  }
  digitalWrite(I2C_SCL, HIGH);
  delayMicroseconds(5);
  pinMode(I2C_SDA, OUTPUT);
  digitalWrite(I2C_SDA, HIGH);
  delayMicroseconds(5);
  pinMode(I2C_SDA, INPUT_PULLUP);
  pinMode(I2C_SCL, INPUT_PULLUP);
}

static void pmuWakeIrqPulse() {
  gpio_reset_pin((gpio_num_t)PIN_AXP_IRQ);
  pinMode(PIN_AXP_IRQ, OUTPUT);
  digitalWrite(PIN_AXP_IRQ, LOW);
  delay(100);
  digitalWrite(PIN_AXP_IRQ, HIGH);
  delay(200);
  pinMode(PIN_AXP_IRQ, INPUT_PULLUP);
}

bool pmuInit() {
  pmuWakeIrqPulse();
  i2cBusRecover();
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setTimeOut(250);
  bool axp = pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  if (!axp) {
    i2cBusRecover();
    Wire.begin(I2C_SDA, I2C_SCL);
    Wire.setTimeOut(250);
    axp = pmu.begin(Wire, AXP2101_SLAVE_ADDRESS, I2C_SDA, I2C_SCL);
  }
  if (!axp) {
    Serial.println(F("WARN AXP2101 not found — EPD may stay blank"));
    pmuOk = false;
    return false;
  }
  pmu.enableSystemVoltageMeasure();
  pmu.enableBattDetection();
  pmu.enableBattVoltageMeasure();
  pmu.enableVbusVoltageMeasure();
  pmu.enableTemperatureMeasure();
  pmu.setDC1Voltage(3300);
  pmu.enableDC1();
  pmu.setALDO3Voltage(3300);
  pmu.enableALDO3();
  pmu.setALDO4Voltage(3300);
  pmu.enableALDO4();
  delay(200);
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
  Serial.println(F("AXP2101 OK — ALDO3 audio + ALDO4 EPD"));
  return true;
}

bool pmuEnableAudioRail() {
  if (!pmuOk) {
    return false;
  }
  pmu.setALDO3Voltage(3300);
  pmu.enableALDO3();
  delay(200);
  return true;
}

float pmuTemperature() {
  if (!pmuOk) {
    return NAN;
  }
  return pmu.getTemperature();
}

void shtc3Sleep() {
  Wire.beginTransmission(SHTC3_ADDR);
  Wire.write(0xB0);
  Wire.write(0x98);
  if (Wire.endTransmission() == 0) {
    Serial.println(F("SHTC3 sleep"));
  }
}

void pmuSleepRails() {
  if (!pmuOk) {
    return;
  }
  pmu.disableIRQ(XPOWERS_AXP2101_ALL_IRQ);
  pmu.clearIrqStatus();
  int power_value = pmu.readRegister(0x26);
  if (!(power_value & 0x04)) {
    pmu.wakeupControl(XPOWERS_AXP2101_WAKEUP_DC_DLO_SELECT, true);
  }
  if (power_value & 0x08) {
    pmu.wakeupControl(XPOWERS_AXP2101_WAKEUP_PWROK_TO_LOW, false);
  }
  if (!(power_value & 0x10)) {
    pmu.wakeupControl(XPOWERS_AXP2101_WAKEUP_IRQ_PIN_TO_LOW, true);
  }
  pmu.disableBattVoltageMeasure();
  pmu.disableBattDetection();
  pmu.enableSleep();
  pmu.disableDC2();
  pmu.disableDC3();
  pmu.disableDC4();
  pmu.disableDC5();
  pmu.disableALDO1();
  pmu.disableALDO2();
  pmu.disableBLDO1();
  pmu.disableBLDO2();
  pmu.disableCPUSLDO();
  pmu.disableDLDO1();
  pmu.disableDLDO2();
  pmu.disableALDO4();
  pmu.disableALDO3();
  Serial.println(F("PMU sleep: AXP sleep, ALDO3 last"));
}

bool pmuReady() {
  return pmuOk;
}

int pmuBatteryPercent() {
  pmuEnsureSnap();
  if (!g_pmuSnap.ready || !g_pmuSnap.cell) {
    return -1;
  }
  return g_pmuSnap.pct;
}

bool pmuCharging() {
  pmuEnsureSnap();
  if (!g_pmuSnap.ready) {
    return false;
  }
  return g_pmuSnap.charging;
}

bool pmuUsbPowered() {
  pmuEnsureSnap();
  if (!g_pmuSnap.ready) {
    return true;
  }
  return g_pmuSnap.usb;
}

float pmuBattVoltage() {
  pmuEnsureSnap();
  if (!g_pmuSnap.ready || !g_pmuSnap.cell) {
    return 0.0f;
  }
  return g_pmuSnap.mv / 1000.0f;
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
  g_pmuSnapOk = false;
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

static const char *pmuPathFromSnap() {
  if (g_pmuSnap.charging) {
    return "laden";
  }
  if (g_pmuSnap.discharge) {
    return "entladen";
  }
  if (g_pmuSnap.standby) {
    return "standby";
  }
  return "—";
}

static void pmuFillSnap() {
  g_pmuSnapMs = millis();
  g_pmuSnapOk = true;
  memset(&g_pmuSnap, 0, sizeof(g_pmuSnap));
  g_pmuSnap.ready = pmuOk;
  g_pmuSnap.pct = -1;
  g_pmuSnap.temp = NAN;
  if (!pmuOk) {
    return;
  }
  g_pmuSnap.cell = pmu.isBatteryConnect();
  g_pmuSnap.pct = g_pmuSnap.cell ? (int)pmu.getBatteryPercent() : -1;
  g_pmuSnap.mv = g_pmuSnap.cell ? pmu.getBattVoltage() : 0;
  g_pmuSnap.usb = pmu.isVbusIn();
  g_pmuSnap.charging = pmu.isCharging();
  g_pmuSnap.discharge = pmu.isDischarge();
  g_pmuSnap.standby = pmu.isStandby();
  g_pmuSnap.vbus = g_pmuSnap.usb ? pmu.getVbusVoltage() : 0;
  g_pmuSnap.vsys = pmu.getSystemVoltage();
  g_pmuSnap.ichg = pmuChgCurrMa(pmu.getChargerConstantCurr());
  g_pmuSnap.vt = pmuTargetMv(pmu.getChargeTargetVoltage());
  g_pmuSnap.iterm = (int)pmu.getChargerTerminationCurr() * 25;
  g_pmuSnap.ilim = pmuVbusLimMa(pmu.getVbusCurrentLimit());
  g_pmuSnap.warn = (int)pmu.getLowBatWarnThreshold() + 5;
  g_pmuSnap.off = (int)pmu.getLowBatShutdownThreshold();
  g_pmuSnap.temp = pmu.getTemperature();
  g_pmuSnap.thermal = pmu.getThermalRegulationStatus();
  g_pmuSnap.limHit = pmu.getCurrentLimitStatus();
  g_pmuSnap.chgSt = (uint8_t)pmu.getChargerStatus();
}

static void pmuEnsureSnap() {
  if (g_pmuSnapOk && (millis() - g_pmuSnapMs) < 2000) {
    return;
  }
  pmuFillSnap();
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
  pmuEnsureSnap();
  j += ",\"batt\":{";
  if (!g_pmuSnap.ready) {
    j += "\"ok\":false}";
    return;
  }
  j += "\"ok\":true,\"cell\":";
  j += g_pmuSnap.cell ? "true" : "false";
  j += ",\"pct\":";
  j += String(g_pmuSnap.pct);
  j += ",\"v\":";
  j += g_pmuSnap.cell ? String(g_pmuSnap.mv / 1000.0f, 2) : "0";
  j += ",\"usb\":";
  j += g_pmuSnap.usb ? "true" : "false";
  j += ",\"vbus\":";
  j += g_pmuSnap.usb ? String(g_pmuSnap.vbus / 1000.0f, 2) : "0";
  j += ",\"vsys\":";
  j += String(g_pmuSnap.vsys / 1000.0f, 2);
  j += ",\"path\":\"";
  j += pmuPathFromSnap();
  j += "\",\"chg\":\"";
  j += pmuChgStage(g_pmuSnap.chgSt);
  j += "\",\"ichg\":";
  j += String(g_pmuSnap.ichg);
  j += ",\"vtarget\":";
  j += String(g_pmuSnap.vt);
  j += ",\"iterm\":";
  j += String(g_pmuSnap.iterm);
  j += ",\"ilim\":";
  j += String(g_pmuSnap.ilim);
  j += ",\"temp\":";
  if (isnan(g_pmuSnap.temp)) {
    j += "null";
  } else {
    j += String(g_pmuSnap.temp, 1);
  }
  j += ",\"thermal\":";
  j += g_pmuSnap.thermal ? "true" : "false";
  j += ",\"limHit\":";
  j += g_pmuSnap.limHit ? "true" : "false";
  j += ",\"warnPct\":";
  j += String(g_pmuSnap.warn);
  j += ",\"offPct\":";
  j += String(g_pmuSnap.off);
  j += "}";
}
