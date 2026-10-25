#include <Arduino.h>
#include <VL53L0X.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "config.h"
#include "web_page.h"

// ============================================================================
// HARDWARE INSTANCES & GLOBAL STATE
// ============================================================================
VL53L0X sensors[5];
WebServer server(80);

const uint8_t xshutPins[5] = {PIN_XSHUT_LEFT, PIN_XSHUT_FRONT_LEFT,
                              PIN_XSHUT_CENTER, PIN_XSHUT_FRONT_RIGHT,
                              PIN_XSHUT_RIGHT};

const uint8_t i2cAddresses[5] = {I2C_ADDR_LEFT, I2C_ADDR_FRONT_LEFT,
                                 I2C_ADDR_CENTER, I2C_ADDR_FRONT_RIGHT,
                                 I2C_ADDR_RIGHT};

const char *sensorNames[5] = {"Left", "Front-Left", "Center", "Front-Right",
                              "Right"};
bool sensorActive[5] = {false, false, false, false, false};
uint16_t dist[5] = {8190, 8190, 8190, 8190, 8190}; // Millimeters

// Operating States
enum BotState {
  STATE_STOPPED,
  STATE_FORWARD,
  STATE_CENTERING,
  STATE_TURN_LEFT,
  STATE_TURN_RIGHT,
  STATE_UTURN,
  STATE_EXIT_REACHED
};

BotState currentState = STATE_STOPPED;
String stateString = "READY AT ENTRY";

// Tunable Navigation Parameters
int baseSpeed = DEFAULT_BASE_SPEED;
int turnSpeed = DEFAULT_TURN_SPEED;
int frontWallDist = DEFAULT_FRONT_WALL_DIST;
int desiredSideDist = DEFAULT_DESIRED_SIDE_DIST;
int wallDetectMax = DEFAULT_WALL_DETECT_MAX;
float kp = DEFAULT_KP;
float kd = DEFAULT_KD;

// Motor PWM variables
int currentPwmA = 0;
int currentPwmB = 0;
float lastError = 0.0;
unsigned long lastSensorReadTime = 0;

// Entry & Exit run tracking variables
unsigned long runStartTime = 0;
int exitConfirmCounter = 0;

// ============================================================================
// MOTOR CONTROL FUNCTIONS (TB6612FNG with ESP32 LEDC PWM)
// ============================================================================
void initMotors() {
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);
  pinMode(PIN_BIN2, OUTPUT);

  // Setup ESP32 LEDC PWM channels
  ledcSetup(PWM_CHAN_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHAN_RIGHT, PWM_FREQ, PWM_RESOLUTION);

  ledcAttachPin(PIN_PWMA, PWM_CHAN_LEFT);
  ledcAttachPin(PIN_PWMB, PWM_CHAN_RIGHT);

  // Stop initially
  digitalWrite(PIN_AIN1, LOW);
  digitalWrite(PIN_AIN2, LOW);
  digitalWrite(PIN_BIN1, LOW);
  digitalWrite(PIN_BIN2, LOW);
  ledcWrite(PWM_CHAN_LEFT, 0);
  ledcWrite(PWM_CHAN_RIGHT, 0);
}

void setMotors(int leftSpeed, int rightSpeed) {
  currentPwmA = abs(leftSpeed);
  currentPwmB = abs(rightSpeed);

  // Left Motor Direction
  if (leftSpeed > 0) {
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);
  } else if (leftSpeed < 0) {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, HIGH);
  } else {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
  }

  // Right Motor Direction
  if (rightSpeed > 0) {
    digitalWrite(PIN_BIN1, HIGH);
    digitalWrite(PIN_BIN2, LOW);
  } else if (rightSpeed < 0) {
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, HIGH);
  } else {
    digitalWrite(PIN_BIN1, LOW);
    digitalWrite(PIN_BIN2, LOW);
  }

  ledcWrite(PWM_CHAN_LEFT, constrain(currentPwmA, 0, 255));
  ledcWrite(PWM_CHAN_RIGHT, constrain(currentPwmB, 0, 255));
}

void stopMotors() { setMotors(0, 0); }

// ============================================================================
// PHYSICAL PUSHBUTTON INTERFACE
// ============================================================================
void initPushbuttons() {
  pinMode(PIN_BUTTON_START, INPUT_PULLUP);
  pinMode(PIN_BUTTON_STOP, INPUT_PULLUP);
  Serial.println("[Buttons] Pushbuttons initialized (Start: GPIO 13, Stop: GPIO 4 with INPUT_PULLUP)");
}

