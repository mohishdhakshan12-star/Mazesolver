#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// TB6612FNG MOTOR DRIVER PIN ASSIGNMENTS (ESP32)
// ============================================================================
#define PIN_PWMA    32    // Left Motor PWM Speed
#define PIN_AIN1    33    // Left Motor Direction 1
#define PIN_AIN2    25    // Left Motor Direction 2

#define PIN_PWMB    26    // Right Motor PWM Speed
#define PIN_BIN1    27    // Right Motor Direction 1
#define PIN_BIN2    14    // Right Motor Direction 2

// Note: TB6612FNG STBY pin is hardware-tied to 3.3V/VCC on this robot.
// GPIO 4 is dedicated to the physical STOP pushbutton.

// ============================================================================
// PHYSICAL PUSHBUTTON ASSIGNMENTS
// ============================================================================
#define PIN_BUTTON_START    13   // Physical START button (Active LOW, internal pull-up)
#define PIN_BUTTON_STOP     4    // Physical STOP button (Active LOW, internal pull-up)

// PWM Channel configuration (ESP32 LEDC)
#define PWM_FREQ         20000 // 20 kHz ultrasonic frequency (no motor whining)
#define PWM_RESOLUTION   8     // 8-bit resolution (0 - 255)
#define PWM_CHAN_LEFT    0     // LEDC Channel 0
#define PWM_CHAN_RIGHT   1     // LEDC Channel 1

// ============================================================================
// 5x TIME-OF-FLIGHT (VL53L0X) I2C & XSHUT PIN ASSIGNMENTS
// ============================================================================
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// XSHUT shutdown control pins for sequential address configuration
#define PIN_XSHUT_LEFT        16   // Sensor 0: Left (90°)
#define PIN_XSHUT_FRONT_LEFT  17   // Sensor 1: Front-Left (45°)
#define PIN_XSHUT_CENTER      18   // Sensor 2: Center / Front (0°)
#define PIN_XSHUT_FRONT_RIGHT 19   // Sensor 3: Front-Right (45°)
#define PIN_XSHUT_RIGHT       23   // Sensor 4: Right (90°)

// Unique dynamic I2C addresses assigned to each sensor at startup
#define I2C_ADDR_LEFT         0x30
#define I2C_ADDR_FRONT_LEFT   0x31
#define I2C_ADDR_CENTER       0x32
#define I2C_ADDR_FRONT_RIGHT  0x33
#define I2C_ADDR_RIGHT        0x34

// ============================================================================
// WI-FI & WEB SERVER CONFIGURATION
// ============================================================================
#define AP_SSID         "MazeBot-ESP32"
#define AP_PASSWORD     "12345678"
#define AP_IP_DEFAULT   "192.168.4.1"

// Wi-Fi Station Mode credentials
#define STA_SSID        "moye"
#define STA_PASSWORD    "mohish d"


// ============================================================================
// DEFAULT MAZE SOLVER NAVIGATION & EXIT DETECTION PARAMETERS
// ============================================================================
#define DEFAULT_BASE_SPEED          150  // Base forward speed (0 - 255)
#define DEFAULT_TURN_SPEED          140  // Speed during turning
#define DEFAULT_FRONT_WALL_DIST     140  // Front wall trigger distance in mm
#define DEFAULT_DESIRED_SIDE_DIST   110  // Desired distance from side wall in mm
#define DEFAULT_WALL_DETECT_MAX     260  // Distance above which wall is considered absent
#define DEFAULT_KP                  0.75 // Proportional gain for wall centering
#define DEFAULT_KD                  0.25 // Derivative gain for damping

// Entry & Exit run detection parameters
#define MIN_RUN_TIME_BEFORE_EXIT_MS 2500 // Min run duration before exit detection triggers
#define EXIT_CONFIRM_SAMPLES        6    // Consecutive sensor checks of open space to confirm exit
#define EXIT_POST_DRIVE_MS          250  // Forward drive time to clear exit opening

#endif // CONFIG_H
