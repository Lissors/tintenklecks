// HTTP + captive DNS + mDNS

#include "config.h"
#include "board.h"
#include "html_pages.h"
#include "FS.h"
#include "SD_MMC.h"

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/time.h>
#include <time.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static WebServer server(80);
static DNSServer dns;
static const byte DNS_PORT = 53;
static bool g_offlineStudio = false;

static File uploadFile;
static String uploadPath;
static bool uploadShow = false;
static bool uploadIsNew = false;
static size_t uploadBytes = 0;
static String uploadMetaJson;

static void jsonEscapeName(const char *src, char *dst, size_t dstLen);

void webSetOfflineStudio(bool on) {
  g_offlineStudio = on;
}

static void sendProgmem(const char *page) {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", page);
}

static void handleRoot() {
  powerNoteActivity();
  if (wifiIsAp() && !g_offlineStudio) {
    sendProgmem(PAGE_SETUP);
  } else {
    sendProgmem(PAGE_MENU);
  }
}

static void handleSetup() {
  powerNoteActivity();
  sendProgmem(PAGE_SETUP);
}

static void handleMenu() {
  powerNoteActivity();
  sendProgmem(PAGE_MENU);
}

static void handleStudio() {
  powerNoteActivity();
  sendProgmem(PAGE_STUDIO);
}

static void handleSandboxRedirect() {
  powerNoteActivity();
  String loc = "/studio";
  if (server.hasArg("edit")) {
    loc += "?edit=";
    loc += server.arg("edit");
  }
  server.sendHeader("Location", loc);
  server.send(302, "text/plain", "studio");
}

static void handleGallery() {
  powerNoteActivity();
  sendProgmem(PAGE_GALLERY);
}

static void handleSystemPage() {
  powerNoteActivity();
  sendProgmem(PAGE_SYSTEM);
}

static void handleFramePage() {
  powerNoteActivity();
  sendProgmem(PAGE_FRAME);
}

static void handleLivePage() {
  powerNoteActivity();
  sendProgmem(PAGE_LIVE);
}

static void handleWifiPost() {
  powerNoteActivity();
  String ssid = server.hasArg("ssid") ? server.arg("ssid") : "";
  ssid.trim();
  String pass = server.hasArg("pass") ? server.arg("pass") : "";
  bool apOk = true;
  if (server.hasArg("apPass")) {
    String ap = server.arg("apPass");
    ap.trim();
    if (ap.length() > 0) {
      apOk = wifiSaveApPass(ap);
    }
  }
  if (!apOk) {
    server.send(400, "text/plain", "AP-Passwort 8–63 Zeichen");
    return;
  }
  if (ssid.length() > 0) {
    wifiSaveCreds(ssid, pass);
  } else if (!server.hasArg("apPass")) {
    server.send(400, "text/plain", "ssid missing");
    return;
  }
  server.send(200, "text/html; charset=utf-8",
              "<html><body style='font-family:sans-serif;background:#1a1612;color:#f3ebe3;padding:2rem'>"
              "<h1>Gespeichert</h1><p>Neustart… danach STA oder wieder AP.</p></body></html>");
  audioPlayClip("neustart");
  audioWaitIdle(5000);
  ESP.restart();
}

static void handleWifiGet() {
  powerNoteActivity();
  String ssid, pass, apPass;
  wifiGetCreds(ssid, pass, apPass);
  char escSsid[80];
  char escPass[200];
  char escAp[200];
  jsonEscapeName(ssid.c_str(), escSsid, sizeof(escSsid));
  jsonEscapeName(pass.c_str(), escPass, sizeof(escPass));
  jsonEscapeName(apPass.c_str(), escAp, sizeof(escAp));
  String j = "{\"ssid\":\"";
  j += escSsid;
  j += "\",\"pass\":\"";
  j += escPass;
  j += "\",\"apPass\":\"";
  j += escAp;
  j += "\"}";
  server.send(200, "application/json", j);
}

static void handleOffline() {
  powerNoteActivity();
  g_offlineStudio = true;
  server.sendHeader("Location", "/menu");
  server.send(302, "text/plain", "offline");
}

static void handleCaptive() {
  sendProgmem(PAGE_SETUP);
}

static String safeBmpName(String name) {
  name.replace('\\', '/');
  int slash = name.lastIndexOf('/');
  if (slash >= 0) {
    name = name.substring(slash + 1);
  }
  if (name.length() < 5) {
    name = "upload.bmp";
  }
  if (!name.endsWith(".bmp") && !name.endsWith(".BMP")) {
    name += ".bmp";
  }
  for (size_t i = 0; i < name.length(); i++) {
    char c = name[i];
    bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-';
    if (!ok) {
      name.setCharAt(i, '_');
    }
  }
  return name;
}

static String baseNameNoExt(const String &bmpName) {
  String n = bmpName;
  int slash = n.lastIndexOf('/');
  if (slash >= 0) {
    n = n.substring(slash + 1);
  }
  if (n.endsWith(".bmp") || n.endsWith(".BMP") || n.endsWith(".json") || n.endsWith(".JSON")) {
    n = n.substring(0, n.length() - (n.endsWith(".json") || n.endsWith(".JSON") ? 5 : 4));
  }
  return n;
}

static String jsonPathForBmp(const String &bmpName) {
  return String(PIC_DIR) + "/" + baseNameNoExt(bmpName) + ".json";
}

static String thumbPathForName(const String &bmpName) {
  return String(PIC_DIR) + "/" + baseNameNoExt(bmpName) + ".jpg";
}

static String srcPathForName(const String &bmpName) {
  return String(PIC_DIR) + "/" + baseNameNoExt(bmpName) + "_src.jpg";
}

static String cleanThumbPathForName(const String &bmpName) {
  return String(PIC_DIR) + "/" + baseNameNoExt(bmpName) + "_thumb.jpg";
}

static String wavPathForName(const String &bmpName) {
  return String(PIC_DIR) + "/" + baseNameNoExt(bmpName) + ".wav";
}

static String bmpPathForName(const String &bmpName) {
  return String(PIC_DIR) + "/" + safeBmpName(bmpName);
}

static bool isBmpFileName(const String &name) {
  return name.endsWith(".bmp") || name.endsWith(".BMP");
}

static const char *const STEM_SUFFIX[] = {
    ".bmp",      ".json",      ".wav",      ".jpg",       ".jpeg",      ".png",
    "_src.jpg",  "_src.jpeg",  "_src.png",  "_thumb.jpg", "_thumb.jpeg", "_thumb.png",
};
static const int STEM_SUFFIX_N = 12;

static int stemSuffixLen(const char *base) {
  size_t len = strlen(base);
  int best = 0;
  for (int i = 0; i < STEM_SUFFIX_N; i++) {
    size_t sl = strlen(STEM_SUFFIX[i]);
    if (len > sl && (int)sl > best && strcasecmp(base + len - sl, STEM_SUFFIX[i]) == 0) {
      best = (int)sl;
    }
  }
  return best;
}

static int collectStemFiles(const String &stem, String *out, int maxOut) {
  const size_t sl = stem.length();
  if (sl == 0 || maxOut <= 0) {
    return 0;
  }
  char dirpath[48];
  snprintf(dirpath, sizeof(dirpath), "%s%s", SD_MOUNT, PIC_DIR);
  DIR *d = opendir(dirpath);
  if (!d) {
    return 0;
  }
  int n = 0;
  struct dirent *ent;
  while (n < maxOut && (ent = readdir(d)) != nullptr) {
    const char *base = ent->d_name;
    if (!base || !base[0] || base[0] == '.' || base[0] == '_' || ent->d_type == DT_DIR) {
      continue;
    }
    int suf = stemSuffixLen(base);
    if (!suf || strlen(base) != sl + (size_t)suf) {
      continue;
    }
    if (strncasecmp(base, stem.c_str(), sl) != 0) {
      continue;
    }
    if (suf > 5) {
      size_t dot = strlen(base) - 1;
      while (dot > 0 && base[dot] != '.') {
        dot--;
      }
      if (dot > 0 &&
          SD_MMC.exists(String(PIC_DIR) + "/" + String(base).substring(0, dot) + ".bmp")) {
        continue;
      }
    }
    out[n++] = String(base);
  }
  closedir(d);
  return n;
}

