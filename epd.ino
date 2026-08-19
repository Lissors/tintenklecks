// Spectra 6 EPD driver (800×480, 4bpp)

#include "config.h"
#include "board.h"
#include <SPI.h>
#include <string.h>
#include <pgmspace.h>

static SPIClass epdSpi(FSPI);
static uint8_t *epdBuffer = nullptr;

static bool epdWaitBusy(uint32_t timeout_ms = 60000) {
  uint32_t start = millis();
  while (digitalRead(PIN_EPD_BUSY) == LOW) {
    if (millis() - start > timeout_ms) {
      Serial.println(F("EPD BUSY timeout"));
      return false;
    }
    delay(5);
    yield();
  }
  return true;
}

static bool epdSendCommand(uint8_t cmd) {
  if (!epdWaitBusy()) {
    return false;
  }
  digitalWrite(PIN_EPD_DC, LOW);
  digitalWrite(PIN_EPD_CS, LOW);
  epdSpi.transfer(cmd);
  digitalWrite(PIN_EPD_CS, HIGH);
  return true;
}

static void epdSendData(uint8_t data) {
  digitalWrite(PIN_EPD_DC, HIGH);
  digitalWrite(PIN_EPD_CS, LOW);
  epdSpi.transfer(data);
  digitalWrite(PIN_EPD_CS, HIGH);
}

static void epdBeginData() {
  digitalWrite(PIN_EPD_DC, HIGH);
  digitalWrite(PIN_EPD_CS, LOW);
}

static void epdEndData() {
  digitalWrite(PIN_EPD_CS, HIGH);
}

static void epdHardwareReset() {
  digitalWrite(PIN_EPD_RST, LOW);
  delay(20);
  digitalWrite(PIN_EPD_RST, HIGH);
  delay(50);
  epdWaitBusy();
}

static void epdPortInit() {
  epdHardwareReset();
  delay(50);

  epdSendCommand(0xAA);
  epdSendData(0x49);
  epdSendData(0x55);
  epdSendData(0x20);
  epdSendData(0x08);
  epdSendData(0x09);
  epdSendData(0x18);

  epdSendCommand(0x01);
  epdSendData(0x3F);

  epdSendCommand(0x00);
  epdSendData(0x5F);
  epdSendData(0x69);

  epdSendCommand(0x03);
  epdSendData(0x00);
  epdSendData(0x54);
  epdSendData(0x00);
  epdSendData(0x44);

  epdSendCommand(0x05);
  epdSendData(0x40);
  epdSendData(0x1F);
  epdSendData(0x1F);
  epdSendData(0x2C);

  epdSendCommand(0x06);
  epdSendData(0x6F);
  epdSendData(0x1F);
  epdSendData(0x17);
  epdSendData(0x49);

  epdSendCommand(0x08);
  epdSendData(0x6F);
  epdSendData(0x1F);
  epdSendData(0x1F);
  epdSendData(0x22);

  epdSendCommand(0x30);
  epdSendData(0x03);

  epdSendCommand(0x50);
  epdSendData(0x3F);

  epdSendCommand(0x60);
  epdSendData(0x02);
  epdSendData(0x00);

  epdSendCommand(0x61);
  epdSendData(0x03);
  epdSendData(0x20);
  epdSendData(0x01);
  epdSendData(0xE0);

  epdSendCommand(0x84);
  epdSendData(0x01);

  epdSendCommand(0xE3);
  epdSendData(0x2F);

  epdSendCommand(0x04);
  epdWaitBusy();
}

static void epdDeepSleep() {
  epdSendCommand(0x07);
  epdSendData(0xA5);
}

void epdPanelSleep() {
  epdDeepSleep();
}

static void epdTurnOnDisplay() {
  epdSendCommand(0x04);
  epdWaitBusy();

  epdSendCommand(0x06);
  epdSendData(0x6F);
  epdSendData(0x1F);
  epdSendData(0x17);
  epdSendData(0x49);

  epdSendCommand(0x12);
  epdSendData(0x00);
  epdWaitBusy();

  epdSendCommand(0x02);
  epdSendData(0x00);
  epdWaitBusy();
}

void epdClear(uint8_t color) {
  if (!epdBuffer) {
    return;
  }
  uint8_t packed = (uint8_t)((color << 4) | color);
  memset(epdBuffer, packed, EPD_BUF_SIZE);
}

void epdSetPixel(int x, int y, uint8_t color) {
  if (!epdBuffer) {
    return;
  }
  if (x < 0 || x >= EPD_WIDTH || y < 0 || y >= EPD_HEIGHT) {
    return;
  }
  uint32_t idx = (uint32_t)y * EPD_ROW_BYTES + (uint32_t)(x / 2);
  if (x & 1) {
    epdBuffer[idx] = (uint8_t)((epdBuffer[idx] & 0xF0) | (color & 0x0F));
  } else {
    epdBuffer[idx] = (uint8_t)((epdBuffer[idx] & 0x0F) | ((color & 0x0F) << 4));
  }
}

static void epdWriteBuffer() {
  if (!epdBuffer) {
    return;
  }
  epdSendCommand(0x10);
  epdBeginData();

  // Bounce through DRAM (SPI/DMA must not read PSRAM directly).
  // No 180° — BMP orientation matches the panel mount.
  uint8_t rowTmp[EPD_ROW_BYTES];
  for (int y = 0; y < EPD_HEIGHT; y++) {
    memcpy(rowTmp, &epdBuffer[(uint32_t)y * EPD_ROW_BYTES], EPD_ROW_BYTES);
    epdSpi.transfer(rowTmp, EPD_ROW_BYTES);
    yield();
  }
  epdEndData();
}

