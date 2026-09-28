#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <math.h>
#include <string.h>

// ======================================================
// OLED configuration
// PB6 = SCL, PB7 = SDA
// ======================================================
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS  0x3C
#define OLED_RESET    -1

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ======================================================
// Pin configuration - matches your diagram.json
// ======================================================
#define MQ2_PIN        PA0
#define DHT_PIN        PA1
#define PIR_PIN        PA2
#define MODE_PIN       PA3
#define ALERT_LED_PIN  PA4
#define NORMAL_LED_PIN PA5
#define LDR_PIN        PA6

#define DHT_TYPE       DHT22

DHT dht(DHT_PIN, DHT_TYPE);

// ======================================================
// Timing and threshold configuration
// ======================================================
const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long DEBOUNCE_TIME = 250;

// The code scales ADC input to a 0-4095-style range.
// You can tune these thresholds after checking Serial Monitor.
const int GAS_WARNING_LEVEL = 1500;
const int GAS_DANGER_LEVEL  = 2500;

const int DARK_LEVEL        = 1200;
const float TEMP_WARNING    = 35.0;
const float TEMP_DANGER     = 42.0;

// MODE button:
// AUTO: alerts appear on red LED for WATCH/DANGER/INTRUDER.
// QUIET: OLED still shows ML results but red LED stays off.
bool quietMode = false;

bool oldButtonState = HIGH;
unsigned long lastButtonPressTime = 0;
unsigned long lastSensorReadTime = 0;

// ======================================================
// ML classifier output
// ======================================================
struct MLResult {
  const char* label;
  float confidence;
  float safeScore;
  float watchScore;
  float dangerScore;
  float intruderScore;
  bool alertRequired;
};

// ======================================================
// Utility methods
// ======================================================
float clamp01(float value) {
  if (value < 0.0f) return 0.0f;
  if (value > 1.0f) return 1.0f;
  return value;
}

float normalize(float value, float minimum, float maximum) {
  if (maximum <= minimum) return 0.0f;
  return clamp01((value - minimum) / (maximum - minimum));
}

// Convert both 10-bit and 12-bit readings to a consistent 0-4095 range.
int scaleADCTo4095(int rawValue) {
  if (rawValue <= 1023) {
    return map(rawValue, 0, 1023, 0, 4095);
  }

  return constrain(rawValue, 0, 4095);
}

// ======================================================
// TinyML safety inference model
//
// Features:
// xTemp     = normalized temperature
// xHumidity = normalized humidity
// xGas      = normalized MQ-2 signal
// xDark     = normalized darkness calculated from LDR
// xMotion   = PIR result: 0 = no motion, 1 = motion
//
// Output classes:
// SAFE, WATCH, DANGER, INTRUDER
// ======================================================
MLResult runTinyML(
  float temperature,
  float humidity,
  int gasLevel,
  int lightLevel,
  bool motionDetected
) {
  MLResult result;

  // Input feature normalization.
  float xTemp = normalize(temperature, 15.0f, 50.0f);
  float xHumidity = normalize(humidity, 20.0f, 95.0f);
  float xGas = normalize((float)gasLevel, 0.0f, 4095.0f);

  /*
    With this Wokwi LDR circuit, a lower ADC reading is treated as darker.
    If your Serial Monitor shows the opposite behavior, use:
    float xDark = normalize(lightLevel, 0.0f, 4095.0f);
  */
  float xDark = 1.0f - normalize((float)lightLevel, 0.0f, 4095.0f);

  float xMotion = motionDetected ? 1.0f : 0.0f;

  // Normal temperature around 25 C produces a larger safe contribution.
  float xNormalTemperature =
    1.0f - clamp01(fabs(temperature - 25.0f) / 18.0f);

  // Compact one-layer neural/weighted classifier scores.
  result.safeScore =
    xNormalTemperature * 0.32f +
    (1.0f - xGas) * 0.30f +
    (1.0f - xDark) * 0.13f +
    (1.0f - xMotion) * 0.20f +
    (1.0f - xHumidity) * 0.05f;

  result.watchScore =
    xTemp * 0.23f +
    xGas * 0.28f +
    xDark * 0.20f +
    xHumidity * 0.12f +
    xMotion * 0.17f;

  result.dangerScore =
    xGas * 0.52f +
    xTemp * 0.30f +
    xDark * 0.10f +
    xHumidity * 0.08f;

  result.intruderScore =
    xMotion * 0.88f +
    xDark * 0.08f +
    xGas * 0.02f +
    xTemp * 0.02f;

  result.safeScore = clamp01(result.safeScore);
  result.watchScore = clamp01(result.watchScore);
  result.dangerScore = clamp01(result.dangerScore);
  result.intruderScore = clamp01(result.intruderScore);

  // Hard safety thresholds override ambiguous model scores.
  bool dangerousGas = gasLevel >= GAS_DANGER_LEVEL;
  bool dangerousTemperature = temperature >= TEMP_DANGER;

  if (motionDetected) {
    result.label = "INTRUDER";
    result.confidence = result.intruderScore;
    result.alertRequired = true;
    return result;
  }

  if (dangerousGas || dangerousTemperature || result.dangerScore >= 0.65f) {
    result.label = "DANGER";
    result.confidence = result.dangerScore;

    if (result.confidence < 0.70f) {
      result.confidence = 0.70f;
    }

    result.alertRequired = true;
    return result;
  }

  if (temperature >= TEMP_WARNING ||
      gasLevel >= GAS_WARNING_LEVEL ||
      lightLevel <= DARK_LEVEL ||
      result.watchScore >= 0.40f) {
    result.label = "WATCH";
    result.confidence = result.watchScore;
    result.alertRequired = true;
    return result;
  }

  result.label = "SAFE";
  result.confidence = result.safeScore;
  result.alertRequired = false;

  return result;
}

