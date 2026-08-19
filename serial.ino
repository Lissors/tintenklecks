// USB serial protocol (optional companion to web)

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"

static void reply(const char *line) {
  Serial.println(line);
  Serial.flush();
}

static bool validName(const String &name) {
  if (name.length() < 5 || name.length() > 64) {
    return false;
  }
  if (name.indexOf('/') >= 0 || name.indexOf('\\') >= 0 || name.indexOf("..") >= 0) {
    return false;
  }
  for (size_t i = 0; i < name.length(); i++) {
    char c = name[i];
    bool ok = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '.' || c == '_' || c == '-';
    if (!ok) {
      return false;
    }
  }
  String lower = name;
  lower.toLowerCase();
  return lower.endsWith(".bmp");
}

static String readLine(uint32_t timeoutMs) {
  String line;
  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    while (Serial.available()) {
      char c = static_cast<char>(Serial.read());
      if (c == '\r') {
        continue;
      }
      if (c == '\n') {
        return line;
      }
      if (line.length() < 200) {
        line += c;
      }
      start = millis();
    }
    delay(1);
  }
  return line;
}

static bool readExactToRam(uint8_t *dst, size_t size) {
  size_t gotTotal = 0;
  uint32_t last = millis();
  size_t nextAck = ACK_EVERY;
  while (gotTotal < size) {
    size_t avail = Serial.available();
    if (avail == 0) {
      if (millis() - last > RX_TIMEOUT_MS) {
        return false;
      }
      yield();
      continue;
    }
    size_t chunk = size - gotTotal;
    if (chunk > avail) {
      chunk = avail;
    }
    if (chunk > 4096) {
      chunk = 4096;
    }
    size_t n = Serial.readBytes(dst + gotTotal, chunk);
    if (n == 0) {
      if (millis() - last > RX_TIMEOUT_MS) {
        return false;
      }
      continue;
    }
    gotTotal += n;
    last = millis();
    while (gotTotal >= nextAck && nextAck <= size) {
      Serial.print(F("ACK "));
      Serial.println(static_cast<unsigned>(gotTotal));
      Serial.flush();
      nextAck += ACK_EVERY;
    }
  }
  return true;
}

static bool writeFileFromRam(const String &path, const uint8_t *data, size_t size) {
  if (SD_MMC.exists(path)) {
    SD_MMC.remove(path);
  }
  File file = SD_MMC.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  size_t off = 0;
  while (off < size) {
    size_t n = size - off;
    if (n > 4096) {
      n = 4096;
    }
    if (file.write(data + off, n) != n) {
      file.close();
      SD_MMC.remove(path);
      return false;
    }
    off += n;
  }
  file.flush();
  file.close();
  return true;
}

static void cmdPut(const String &args) {
  if (!sdOk()) {
    reply("ERR no SD");
    return;
  }
  int sp = args.indexOf(' ');
  if (sp <= 0) {
    reply("ERR usage");
    return;
  }
  String name = args.substring(0, sp);
  name.trim();
  size_t size = (size_t)args.substring(sp + 1).toInt();
  if (!validName(name) || size == 0 || size > MAX_BMP) {
    reply("ERR bad name/size");
    return;
  }
  uint8_t *ram = (uint8_t *)ps_malloc(size);
  if (!ram) {
    reply("ERR no PSRAM");
    return;
  }
  reply("READY");
  if (!readExactToRam(ram, size)) {
    free(ram);
    reply("ERR timeout rx");
    return;
  }
  reply("WRITING");
  String path = String(PIC_DIR) + "/" + name;
  if (!writeFileFromRam(path, ram, size)) {
    free(ram);
    reply("ERR write SD");
    return;
  }
  char buf[96];
  snprintf(buf, sizeof(buf), "OK %s %u", name.c_str(), (unsigned)size);
  reply(buf);
  reply("SHOWING");
  bool ok = bmpShowFromMemory(ram, size);
  free(ram);
  reply(ok ? "DONE" : "ERR show");
}

static void handleLine(String line) {
  line.trim();
  if (!line.length()) {
    return;
  }
  if (line.equalsIgnoreCase("PING")) {
    reply("PONG Tintenklecks");
    return;
  }
  if (line.equalsIgnoreCase("INFO")) {
    char buf[128];
    snprintf(buf, sizeof(buf), "INFO SD=%d AP=%d", sdOk() ? 1 : 0, wifiIsAp() ? 1 : 0);
    reply(buf);
    return;
  }
  if (line.startsWith("PUT ")) {
    cmdPut(line.substring(4));
    return;
  }
  if (line.startsWith("SHOW ")) {
    String name = line.substring(5);
    name.trim();
    if (!validName(name)) {
      reply("ERR bad name");
      return;
    }
    String path = String(PIC_DIR) + "/" + name;
    reply("SHOWING");
    reply(bmpShowFromSd(path.c_str()) ? "DONE" : "ERR show");
    return;
  }
  reply("ERR unknown");
}

void serialProtocolBegin() {
}

void serialProtocolLoop() {
  if (!Serial.available()) {
    return;
  }
  String line = readLine(50);
  if (line.length()) {
    handleLine(line);
  }
}
