Real-Time System Mid-term Project Assignment

Task 1: Single LED Blink with Temperature Conditions
Redefine the LED blinking behavior to respond to different temperature conditions (at least 3 different behaviors).
Use semaphores to manage task synchronization in your implementation.
Ensure that the condition handling and semaphore logic are clearly explained in your code and report.

Task 2: NeoPixel LED Control Based on Humidity
Redefine NeoPixel (RGB LED) color patterns to represent different humidity levels (at least 3 levels/colors).
Utilize semaphore synchronization technique for updating and displaying color changes.
Clearly show the mapping between humidity value ranges and colors.

Task 3: Temperature and Humidity Monitoring with LCD Display
Define conditions for creating/releasing semaphores based on sensor readings.
Integrate an LCD module to display real-time temperature and humidity values.
At least 3 different display states (e.g., normal, warning, critical) according to measurement conditions.

Task 4: Web Server in Access Point Mode
Redesign the web server interface for better usability.
Web server must enable control over two devices (e.g., LED1 and LED2, which can be renamed or replaced as appropriate for your group’s application).
The interface must include at least two buttons and appropriately labeled control actions.

Task 5: TinyML Deployment & Accuracy Evaluation
Describe the dataset used for model training, including data collection and labeling steps.
Implement a TinyML model and run it on your microcontroller.
Measure and evaluate the recognition accuracy of your model on the hardware, and provide discussion and conclusion regarding performance.

# Espressif 32: development platform for [PlatformIO](https://platformio.org)

[![Build Status](https://github.com/platformio/platform-espressif32/workflows/Examples/badge.svg)](https://github.com/platformio/platform-espressif32/actions)

ESP32 is a series of low-cost, low-power system on a chip microcontrollers with integrated Wi-Fi and Bluetooth. ESP32 integrates an antenna switch, RF balun, power amplifier, low-noise receive amplifier, filters, and power management modules.

* [Home](https://registry.platformio.org/platforms/platformio/espressif32) (home page in the PlatformIO Registry)
* [Documentation](https://docs.platformio.org/page/platforms/espressif32.html) (advanced usage, packages, boards, frameworks, etc.)

# Usage

1. [Install PlatformIO](https://platformio.org)
2. Create PlatformIO project and configure a platform option in [platformio.ini](https://docs.platformio.org/page/projectconf.html) file:

## Stable version

See `platform` [documentation](https://docs.platformio.org/en/latest/projectconf/sections/env/options/platform/platform.html#projectconf-env-platform) for details.

```ini
[env:stable]
; recommended to pin to a version, see https://github.com/platformio/platform-espressif32/releases
; platform = espressif32 @ ^6.0.1
platform = espressif32
board = yolo_uno
framework = arduino
monitor_speed = 115200

build_flags =
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1

## Development version

```ini
[env:development]
platform = https://github.com/platformio/platform-espressif32.git
board = yolo_uno
framework = arduino
monitor_speed = 115200
build_flags =
    -D ARDUINO_USB_MODE=1
    -D ARDUINO_USB_CDC_ON_BOOT=1

    
# Configuration

Please navigate to [documentation](https://docs.platformio.org/page/platforms/espressif32.html).
