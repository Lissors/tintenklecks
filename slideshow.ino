// Automatic picture rotation + anniversary preference (birth, death, special)

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"
#include <Preferences.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

static Preferences framePrefs;

// mode: 0=off, 1=interval minutes, 2=once per day
static int g_mode = 0;
static int g_intervalMin = 5;
static int g_dailyHour = 8;
static int g_dailyMin = 0;
static int g_hang = 0;  // 0=Hochkant 480×800, 1=Quer 800×480
static uint32_t g_lastSwitchMs = 0;
static time_t g_lastSwitchEpoch = 0;  // wall clock; survives deep sleep via NVS
static int g_lastDailyYday = -1;
static String g_lastFile;
static bool g_timeOk = false;

static bool intervalOk(int m) {
  return m == 5 || m == 10 || m == 30 || m == 60;
}

static void noteSwitchTime() {
  g_lastSwitchMs = millis();
  time_t now = time(nullptr);
  if (now > 1700000000) {
    g_lastSwitchEpoch = now;
    framePrefs.begin("frame", false);
    framePrefs.putULong("lsepoch", (uint32_t)g_lastSwitchEpoch);
    framePrefs.end();
  }
}

static String jsonField(const String &json, const char *key) {
  String needle = String("\"") + key + "\":";
  int p = json.indexOf(needle);
  if (p < 0) {
    return "";
  }
  p += needle.length();
  while (p < (int)json.length() && (json[p] == ' ')) {
    p++;
  }
  if (p >= (int)json.length()) {
    return "";
  }
  if (json[p] == '"') {
    p++;
    int e = json.indexOf('"', p);
    if (e < 0) {
      return "";
    }
    return json.substring(p, e);
  }
  return "";
}

static bool parseDayMonth(const String &s, int &day, int &month) {
  String t = s;
  t.trim();
  if (t.length() < 5) {
    return false;
  }
  int d = t.substring(0, 2).toInt();
  int m = t.substring(3, 5).toInt();
  if (d < 1 || d > 31 || m < 1 || m > 12) {
    return false;
  }
  day = d;
  month = m;
  return true;
}

static void addDays(int &y, int &m, int &d, int add) {
  struct tm t = {};
  t.tm_year = y - 1900;
  t.tm_mon = m - 1;
  t.tm_mday = d + add;
  t.tm_hour = 12;
  mktime(&t);
  y = t.tm_year + 1900;
  m = t.tm_mon + 1;
  d = t.tm_mday;
}

static bool matchAnniversarySoon(const String &date, int ty, int tm, int td) {
  int bd, bm;
  if (!parseDayMonth(date, bd, bm)) {
    return false;
  }
  int y1 = ty, m1 = tm, d1 = td;
  int y2 = ty, m2 = tm, d2 = td;
  addDays(y1, m1, d1, 1);
  addDays(y2, m2, d2, 2);
  (void)y1;
  (void)y2;
  return (bm == m1 && bd == d1) || (bm == m2 && bd == d2);
}

static String readMetaFile(const String &bmpName) {
  String base = bmpName;
  int slash = base.lastIndexOf('/');
  if (slash >= 0) {
    base = base.substring(slash + 1);
  }
  if (base.endsWith(".bmp") || base.endsWith(".BMP")) {
    base = base.substring(0, base.length() - 4);
  }
  String path = String(PIC_DIR) + "/" + base + ".json";
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return "";
  }
  String s;
  s.reserve((size_t)f.size() + 8);
  while (f.available()) {
    s += (char)f.read();
  }
  f.close();
  return s;
}

static const int SLIDE_MAX = 256;
static const char *DECK_PATH = "/pic/_deck.txt";

static void shuffleStrings(String *a, int n) {
  for (int i = n - 1; i > 0; i--) {
    int j = random(i + 1);
    String tmp = a[i];
    a[i] = a[j];
    a[j] = tmp;
  }
}

static bool nameIn(const String *a, int n, const String &s) {
  for (int i = 0; i < n; i++) {
    if (a[i] == s) {
      return true;
    }
  }
  return false;
}

static int deckLoad(String *out, int maxN) {
  File f = SD_MMC.open(DECK_PATH, FILE_READ);
  if (!f) {
    return 0;
  }
  int n = 0;
  while (f.available() && n < maxN) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      out[n++] = line;
    }
  }
  f.close();
  return n;
}

