#pragma once

// Waveshare ESP32-S3-PhotoPainter — Arduino IDE only (no ESP-IDF)

// EPD (Spectra 6 / E6) — FSPI
static const int PIN_EPD_DC = 8;
static const int PIN_EPD_CS = 9;
static const int PIN_EPD_SCK = 10;
static const int PIN_EPD_MOSI = 11;
static const int PIN_EPD_RST = 12;
static const int PIN_EPD_BUSY = 13;

// Audio ES8311 (Waveshare PhotoPainter board_cfg)
static const int PIN_I2S_MCLK = 14;
static const int PIN_I2S_BCLK = 15;
static const int PIN_I2S_WS = 16;
static const int PIN_I2S_DOUT = 17;  // ESP → ES8311 DSDIN
static const int PIN_I2S_DIN = 18;   // mic
static const int PIN_AUDIO_PA = 7;
static const uint8_t ES8311_ADDR = 0x18;
static const uint8_t ES7210_ADDR = 0x40;
static const uint8_t SHTC3_ADDR = 0x70;

// AXP2101 IRQ (wake PMIC after enableSleep; >16 ms low)
static const int PIN_AXP_IRQ = 21;

// SDIO 4-bit
static const int SD_CLK = 39;
static const int SD_CMD = 41;
static const int SD_D0 = 40;
static const int SD_D1 = 1;
static const int SD_D2 = 2;
static const int SD_D3 = 38;

// AXP2101 I2C
static const int I2C_SDA = 47;
static const int I2C_SCL = 48;

static const int PIN_LED_GREEN = 42;
static const int PIN_LED_RED = 45;
static const int PIN_BOOT = 0;  // BOOT — deep-sleep wake, Web an, kein Bildwechsel
static const int PIN_KEY = 4;   // KEY — nächstes Bild (Zufall, oder nächste fällige Erinnerung)

static const int EPD_WIDTH = 800;
static const int EPD_HEIGHT = 480;
static const int EPD_ROW_BYTES = (EPD_WIDTH / 2);
static const int EPD_BUF_SIZE = (EPD_ROW_BYTES * EPD_HEIGHT);

static const uint8_t EPD_BLACK = 0x0;
static const uint8_t EPD_WHITE = 0x1;
static const uint8_t EPD_YELLOW = 0x2;
static const uint8_t EPD_RED = 0x3;
static const uint8_t EPD_BLUE = 0x5;
static const uint8_t EPD_GREEN = 0x6;

static const char *SD_MOUNT = "/sdcard";
static const char *PIC_DIR = "/pic";
static const char *LIST_CACHE_PATH = "/pic/_gallery.json";
static const char *MEMORIES_PATH = "/pic/_erinnerungen.json";
static const char *DEVICE_NAME = "Tintenklecks";
static const char *COPYRIGHT = "© 2026 Ingo Lissors";
static const char *AP_SSID = "Tintenklecks";
static const char *AP_PASS = "tintenklecks";  // WPA2 ≥ 8 chars
static const char *MDNS_HOST = "tintenklecks";

static const uint32_t RX_TIMEOUT_MS = 45000;
static const size_t MAX_BMP = 4UL * 1024UL * 1024UL;
static const size_t ACK_EVERY = 16384;
static const uint32_t WIFI_CONNECT_MS = 15000;
