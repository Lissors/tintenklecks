// ES8311 + PA speaker — PhotoPainter pins (GPIO 14/15/16/17, PA 7)

#include "config.h"
#include "board.h"
#include "FS.h"
#include "SD_MMC.h"
#include <Wire.h>
#include <math.h>
#include <driver/i2s_std.h>
#include <driver/gpio.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <Preferences.h>

static bool g_audioOk = false;
static bool g_playing = false;
static i2s_chan_handle_t g_tx = nullptr;
static uint8_t *g_pcm = nullptr;
static size_t g_pcmLen = 0;
static size_t g_pcmPos = 0;
static uint32_t g_rate = 22050;
static uint16_t g_channels = 1;
static TaskHandle_t g_playTask = nullptr;
static SemaphoreHandle_t g_audioMu = nullptr;

static bool es8311Write(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  Wire.write(val);
  return Wire.endTransmission() == 0;
}

static uint8_t es8311Read(uint8_t reg) {
  Wire.beginTransmission(ES8311_ADDR);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return 0xFF;
  }
  if (Wire.requestFrom((uint8_t)ES8311_ADDR, (uint8_t)1) != 1) {
    return 0xFF;
  }
  return (uint8_t)Wire.read();
}

static const uint8_t AUDIO_DAC_GAIN = 0x60;  // REG17 analog gain
static uint8_t g_volPct = 80;
static bool g_volLoaded = false;

static void es8311SetVolume(uint8_t pct);

static void volLoad() {
  if (g_volLoaded) {
    return;
  }
  Preferences p;
  p.begin("audio", true);
  g_volPct = p.getUChar("vol", 80);
  p.end();
  if (g_volPct > 100) {
    g_volPct = 80;
  }
  g_volLoaded = true;
}

uint8_t audioVolume() {
  volLoad();
  return g_volPct;
}

bool audioSetVolume(uint8_t pct) {
  volLoad();
  if (pct > 100) {
    pct = 100;
  }
  g_volPct = pct;
  Preferences p;
  p.begin("audio", false);
  p.putUChar("vol", g_volPct);
  p.end();
  if (g_audioOk) {
    es8311SetVolume(g_volPct);
  }
  return true;
}

static void es8311SetVolume(uint8_t pct) {
  if (pct > 100) {
    pct = 100;
  }
  if (pct == 0) {
    uint8_t m = es8311Read(0x31);
    es8311Write(0x31, (uint8_t)((m & 0x9F) | 0x60));
    return;
  }
  // REG32 DAC volume 0…255
  es8311Write(0x32, (uint8_t)(((uint16_t)pct * 255u + 99u) / 100u));
  uint8_t m = es8311Read(0x31);
  es8311Write(0x31, (uint8_t)(m & 0x9F));  // unmute
  es8311Write(0x17, AUDIO_DAC_GAIN);
}

