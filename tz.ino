// Location / POSIX TZ (DST) + NTP after wake

#include "tz_cities.h"
#include "config.h"
#include "board.h"

const char *tzCitiesJson() { return TZ_CITIES_JSON; }

#include <Preferences.h>
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>
#include "esp_sntp.h"

static const char *TZ_DEFAULT_CITY = "Berlin";
static const char *TZ_DEFAULT_POSIX = "CET-1CEST,M3.5.0,M10.5.0/3";

static Preferences tzPrefs;
static String g_tzCity;
static String g_tzPosix;
static bool g_tzLoaded = false;

static const uint32_t NTP_HOLD_MS = 10UL * 60UL * 1000UL;
static uint32_t g_ntpHoldUntil = 0;
static bool g_ntpOk = false;
static bool g_ntpTried = false;
static char g_ntpAt[24] = "";
static volatile bool g_sntpGot = false;

static void ntpSyncCb(struct timeval *tv) {
  (void)tv;
  g_sntpGot = true;
}

static bool ntpHoldActive() {
  if (!g_ntpHoldUntil) {
    return false;
  }
  if ((int32_t)(millis() - g_ntpHoldUntil) >= 0) {
    return false;
  }
  return true;
}

static uint32_t ntpHoldRemainSec() {
  if (!ntpHoldActive()) {
    return 0;
  }
  uint32_t left = g_ntpHoldUntil - millis();
  return (left + 999UL) / 1000UL;
}

static void ntpNoteSuccess() {
  g_ntpOk = true;
  g_ntpTried = true;
  time_t now = time(nullptr);
  struct tm ti;
  if (now > 1700000000 && localtime_r(&now, &ti)) {
    snprintf(g_ntpAt, sizeof(g_ntpAt), "%04d-%02d-%02dT%02d:%02d", ti.tm_year + 1900, ti.tm_mon + 1,
             ti.tm_mday, ti.tm_hour, ti.tm_min);
  } else {
    g_ntpAt[0] = 0;
  }
  slideshowInvalidateMemPreview();
}

static bool tzPosixOk(const String &p) {
  if (p.length() < 2 || p.length() > 64) {
    return false;
  }
  for (size_t i = 0; i < p.length(); i++) {
    char c = p[i];
    const bool ok = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
                    c == '<' || c == '>' || c == '-' || c == '+' || c == ',' || c == '.' || c == ':' ||
                    c == '/';
    if (!ok) {
      return false;
    }
  }
  return true;
}

static void tzLoad() {
  if (g_tzLoaded) {
    return;
  }
  tzPrefs.begin("tz", true);
  g_tzCity = tzPrefs.getString("city", TZ_DEFAULT_CITY);
  g_tzPosix = tzPrefs.getString("posix", TZ_DEFAULT_POSIX);
  tzPrefs.end();
  g_tzCity.trim();
  g_tzPosix.trim();
  if (g_tzCity.length() < 1 || g_tzCity.length() > 48) {
    g_tzCity = TZ_DEFAULT_CITY;
  }
  if (!tzPosixOk(g_tzPosix)) {
    g_tzPosix = TZ_DEFAULT_POSIX;
  }
  g_tzLoaded = true;
}

void tzApply() {
  tzLoad();
  setenv("TZ", g_tzPosix.c_str(), 1);
  tzset();
}

void tzBegin() {
  tzLoad();
  tzApply();
}

String tzCity() {
  tzLoad();
  return g_tzCity;
}

String tzPosix() {
  tzLoad();
  return g_tzPosix;
}

bool tzSet(const String &city, const String &posix) {
  tzLoad();
  String c = city;
  c.trim();
  String p = posix;
  p.trim();
  if (c.length() < 1 || c.length() > 48 || !tzPosixOk(p)) {
    return false;
  }
  g_tzCity = c;
  g_tzPosix = p;
  tzPrefs.begin("tz", false);
  tzPrefs.putString("city", g_tzCity);
  tzPrefs.putString("posix", g_tzPosix);
  tzPrefs.end();
  tzApply();
  return true;
}

void tzGetJson(String &out) {
  tzLoad();
  out = "{\"city\":\"";
  out += g_tzCity;
  out += "\",\"posix\":\"";
  out += g_tzPosix;
  out += "\"}";
}

void ntpHoldAfterManual() {
  esp_sntp_stop();
  sntp_set_time_sync_notification_cb(nullptr);
  g_sntpGot = false;
  g_ntpHoldUntil = millis() + NTP_HOLD_MS;
  g_ntpOk = false;
  g_ntpTried = false;
  g_ntpAt[0] = 0;
  slideshowInvalidateMemPreview();
}

void ntpLoop() {
  if (!g_ntpHoldUntil) {
    return;
  }
  if ((int32_t)(millis() - g_ntpHoldUntil) < 0) {
    return;
  }
  g_ntpHoldUntil = 0;
  ntpSyncToRtc();
}

void ntpAppendJson(String &out) {
  out += ",\"ntp\":\"";
  if (ntpHoldActive()) {
    out += "hold";
  } else if (wifiIsAp()) {
    out += "ap";
  } else if (g_ntpOk) {
    out += "ok";
  } else if (g_ntpTried) {
    out += "fail";
  } else {
    out += "off";
  }
  out += "\",\"ntpAt\":";
  if (g_ntpAt[0]) {
    out += "\"";
    out += g_ntpAt;
    out += "\"";
  } else {
    out += "null";
  }
  out += ",\"ntpHoldSec\":";
  out += String(ntpHoldRemainSec());
}

bool ntpSyncToRtc() {
  if (ntpHoldActive()) {
    Serial.println(F("NTP hold"));
    return false;
  }
  if (wifiIsAp() || WiFi.status() != WL_CONNECTED) {
    return false;
  }
  tzLoad();
  esp_sntp_stop();
  sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
  sntp_set_sync_status(SNTP_SYNC_STATUS_RESET);
  g_sntpGot = false;
  sntp_set_time_sync_notification_cb(ntpSyncCb);
  configTzTime(g_tzPosix.c_str(), "pool.ntp.org", "time.nist.gov");
  tzApply();
  g_ntpTried = true;
  g_ntpOk = false;
  Serial.println(F("NTP sync…"));
  for (int i = 0; i < 80; i++) {
    if (g_sntpGot || sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED) {
      time_t now = time(nullptr);
      if (now > 1700000000) {
        tzApply();
        slideshowSetTimeOk(true);
        rtcSyncFromSystem();
        ntpNoteSuccess();
        Serial.println(F("NTP OK → RTC"));
        return true;
      }
    }
    delay(250);
    yield();
  }
  Serial.println(F("NTP timeout"));
  return false;
}
