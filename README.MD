# 🛰️ Submarine Sonar & Radar System (ESP32 CYD + HLK-LD2410)

An interactive, retro-futuristic **Submarine Sonar System** built for the **ESP32 Cheap Yellow Display (CYD / ESP32-2432S028)** utilizing the **HLK-LD2410 24GHz mmWave Radar** sensor. 

This project simulates a real-time radar sweep animation, detecting moving targets, plotting their positions on a circular radar grid, and displaying accurate distance and signal energy telemetry on a dedicated lateral HUD panel.

<p align="center">
  <img src="ESP32CYD.JPG" alt="Preview do Sonar" width="500">
</p>

<p align="center">
  <img src="ld2410b.jpg" alt="Preview do Sonar" width="500">
</p>



---

## 📸 Project Features

* **Submarine-Style Radar UI:** Smooth 360° radar sweep animation rendered in classic green/red vector graphics.
* **Persistent Target Tracking:** Retains target position on screen throughout the full radar sweep cycle for enhanced tracking.
* **Real-time Telemetry HUD:** Displays target presence (`CONTATO!` / `SEM CONTATO`), distance in centimeters, and movement energy percentage.
* **Flicker-Free Rendering:** Optimized screen restoration logic ensuring grid axes and target points remain stable during sweeps.

---

## 🧰 Hardware Requirements

| Component | Quantity | Description |
| :--- | :---: | :--- |
| **ESP32 CYD** (*Cheap Yellow Display*) | 1 | ESP32-2432S028 development board with integrated 2.8" TFT display |
| **HLK-LD2410 Sensor** | 1 | 24GHz human presence / mmWave radar module (LD2410B/C) |
| **Jumper Wires / Connector** | 4 | JST-SH connector or Female Dupont wires for UART connection |
| **Micro-USB or USB-C Cable** | 1 | Power supply & programming cable |

---

## 🔌 Wiring & Pinout Diagram

The **HLK-LD2410** connects to the **UART2** expansion header (`P4` connector) on the CYD board:

```
┌─────────────────────────────────┐               ┌─────────────────────────────────┐
│     ESP32 CYD (Connector P4)    │               │        HLK-LD2410 Radar        │
│                                 │               │                                 │
│                        5V/VIN ├───────────────┤ VCC (5V Power Input)           │
│                           GND ├───────────────┤ GND (Ground)                    │
│                      GPIO 35 ├───────────────┤ TX (Radar Serial Output)        │
│                      GPIO 22 ├───────────────┤ RX (Radar Serial Input)         │
└─────────────────────────────────┘               └─────────────────────────────────┘
```

> ⚠️ **Important Notes:**
> 1. **5V Power:** The HLK-LD2410 requires a steady **5V power supply** to function reliably. Ensure it connects to the 5V/VIN pin of the CYD.
> 2. **Serial Baud Rate:** Communication runs at **256,000 baud** by default on hardware serial port `Serial2`.
> 3. **TX/RX Crossover:** Connect Sensor TX -> ESP32 RX (GPIO 35) and Sensor RX -> ESP32 TX (GPIO 22).

---

## 💻 Software & Library Setup

### Required Libraries
Install the following libraries via the **Arduino IDE Library Manager**:

1. **`TFT_eSPI`** by Bodmer
2. **`ld2410`** by ntechde (or cpyne/npredictable)

### ⚙️ Configuring `TFT_eSPI` for CYD

Before compiling, update your `User_Setup.h` (or select the appropriate setup header for the ESP32 Cheap Yellow Display) inside the `TFT_eSPI` library folder with the following pin definition:

```cpp
#define ILI9341_DRIVER
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1
#define TFT_BL   21
```

---

## 📜 Full Source Code (`main.cpp` / `sketch.ino`)

