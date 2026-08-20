#pragma once
#include <Arduino.h>

bool epdInit();
void epdClear(uint8_t color);
void epdSetPixel(int x, int y, uint8_t color);
void epdDisplayCurrentBuffer();
void epdForceBatteryWarn(bool on);  // test: overlay even if battery ≥ 10 %

bool pmuInit();
bool pmuReady();
int pmuBatteryPercent();  // -1 if unknown / no battery
bool pmuCharging();
bool pmuUsbPowered();  // VBUS in — USB, no deep sleep
float pmuBattVoltage();  // volts, 0 if unknown
bool pmuEnableAudioRail();  // ALDO3 for ES8311, on demand

bool bmpShowFromMemory(const uint8_t *data, size_t len);
bool bmpShowFromSd(const char *path);
/** Show 1–3 BMPs stacked vertically on the panel. */
bool bmpShowCompositeFromSd(const char *const *paths, int count);

bool sdInit();
bool sdOk();

struct PicCand {
  String file;
  bool anniversarySoon;
  bool noDates;
};

bool rtcInit();
bool rtcReady();
bool rtcGet(struct tm *out);
bool rtcSet(const struct tm *in);
bool rtcApplyToSystem();
bool rtcSyncFromSystem();

// WiFi / web
bool wifiBringUp();   // true = STA, false = AP
bool wifiIsAp();
void wifiLoop();
bool wifiSaveCreds(const String &ssid, const String &pass);
bool wifiSaveApPass(const String &apPass);
bool wifiClearCreds();
String wifiApPass();
void wifiGetCreds(String &ssid, String &pass, String &apPass);
void webBegin();
void webLoop();
void webAfterSdReady();  // load/build gallery list cache without blocking HTTP
void webSetOfflineStudio(bool on);

void serialProtocolLoop();
void serialProtocolBegin();

void slideshowBegin();
void slideshowLoop();
void slideshowSetTimeOk(bool ok);
void slideshowGetJson(String &out);
bool slideshowSet(int mode, int intervalMin, int dailyHour, int dailyMin);
void slideshowForceNow();  // KEY / Jetzt wechseln: nur Zufall
void slideshowOnTimer();   // Intervall / Uhr: Erinnerungen zuerst, sonst Zufall
const char *hangValue();   // "portrait" | "landscape" — System, gilt für neue Bilder
bool hangSet(const char *v);
/** Remember which file(s) are on the panel (comma-separated if composite). */
void slideshowNoteShown(const String &files);
/** Forget that note when the file is deleted. */
void slideshowForgetShown(const String &file);
void slideshowDeckRemove(const String &file);
void slideshowDeckAdd(const String &file);
String slideshowLastShown();
bool slideshowReshowLast();
/** Deep sleep allowed: daily mode, or interval ≥ 10 min. */
bool slideshowSleepAllowed();
/** Seconds until next auto switch; -1 if unknown / not applicable. */
int64_t slideshowSecondsUntilNext();

void epdPanelSleep();  // panel deep-sleep command (after refresh already done)

void powerNoteActivity();
void powerNoteBusy(bool on);
bool powerBusy();
bool powerClientHere();
void powerOnBoot();  // after SD/EPD/slideshow — timer-wake may switch image
void powerLoop();
void powerSleepNow();  // zzz — now, next picture or BOOT wakes

void keyBegin();
void keyLoop();
void keyPrepareSleepWake();  // KEY as ext1 wake (active low)

void tzBegin();
void tzApply();
String tzCity();
String tzPosix();
bool tzSet(const String &city, const String &posix);
void tzGetJson(String &out);
const char *tzCitiesJson();
bool ntpSyncToRtc();

void ntfyBegin();
void ntfyBatteryWatch();
String ntfyTopic();
String ntfyPrio();
bool ntfySet(const String &topic, const String &prio);
bool ntfySendTest();
bool ntfySendBatteryProbe();

// Audio (ES8311 + speaker)
bool audioInit();
bool audioEnsureReady();  // lazy init on first speak
bool audioReady();
bool audioBusy();
void audioStop();
/** Play WAV from SD path (async). Returns false if missing/busy/bad. */
bool audioPlaySd(const char *path);
bool audioPlayClip(const char *clip);  // /sound/{clip}.wav, no-op if missing
void audioWaitIdle(uint32_t maxMs);
bool audioPlayBeep(uint32_t ms);
void audioLoop();