static String g_listCache;
static bool g_listCacheValid = false;
static volatile bool g_listBuilding = false;
static volatile bool g_listReload = false;
static volatile uint32_t g_listGen = 0;
static volatile bool g_orphanSweepWanted = true;
static const char *LIST_CACHE_PATH = "/pic/_gallery.json";

static String readWholeFile(const String &path);
static bool writeWholeFile(const String &path, const String &data);
static void kickListBuild();
static int cleanupOrphansRun();
static void sendFileCached(File &f, const char *mime);

static void invalidateListCache() {
  g_listCacheValid = false;
}

static void invalidateListCacheHard() {
  g_listCacheValid = false;
  g_listGen = g_listGen + 1;
  g_orphanSweepWanted = true;
  g_listCache = "";
  if (sdOk() && SD_MMC.exists(LIST_CACHE_PATH)) {
    SD_MMC.remove(LIST_CACHE_PATH);
  }
  kickListBuild();
}

static String readWholeFile(const String &path) {
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return "";
  }
  String s;
  s.reserve((size_t)f.size() + 8);
  uint8_t buf[512];
  int n;
  while ((n = f.read(buf, sizeof(buf))) > 0) {
    s.concat(buf, (unsigned int)n);
  }
  f.close();
  return s;
}

static bool writeWholeFile(const String &path, const String &data) {
  if (SD_MMC.exists(path)) {
    SD_MMC.remove(path);
  }
  File f = SD_MMC.open(path, FILE_WRITE);
  if (!f) {
    return false;
  }
  size_t n = f.print(data);
  f.close();
  return n == data.length() || (data.length() == 0 && n == 0);
}

static void handleUpload() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    powerNoteBusy(true);
    uploadShow = server.hasArg("show") && server.arg("show") == "1";
    uploadMetaJson = server.hasArg("meta") ? server.arg("meta") : "";
    String name = safeBmpName(up.filename);
    uploadPath = String(PIC_DIR) + "/" + name;
    uploadBytes = 0;
    uploadIsNew = false;
    if (!sdOk()) {
      return;
    }
    uploadIsNew = !SD_MMC.exists(uploadPath);
    if (SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    uploadFile = SD_MMC.open(uploadPath, FILE_WRITE);
    Serial.printf("UPLOAD start %s\n", uploadPath.c_str());
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(up.buf, up.currentSize);
      uploadBytes += up.currentSize;
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    Serial.printf("UPLOAD end %u bytes\n", (unsigned)uploadBytes);
  }
}

static void handleUploadDone() {
  powerNoteBusy(false);
  powerNoteActivity();
  if (server.hasArg("show")) {
    uploadShow = server.arg("show") == "1";
  }
  if (server.hasArg("meta")) {
    uploadMetaJson = server.arg("meta");
  }
  if (!sdOk() || uploadPath.length() == 0) {
    server.send(500, "text/plain", "SD/upload failed");
    return;
  }
  if (uploadMetaJson.length() > 0) {
    String jp = jsonPathForBmp(uploadPath);
    if (!writeWholeFile(jp, uploadMetaJson)) {
      Serial.println(F("WARN meta write failed"));
    } else {
      Serial.printf("META wrote %s\n", jp.c_str());
    }
  }
  String msg = "OK " + uploadPath + " (" + String(uploadBytes) + " bytes)";
  if (uploadShow) {
    server.send(200, "text/plain", msg);
    delay(50);
    powerNoteBusy(true);
    if (bmpShowFromSd(uploadPath.c_str())) {
      String base = uploadPath;
      int slash = base.lastIndexOf('/');
      if (slash >= 0) {
        base = base.substring(slash + 1);
      }
      slideshowNoteShown(base);
    }
    powerNoteBusy(false);
  } else {
    server.send(200, "text/plain", msg);
  }
  uploadMetaJson = "";
  {
    String base = uploadPath;
    int slash = base.lastIndexOf('/');
    if (slash >= 0) {
      base = base.substring(slash + 1);
    }
    if (base.length() && base.charAt(0) != '_') {
      if (uploadIsNew) {
        slideshowDeckAdd(base);
      }
      invalidateListCacheHard();
    }
  }
}

static bool loadListCacheFromSd() {
  if (!sdOk() || !SD_MMC.exists(LIST_CACHE_PATH)) {
    return false;
  }
  String s = readWholeFile(LIST_CACHE_PATH);
  if (s.length() < 2 || s.charAt(0) != '[') {
    return false;
  }
  if (s != "[]" && s.indexOf("\"ct\":0") < 0 && s.indexOf("\"ct\":1") < 0) {
    Serial.println(F("list cache stale (no ct flag) — rebuild"));
    return false;
  }
  if (s != "[]" && s.indexOf("\"name\":") < 0) {
    Serial.println(F("list cache stale (no name field) — rebuild"));
    return false;
  }
  g_listCache = s;
  g_listCacheValid = true;
  Serial.printf("list cache loaded %u bytes\n", (unsigned)s.length());
  return true;
}

static bool markCleanThumbInIndex(const String &bmpName) {
  if (bmpName.length() == 0) {
    return false;
  }
  if (g_listBuilding) {
    return false;
  }
  if (g_listReload) {
    g_listReload = false;
    g_listCache = "";
    loadListCacheFromSd();
  }
  if (g_listCache.length() == 0) {
    return false;
  }
  String key = String("{\"file\":\"") + baseNameNoExt(bmpName) + ".bmp\"";
  int at = g_listCache.indexOf(key);
  if (at < 0) {
    return false;
  }
  int end = g_listCache.indexOf('}', at);
  int flag = g_listCache.indexOf("\"ct\":", at);
  if (flag < 0 || end < 0 || flag > end) {
    return false;
  }
  int pos = flag + 5;
  if (pos >= (int)g_listCache.length()) {
    return false;
  }
  if (g_listCache.charAt(pos) == '1') {
    return true;
  }
  if (g_listCache.charAt(pos) != '0') {
    return false;
  }
  g_listCache.setCharAt(pos, '1');
  File f = SD_MMC.open(LIST_CACHE_PATH, "r+");
  if (!f) {
    invalidateListCache();
    return true;
  }
  f.seek((uint32_t)pos);
  f.write((uint8_t)'1');
  f.close();
  return true;
}

static uint16_t hashStem(const char *s) {
  uint32_t h = 2166136261u;
  while (*s) {
    h ^= (uint8_t)(*s++);
    h *= 16777619u;
  }
  return (uint16_t)(h & 511u);
}

static void utf8Trunc(char *s) {
  int i = 0;
  while (s[i]) {
    uint8_t c = (uint8_t)s[i];
    int need = c < 0x80 ? 1 : (c >= 0xF0 ? 4 : (c >= 0xE0 ? 3 : (c >= 0xC0 ? 2 : 0)));
    if (need == 0) {
      break;
    }
    int k = 1;
    while (k < need && ((uint8_t)s[i + k] & 0xC0) == 0x80) {
      k++;
    }
    if (k < need) {
      break;
    }
    i += need;
  }
  s[i] = 0;
}

static void readStemName(const char *stem, char *out, size_t outLen) {
  out[0] = 0;
  char path[80];
  snprintf(path, sizeof(path), "%s/%s.json", PIC_DIR, stem);
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return;
  }
  char buf[512];
  int n = (int)f.read((uint8_t *)buf, sizeof(buf) - 1);
  f.close();
  if (n <= 0) {
    return;
  }
  buf[n] = 0;
  const char *k = nullptr;
  for (int i = 0; i + 6 <= n; i++) {
    if (buf[i] == '"' && strncmp(buf + i, "\"name\"", 6) == 0) {
      k = buf + i + 6;
      break;
    }
  }
  if (!k) {
    return;
  }
  while (*k == ' ' || *k == '\t') {
    k++;
  }
  if (*k++ != ':') {
    return;
  }
  while (*k == ' ' || *k == '\t') {
    k++;
  }
  if (*k++ != '"') {
    return;
  }
  size_t o = 0;
  while (*k && *k != '"' && o + 1 < outLen) {
    char c = *k++;
    if (c == '\\') {
      char e = *k;
      if (!e) {
        break;
      }
      k++;
      switch (e) {
        case 'b':
        case 'f':
        case 'n':
        case 'r':
        case 't':
          c = ' ';
          break;
        case 'u':
          for (int j = 0; j < 4 && *k; j++) {
            k++;
          }
          c = ' ';
          break;
        default:
          c = e;
          break;
      }
    }
    out[o++] = c;
  }
  out[o] = 0;
  utf8Trunc(out);
}