static void deckSave(const String *a, int n) {
  if (SD_MMC.exists(DECK_PATH)) {
    SD_MMC.remove(DECK_PATH);
  }
  if (n < 1) {
    return;
  }
  File f = SD_MMC.open(DECK_PATH, FILE_WRITE);
  if (!f) {
    return;
  }
  for (int i = 0; i < n; i++) {
    f.println(a[i]);
  }
  f.close();
}

void slideshowDeckRemove(const String &file) {
  if (file.length() < 1 || !sdOk()) {
    return;
  }
  String *deck = new String[SLIDE_MAX];
  if (!deck) {
    return;
  }
  int dn = deckLoad(deck, SLIDE_MAX);
  int w = 0;
  for (int i = 0; i < dn; i++) {
    if (deck[i] != file) {
      deck[w++] = deck[i];
    }
  }
  if (w != dn) {
    deckSave(deck, w);
  }
  delete[] deck;
}

void slideshowDeckAdd(const String &file) {
  if (file.length() < 1 || file.charAt(0) == '_' || !sdOk()) {
    return;
  }
  String *deck = new String[SLIDE_MAX];
  if (!deck) {
    return;
  }
  int dn = deckLoad(deck, SLIDE_MAX);
  if (dn < 1 || dn >= SLIDE_MAX || nameIn(deck, dn, file)) {
    delete[] deck;
    return;
  }
  int pos = random(dn + 1);
  for (int i = dn; i > pos; i--) {
    deck[i] = deck[i - 1];
  }
  deck[pos] = file;
  deckSave(deck, dn + 1);
  delete[] deck;
}

static bool pickFromPot(const String *pool, int poolN, const String &avoid, String &picked) {
  if (poolN < 1) {
    return false;
  }
  String *deck = new String[SLIDE_MAX];
  if (!deck) {
    return false;
  }
  int dn = deckLoad(deck, SLIDE_MAX);
  int w = 0;
  for (int i = 0; i < dn; i++) {
    if (nameIn(pool, poolN, deck[i])) {
      deck[w++] = deck[i];
    }
  }
  dn = w;
  if (dn < 1) {
    for (int i = 0; i < poolN; i++) {
      deck[i] = pool[i];
    }
    dn = poolN;
    shuffleStrings(deck, dn);
    if (dn > 1 && avoid.length() > 0 && deck[dn - 1] == avoid) {
      int j = random(dn - 1);
      String tmp = deck[dn - 1];
      deck[dn - 1] = deck[j];
      deck[j] = tmp;
    }
  }
  picked = deck[dn - 1];
  dn--;
  deckSave(deck, dn);
  delete[] deck;
  return true;
}

static bool collectCandidates(PicCand *out, int maxN, int &count) {
  count = 0;
  if (!sdOk()) {
    return false;
  }
  char dirpath[48];
  snprintf(dirpath, sizeof(dirpath), "%s%s", SD_MOUNT, PIC_DIR);
  DIR *d = opendir(dirpath);
  if (!d) {
    return false;
  }

  time_t now = time(nullptr);
  struct tm ti;
  bool haveTime = g_timeOk && now > 1700000000 && localtime_r(&now, &ti);
  int ty = haveTime ? (ti.tm_year + 1900) : 0;
  int tm = haveTime ? (ti.tm_mon + 1) : 0;
  int td = haveTime ? ti.tm_mday : 0;

  uint16_t tick = 0;
  struct dirent *ent;
  while ((ent = readdir(d)) != nullptr && count < maxN) {
    const char *base = ent->d_name;
    if (!base || !base[0] || base[0] == '.' || base[0] == '_') {
      continue;
    }
    if (ent->d_type == DT_DIR) {
      continue;
    }
    size_t len = strlen(base);
    if (len < 5) {
      continue;
    }
    const char *ext = base + len - 4;
    const bool dotBmp =
        ext[0] == '.' && (ext[1] == 'b' || ext[1] == 'B') && (ext[2] == 'm' || ext[2] == 'M') &&
        (ext[3] == 'p' || ext[3] == 'P');
    if (!dotBmp) {
      continue;
    }
    String meta = readMetaFile(base);
    String birth = jsonField(meta, "birth");
    String death = jsonField(meta, "death");
    String special = jsonField(meta, "special");
    String kind = jsonField(meta, "kind");
    PicCand &c = out[count];
    c.file = base;
    if (kind == "memory") {
      c.noDates = false;
    } else if (kind == "normal") {
      c.noDates = true;
    } else {
      c.noDates = (birth.length() == 0 && death.length() == 0 && special.length() == 0);
    }
    c.anniversarySoon =
        !c.noDates && haveTime &&
        (matchAnniversarySoon(birth, ty, tm, td) || matchAnniversarySoon(death, ty, tm, td) ||
         matchAnniversarySoon(special, ty, tm, td));
    count++;
    if ((++tick & 15) == 0) {
      yield();
    }
  }
  closedir(d);
  return count > 0;
}

