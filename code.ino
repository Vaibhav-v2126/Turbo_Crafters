
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// =================================================
// 1. WIFI + FIREBASE CONFIGURATION
// =================================================

const char* WIFI_SSID = "VR";
const char* WIFI_PASSWORD = "135792468";

const char* FIREBASE_URL =
  "https://dockbot-command-center-default-rtdb.asia-southeast1.firebasedatabase.app/dockbot/live.json";

// =================================================
// 2. LINE FOLLOWER MOTOR PINS
// =================================================

#define sensorNumber 8

#define left_motor_forward   32
#define left_motor_backward  33

#define right_motor_forward  26
#define right_motor_backward 27

#define left_motor_speed     14
#define right_motor_speed    13

// =================================================
// 3. IR SENSOR ARRAY
// =================================================

int sensorPins[sensorNumber] = {
  19, 18, 5, 17, 16, 4, 2, 15
};

int sensorADC[sensorNumber];

int WeightValue[sensorNumber] = {
  10, 20, 30, 40, 50, 60, 70, 80
};

int bitWeight[sensorNumber] = {
  1, 2, 4, 8, 16, 32, 64, 128
};

int sumOnSensor = 0;
int sensorWight = 0;
int bitSensor = 0;

// =================================================
// 4. TOP MODULE GPIO PINS
// =================================================

#define SENSOR_OUT 25
#define SENSOR_IN  34

#define TRIG_PIN 21
#define ECHO_PIN 22

#define MQ135_PIN 35

#define BUZZER_PIN 23
#define LED_PIN 12

// =================================================
// 5. SENSOR VARIABLES
// =================================================

long duration = 0;

float distance = 0;

int electrodeValue = 0;
int airQuality = 0;

const int SPILL_THRESHOLD = 500;
const float OBSTACLE_DISTANCE = 15.0;

bool spillDetected = false;
bool obstacleDetected = false;
bool safetyActive = false;

// =================================================
// 6. PID VARIABLES
// =================================================

float line_position = 45;

float error = 0;
float center_position = 45;

float derivative = 0;
float previous_error = 0;

int base_speed = 60;

float kp = 2.5;
float kd = 1.2;

// =================================================
// 7. MOTOR SETTINGS
// =================================================

const int MAX_PWM = 85;

const int TURN_SPEED = 60;

const int LOST_SPEED = 60;

// =================================================
// 8. SERIAL TIMING
// =================================================

unsigned long lastPrintTime = 0;

const unsigned long PRINT_INTERVAL = 250;

// =================================================
// 9. FIREBASE TELEMETRY
// =================================================

struct TelemetryData {
  int electrode;
  int airQuality;
  float distanceCm;

  bool spillDetected;
  bool obstacleDetected;
  bool emergencyStop;
};

TelemetryData telemetry;

portMUX_TYPE telemetryMux = portMUX_INITIALIZER_UNLOCKED;

// =================================================
// 10. UPDATE TELEMETRY BUFFER
// =================================================

void updateTelemetryBuffer() {

  portENTER_CRITICAL(&telemetryMux);

  telemetry.electrode = electrodeValue;
  telemetry.airQuality = airQuality;
  telemetry.distanceCm = distance;

  telemetry.spillDetected = spillDetected;
  telemetry.obstacleDetected = obstacleDetected;
  telemetry.emergencyStop = safetyActive;

  portEXIT_CRITICAL(&telemetryMux);
}

// =================================================
// 11. FIREBASE COMMUNICATION TASK
// =================================================

