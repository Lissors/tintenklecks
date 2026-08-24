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
static int g_lastDailyYmd = 0;  // YYYYMMDD, 0 = noch kein Tageswechsel
static String g_lastFile;
static bool g_timeOk = false;
static int g_memMore = 0;  // weitere fällige Erinnerungen (0 = keine Runde)
static String g_memSeen;   // per KEY schon gezeigte fällige Dateien, kommagetrennt
static int g_memSeenDay = 0;
static const int64_t MEM_CYCLE_SEC = 3 * 3600;
static const int SLIDE_MAX = 256;

static String g_memNextJson;
static uint32_t g_memNextMs = 0;
static bool g_memNextValid = false;
static int g_potLeft = -1;
static int g_potTotal = -1;

void slideshowInvalidatePot() {
  g_potLeft = -1;
}

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

static void persistMemMore() {
  framePrefs.begin("frame", false);
  framePrefs.putInt("memmore", g_memMore);
  framePrefs.putString("memseen", g_memSeen);
  framePrefs.putInt("memday", g_memSeenDay);
  framePrefs.end();
}

static int todayYmd() {
  time_t now = time(nullptr);
  struct tm ti;
  if (now < 1700000000 || !localtime_r(&now, &ti)) {
    return 0;
  }
  return (ti.tm_year + 1900) * 10000 + (ti.tm_mon + 1) * 100 + ti.tm_mday;
}

static bool memSeenHas(const String &file) {
  if (!file.length() || !g_memSeen.length()) {
    return false;
  }
  return (String(",") + g_memSeen + ",").indexOf(String(",") + file + ",") >= 0;
}

static void memSeenAdd(const String &file) {
  if (!file.length() || memSeenHas(file)) {
    return;
  }
  if (g_memSeen.length()) {
    g_memSeen += ",";
  }
  g_memSeen += file;
}

static void memSeenRollDay() {
  int d = todayYmd();
  if (!d) {
    return;
  }
  if (d != g_memSeenDay) {
    g_memSeen = "";
    g_memSeenDay = d;
  }
}

void slideshowForgetMemoryCycle() {
  g_memMore = 0;
  persistMemMore();
  epdSetMoreMemoriesHint(0);
}

static String lastShownOne() {
  String last = g_lastFile;
  int comma = last.indexOf(',');
  if (comma >= 0) {
    last = last.substring(0, comma);
  }
  last.trim();
  return last;
}

static bool pickDueMemory(PicCand *cands, int n, String *paths, int &outCount, bool fromKey) {
  String due[SLIDE_MAX];
  int dueN = 0;
  for (int i = 0; i < n; i++) {
    if (cands[i].anniversarySoon) {
      due[dueN++] = cands[i].file;
    }
  }
  if (dueN < 1) {
    return false;
  }
  memSeenRollDay();

  if (fromKey) {
    String last = lastShownOne();
    int lastDue = -1;
    for (int i = 0; i < dueN; i++) {
      if (due[i] == last) {
        lastDue = i;
        break;
      }
    }
    String chosen;
    bool found = false;
    if (lastDue >= 0) {
      for (int i = lastDue + 1; i < dueN; i++) {
        if (!memSeenHas(due[i])) {
          chosen = due[i];
          found = true;
          break;
        }
      }
      if (!found) {
        for (int i = 0; i < lastDue; i++) {
          if (!memSeenHas(due[i])) {
            chosen = due[i];
            found = true;
            break;
          }
        }
      }
    } else {
      int restN = 0;
      for (int i = 0; i < dueN; i++) {
        if (!memSeenHas(due[i])) {
          restN++;
        }
      }
      if (restN < 1) {
        return false;
      }
      int skip = random(restN);
      for (int i = 0; i < dueN; i++) {
        if (memSeenHas(due[i])) {
          continue;
        }
        if (skip == 0) {
          chosen = due[i];
          found = true;
          break;
        }
        skip--;
      }
    }
    if (!found) {
      return false;
    }
    paths[0] = chosen;
    outCount = 1;
    memSeenAdd(chosen);
    int left = 0;
    for (int i = 0; i < dueN; i++) {
      if (!memSeenHas(due[i])) {
        left++;
      }
    }
    g_memMore = left;
    persistMemMore();
    return true;
  }

  int start = 0;
  if (dueN >= 2) {
    String last = lastShownOne();
    int at = -1;
    for (int i = 0; i < dueN; i++) {
      if (due[i] == last) {
        at = i;
        break;
      }
    }
    if (at >= 0) {
      start = (at + 1) % dueN;
    } else {
      start = random(dueN);
    }
  }
  paths[0] = due[start];
  outCount = 1;
  memSeenAdd(due[start]);
  g_memMore = dueN >= 2 ? (dueN - 1) : 0;
  persistMemMore();
  return true;
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
  return (bm == tm && bd == td) || (bm == m1 && bd == d1) || (bm == m2 && bd == d2);
}

