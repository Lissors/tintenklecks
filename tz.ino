// Location / POSIX TZ (DST) + NTP after wake

#include "tz_cities.h"
#include "config.h"
#include "board.h"

const char *tzCitiesJson() { return TZ_CITIES_JSON; }

#include <Preferences.h>
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>

static const char *TZ_DEFAULT_CITY = "Berlin";
static const char *TZ_DEFAULT_POSIX = "CET-1CEST,M3.5.0,M10.5.0/3";

static Preferences tzPrefs;
static String g_tzCity;
static String g_tzPosix;
static bool g_tzLoaded = false;

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

bool ntpSyncToRtc() {
  if (wifiIsAp() || WiFi.status() != WL_CONNECTED) {
    return false;
  }
  tzLoad();
  configTzTime(g_tzPosix.c_str(), "pool.ntp.org", "time.nist.gov");
  tzApply();
  Serial.println(F("NTP sync…"));
  for (int i = 0; i < 20; i++) {
    time_t now = time(nullptr);
    if (now > 1700000000) {
      tzApply();
      slideshowSetTimeOk(true);
      rtcSyncFromSystem();
      Serial.println(F("NTP OK → RTC"));
      return true;
    }
    delay(250);
    yield();
  }
  Serial.println(F("NTP timeout"));
  return false;
}