// MCLK = Fs×256 — same divider set works for 16/22.05/24 kHz
static bool es8311Init(uint32_t sampleRate) {
  (void)sampleRate;
  delay(10);
  Wire.beginTransmission(ES8311_ADDR);
  if (Wire.endTransmission() != 0) {
    return false;
  }

  const uint8_t kPreDiv = 0x01;
  const uint8_t kPreMultiCode = 0;
  const uint8_t kAdcDiv = 0x01;
  const uint8_t kDacDiv = 0x01;
  const uint8_t kFsMode = 0x00;
  const uint8_t kAdcOsr = 0x10;
  const uint8_t kDacOsr = 0x10;
  const uint8_t kLrckH = 0x00;
  const uint8_t kLrckL = 0xFF;
  const uint8_t kBclkDiv = 0x08;  // MCLK/8 = Fs*32 → stereo 16-bit

  es8311Write(0x00, 0x1F);
  delay(50);
  es8311Write(0x00, 0x00);
  delay(50);

  es8311Write(0x44, 0x08);
  es8311Write(0x01, 0x30);
  es8311Write(0x02, 0x00);
  es8311Write(0x03, 0x10);
  es8311Write(0x16, 0x24);
  es8311Write(0x04, 0x10);
  es8311Write(0x05, 0x00);
  es8311Write(0x0B, 0x00);
  es8311Write(0x0C, 0x00);
  es8311Write(0x10, 0x1F);
  es8311Write(0x11, 0x7F);
  es8311Write(0x00, 0x80);
  es8311Write(0x00, (uint8_t)(es8311Read(0x00) & (uint8_t)~0x40));  // I2S slave

  es8311Write(0x01, 0x3F);  // external MCLK
  uint8_t r06 = es8311Read(0x06);
  es8311Write(0x06, (uint8_t)(r06 & (uint8_t)~0x20));

  es8311Write(0x13, 0x10);
  es8311Write(0x1B, 0x0A);
  es8311Write(0x1C, 0x6A);
  es8311Write(0x44, 0x58);

  uint8_t r02 = es8311Read(0x02);
  r02 = (uint8_t)((r02 & 7) | ((uint8_t)(kPreDiv - 1) << 5) | (uint8_t)(kPreMultiCode << 3));
  es8311Write(0x02, r02);
  es8311Write(0x05, (uint8_t)(((kAdcDiv - 1) << 4) | ((kDacDiv - 1) << 0)));

  r02 = es8311Read(0x03);
  es8311Write(0x03, (uint8_t)((r02 & 0x80) | (kFsMode << 6) | kAdcOsr));
  r02 = es8311Read(0x04);
  es8311Write(0x04, (uint8_t)((r02 & 0x80) | kDacOsr));

  r02 = es8311Read(0x07);
  es8311Write(0x07, (uint8_t)((r02 & 0xC0) | kLrckH));
  es8311Write(0x08, kLrckL);

  uint8_t bclkLo = kBclkDiv < 19 ? (uint8_t)(kBclkDiv - 1) : kBclkDiv;
  r06 = es8311Read(0x06);
  es8311Write(0x06, (uint8_t)((r06 & 0xE0) | bclkLo));

  // 16-bit Philips I2S, DAC SDP active
  uint8_t dac9 = es8311Read(0x09);
  uint8_t adcA = es8311Read(0x0A);
  dac9 = (uint8_t)((dac9 & (uint8_t)~0x1C) | 0x0C);
  adcA = (uint8_t)((adcA & (uint8_t)~0x1C) | 0x0C);
  dac9 = (uint8_t)((dac9 & 0xFC) | 0x0C);
  adcA = (uint8_t)((adcA & 0xFC) | 0x0C);
  dac9 = (uint8_t)(dac9 & (uint8_t)~0x40);  // DAC SDP enable (bit6 clear)
  adcA = (uint8_t)((adcA & 0xBF) | 0x40);
  es8311Write(0x09, dac9);
  es8311Write(0x0A, adcA);

  es8311Write(0x17, AUDIO_DAC_GAIN);
  es8311Write(0x0E, 0x02);
  es8311Write(0x12, 0x00);
  es8311Write(0x14, 0x1A);
  es8311Write(0x14, (uint8_t)(es8311Read(0x14) & (uint8_t)~0x40));
  es8311Write(0x0D, 0x01);
  es8311Write(0x15, 0x40);
  es8311Write(0x37, 0x08);
  es8311Write(0x45, 0x00);
  es8311Write(0x0F, 0x9C);
  delay(50);

  es8311SetVolume(audioVolume());

  for (int i = 0; i < 50; i++) {
    if ((es8311Read(0x0B) & 0x03) == 0x02) {
      break;
    }
    delay(10);
  }
  uint8_t st = es8311Read(0x0B);
  Serial.printf("ES8311 REG0B=0x%02X %s\n", st, (st & 3) == 2 ? "NORMAL" : "wait");
  return true;
}