void checkPushbuttons() {
  static int lastStartState = HIGH;
  static int lastStopState = HIGH;
  static unsigned long lastStartDebounce = 0;
  static unsigned long lastStopDebounce = 0;

  int readingStart = digitalRead(PIN_BUTTON_START);
  int readingStop = digitalRead(PIN_BUTTON_STOP);

  // STOP button (GPIO 4) - Immediate Emergency Stop
  if (readingStop == LOW && lastStopState == HIGH && (millis() - lastStopDebounce > 150)) {
    lastStopDebounce = millis();
    stopMotors();
    currentState = STATE_STOPPED;
    stateString = "STOPPED (BUTTON)";
    Serial.println("\n[BUTTON] >>> STOP BUTTON PRESSED! Motors stopped immediately. <<<");
  }
  lastStopState = readingStop;

  // START button (GPIO 13) - Starts Entry-to-Exit run
  if (readingStart == LOW && lastStartState == HIGH && (millis() - lastStartDebounce > 250)) {
    lastStartDebounce = millis();
    if (currentState == STATE_STOPPED || currentState == STATE_EXIT_REACHED) {
      Serial.println("\n[BUTTON] >>> START BUTTON PRESSED! Beginning Entry-to-Exit Maze Run... <<<");
      stopMotors();
      delay(300); // Hand release delay
      runStartTime = millis();
      exitConfirmCounter = 0;
      lastError = 0;
      currentState = STATE_FORWARD;
      stateString = "ENTRY RUN";
    }
  }
  lastStartState = readingStart;
}

// ============================================================================
// SEQUENTIAL I2C INITIALIZATION FOR 5x VL53L0X TOF SENSORS
// ============================================================================
void initToFSensors() {
  Serial.println("\n[ToF] Initializing 5x Time-of-Flight Sensors...");

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000); // 100 kHz Standard I2C
  Wire.setTimeOut(50);                          // 50ms I2C timeout prevents MCU hang

  // Step 1: Put all sensors in hardware reset / shutdown
  for (int i = 0; i < 5; i++) {
    pinMode(xshutPins[i], OUTPUT);
    digitalWrite(xshutPins[i], LOW);
  }
  delay(50);

  // Step 2: Power up and assign unique I2C address one by one
  for (int i = 0; i < 5; i++) {
    digitalWrite(xshutPins[i], HIGH);
    delay(20);

    sensors[i].setTimeout(25);
    if (sensors[i].init()) {
      sensors[i].setAddress(i2cAddresses[i]);
      sensors[i].setTimeout(25);
      sensors[i].startContinuous(30); // 30ms continuous measurement period
      sensorActive[i] = true;
      Serial.printf("  [OK] %s ToF initialized at address 0x%02X\n",
                    sensorNames[i], i2cAddresses[i]);
    } else {
      sensorActive[i] = false;
      Serial.printf("  [FAIL] %s ToF failed at default 0x29!\n",
                    sensorNames[i]);
    }
    delay(15);
  }
}

void updateSensorReadings() {
  for (int i = 0; i < 5; i++) {
    if (sensorActive[i]) {
      uint16_t d = sensors[i].readRangeContinuousMillimeters();
      if (!sensors[i].timeoutOccurred() && d < 4000) {
        dist[i] = d;
      } else {
        dist[i] = 8190; // Out of range flag
      }
    } else {
      dist[i] = 8190;
    }
  }
}

