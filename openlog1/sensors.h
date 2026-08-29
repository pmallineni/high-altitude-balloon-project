#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SGP30.h>
#include <LTR390.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_BMP3XX.h>
#include <bmp3.h>
#include <bmp3_defs.h>
#include <Adafruit_LSM9DS1.h>


#include <ICM_20948.h>
#include <RTC.h>

class SensorBase {
  public: 
    virtual void begin() = 0;
    virtual void takeMeasurement() = 0;
    virtual void print(Print &out) = 0;
    virtual String getHeader() = 0;
    virtual ~SensorBase() {}
};

class I2CSensor : public SensorBase {
  protected: 
    static TwoWire *wire;
  public: 
    static void setWire(TwoWire &w) { wire = &w; }
};

class BME280Sensor : public I2CSensor {
  Adafruit_BME280 bme; //I2C connection
  public: 
    sensors_event_t tempEvent, pressureEvent, humidityEvent;
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;
};

class SGP30Sensor : public I2CSensor {
  Adafruit_SGP30 sgp; 

  // Optional linked temperature/humidity for absolute humidity
  sensors_event_t *tempEventPtr = nullptr;
  sensors_event_t *humidityEventPtr = nullptr;

  public: 
    void begin() override;
    void begin(sensors_event_t *t, sensors_event_t *h); //linked to BME280
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;
    static uint32_t getAbsoluteHumidity(float temperature, float humidity);
    float temperature() const { return tempEventPtr ? tempEventPtr->temperature : NAN; }
    float humidity() const { return humidityEventPtr ? humidityEventPtr->relative_humidity : NAN; }

};

class LTR390Sensor : public I2CSensor {
  float currentLux = NAN;
  float currentUVI = NAN;
  LTR390 *ltr390;
  public: 
    LTR390Sensor() : ltr390(nullptr) {}
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;

};

class SHT31Sensor : public I2CSensor {
  float currentTemp = NAN;
  float currentHumidity = NAN;
  Adafruit_SHT31 *sht31;
  public: 
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;
};

class BMP388Sensor : public I2CSensor {
  double temperature = NAN;
  double pressure = NAN;
  float altitude = NAN;
  Adafruit_BMP3XX bmp; 
  #define SEALEVELPRESSURE_HPA (1013.25)
  public: 
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;
};

class LSM9DS1Sensor : public I2CSensor {
  Adafruit_LSM9DS1 *lsm;
  sensors_event_t a, m, g, temp;
  #define LSM9DS1_SCK A5
  #define LSM9DS1_MISO 12
  #define LSM9DS1_MOSI A4
  #define LSM9DS1_XGCS 6
  #define LSM9DS1_MCS 5
  void setupSensor(); 
  public: 
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;

};



class IMUSensor : public SensorBase {
  ICM_20948_SPI myICM;  // If using SPI create an ICM_20948_SPI object
  const byte PIN_IMU_POWER = 27; // The Red SparkFun version of the OLA (V10) uses pin 27
  //const byte PIN_IMU_POWER = 22; // The Black SparkX version of the OLA (X04) uses pin 22
  const byte PIN_IMU_INT = 37;
  const byte PIN_IMU_CHIP_SELECT = 44;
  const byte PIN_SPI_SCK = 5;
  const byte PIN_SPI_CIPO = 6;
  const byte PIN_SPI_COPI = 7;
  public:
    void begin() override;
    void takeMeasurement() override;
    void print(Print &out) override;
    String getHeader() override;
    
    void printFormattedFloat(Print &out, float val, uint8_t leading, uint8_t decimals);
    bool enableCIPOpullUp();
    void imuPowerOn(); 
    void imuPowerOff();


};

class RTCSensor : public SensorBase {
  Apollo3RTC myRTC; 
  //#define MANUALLY_SET_TIME
  int calculateDayOfYear(int day, int month, int year);
  bool parseAndSetRTC(String timeString);
  public: 
    void begin() override;
    void takeMeasurement() override;
    void print( Print &out) override;
    String getHeader() override;
    uint64_t rtcMillis();


};

#endif

