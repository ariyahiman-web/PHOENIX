#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>

// ========== WiFi AP ==========
const char* apSsid     = "ESP32_Car";
const char* apPassword = "12345678";

WebServer server(80);

// ========== Pins ==========
#define IN1 14
#define IN2 27
#define IN3 26
#define IN4 25
#define ENA_PIN 33
#define ENB_PIN 32

#define BUZZ_PIN  23
#define TRIG_PIN  4
#define ECHO_PIN  5
#define SERVO_PIN 15
#define LDR_PIN   34

const uint8_t LED_PINS[3] = {22, 19, 21};
const uint8_t BTN_PINS[3] = {16, 17, 18};

// ========== PWM ==========
const int PWM_FREQ = 1000;
const int PWM_RES  = 8;
const int CH_A      = 0;   // timer 0 (shared with CH_B, same freq)
const int CH_B      = 1;
const int CH_BUZZER = 2;   // timer 1
const int CH_SERVO  = 4;   // timer 2 (50 Hz, 16-bit)

#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  inline void pwmAttach(int pin, int ch, int freq, int res) { ledcAttach(pin, freq, res); }
  inline void pwmWrite(int pin, int ch, uint32_t duty)      { ledcWrite(pin, duty); }
  inline void pwmTone(int pin, int ch, uint32_t f)          { ledcWriteTone(pin, f); }
#else
  inline void pwmAttach(int pin, int ch, int freq, int res) { ledcSetup(ch, freq, res); ledcAttachPin(pin, ch); }
  inline void pwmWrite(int pin, int ch, uint32_t duty)      { ledcWrite(ch, duty); }
  inline void pwmTone(int pin, int ch, uint32_t f)          { ledcWriteTone(ch, f); }
#endif

// ========== Settings ==========
const int LDR_ON_THRESHOLD  = 250;   // LEDs on below this
const int LDR_OFF_THRESHOLD = 300;   // LEDs off above this (hysteresis)

uint8_t manualSpeed = 230;
uint8_t autoSpeed   = 200;

const unsigned long BACK_TIME        = 600;
const unsigned long TURN_TIME        = 700;
const float         OBSTACLE_DIST_CM = 20.0;
const unsigned long SONAR_INTERVAL   = 60;
const unsigned long CMD_TIMEOUT      = 500;   // failsafe

// ========== State ==========
bool forwardCmd = false, backCmd = false, leftCmd = false, rightCmd = false;
bool obstacleMode = false;
unsigned long lastCmdMs = 0;

bool ledState[3] = {false, false, false};
bool ldrActive = false;

float distanceCm = -1.0;
unsigned long lastSonarTime = 0;

enum AutoState { AUTO_IDLE, AUTO_FORWARD, AUTO_BACKWARD, AUTO_TURN };
AutoState autoState = AUTO_IDLE;
unsigned long autoStateStart = 0;
bool turnLeftNext = true;