```cpp
#include <TFT_eSPI.h>
#include <ld2410.h>

TFT_eSPI tft = TFT_eSPI();
ld2410 radar;

// ESP32 CYD Expansion Port Pins (UART2)
#define RXD2 35
#define TXD2 22

#define RADAR_X 110
#define RADAR_Y 120
#define RADAR_R 95
#define MAX_DIST_CM 600

// Submarine Sonar Color Palette (RGB565)
#define COR_FUNDO       TFT_BLACK
#define COR_VARREDURA   TFT_GREEN
#define COR_GRADE       0x03E0
#define COR_TEXTO_GLOW  TFT_GREEN
#define COR_ALERTA_MOV  TFT_RED
#define COR_HUD_BORDAS  0x03E0

#define LIMIAR_ENERGIA_MINIMA 35

float sweepAngle = 0;
float lastAngle = 0;

// Target Tracking Memory
bool alvoDetectadoNesteGiro = false;
bool exibirAlvoNaTela = false;
int alvoDistanciaCm = 0;
int alvoEnergia = 0;
int alvoX = -1;
int alvoY = -1;

void desenharGradeRadar() {
  for (int r = 16; r <= RADAR_R; r += 16) {
    tft.drawCircle(RADAR_X, RADAR_Y, r, COR_GRADE);
  }
  tft.drawFastHLine(RADAR_X - RADAR_R - 5, RADAR_Y, (RADAR_R * 2) + 10, COR_GRADE);
  tft.drawFastVLine(RADAR_X, RADAR_Y - RADAR_R - 5, (RADAR_R * 2) + 10, COR_GRADE);

  // Side HUD Panel
  tft.fillRect(220, 0, 100, 240, COR_FUNDO); 
  tft.drawFastVLine(220, 0, 240, COR_HUD_BORDAS);

  tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
  tft.setTextSize(1);
  tft.drawString("SONAR SUB", 235, 10);
  tft.drawString("=================", 223, 20);

  tft.drawString("STATUS:", 225, 40);
  tft.drawString("DISTANCIA:", 225, 105);
  tft.drawString("ENERGIA:", 225, 170);

  tft.drawRect(223, 53, 93, 38, COR_HUD_BORDAS);
  tft.drawRect(223, 118, 93, 38, COR_HUD_BORDAS);
  tft.drawRect(223, 183, 93, 38, COR_HUD_BORDAS);
}

void atualizarPainelHUD(bool ativo, int distCm, int energia) {
  tft.setTextSize(1);

  if (ativo) {
    tft.fillRect(224, 54, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_ALERTA_MOV, COR_FUNDO);
    tft.drawString(" CONTATO! ", 235, 68);

    tft.fillRect(224, 119, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.setTextSize(2);
    tft.drawString(String(distCm) + "cm", 230, 130);

    tft.fillRect(224, 184, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.drawString(String(energia) + "%", 245, 195);
  } else {
    tft.fillRect(224, 54, 91, 36, COR_FUNDO);
    tft.setTextColor(COR_TEXTO_GLOW, COR_FUNDO);
    tft.drawString(" SEM ", 250, 62);
    tft.drawString(" CONTATO ", 240, 74);

    tft.fillRect(224, 119, 91, 36, COR_FUNDO);
    tft.fillRect(224, 184, 91, 36, COR_FUNDO);
  }
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(256000, SERIAL_8N1, RXD2, TXD2);

  tft.init();
  tft.invertDisplay(true);
  tft.setRotation(1);
  tft.fillScreen(COR_FUNDO);

  desenharGradeRadar();
  radar.begin(Serial2);
}

void loop() {
  radar.read();

  // 1. Radar Data Acquisition
  if (radar.presenceDetected() && radar.movingTargetDetected()) {
    int energia = radar.movingTargetEnergy();
    if (energia >= LIMIAR_ENERGIA_MINIMA) {
      alvoDetectadoNesteGiro = true;
      alvoDistanciaCm = radar.movingTargetDistance();
      alvoEnergia = energia;
    }
  }

  // 2. Full Sweep Cycle Completion Check (360°)
  if (sweepAngle < lastAngle) {
    if (alvoDetectadoNesteGiro) {
      // Erase previous target marker
      if (alvoX != -1) {
        tft.fillCircle(alvoX, alvoY, 6, COR_FUNDO);
      }

      // Calculate target X,Y coordinates
      int targetR = map(constrain(alvoDistanciaCm, 0, MAX_DIST_CM), 0, MAX_DIST_CM, 0, RADAR_R);
      float anguloPlot = 4.71239; // Top quadrant (270 degrees)
      alvoX = RADAR_X + cos(anguloPlot) * targetR;
      alvoY = RADAR_Y + sin(anguloPlot) * targetR;

      exibirAlvoNaTela = true;
    } else {
      // Clear marker if no target detected during current sweep
      if (alvoX != -1) {
        tft.fillCircle(alvoX, alvoY, 6, COR_FUNDO);
        alvoX = -1;
      }
      exibirAlvoNaTela = false;
    }

    atualizarPainelHUD(exibirAlvoNaTela, alvoDistanciaCm, alvoEnergia);
    alvoDetectadoNesteGiro = false;
  }
  lastAngle = sweepAngle;

  // 3. Radar Beam Sweep Animation
  int px = RADAR_X + cos(sweepAngle) * RADAR_R;
  int py = RADAR_Y + sin(sweepAngle) * RADAR_R;

  tft.drawLine(RADAR_X, RADAR_Y, px, py, COR_VARREDURA);
  delay(8);
  tft.drawLine(RADAR_X, RADAR_Y, px, py, COR_FUNDO);

  // 4. Background Grid Restoration
  tft.drawFastHLine(RADAR_X - RADAR_R - 5, RADAR_Y, (RADAR_R * 2) + 10, COR_GRADE);
  tft.drawFastVLine(RADAR_X, RADAR_Y - RADAR_R - 5, (RADAR_R * 2) + 10, COR_GRADE);
  for (int r = 16; r <= RADAR_R; r += 16) {
    tft.drawCircle(RADAR_X, RADAR_Y, r, COR_GRADE);
  }

  // 5. Render Target Marker
  if (exibirAlvoNaTela && alvoX != -1) {
    tft.fillCircle(alvoX, alvoY, 4, COR_ALERTA_MOV);
    tft.drawCircle(alvoX, alvoY, 6, COR_VARREDURA);
  }

  // Sweep angle progression
  sweepAngle += 0.07;
  if (sweepAngle >= 6.28318) {
    sweepAngle = 0;
  }
}
```

---

## ⚙️ Customization Parameters

You can adjust key parameters directly at the top of the code:

* `#define LIMIAR_ENERGIA_MINIMA 35`: Minimum motion energy required to trigger a target contact alert.
* `#define MAX_DIST_CM 600`: Maximum detection range in centimeters (defaults to 6 meters).

---

## 📄 License

Distributed under the MIT License. Feel free to modify and share!
