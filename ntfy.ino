// Once per calendar day: ntfy if battery < 10 %. No extra wake — piggybacks on boot/loop.

#include "config.h"
#include "board.h"

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Preferences.h>
#include <time.h>

static const int NTFY_BATT_PCT = 10;
static const uint32_t NTFY_RETRY_MS = 30UL * 60UL * 1000UL;

static Preferences ntfyPrefs;
static String g_topic;
static String g_prio;
static int g_sentYday = -1;
static bool g_loaded = false;
static uint32_t g_lastTryMs = 0;

static String ntfyPrioNorm(const String &p) {
  if (p == "min" || p == "low" || p == "default" || p == "high" || p == "urgent") {
    return p;
  }
  return "high";
}

static void ntfyLoad() {
  if (g_loaded) {
    return;
  }
  ntfyPrefs.begin("ntfy", true);
  g_topic = ntfyPrefs.getString("topic", "");
  g_prio = ntfyPrioNorm(ntfyPrefs.getString("prio", "high"));
  g_sentYday = ntfyPrefs.getInt("yday", -1);
  ntfyPrefs.end();
  g_topic.trim();
  g_loaded = true;
}

void ntfyBegin() { ntfyLoad(); }

String ntfyTopic() {
  ntfyLoad();
  return g_topic;
}

String ntfyPrio() {
  ntfyLoad();
  return g_prio;
}

bool ntfySet(const String &t, const String &p) {
  ntfyLoad();
  g_topic = t;
  g_topic.trim();
  if (g_topic.length() > 96) {
    g_topic.remove(96);
  }
  g_prio = ntfyPrioNorm(p);
  ntfyPrefs.begin("ntfy", false);
  ntfyPrefs.putString("topic", g_topic);
  ntfyPrefs.putString("prio", g_prio);
  ntfyPrefs.end();
  return true;
}

static int todayKey() {
  time_t now = time(nullptr);
  if (now < 1700000000) {
    return -1;
  }
  struct tm t;
  localtime_r(&now, &t);
  return t.tm_year * 400 + t.tm_yday;
}

static String ntfyUrl() {
  if (g_topic.startsWith("http://") || g_topic.startsWith("https://")) {
    return g_topic;
  }
  String u = "https://ntfy.sh/";
  u += g_topic;
  return u;
}

static bool ntfyPost(const String &body, const char *prio, const char *tags) {
  ntfyLoad();
  if (g_topic.length() < 1) {
    return false;
  }
  if (WiFi.status() != WL_CONNECTED) {
    return false;
  }
  const String url = ntfyUrl();
  HTTPClient http;
  http.setTimeout(8000);
  int code = 0;
  const char *pr = (prio && prio[0]) ? prio : g_prio.c_str();
  if (url.startsWith("https://")) {
    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(8000);
    if (!http.begin(client, url)) {
      return false;
    }
    http.addHeader("Content-Type", "text/plain; charset=utf-8");
    http.addHeader("Title", "Tintenklecks");
    http.addHeader("Priority", pr);
    if (tags && tags[0]) {
      http.addHeader("Tags", tags);
    }
    code = http.POST(body);
    http.end();
  } else {
    if (!http.begin(url)) {
      return false;
    }
    http.addHeader("Content-Type", "text/plain; charset=utf-8");
    http.addHeader("Title", "Tintenklecks");
    http.addHeader("Priority", pr);
    if (tags && tags[0]) {
      http.addHeader("Tags", tags);
    }
    code = http.POST(body);
    http.end();
  }
  Serial.printf("ntfy POST %d\n", code);
  return code >= 200 && code < 300;
}

bool ntfySendTest() {
  ntfyLoad();
  if (g_topic.length() < 1) {
    return false;
  }
  return ntfyPost("Tintenklecks ntfy-Probe", nullptr, nullptr);
}

bool ntfySendBatteryProbe() {
  ntfyLoad();
  if (g_topic.length() < 1) {
    return false;
  }
  return ntfyPost("Akku < 10 %", nullptr, "battery,warning");
}

bool ntfySendWake() {
  ntfyLoad();
  if (g_topic.length() < 1) {
    return false;
  }
  if (wifiIsAp() || WiFi.status() != WL_CONNECTED) {
    return false;
  }
  const int bat = pmuBatteryPercent();
  String body = "Aufgewacht, Akku ";
  if (bat < 0) {
    body += "—";
  } else {
    body += String(bat);
    body += " %";
  }
  return ntfyPost(body, "min", "battery");
}

void ntfyBatteryWatch() {
  ntfyLoad();
  if (g_topic.length() < 1) {
    return;
  }
  if (powerBusy() || powerClientHere()) {
    return;
  }
  if (wifiIsAp() || WiFi.status() != WL_CONNECTED) {
    return;
  }
  if (pmuCharging()) {
    return;
  }
  const int bat = pmuBatteryPercent();
  if (bat < 0 || bat >= NTFY_BATT_PCT) {
    return;
  }
  const int yday = todayKey();
  if (yday < 0 || yday == g_sentYday) {
    return;
  }
  if (g_lastTryMs != 0 && (millis() - g_lastTryMs) < NTFY_RETRY_MS) {
    return;
  }
  g_lastTryMs = millis();
  String body = "Akku ";
  body += String(bat);
  body += " %";
  if (!ntfyPost(body, nullptr, "battery,warning")) {
    return;
  }
  g_sentYday = yday;
  ntfyPrefs.begin("ntfy", false);
  ntfyPrefs.putInt("yday", g_sentYday);
  ntfyPrefs.end();
}
