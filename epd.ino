// Spectra 6 EPD driver (800×480, 4bpp)

#include "config.h"
#include "board.h"
#include <SPI.h>
#include <string.h>
#include <driver/gpio.h>

static SPIClass epdSpi(FSPI);
static uint8_t *epdBuffer = nullptr;
static bool g_epdAlive = true;

static bool epdWaitBusy(uint32_t timeout_ms = 60000) {
  if (!g_epdAlive) {
    return false;
  }
  uint32_t start = millis();
  while (digitalRead(PIN_EPD_BUSY) == LOW) {
    if (millis() - start > timeout_ms) {
      Serial.println(F("EPD BUSY timeout"));
      g_epdAlive = false;
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
  if (!g_epdAlive) {
    return;
  }
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

static bool epdHardwareReset() {
  g_epdAlive = true;
  digitalWrite(PIN_EPD_RST, LOW);
  delay(20);
  digitalWrite(PIN_EPD_RST, HIGH);
  delay(80);
  if (epdWaitBusy(3000)) {
    return true;
  }
  g_epdAlive = true;
  digitalWrite(PIN_EPD_RST, LOW);
  delay(20);
  digitalWrite(PIN_EPD_RST, HIGH);
  delay(80);
  return epdWaitBusy(3000);
}

static bool epdPortInit() {
  if (!epdHardwareReset()) {
    return false;
  }
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
  return g_epdAlive;
}

static void epdDeepSleep() {
  epdSendCommand(0x07);
  epdSendData(0xA5);
}

void epdPanelSleep() {
  epdDeepSleep();
}

static bool epdTurnOnDisplay() {
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
  return g_epdAlive;
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
  gpio_reset_pin((gpio_num_t)PIN_EPD_DC);
  gpio_reset_pin((gpio_num_t)PIN_EPD_CS);
  gpio_reset_pin((gpio_num_t)PIN_EPD_SCK);
  gpio_reset_pin((gpio_num_t)PIN_EPD_MOSI);
  gpio_reset_pin((gpio_num_t)PIN_EPD_RST);
  gpio_reset_pin((gpio_num_t)PIN_EPD_BUSY);
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

static bool epdHangLandscape() {
  const char *h = hangValue();
  return h && h[0] == 'l';
}

static void epdVisSize(int &w, int &h) {
  if (epdHangLandscape()) {
    w = EPD_WIDTH;
    h = EPD_HEIGHT;
  } else {
    w = EPD_HEIGHT;
    h = EPD_WIDTH;
  }
}

static void epdSetPixelVis(int vx, int vy, uint8_t color) {
  if (epdHangLandscape()) {
    epdSetPixel(vx, vy, color);
    return;
  }
  // Portrait as hung: 480×800. Same map as bmp 90° CW (dx=sy, dy=479-sx).
  epdSetPixel(vy, EPD_HEIGHT - 1 - vx, color);
}

static const int HINT_BOX = 48;
static const int HINT_MARGIN = 8;

// 8×8, bit0 = left, row 0 = top.
static const uint8_t ICON_BAT[8] = {0x3C, 0x7E, 0x42, 0x42, 0x42, 0x5A, 0x42, 0x7E};
static const uint8_t ICON_ARROW[8] = {0x08, 0x18, 0x38, 0x7F, 0x7F, 0x38, 0x18, 0x08};

static void epdDrawHintBox(int x0, int y0) {
  for (int y = 0; y < HINT_BOX; y++) {
    for (int x = 0; x < HINT_BOX; x++) {
      uint8_t c = EPD_WHITE;
      if (x < 4 || y < 4 || x >= HINT_BOX - 4 || y >= HINT_BOX - 4) {
        c = EPD_BLACK;
      }
      epdSetPixelVis(x0 + x, y0 + y, c);
    }
  }
}

static void epdDrawIcon8(int x0, int y0, const uint8_t *rows) {
  const int scale = 4;
  const int pad = (HINT_BOX - 8 * scale) / 2;
  for (int r = 0; r < 8; r++) {
    uint8_t bits = rows[r];
    for (int c = 0; c < 8; c++) {
      if (bits & (1 << c)) {
        int px = x0 + pad + c * scale;
        int py = y0 + pad + r * scale;
        for (int dy = 0; dy < scale; dy++) {
          for (int dx = 0; dx < scale; dx++) {
            epdSetPixelVis(px + dx, py + dy, EPD_BLACK);
          }
        }
      }
    }
  }
}

static bool g_forceBattWarn = false;
static int g_moreMemoriesHint = 0;

void epdForceBatteryWarn(bool on) { g_forceBattWarn = on; }

void epdSetMoreMemoriesHint(int extra) {
  g_moreMemoriesHint = extra < 0 ? 0 : extra;
}

static void epdDrawBatteryWarn() {
  int visW = 0, visH = 0;
  epdVisSize(visW, visH);
  (void)visW;
  int x0 = HINT_MARGIN;
  int y0 = visH - HINT_BOX - HINT_MARGIN;
  epdDrawHintBox(x0, y0);
  epdDrawIcon8(x0, y0, ICON_BAT);
}

static void epdDrawMoreMemoriesHint(int extra) {
  if (extra < 1) {
    return;
  }
  int visW = 0, visH = 0;
  epdVisSize(visW, visH);
  int x0 = visW - HINT_BOX - HINT_MARGIN;
  int y0 = visH - HINT_BOX - HINT_MARGIN;
  epdDrawHintBox(x0, y0);
  epdDrawIcon8(x0, y0, ICON_ARROW);
}

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
  if (g_moreMemoriesHint > 0) {
    epdDrawMoreMemoriesHint(g_moreMemoriesHint);
  }
  if (warn) {
    epdDrawBatteryWarn();
  }
  Serial.println(F("EPD refresh…"));
  if (!epdPortInit()) {
    Serial.println(F("EPD init failed"));
    return;
  }
  epdWriteBuffer();
  if (!epdTurnOnDisplay()) {
    Serial.println(F("EPD display failed"));
    return;
  }
  epdDeepSleep();
  Serial.println(F("EPD done"));
}