static bool pickNextFiles(String *paths, int &outCount, bool allowMemory) {
  outCount = 0;
  PicCand *cands = new PicCand[SLIDE_MAX];
  if (!cands) {
    return false;
  }
  int n = 0;
  if (!collectCandidates(cands, SLIDE_MAX, n)) {
    delete[] cands;
    return false;
  }

  // Timer: Erinnerung morgen/übermorgen, bis zu 3. KEY: diesen Block nicht.
  if (allowMemory) {
    int specialIdx[SLIDE_MAX];
    int specialN = 0;
    for (int i = 0; i < n; i++) {
      if (cands[i].anniversarySoon) {
        specialIdx[specialN++] = i;
      }
    }
    if (specialN > 0) {
      for (int i = specialN - 1; i > 0; i--) {
        int j = random(i + 1);
        int tmp = specialIdx[i];
        specialIdx[i] = specialIdx[j];
        specialIdx[j] = tmp;
      }
      outCount = specialN > 3 ? 3 : specialN;
      for (int i = 0; i < outCount; i++) {
        paths[i] = cands[specialIdx[i]].file;
      }
      delete[] cands;
      return true;
    }
  }

  // Nur Zufall (keine Erinnerungen). Ohne solche Bilder: nichts.
  String *pool = new String[SLIDE_MAX];
  if (!pool) {
    delete[] cands;
    return false;
  }
  int poolN = 0;
  for (int i = 0; i < n; i++) {
    if (cands[i].noDates) {
      pool[poolN++] = cands[i].file;
    }
  }
  delete[] cands;
  if (poolN < 1) {
    delete[] pool;
    return false;
  }

  String avoid = g_lastFile;
  if (avoid.indexOf(',') >= 0) {
    avoid = "";
  }
  String picked;
  bool ok = pickFromPot(pool, poolN, avoid, picked);
  delete[] pool;
  if (!ok) {
    return false;
  }
  paths[0] = picked;
  outCount = 1;
  return true;
}

static void doSwitch(bool allowMemory) {
  String files[3];
  int count = 0;
  if (!pickNextFiles(files, count, allowMemory) || count < 1) {
    Serial.println(F("Slideshow: no pictures"));
    return;
  }

  const char *paths[3];
  String full[3];
  for (int i = 0; i < count; i++) {
    full[i] = String(PIC_DIR) + "/" + files[i];
    paths[i] = full[i].c_str();
    Serial.printf("Slideshow[%d] → %s\n", i, paths[i]);
  }

  bool ok = false;
  if (count == 1) {
    ok = bmpShowFromSd(paths[0]);
  } else {
    ok = bmpShowCompositeFromSd(paths, count);
  }
  if (ok) {
    String joined = files[0];
    for (int i = 1; i < count; i++) {
      joined += ",";
      joined += files[i];
    }
    slideshowNoteShown(joined);
  }
  noteSwitchTime();
}

void slideshowNoteShown(const String &files) {
  g_lastFile = files;
  framePrefs.begin("frame", false);
  framePrefs.putString("last", g_lastFile);
  framePrefs.end();
}

void slideshowForgetShown(const String &file) {
  if (g_lastFile.length() == 0 || file.length() == 0) {
    return;
  }
  if ((String(",") + g_lastFile + ",").indexOf(String(",") + file + ",") < 0) {
    return;
  }
  slideshowNoteShown("");
}

String slideshowLastShown() { return g_lastFile; }