// ======================================================
// OLED display functions
// ======================================================
void showStartupScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);
  display.setCursor(13, 3);
  display.println("PROJECT 3");

  display.setTextSize(1);
  display.setCursor(8, 29);
  display.println("TinyML Safety AI");

  display.setCursor(5, 47);
  display.println("Loading sensors...");
  display.display();
}

void showDashboard(
  float temperature,
  float humidity,
  int gasLevel,
  int lightLevel,
  bool motionDetected,
  const MLResult& ml
) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  // Line 1: DHT22
  display.setCursor(0, 0);
  display.print("T:");
  display.print(temperature, 1);
  display.print("C H:");
  display.print(humidity, 0);
  display.println("%");

  // Line 2: MQ-2
  display.setCursor(0, 12);
  display.print("Gas:");
  display.print(gasLevel);

  if (gasLevel >= GAS_DANGER_LEVEL) {
    display.println(" HIGH");
  } else if (gasLevel >= GAS_WARNING_LEVEL) {
    display.println(" MID");
  } else {
    display.println(" OK");
  }

  // Line 3: LDR and PIR
  display.setCursor(0, 24);
  display.print("L:");
  display.print(lightLevel);
  display.print(lightLevel <= DARK_LEVEL ? " DARK" : " LIGHT");

  // Line 4: ML classification
  display.setCursor(0, 36);
  display.print("ML:");
  display.print(ml.label);
  display.print(" ");
  display.print(ml.confidence * 100.0f, 0);
  display.println("%");

  // Bottom line: mode and active status
  display.drawLine(0, 48, 127, 48, SSD1306_WHITE);
  display.setCursor(0, 53);

  if (quietMode) {
    display.print("QUIET ");
  } else {
    display.print("AUTO ");
  }

  if (motionDetected) {
    display.println("MOTION");
  } else if (ml.alertRequired) {
    display.println("ALERT");
  } else {
    display.println("NORMAL");
  }

  display.display();
}

// ======================================================
// LED output functions
// ======================================================
void updateLEDs(const MLResult& ml) {
  bool alertLEDOn = ml.alertRequired && !quietMode;
  bool normalLEDOn = !ml.alertRequired;

  digitalWrite(ALERT_LED_PIN, alertLEDOn ? HIGH : LOW);
  digitalWrite(NORMAL_LED_PIN, normalLEDOn ? HIGH : LOW);
}

// ======================================================
// MODE pushbutton control
// PA3 uses INPUT_PULLUP:
//
// Not pressed = HIGH
// Pressed = LOW
// ======================================================
void handleModeButton() {
  bool currentButtonState = digitalRead(MODE_PIN);

  if (oldButtonState == HIGH &&
      currentButtonState == LOW &&
      millis() - lastButtonPressTime >= DEBOUNCE_TIME) {

    quietMode = !quietMode;
    lastButtonPressTime = millis();

    Serial.print("Mode changed to: ");
    Serial.println(quietMode ? "QUIET" : "AUTO");
  }

  oldButtonState = currentButtonState;
}