static int anniversaryOffset(const String &date, int ty, int tm, int td) {
  int bd, bm;
  if (!parseDayMonth(date, bd, bm)) {
    return -1;
  }
  if (bm == tm && bd == td) {
    return 0;
  }
  int y1 = ty, m1 = tm, d1 = td;
  addDays(y1, m1, d1, 1);
  if (bm == m1 && bd == d1) {
    return 1;
  }
  int y2 = ty, m2 = tm, d2 = td;
  addDays(y2, m2, d2, 2);
  if (bm == m2 && bd == d2) {
    return 2;
  }
  return -1;
}

static void jsonAppendEscaped(String &out, const String &s) {
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"' || c == '\\') {
      out += '\\';
    }
    if ((uint8_t)c < 0x20) {
      out += ' ';
    } else {
      out += c;
    }
  }
}

void slideshowInvalidateMemPreview() {
  g_memNextValid = false;
}

static String readSdWhole(const char *path) {
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

static int jsonObjEnd(const String &s, int open) {
  if (open < 0 || open >= (int)s.length() || s.charAt(open) != '{') {
    return -1;
  }
  int depth = 0;
  bool inStr = false;
  bool esc = false;
  for (int i = open; i < (int)s.length(); i++) {
    char c = s.charAt(i);
    if (inStr) {
      if (esc) {
        esc = false;
        continue;
      }
      if (c == '\\') {
        esc = true;
        continue;
      }
      if (c == '"') {
        inStr = false;
      }
      continue;
    }
    if (c == '"') {
      inStr = true;
    } else if (c == '{') {
      depth++;
    } else if (c == '}') {
      depth--;
      if (depth == 0) {
        return i;
      }
    }
  }
  return -1;
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
  return readSdWhole(path.c_str());
}

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
  slideshowInvalidatePot();
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

static int loadNormalPool(String *pool, int maxN) {
  int poolN = 0;
  if (!pool || maxN < 1 || !sdOk()) {
    return 0;
  }
  String g = readSdWhole(LIST_CACHE_PATH);
  if (g.length() < 2 || g.charAt(0) != '[') {
    return 0;
  }
  int pos = 0;
  while (poolN < maxN) {
    int open = g.indexOf('{', pos);
    if (open < 0) {
      break;
    }
    int close = jsonObjEnd(g, open);
    if (close < 0) {
      break;
    }
    pos = close + 1;
    String rec = g.substring(open, close + 1);
    if (jsonField(rec, "kind") != "normal") {
      continue;
    }
    String file = jsonField(rec, "file");
    if (file.length()) {
      pool[poolN++] = file;
    }
  }
  return poolN;
}

void slideshowDeckRefill() {
  if (!sdOk()) {
    return;
  }
  String *pool = new String[SLIDE_MAX];
  if (!pool) {
    return;
  }
  int n = loadNormalPool(pool, SLIDE_MAX);
  shuffleStrings(pool, n);
  String avoid = lastShownOne();
  if (n > 1 && avoid.length() > 0 && pool[n - 1] == avoid) {
    int j = random(n - 1);
    String tmp = pool[n - 1];
    pool[n - 1] = pool[j];
    pool[j] = tmp;
  }
  deckSave(pool, n);
  g_potLeft = n;
  g_potTotal = n;
  delete[] pool;
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

static bool memDatesForFile(const String &mems, const String &file, String &birth, String &death,
                            String &special) {
  birth = "";
  death = "";
  special = "";
  if (mems.length() < 2 || file.length() == 0) {
    return false;
  }
  String needle = String("\"file\":\"") + file + "\"";
  int at = mems.indexOf(needle);
  if (at < 0) {
    return false;
  }
  int open = at;
  while (open > 0 && mems.charAt(open) != '{') {
    open--;
  }
  int close = jsonObjEnd(mems, open);
  if (close < 0) {
    return false;
  }
  String rec = mems.substring(open, close + 1);
  birth = jsonField(rec, "birth");
  death = jsonField(rec, "death");
  special = jsonField(rec, "special");
  return true;
}

static bool collectCandidates(PicCand *out, int maxN, int &count) {
  count = 0;
  if (!sdOk() || maxN < 1) {
    return false;
  }
  String gallery = readSdWhole(LIST_CACHE_PATH);
  if (gallery.length() < 2 || gallery.charAt(0) != '[') {
    return false;
  }
  String mems = readSdWhole(MEMORIES_PATH);

  time_t now = time(nullptr);
  struct tm ti;
  bool haveTime = g_timeOk && now > 1700000000 && localtime_r(&now, &ti);
  int ty = haveTime ? (ti.tm_year + 1900) : 0;
  int tm = haveTime ? (ti.tm_mon + 1) : 0;
  int td = haveTime ? ti.tm_mday : 0;

  int pos = 0;
  uint16_t tick = 0;
  while (count < maxN) {
    int open = gallery.indexOf('{', pos);
    if (open < 0) {
      break;
    }
    int close = jsonObjEnd(gallery, open);
    if (close < 0) {
      break;
    }
    pos = close + 1;
    String rec = gallery.substring(open, close + 1);
    String file = jsonField(rec, "file");
    if (!file.length()) {
      continue;
    }
    PicCand &c = out[count];
    c.file = file;
    if (jsonField(rec, "kind") == "memory") {
      c.noDates = false;
      String birth, death, special;
      memDatesForFile(mems, file, birth, death, special);
      c.anniversarySoon = haveTime && (matchAnniversarySoon(birth, ty, tm, td) ||
                                       matchAnniversarySoon(death, ty, tm, td) ||
                                       matchAnniversarySoon(special, ty, tm, td));
    } else {
      c.noDates = true;
      c.anniversarySoon = false;
    }
    count++;
    if ((++tick & 15) == 0) {
      yield();
    }
  }
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

  // Timer: fällige Erinnerung (eine). KEY: nur wenn mindestens zwei fällig, die nächste.
  int dueN = 0;
  for (int i = 0; i < n; i++) {
    if (cands[i].anniversarySoon) {
      dueN++;
    }
  }
  bool takeMemory = (allowMemory && dueN >= 1) || (!allowMemory && dueN >= 2);
  if (takeMemory && pickDueMemory(cands, n, paths, outCount, !allowMemory)) {
    delete[] cands;
    return true;
  }

  g_memMore = 0;
  persistMemMore();

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

  epdSetMoreMemoriesHint(g_memMore);
  const char *paths[3];
  String full[3];
  full[0] = String(PIC_DIR) + "/" + files[0];
  paths[0] = full[0].c_str();
  Serial.printf("Slideshow → %s (weitere=%d)\n", paths[0], g_memMore);

  bool ok = bmpShowFromSd(paths[0]);
  if (ok) {
    slideshowNoteShown(files[0]);
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
  epdSetMoreMemoriesHint(g_memMore);
  String full = String(PIC_DIR) + "/" + files[0];
  return bmpShowFromSd(full.c_str());
}

void slideshowBegin() {
  framePrefs.begin("frame", true);
  g_mode = framePrefs.getInt("mode", 0);
  g_intervalMin = framePrefs.getInt("imin", 5);
  g_dailyHour = framePrefs.getInt("dh", 8);
  g_dailyMin = framePrefs.getInt("dm", 0);
  g_lastFile = framePrefs.getString("last", "");
  g_lastDailyYmd = framePrefs.getInt("dymd", 0);
  g_lastSwitchEpoch = (time_t)framePrefs.getULong("lsepoch", 0);
  g_hang = framePrefs.getInt("hang", 0);
  if (g_hang != 0 && g_hang != 1) {
    g_hang = 0;
  }
  g_memMore = framePrefs.getInt("memmore", 0);
  if (g_memMore < 0) {
    g_memMore = 0;
  }
  g_memSeen = framePrefs.getString("memseen", "");
  g_memSeenDay = framePrefs.getInt("memday", 0);
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

bool slideshowTimeOk() {
  return g_timeOk;
}

void slideshowAppendMemoryJson(String &out) {
  if (g_memNextValid && (millis() - g_memNextMs) < 60000UL && g_memNextJson.length() > 0) {
    out += ",\"memoryNext\":";
    out += g_memNextJson;
    return;
  }

  g_memNextJson = "null";
  time_t now = time(nullptr);
  struct tm ti;
  bool haveTime = g_timeOk && now > 1700000000 && localtime_r(&now, &ti);
  if (haveTime && sdOk()) {
    String arr = readSdWhole(MEMORIES_PATH);
    if (arr.length() >= 2 && arr.charAt(0) == '[') {
      int ty = ti.tm_year + 1900;
      int tmo = ti.tm_mon + 1;
      int td = ti.tm_mday;
      int bestOff = 99;
      int count = 0;
      String bestFile;
      String bestName;
      String bestWhen;
      int pos = 0;
      uint16_t tick = 0;
      while (true) {
        int open = arr.indexOf('{', pos);
        if (open < 0) {
          break;
        }
        int close = jsonObjEnd(arr, open);
        if (close < 0) {
          break;
        }
        pos = close + 1;
        String rec = arr.substring(open, close + 1);
        String birth = jsonField(rec, "birth");
        String death = jsonField(rec, "death");
        String special = jsonField(rec, "special");
        int off = 99;
        String when;
        const String *fields[3] = {&birth, &death, &special};
        for (int i = 0; i < 3; i++) {
          int bd, bm;
          int o = anniversaryOffset(*fields[i], ty, tmo, td);
          if (o < 0 || o > off) {
            continue;
          }
          if (!parseDayMonth(*fields[i], bd, bm)) {
            continue;
          }
          char w[8];
          snprintf(w, sizeof(w), "%02d.%02d.", bd, bm);
          off = o;
          when = w;
        }
        if (off > 2) {
          continue;
        }
        count++;
        if (off < bestOff) {
          bestOff = off;
          bestFile = jsonField(rec, "file");
          bestWhen = when;
          String name = jsonField(rec, "name");
          name.trim();
          if (name.length() == 0) {
            name = bestFile;
            if (name.endsWith(".bmp") || name.endsWith(".BMP")) {
              name = name.substring(0, name.length() - 4);
            }
          }
          bestName = name;
        }
        if ((++tick & 15) == 0) {
          yield();
        }
      }
      if (count > 0 && bestFile.length() > 0) {
        g_memNextJson = "{\"count\":";
        g_memNextJson += String(count);
        g_memNextJson += ",\"when\":\"";
        jsonAppendEscaped(g_memNextJson, bestWhen);
        g_memNextJson += "\",\"name\":\"";
        jsonAppendEscaped(g_memNextJson, bestName);
        g_memNextJson += "\",\"file\":\"";
        jsonAppendEscaped(g_memNextJson, bestFile);
        g_memNextJson += "\"}";
      }
    }
  }

  g_memNextValid = true;
  g_memNextMs = millis();
  out += ",\"memoryNext\":";
  out += g_memNextJson;
}

void slideshowAppendPotJson(String &j) {
  if (g_potLeft < 0) {
    g_potLeft = 0;
    g_potTotal = 0;
    String *pool = new String[SLIDE_MAX];
    if (pool) {
      int poolN = loadNormalPool(pool, SLIDE_MAX);
      g_potTotal = poolN;
      String *deck = new String[SLIDE_MAX];
      int left = 0;
      if (deck) {
        int dn = deckLoad(deck, SLIDE_MAX);
        for (int i = 0; i < dn; i++) {
          if (nameIn(pool, poolN, deck[i])) {
            left++;
          }
        }
        delete[] deck;
      }
      g_potLeft = left;
      delete[] pool;
    }
  }
  j += ",\"potLeft\":";
  j += String(g_potLeft);
  j += ",\"potTotal\":";
  j += String(g_potTotal);
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
  ntpAppendJson(out);
  slideshowAppendMemoryJson(out);
  slideshowAppendPotJson(out);
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

static int dateYmd(const struct tm &ti) {
  return (ti.tm_year + 1900) * 10000 + (ti.tm_mon + 1) * 100 + ti.tm_mday;
}

static void markDailyDone() {
  time_t now = time(nullptr);
  struct tm ti;
  if (!g_timeOk || now < 1700000000 || !localtime_r(&now, &ti)) {
    return;
  }
  g_lastDailyYmd = dateYmd(ti);
  framePrefs.begin("frame", false);
  framePrefs.putInt("dymd", g_lastDailyYmd);
  framePrefs.end();
}

static int64_t secondsUntilDaily() {
  if (g_mode != 2 || !g_timeOk) {
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
  if (dateYmd(ti) == g_lastDailyYmd) {
    struct tm target = ti;
    target.tm_mday += 1;
    target.tm_hour = g_dailyHour;
    target.tm_min = g_dailyMin;
    target.tm_sec = 0;
    time_t tNext = mktime(&target);
    int64_t left = (int64_t)tNext - (int64_t)now;
    return left > 0 ? left : 0;
  }
  struct tm target = ti;
  target.tm_hour = g_dailyHour;
  target.tm_min = g_dailyMin;
  target.tm_sec = 0;
  time_t tAt = mktime(&target);
  if (tAt <= now) {
    return 0;
  }
  return (int64_t)tAt - (int64_t)now;
}

static bool dailyDueNow() { return secondsUntilDaily() == 0; }

int64_t slideshowSecondsUntilNext() {
  time_t now = time(nullptr);
  int64_t memLeft = -1;
  if (g_memMore > 0 && g_timeOk && g_lastSwitchEpoch > 1700000000 && now > 1700000000) {
    memLeft = MEM_CYCLE_SEC - ((int64_t)now - (int64_t)g_lastSwitchEpoch);
    if (memLeft < 0) {
      memLeft = 0;
    }
  }

  int64_t sched = -1;
  if (g_mode == 1) {
    const int64_t need = (int64_t)g_intervalMin * 60;
    if (g_lastSwitchEpoch > 1700000000 && now > 1700000000) {
      int64_t elapsed = (int64_t)now - (int64_t)g_lastSwitchEpoch;
      sched = need - elapsed;
    } else {
      uint32_t elapsedMs = millis() - g_lastSwitchMs;
      sched = need - (int64_t)(elapsedMs / 1000UL);
    }
    if (sched < 0) {
      sched = 0;
    }
  } else if (g_mode == 2) {
    sched = secondsUntilDaily();
  }

  if (memLeft >= 0 && sched >= 0) {
    return memLeft < sched ? memLeft : sched;
  }
  if (memLeft >= 0) {
    return memLeft;
  }
  return sched;
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
}

void slideshowOnTimer() {
  doSwitch(true);
  if (g_mode == 2) {
    markDailyDone();
  }
}

void slideshowLoop() {
  if (!sdOk()) {
    return;
  }

  if (dailyDueNow()) {
    powerNoteBusy(true);
    doSwitch(true);
    powerNoteBusy(false);
    markDailyDone();
    return;
  }

  if (g_mode != 0 && g_memMore > 0 && g_timeOk) {
    time_t now = time(nullptr);
    if (now > 1700000000 && g_lastSwitchEpoch > 1700000000 &&
        ((int64_t)now - (int64_t)g_lastSwitchEpoch) >= MEM_CYCLE_SEC) {
      powerNoteBusy(true);
      doSwitch(true);
      powerNoteBusy(false);
      return;
    }
    if (g_mode == 1) {
      return;
    }
  }

  if (g_mode == 0) {
    return;
  }

  if (g_mode == 1) {
    uint32_t need = (uint32_t)g_intervalMin * 60UL * 1000UL;
    if (millis() - g_lastSwitchMs >= need) {
      powerNoteBusy(true);
      doSwitch(true);
      powerNoteBusy(false);
    }
  }
}