static const char LAT1_FOLD[65] =
    "AAAAAAACEEEEIIIIDNOOOOOxOUUUUYTsaaaaaaaceeeeiiiidnooooo/ouuuuyty";

static uint8_t foldNext(const uint8_t **pp) {
  const uint8_t *p = *pp;
  uint8_t c = *p++;
  if (c == 0xC3 && *p >= 0x80 && *p <= 0xBF) {
    c = (uint8_t)LAT1_FOLD[*p - 0x80];
    p++;
  }
  *pp = p;
  return (uint8_t)tolower(c);
}

static int cmpFolded(const char *a, const char *b) {
  const uint8_t *pa = (const uint8_t *)a;
  const uint8_t *pb = (const uint8_t *)b;
  while (*pa && *pb) {
    uint8_t ca = foldNext(&pa);
    uint8_t cb = foldNext(&pb);
    if (ca != cb) {
      return ca < cb ? -1 : 1;
    }
  }
  return *pa ? 1 : (*pb ? -1 : 0);
}

static int cmpNameThenStem(const char *na, const char *sa, const char *nb, const char *sb) {
  const bool ea = (na[0] == 0);
  const bool eb = (nb[0] == 0);
  if (ea != eb) {
    return ea ? -1 : 1;
  }
  if (!ea) {
    int c = cmpFolded(na, nb);
    if (c != 0) {
      return c;
    }
  }
  return strcmp(sa, sb);
}

static void jsonEscapeName(const char *src, char *dst, size_t dstLen) {
  size_t o = 0;
  for (const uint8_t *p = (const uint8_t *)src; *p && o + 2 < dstLen; p++) {
    if (*p == '"' || *p == '\\') {
      dst[o++] = '\\';
      dst[o++] = (char)*p;
    } else if (*p < 0x20) {
      dst[o++] = ' ';
    } else {
      dst[o++] = (char)*p;
    }
  }
  dst[o] = 0;
}

static String buildListJsonToSd() {
  const char *tmpPath = "/pic/_gallery.tmp";
  if (SD_MMC.exists(tmpPath)) {
    SD_MMC.remove(tmpPath);
  }

  char dirpath[48];
  snprintf(dirpath, sizeof(dirpath), "%s%s", SD_MOUNT, PIC_DIR);
  DIR *d = opendir(dirpath);
  if (!d) {
    Serial.println(F("list build: opendir failed"));
    return "";
  }

  static const int MAX_STEMS = 600;
  static const int STEM_LEN = 56;
  static const int NAME_LEN = 64;
  static const int HASH_N = 512;
  char (*stems)[STEM_LEN] = (char (*)[STEM_LEN])ps_malloc((size_t)MAX_STEMS * STEM_LEN);
  char (*names)[NAME_LEN] = (char (*)[NAME_LEN])ps_malloc((size_t)MAX_STEMS * NAME_LEN);
  uint8_t *flags = (uint8_t *)ps_malloc(MAX_STEMS);
  int16_t *head = (int16_t *)ps_malloc(HASH_N * sizeof(int16_t));
  int16_t *nxt = (int16_t *)ps_malloc(MAX_STEMS * sizeof(int16_t));
  if (!stems || !names || !flags || !head || !nxt) {
    if (stems) {
      free(stems);
    }
    if (names) {
      free(names);
    }
    if (flags) {
      free(flags);
    }
    if (head) {
      free(head);
    }
    if (nxt) {
      free(nxt);
    }
    closedir(d);
    Serial.println(F("list build: no PSRAM"));
    return "";
  }
  memset(flags, 0, MAX_STEMS);
  for (int i = 0; i < HASH_N; i++) {
    head[i] = -1;
  }
  int nStem = 0;
  uint32_t t0 = millis();
  uint16_t tick = 0;
  bool stemOverflow = false;

  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr) {
    const char *base = ent->d_name;
    if (!base || !base[0] || base[0] == '.' || base[0] == '_') {
      continue;
    }
    if (ent->d_type == DT_DIR) {
      continue;
    }
    size_t len = strlen(base);
    if (len <= 4) {
      continue;
    }
    const char *ext = base + len - 4;
    const bool dotBmp =
        ext[0] == '.' && (ext[1] == 'b' || ext[1] == 'B') && (ext[2] == 'm' || ext[2] == 'M') &&
        (ext[3] == 'p' || ext[3] == 'P');
    const bool dotJpg =
        ext[0] == '.' && (ext[1] == 'j' || ext[1] == 'J') && (ext[2] == 'p' || ext[2] == 'P') &&
        (ext[3] == 'g' || ext[3] == 'G');
    char stem[STEM_LEN];
    stem[0] = 0;
    uint8_t bit = 0;
    if (dotBmp) {
      size_t n = len - 4;
      if (n >= STEM_LEN) {
        n = STEM_LEN - 1;
      }
      memcpy(stem, base, n);
      stem[n] = 0;
      bit = 1;
    } else if (dotJpg) {
      if (len > 10 && strncasecmp(base + len - 10, "_thumb.jpg", 10) == 0) {
        size_t n = len - 10;
        if (n >= STEM_LEN) {
          n = STEM_LEN - 1;
        }
        memcpy(stem, base, n);
        stem[n] = 0;
        bit = 8;
      } else if (len > 8 && strncasecmp(base + len - 8, "_src.jpg", 8) == 0) {
        size_t n = len - 8;
        if (n >= STEM_LEN) {
          n = STEM_LEN - 1;
        }
        memcpy(stem, base, n);
        stem[n] = 0;
        bit = 4;
      } else {
        size_t n = len - 4;
        if (n >= STEM_LEN) {
          n = STEM_LEN - 1;
        }
        memcpy(stem, base, n);
        stem[n] = 0;
        bit = 2;
      }
    }
    if (bit && stem[0]) {
      uint16_t hv = hashStem(stem);
      int ix = -1;
      for (int i = head[hv]; i >= 0; i = nxt[i]) {
        if (strcmp(stems[i], stem) == 0) {
          ix = i;
          break;
        }
      }
      if (ix < 0 && nStem >= MAX_STEMS) {
        stemOverflow = true;
      }
      if (ix < 0 && nStem < MAX_STEMS) {
        ix = nStem++;
        strncpy(stems[ix], stem, STEM_LEN - 1);
        stems[ix][STEM_LEN - 1] = 0;
        flags[ix] = 0;
        nxt[ix] = head[hv];
        head[hv] = (int16_t)ix;
      }
      if (ix >= 0) {
        flags[ix] |= bit;
      }
    }
    if ((++tick & 63) == 0) {
      yield();
    }
  }
  closedir(d);
  if (stemOverflow) {
    Serial.printf("list build: more than %d images — gallery truncated\n", MAX_STEMS);
  }

  uint32_t tName = millis();
  for (int i = 0; i < nStem; i++) {
    names[i][0] = 0;
    if (flags[i] & 1) {
      readStemName(stems[i], names[i], NAME_LEN);
    }
    if ((i & 7) == 0) {
      yield();
    }
  }
  tName = millis() - tName;

  int16_t *order = nxt;
  for (int i = 0; i < nStem; i++) {
    order[i] = (int16_t)i;
  }
  for (int gap = nStem / 2; gap > 0; gap /= 2) {
    for (int i = gap; i < nStem; i++) {
      int16_t v = order[i];
      int j = i;
      while (j >= gap) {
        int16_t w = order[j - gap];
        if (cmpNameThenStem(names[w], stems[w], names[v], stems[v]) <= 0) {
          break;
        }
        order[j] = w;
        j -= gap;
      }
      order[j] = v;
    }
    yield();
  }

  File out = SD_MMC.open(tmpPath, FILE_WRITE);
  if (!out) {
    free(stems);
    free(names);
    free(flags);
    free(head);
    free(nxt);
    return "";
  }
  out.print('[');
  bool first = true;
  uint16_t nBmp = 0;
  uint16_t nClean = 0;
  uint16_t nOrphan = 0;
  uint16_t nNamed = 0;
  char nameOut[NAME_LEN * 2];
  for (int k = 0; k < nStem; k++) {
    const int i = order[k];
    if ((flags[i] & 1) == 0) {
      nOrphan++;
      continue;
    }
    if (!first) {
      out.print(',');
    }
    first = false;
    const bool hasPreview = (flags[i] & 14) != 0;
    const bool hasClean = (flags[i] & 8) != 0;
    if (hasClean) {
      nClean++;
    }
    if (names[i][0]) {
      nNamed++;
    }
    jsonEscapeName(names[i], nameOut, sizeof(nameOut));
    out.print("{\"file\":\"");
    out.print(stems[i]);
    out.print(".bmp\",\"name\":\"");
    out.print(nameOut);
    out.print("\",\"thumb\":");
    out.print(hasPreview ? "true" : "false");
    out.print(",\"src\":");
    out.print((flags[i] & 4) ? "true" : "false");
    out.print(",\"ct\":");
    out.print(hasClean ? '1' : '0');
    out.print('}');
    nBmp++;
  }
  out.print(']');
  out.close();
  free(stems);
  free(names);
  free(flags);
  free(head);
  free(nxt);

  if (SD_MMC.exists(LIST_CACHE_PATH)) {
    SD_MMC.remove(LIST_CACHE_PATH);
  }
  SD_MMC.rename(tmpPath, LIST_CACHE_PATH);
  Serial.printf(
      "list index %u bmps (%u clean thumbs, %u named, %u orphan stems) in %u ms (%u ms names)\n",
      (unsigned)nBmp, (unsigned)nClean, (unsigned)nNamed, (unsigned)nOrphan,
      (unsigned)(millis() - t0), (unsigned)tName);
  return LIST_CACHE_PATH;
}