bool slideshowReshowLast() {
  if (g_lastFile.length() < 1) {
    return false;
  }
  String files[3];
  int count = 0;
  int start = 0;
  while (count < 3) {
    int comma = g_lastFile.indexOf(',', start);
    String one = comma < 0 ? g_lastFile.substring(start) : g_lastFile.substring(start, comma);
    one.trim();
    if (one.length() > 0) {
      files[count++] = one;
    }
    if (comma < 0) {
      break;
    }
    start = comma + 1;
  }
  if (count < 1) {
    return false;
  }
  const char *paths[3];
  String full[3];
  for (int i = 0; i < count; i++) {
    full[i] = String(PIC_DIR) + "/" + files[i];
    paths[i] = full[i].c_str();
  }
  if (count == 1) {
    return bmpShowFromSd(paths[0]);
  }
  return bmpShowCompositeFromSd(paths, count);
}

void slideshowBegin() {
  framePrefs.begin("frame", true);
  g_mode = framePrefs.getInt("mode", 0);
  g_intervalMin = framePrefs.getInt("imin", 5);
  g_dailyHour = framePrefs.getInt("dh", 8);
  g_dailyMin = framePrefs.getInt("dm", 0);
  g_lastFile = framePrefs.getString("last", "");
  g_lastDailyYday = framePrefs.getInt("yday", -1);
  g_lastSwitchEpoch = (time_t)framePrefs.getULong("lsepoch", 0);
  g_hang = framePrefs.getInt("hang", 0);
  if (g_hang != 0 && g_hang != 1) {
    g_hang = 0;
  }
  framePrefs.end();

  if (!intervalOk(g_intervalMin)) {
    g_intervalMin = 5;
  }
  g_lastSwitchMs = millis();
  if (g_lastSwitchEpoch > 1700000000) {
    time_t now = time(nullptr);
    if (now > 1700000000 && now >= g_lastSwitchEpoch) {
      uint64_t elapsedMs = (uint64_t)(now - g_lastSwitchEpoch) * 1000ULL;
      uint64_t needMs = (uint64_t)g_intervalMin * 60ULL * 1000ULL;
      if (elapsedMs < needMs) {
        g_lastSwitchMs = millis() - (uint32_t)elapsedMs;
      } else {
        g_lastSwitchMs = millis() - (uint32_t)needMs;
      }
    }
  }
  randomSeed((uint32_t)esp_random());
  Serial.printf("Slideshow mode=%d interval=%d daily=%02d:%02d\n", g_mode, g_intervalMin, g_dailyHour,
                g_dailyMin);
}

void slideshowSetTimeOk(bool ok) {
  g_timeOk = ok;
}

void slideshowGetJson(String &out) {
  time_t now = time(nullptr);
  struct tm ti;
  bool have = g_timeOk && now > 1700000000 && localtime_r(&now, &ti);
  out = "{";
  out += "\"mode\":";
  out += String(g_mode);
  out += ",\"intervalMin\":";
  out += String(g_intervalMin);
  out += ",\"dailyHour\":";
  out += String(g_dailyHour);
  out += ",\"dailyMin\":";
  out += String(g_dailyMin);
  out += ",\"timeOk\":";
  out += g_timeOk ? "true" : "false";
  out += ",\"rtc\":";
  out += rtcReady() ? "true" : "false";
  out += ",\"last\":\"";
  out += g_lastFile;
  out += "\"";
  if (have) {
    char buf[24];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02dT%02d:%02d", ti.tm_year + 1900, ti.tm_mon + 1, ti.tm_mday,
             ti.tm_hour, ti.tm_min);
    out += ",\"now\":\"";
    out += buf;
    out += "\"";
  } else {
    out += ",\"now\":null";
  }
  out += "}";
}

bool slideshowSet(int mode, int intervalMin, int dailyHour, int dailyMin) {
  if (mode < 0 || mode > 2) {
    return false;
  }
  if (!intervalOk(intervalMin)) {
    intervalMin = 5;
  }
  if (dailyHour < 0 || dailyHour > 23) {
    dailyHour = 8;
  }
  if (dailyMin < 0 || dailyMin > 59) {
    dailyMin = 0;
  }
  g_mode = mode;
  g_intervalMin = intervalMin;
  g_dailyHour = dailyHour;
  g_dailyMin = dailyMin;
  framePrefs.begin("frame", false);
  framePrefs.putInt("mode", g_mode);
  framePrefs.putInt("imin", g_intervalMin);
  framePrefs.putInt("dh", g_dailyHour);
  framePrefs.putInt("dm", g_dailyMin);
  framePrefs.end();
  noteSwitchTime();
  return true;
}