void firebaseTask(void *parameter) {

  unsigned long lastReconnectAttempt = 0;

  for (;;) {

    // Reconnect if Wi-Fi is disconnected

    if (WiFi.status() != WL_CONNECTED) {

      if (millis() - lastReconnectAttempt > 10000) {

        lastReconnectAttempt = millis();

        Serial.println("Firebase task: reconnecting WiFi");

        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }

      vTaskDelay(pdMS_TO_TICKS(2000));

      continue;
    }

    // Copy sensor values safely

    TelemetryData snapshot;

    portENTER_CRITICAL(&telemetryMux);

    snapshot = telemetry;

    portEXIT_CRITICAL(&telemetryMux);

    // Prepare robot status

    String robotStatus =
      snapshot.emergencyStop
      ? "EMERGENCY_STOP"
      : "PATROLLING";

    // Build Firebase JSON

    String json = "{";

    json += "\"status\":\"";
    json += robotStatus;
    json += "\"";

    json += ",\"spillDetected\":";
    json += snapshot.spillDetected ? "true" : "false";

    json += ",\"obstacleDetected\":";
    json += snapshot.obstacleDetected ? "true" : "false";

    json += ",\"electrode\":";
    json += String(snapshot.electrode);

    json += ",\"airQuality\":";
    json += String(snapshot.airQuality);

    json += ",\"distanceCm\":";
    json += String(snapshot.distanceCm, 1);

    // These are placeholders until checkpoint logic is added

    json += ",\"checkpoint\":\"NOT_CONFIGURED\"";
    json += ",\"zone\":\"UNKNOWN\"";

    // Battery is not measured by the current hardware

    json += ",\"battery\":null";

    json += ",\"uptimeMs\":";
    json += String(millis());

    json += "}";

    // HTTPS connection

    WiFiClientSecure client;

    // Demo configuration only.
    // Replace with certificate validation for deployment.

    client.setInsecure();

    HTTPClient http;

    http.setTimeout(1000);

    if (http.begin(client, FIREBASE_URL)) {

      http.addHeader("Content-Type", "application/json");

      // PATCH updates the live record

      int responseCode = http.sendRequest(
        "PATCH",
        (uint8_t*)json.c_str(),
        json.length()
      );

      Serial.print("Firebase HTTP response: ");
      Serial.println(responseCode);

      if (responseCode < 0) {
        Serial.println(http.errorToString(responseCode));
      }

      http.end();

    } else {

      Serial.println("Firebase connection failed");

    }

    // Upload every 2 seconds

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

// =================================================
// 12. SETUP
// =================================================

void setup() {

  Serial.begin(115200);

  // IR sensors

  for (int i = 0; i < sensorNumber; i++) {
    pinMode(sensorPins[i], INPUT_PULLUP);
  }

  // Motor pins

  pinMode(left_motor_forward, OUTPUT);
  pinMode(left_motor_backward, OUTPUT);

  pinMode(right_motor_forward, OUTPUT);
  pinMode(right_motor_backward, OUTPUT);

  pinMode(left_motor_speed, OUTPUT);
  pinMode(right_motor_speed, OUTPUT);

  // Electrode sensor

  pinMode(SENSOR_OUT, OUTPUT);
  digitalWrite(SENSOR_OUT, LOW);

  pinMode(SENSOR_IN, INPUT);

  // Ultrasonic sensor

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  digitalWrite(TRIG_PIN, LOW);

  // MQ135

  pinMode(MQ135_PIN, INPUT);

  // Buzzer and LED

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  // Initial motor stop

  emergencyStop();

  Serial.println();
  Serial.println("============================");
  Serial.println("   DOCKBOT PATROL SYSTEM");
  Serial.println("============================");

  Serial.println("Line follower: READY");
  Serial.println("Spill detection: READY");
  Serial.println("Obstacle detection: READY");
  Serial.println("MQ135: READY");

  // Initialize telemetry

  updateTelemetryBuffer();

  // Initialize Wi-Fi

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");

  // Start Firebase on a separate core

  xTaskCreatePinnedToCore(
    firebaseTask,
    "FirebaseTask",
    8192,
    NULL,
    1,
    NULL,
    0
  );

  Serial.println("Firebase task started");

  Serial.println("============================");
}

// =================================================
// 13. MAIN LOOP
// =================================================

void loop() {

  // FIRST: Always check safety sensors

  updateSafety();

  // SECOND: Stop if any danger is detected

  if (safetyActive) {

    emergencyStop();

    printStatus();

    return;
  }

  // THIRD: Run normal line follower

  PID_Controller(base_speed, kp, kd);

  printStatus();
}

// =================================================
// 14. SPILL + OBSTACLE MONITORING
// =================================================

void updateSafety() {

  // ELECTRODE SPILL SENSOR

  digitalWrite(SENSOR_OUT, HIGH);

  delay(10);

  electrodeValue = analogRead(SENSOR_IN);

  digitalWrite(SENSOR_OUT, LOW);

  spillDetected =
    (electrodeValue > SPILL_THRESHOLD);

  // ULTRASONIC SENSOR

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {

    distance = -1;

    obstacleDetected = false;

  }

  else {

    distance = duration * 0.0343 / 2.0;

    obstacleDetected =
      (distance < OBSTACLE_DISTANCE);
  }

  // MQ135 GAS SENSOR

  airQuality = analogRead(MQ135_PIN);

  // BUZZER CONTROL

  if (spillDetected) {

    digitalWrite(BUZZER_PIN, HIGH);

  }

  else {

    digitalWrite(BUZZER_PIN, LOW);

  }

  // LED CONTROL

  if (obstacleDetected) {

    digitalWrite(LED_PIN, HIGH);

  }

  else {

    digitalWrite(LED_PIN, LOW);

  }

  // OVERALL SAFETY CONDITION

  safetyActive =
    spillDetected || obstacleDetected;

  // Update Firebase telemetry buffer

  updateTelemetryBuffer();
}

// =================================================
// 15. EMERGENCY STOP
// =================================================

void emergencyStop() {

  analogWrite(left_motor_speed, 0);

  analogWrite(right_motor_speed, 0);

  digitalWrite(left_motor_forward, LOW);
  digitalWrite(left_motor_backward, LOW);

  digitalWrite(right_motor_forward, LOW);
  digitalWrite(right_motor_backward, LOW);
}

// =================================================
// 16. PRINT SYSTEM STATUS
// =================================================

void printStatus() {

  if (millis() - lastPrintTime < PRINT_INTERVAL) {
    return;
  }

  lastPrintTime = millis();

  Serial.println("----------------------------");

  Serial.print("Electrode: ");
  Serial.println(electrodeValue);

  Serial.print("Spill: ");
  Serial.println(
    spillDetected ? "DETECTED" : "SAFE"
  );

  Serial.print("Distance: ");

  if (distance < 0) {
    Serial.println("Out of range");
  }

  else {
    Serial.print(distance);
    Serial.println(" cm");
  }

  Serial.print("Obstacle: ");

  Serial.println(
    obstacleDetected ? "DETECTED" : "CLEAR"
  );

  Serial.print("MQ135: ");
  Serial.println(airQuality);

  Serial.print("WiFi: ");

  Serial.println(
    WiFi.status() == WL_CONNECTED
    ? "CONNECTED"
    : "DISCONNECTED"
  );

  if (spillDetected) {
    Serial.println("!!! LIQUID SPILL - EMERGENCY STOP !!!");
  }

  if (obstacleDetected) {
    Serial.println("!!! OBSTACLE - EMERGENCY STOP !!!");
  }

  if (!safetyActive) {
    Serial.println("DOCKBOT STATUS: PATROLLING");
  }
}

// =================================================
// 17. STABLE IR SENSOR READING
// =================================================

int readStable(int pin) {

  int count = 0;

  for (int i = 0; i < 11; i++) {

    count += digitalRead(pin);
  }

  if (count >= 7) {
    return 1;
  }

  else {
    return 0;
  }
}

// =================================================
// 18. READ BLACK LINE
// =================================================

void read_black_line() {

  sumOnSensor = 0;

  sensorWight = 0;

  bitSensor = 0;

  for (int i = 0; i < sensorNumber; i++) {

    sensorADC[i] = readStable(sensorPins[i]);

    sumOnSensor += sensorADC[i];

    sensorWight +=
      sensorADC[i] * WeightValue[i];

    bitSensor +=
      sensorADC[i] * bitWeight[i];
  }
}

// =================================================
// 19. MOTOR CONTROL
// =================================================

void motor(int LPWM, int RPWM) {

  // Safety override

  if (safetyActive) {

    emergencyStop();

    return;
  }

  LPWM = constrain(LPWM, -MAX_PWM, MAX_PWM);

  RPWM = constrain(RPWM, -MAX_PWM, MAX_PWM);

  // LEFT MOTOR

  if (LPWM > 0) {

    digitalWrite(left_motor_forward, HIGH);
    digitalWrite(left_motor_backward, LOW);

  }

  else if (LPWM < 0) {

    digitalWrite(left_motor_forward, LOW);
    digitalWrite(left_motor_backward, HIGH);

  }

  else {

    digitalWrite(left_motor_forward, LOW);
    digitalWrite(left_motor_backward, LOW);
  }

  // RIGHT MOTOR

  if (RPWM > 0) {

    digitalWrite(right_motor_forward, HIGH);
    digitalWrite(right_motor_backward, LOW);

  }

  else if (RPWM < 0) {

    digitalWrite(right_motor_forward, LOW);
    digitalWrite(right_motor_backward, HIGH);

  }

  else {

    digitalWrite(right_motor_forward, LOW);
    digitalWrite(right_motor_backward, LOW);
  }

  analogWrite(left_motor_speed, abs(LPWM));

  analogWrite(right_motor_speed, abs(RPWM));
}

// =================================================
// 20. SAFE WAIT
// =================================================

bool safeWait(unsigned long waitTime) {

  unsigned long startTime = millis();

  while (millis() - startTime < waitTime) {

    updateSafety();

    if (safetyActive) {

      emergencyStop();

      return false;
    }

    delay(5);
  }

  return true;
}

// =================================================
// 21. RIGHT TURN
// =================================================

void turnRight() {

  Serial.println("90 DEGREE RIGHT TURN");

  motor(60, 60);

  if (!safeWait(60)) {
    return;
  }

  unsigned long startTime = millis();

  while (millis() - startTime < 1200) {

    updateSafety();

    if (safetyActive) {

      emergencyStop();

      return;
    }

    read_black_line();

    motor(TURN_SPEED, -TURN_SPEED);

    if (millis() - startTime > 120) {

      if (bitRead(bitSensor, 3) ||
          bitRead(bitSensor, 4)) {

        break;
      }
    }

    delay(5);
  }

  previous_error = 0;
}

// =================================================
// 22. LEFT TURN
// =================================================

void turnLeft() {

  Serial.println("90 DEGREE LEFT TURN");

  motor(60, 60);

  if (!safeWait(60)) {
    return;
  }

  unsigned long startTime = millis();

  while (millis() - startTime < 1200) {

    updateSafety();

    if (safetyActive) {

      emergencyStop();

      return;
    }

    read_black_line();

    motor(-TURN_SPEED, TURN_SPEED);

    if (millis() - startTime > 120) {

      if (bitRead(bitSensor, 3) ||
          bitRead(bitSensor, 4)) {

        break;
      }
    }

    delay(5);
  }

  previous_error = 0;
}

// =================================================
// 23. PID LINE FOLLOWING CONTROLLER
// =================================================

void PID_Controller(int base_speed, float p, float d) {

  // Read line sensors

  read_black_line();

  // RIGHT 90 DEGREE DETECTION

  if (bitSensor == 0b11110000 ||
      bitSensor == 0b11100000) {

    turnRight();

    return;
  }

  // LEFT 90 DEGREE DETECTION

  if (bitSensor == 0b00001111 ||
      bitSensor == 0b00000111) {

    turnLeft();

    return;
  }

  // LINE LOST

  if (sumOnSensor == 0) {

    if (previous_error > 0) {

      motor(LOST_SPEED, -LOST_SPEED);

    }

    else {

      motor(-LOST_SPEED, LOST_SPEED);
    }

    return;
  }

  // LINE POSITION

  line_position =
    (float)sensorWight / (float)sumOnSensor;

  // PID CALCULATION

  error = line_position - center_position;

  derivative = error - previous_error;

  float correction =
    (error * p) + (derivative * d);

  correction = constrain(correction, -55.0, 55.0);

  // MOTOR SPEED CORRECTION

  int right_motor_correction =
    base_speed - correction;

  int left_motor_correction =
    base_speed + correction;

  left_motor_correction =
    constrain(left_motor_correction, 0, MAX_PWM);

  right_motor_correction =
    constrain(right_motor_correction, 0, MAX_PWM);

  // MOVE ROBOT

  motor(left_motor_correction, right_motor_correction);

  previous_error = error;
}