static void listBuildTask(void *arg) {
  (void)arg;
  powerNoteBusy(true);
  Serial.println(F("list build start"));
  bool raced = false;
  for (int pass = 0; pass < 3; pass++) {
    uint32_t gen = g_listGen;
    buildListJsonToSd();
    if (g_orphanSweepWanted) {
      g_orphanSweepWanted = false;
      int swept = cleanupOrphansRun();
      if (swept > 0) {
        Serial.printf("orphan sweep removed %d file(s) — reindex\n", swept);
        buildListJsonToSd();
      }
    }
    raced = (g_listGen != gen);
    if (!raced) {
      break;
    }
    Serial.println(F("list build raced a delete — rescan"));
  }
  g_listReload = true;
  g_listBuilding = false;
  powerNoteBusy(false);
  Serial.println(F("list build done"));
  if (raced) {
    kickListBuild();
  }
  vTaskDelete(nullptr);
}

static void kickListBuild() {
  if (g_listBuilding || !sdOk()) {
    return;
  }
  g_listBuilding = true;
  BaseType_t ok = xTaskCreatePinnedToCore(listBuildTask, "listBuild", 16384, nullptr, 1, nullptr, 1);
  if (ok != pdPASS) {
    g_listBuilding = false;
    Serial.println(F("list build task failed"));
  }
}

static void handleList() {
  if (!sdOk()) {
    server.send(500, "application/json", "{\"error\":\"no SD\"}");
    return;
  }
  if (g_listReload && !g_listBuilding) {
    g_listReload = false;
    g_listCache = "";
  }
  if (g_listCache.length() == 0) {
    loadListCacheFromSd();
  }
  if (g_listCache.length() > 0) {
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", g_listCache);
    if (!g_listCacheValid && !g_listBuilding) {
      kickListBuild();
    }
    return;
  }
  kickListBuild();
  server.sendHeader("Cache-Control", "no-store");
  server.sendHeader("Retry-After", "2");
  server.send(200, "application/json", "[]");
}

static void handleListRebuild() {
  powerNoteActivity();
  if (!sdOk()) {
    server.send(500, "text/plain", "no SD");
    return;
  }
  if (g_listBuilding) {
    server.send(200, "text/plain", "BUSY");
    return;
  }
  g_listCacheValid = false;
  g_listCache = "";
  if (SD_MMC.exists(LIST_CACHE_PATH)) {
    SD_MMC.remove(LIST_CACHE_PATH);
  }
  kickListBuild();
  server.send(200, "text/plain", "OK rebuild started");
}

static int cleanupOrphansPass(bool *more) {
  *more = false;
  if (!sdOk()) {
    return -1;
  }
  char dirpath[48];
  snprintf(dirpath, sizeof(dirpath), "%s%s", SD_MOUNT, PIC_DIR);
  DIR *d = opendir(dirpath);
  if (!d) {
    return -1;
  }
  static const int MAX_STEMS = 600;
  static const int STEM_LEN = 56;
  static const int HASH_N = 512;
  char (*stems)[STEM_LEN] = (char (*)[STEM_LEN])ps_malloc((size_t)MAX_STEMS * STEM_LEN);
  int16_t *head = (int16_t *)ps_malloc(HASH_N * sizeof(int16_t));
  int16_t *nxt = (int16_t *)ps_malloc(MAX_STEMS * sizeof(int16_t));
  if (!stems || !head || !nxt) {
    if (stems) {
      free(stems);
    }
    if (head) {
      free(head);
    }
    if (nxt) {
      free(nxt);
    }
    closedir(d);
    return -1;
  }
  for (int i = 0; i < HASH_N; i++) {
    head[i] = -1;
  }
  int nStem = 0;
  uint16_t tick = 0;
  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr) {
    const char *base = ent->d_name;
    if (!base || !base[0] || base[0] == '.' || base[0] == '_' || ent->d_type == DT_DIR) {
      continue;
    }
    size_t len = strlen(base);
    if (len <= 4 || strcasecmp(base + len - 4, ".bmp") != 0) {
      continue;
    }
    char stem[STEM_LEN];
    size_t n = len - 4;
    if (n >= STEM_LEN) {
      n = STEM_LEN - 1;
    }
    for (size_t i = 0; i < n; i++) {
      stem[i] = (char)tolower((unsigned char)base[i]);
    }
    stem[n] = 0;
    uint16_t hv = hashStem(stem);
    if (nStem < MAX_STEMS) {
      strncpy(stems[nStem], stem, STEM_LEN - 1);
      stems[nStem][STEM_LEN - 1] = 0;
      nxt[nStem] = head[hv];
      head[hv] = (int16_t)nStem;
      nStem++;
    }
    if ((++tick & 63) == 0) {
      yield();
    }
  }

  static const int KILL_MAX = 64;
  String kill[KILL_MAX];
  int nKill = 0;
  closedir(d);
  d = opendir(dirpath);
  if (!d) {
    free(stems);
    free(head);
    free(nxt);
    return -1;
  }
  while ((ent = readdir(d)) != nullptr) {
    const char *base = ent->d_name;
    if (!base || !base[0] || base[0] == '.' || base[0] == '_' || ent->d_type == DT_DIR) {
      continue;
    }
    size_t len = strlen(base);
    int suf = stemSuffixLen(base);
    if (!suf || strcasecmp(base + len - 4, ".bmp") == 0) {
      continue;
    }
    char stem[STEM_LEN];
    size_t n = len - (size_t)suf;
    if (n >= STEM_LEN) {
      n = STEM_LEN - 1;
    }
    for (size_t i = 0; i < n; i++) {
      stem[i] = (char)tolower((unsigned char)base[i]);
    }
    stem[n] = 0;
    bool known = false;
    for (int i = head[hashStem(stem)]; i >= 0; i = nxt[i]) {
      if (strcmp(stems[i], stem) == 0) {
        known = true;
        break;
      }
    }
    bool leftoverSidecar = (suf == 4 && (strcasecmp(base + len - 4, ".jpg") == 0 ||
                                         strcasecmp(base + len - 4, ".wav") == 0)) ||
                           (suf == 5 && strcasecmp(base + len - 5, ".jpeg") == 0);
    if (known && !leftoverSidecar) {
      continue;
    }
    if (!known) {
      if (SD_MMC.exists(String(PIC_DIR) + "/" + String(base).substring(0, len - (size_t)suf) +
                        ".bmp")) {
        continue;
      }
      size_t dot = len - 1;
      while (dot > 0 && base[dot] != '.') {
        dot--;
      }
      if (dot > 0 && dot != len - (size_t)suf &&
          SD_MMC.exists(String(PIC_DIR) + "/" + String(base).substring(0, dot) + ".bmp")) {
        continue;
      }
    }
    if (nKill < KILL_MAX) {
      kill[nKill++] = String(base);
    } else {
      *more = true;
    }
    if ((++tick & 63) == 0) {
      yield();
    }
  }
  closedir(d);
  free(stems);
  free(head);
  free(nxt);

  int gone = 0;
  for (int i = 0; i < nKill; i++) {
    if (SD_MMC.remove(String(PIC_DIR) + "/" + kill[i])) {
      gone++;
      Serial.printf("ORPHAN del %s\n", kill[i].c_str());
    }
  }
  return gone;
}