// ======================================================
// Serial Monitor output
// ======================================================
void printReadings(
  float temperature,
  float humidity,
  int gasLevel,
  int lightLevel,
  bool motionDetected,
  const MLResult& ml
) {
  Serial.println("------------------------------------------------");

  Serial.print("Temperature: ");
  Serial.print(temperature, 1);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity, 1);
  Serial.println(" %");

  Serial.print("MQ-2 gas ADC: ");
  Serial.println(gasLevel);

  Serial.print("LDR light ADC: ");
  Serial.println(lightLevel);

  Serial.print("PIR motion: ");
  Serial.println(motionDetected ? "DETECTED" : "NO");

  Serial.print("TinyML classification: ");
  Serial.println(ml.label);

  Serial.print("Confidence: ");
  Serial.print(ml.confidence * 100.0f, 1);
  Serial.println("%");

  Serial.print("Scores [safe/watch/danger/intruder]: ");
  Serial.print(ml.safeScore, 2);
  Serial.print(" / ");
  Serial.print(ml.watchScore, 2);
  Serial.print(" / ");
  Serial.print(ml.dangerScore, 2);
  Serial.print(" / ");
  Serial.println(ml.intruderScore, 2);

  Serial.print("System mode: ");
  Serial.println(quietMode ? "QUIET" : "AUTO");

  Serial.print("Red ALERT LED: ");
  Serial.println((ml.alertRequired && !quietMode) ? "ON" : "OFF");

  Serial.print("Green NORMAL LED: ");
  Serial.println(ml.alertRequired ? "OFF" : "ON");
}

// ======================================================
// Setup
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  // OLED I2C uses PB6 (SCL) and PB7 (SDA).
  Wire.begin();

  pinMode(PIR_PIN, INPUT);
  pinMode(MODE_PIN, INPUT_PULLUP);

  pinMode(ALERT_LED_PIN, OUTPUT);
  pinMode(NORMAL_LED_PIN, OUTPUT);

  digitalWrite(ALERT_LED_PIN, LOW);
  digitalWrite(NORMAL_LED_PIN, LOW);

  dht.begin();

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("ERROR: SSD1306 OLED not detected.");

    while (true) {
      delay(100);
    }
  }

  showStartupScreen();
  delay(1800);

  Serial.println("==============================================");
  Serial.println("Project 3 - Blue Pill TinyML Monitor");
  Serial.println("Sensors: MQ-2, DHT22, PIR, LDR");
  Serial.println("Outputs: OLED, Red Alert LED, Green Normal LED");
  Serial.println("==============================================");
}

// ======================================================
// Main application loop
// ======================================================
void loop() {
  handleModeButton();

  if (millis() - lastSensorReadTime < SENSOR_INTERVAL) {
    return;
  }

  lastSensorReadTime = millis();

  // Read DHT22.
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read MQ-2 and LDR analog outputs.
  int gasRaw = analogRead(MQ2_PIN);
  int ldrRaw = analogRead(LDR_PIN);

  // Normalize potentially 10-bit readings to 0-4095.
  int gasLevel = scaleADCTo4095(gasRaw);
  int lightLevel = scaleADCTo4095(ldrRaw);

  // Read PIR output.
  bool motionDetected = digitalRead(PIR_PIN) == HIGH;

  // Prevent invalid temperature/humidity values from reaching the ML model.
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("WARNING: DHT22 read failed. Applying default values.");

    temperature = 25.0f;
    humidity = 50.0f;
  }

  // Run on-device TinyML inference.
  MLResult ml = runTinyML(
    temperature,
    humidity,
    gasLevel,
    lightLevel,
    motionDetected
  );

  // Update circuit outputs and screen.
  updateLEDs(ml);

  showDashboard(
    temperature,
    humidity,
    gasLevel,
    lightLevel,
    motionDetected,
    ml
  );

  printReadings(
    temperature,
    humidity,
    gasLevel,
    lightLevel,
    motionDetected,
    ml
  );
}