// ============================================================================
// WEB SERVER & JSON API HANDLERS (TELEMETRY & PARAMETER TUNING)
// ============================================================================
void setupWebServer() {
  // Serve Main Telemetry Web Page
  server.on("/", HTTP_GET,
            []() { server.send_P(200, "text/html", INDEX_HTML); });

  // Fast JSON Telemetry Endpoint
  server.on("/api/status", HTTP_GET, []() {
    char json[256];
    snprintf(json, sizeof(json),
             "{\"l\":%u,\"fl\":%u,\"c\":%u,\"fr\":%u,\"r\":%u,\"state\":\"%s\","
             "\"pwma\":%d,\"pwmb\":%d}",
             dist[0], dist[1], dist[2], dist[3], dist[4], stateString.c_str(),
             currentPwmA, currentPwmB);
    server.send(200, "application/json", json);
  });

  // Robot Tuning Endpoint (Wireless start/stop disabled)
  server.on("/api/control", HTTP_GET, []() {
    if (server.hasArg("action")) {
      String act = server.arg("action");

      if (act == "start" || act == "stop" || act == "manual") {
        // Wireless start/stop is explicitly disabled for safety
        server.send(200, "application/json",
                    "{\"status\":\"disabled\",\"message\":\"Wireless start/stop is disabled. Use physical pushbuttons on bot (Start: GPIO 13, Stop: GPIO 4)\"}");
        return;
      } else if (act == "tune") {
        String param = server.arg("param");
        int val = server.arg("val").toInt();
        if (param == "speed")
          baseSpeed = constrain(val, 50, 255);
        if (param == "front")
          frontWallDist = constrain(val, 50, 400);
        if (param == "side")
          desiredSideDist = constrain(val, 40, 300);
      }
    }
    server.send(200, "text/plain", "OK");
  });

  server.begin();
  Serial.println("[Web] HTTP Telemetry Server running on port 80");
}

// ============================================================================
// MAZE SOLVER ENTRY-TO-EXIT AUTONOMOUS NAVIGATION ALGORITHM
// ============================================================================
void runAutonomousMazeSolver() {
  uint16_t dLeft = dist[0];
  uint16_t dFrontLeft = dist[1];
  uint16_t dCenter = dist[2];
  uint16_t dFrontRight = dist[3];
  uint16_t dRight = dist[4];

  // ==========================================================================
  // EXIT RUN COMPLETION DETECTION
  // Check if the robot has arrived at the Exit (walls disappear completely)
  // Only valid after minimum run time in the maze to prevent false triggers at entry.
  // ==========================================================================
  if (millis() - runStartTime > MIN_RUN_TIME_BEFORE_EXIT_MS) {
    bool leftOpen = (dLeft >= wallDetectMax);
    bool rightOpen = (dRight >= wallDetectMax);
    bool centerOpen = (dCenter >= wallDetectMax);
    bool frontLeftOpen = (dFrontLeft >= wallDetectMax);
    bool frontRightOpen = (dFrontRight >= wallDetectMax);

    // If completely clear on all forward & side sensors (outside the maze corridor)
    if (leftOpen && rightOpen && centerOpen && frontLeftOpen && frontRightOpen) {
      exitConfirmCounter++;
      if (exitConfirmCounter >= EXIT_CONFIRM_SAMPLES) {
        // Exit reached! Drive forward slightly to clear the maze threshold
        stateString = "EXIT REACHED!";
        setMotors(baseSpeed, baseSpeed);
        delay(EXIT_POST_DRIVE_MS);
        stopMotors();

        currentState = STATE_EXIT_REACHED;
        Serial.println("\n==============================================");
        Serial.println("  [MAZE] >>> EXIT REACHED! MAZE SOLVED! <<<   ");
        Serial.println("  Motors stopped. Press START for new run.    ");
        Serial.println("==============================================\n");
        return;
      }
    } else {
      if (exitConfirmCounter > 0) exitConfirmCounter--;
    }
  }

  // ==========================================================================
  // FRONT OBSTACLE & INTERSECTION TURNING
  // ==========================================================================
  bool frontBlocked = (dCenter < frontWallDist) ||
                      (dFrontLeft < (frontWallDist * 0.85)) ||
                      (dFrontRight < (frontWallDist * 0.85));

  if (frontBlocked) {
    // Stop momentarily to prevent collision
    stopMotors();
    delay(20);

    // Clearances for side decisions
    bool leftClear = (dLeft > (wallDetectMax * 0.7));
    bool rightClear = (dRight > (wallDetectMax * 0.7));

    if (leftClear && (!rightClear || dLeft >= dRight)) {
      // Left path available (prioritize left for consistent wall-following)
      currentState = STATE_TURN_LEFT;
      stateString = "TURNING LEFT";
      setMotors(-turnSpeed, turnSpeed); // Spin turn left
      delay(180);
    } else if (rightClear) {
      // Right path available
      currentState = STATE_TURN_RIGHT;
      stateString = "TURNING RIGHT";
      setMotors(turnSpeed, -turnSpeed); // Spin turn right
      delay(180);
    } else {
      // Dead end: both left and right blocked
      currentState = STATE_UTURN;
      stateString = "U-TURN ESCAPE";
      setMotors(-turnSpeed, turnSpeed); // 180° Spin escape
      delay(360);
    }
    lastError = 0;
    return;
  }

  // ==========================================================================
  // CORRIDOR CENTERING & WALL FOLLOWING (PD CONTROLLER)
  // ==========================================================================
  bool hasLeftWall = (dLeft < wallDetectMax);
  bool hasRightWall = (dRight < wallDetectMax);

  float error = 0.0;

  if (hasLeftWall && hasRightWall) {
    // Both walls present: Center between corridor walls
    currentState = STATE_CENTERING;
    stateString = "CORRIDOR CENTERING";
    error = ((float)dLeft - (float)dRight);
  } else if (hasLeftWall) {
    // Follow Left Wall
    currentState = STATE_FORWARD;
    stateString = "FOLLOWING LEFT";
    error = ((float)dLeft - (float)desiredSideDist);
  } else if (hasRightWall) {
    // Follow Right Wall
    currentState = STATE_FORWARD;
    stateString = "FOLLOWING RIGHT";
    error = ((float)desiredSideDist - (float)dRight);
  } else {
    // Open area before exit: Drive straight
    currentState = STATE_FORWARD;
    stateString = "DRIVING STRAIGHT";
    error = 0.0;
  }

  // PD Steering Controller
  float derivative = error - lastError;
  lastError = error;

  float correction = (kp * error) + (kd * derivative);

  int leftPwm = constrain(baseSpeed - (int)correction, 50, 255);
  int rightPwm = constrain(baseSpeed + (int)correction, 50, 255);

  setMotors(leftPwm, rightPwm);
}