static bool keepUnderscoreName(const char *base) {
  return !strcmp(base, "_gallery.json") || !strcmp(base, "_gallery.tmp") ||
         !strcmp(base, "_deck.txt");
}

static int cleanupUnderscorePic() {
  if (!sdOk()) {
    return -1;
  }
  char dirpath[48];
  snprintf(dirpath, sizeof(dirpath), "%s%s", SD_MOUNT, PIC_DIR);
  DIR *d = opendir(dirpath);
  if (!d) {
    return -1;
  }
  static const int MAX_KILL = 80;
  static const int NAME_LEN = 64;
  char (*kill)[NAME_LEN] = (char (*)[NAME_LEN])malloc((size_t)MAX_KILL * NAME_LEN);
  if (!kill) {
    closedir(d);
    return -1;
  }
  int nKill = 0;
  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr) {
    const char *base = ent->d_name;
    if (!base || base[0] != '_' || ent->d_type == DT_DIR) {
      continue;
    }
    if (keepUnderscoreName(base)) {
      continue;
    }
    if (nKill < MAX_KILL) {
      strncpy(kill[nKill], base, NAME_LEN - 1);
      kill[nKill][NAME_LEN - 1] = 0;
      nKill++;
    }
  }
  closedir(d);
  int gone = 0;
  for (int i = 0; i < nKill; i++) {
    if (SD_MMC.remove(String(PIC_DIR) + "/" + kill[i])) {
      gone++;
      Serial.printf("DEL _ %s\n", kill[i]);
    }
  }
  free(kill);
  return gone;
}

static int cleanupOrphansRun() {
  int total = 0;
  int hidden = cleanupUnderscorePic();
  if (hidden > 0) {
    total += hidden;
  }
  for (int round = 0; round < 16; round++) {
    bool more = false;
    int gone = cleanupOrphansPass(&more);
    if (gone < 0) {
      return total > 0 ? total : -1;
    }
    total += gone;
    if (!more || gone == 0) {
      break;
    }
  }
  return total;
}

static void handleCleanupOrphans() {
  powerNoteActivity();
  if (!sdOk()) {
    server.send(500, "text/plain", "no SD");
    return;
  }
  if (g_listBuilding) {
    server.send(200, "text/plain", "Index wird gerade gebaut — dabei wird ohnehin aufgeräumt.");
    return;
  }
  int gone = cleanupOrphansRun();
  if (gone < 0) {
    server.send(500, "text/plain", "Aufräumen fehlgeschlagen");
    return;
  }
  if (gone > 0) {
    invalidateListCacheHard();
  }
  server.send(200, "text/plain", "OK " + String(gone) + " Fragmente entfernt");
}

static void sendFileCached(File &f, const char *mime) {
  String etag = "\"" + String((uint32_t)f.size(), HEX) + "-" +
                String((uint32_t)f.getLastWrite(), HEX) + "\"";
  server.sendHeader("ETag", etag);
  server.sendHeader("Cache-Control", "no-cache");
  if (server.header("If-None-Match") == etag) {
    f.close();
    server.send(304, mime, "");
    return;
  }
  f.seek(0);
  server.streamFile(f, mime);
  f.close();
}

static void handlePic() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String path = bmpPathForName(server.arg("name"));
  if (!SD_MMC.exists(path)) {
    server.send(404, "text/plain", "not found");
    return;
  }
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    server.send(500, "text/plain", "open failed");
    return;
  }
  server.sendHeader("Cache-Control", "no-store");
  server.streamFile(f, "image/bmp");
  f.close();
}

static void handleThumb() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  bool ours = true;
  File f = SD_MMC.open(cleanThumbPathForName(server.arg("name")), FILE_READ);
  if (!f) {
    ours = false;
    f = SD_MMC.open(thumbPathForName(server.arg("name")), FILE_READ);
  }
  if (!f) {
    f = SD_MMC.open(srcPathForName(server.arg("name")), FILE_READ);
    if (f && f.size() > 400ul * 1024ul) {
      f.close();
      f = File();
    }
  }
  if (!f) {
    server.send(404, "text/plain", "no thumb");
    return;
  }
  if (f.size() < 4) {
    f.close();
    server.send(500, "text/plain", "open failed");
    return;
  }
  if (!ours) {
    uint8_t mag[2] = {0, 0};
    if (f.read(mag, 2) != 2 || mag[0] != 0xFF || mag[1] != 0xD8) {
      f.close();
      server.send(404, "text/plain", "no thumb");
      return;
    }
    f.seek(0);
  }
  server.sendHeader("Cache-Control", "no-store");
  server.streamFile(f, "image/jpeg");
  f.close();
}

static void handleSrc() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String path = srcPathForName(server.arg("name"));
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    server.send(404, "text/plain", "no src");
    return;
  }
  const size_t sz = f.size();
  if (server.hasArg("info")) {
    f.close();
    server.sendHeader("Cache-Control", "no-store");
    server.send(200, "application/json", "{\"size\":" + String((unsigned)sz) + "}");
    return;
  }
  if (sz < 32 || sz > 6ul * 1024ul * 1024ul) {
    f.close();
    server.sendHeader("Cache-Control", "no-store");
    server.send(sz < 32 ? 404 : 413, "text/plain", sz < 32 ? "no src" : "src too large");
    return;
  }
  sendFileCached(f, "image/jpeg");
}

static void handleThumbUpload() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    powerNoteBusy(true);
    uploadBytes = 0;
    String base = server.hasArg("name") ? server.arg("name") : up.filename;
    String kind = server.hasArg("kind") ? server.arg("kind") : "";
    if (kind == "src") {
      uploadPath = srcPathForName(base);
    } else if (kind == "clean") {
      uploadPath = cleanThumbPathForName(base);
    } else {
      uploadPath = thumbPathForName(base);
    }
    if (!sdOk() || !SD_MMC.exists(bmpPathForName(base))) {
      uploadPath = "";
      return;
    }
    if (SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    uploadFile = SD_MMC.open(uploadPath, FILE_WRITE);
    Serial.printf("IMG start %s\n", uploadPath.c_str());
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(up.buf, up.currentSize);
      uploadBytes += up.currentSize;
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    Serial.printf("IMG end %u bytes\n", (unsigned)uploadBytes);
  }
}

