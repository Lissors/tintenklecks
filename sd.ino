// SD card (SDIO 4-bit)

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"

static bool g_sdOk = false;

bool sdOk() {
  return g_sdOk;
}

bool sdInit() {
  if (!SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3)) {
    Serial.println(F("ERR SD setPins"));
    g_sdOk = false;
    return false;
  }
  if (!SD_MMC.begin(SD_MOUNT, false)) {
    Serial.println(F("ERR SD mount — FAT32 card?"));
    g_sdOk = false;
    return false;
  }
  if (!SD_MMC.exists(PIC_DIR)) {
    SD_MMC.mkdir(PIC_DIR);
  }
  g_sdOk = true;
  Serial.printf("SD OK %llu MB\n",
                (unsigned long long)(SD_MMC.totalBytes() / (1024ULL * 1024ULL)));
  return true;
}