// ============================================================================
// SETUP & MAIN LOOP
// ============================================================================
void setup() {
  // Disable hardware brownout detector to prevent reset on USB power sag
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(100);

  Serial.println("\n==============================================");
  Serial.println("  MAZE SOLVER BOT - ENTRY & EXIT RUN FIRMWARE ");
  Serial.println("  Start: GPIO 13 | Stop: GPIO 4               ");
  Serial.println("  5x VL53L0X ToF Array + TB6612FNG Driver     ");
  Serial.println("==============================================");

  // 1. INITIALIZE HARDWARE PUSHBUTTONS FIRST
  initPushbuttons();

  // 2. START WI-FI TELEMETRY
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);
  WiFi.setTxPower(WIFI_POWER_11dBm);

  WiFi.softAP(AP_SSID, AP_PASSWORD);
  IPAddress apIP = WiFi.softAPIP();

  Serial.println("\n[Wi-Fi] Telemetry Access Point started:");
  Serial.printf("  SSID: %s\n", AP_SSID);
  Serial.printf("  Password: %s\n", AP_PASSWORD);
  Serial.printf("  Live Radar HUD: http://%s/\n", apIP.toString().c_str());

  // Connect to Station Wi-Fi (moye) in background
  if (strlen(STA_SSID) > 0) {
    Serial.printf("[Wi-Fi] Connecting to Wi-Fi SSID '%s' ...\n", STA_SSID);
    WiFi.begin(STA_SSID, STA_PASSWORD);
  }

  // 3. START WEB SERVER (Telemetry only)
  setupWebServer();

  // 4. INITIALIZE MOTORS & SENSORS
  initMotors();
  initToFSensors();

  Serial.println("\n[System] Ready! Place bot at Entry and press START button (GPIO 13).");
}

void loop() {
  // Always check physical pushbuttons with highest priority
  checkPushbuttons();

  // Handle incoming web client requests (Telemetry)
  server.handleClient();

  // Read 5 ToF sensors every 35ms
  if (millis() - lastSensorReadTime >= 35) {
    lastSensorReadTime = millis();
    updateSensorReadings();

    // Run autonomous navigation if active
    if (currentState != STATE_STOPPED && currentState != STATE_EXIT_REACHED) {
      runAutonomousMazeSolver();
    }
  }

  // Periodic serial monitor readout every 1.5 seconds
  static unsigned long lastLog = 0;
  if (millis() - lastLog >= 1500) {
    lastLog = millis();
    Serial.printf("[Sensors] L:%4u | FL:%4u | C:%4u | FR:%4u | R:%4u | State: %s\n",
                  dist[0], dist[1], dist[2], dist[3], dist[4], stateString.c_str());
  }
}
