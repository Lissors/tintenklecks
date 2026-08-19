// WiFi STA / AP + Preferences

#include "config.h"
#include "board.h"
#include <WiFi.h>
#include <Preferences.h>

static Preferences wifiPrefs;
static bool g_apMode = false;

bool wifiIsAp() {
  return g_apMode;
}

static bool loadCreds(String &ssid, String &pass) {
  wifiPrefs.begin("wifi", true);
  ssid = wifiPrefs.getString("ssid", "");
  pass = wifiPrefs.getString("pass", "");
  wifiPrefs.end();
  return ssid.length() > 0;
}

static String apPassStored() {
  wifiPrefs.begin("wifi", true);
  String p = wifiPrefs.getString("appass", AP_PASS);
  wifiPrefs.end();
  p.trim();
  if (p.length() < 8 || p.length() > 63) {
    return String(AP_PASS);
  }
  return p;
}

String wifiApPass() { return apPassStored(); }

void wifiGetCreds(String &ssid, String &pass, String &apPass) {
  wifiPrefs.begin("wifi", true);
  ssid = wifiPrefs.getString("ssid", "");
  pass = wifiPrefs.getString("pass", "");
  apPass = wifiPrefs.getString("appass", AP_PASS);
  wifiPrefs.end();
  apPass.trim();
  if (apPass.length() < 8 || apPass.length() > 63) {
    apPass = AP_PASS;
  }
}

bool wifiSaveCreds(const String &ssid, const String &pass) {
  wifiPrefs.begin("wifi", false);
  wifiPrefs.putString("ssid", ssid);
  wifiPrefs.putString("pass", pass);
  wifiPrefs.end();
  return true;
}

bool wifiSaveApPass(const String &apPass) {
  String p = apPass;
  p.trim();
  if (p.length() < 8 || p.length() > 63) {
    return false;
  }
  wifiPrefs.begin("wifi", false);
  wifiPrefs.putString("appass", p);
  wifiPrefs.end();
  return true;
}

bool wifiClearCreds() {
  wifiPrefs.begin("wifi", false);
  wifiPrefs.remove("ssid");
  wifiPrefs.remove("pass");
  wifiPrefs.end();
  return true;
}

static bool connectSta(const String &ssid, const String &pass) {
  Serial.printf("WiFi STA → %s\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(MDNS_HOST);
  WiFi.begin(ssid.c_str(), pass.c_str());
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - t0 > WIFI_CONNECT_MS) {
      Serial.println(F("WiFi STA timeout"));
      return false;
    }
    delay(200);
    Serial.print('.');
  }
  Serial.println();
  Serial.print(F("STA IP "));
  Serial.println(WiFi.localIP());
  return true;
}

static void startAp() {
  String apPass = wifiApPass();
  Serial.printf("WiFi AP → %s\n", AP_SSID);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, apPass.c_str());
  delay(100);
  Serial.print(F("AP IP "));
  Serial.println(WiFi.softAPIP());
  g_apMode = true;
}

bool wifiBringUp() {
  String ssid, pass;
  if (loadCreds(ssid, pass) && connectSta(ssid, pass)) {
    g_apMode = false;
    tzApply();
    if (!ntpSyncToRtc()) {
      rtcApplyToSystem();
    }
    return true;
  }
  WiFi.disconnect(true);
  startAp();
  if (!rtcApplyToSystem()) {
    slideshowSetTimeOk(false);
  }
  return false;
}

void wifiLoop() {
}
