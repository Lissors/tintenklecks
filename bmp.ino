// 24-bit BMP → Spectra 6 framebuffer → panel

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"

struct Rgb6 {
  uint8_t mr, mg, mb;  // measured (match)
  uint8_t dr, dg, db;  // device (exact BMP from studio)
  uint8_t code;
};

// Match like aitjcize/esp32-photoframe measured defaults (MIT, Wei-Ning Huang);
// accept pure device RGB too.
static const Rgb6 kPalette[] = {
    {2, 2, 2, 0, 0, 0, EPD_BLACK},
    {190, 200, 200, 255, 255, 255, EPD_WHITE},
    {205, 202, 0, 255, 255, 0, EPD_YELLOW},
    {135, 19, 0, 255, 0, 0, EPD_RED},
    {5, 64, 158, 0, 0, 255, EPD_BLUE},
    {39, 102, 60, 0, 255, 0, EPD_GREEN},
};

static uint8_t nearestEpd(uint8_t r, uint8_t g, uint8_t b) {
  // Exact device RGB (studio dither output)
  for (size_t i = 0; i < sizeof(kPalette) / sizeof(kPalette[0]); i++) {
    if (r == kPalette[i].dr && g == kPalette[i].dg && b == kPalette[i].db) {
      return kPalette[i].code;
    }
  }
  uint32_t best = 0xFFFFFFFFu;
  uint8_t code = EPD_WHITE;
  for (size_t i = 0; i < sizeof(kPalette) / sizeof(kPalette[0]); i++) {
    int dr = (int)r - (int)kPalette[i].mr;
    int dg = (int)g - (int)kPalette[i].mg;
    int db = (int)b - (int)kPalette[i].mb;
    uint32_t d = (uint32_t)(dr * dr + dg * dg + db * db);
    // also distance to device RGB (exported BMPs)
    int er = (int)r - (int)kPalette[i].dr;
    int eg = (int)g - (int)kPalette[i].dg;
    int eb = (int)b - (int)kPalette[i].db;
    uint32_t de = (uint32_t)(er * er + eg * eg + eb * eb);
    if (de < d) {
      d = de;
    }
    if (d < best) {
      best = d;
      code = kPalette[i].code;
    }
  }
  return code;
}

static uint16_t readU16(const uint8_t *p) {
  return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t readU32(const uint8_t *p) {
  return (uint32_t)(p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24));
}

static int32_t readI32(const uint8_t *p) {
  return (int32_t)readU32(p);
}

/** Put BMP pixel (sx,sy) onto panel coords with orientation. */
static void putOriented(int sx, int sy, int srcW, int srcH, uint8_t color) {
  int dx, dy;
  if (srcW == EPD_WIDTH && srcH == EPD_HEIGHT) {
    // Landscape 800×480 — panel native
    dx = sx;
    dy = sy;
  } else if (srcW == EPD_HEIGHT && srcH == EPD_WIDTH) {
    // Portrait 480×800 → rotate 90° CW into 800×480
    dx = sy;
    dy = srcW - 1 - sx;
  } else {
    // other size: already filled white
    return;
  }
  epdSetPixel(dx, dy, color);
}

bool bmpShowFromMemory(const uint8_t *data, size_t len) {
  if (!data || len < 54) {
    Serial.println(F("ERR bmp too small"));
    return false;
  }
  if (data[0] != 'B' || data[1] != 'M') {
    Serial.println(F("ERR not BMP"));
    return false;
  }

  uint32_t pixelOffset = readU32(data + 10);
  uint32_t dibSize = readU32(data + 14);
  if (dibSize < 40 || pixelOffset >= len) {
    Serial.println(F("ERR bmp header"));
    return false;
  }

  int32_t width = readI32(data + 18);
  int32_t heightRaw = readI32(data + 22);
  uint16_t planes = readU16(data + 26);
  uint16_t bpp = readU16(data + 28);
  uint32_t compression = readU32(data + 30);

  if (planes != 1 || bpp != 24 || compression != 0) {
    Serial.println(F("ERR need 24-bit uncompressed BMP"));
    return false;
  }
  if (width <= 0) {
    return false;
  }

  bool bottomUp = heightRaw > 0;
  int32_t height = heightRaw < 0 ? -heightRaw : heightRaw;
  if (height <= 0) {
    return false;
  }

  // Row size padded to 4 bytes
  uint32_t rowSize = ((uint32_t)width * 3u + 3u) & ~3u;
  uint32_t need = pixelOffset + rowSize * (uint32_t)height;
  if (need > len) {
    Serial.println(F("ERR bmp truncated"));
    return false;
  }

  Serial.printf("BMP %dx%d → EPD\n", (int)width, (int)height);
  epdClear(EPD_WHITE);

  for (int32_t row = 0; row < height; row++) {
    int32_t srcY = bottomUp ? (height - 1 - row) : row;
    const uint8_t *line = data + pixelOffset + (uint32_t)row * rowSize;
    for (int32_t x = 0; x < width; x++) {
      const uint8_t *px = line + (uint32_t)x * 3u;
      // BMP is BGR
      uint8_t b = px[0], g = px[1], r = px[2];
      uint8_t c = nearestEpd(r, g, b);
      putOriented((int)x, (int)srcY, (int)width, (int)height, c);
    }
    if ((row & 31) == 0) {
      yield();
    }
  }

  epdDisplayCurrentBuffer();
  return true;
}