static bool i2sStart(uint32_t sampleRate) {
  if (g_tx) {
    i2s_channel_disable(g_tx);
    i2s_del_channel(g_tx);
    g_tx = nullptr;
  }

  i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
  chan_cfg.auto_clear = true;
  chan_cfg.dma_desc_num = 8;
  chan_cfg.dma_frame_num = 256;
  if (i2s_new_channel(&chan_cfg, &g_tx, nullptr) != ESP_OK) {
    return false;
  }

  i2s_std_config_t std_cfg = {
      .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sampleRate),
      .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
      .gpio_cfg =
          {
              .mclk = (gpio_num_t)PIN_I2S_MCLK,
              .bclk = (gpio_num_t)PIN_I2S_BCLK,
              .ws = (gpio_num_t)PIN_I2S_WS,
              .dout = (gpio_num_t)PIN_I2S_DOUT,
              .din = I2S_GPIO_UNUSED,
              .invert_flags = {.mclk_inv = false, .bclk_inv = false, .ws_inv = false},
          },
  };
  std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;

  if (i2s_channel_init_std_mode(g_tx, &std_cfg) != ESP_OK) {
    i2s_del_channel(g_tx);
    g_tx = nullptr;
    return false;
  }
  if (i2s_channel_enable(g_tx) != ESP_OK) {
    i2s_del_channel(g_tx);
    g_tx = nullptr;
    return false;
  }
  return true;
}

static void audioPa(bool on) {
  gpio_hold_dis((gpio_num_t)PIN_AUDIO_PA);
  gpio_reset_pin((gpio_num_t)PIN_AUDIO_PA);
  pinMode(PIN_AUDIO_PA, OUTPUT);
  digitalWrite(PIN_AUDIO_PA, on ? HIGH : LOW);
}

bool audioInit() {
  if (g_audioOk) {
    return true;
  }
  if (!g_audioMu) {
    g_audioMu = xSemaphoreCreateMutex();
  }
  audioPa(false);

  bool found = false;
  for (int i = 0; i < 8; i++) {
    delay(50);
    Wire.beginTransmission(ES8311_ADDR);
    if (Wire.endTransmission() == 0) {
      found = true;
      break;
    }
  }
  if (!found) {
    Serial.println(F("WARN ES8311 not found — speak disabled"));
    g_audioOk = false;
    return false;
  }

  g_audioOk = true;
  Serial.println(F("Audio ES8311 present (lazy I2S)"));
  return true;
}

bool audioEnsureReady() {
  if (g_audioOk) {
    return true;
  }
  pmuEnableAudioRail();
  return audioInit();
}

bool audioReady() {
  return g_audioOk;
}

bool audioBusy() {
  return g_playing;
}

void audioStop() {
  if (g_audioMu) {
    xSemaphoreTake(g_audioMu, pdMS_TO_TICKS(200));
  }
  g_playing = false;
  g_pcmPos = 0;
  if (g_pcm) {
    free(g_pcm);
    g_pcm = nullptr;
  }
  g_pcmLen = 0;
  if (g_audioMu) {
    xSemaphoreGive(g_audioMu);
  }
}

void audioPowerDown() {
  audioStop();
  audioPa(false);
}

