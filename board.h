#pragma once
#include <Arduino.h>

bool epdInit();
void epdClear(uint8_t color);
void epdSetPixel(int x, int y, uint8_t color);
void epdDisplayCurrentBuffer();
void epdForceBatteryWarn(bool on);  // test: overlay even if battery ≥ 10 %
void epdSetMoreMemoriesHint(int extra);  // 0 = aus; sonst Pfeil unten rechts

bool pmuInit();
bool pmuReady();
int pmuBatteryPercent();  // -1 if unknown / no battery
bool pmuCharging();
bool pmuUsbPowered();  // VBUS in — USB, no deep sleep
float pmuBattVoltage();  // volts, 0 if unknown
void pmuAppendJson(String &j);  // /api/status: all AXP2101 battery fields
int pmuChargeMa();              // setpoint 100…1000, or -1
bool pmuSetChargeMa(int ma);    // exact AXP step
bool pmuEnableAudioRail();  // ALDO3 for ES8311, on demand
void pmuSleepRails();       // AXP sleep + unused rails; ALDO3 last
void shtc3Sleep();          // SHTC3 sleep command (NACK = absent)
float pmuTemperature();     // °C, NAN if unknown

bool bmpShowFromMemory(const uint8_t *data, size_t len);
bool bmpShowFromSd(const char *path);
/** Show 1–3 BMPs stacked vertically on the panel. */
bool bmpShowCompositeFromSd(const char *const *paths, int count);

bool sdInit();
void sdDeinit();  // unmount before PMIC sleep
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
bool rtcEpoch(time_t *out);  // PCF85063 → local epoch

void powerSleepCalOnWake();  // after rtcApply: ESP-timer ppm vs quartz

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
bool slideshowTimeOk();
void slideshowGetJson(String &out);
void slideshowAppendMemoryJson(String &out);
void slideshowAppendPotJson(String &out);
void slideshowInvalidateMemPreview();
void slideshowInvalidatePot();
bool slideshowSet(int mode, int intervalMin, int dailyHour, int dailyMin);
void slideshowForceNow();  // KEY / Jetzt wechseln: Zufall; bei mehreren fälligen Erinnerungen die nächste
void slideshowOnTimer();   // Intervall / Uhr: Erinnerungen zuerst (eine, alle 3 h die nächste), sonst Zufall
void slideshowHoldDue();   // BOOT-Wake: fälligen Wechsel nicht nachholen; KEY bleibt
void slideshowForgetMemoryCycle();  // Galerie/Studio-Anzeige: 3-Stunden-Runde aus
const char *hangValue();   // "portrait" | "landscape" — System, gilt für neue Bilder
bool hangSet(const char *v);
/** Remember which file is on the panel. */
void slideshowNoteShown(const String &files);
/** Forget that note when the file is deleted. */
void slideshowForgetShown(const String &file);
void slideshowDeckRemove(const String &file);
void slideshowDeckAdd(const String &file);
void slideshowDeckRefill();  // alle Zufallsbilder zurück in den Topf, neu gemischt
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
void powerCaptureWake();  // Tasten + Wake-Grund sofort nach Sleep (vor WLAN)
void powerOnBoot();  // after SD/EPD/slideshow — timer-wake may switch image
void powerLoop();
void powerSleepNow();     // auto: skipped if USB or stay-awake
void powerSleepForced();  // zzz — sleeps even if stay-awake
bool powerStayAwake();
void powerSetStayAwake(bool on);
void ledsAfterWake();  // Hold von Grün/Rot lösen
void ledsOff();        // beide aus (aktiv low)

void keyBegin();
void keyLoop();
void keyPrepareSleepWake();  // KEY+BOOT as ext1 wake (active low)

void tzBegin();
void tzApply();
String tzCity();
String tzPosix();
bool tzSet(const String &city, const String &posix);
void tzGetJson(String &out);
const char *tzCitiesJson();
bool ntpSyncToRtc();
void ntpHoldAfterManual();
void ntpLoop();
void ntpAppendJson(String &out);

void ntfyBegin();
void ntfyBatteryWatch();
String ntfyTopic();
String ntfyPrio();
bool ntfySet(const String &topic, const String &prio);
bool ntfySendTest();
bool ntfySendBatteryProbe();
bool ntfySendWake();

// Audio (ES8311 + speaker)
bool audioInit();
bool audioEnsureReady();  // lazy init on first speak
bool audioReady();
bool audioBusy();
void audioStop();
void audioPowerDown();  // stop, PA off, codec suspend (before ALDO3 cut)
uint8_t audioVolume();  // 0…100, default 80
bool audioSetVolume(uint8_t pct);
/** Play WAV from SD path (async). Returns false if missing/busy/bad. */
bool audioPlaySd(const char *path);
bool audioPlayClip(const char *clip);  // /sound/{clip}.wav, no-op if missing
void audioWaitIdle(uint32_t maxMs);
bool audioPlayBeep(uint32_t ms);
void audioLoop();