bool epdInit() {
  pinMode(PIN_EPD_DC, OUTPUT);
  pinMode(PIN_EPD_CS, OUTPUT);
  pinMode(PIN_EPD_RST, OUTPUT);
  pinMode(PIN_EPD_BUSY, INPUT);
  digitalWrite(PIN_EPD_CS, HIGH);

  epdSpi.begin(PIN_EPD_SCK, -1, PIN_EPD_MOSI, PIN_EPD_CS);
  epdSpi.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  if (!epdBuffer) {
    epdBuffer = (uint8_t *)ps_malloc(EPD_BUF_SIZE);
  }
  if (!epdBuffer) {
    Serial.println(F("ERR EPD framebuffer"));
    return false;
  }
  epdClear(EPD_WHITE);
  Serial.println(F("EPD framebuffer OK"));
  return true;
}

// 5×7, bit0 = top. Glyphs for "Akku < 10 %"
static const uint8_t FONT5X7[][5] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00},  // space
    {0x23, 0x13, 0x08, 0x64, 0x62},  // %
    {0x08, 0x08, 0x08, 0x08, 0x08},  // -
    {0x08, 0x14, 0x22, 0x41, 0x00},  // <
    {0x3E, 0x51, 0x49, 0x45, 0x3E},  // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},  // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},  // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},  // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},  // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},  // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},  // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},  // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},  // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},  // 9
    {0x7E, 0x11, 0x11, 0x11, 0x7E},  // A
    {0x00, 0x00, 0x00, 0x00, 0x00},
    {0x7F, 0x49, 0x49, 0x49, 0x36},  // unused
};

static int epdFontIndex(char c) {
  if (c == ' ') {
    return 0;
  }
  if (c == '%') {
    return 1;
  }
  if (c == '-') {
    return 2;
  }
  if (c == '<') {
    return 3;
  }
  if (c >= '0' && c <= '9') {
    return 4 + (c - '0');
  }
  if (c == 'A') {
    return 14;
  }
  if (c == 'k') {
    return 15;
  }
  if (c == 'u') {
    return 16;
  }
  return 0;
}

// k and u as 5×7
static const uint8_t FONT_K[5] PROGMEM = {0x7F, 0x08, 0x14, 0x22, 0x41};
static const uint8_t FONT_U[5] PROGMEM = {0x3F, 0x40, 0x40, 0x40, 0x3F};

static void epdSetPixelVis(int vx, int vy, uint8_t color) {
  // Portrait as hung: 480×800. Same map as bmp 90° CW (dx=sy, dy=479-sx).
  epdSetPixel(vy, EPD_HEIGHT - 1 - vx, color);
}

static void epdDot(int vx, int vy, uint8_t color, int scale) {
  for (int dy = 0; dy < scale; dy++) {
    for (int dx = 0; dx < scale; dx++) {
      epdSetPixelVis(vx + dx, vy + dy, color);
    }
  }
}

static void epdDrawGlyph(int vx, int vy, const uint8_t *col, uint8_t color, int scale) {
  for (int cx = 0; cx < 5; cx++) {
    uint8_t bits = col[cx];
    for (int cy = 0; cy < 7; cy++) {
      if (bits & (1 << cy)) {
        epdDot(vx + cx * scale, vy + cy * scale, color, scale);
      }
    }
  }
}

static void epdDrawChar(int vx, int vy, char c, uint8_t color, int scale) {
  uint8_t col[5];
  if (c == 'k') {
    memcpy_P(col, FONT_K, 5);
  } else if (c == 'u') {
    memcpy_P(col, FONT_U, 5);
  } else {
    int i = epdFontIndex(c);
    memcpy_P(col, FONT5X7[i], 5);
  }
  epdDrawGlyph(vx, vy, col, color, scale);
  epdDrawGlyph(vx + 1, vy, col, color, scale);  // fett
}

static void epdDrawBatteryWarn() {
  const char *msg = "Akku < 10 %";
  const int scale = 3;
  const int gw = 6 * scale;
  const int gh = 7 * scale;
  const int visW = EPD_HEIGHT;  // 480
  const int visH = EPD_WIDTH;   // 800
  int n = 0;
  for (const char *p = msg; *p; p++) {
    n++;
  }
  int tw = n * gw + 1;
  int th = gh + 2;
  int x0 = visW - tw - 18;
  int y0 = visH - th - 14;
  if (x0 < 8) {
    x0 = 8;
  }
  if (y0 < 8) {
    y0 = 8;
  }
  for (int y = y0 - 4; y < y0 + th + 4; y++) {
    for (int x = x0 - 4; x < x0 + tw + 4; x++) {
      epdSetPixelVis(x, y, EPD_WHITE);
    }
  }
  int x = x0;
  for (const char *p = msg; *p; p++) {
    epdDrawChar(x, y0, *p, EPD_BLACK, scale);
    x += gw;
  }
}

static bool g_forceBattWarn = false;

void epdForceBatteryWarn(bool on) { g_forceBattWarn = on; }

void epdDisplayCurrentBuffer() {
  if (!epdBuffer) {
    return;
  }
  bool warn = g_forceBattWarn;
  if (!warn && !pmuUsbPowered() && !pmuCharging()) {
    int pct = pmuBatteryPercent();
    if (pct >= 0 && pct < 10) {
      warn = true;
    }
  }
  if (warn) {
    epdDrawBatteryWarn();
  }
  Serial.println(F("EPD refresh…"));
  epdPortInit();
  epdWriteBuffer();
  epdTurnOnDisplay();
  epdDeepSleep();
  Serial.println(F("EPD done"));
}
