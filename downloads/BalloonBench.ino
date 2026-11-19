// XIAO ESP32S3, WS2812B x16 and MAX98357A. Arduino-ESP32 3.3.x.
// The ring and the XIAO share one protected single-cell LiPo supply.
// Boot test: dim amber for 2 s, off, one soft note, amber and a second note.
// After these checks the ring goes dark. A call from WristBench gives a short
// green light and one soft chime. No router, microphone or pairing is needed.
#include <Arduino.h>
#include <ESP_I2S.h>
#include <esp32-hal-rmt.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_now.h>
#include <atomic>
#include <math.h>

constexpr int kRingPin = 4;  // XIAO D3
constexpr int kBclk = 7;     // D8
constexpr int kWord = 8;     // D9
constexpr int kAudioOut = 9; // D10
constexpr size_t kPixels = 16;
constexpr int kSampleRate = 16000;
I2SClass audio;
rmt_data_t pulse[kPixels * 24];
bool ringReady = false;
bool audioReady = false;
std::atomic<bool> callPending{false};
constexpr char kCall[] = "BALLOON_V1_CALL";

void receiveCall(const esp_now_recv_info_t *, const uint8_t *data, int length) {
  if (data && length == sizeof(kCall) && memcmp(data, kCall, sizeof(kCall)) == 0) {
    callPending.store(true);
  }
}

bool listenForWrist() {
  return WiFi.mode(WIFI_STA) &&
         esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE) == ESP_OK &&
         esp_now_init() == ESP_OK &&
         esp_now_register_recv_cb(receiveCall) == ESP_OK;
}

// WS2812B receives green, red, blue, then holds the result during a low interval.
bool ringColor(uint8_t red, uint8_t green, uint8_t blue) {
  const uint8_t grb[] = {green, red, blue};
  size_t symbol = 0;
  for (size_t led = 0; led < kPixels; ++led) {
    for (uint8_t channel : grb) {
      for (int bit = 7; bit >= 0; --bit) {
        const bool one = channel & (1U << bit);
        rmt_data_t &p = pulse[symbol++];
        p.level0 = 1;
        p.duration0 = one ? 16 : 7;
        p.level1 = 0;
        p.duration1 = one ? 9 : 18;  // 25 ticks at 20 MHz = 1.25 us
      }
    }
  }
  bool sent = rmtWrite(kRingPin, pulse, symbol, 100);
  delayMicroseconds(350);
  return sent;
}

bool playNote(int pitch) {
  constexpr int frames = kSampleRate / 4;
  constexpr int fadeFrames = kSampleRate / 100;
  constexpr int framesPerBlock = 80;
  int16_t stereo[framesPerBlock * 2];
  // The last 100 ms are silent so the short note clears the DMA queue.
  for (int start = 0; start < frames + kSampleRate / 10; start += framesPerBlock) {
    for (int i = 0; i < framesPerBlock; ++i) {
      const int position = start + i;
      int16_t value = 0;
      if (position < frames) {
        const int ramp = min(fadeFrames, min(position, frames - 1 - position));
        value = static_cast<int16_t>(
            1200.0f * ramp / fadeFrames * sinf(2.0f * PI * pitch * position / kSampleRate));
      }
      stereo[i * 2] = value;
      stereo[i * 2 + 1] = value;
    }
    if (audio.write(reinterpret_cast<const uint8_t *>(stereo), sizeof(stereo)) != sizeof(stereo)) {
      return false;
    }
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  ringReady = rmtInit(kRingPin, RMT_TX_MODE, RMT_MEM_NUM_BLOCKS_1, 20000000);
  if (ringReady) {
    ringColor(0, 0, 0);
    ringColor(12, 4, 0);
    delay(2000);
    ringColor(0, 0, 0);
  } else {
    Serial.println("Ring output did not start.");
  }

  delay(600);
  audio.setPins(kBclk, kWord, kAudioOut);
  audioReady = audio.begin(I2S_MODE_STD, kSampleRate, I2S_DATA_BIT_WIDTH_16BIT,
                           I2S_SLOT_MODE_STEREO);
  if (audioReady) {
    if (!playNote(440)) Serial.println("First audio write failed.");
  } else {
    Serial.println("I2S output did not start.");
  }
  delay(600);
  if (ringReady) ringColor(12, 4, 0);
  if (audioReady) {
    if (!playNote(660)) Serial.println("Second audio write failed.");
  }
  delay(750);
  if (ringReady) ringColor(0, 0, 0);
  Serial.println(listenForWrist() ? "Ready for wrist calls." : "Wireless reception did not start.");
}

void loop() {
  if (callPending.exchange(false)) {
    if (ringReady) ringColor(0, 12, 3);
    if (audioReady) playNote(550);
    delay(500);
    if (ringReady) ringColor(0, 0, 0);
  }
  delay(10);
}