static bool parseWav(const uint8_t *buf, size_t len, size_t *dataOff, size_t *dataLen) {
  if (len < 44 || buf[0] != 'R' || buf[1] != 'I' || buf[2] != 'F' || buf[3] != 'F') {
    return false;
  }
  if (buf[8] != 'W' || buf[9] != 'A' || buf[10] != 'V' || buf[11] != 'E') {
    return false;
  }
  size_t pos = 12;
  bool haveFmt = false;
  *dataOff = 0;
  *dataLen = 0;
  while (pos + 8 <= len) {
    uint32_t id = (uint32_t)buf[pos] | ((uint32_t)buf[pos + 1] << 8) | ((uint32_t)buf[pos + 2] << 16) |
                  ((uint32_t)buf[pos + 3] << 24);
    uint32_t sz = (uint32_t)buf[pos + 4] | ((uint32_t)buf[pos + 5] << 8) | ((uint32_t)buf[pos + 6] << 16) |
                  ((uint32_t)buf[pos + 7] << 24);
    pos += 8;
    if (pos + sz > len) {
      break;
    }
    if (id == 0x20746d66u) {  // "fmt "
      if (sz < 16) {
        return false;
      }
      uint16_t fmt = (uint16_t)buf[pos] | ((uint16_t)buf[pos + 1] << 8);
      g_channels = (uint16_t)buf[pos + 2] | ((uint16_t)buf[pos + 3] << 8);
      g_rate = (uint32_t)buf[pos + 4] | ((uint32_t)buf[pos + 5] << 8) | ((uint32_t)buf[pos + 6] << 16) |
               ((uint32_t)buf[pos + 7] << 24);
      uint16_t bits = (uint16_t)buf[pos + 14] | ((uint16_t)buf[pos + 15] << 8);
      if ((fmt != 1 && fmt != 0xFFFE) || bits != 16 || (g_channels != 1 && g_channels != 2)) {
        return false;
      }
      haveFmt = true;
    } else if (id == 0x61746164u) {  // "data"
      *dataOff = pos;
      *dataLen = sz;
      if (*dataOff + *dataLen > len) {
        *dataLen = len - *dataOff;
      }
      break;
    }
    pos += sz + (sz & 1);
  }
  return haveFmt && *dataLen > 0;
}

static void audioPlayTask(void *arg) {
  (void)arg;
  const size_t chunk = 256;
  int16_t out[chunk * 2];

  while (true) {
    if (!g_playing || !g_tx || !g_pcm) {
      vTaskDelay(pdMS_TO_TICKS(20));
      continue;
    }

    size_t frames = 0;
    if (g_audioMu) {
      xSemaphoreTake(g_audioMu, portMAX_DELAY);
    }
    while (frames < chunk && g_playing && g_pcm && g_pcmPos + 2 <= g_pcmLen) {
      int16_t s = (int16_t)(g_pcm[g_pcmPos] | (g_pcm[g_pcmPos + 1] << 8));
      g_pcmPos += 2;
      s = (int16_t)((int32_t)s * 2 / 5);
      if (g_channels == 2) {
        int16_t s2 = (g_pcmPos + 2 <= g_pcmLen)
                         ? (int16_t)(g_pcm[g_pcmPos] | (g_pcm[g_pcmPos + 1] << 8))
                         : s;
        g_pcmPos += 2;
        s2 = (int16_t)((int32_t)s2 * 2 / 5);
        out[frames * 2] = s;
        out[frames * 2 + 1] = s2;
      } else {
        out[frames * 2] = s;
        out[frames * 2 + 1] = s;
      }
      frames++;
    }
    bool done = g_playing && g_pcm && g_pcmPos >= g_pcmLen;
    if (g_audioMu) {
      xSemaphoreGive(g_audioMu);
    }

    if (frames == 0) {
      audioStop();
      continue;
    }

    size_t written = 0;
    esp_err_t err = i2s_channel_write(g_tx, out, frames * 4, &written, pdMS_TO_TICKS(200));
    if (err != ESP_OK) {
      Serial.printf("i2s_write err=%d\n", (int)err);
      audioStop();
      continue;
    }
    if (done) {
      audioStop();
    }
  }
}

static bool audioEnsurePlayTask() {
  if (g_playTask) {
    return true;
  }
  BaseType_t ok = xTaskCreatePinnedToCore(audioPlayTask, "audioPlay", 4096, nullptr, 1, &g_playTask, 1);
  return ok == pdPASS;
}

static bool audioStartPcm(uint8_t *pcm, size_t dataLen, uint32_t rate, uint16_t channels) {
  if (!g_audioOk || !pcm || dataLen < 4) {
    return false;
  }
  audioStop();
  g_rate = rate ? rate : 22050;
  g_channels = channels ? channels : 1;

  if (!i2sStart(g_rate) || !es8311Init(g_rate)) {
    Serial.println(F("audio: I2S/ES8311 init fail"));
    free(pcm);
    return false;
  }
  audioPa(true);
  es8311SetVolume(audioVolume());
  delay(20);

  if (!audioEnsurePlayTask()) {
    free(pcm);
    return false;
  }

  if (g_audioMu) {
    xSemaphoreTake(g_audioMu, portMAX_DELAY);
  }
  g_pcm = pcm;
  g_pcmLen = dataLen;
  g_pcmPos = 0;
  g_playing = true;
  if (g_audioMu) {
    xSemaphoreGive(g_audioMu);
  }
  Serial.printf("audio play %u Hz ch=%u bytes=%u\n", (unsigned)g_rate, (unsigned)g_channels,
                (unsigned)dataLen);
  return true;
}

