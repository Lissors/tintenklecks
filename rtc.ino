// PCF85063A RTC (Waveshare PhotoPainter) — I2C 0x51

#include "config.h"
#include "board.h"
#include <Wire.h>
#include <sys/time.h>
#include <time.h>

static const uint8_t PCF85063_ADDR = 0x51;
static bool rtcOk = false;

static uint8_t bcd2bin(uint8_t v) {
  return (uint8_t)(((v >> 4) * 10) + (v & 0x0F));
}

static uint8_t bin2bcd(uint8_t v) {
  return (uint8_t)(((v / 10) << 4) | (v % 10));
}

static bool rtcWriteReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static bool rtcReadRegs(uint8_t reg, uint8_t *buf, size_t n) {
  Wire.beginTransmission(PCF85063_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  if (Wire.requestFrom((int)PCF85063_ADDR, (int)n) != (int)n) {
    return false;
  }
  for (size_t i = 0; i < n; i++) {
    buf[i] = (uint8_t)Wire.read();
  }
  return true;
}

bool rtcInit() {
  // Control_1: 24h mode, RTC enabled (STOP=0)
  if (!rtcWriteReg(0x00, 0x00)) {
    Serial.println(F("WARN PCF85063 not found"));
    rtcOk = false;
    return false;
  }
  rtcOk = true;
  Serial.println(F("PCF85063 RTC OK"));
  return true;
}

bool rtcReady() {
  return rtcOk;
}

bool rtcGet(struct tm *out) {
  if (!rtcOk || !out) {
    return false;
  }
  uint8_t b[7];
  if (!rtcReadRegs(0x04, b, 7)) {
    return false;
  }
  if (b[0] & 0x80) {
    // oscillator stop — time invalid
    return false;
  }
  out->tm_sec = bcd2bin(b[0] & 0x7F);
  out->tm_min = bcd2bin(b[1] & 0x7F);
  out->tm_hour = bcd2bin(b[2] & 0x3F);
  out->tm_mday = bcd2bin(b[3] & 0x3F);
  out->tm_wday = bcd2bin(b[4] & 0x07);
  out->tm_mon = bcd2bin(b[5] & 0x1F) - 1;
  out->tm_year = bcd2bin(b[6]) + 100;  // 2000+
  out->tm_isdst = -1;
  if (out->tm_mday < 1 || out->tm_mon < 0 || out->tm_mon > 11) {
    return false;
  }
  return true;
}

bool rtcSet(const struct tm *in) {
  if (!rtcOk || !in) {
    return false;
  }
  // STOP clock while writing
  rtcWriteReg(0x00, 0x20);
  bool ok = true;
  ok = rtcWriteReg(0x04, bin2bcd((uint8_t)in->tm_sec) & 0x7F) && ok;
  ok = rtcWriteReg(0x05, bin2bcd((uint8_t)in->tm_min)) && ok;
  ok = rtcWriteReg(0x06, bin2bcd((uint8_t)in->tm_hour)) && ok;
  ok = rtcWriteReg(0x07, bin2bcd((uint8_t)in->tm_mday)) && ok;
  ok = rtcWriteReg(0x08, bin2bcd((uint8_t)(in->tm_wday % 7))) && ok;
  ok = rtcWriteReg(0x09, bin2bcd((uint8_t)(in->tm_mon + 1))) && ok;
  int y = in->tm_year + 1900;
  if (y < 2000) {
    y = 2000;
  }
  if (y > 2099) {
    y = 2099;
  }
  ok = rtcWriteReg(0x0A, bin2bcd((uint8_t)(y - 2000))) && ok;
  rtcWriteReg(0x00, 0x00);  // clear STOP
  return ok;
}

bool rtcApplyToSystem() {
  struct tm t;
  if (!rtcGet(&t)) {
    return false;
  }
  time_t epoch = mktime(&t);
  if (epoch < 1700000000) {
    // before ~2023 — treat as unset
    return false;
  }
  struct timeval tv = {.tv_sec = epoch, .tv_usec = 0};
  settimeofday(&tv, nullptr);
  slideshowSetTimeOk(true);
  Serial.printf("RTC → system %04d-%02d-%02d %02d:%02d\n", t.tm_year + 1900, t.tm_mon + 1,
                t.tm_mday, t.tm_hour, t.tm_min);
  return true;
}

bool rtcSyncFromSystem() {
  time_t now = time(nullptr);
  if (now < 1700000000) {
    return false;
  }
  struct tm t;
  if (!localtime_r(&now, &t)) {
    return false;
  }
  if (!rtcSet(&t)) {
    return false;
  }
  Serial.println(F("system → RTC synced"));
  return true;
}