// ========== HTML ==========
const char index_html[] PROGMEM = R"=====(<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8" />
  <title>ESP32 Web Car</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; text-align: center; background: #111; color: #eee; }
    h1 { margin-top: 10px; }
    .btn { display: inline-block; padding: 18px 26px; margin: 8px; font-size: 18px; border-radius: 10px; border: none; cursor: pointer; min-width: 110px; background: #333; color: #fff; touch-action: none; user-select: none; }
    .btn:active { background: #555; }
    .grid { margin-top: 25px; }
    .row { margin: 6px 0; }
    .mode-btn-on { background: #1b5e20; }
    .mode-btn-off { background: #b71c1c; }
  </style>
</head>
<body>
  <h1>ESP32 Web Controlled Car</h1>
  <p>Hold the direction buttons to move. Toggle Obstacle Mode for auto driving.</p>

  <div class="grid">
    <div class="row"><button class="btn" data-dir="f">Forward</button></div>
    <div class="row">
      <button class="btn" data-dir="l">Left</button>
      <button class="btn" data-dir="s">Stop</button>
      <button class="btn" data-dir="r">Right</button>
    </div>
    <div class="row"><button class="btn" data-dir="b">Backward</button></div>
  </div>

  <h2>Obstacle Avoiding Mode</h2>
  <div class="row">
    <button class="btn mode-btn-on" onclick="setMode(1)">Mode ON</button>
    <button class="btn mode-btn-off" onclick="setMode(0)">Mode OFF</button>
  </div>

  <script>
    function sendCmd(dir, state) {
      fetch('/cmd', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'dir=' + dir + '&state=' + state
      }).catch(e => console.log(e));
    }
    function setMode(auto) {
      fetch('/mode', {
        method: 'POST',
        headers: {'Content-Type': 'application/x-www-form-urlencoded'},
        body: 'auto=' + auto
      }).catch(e => console.log(e));
    }
    document.querySelectorAll('button[data-dir]').forEach(btn => {
      let timer = null;
      btn.addEventListener('pointerdown', event => {
        event.preventDefault();
        btn.setPointerCapture(event.pointerId);
        sendCmd(btn.dataset.dir, 1);
        if (timer) clearInterval(timer);
        timer = setInterval(() => sendCmd(btn.dataset.dir, 1), 150); // heartbeat
      });
      function release(event) {
        if (timer) { clearInterval(timer); timer = null; }
        if (btn.hasPointerCapture(event.pointerId)) {
          btn.releasePointerCapture(event.pointerId);
        }
        sendCmd(btn.dataset.dir, 0);
      }
      btn.addEventListener('pointerup', release);
      btn.addEventListener('pointercancel', release);
    });
  </script>
</body>
</html>)=====";

// ========== Motor helpers ==========
void setMotorSpeed(uint8_t l, uint8_t r) {
  pwmWrite(ENA_PIN, CH_A, l);
  pwmWrite(ENB_PIN, CH_B, r);
}
void driveStop() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  setMotorSpeed(0, 0);
}
void driveForward(uint8_t s) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setMotorSpeed(s, s);
}
void driveBackward(uint8_t s) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  setMotorSpeed(s, s);
}
void driveLeft(uint8_t s) {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  setMotorSpeed(s, s);
}
void driveRight(uint8_t s) {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  setMotorSpeed(s, s);
}

// ========== Servo (manual LEDC, 50 Hz, 16-bit) ==========
void servoWriteAngle(int ang) {
  ang = constrain(ang, 0, 180);
  uint32_t us   = 500 + (uint32_t)ang * (2400 - 500) / 180;
  uint32_t duty = us * 65535UL / 20000UL;
  pwmWrite(SERVO_PIN, CH_SERVO, duty);
}

// Non-blocking 0..180..0 sweep
void updateServoSweep() {
  static unsigned long last = 0;
  static int ang = 0, dir = 1;
  unsigned long now = millis();
  if (now - last < 60) return;
  last = now;
  servoWriteAngle(ang);
  ang += dir;
  if (ang >= 135) { ang = 135; dir = -1; }
  if (ang <= 45)   { ang = 45;   dir = 1;  }
}

// ========== Sonar ==========
float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 20000UL);
  if (duration == 0) return -1.0;
  return (duration * 0.0343f) / 2.0f;
}

void updateSonar() {
  unsigned long now = millis();
  if (now - lastSonarTime < SONAR_INTERVAL) return;
  lastSonarTime = now;
  distanceCm = readDistanceCm();
  pwmTone(BUZZ_PIN, CH_BUZZER, (distanceCm > 0 && distanceCm < 5.0) ? 2500 : 0);
}

// ========== Drive logic ==========
void handleManualMovement() {
  // failsafe: no heartbeat -> stop
  if ((forwardCmd || backCmd || leftCmd || rightCmd) && millis() - lastCmdMs > CMD_TIMEOUT) {
    forwardCmd = backCmd = leftCmd = rightCmd = false;
  }
  if (forwardCmd)       driveForward(manualSpeed);
  else if (backCmd)     driveBackward(manualSpeed);
  else if (leftCmd)     driveLeft(manualSpeed);
  else if (rightCmd)    driveRight(manualSpeed);
  else                  driveStop();
}

void handleAutoObstacle() {
  unsigned long now = millis();
  switch (autoState) {
    case AUTO_IDLE:
      driveStop();
      autoState = AUTO_FORWARD;
      break;
    case AUTO_FORWARD:
      if (distanceCm > 0 && distanceCm < OBSTACLE_DIST_CM) {
        driveStop();
        autoState = AUTO_BACKWARD;
        autoStateStart = now;
      } else {
        driveForward(autoSpeed);
      }
      break;
    case AUTO_BACKWARD:
      if (now - autoStateStart >= BACK_TIME) {
        driveStop();
        autoState = AUTO_TURN;
        autoStateStart = now;
      } else {
        driveBackward(autoSpeed);
      }
      break;
    case AUTO_TURN:
      if (now - autoStateStart >= TURN_TIME) {
        driveStop();
        turnLeftNext = !turnLeftNext;
        autoState = AUTO_FORWARD;
      } else if (turnLeftNext) {
        driveLeft(autoSpeed);
      } else {
        driveRight(autoSpeed);
      }
      break;
  }
}