static void handleThumbUploadDone() {
  powerNoteBusy(false);
  if (!sdOk() || uploadPath.length() == 0 || uploadBytes < 32) {
    if (uploadPath.length() && sdOk() && SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    server.send(500, "text/plain", "thumb upload failed");
    return;
  }
  server.send(200, "text/plain", "OK " + uploadPath + " (" + String(uploadBytes) + " bytes)");
  if (server.arg("kind") == "clean" && markCleanThumbInIndex(server.arg("name"))) {
    return;
  }
  invalidateListCache();
}

static void handleThumbClear() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  SD_MMC.remove(thumbPathForName(server.arg("name")));
  SD_MMC.remove(cleanThumbPathForName(server.arg("name")));
  invalidateListCacheHard();
  server.send(200, "text/plain", "OK cleared");
}

static void handleAudioUpload() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    powerNoteBusy(true);
    uploadBytes = 0;
    uploadPath = "";
    if (sdOk()) {
      String old = wavPathForName(server.hasArg("name") ? server.arg("name") : up.filename);
      if (old.length() && SD_MMC.exists(old)) {
        SD_MMC.remove(old);
      }
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
  }
}

static void handleAudioUploadDone() {
  powerNoteBusy(false);
  if (!sdOk() || uploadPath.length() == 0 || uploadBytes < 44) {
    if (uploadPath.length() && SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    server.send(410, "text/plain", "per-image wav removed");
    return;
  }
  invalidateListCache();
  server.send(200, "text/plain", "OK " + uploadPath + " (" + String(uploadBytes) + " bytes)");
}

static void handleMetaGet() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "application/json", "{}");
    return;
  }
  String meta = readWholeFile(jsonPathForBmp(server.arg("name")));
  server.sendHeader("Cache-Control", "no-store");
  if (meta.length() == 0) {
    server.send(404, "application/json", "null");
    return;
  }
  server.send(200, "application/json", meta);
}

static void handleMetaPost() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String json = server.hasArg("json") ? server.arg("json") : server.arg("plain");
  if (json.length() == 0) {
    server.send(400, "text/plain", "json body required");
    return;
  }
  if (!SD_MMC.exists(bmpPathForName(server.arg("name")))) {
    server.send(404, "text/plain", "image gone");
    return;
  }
  String jp = jsonPathForBmp(server.arg("name"));
  if (!writeWholeFile(jp, json)) {
    server.send(500, "text/plain", "write failed");
    return;
  }
  server.send(200, "text/plain", "OK " + jp);
  invalidateListCacheHard();
}

static void handleDelete() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String stem = baseNameNoExt(server.arg("name"));
  if (stem.length() == 0) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String names[16];
  int n = collectStemFiles(stem, names, 16);
  int gone = 0;
  for (int i = 0; i < n; i++) {
    if (SD_MMC.remove(String(PIC_DIR) + "/" + names[i])) {
      gone++;
      Serial.printf("DEL %s\n", names[i].c_str());
      if (isBmpFileName(names[i])) {
        slideshowForgetShown(names[i]);
        slideshowDeckRemove(names[i]);
      }
    }
  }
  invalidateListCacheHard();
  if (gone == 0) {
    server.send(404, "text/plain", "not found");
    return;
  }
  server.send(200, "text/plain", "OK deleted " + String(gone) + "/" + String(n) + " files");
}

static void handleRename() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name") || !server.hasArg("to")) {
    server.send(400, "text/plain", "name and to required");
    return;
  }
  String fromBmp = bmpPathForName(server.arg("name"));
  String toBmp = bmpPathForName(server.arg("to"));
  String fromStem = baseNameNoExt(server.arg("name"));
  String toStem = baseNameNoExt(safeBmpName(server.arg("to")));
  if (fromStem.length() == 0 || toStem.length() == 0) {
    server.send(400, "text/plain", "name and to required");
    return;
  }
  if (!SD_MMC.exists(fromBmp)) {
    server.send(404, "text/plain", "source not found");
    return;
  }
  if (SD_MMC.exists(toBmp)) {
    server.send(409, "text/plain", "target exists");
    return;
  }
  if (!SD_MMC.rename(fromBmp, toBmp)) {
    server.send(500, "text/plain", "rename bmp failed");
    return;
  }
  String names[16];
  int n = collectStemFiles(fromStem, names, 16);
  for (int i = 0; i < n; i++) {
    String suffix = names[i].substring(fromStem.length());
    String from = String(PIC_DIR) + "/" + names[i];
    String to = String(PIC_DIR) + "/" + toStem + suffix;
    if (from == to) {
      continue;
    }
    if (SD_MMC.exists(to)) {
      SD_MMC.remove(to);
    }
    SD_MMC.rename(from, to);
  }
  invalidateListCacheHard();
  server.send(200, "text/plain", "OK " + toBmp);
}

static void handleAudioGet() {
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String path = wavPathForName(server.arg("name"));
  if (!SD_MMC.exists(path)) {
    server.send(404, "text/plain", "no audio");
    return;
  }
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    server.send(500, "text/plain", "open failed");
    return;
  }
  sendFileCached(f, "audio/wav");
}

static void handleSpeak() {
  powerNoteActivity();
  if (!audioEnsureReady()) {
    server.send(503, "text/plain", "audio not ready (ES8311?)");
    return;
  }
  if (server.hasArg("test")) {
    if (!audioPlayBeep(500)) {
      server.send(500, "text/plain", "beep failed");
      return;
    }
    server.send(200, "text/plain", "OK beep");
    return;
  }
  if (!sdOk() || !server.hasArg("clip")) {
    server.send(400, "text/plain", "use test=1 or clip=welcome");
    return;
  }
  String clip = server.arg("clip");
  clip.replace("..", "");
  clip.replace('/', '_');
  clip.replace('\\', '_');
  if (clip.length() < 1 || clip.length() > 48) {
    server.send(400, "text/plain", "bad clip");
    return;
  }
  String path = String("/sound/") + clip + ".wav";
  if (!SD_MMC.exists(path)) {
    server.send(404, "text/plain", "clip not found");
    return;
  }
  if (!audioPlaySd(path.c_str())) {
    server.send(500, "text/plain", "play failed");
    return;
  }
  server.send(200, "text/plain", "OK speaking");
}

static void handleSoundUpload() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    powerNoteBusy(true);
    uploadBytes = 0;
    String clip = server.hasArg("clip") ? server.arg("clip") : "";
    clip.replace("..", "");
    clip.replace('/', '_');
    clip.replace('\\', '_');
    uploadPath = "";
    if (!sdOk() || clip.length() < 1 || clip.length() > 48) {
      return;
    }
    if (!SD_MMC.exists("/sound")) {
      SD_MMC.mkdir("/sound");
    }
    uploadPath = String("/sound/") + clip + ".wav";
    if (SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    uploadFile = SD_MMC.open(uploadPath, FILE_WRITE);
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(up.buf, up.currentSize);
      uploadBytes += up.currentSize;
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
  }
}