bool audioPlaySd(const char *path) {
  if (!audioEnsureReady() || !path || !sdOk()) {
    return false;
  }

  File f = SD_MMC.open(path, FILE_READ);
  if (!f) {
    return false;
  }
  size_t sz = f.size();
  if (sz < 44 || sz > 1536UL * 1024UL) {
    Serial.printf("audio: skip size %u\n", (unsigned)sz);
    f.close();
    return false;
  }
  uint8_t *fileBuf = (uint8_t *)heap_caps_malloc(sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!fileBuf) {
    fileBuf = (uint8_t *)malloc(sz);
  }
  if (!fileBuf) {
    f.close();
    return false;
  }
  size_t got = 0;
  while (got < sz) {
    size_t chunk = sz - got;
    if (chunk > 4096) {
      chunk = 4096;
    }
    int n = f.read(fileBuf + got, chunk);
    if (n <= 0) {
      break;
    }
    got += (size_t)n;
    yield();
  }
  f.close();
  if (got != sz) {
    free(fileBuf);
    return false;
  }

  size_t dataOff = 0, dataLen = 0;
  if (!parseWav(fileBuf, sz, &dataOff, &dataLen)) {
    free(fileBuf);
    Serial.println(F("audio: bad WAV (need PCM 16-bit mono/stereo)"));
    return false;
  }

  uint8_t *pcm = (uint8_t *)heap_caps_malloc(dataLen, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!pcm) {
    pcm = (uint8_t *)malloc(dataLen);
  }
  if (!pcm) {
    free(fileBuf);
    return false;
  }
  memcpy(pcm, fileBuf + dataOff, dataLen);
  free(fileBuf);

  return audioStartPcm(pcm, dataLen, g_rate, g_channels);
}

bool audioPlayClip(const char *clip) {
  if (!clip || !clip[0]) {
    return false;
  }
  if (!sdOk()) {
    Serial.println(F("audio: SD not ready"));
    return false;
  }
  String path = String("/sound/") + clip + ".wav";
  if (!SD_MMC.exists(path)) {
    Serial.printf("audio: missing %s\n", path.c_str());
    return false;
  }
  if (!audioPlaySd(path.c_str())) {
    Serial.printf("audio: play fail %s\n", path.c_str());
    return false;
  }
  return true;
}

void audioWaitIdle(uint32_t maxMs) {
  uint32_t t0 = millis();
  while (audioBusy() && (millis() - t0) < maxMs) {
    delay(20);
    yield();
  }
}

bool audioPlayBeep(uint32_t ms) {
  if (!audioEnsureReady()) {
    return false;
  }
  const uint32_t rate = 22050;
  const uint32_t samples = (rate * (ms ? ms : 400)) / 1000;
  size_t bytes = samples * 2;
  uint8_t *pcm = (uint8_t *)heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!pcm) {
    pcm = (uint8_t *)malloc(bytes);
  }
  if (!pcm) {
    return false;
  }
  for (uint32_t i = 0; i < samples; i++) {
    float t = (float)i / (float)rate;
    int16_t s = (int16_t)(sinf(2.0f * 3.1415926f * 880.0f * t) * 5000.0f);
    pcm[i * 2] = (uint8_t)(s & 0xFF);
    pcm[i * 2 + 1] = (uint8_t)((s >> 8) & 0xFF);
  }
  g_channels = 1;
  return audioStartPcm(pcm, bytes, rate, 1);
}

void audioLoop() {
}
