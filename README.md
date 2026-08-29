# OpenLog1 Data Logger for High-Altitude Balloon Atmospheric Measurements

This repository contains a lightweight Arduino-based data logging framework for use with the SparkFun OpenLog Artemis. The code is designed to make it easy to connect, configure, and log data from a custom set of environmental and motion sensors without the overhead of the official vendor firmware (find here: https://github.com/sparkfun/OpenLog_Artemis).

The source code was developed and used for an atmospheric science and high-altitude balloon payload as part of NC Space Grant's High-Altitude-Balloon (HAB) Challenge 2026. South Piedmont Community College team _BLAST!!_ launched a CubeSat 2U payload into the air using a helium balloon, reaching a max altitude of 96,003 ft, highest of any other team (https://ncspacegrant.ncsu.edu/2026/05/29/north-carolina-community-colleges-fly-high-in-the-annual-high-altitude-ballooning-challenge/). Throughout the flight, this system was used as our primary mechanism to record environmental and motion data.


## 1) Why this project exists

1. To provide a small, transparent, and editable data logging framework.
2. To make it easier to add sensors that are not supported by the OpenLog Artemis stock software, especially devices like the `LTR390`, `LSM9DS1`, and `BMP388`.

Sparkfun's official firmware has a lot more features, but it supports only a limited set of sensors and is much more complex, making it difficult to add new sensors. The process is also poorly documented (by their own admission). To add a new sensor in this project, just create a new class implementing `SensorBase` or `I2CSensor`, and update `setup()`. Note that the only difference between `openlog1.ino` and `openlog2.ino` is just which sensors are used. 

## 2) Required board support

The logger uses Apollo3-based board support and RTC functionality, so the matching Apollo3 core must be installed in the Arduino IDE. If your board does not have an RTC, just comment `rtcSensor` out in `setup()`.

1. Open Arduino IDE.
2. Go to `Tools > Board > Boards Manager`.
3. Install the Apollo3 core used by your OpenLog Artemis hardware.
4. Select the correct board in `Tools > Board`.

## 3) External libraries required

The sketch in `openlog1.ino` and `sensors.h` uses the following libraries:

| Library | Version | Purpose |
| --- | --- | --- |
| `SdFat` | 2.3.0 | SD card filesystem and CSV log writing |
| `RTC` | 2.0.0 | Real-time clock support and timestamping |
| `Adafruit BusIO` | 1.17.4 | Shared dependency for Adafruit sensor libraries |
| `Adafruit Unified Sensor` | 1.1.15 | Common sensor abstraction layer |
| `Adafruit BME280 Library` | 2.3.0 | Temperature, humidity, and pressure measurements |
| `Adafruit SGP30 Sensor` | 2.0.3 | TVOC and CO2-equivalent measurements |
| `LTR390` | 1.0.9 | UV index and ambient light measurements |
| `Adafruit SHT31 Library` | 2.2.2 | Secondary temperature and humidity measurements |
| `Adafruit LSM9DS1 Library` | 2.2.1 | 9-axis IMU data (accelerometer, gyroscope, magnetometer) |
| `SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library` | 1.3.2 | SPI IMU used for motion sensing |
| `Adafruit BMP3XX Library` | 2.1.6 | BMP388 pressure and temperature measurements |

## 4) Important hardware note: ICM-20948

The `ICM-20948` is an onboard sensor on some OpenLog Artemis models, but it is not included on all versions of the board. This project assumes the `ICM-20948` is present on the hardware being used.

If your specific OpenLog Artemis model does not include the onboard ICM-20948, remove or disable the relevant sensor initialization in the code, or replace it with another compatible IMU.

## 5) Installation in Arduino IDE

1. Open `Sketch > Include Library > Manage Libraries...`
2. Install each library listed above.
3. Restart Arduino IDE if needed.
4. Verify the sketch with `Sketch > Verify/Compile`.

## 6) Sensor overview

### BME280
- Measures temperature, humidity, and pressure.
- Used for core environmental monitoring and as the reference humidity source for the SGP30 sensor.

### SGP30
- Measures TVOC and eCO2.
- Useful for air-quality tracking in the payload environment.

### LTR390
- Measures UV index and ambient light in lux.
- This project uses the `LTR390` third-party library instead of the official Adafruit version, because the Adafruit one just didn't work for my particular sensor.

### SHT31
- Measures temperature and relative humidity.
- Provides a second environmental sensor for redundancy and comparison.

### LSM9DS1
- Measures accelerometer, gyroscope, and magnetometer data.
- Provides a 9-axis motion dataset suitable for attitude and vibration tracking.

### ICM-20948
- Measures acceleration, rotation, magnetic field, and temperature.
- Used by the `IMUSensor` class when the onboard sensor is present.

### BMP388
- Measures pressure and temperature.
- Suitable for additional atmospheric measurements and pressure trend tracking.

### RTC
- Provides the date/time needed for timestamped CSV data.
- Critical for correlating sensor readings with flight time.

### SdFat
- Provides the SD card interface and file creation logic for writing `.CSV` data logs.

## 7) Extensibility and sensor selection

This project is intentionally modular. If a sensor is not needed for a particular flight or payload configuration, it can be disabled by commenting out its initialization and logging lines in the sketch. This allows the same codebase to be repurposed for different use cases without needing to replace the whole system.

## 8) Quick installation checklist

Before uploading, confirm these are installed:

- Apollo3 board package
- `SdFat` 2.3.0
- `RTC` 2.0.0
- `Adafruit BusIO` 1.17.4
- `Adafruit Unified Sensor` 1.1.15
- `Adafruit BME280 Library` 2.3.0
- `Adafruit SGP30 Sensor` 2.0.3
- `LTR390` 1.0.9
- `Adafruit SHT31 Library` 2.2.2
- `Adafruit LSM9DS1 Library` 2.2.1
- `SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library` 1.3.2
- `Adafruit BMP3XX Library` 2.1.6