// SD card (SDIO 4-bit)

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"
#include <driver/gpio.h>

static bool g_sdOk = false;

bool sdOk() {
  return g_sdOk;
}

bool sdInit() {
  gpio_reset_pin((gpio_num_t)SD_CLK);
  gpio_reset_pin((gpio_num_t)SD_CMD);
  gpio_reset_pin((gpio_num_t)SD_D0);
  gpio_reset_pin((gpio_num_t)SD_D1);
  gpio_reset_pin((gpio_num_t)SD_D2);
  gpio_reset_pin((gpio_num_t)SD_D3);
  if (!SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3)) {
    Serial.println(F("ERR SD setPins"));
    g_sdOk = false;
    return false;
  }
  for (int t = 0; t < 3; t++) {
    if (SD_MMC.begin(SD_MOUNT, false)) {
      if (!SD_MMC.exists(PIC_DIR)) {
        SD_MMC.mkdir(PIC_DIR);
      }
      if (SD_MMC.exists("/sleep.log")) {
        SD_MMC.remove("/sleep.log");
      }
      g_sdOk = true;
      Serial.printf("SD OK %llu MB\n",
                    (unsigned long long)(SD_MMC.totalBytes() / (1024ULL * 1024ULL)));
      return true;
    }
    SD_MMC.end();
    delay(200);
  }
  Serial.println(F("ERR SD mount — FAT32 card?"));
  g_sdOk = false;
  return false;
}

void sdDeinit() {
  if (g_sdOk) {
    SD_MMC.end();
    g_sdOk = false;
  }
  pinMode(SD_CLK, INPUT);
  pinMode(SD_CMD, INPUT);
  pinMode(SD_D0, INPUT);
  pinMode(SD_D1, INPUT);
  pinMode(SD_D2, INPUT);
  pinMode(SD_D3, INPUT);
  Serial.println(F("SD unmount"));
}