static void handleSoundUploadDone() {
  powerNoteBusy(false);
  if (!sdOk() || uploadPath.length() == 0 || uploadBytes < 44) {
    if (uploadPath.length() && SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    server.send(400, "text/plain", "sound upload failed");
    return;
  }
  server.send(200, "text/plain", "OK " + uploadPath + " (" + String(uploadBytes) + " bytes)");
}

static void handleShow() {
  powerNoteActivity();
  if (!sdOk() || !server.hasArg("name")) {
    server.send(400, "text/plain", "name required");
    return;
  }
  String path = bmpPathForName(server.arg("name"));
  if (!SD_MMC.exists(path)) {
    server.send(404, "text/plain", "not found");
    return;
  }
  String shown = safeBmpName(server.arg("name"));
  server.send(200, "text/plain", "OK");
  delay(50);
  powerNoteBusy(true);
  if (bmpShowFromSd(path.c_str())) {
    slideshowNoteShown(shown);
  }
  powerNoteBusy(false);
}

static const char *SHOW_BMP_PATH = "/pic/_preview.bmp";

static void handleShowBmp() {
  HTTPUpload &up = server.upload();
  if (up.status == UPLOAD_FILE_START) {
    powerNoteBusy(true);
    uploadBytes = 0;
    uploadPath = SHOW_BMP_PATH;
    if (!sdOk()) {
      uploadPath = "";
      powerNoteBusy(false);
      return;
    }
    if (SD_MMC.exists(uploadPath)) {
      SD_MMC.remove(uploadPath);
    }
    uploadFile = SD_MMC.open(uploadPath, FILE_WRITE);
    Serial.println(F("SHOW-BMP start"));
  } else if (up.status == UPLOAD_FILE_WRITE) {
    if (uploadFile) {
      uploadFile.write(up.buf, up.currentSize);
      uploadBytes += up.currentSize;
    }
  } else if (up.status == UPLOAD_FILE_END) {
    if (uploadFile) {
      uploadFile.close();
    }
    Serial.printf("SHOW-BMP end %u bytes\n", (unsigned)uploadBytes);
  }
}

static void handleShowBmpDone() {
  powerNoteActivity();
  if (!sdOk() || uploadPath.length() == 0 || uploadBytes < 54) {
    powerNoteBusy(false);
    server.send(500, "text/plain", "show failed");
    return;
  }
  server.send(200, "text/plain", "OK shown");
  delay(50);
  bmpShowFromSd(SHOW_BMP_PATH);
  powerNoteBusy(false);
}

static void handleDisplay() {
  powerNoteActivity();
  if (server.hasArg("blank") && server.arg("blank") == "1") {
    powerNoteBusy(true);
    epdClear(EPD_WHITE);
    epdDisplayCurrentBuffer();
    powerNoteBusy(false);
    server.send(200, "text/plain", "OK blank");
    return;
  }
  server.send(400, "text/plain", "use blank=1");
}

static void handleBattWarnTest() {
  powerNoteActivity();
  bool ntfyOk = ntfySendBatteryProbe();
  powerNoteBusy(true);
  epdForceBatteryWarn(true);
  bool ok = slideshowReshowLast();
  if (!ok) {
    epdDisplayCurrentBuffer();
  }
  epdForceBatteryWarn(false);
  powerNoteBusy(false);
  String msg = ok ? "OK warn" : "OK warn (kein letztes Bild)";
  if (!ntfyOk) {
    msg += " · ntfy fehlt (Thema/WLAN?)";
  } else {
    msg += " · ntfy gesendet";
  }
  server.send(200, "text/plain", msg);
}

static void handleReboot() {
  powerNoteActivity();
  server.send(200, "text/plain", "OK reboot");
  audioPlayClip("neustart");
  audioWaitIdle(5000);
  ESP.restart();
}

static void handleSleepNow() {
  if (pmuUsbPowered()) {
    server.send(200, "text/plain", "USB");
    return;
  }
  server.send(200, "text/plain", "OK sleep");
  delay(200);
  powerSleepNow();
}

static void handleWifiClear() {
  powerNoteActivity();
  wifiClearCreds();
  server.send(200, "text/plain", "OK wifi cleared — reboot");
  audioPlayClip("neustart");
  audioWaitIdle(5000);
  ESP.restart();
}

static void handleNtfyGet() {
  String t = ntfyTopic();
  char esc[200];
  jsonEscapeName(t.c_str(), esc, sizeof(esc));
  String j = "{\"topic\":\"";
  j += esc;
  j += "\",\"prio\":\"";
  j += ntfyPrio();
  j += "\"}";
  server.send(200, "application/json", j);
}

static void handleNtfyPost() {
  powerNoteActivity();
  String t = server.hasArg("topic") ? server.arg("topic") : "";
  String p = server.hasArg("prio") ? server.arg("prio") : "high";
  ntfySet(t, p);
  server.send(200, "text/plain", "OK");
}

static void handleNtfyTest() {
  powerNoteActivity();
  if (!ntfySendTest()) {
    server.send(500, "text/plain", "fehlgeschlagen (WLAN/Thema?)");
    return;
  }
  server.send(200, "text/plain", "OK");
}

static void handlePalette() {
  server.send(200, "application/json",
              "{\"black\":{\"r\":2,\"g\":2,\"b\":2},\"white\":{\"r\":190,\"g\":200,\"b\":200},"
              "\"yellow\":{\"r\":205,\"g\":202,\"b\":0},\"red\":{\"r\":135,\"g\":19,\"b\":0},"
              "\"blue\":{\"r\":5,\"g\":64,\"b\":158},\"green\":{\"r\":39,\"g\":102,\"b\":60},"
              "\"source\":\"esp32-photoframe-defaults\"}");
}

static void handleStatus() {
  powerNoteActivity();
  int bat = pmuBatteryPercent();
  String j = "{";
  j += "\"device\":\"";
  j += DEVICE_NAME;
  j += "\",\"copyright\":\"";
  j += COPYRIGHT;
  j += "\",\"ap\":";
  j += wifiIsAp() ? "true" : "false";
  j += ",\"ip\":\"";
  if (wifiIsAp()) {
    j += WiFi.softAPIP().toString();
  } else {
    j += WiFi.localIP().toString();
  }
  j += "\",\"sd\":";
  j += sdOk() ? "true" : "false";
  j += ",\"battery\":";
  j += String(bat);
  j += ",\"usb\":";
  j += pmuUsbPowered() ? "true" : "false";
  j += ",\"charging\":";
  j += pmuCharging() ? "true" : "false";
  j += ",\"voltage\":";
  j += String(pmuBattVoltage(), 2);
  j += ",\"heap\":";
  j += String((unsigned)ESP.getFreeHeap());
  j += ",\"pmu\":";
  j += pmuReady() ? "true" : "false";
  j += "}";
  server.send(200, "application/json", j);
}

static void handleFrameGet() {
  String out;
  slideshowGetJson(out);
  server.send(200, "application/json", out);
}

static void handleFramePost() {
  powerNoteActivity();
  int mode = server.hasArg("mode") ? server.arg("mode").toInt() : 0;
  int imin = server.hasArg("intervalMin") ? server.arg("intervalMin").toInt() : 5;
  int dh = server.hasArg("dailyHour") ? server.arg("dailyHour").toInt() : 8;
  int dm = server.hasArg("dailyMin") ? server.arg("dailyMin").toInt() : 0;
  if (!slideshowSet(mode, imin, dh, dm)) {
    server.send(400, "text/plain", "invalid settings");
    return;
  }
  String out;
  slideshowGetJson(out);
  server.send(200, "application/json", out);
}

static void handleFrameNow() {
  powerNoteActivity();
  powerNoteBusy(true);
  slideshowForceNow();
  powerNoteBusy(false);
  server.send(200, "text/plain", "OK switched");
}

static void handleTzGet() {
  String out;
  tzGetJson(out);
  server.send(200, "application/json", out);
}

static void handleTzPost() {
  powerNoteActivity();
  String city = server.hasArg("city") ? server.arg("city") : "";
  String posix = server.hasArg("posix") ? server.arg("posix") : "";
  if (!tzSet(city, posix)) {
    server.send(400, "text/plain", "Stadt/Zeitzone ungültig");
    return;
  }
  ntpSyncToRtc();
  String out;
  tzGetJson(out);
  server.send(200, "application/json", out);
}

static void handleTzCities() {
  server.sendHeader("Cache-Control", "public, max-age=86400");
  server.send_P(200, "application/json; charset=utf-8", tzCitiesJson());
}

static void handleTimeGet() {
  String out;
  slideshowGetJson(out);
  server.send(200, "application/json", out);
}

static void handleTimePost() {
  powerNoteActivity();
  if (server.hasArg("epoch")) {
    time_t e = (time_t)strtoul(server.arg("epoch").c_str(), nullptr, 10);
    if (e < 1700000000) {
      server.send(400, "text/plain", "epoch invalid");
      return;
    }
    tzApply();
    struct timeval tv = {.tv_sec = e, .tv_usec = 0};
    settimeofday(&tv, nullptr);
    slideshowSetTimeOk(true);
    if (!rtcSyncFromSystem()) {
      server.send(500, "text/plain", "RTC write failed");
      return;
    }
    String out;
    slideshowGetJson(out);
    server.send(200, "application/json", out);
    return;
  }
  if (!server.hasArg("iso") && !(server.hasArg("year") && server.hasArg("month") && server.hasArg("day"))) {
    server.send(400, "text/plain", "need iso=YYYY-MM-DDTHH:MM or epoch= or year/month/day/hour/min");
    return;
  }
  struct tm t = {};
  if (server.hasArg("iso")) {
    String iso = server.arg("iso");
    int y = iso.substring(0, 4).toInt();
    int mo = iso.substring(5, 7).toInt();
    int d = iso.substring(8, 10).toInt();
    int hh = 12, mm = 0;
    int tpos = iso.indexOf('T');
    if (tpos < 0) {
      tpos = iso.indexOf(' ');
    }
    if (tpos > 0 && iso.length() >= (size_t)(tpos + 5)) {
      hh = iso.substring(tpos + 1, tpos + 3).toInt();
      mm = iso.substring(tpos + 4, tpos + 6).toInt();
    }
    t.tm_year = y - 1900;
    t.tm_mon = mo - 1;
    t.tm_mday = d;
    t.tm_hour = hh;
    t.tm_min = mm;
    t.tm_sec = 0;
    t.tm_isdst = -1;
  } else {
    t.tm_year = server.arg("year").toInt() - 1900;
    t.tm_mon = server.arg("month").toInt() - 1;
    t.tm_mday = server.arg("day").toInt();
    t.tm_hour = server.hasArg("hour") ? server.arg("hour").toInt() : 12;
    t.tm_min = server.hasArg("min") ? server.arg("min").toInt() : 0;
    t.tm_sec = 0;
    t.tm_isdst = -1;
  }
  tzApply();
  mktime(&t);
  if (!rtcSet(&t)) {
    server.send(500, "text/plain", "RTC write failed");
    return;
  }
  if (!rtcApplyToSystem()) {
    server.send(500, "text/plain", "system time failed");
    return;
  }
  String out;
  slideshowGetJson(out);
  server.send(200, "application/json", out);
}

void webBegin() {
  if (wifiIsAp()) {
    dns.start(DNS_PORT, "*", WiFi.softAPIP());
  }

  server.on("/", HTTP_GET, handleRoot);
  server.on("/setup", HTTP_GET, handleSetup);
  server.on("/menu", HTTP_GET, handleMenu);
  server.on("/studio", HTTP_GET, handleStudio);
  server.on("/sandbox", HTTP_GET, handleSandboxRedirect);
  server.on("/sandbox2", HTTP_GET, handleSandboxRedirect);
  server.on("/gallery", HTTP_GET, handleGallery);
  server.on("/system", HTTP_GET, handleSystemPage);
  server.on("/frame", HTTP_GET, handleFramePage);
  server.on("/live", HTTP_GET, handleLivePage);
  server.on("/wifi", HTTP_POST, handleWifiPost);
  server.on("/api/wifi", HTTP_GET, handleWifiGet);
  server.on("/offline", HTTP_POST, handleOffline);

  static const char *KEEP_HEADERS[] = {"If-None-Match"};
  server.collectHeaders(KEEP_HEADERS, 1);

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/frame", HTTP_GET, handleFrameGet);
  server.on("/api/frame", HTTP_POST, handleFramePost);
  server.on("/api/frame-now", HTTP_POST, handleFrameNow);
  server.on("/api/time", HTTP_GET, handleTimeGet);
  server.on("/api/time", HTTP_POST, handleTimePost);
  server.on("/api/tz", HTTP_GET, handleTzGet);
  server.on("/api/tz", HTTP_POST, handleTzPost);
  server.on("/api/tz-cities", HTTP_GET, handleTzCities);
  server.on("/api/palette", HTTP_GET, handlePalette);
  server.on("/api/list", HTTP_GET, handleList);
  server.on("/api/list-rebuild", HTTP_POST, handleListRebuild);
  server.on("/api/cleanup-orphans", HTTP_POST, handleCleanupOrphans);
  server.on("/api/pic", HTTP_GET, handlePic);
  server.on("/api/thumb", HTTP_GET, handleThumb);
  server.on("/api/src", HTTP_GET, handleSrc);
  server.on("/api/audio", HTTP_GET, handleAudioGet);
  server.on("/api/speak", HTTP_POST, handleSpeak);
  server.on("/api/sound", HTTP_POST, handleSoundUploadDone, handleSoundUpload);
  server.on("/api/meta", HTTP_GET, handleMetaGet);
  server.on("/api/meta", HTTP_POST, handleMetaPost);
  server.on("/api/delete", HTTP_POST, handleDelete);
  server.on("/api/rename", HTTP_POST, handleRename);
  server.on("/api/show", HTTP_POST, handleShow);
  server.on("/api/show-bmp", HTTP_POST, handleShowBmpDone, handleShowBmp);
  server.on("/api/display", HTTP_POST, handleDisplay);
  server.on("/api/batt-warn", HTTP_POST, handleBattWarnTest);
  server.on("/api/reboot", HTTP_POST, handleReboot);
  server.on("/api/sleep", HTTP_POST, handleSleepNow);
  server.on("/api/wifi-clear", HTTP_POST, handleWifiClear);
  server.on("/api/ntfy", HTTP_GET, handleNtfyGet);
  server.on("/api/ntfy", HTTP_POST, handleNtfyPost);
  server.on("/api/ntfy-test", HTTP_POST, handleNtfyTest);

  server.on("/api/upload", HTTP_POST, handleUploadDone, handleUpload);
  server.on("/api/upload-thumb", HTTP_POST, handleThumbUploadDone, handleThumbUpload);
  server.on("/api/thumb-clear", HTTP_POST, handleThumbClear);
  server.on("/api/upload-audio", HTTP_POST, handleAudioUploadDone, handleAudioUpload);

  server.on("/generate_204", handleCaptive);
  server.on("/gen_204", handleCaptive);
  server.on("/hotspot-detect.html", handleCaptive);
  server.on("/canonical.html", handleCaptive);
  server.on("/ncsi.txt", handleCaptive);
  server.on("/connecttest.txt", handleCaptive);
  server.on("/redirect", handleCaptive);
  server.onNotFound([]() {
    if (wifiIsAp()) {
      sendProgmem(PAGE_SETUP);
    } else {
      server.send(404, "text/plain", "not found");
    }
  });

  server.begin();
  Serial.println(F("HTTP :80"));
}

void webAfterSdReady() {
  if (!sdOk()) {
    return;
  }
  int hidden = cleanupUnderscorePic();
  if (hidden > 0) {
    Serial.printf("boot: removed %d /pic/_ file(s)\n", hidden);
  }
  if (!loadListCacheFromSd()) {
    Serial.println(F("no list cache — background build"));
    kickListBuild();
  }
}

void webLoop() {
  if (wifiIsAp()) {
    dns.processNextRequest();
  }
  server.handleClient();

  static bool bootSweepDone = false;
  if (!bootSweepDone && millis() > 15000 && sdOk()) {
    bootSweepDone = true;
    kickListBuild();
  }

  static bool mdnsTried = false;
  if (!mdnsTried && millis() > 8000) {
    mdnsTried = true;
    if (!wifiIsAp()) {
      if (MDNS.begin(MDNS_HOST)) {
        MDNS.addService("http", "tcp", 80);
        Serial.printf("mDNS http://%s.local\n", MDNS_HOST);
      } else {
        Serial.println(F("mDNS failed — use IP"));
      }
    }
    ArduinoOTA.setHostname(MDNS_HOST);
    ArduinoOTA.onStart([]() {
      powerNoteBusy(true);
      powerNoteActivity();
      Serial.println(F("OTA from Arduino IDE"));
    });
    ArduinoOTA.onEnd([]() {
      powerNoteBusy(false);
      Serial.println(F("OTA end"));
    });
    ArduinoOTA.onError([](ota_error_t e) {
      powerNoteBusy(false);
      Serial.printf("OTA error %u\n", (unsigned)e);
    });
    ArduinoOTA.begin();
    Serial.println(F("Arduino OTA: Port tintenklecks"));
  }
  ArduinoOTA.handle();

  audioLoop();
}
