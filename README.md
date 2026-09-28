```
```

````
# 🏠 Home Monitoring & Learning Supervision System

### Intelligent IoT-Based Home Monitoring, Environmental Sensing & Embedded Machine Learning Platform

<p align="center">

![STM32](https://img.shields.io/badge/MCU-STM32-03234B?style=for-the-badge&logo=stmicroelectronics)
![Embedded C](https://img.shields.io/badge/Embedded-C-00599C?style=for-the-badge&logo=c)
![Python](https://img.shields.io/badge/Python-3.x-3776AB?style=for-the-badge&logo=python)
![Machine Learning](https://img.shields.io/badge/Machine%20Learning-Scikit--Learn-F7931E?style=for-the-badge&logo=scikit-learn)
![IoT](https://img.shields.io/badge/IoT-Sensor%20Monitoring-00A98F?style=for-the-badge)
![Wokwi](https://img.shields.io/badge/Simulation-Wokwi-6B4FBB?style=for-the-badge)
![GitHub](https://img.shields.io/badge/Repository-GitHub-181717?style=for-the-badge&logo=github)

</p>

---

# 📌 Table of Contents

- [Project Overview](#-project-overview)
- [Problem Statement](#-problem-statement)
- [Proposed Solution](#-proposed-solution)
- [Objectives](#-objectives)
- [Key Features](#-key-features)
- [System Architecture](#-system-architecture)
- [System Workflow](#-system-workflow)
- [Hardware Components](#-hardware-components)
- [Complete Hardware Connections](#-complete-hardware-connections)
- [STM32 Pin Configuration](#-stm32-pin-configuration)
- [Sensor Details](#-sensor-details)
- [OLED Display](#-oled-display)
- [LED Connections](#-led-connections)
- [Push Button](#-push-button)
- [ST-Link Programming](#-st-link-programming)
- [Power Supply](#-power-supply)
- [Machine Learning](#-machine-learning)
- [Dataset](#-dataset)
- [Data Preprocessing](#-data-preprocessing)
- [K-Means Clustering](#-k-means-clustering)
- [Decision Tree Classification](#-decision-tree-classification)
- [K-Means vs Decision Tree](#-k-means-vs-decision-tree)
- [ML-to-C Conversion](#-ml-to-c-conversion)
- [Embedded ML Architecture](#-embedded-ml-architecture)
- [STM32 Firmware](#-stm32-firmware)
- [OLED Output](#-oled-output)
- [Wokwi Simulation](#-wokwi-simulation)
- [Software Requirements](#-software-requirements)
- [Installation](#-installation)
- [Running the ML Models](#-running-the-ml-models)
- [Project Structure](#-project-structure)
- [Testing](#-testing)
- [Troubleshooting](#-troubleshooting)
- [Advantages](#-advantages)
- [Limitations](#-limitations)
- [Future Scope](#-future-scope)
- [Learning Outcomes](#-learning-outcomes)
- [Project Outcome](#-project-outcome)
- [Author](#-author)
- [License](#-license)

---

# 🚀 Project Overview

The **Home Monitoring & Learning Supervision System** is an intelligent embedded IoT platform designed to monitor environmental conditions and human activity using multiple sensors connected to an STM32 microcontroller.

The system collects real-time sensor data including:

- Temperature
- Humidity
- Light intensity
- LPG/gas level
- Smoke level
- Carbon monoxide
- Motion

The collected information is processed by the STM32 and displayed on an **SSD1306 OLED display**.

Machine Learning is integrated into the system to move beyond simple threshold-based monitoring.

Two Machine Learning approaches are used:

1. **K-Means Clustering** for discovering patterns in IoT telemetry data.
2. **Decision Tree Classification** for classifying sensor conditions into predefined home/environment states.

The trained Decision Tree is converted into lightweight **Embedded C decision logic** and deployed on the STM32.

This allows the system to perform ML inference locally without requiring Python, a cloud server, or a large Machine Learning framework during runtime.

---

# 🎯 Problem Statement

Conventional monitoring systems generally use independent threshold conditions.

For example:

```text
Temperature > Threshold
        ↓
Temperature Alert
````

```
```

```
Smoke > Threshold
        ↓
Smoke Alert
```

```
```

```
Motion = HIGH
        ↓
Motion Detected
```

Although this approach is simple, it does not consider relationships between multiple sensor parameters.

For example:

```
```

```
Low Light
+
Motion Detected
+
Normal Temperature
```

may represent an occupied room during a low-light condition.

Similarly:

```
```

```
High Temperature
+
High Smoke
+
High LPG
```

may indicate an abnormal environmental condition.

Therefore, this project combines multiple sensor inputs and Machine Learning to provide a more comprehensive classification of the overall environment.

---

# 💡 Proposed Solution

The proposed system consists of four major layers:

```
```

```
┌───────────────────────────────────────┐
│             SENSOR LAYER              │
│ Temperature | Humidity | Light        │
│ LPG | Smoke | CO | Motion             │
└───────────────────┬───────────────────┘
                    │
                    ▼
┌───────────────────────────────────────┐
│          STM32 PROCESSING LAYER       │
│ ADC | GPIO | I²C | Sensor Processing  │
│ Embedded ML Inference                 │
└───────────────────┬───────────────────┘
                    │
                    ▼
┌───────────────────────────────────────┐
│        INTELLIGENCE / ML LAYER        │
│ K-Means + Decision Tree               │
└───────────────────┬───────────────────┘
                    │
                    ▼
┌───────────────────────────────────────┐
│         OUTPUT / DISPLAY LAYER        │
│ SSD1306 OLED | LEDs | Alerts          │
└───────────────────────────────────────┘
```

---

# 🎯 Objectives

The primary objectives are:

-  Monitor home environmental conditions in real time. 
-  Interface multiple sensors with an STM32. 
-  Collect IoT telemetry data. 
-  Analyze multi-dimensional sensor data. 
-  Discover sensor patterns using K-Means. 
-  Classify home/environment states using a Decision Tree. 
-  Convert ML decision rules into Embedded C. 
-  Run ML inference directly on STM32. 
-  Display sensor values and classification results. 
-  Build a low-cost intelligent monitoring platform. 
-  Demonstrate the integration of IoT, Embedded Systems and Machine Learning. 

---

# ✨ Key Features

-  🌡️ Temperature monitoring 
-  💧 Humidity monitoring 
-  💡 Light monitoring 
-  🔥 LPG/gas monitoring 
-  🚨 Smoke monitoring 
-  ☠️ Carbon monoxide monitoring 
-  🕵️ Motion detection 
-  🖥️ SSD1306 OLED display 
-  🤖 Machine Learning classification 
-  🔵 K-Means clustering 
-  🌳 Decision Tree classification 
-  ⚡ Embedded ML inference 
-  🔌 STM32-based implementation 
-  🧪 Wokwi simulation 
-  📊 IoT telemetry analysis 
-  💾 Local processing 
-  🔄 Real-time monitoring 
-  🔧 ST-Link debugging/programming 

---

# 🏗️ System Architecture

```
```

```
                         ┌───────────────────────┐
                         │      ENVIRONMENT      │
                         └───────────┬───────────┘
                                     │
                  ┌──────────────────┼──────────────────┐
                  │                  │                  │
                  ▼                  ▼                  ▼
             ┌─────────┐       ┌─────────┐       ┌─────────┐
             │  DHT11  │       │   LDR   │       │   PIR   │
             │ Temp/Hum │       │  Light  │       │ Motion  │
             └────┬────┘       └────┬────┘       └────┬────┘
                  │                  │                  │
                  └──────────────────┼──────────────────┘
                                     │
                  ┌──────────────────┼──────────────────┐
                  │                  │                  │
                  ▼                  ▼                  ▼
             ┌─────────┐       ┌─────────┐       ┌─────────┐
             │ MQ Gas  │       │  Smoke  │       │   CO    │
             │ Sensor  │       │ Sensor  │       │ Sensor  │
             └────┬────┘       └────┬────┘       └────┬────┘
                  │                  │                  │
                  └──────────────────┼──────────────────┘
                                     │
                                     ▼
                          ┌─────────────────────┐
                          │        STM32        │
                          │                     │
                          │ GPIO                │
                          │ ADC                 │
                          │ I²C                 │
                          │ Sensor Processing   │
                          │ ML Inference        │
                          └──────────┬──────────┘
                                     │
                   ┌─────────────────┼─────────────────┐
                   │                 │                 │
                   ▼                 ▼                 ▼
              ┌─────────┐       ┌─────────┐       ┌─────────┐
              │ SSD1306 │       │  LEDs   │       │ Button  │
              │  OLED   │       │ Status  │       │  Input  │
              └─────────┘       └─────────┘       └─────────┘
```

---

# 🔄 System Workflow

The complete system operates as follows:

```
```

```
1. Power ON
      ↓
2. STM32 Initialization
      ↓
3. Initialize GPIO
      ↓
4. Initialize ADC
      ↓
5. Initialize I²C
      ↓
6. Initialize OLED
      ↓
7. Read Temperature
      ↓
8. Read Humidity
      ↓
9. Read Light
      ↓
10. Read LPG/Gas
      ↓
11. Read Smoke
      ↓
12. Read CO
      ↓
13. Read PIR Motion
      ↓
14. Process Sensor Values
      ↓
15. Run ML Classification
      ↓
16. Determine Home State
      ↓
17. Display Results
      ↓
18. Update LEDs / Alerts
      ↓
19. Repeat
```

---

# 🔧 Hardware Components

| Component          | Quantity    | Purpose                   |
| ------------------ | ----------- | ------------------------- |
| STM32 Blue Pill    | 1           | Main controller           |
| DHT11/DHT22        | 1           | Temperature + humidity    |
| LDR Module         | 1           | Light detection           |
| MQ Gas Sensor      | 1           | LPG/gas detection         |
| Smoke Sensor       | 1           | Smoke detection           |
| CO Sensor          | 1           | Carbon monoxide detection |
| PIR Sensor         | 1           | Motion detection          |
| SSD1306 OLED 0.96" | 1           | Display                   |
| LED                | 2           | Status indication         |
| Push Button        | 1           | User input                |
| ST-Link            | 1           | Programming/debugging     |
| Resistors          | As required | Pull-up/current limiting  |
| Breadboard         | 1           | Prototyping               |
| Jumper Wires       | As required | Connections               |

---

# 🔌 Complete Hardware Connections

> **Important:** The exact GPIO pins can be changed according to the STM32CubeIDE configuration. The table below represents the project's recommended logical pin assignment. If your physical circuit uses different pins, update this section and the firmware accordingly.

---

# 1️⃣ DHT11 Temperature & Humidity Sensor

### DHT11 Pins

| DHT11 Pin | Connection |
| --------- | ---------- |
| VCC       | 3.3V       |
| DATA      | STM32 GPIO |
| GND       | GND        |

### Recommended STM32 Connection

```
```

```
DHT11 VCC
   │
   └────── 3.3V

DHT11 DATA
   │
   └────── STM32 GPIO

DHT11 GND
   │
   └────── GND
```

### Recommended Pin

```
```

```
DHT11 DATA → PA1
```

A pull-up resistor may be required on the DATA line depending on the DHT11 module.

---

# 2️⃣ LDR Sensor

The LDR module can provide:

-  Analog output `AO` 
-  Digital output `DO` 

For Machine Learning, the **analog output is preferred** because it provides more information than a binary digital output.

### Connection

| LDR Pin | STM32         |
| ------- | ------------- |
| VCC     | 3.3V          |
| GND     | GND           |
| AO      | ADC Pin       |
| DO      | Optional GPIO |

### Recommended Connection

```
```

```
LDR VCC → 3.3V
LDR GND → GND
LDR AO  → PA0 / ADC
```

### Important

If using `AO`:

```
```

```
Light Level
    ↓
ADC
    ↓
Numerical Sensor Value
    ↓
ML Model
```

If using only `DO`:

```
```

```
Light
 ↓
Threshold
 ↓
0 / 1
```

For ML, analog `AO` is generally more useful.

---

# 3️⃣ MQ Gas / LPG Sensor

Typical MQ modules contain:

```
```

```
VCC
GND
AO
DO
```

### Recommended Connection

| MQ Pin | STM32              |
| ------ | ------------------ |
| VCC    | Appropriate supply |
| GND    | GND                |
| AO     | ADC                |
| DO     | Optional GPIO      |

Example:

```
```

```
MQ AO → PA2 / ADC
```

### Important Safety Note

Many MQ sensor modules are designed around a 5V supply and their analog output may approach the module supply voltage.

The STM32 ADC input must remain within the STM32 pin's permitted voltage range.

**Do not connect an unverified 5V analog output directly to a 3.3V-only STM32 ADC input.**

Use an appropriate voltage divider or signal-conditioning circuit when required.

---

# 4️⃣ Smoke Sensor

If a separate smoke sensor is used:

```
```

```
Smoke Sensor VCC → Appropriate Supply
Smoke Sensor GND → GND
Smoke Sensor AO  → STM32 ADC
```

Recommended example:

```
```

```
Smoke AO → PA3 / ADC
```

Again, verify the sensor output voltage before connecting it to the STM32 ADC.

---

# 5️⃣ CO Sensor

For a CO sensor module:

| CO Sensor | STM32              |
| --------- | ------------------ |
| VCC       | Appropriate supply |
| GND       | GND                |
| AO        | ADC                |
| DO        | Optional GPIO      |

Example:

```
```

```
CO AO → PA4 / ADC
```

The analog output is preferred when the ML model uses a continuous CO measurement.

---

# 6️⃣ PIR Motion Sensor

The PIR sensor normally provides a digital output.

### Connection

| PIR Pin | STM32                          |
| ------- | ------------------------------ |
| VCC     | 5V or 3.3V depending on module |
| GND     | GND                            |
| OUT     | GPIO                           |

Example:

```
```

```
PIR OUT → PB0
```

### Output

```
```

```
LOW  → No Motion
HIGH → Motion Detected
```

The PIR output voltage must be compatible with the STM32 input.

---

# 7️⃣ SSD1306 OLED

The SSD1306 OLED communicates using I²C.

Typical pins:

```
```

```
VCC
GND
SCL
SDA
```

### Recommended Connection

| OLED | STM32   |
| ---- | ------- |
| VCC  | 3.3V    |
| GND  | GND     |
| SCL  | I²C SCL |
| SDA  | I²C SDA |

Example STM32 I²C1 mapping:

```
```

```
OLED SCL → PB6
OLED SDA → PB7
```

### Connection Diagram

```
```

```
STM32                 SSD1306
─────                 ───────
3.3V  ──────────────→ VCC
GND   ──────────────→ GND
PB6   ──────────────→ SCL
PB7   ──────────────→ SDA
```

---

# 8️⃣ LED Connections

Two LEDs can be used to indicate system status.

Example:

```
```

```
LED 1 → Normal
LED 2 → Alert
```

### Recommended Connections

```
```

```
STM32 PA5
   │
   ▼
Resistor
   │
   ▼
LED 1
   │
   ▼
GND
```

```
```

```
STM32 PA6
   │
   ▼
Resistor
   │
   ▼
LED 2
   │
   ▼
GND
```

A suitable current-limiting resistor should be used.

---

# 9️⃣ Push Button

The push button can be used for:

-  Changing OLED pages 
-  Manual reset 
-  Selecting display mode 
-  Triggering a manual status update 

Example:

```
```

```
Button
  │
  ├──── GPIO PB1
  │
  └──── GND
```

Configure the GPIO with an appropriate pull-up or pull-down arrangement.

---

# 🔟 ST-Link Connection

ST-Link is used to program and debug the STM32.

### ST-Link to STM32

| ST-Link | STM32                                |
| ------- | ------------------------------------ |
| SWDIO   | PA13                                 |
| SWCLK   | PA14                                 |
| GND     | GND                                  |
| 3.3V    | 3.3V reference/supply as appropriate |

### Diagram

```
```

```
ST-Link                 STM32
───────                 ─────
SWDIO  ───────────────→ PA13
SWCLK  ───────────────→ PA14
GND    ───────────────→ GND
3.3V   ───────────────→ 3.3V
```

> Avoid connecting incompatible voltages. Always verify the ST-Link and target board supply configuration.

---

# 📌 Complete Connection Table

| Device    | Pin   | STM32 Pin | Interface |
| --------- | ----- | --------- | --------- |
| DHT11     | DATA  | PA1       | GPIO      |
| LDR       | AO    | PA0       | ADC       |
| MQ Gas    | AO    | PA2       | ADC       |
| Smoke     | AO    | PA3       | ADC       |
| CO Sensor | AO    | PA4       | ADC       |
| PIR       | OUT   | PB0       | GPIO      |
| OLED      | SCL   | PB6       | I²C1      |
| OLED      | SDA   | PB7       | I²C1      |
| LED 1     | Anode | PA5       | GPIO      |
| LED 2     | Anode | PA6       | GPIO      |
| Button    | OUT   | PB1       | GPIO      |
| ST-Link   | SWDIO | PA13      | SWD       |
| ST-Link   | SWCLK | PA14      | SWD       |

---

# ⚠️ Voltage and Power Considerations

The STM32 GPIO and ADC pins have specific voltage limits.

Before connecting any external sensor:

1.  Check the sensor operating voltage. 
2.  Check the sensor output voltage. 
3.  Verify STM32 input limits. 
4.  Use a voltage divider when required. 
5.  Use a common ground. 
6.  Do not directly connect an unknown 5V analog signal to a 3.3V ADC input. 

### General Power Structure

```
```

```
                 POWER
                   │
          ┌────────┴────────┐
          │                 │
        3.3V               5V*
          │                 │
     ┌────┴────┐       ┌────┴────┐
     │ STM32   │       │ Sensors │
     │ OLED    │       │ if req. │
     │ DHT     │       │ MQ      │
     └─────────┘       └─────────┘
```

`*` Only use 5V where the specific sensor/module requires it and ensure signal levels are safe for the STM32.

---

# 🧠 Machine Learning Architecture

The Machine Learning section consists of:

```
```

```
                 IoT Telemetry Dataset
                           │
                           ▼
                  Data Preprocessing
                           │
                           ▼
                    Feature Selection
                           │
              ┌────────────┴────────────┐
              │                         │
              ▼                         ▼
       K-Means Clustering       Decision Tree
              │                         │
              ▼                         ▼
       Pattern Discovery          Classification
              │                         │
              └────────────┬────────────┘
                           │
                           ▼
                    Model Evaluation
                           │
                           ▼
                   Rule Extraction
                           │
                           ▼
                     Embedded C
                           │
                           ▼
                         STM32
```

---

# 📊 Dataset

The project uses IoT telemetry data containing environmental and activity-related sensor values.

Typical columns include:

```
```

```
ts
device
co
humidity
light
lpg
motion
smoke
temp
```

Example:

```
```

```
ts,device,co,humidity,light,lpg,motion,smoke,temp
1590000000,b8:27:xx,0.12,62,450,0.05,1,0.02,28.4
```

---

# 🧹 Data Preprocessing

Raw telemetry data is processed before ML training.

The pipeline is:

```
```

```
Raw Dataset
     │
     ▼
Remove Unnecessary Columns
     │
     ▼
Handle Missing Values
     │
     ▼
Remove Invalid Records
     │
     ▼
Handle Outliers
     │
     ▼
Select Features
     │
     ▼
Scale Numerical Features
     │
     ▼
ML Dataset
```

---

# 📐 Feature Vector

The model can use:

```
```

```
X = [
    temperature,
    humidity,
    light,
    lpg,
    smoke,
    co,
    motion
]
```

Each sensor reading becomes part of a multi-dimensional feature vector.

---

# 🔵 K-Means Clustering

K-Means is used as an unsupervised learning algorithm.

Its purpose is to discover natural patterns within the sensor dataset.

## K-Means Workflow

```
```

```
Dataset
   │
   ▼
Feature Selection
   │
   ▼
Feature Scaling
   │
   ▼
Choose K
   │
   ▼
Initialize Centroids
   │
   ▼
Assign Data Points
   │
   ▼
Update Centroids
   │
   ▼
Repeat Until Convergence
   │
   ▼
Final Clusters
```

---

# 🎯 Why K-Means?

K-Means answers:

```
```

```
"What natural groups exist in our sensor data?"
```

This is useful when the dataset does not initially have reliable labels.

Possible clusters could represent:

```
```

```
Cluster 0 → Normal
Cluster 1 → Occupied
Cluster 2 → Low Light
Cluster 3 → High Activity
Cluster 4 → Abnormal
```

The exact interpretation depends on the actual dataset.

---

# 🌳 Decision Tree Classification

The Decision Tree is the primary supervised classification model.

It learns rules from labeled examples.

Example:

```
```

```
                Light < Threshold?
                   /          \
                 YES           NO
                 /              \
             Motion?          Normal
             /    \
           YES     NO
           │        │
       Occupied   Low-Light
```

Another branch may use:

```
```

```
Temperature
Smoke
LPG
CO
```

to detect abnormal environmental conditions.

---

# 🎯 Why Decision Tree?

The Decision Tree is particularly suitable for embedded implementation because its learned structure can be represented using simple conditional statements.

Advantages:

-  Simple inference 
-  Low computational complexity 
-  Easy to interpret 
-  Easy to visualize 
-  Easy to convert to C 
-  Suitable for microcontrollers 

---

# 🔵 K-Means vs 🌳 Decision Tree

| Parameter               | K-Means                 | Decision Tree        |
| ----------------------- | ----------------------- | -------------------- |
| Learning Type           | Unsupervised            | Supervised           |
| Labels Required         | No                      | Yes                  |
| Main Purpose            | Pattern discovery       | Classification       |
| Output                  | Cluster ID              | Class label          |
| Training                | Unsupervised            | Supervised           |
| Interpretability        | Moderate                | High                 |
| Embedded Implementation | Less direct             | Very suitable        |
| Project Role            | Explore sensor patterns | Final classification |

---

# 🔄 ML-to-C Conversion

A major part of the project is converting the trained ML model into Embedded C.

The workflow is:

```
```

```
Python Dataset
      │
      ▼
Preprocessing
      │
      ▼
Decision Tree Training
      │
      ▼
Model Evaluation
      │
      ▼
Extract Tree Thresholds
      │
      ▼
Generate C Conditions
      │
      ▼
Compile in STM32CubeIDE
      │
      ▼
Flash STM32
      │
      ▼
Real-Time ML Inference
```

---

# 💻 Example ML-to-C Conversion

A Decision Tree might produce rules such as:

```
```

```
IF temperature > 35
AND smoke > 500
THEN state = SMOKE_ALERT
```

This becomes:

```
```

```
if (temperature > 35.0f)
{
    if (smoke > 500.0f)
    {
        state = STATE_SMOKE_ALERT;
    }
}
```

Another branch:

```
```

```
if (motion == 1 && light < 200)
{
    state = STATE_OCCUPIED;
}
```

The actual values must be obtained from the trained model.

---

# ⚡ Embedded ML

The STM32 does not need to run the full Python Machine Learning environment.

Instead:

```
```

```
Python
  ↓
Train Model
  ↓
Extract Rules
  ↓
C Code
  ↓
STM32
```

This reduces runtime requirements.

---

# 🚀 Advantages of Embedded ML

-  No Python runtime 
-  No scikit-learn dependency 
-  No cloud server 
-  Low latency 
-  Local decision making 
-  Low memory requirement 
-  Suitable for real-time operation 
-  Reduced network dependency 

---

# 🧠 STM32 Firmware Architecture

The firmware can be organized as:

```
```

```
main.c
 │
 ├── System Initialization
 │
 ├── GPIO Initialization
 │
 ├── ADC Initialization
 │
 ├── I²C Initialization
 │
 ├── Sensor Drivers
 │    ├── DHT
 │    ├── LDR
 │    ├── MQ
 │    ├── Smoke
 │    ├── CO
 │    └── PIR
 │
 ├── OLED Driver
 │
 ├── ML Classifier
 │
 ├── State Manager
 │
 └── Display Manager
```

---

# 🔄 Firmware Execution

```
```

```
while (1)
{
    Read_Temperature();
    Read_Humidity();
    Read_Light();
    Read_Gas();
    Read_Smoke();
    Read_CO();
    Read_Motion();

    home_state = ML_Classify(
        temperature,
        humidity,
        light,
        gas,
        smoke,
        co,
        motion
    );

    OLED_Display_Sensors();
    OLED_Display_State(home_state);

    Update_Status_LED();

    HAL_Delay(500);
}
```

The exact function names depend on the implementation.

---

# 🖥️ OLED Output

Example:

```
```

```
┌──────────────────────┐
│   HOME MONITOR       │
├──────────────────────┤
│ TEMP : 28.4 C        │
│ HUM  : 62 %          │
│ LIGHT: 450           │
│ LPG  : 0.05         │
│ SMOKE: 0.02         │
│ CO   : 0.12         │
│ MOTION: YES          │
├──────────────────────┤
│ STATE: OCCUPIED      │
└──────────────────────┘
```

---

# 🚨 Alert Logic

The system can use LEDs to indicate system status.

Example:

```
```

```
GREEN LED
    ↓
Normal Condition
```

```
```

```
RED LED
    ↓
Abnormal / Alert Condition
```

Example:

```
```

```
if (home_state == STATE_NORMAL)
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port,
                      LED_GREEN_Pin,
                      GPIO_PIN_SET);

    HAL_GPIO_WritePin(LED_RED_GPIO_Port,
                      LED_RED_Pin,
                      GPIO_PIN_RESET);
}
else
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port,
                      LED_GREEN_Pin,
                      GPIO_PIN_RESET);

    HAL_GPIO_WritePin(LED_RED_GPIO_Port,
                      LED_RED_Pin,
                      GPIO_PIN_SET);
}
```

---

# 🧪 Wokwi Simulation

The project can be simulated using Wokwi.

The simulation can contain:

```
```

```
STM32
 │
 ├── DHT11
 ├── LDR
 ├── PIR
 ├── MQ Sensor
 ├── OLED
 ├── LEDs
 └── Push Button
```

---

# 🧪 Simulation Workflow

```
```

```
Wokwi
  ↓
Load Circuit
  ↓
Compile Firmware
  ↓
Start Simulation
  ↓
Change Sensor Values
  ↓
Observe STM32 Processing
  ↓
Observe OLED
  ↓
Observe LEDs
```

---

# 🛠️ Software Requirements

## Required

-  STM32CubeIDE 
-  Python 3.x 
-  Git 
-  Wokwi 
-  ST-Link software/tools 

## Python Packages

```
```

```
pip install numpy pandas matplotlib scikit-learn
```

---

# 📥 Installation

Clone the repository:

```
```

```
git clone https://github.com/sriramkanuri-tech/Home-Monitoring-Learning-Supervision-System.git
```

Enter the directory:

```
```

```
cd Home-Monitoring-Learning-Supervision-System
```

Install dependencies:

```
```

```
pip install -r requirements.txt
```

Or:

```
```

```
pip install numpy pandas matplotlib scikit-learn
```

---

# ▶️ Run K-Means

Example:

```
```

```
python machine-learning/kmeans/train_kmeans.py
```

The script should:

```
```

```
Load Dataset
     ↓
Preprocess Data
     ↓
Select Features
     ↓
Scale Data
     ↓
Train K-Means
     ↓
Assign Clusters
     ↓
Evaluate Clusters
     ↓
Generate Results
```

---

# ▶️ Run Decision Tree

Example:

```
```

```
python machine-learning/decision-tree/train_decision_tree.py
```

Workflow:

```
```

```
Load Dataset
     ↓
Preprocess Data
     ↓
Select Features
     ↓
Split Dataset
     ↓
Train Decision Tree
     ↓
Predict
     ↓
Evaluate
     ↓
Extract Rules
```

---

# 📊 ML Evaluation

## Decision Tree

Recommended metrics:

```
```

```
Accuracy
Precision
Recall
F1-Score
Confusion Matrix
```

## K-Means

Recommended metrics:

```
```

```
Inertia
Silhouette Score
Cluster Distribution
Cluster Visualization
```

---

# 📁 Project Structure

```
```

```
Home-Monitoring-Learning-Supervision-System/
│
├── README.md
├── LICENSE
├── requirements.txt
│
├── dataset/
│   ├── raw/
│   │   └── iot_telemetry_data.csv
│   │
│   └── processed/
│       └── processed_dataset.csv
│
├── machine-learning/
│   │
│   ├── preprocessing/
│   │   ├── data_cleaning.py
│   │   └── feature_preprocessing.py
│   │
│   ├── kmeans/
│   │   ├── train_kmeans.py
│   │   ├── cluster_analysis.py
│   │   └── visualization.py
│   │
│   └── decision-tree/
│       ├── train_decision_tree.py
│       ├── evaluate_model.py
│       ├── visualize_tree.py
│       └── extract_rules.py
│
├── embedded/
│   │
│   └── STM32_Project/
│       │
│       ├── Core/
│       │   ├── Inc/
│       │   │   ├── main.h
│       │   │   ├── sensors.h
│       │   │   ├── oled.h
│       │   │   └── ml_model.h
│       │   │
│       │   └── Src/
│       │       ├── main.c
│       │       ├── sensors.c
│       │       ├── oled.c
│       │       └── ml_model.c
│       │
│       ├── Drivers/
│       │
│       └── STM32_Project.ioc
│
├── simulation/
│   │
│   └── Wokwi/
│       ├── diagram.json
│       └── wokwi.md
│
├── documentation/
│   ├── Hardware_Connections.md
│   ├── Machine_Learning.md
│   ├── System_Architecture.md
│   └── Testing.md
│
└── results/
    ├── kmeans/
    ├── decision-tree/
    ├── confusion-matrix/
    └── plots/
```

---

# 🧪 Testing Strategy

Testing is divided into four stages.

## 1. Sensor Testing

Verify individual sensor operation.

```
```

```
DHT → Temperature/Humidity
LDR → Light
MQ → Gas
Smoke → Smoke
CO → CO
PIR → Motion
```

---

## 2. Communication Testing

Verify:

```
```

```
I²C
GPIO
ADC
SWD
```

---

## 3. ML Testing

Verify:

```
```

```
Dataset
 ↓
Preprocessing
 ↓
Training
 ↓
Validation
 ↓
Testing
 ↓
Prediction
```

---

## 4. Embedded Testing

Verify:

```
```

```
Sensor Reading
      ↓
ADC/GPIO
      ↓
ML Inference
      ↓
Classification
      ↓
OLED
      ↓
LED
```

---

# 🔍 Test Cases

| Test               | Input                     | Expected Result         |
| ------------------ | ------------------------- | ----------------------- |
| Normal Environment | Normal sensor values      | NORMAL                  |
| Motion             | PIR HIGH                  | Motion detected         |
| Low Light          | Low LDR value             | Low-light state         |
| High Smoke         | High smoke value          | Smoke alert             |
| High Gas           | High LPG value            | Gas alert               |
| High Temperature   | High temperature          | High-temperature state  |
| Combined Abnormal  | Multiple abnormal sensors | Abnormal state          |
| No Motion          | PIR LOW                   | No motion               |
| OLED Test          | Valid I²C                 | Sensor values displayed |

The exact classification depends on the trained model.

---

# 🛠️ Troubleshooting

## OLED Not Displaying

Check:

```
```

```
VCC
GND
SCL
SDA
```

Also verify:

-  I²C peripheral is enabled. 
-  Correct I²C instance is used. 
-  OLED I²C address is correct. 
-  Pull-up configuration is appropriate. 

Common SSD1306 addresses include:

```
```

```
0x3C
0x3D
```

---

## DHT Sensor Not Working

Check:

-  DATA pin 
-  VCC 
-  GND 
-  Timing implementation 
-  Pull-up resistor if required 

---

## PIR Always HIGH

Check:

-  Sensor warm-up time. 
-  VCC. 
-  GND. 
-  OUT pin. 
-  Sensor sensitivity/time-delay potentiometers. 

---

## LDR Digital Output Problem

If using `DO`, the module is threshold-based.

For ML, use:

```
```

```
AO → ADC
```

rather than only:

```
```

```
DO → GPIO
```

This provides a continuous numerical measurement.

---

## MQ Sensor Reading Problem

MQ sensors require:

-  Appropriate supply 
-  Warm-up time 
-  Calibration 
-  Correct ADC connection 

Never connect an unknown sensor output directly to an STM32 ADC without checking its voltage range.

---

## STM32 Not Detected by ST-Link

Check:

```
```

```
SWDIO → PA13
SWCLK → PA14
GND   → GND
```

Also check:

-  ST-Link drivers 
-  Target power 
-  SWD configuration 
-  USB cable 
-  ST-Link connection 

---

# ⚠️ Important Hardware Safety Notes

This is an educational prototype.

For gas, smoke, LPG, and CO sensing:

-  Do not treat the prototype as a certified safety device. 
-  MQ sensors require proper calibration. 
-  Sensor readings are affected by environmental conditions. 
-  Do not use the system as a replacement for certified smoke, gas, or CO alarms. 
-  Ensure appropriate ventilation and electrical safety when testing gas sensors. 
-  Verify all sensor voltage levels before connecting them to the STM32. 

---

# 📈 Advantages

## Hardware

-  Low-cost components 
-  Compact architecture 
-  Real-time monitoring 
-  Expandable sensor interface 
-  STM32-based processing 

## Machine Learning

-  Multi-sensor analysis 
-  Pattern discovery 
-  Classification 
-  Interpretable Decision Tree 
-  Lightweight inference 

## Embedded System

-  Offline operation 
-  Low latency 
-  Low computational overhead 
-  No cloud requirement for basic classification 
-  Suitable for real-time applications 

---

# ⚠️ Limitations

1.  Sensor accuracy directly affects ML performance. 
2.  Gas sensors require calibration. 
3.  Sensor noise can affect predictions. 
4.  K-Means requires selecting an appropriate number of clusters. 
5.  Decision Trees can overfit. 
6.  Model performance depends on dataset quality. 
7.  Real-world conditions may differ from training data. 
8.  STM32 has limited memory and computational resources. 
9.  Complex ML models may require additional optimization. 
10.  The system is a prototype and is not a certified safety system. 

---

# 🔮 Future Scope

The system can be expanded with:

## 📱 Mobile Application

A mobile application can provide:

-  Live sensor values 
-  Home state 
-  Alerts 
-  Historical data 
-  Device status 

---

## 🌐 Web Dashboard

A web dashboard can provide:

```
```

```
Real-Time Sensor Graphs
        +
ML Predictions
        +
Alert History
        +
Device Status
        +
Historical Analytics
```

---

## ☁️ Cloud Integration

The system can be connected to:

-  MQTT 
-  Firebase 
-  AWS 
-  Azure 
-  Other IoT platforms 

for remote monitoring and historical data storage.

---

## 📡 Wireless Communication

Future hardware versions can use:

-  ESP32 
-  Wi-Fi 
-  Bluetooth 
-  LoRa 
-  Zigbee 
-  MQTT 

---

## 🧠 TinyML

The Decision Tree approach can be extended toward TinyML.

Possible future models:

```
```

```
Decision Tree
Random Forest
Small Neural Network
Quantized Neural Network
```

Models can be optimized for microcontroller deployment.

---

# 🔐 Security Considerations

If wireless/cloud connectivity is added in the future, the system should implement:

-  Authentication 
-  Encrypted communication 
-  Secure MQTT 
-  Device authorization 
-  Secure firmware updates 
-  Access control 
-  Protected API credentials 

---

# 📚 Learning Outcomes

This project provides practical experience in:

### Embedded Systems

-  STM32 
-  Embedded C 
-  GPIO 
-  ADC 
-  I²C 
-  SWD 
-  Sensor interfacing 

### IoT

-  Sensor telemetry 
-  Environmental monitoring 
-  Real-time data acquisition 
-  IoT architecture 

### Machine Learning

-  Data preprocessing 
-  Feature selection 
-  Feature scaling 
-  K-Means 
-  Decision Trees 
-  Classification 
-  Model evaluation 

### Embedded ML

-  Model interpretation 
-  Rule extraction 
-  ML-to-C conversion 
-  Lightweight inference 
-  Microcontroller deployment 

### Development

-  Python 
-  STM32CubeIDE 
-  Wokwi 
-  Git 
-  GitHub 
-  Hardware debugging 

---

# 🏆 Project Outcome

The final project integrates:

```
```

```
              ┌───────────────┐
              │    Sensors    │
              └───────┬───────┘
                      │
                      ▼
              ┌───────────────┐
              │     STM32     │
              └───────┬───────┘
                      │
                      ▼
              ┌───────────────┐
              │ Data Processing│
              └───────┬───────┘
                      │
                      ▼
              ┌───────────────┐
              │  ML Inference │
              └───────┬───────┘
                      │
              ┌───────┴────────┐
              │                │
              ▼                ▼
       ┌────────────┐    ┌────────────┐
       │   Normal   │    │   Alert    │
       └──────┬─────┘    └──────┬─────┘
              │                 │
              └────────┬────────┘
                       ▼
                ┌─────────────┐
                │ OLED Display│
                └─────────────┘
```

The project demonstrates a complete pipeline:

```
```

```
Physical Environment
        ↓
Sensor Data
        ↓
STM32 Acquisition
        ↓
Data Processing
        ↓
Machine Learning
        ↓
Decision Tree
        ↓
C-Based ML Inference
        ↓
Home-State Classification
        ↓
OLED / LED Output
```

---

# 🌟 What Makes This Project Different?

The project does not simply read individual sensors.

It combines:

```
```

```
IoT
+
Multi-Sensor Data
+
Machine Learning
+
Embedded Systems
+
STM32
+
Real-Time Classification
```

The Machine Learning model is trained externally and then converted into a form that can operate directly on the embedded controller.

This creates a bridge between:

```
```

```
Machine Learning
        ↕
Embedded Systems
        ↕
IoT
```

---

# 📌 Complete Project Flow

```
```

```
                     ┌─────────────────────┐
                     │  Physical Environment│
                     └──────────┬──────────┘
                                │
                                ▼
                     ┌─────────────────────┐
                     │      Sensors        │
                     │                     │
                     │ Temp / Humidity     │
                     │ Light               │
                     │ LPG / Gas           │
                     │ Smoke               │
                     │ CO                  │
                     │ Motion              │
                     └──────────┬──────────┘
                                │
                                ▼
                     ┌─────────────────────┐
                     │       STM32         │
                     │                     │
                     │ GPIO                │
                     │ ADC                 │
                     │ I²C                 │
                     └──────────┬──────────┘
                                │
                                ▼
                     ┌─────────────────────┐
                     │  Sensor Processing  │
                     └──────────┬──────────┘
                                │
                                ▼
                  ┌────────────────────────────┐
                  │      Machine Learning      │
                  │                            │
                  │  K-Means + Decision Tree  │
                  └────────────┬───────────────┘
                               │
                               ▼
                     ┌─────────────────────┐
                     │   Embedded C Model  │
                     └──────────┬──────────┘
                                │
                                ▼
                     ┌─────────────────────┐
                     │ Home-State Result   │
                     └──────────┬──────────┘
                                │
                    ┌───────────┴───────────┐
                    │                       │
                    ▼                       ▼
             ┌─────────────┐        ┌─────────────┐
             │ SSD1306 OLED│        │ LEDs / Alert│
             └─────────────┘        └─────────────┘
```

---

# 👨‍💻 Author

## Sriram Kanuri

**B.Tech – Electronics & Communication Engineering**

Project Area:

```
```

```
Embedded Systems
IoT
Machine Learning
STM32
Embedded ML
```

---

# 📜 License

This project is developed primarily for:

-  Educational purposes 
-  Academic project work 
-  Embedded systems experimentation 
-  Machine Learning experimentation 
-  Research and prototyping 

---

# ⭐ Support

If you find this project useful:

-  ⭐ Star the repository 
-  🍴 Fork the repository 
-  🐛 Report issues 
-  💡 Suggest improvements 
-  🔧 Contribute improvements 

---

# 📌 Final Project Summary

The **Home Monitoring & Learning Supervision System** is an intelligent embedded IoT platform that combines multi-sensor environmental monitoring with Machine Learning.

The system collects:

```
```

```
Temperature
Humidity
Light
LPG
Smoke
CO
Motion
```

using multiple sensors connected to an STM32 microcontroller.

The collected data is analyzed using Machine Learning.

**K-Means** is used to discover natural patterns in the IoT telemetry dataset.

**Decision Tree** classification is then used to classify sensor conditions into meaningful home/environment states.

The trained Decision Tree is converted into lightweight Embedded C logic and deployed on the STM32.

The final system performs:

```
```

```
Sensor Acquisition
        ↓
Data Processing
        ↓
Machine Learning
        ↓
Classification
        ↓
Embedded C Inference
        ↓
STM32
        ↓
OLED Display
        ↓
Status / Alerts
```

This project demonstrates the integration of:

**IoT + Embedded Systems + Machine Learning + STM32 + Sensors + Embedded C + Real-Time Intelligence**

into a single intelligent home monitoring platform.

---

 \<p align="center"> 

### 🏠 Home Monitoring & Learning Supervision System

**Built with STM32 • IoT • Machine Learning • Embedded C**

⭐ **Intelligent Monitoring at the Edge** ⭐

 \</p> \`\`\` 

### One important thing before you push this

The README above uses a **recommended STM32 pin map**:

| Function      | Pin    |
| ------------- | ------ |
| LDR AO        | `PA0`  |
| DHT DATA      | `PA1`  |
| MQ Gas AO     | `PA2`  |
| Smoke AO      | `PA3`  |
| CO AO         | `PA4`  |
| LED 1         | `PA5`  |
| LED 2         | `PA6`  |
| OLED SCL      | `PB6`  |
| OLED SDA      | `PB7`  |
| PIR OUT       | `PB0`  |
| Button        | `PB1`  |
| ST-Link SWDIO | `PA13` |
| ST-Link SWCLK | `PA14` |
