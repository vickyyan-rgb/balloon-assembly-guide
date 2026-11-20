#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "page.h"

constexpr uint8_t in1 = A1, in2 = A0;
constexpr uint8_t physicalStop = 32;  // Existing normally-open wrist button to GND
constexpr bool clockwiseUsesIn1 = true;  // Calibrate facing the output shaft
WebServer server(80);
SemaphoreHandle_t lock;
int direction = 0;
uint32_t endsAt = 0, stoppedAt = 0;

void stopLocked() {
  ledcWrite(in1, 0);
  ledcWrite(in2, 0);
  if (direction) stoppedAt = millis();
  direction = 0;
}

// Independent of HTTP parsing, including a slow or disconnected browser.
void motorWatch(void*) {
  for (;;) {
    xSemaphoreTake(lock, portMAX_DELAY);
    if (direction && (int32_t(millis() - endsAt) >= 0 ||
        digitalRead(physicalStop) == LOW || WiFi.softAPgetStationNum() == 0)) stopLocked();
    xSemaphoreGive(lock);
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

int numberArg(const char* key, int maximum) {
  String s = server.arg(key);
  if (!s.length() || s.length() > 5) return -1;
  for (char c : s) if (c < '0' || c > '9') return -1;
  int v = s.toInt();
  return v <= maximum ? v : -1;
}

void moveMotor() {
  int pwm = numberArg("speed", 100), ms = numberArg("ms", 25000);
  String which = server.arg("dir");
  if (pwm < 1 || ms < 0 || (which != "down" && which != "up")) {
    server.send(400, "text/plain", "Invalid settings"); return;
  }
  xSemaphoreTake(lock, portMAX_DELAY);
  bool ready = !direction && millis()-stoppedAt >= 250 && digitalRead(physicalStop) == HIGH;
  if (ready && ms > 0) {
    direction = which == "down" ? 1 : -1;
    endsAt = millis() + ms;
    ledcWrite(((direction == -1) == clockwiseUsesIn1) ? in1 : in2, pwm*255/100);
  }
  xSemaphoreGive(lock);
  server.send(ready ? 200 : 409, "text/plain", ready ? "OK" : "Busy or stop held");
}

void setup() {
  Serial.begin(115200);
  pinMode(physicalStop, INPUT_PULLUP);
  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  if (!ledcAttach(in1, 20000, 8) || !ledcAttach(in2, 20000, 8)) return;
  lock = xSemaphoreCreateMutex();
  if (!lock) { Serial.println("Motor lock failed"); return; }
  IPAddress address(192, 168, 4, 1);
  if (!WiFi.mode(WIFI_AP) ||
      !WiFi.softAPConfig(address, address, IPAddress(255, 255, 255, 0)) ||
      !WiFi.softAP("Balloon-Motor", "PebbleRun27")) {
    Serial.println("Balloon-Motor Wi-Fi startup failed");
    return;
  }
  if (xTaskCreate(motorWatch, "motor-stop", 4096, nullptr, 3, nullptr) != pdPASS) return;
  server.on("/", HTTP_GET, [](){server.send_P(200,"text/html; charset=utf-8",page);});
  server.on("/move", HTTP_POST, moveMotor);
  server.on("/stop", HTTP_POST, [](){
    xSemaphoreTake(lock, portMAX_DELAY); stopLocked(); xSemaphoreGive(lock);
    server.send(200,"text/plain","OK");
  });
  server.on("/state", HTTP_GET, [](){
    xSemaphoreTake(lock, portMAX_DELAY); int state=direction; xSemaphoreGive(lock);
    server.sendHeader("Cache-Control","no-store");
    server.send(200,"application/json",String(state));
  });
  server.begin();
  Serial.print("Balloon-Motor ready: http://"); Serial.println(WiFi.softAPIP());
}
void loop() { server.handleClient(); delay(2); }