bool bmpShowFromSd(const char *path) {
  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    Serial.println(F("ERR open bmp"));
    return false;
  }
  size_t size = f.size();
  if (size == 0 || size > MAX_BMP) {
    f.close();
    return false;
  }
  uint8_t *ram = (uint8_t *)ps_malloc(size);
  if (!ram) {
    f.close();
    Serial.println(F("ERR PSRAM"));
    return false;
  }
  size_t got = f.read(ram, size);
  f.close();
  bool ok = (got == size) && bmpShowFromMemory(ram, size);
  free(ram);
  return ok;
}

static bool bmpSamplePanel(const uint8_t *data, size_t len, int lx, int ly, uint8_t &outColor) {
  if (!data || len < 54 || data[0] != 'B' || data[1] != 'M') {
    return false;
  }
  uint32_t pixelOffset = readU32(data + 10);
  int32_t width = readI32(data + 18);
  int32_t heightRaw = readI32(data + 22);
  uint16_t bpp = readU16(data + 28);
  uint32_t compression = readU32(data + 30);
  if (bpp != 24 || compression != 0 || width <= 0) {
    return false;
  }
  bool bottomUp = heightRaw > 0;
  int32_t height = heightRaw < 0 ? -heightRaw : heightRaw;
  if (height <= 0) {
    return false;
  }
  uint32_t rowSize = ((uint32_t)width * 3u + 3u) & ~3u;

  int sx, sy;
  if (width == EPD_WIDTH && height == EPD_HEIGHT) {
    sx = lx;
    sy = ly;
  } else if (width == EPD_HEIGHT && height == EPD_WIDTH) {
    // inverse of putOriented portrait CW
    sx = width - 1 - ly;
    sy = lx;
  } else {
    // generic: map 800×480 logical onto source contain
    sx = lx * width / EPD_WIDTH;
    sy = ly * height / EPD_HEIGHT;
  }
  if (sx < 0 || sy < 0 || sx >= width || sy >= height) {
    return false;
  }
  int32_t fileRow = bottomUp ? (height - 1 - sy) : sy;
  const uint8_t *line = data + pixelOffset + (uint32_t)fileRow * rowSize;
  const uint8_t *px = line + (uint32_t)sx * 3u;
  outColor = nearestEpd(px[2], px[1], px[0]);
  return true;
}

/** Blit one BMP into a panel slot (contain / letterbox). Does not clear or refresh. */
static bool bmpBlitSlot(const uint8_t *data, size_t len, int slotX, int slotY, int slotW, int slotH) {
  if (slotW <= 0 || slotH <= 0) {
    return false;
  }
  // Fit 800×480 logical frame into slot
  float scale = (float)slotW / (float)EPD_WIDTH;
  float scaleY = (float)slotH / (float)EPD_HEIGHT;
  if (scaleY < scale) {
    scale = scaleY;
  }
  int drawW = (int)(EPD_WIDTH * scale);
  int drawH = (int)(EPD_HEIGHT * scale);
  if (drawW < 1) {
    drawW = 1;
  }
  if (drawH < 1) {
    drawH = 1;
  }
  int ox = slotX + (slotW - drawW) / 2;
  int oy = slotY + (slotH - drawH) / 2;

  for (int py = oy; py < oy + drawH; py++) {
    for (int px = ox; px < ox + drawW; px++) {
      int lx = (px - ox) * EPD_WIDTH / drawW;
      int ly = (py - oy) * EPD_HEIGHT / drawH;
      uint8_t c = EPD_WHITE;
      if (!bmpSamplePanel(data, len, lx, ly, c)) {
        c = EPD_WHITE;
      }
      epdSetPixel(px, py, c);
    }
    if ((py & 15) == 0) {
      yield();
    }
  }
  return true;
}

bool bmpShowCompositeFromSd(const char *const *paths, int count) {
  if (!paths || count < 1) {
    return false;
  }
  if (count > 3) {
    count = 3;
  }
  if (count == 1) {
    return bmpShowFromSd(paths[0]);
  }

  epdClear(EPD_WHITE);
  const int slotH = EPD_HEIGHT / count;
  for (int i = 0; i < count; i++) {
    File f = SD_MMC.open(paths[i], FILE_READ);
    if (!f) {
      Serial.printf("ERR composite open %s\n", paths[i]);
      continue;
    }
    size_t size = f.size();
    if (size == 0 || size > MAX_BMP) {
      f.close();
      continue;
    }
    uint8_t *ram = (uint8_t *)ps_malloc(size);
    if (!ram) {
      f.close();
      Serial.println(F("ERR PSRAM composite"));
      continue;
    }
    size_t got = f.read(ram, size);
    f.close();
    if (got == size) {
      int y = i * slotH;
      int h = (i == count - 1) ? (EPD_HEIGHT - y) : slotH;
      bmpBlitSlot(ram, size, 0, y, EPD_WIDTH, h);
      Serial.printf("Composite[%d] %s → y=%d h=%d\n", i, paths[i], y, h);
    }
    free(ram);
  }
  epdDisplayCurrentBuffer();
  return true;
}
