#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <string.h>

// Bench test only. Remove the tether and spool; secure the motor body first.
// A1/GPIO25 -> AIN1, A0/GPIO26 -> AIN2, NO pushbutton GPIO32 -> GND.
// Board in Arduino IDE: Adafruit Feather ESP32 V2.
constexpr uint8_t motorIn1 = 25;
constexpr uint8_t motorIn2 = 26;
constexpr uint8_t buttonPin = 32;
constexpr unsigned long debounceMs = 35;
constexpr unsigned long maxRunMs = 300;
constexpr unsigned long reversePauseMs = 250;
constexpr uint8_t radioChannel = 1;
constexpr uint8_t broadcastAddress[6] = {0xff,0xff,0xff,0xff,0xff,0xff};
constexpr char balloonCall[] = "BALLOON_V1_CALL";

bool radioReady = false;
bool rawPressed = false;
bool armed = false;
bool running = false;
bool nextForward = true;
unsigned long changedAt = 0;
unsigned long startedAt = 0;
unsigned long stoppedAt = 0;

void motorOff() {
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
}

bool startRadio() {
  if (!WiFi.mode(WIFI_STA)) return false;
  WiFi.setAutoReconnect(false);
  if (esp_wifi_set_channel(radioChannel, WIFI_SECOND_CHAN_NONE) != ESP_OK) return false;
  if (esp_now_init() != ESP_OK) return false;
  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, broadcastAddress, sizeof(broadcastAddress));
  peer.channel = radioChannel;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  return esp_now_add_peer(&peer) == ESP_OK;
}

void setup() {
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  motorOff();
  pinMode(buttonPin, INPUT_PULLUP);
  radioReady = startRadio();
  rawPressed = digitalRead(buttonPin) == LOW;
  changedAt = millis();
  stoppedAt = changedAt;
}

void loop() {
  const unsigned long now = millis();
  const bool pressed = digitalRead(buttonPin) == LOW;
  if (pressed != rawPressed) {
    rawPressed = pressed;
    changedAt = now;
  }

  // Cut power on the first release sample, or when the test pulse ends.
  if (running && (!pressed || now - startedAt >= maxRunMs)) {
    motorOff();
    running = false;
    stoppedAt = now;
  }

  if (now - changedAt < debounceMs) return;
  if (!pressed) {
    if (now - stoppedAt >= reversePauseMs) armed = true;
    return;
  }
  if (!armed || running) return;

  armed = false;  // A held or stuck switch cannot produce repeated pulses.
  digitalWrite(nextForward ? motorIn1 : motorIn2, HIGH);
  nextForward = !nextForward;
  startedAt = now;
  running = true;
  if (radioReady) {
    // Include the terminating NUL; the balloon uses the same message and channel.
    esp_now_send(broadcastAddress, reinterpret_cast<const uint8_t*>(balloonCall), sizeof(balloonCall));
  }
}