bool slideshowSleepAllowed() {
  if (g_mode == 2) {
    return true;
  }
  if (g_mode == 1 && g_intervalMin >= 10) {
    return true;
  }
  return false;
}

int64_t slideshowSecondsUntilNext() {
  if (g_mode == 1) {
    const int64_t need = (int64_t)g_intervalMin * 60;
    time_t now = time(nullptr);
    if (g_lastSwitchEpoch > 1700000000 && now > 1700000000) {
      int64_t elapsed = (int64_t)now - (int64_t)g_lastSwitchEpoch;
      int64_t left = need - elapsed;
      return left > 0 ? left : 0;
    }
    uint32_t elapsedMs = millis() - g_lastSwitchMs;
    int64_t left = need - (int64_t)(elapsedMs / 1000UL);
    return left > 0 ? left : 0;
  }

  if (g_mode == 2) {
    if (!g_timeOk) {
      return -1;
    }
    time_t now = time(nullptr);
    if (now < 1700000000) {
      return -1;
    }
    struct tm ti;
    if (!localtime_r(&now, &ti)) {
      return -1;
    }
    struct tm target = ti;
    target.tm_hour = g_dailyHour;
    target.tm_min = g_dailyMin;
    target.tm_sec = 0;
    time_t tNext = mktime(&target);
    if (g_lastDailyYday == ti.tm_yday) {
      target.tm_mday += 1;
      tNext = mktime(&target);
    } else if (tNext <= now) {
      return 0;
    }
    int64_t left = (int64_t)tNext - (int64_t)now;
    return left > 0 ? left : 0;
  }

  return -1;
}

const char *hangValue() {
  return g_hang ? "landscape" : "portrait";
}

bool hangSet(const char *v) {
  int n = -1;
  if (v && (!strcmp(v, "landscape") || !strcmp(v, "quer"))) {
    n = 1;
  } else if (v && (!strcmp(v, "portrait") || !strcmp(v, "hoch"))) {
    n = 0;
  }
  if (n < 0) {
    return false;
  }
  g_hang = n;
  framePrefs.begin("frame", false);
  framePrefs.putInt("hang", g_hang);
  framePrefs.end();
  return true;
}

void slideshowForceNow() {
  doSwitch(false);
  if (g_mode == 2) {
    time_t now = time(nullptr);
    struct tm ti;
    if (g_timeOk && now > 1700000000 && localtime_r(&now, &ti)) {
      g_lastDailyYday = ti.tm_yday;
      framePrefs.begin("frame", false);
      framePrefs.putInt("yday", g_lastDailyYday);
      framePrefs.end();
    }
  }
}

void slideshowOnTimer() {
  doSwitch(true);
  if (g_mode == 2) {
    time_t now = time(nullptr);
    struct tm ti;
    if (g_timeOk && now > 1700000000 && localtime_r(&now, &ti)) {
      g_lastDailyYday = ti.tm_yday;
      framePrefs.begin("frame", false);
      framePrefs.putInt("yday", g_lastDailyYday);
      framePrefs.end();
    }
  }
}

void slideshowLoop() {
  if (g_mode == 0) {
    return;
  }
  if (!sdOk()) {
    return;
  }

  if (g_mode == 1) {
    uint32_t need = (uint32_t)g_intervalMin * 60UL * 1000UL;
    if (millis() - g_lastSwitchMs >= need) {
      powerNoteBusy(true);
      doSwitch(true);
      powerNoteBusy(false);
    }
    return;
  }

  if (!g_timeOk) {
    return;
  }
  time_t now = time(nullptr);
  if (now < 1700000000) {
    return;
  }
  struct tm ti;
  if (!localtime_r(&now, &ti)) {
    return;
  }
  if (ti.tm_hour == g_dailyHour && ti.tm_min == g_dailyMin) {
    if (g_lastDailyYday != ti.tm_yday) {
      powerNoteBusy(true);
      doSwitch(true);
      powerNoteBusy(false);
      g_lastDailyYday = ti.tm_yday;
      framePrefs.begin("frame", false);
      framePrefs.putInt("yday", g_lastDailyYday);
      framePrefs.end();
    }
  }
}
