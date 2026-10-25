# MazeBot ESP32 — Autonomous Maze Solver & Wireless HUD

Autonomous maze-solving robot powered by an ESP32, TB6612FNG motor driver, and a 5x VL53L0X Time-of-Flight laser sensor array. Features physical pushbutton control for competition compliance and a real-time wireless radar telemetry dashboard.

---

## 📌 Hardware Pinout (ESP32 DevKit)

### 1. TB6612FNG Motor Driver
| TB6612FNG Pin | ESP32 GPIO | Description |
|:---|:---|:---|
| **PWMA** | `GPIO 32` | Left Motor Speed (LEDC PWM) |
| **AIN1** | `GPIO 33` | Left Motor Direction 1 |
| **AIN2** | `GPIO 25` | Left Motor Direction 2 |
| **PWMB** | `GPIO 26` | Right Motor Speed (LEDC PWM) |
| **BIN1** | `GPIO 27` | Right Motor Direction 1 |
| **BIN2** | `GPIO 14` | Right Motor Direction 2 |
| **STBY** | `3.3V / VCC` | Hardwired Active HIGH |
| **VM**   | Battery (+) | 7.4V - 12V Motor Power |
| **VCC**  | 5V / 3.3V | Logic Power |
| **GND**  | GND | Common System Ground |

---

### 2. Physical Pushbuttons
| Button Function | ESP32 GPIO | Configuration | Description |
|:---|:---:|:---|:---|
| **START** | `GPIO 13` | `INPUT_PULLUP` | Initiates the Entry-to-Exit maze solving run |
| **STOP**  | `GPIO 4`  | `INPUT_PULLUP` | Emergency Stop (halts motors immediately) |

---

### 3. 5x Time-of-Flight (VL53L0X) Sensors Array
All sensors share the hardware I2C bus:
- **SDA**: `GPIO 21`
- **SCL**: `GPIO 22`
- **VCC**: `3.3V` (from AMS1117 regulator)
- **GND**: `GND`

Each sensor's **XSHUT** pin is connected to an independent GPIO for sequential initialization and unique I2C address assignment:

| Sensor Target | Angle | XSHUT GPIO | Dynamic I2C Address |
|:---|:---:|:---:|:---:|
| **Left ToF** | -90° | `GPIO 16` | `0x30` |
| **Front-Left ToF** | -45° | `GPIO 17` | `0x31` |
| **Center ToF** | 0° (Front) | `GPIO 18` | `0x32` |
| **Front-Right ToF** | +45° | `GPIO 19` | `0x33` |
| **Right ToF** | +90° | `GPIO 23` | `0x34` |

---

## 🏁 Entry-to-Exit Maze Run Logic

1. **Place at Entry**: Place the robot at the maze entrance.
2. **Start**: Press the **START** button (`GPIO 13`). The robot waits 300ms for hand clearance and begins solving.
3. **Corridor Navigation**:
   - Uses a Proportional-Derivative (PD) controller for corridor centering between walls.
   - Follows individual walls when single-sided corridors occur.
   - Evaluates obstacle distances ahead and turns at intersections (prioritizing the left-hand rule).
   - Automatically executes 180° escape turns at dead ends.
4. **Auto Exit Stop**: Once all forward and side sensors detect open space beyond the maze threshold (`> 260 mm`), the bot confirms the exit, drives forward slightly to clear the threshold, and automatically halts motors.
5. **Emergency Stop**: Pressing the **STOP** button (`GPIO 4`) immediately cuts motor power at any point.

---

## 📡 Wireless Monitoring HUD

The ESP32 broadcasts its own Wi-Fi Access Point for live diagnostics:
- **Wi-Fi SSID**: `MazeBot-ESP32`
- **Password**: `12345678`
- **Web Dashboard URL**: [http://192.168.4.1/](http://192.168.4.1/)

### Dashboard Features:
- **Interactive Radar HUD**: Visualizes the robot chassis with 5 real-time distance beams updating at 10 Hz.
- **Safety Zones**: Colors transition dynamically (Green > 180mm, Yellow 100-180mm, Red < 100mm).
- **Physical Button Status**: Displays live operating states (`READY AT ENTRY`, `ENTRY RUN`, `EXIT REACHED`, `STOPPED`).
- **Live Tuning**: Fine-tune base speed, front obstacle threshold, and desired side wall distance wirelessly.

---

## 🚀 How to Build & Upload Firmware

Using PlatformIO:
```cmd
python -m platformio run --target upload
```
*(or run `upload.bat`)*