// ========== Buttons / LEDs ==========
void updateButtons() {
  static bool raw[3]    = {HIGH, HIGH, HIGH};
  static bool stable[3] = {HIGH, HIGH, HIGH};
  static unsigned long changeT[3] = {0, 0, 0};
  unsigned long now = millis();
  for (int i = 0; i < 3; i++) {
    bool r = digitalRead(BTN_PINS[i]);
    if (r != raw[i]) { raw[i] = r; changeT[i] = now; }
    if (now - changeT[i] > 30 && r != stable[i]) {
      stable[i] = r;
      if (r == LOW) ledState[i] = !ledState[i];   // toggle once per press
    }
  }
}

void updateLdrAndLeds() {
  int v = analogRead(LDR_PIN);

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 500) {
    lastPrint = millis();
    Serial.print("Live LDR Reading: ");
    Serial.println(v);
  }

  if (!ldrActive && v < LDR_ON_THRESHOLD)  ldrActive = true;
  if (ldrActive  && v > LDR_OFF_THRESHOLD) ldrActive = false;

  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PINS[i], (ldrActive || ledState[i]) ? HIGH : LOW);
  }
}

// ========== Web handlers ==========
void handleRoot() { server.send_P(200, "text/html", index_html); }

void handleCmd() {
  if (!server.hasArg("dir") || !server.hasArg("state")) {
    server.send(400, "text/plain", "Missing params");
    return;
  }
  String dir = server.arg("dir");
  bool pressed = server.arg("state").toInt() != 0;
  if (!obstacleMode) {
    if (pressed) lastCmdMs = millis();
    if (dir == "f")      { forwardCmd = pressed; if (pressed) backCmd = leftCmd = rightCmd = false; }
    else if (dir == "b") { backCmd    = pressed; if (pressed) forwardCmd = leftCmd = rightCmd = false; }
    else if (dir == "l") { leftCmd    = pressed; if (pressed) forwardCmd = backCmd = rightCmd = false; }
    else if (dir == "r") { rightCmd   = pressed; if (pressed) forwardCmd = backCmd = leftCmd = false; }
    else if (dir == "s") { forwardCmd = backCmd = leftCmd = rightCmd = false; driveStop(); }
  }
  server.send(200, "text/plain", "OK");
}

void handleMode() {
  if (!server.hasArg("auto")) {
    server.send(400, "text/plain", "Missing auto param");
    return;
  }
  if (server.arg("auto").toInt() == 1) {
    obstacleMode = true;
    forwardCmd = backCmd = leftCmd = rightCmd = false;
    autoState = AUTO_FORWARD;
    autoStateStart = millis();
  } else {
    obstacleMode = false;
    autoState = AUTO_IDLE;
    driveStop();
  }
  server.send(200, "text/plain", "Mode set");
}

void handleNotFound() { server.send(404, "text/plain", "Not found"); }

// ========== Setup ==========
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  for (int i = 0; i < 3; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    digitalWrite(LED_PINS[i], LOW);
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }

  pwmAttach(ENA_PIN,   CH_A,      PWM_FREQ, PWM_RES);
  pwmAttach(ENB_PIN,   CH_B,      PWM_FREQ, PWM_RES);
  pwmAttach(BUZZ_PIN,  CH_BUZZER, 2000,     8);
  pwmAttach(SERVO_PIN, CH_SERVO,  50,       16);
  pwmWrite(BUZZ_PIN, CH_BUZZER, 0);
  servoWriteAngle(0);

  driveStop();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204); });
  server.on("/cmd", HTTP_ANY, handleCmd);
  server.on("/mode", HTTP_ANY, handleMode);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("System Ready");
}

// ========== Loop ==========
void loop() {
  server.handleClient();

  updateSonar();

  if (obstacleMode) handleAutoObstacle();
  else              handleManualMovement();

  updateButtons();
  updateLdrAndLeds();
  updateServoSweep();
}