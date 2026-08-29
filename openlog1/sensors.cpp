#include "sensors.h"
#include <Arduino.h>

TwoWire* I2CSensor::wire = nullptr;

void BME280Sensor::begin() {
  if (!wire || !bme.begin(BME280_ADDRESS, wire)) {
  Serial.println("BME280 not found!");
  while (1) delay(10);
  }
}
void BME280Sensor::takeMeasurement() {
  bme.getTemperatureSensor()->getEvent(&tempEvent);
  bme.getPressureSensor()->getEvent(&pressureEvent);
  bme.getHumiditySensor()->getEvent(&humidityEvent);
}
void BME280Sensor::print(Print &out) {
  out.print(tempEvent.temperature);
  out.print(", ");
  out.print(humidityEvent.relative_humidity);
  out.print(", ");
  out.print(pressureEvent.pressure);
  out.print(", ");
}
String BME280Sensor::getHeader() {
  return String("Temp (°C), Humid. (%RH), Press. (hPa)");
}


void SGP30Sensor::begin() {
  if (!wire || !sgp.begin(wire)) {
    Serial.println("SGP30 not found!");
    while (1) delay(10);
  }
}
void SGP30Sensor::begin(sensors_event_t *t, sensors_event_t *h) {
  tempEventPtr = t; 
  humidityEventPtr = h;
  begin();
}
void SGP30Sensor::takeMeasurement() {
  if (tempEventPtr && humidityEventPtr) {
    sgp.setHumidity(
      getAbsoluteHumidity(tempEventPtr->temperature, humidityEventPtr->relative_humidity)
    );
  }
  sgp.IAQmeasure();
}
void SGP30Sensor::print(Print &out) {
  out.print(sgp.TVOC);
  out.print(", ");
  out.print(sgp.eCO2);
  out.print(", ");
}
String SGP30Sensor::getHeader() {
  return String("TVOC (ppb), eCO2 (ppm)");
}

/* return absolute humidity [mg/m^3] with approximation formula
* @param temperature [°C]
* @param humidity [%RH]
*/
uint32_t SGP30Sensor::getAbsoluteHumidity(float temperature, float humidity) {
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.16 
    // https://files.seeedstudio.com/wiki/Grove-VOC_and_eCO2_Gas_Sensor-SGP30/res/Sensirion_Gas_Sensors_SGP30_Driver-Integration-Guide_HW_I2C.pdf
    const float absoluteHumidity = 216.7f * ((humidity / 100.0f) * 6.112f * exp((17.62f * temperature) / (243.12f + temperature)) / (273.15f + temperature)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity); // [mg/m^3]
    return absoluteHumidityScaled;
}


void LTR390Sensor::begin() {
  if (!wire) {
    Serial.println("I2C bus not configured!");
    while (1) delay(10);
  }
  ltr390 = new LTR390(wire);
  if (!ltr390->init()) {
    Serial.println("LTR390 not connected!");
    while(1) delay(10);
  }
  ltr390->setMode(LTR390_MODE_UVS);
  ltr390->setGain(LTR390_GAIN_18);
  ltr390->setResolution(LTR390_RESOLUTION_20BIT);
  //ltr390->setThresholds(100, 1000);
  //ltr390->configInterrupt(true, mode);
}



void LTR390Sensor::takeMeasurement() {
    ltr390->setGain(LTR390_GAIN_18);                  //Recommended for UVI - x18
    ltr390->setResolution(LTR390_RESOLUTION_20BIT);   //Recommended for UVI - 20-bit
    ltr390->setMode(LTR390_MODE_UVS); 
    delay(50);
    currentUVI = ltr390->getUVI();

    ltr390->setGain(LTR390_GAIN_3);                   //Recommended for Lux - x3
    ltr390->setResolution(LTR390_RESOLUTION_18BIT);   //Recommended for Lux - 18-bit
    ltr390->setMode(LTR390_MODE_ALS);
    delay(50);
    currentLux = ltr390->getLux();


}
void LTR390Sensor::print(Print &out) {
  out.print(currentUVI);
  out.print(", ");
  out.print(currentLux);
  out.print(", ");

}
String LTR390Sensor::getHeader() {
  return String("UV (UVI), Amb. Light (Lux)");
}

void SHT31Sensor::begin() {
  if (!wire) {
    Serial.println("I2C bus not configured!");
    while (1) delay(10);
  }
  sht31 = new Adafruit_SHT31(wire);
  if (! sht31->begin(0x44)) {
    Serial.println("Couldn't find SHT31");
    while (1) delay(10);
  }
}
void SHT31Sensor::takeMeasurement() {
  currentTemp = sht31->readTemperature();
  currentHumidity = sht31->readHumidity();

}
void SHT31Sensor::print(Print &out) {
  out.print(currentTemp);
  out.print(", ");
  out.print(currentHumidity);
  out.print(", ");
}
String SHT31Sensor::getHeader() {
  return String(F("shtTemp (°C), shtHumidity (RH%)"));
}

void BMP388Sensor::begin() {
  if (!wire) {
    Serial.println("I2C bus not configured!");
    while (1) delay(10);
  }
  if (! bmp.begin_I2C(BMP3XX_DEFAULT_ADDRESS, wire )) {
    Serial.println("Could not find a valid BMP3 sensor, check wiring!");
    while (1) delay(10);
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
}
void BMP388Sensor::takeMeasurement() {
  bmp.performReading();
  temperature = bmp.temperature;
  pressure = bmp.pressure / 100.0; // hPa
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  
}
void BMP388Sensor::print(Print &out) {
  out.print(temperature);
  out.print(", ");
  out.print(pressure);
  out.print(", ");
  out.print(altitude);
  out.print(", ");
}
String BMP388Sensor::getHeader() {
  return String(F("bmp388Temp (°C), bmp388Pressure (hPa), bmp388Approx. Alt"));
}

void LSM9DS1Sensor::begin() {
  if (!wire) {
    Serial.println("I2C bus not configured!");
    while (1) delay(10);
  }
  lsm = new Adafruit_LSM9DS1(wire);
  // Try to initialise and warn if we couldn't detect the chip
  if (!lsm->begin())
  {
    Serial.println("Oops ... unable to initialize the LSM9DS1. Check your wiring!");
    while (1) delay(10);
  }
  Serial.println("Found LSM9DS1 9DOF");

  // helper to just set the default scaling we want, see above!
  setupSensor();
}
void LSM9DS1Sensor::setupSensor() 
{
  // 1.) Set the accelerometer range
  lsm->setupAccel(lsm->LSM9DS1_ACCELRANGE_2G, lsm->LSM9DS1_ACCELDATARATE_10HZ);
  //lsm->setupAccel(lsm->LSM9DS1_ACCELRANGE_4G, lsm->LSM9DS1_ACCELDATARATE_119HZ);
  //lsm->setupAccel(lsm->LSM9DS1_ACCELRANGE_8G, lsm->LSM9DS1_ACCELDATARATE_476HZ);
  //lsm->setupAccel(lsm->LSM9DS1_ACCELRANGE_16G, lsm->LSM9DS1_ACCELDATARATE_952HZ);
  
  // 2.) Set the magnetometer sensitivity
  lsm->setupMag(lsm->LSM9DS1_MAGGAIN_4GAUSS);
  //lsm->setupMag(lsm->LSM9DS1_MAGGAIN_8GAUSS);
  //lsm->setupMag(lsm->LSM9DS1_MAGGAIN_12GAUSS);
  //lsm->setupMag(lsm->LSM9DS1_MAGGAIN_16GAUSS);

  // 3.) Setup the gyroscope
  lsm->setupGyro(lsm->LSM9DS1_GYROSCALE_245DPS);
  //lsm->setupGyro(lsm->LSM9DS1_GYROSCALE_500DPS);
  //lsm->setupGyro(lsm->LSM9DS1_GYROSCALE_2000DPS);
}
void LSM9DS1Sensor::takeMeasurement() {
  lsm->read();
  lsm->getEvent(&a, &m, &g, &temp); 
}
void LSM9DS1Sensor::print(Print &out) {
  out.print(a.acceleration.x);
  out.print(", ");
  out.print(a.acceleration.y);
  out.print(", ");
  out.print(a.acceleration.z);
  out.print(", ");
  out.print(g.gyro.x);
  out.print(", ");
  out.print(g.gyro.y);
  out.print(", ");
  out.print(g.gyro.z);
  out.print(", ");
  out.print(m.magnetic.x);
  out.print(", ");
  out.print(m.magnetic.y);
  out.print(", ");
  out.print(m.magnetic.z);
  out.print(", ");

}
String LSM9DS1Sensor::getHeader() {
  return String(F("(m/s^2) accX, accY, accZ, (rad/s) gyrX, gyrY, gyrZ, (µT) magX, magY, magZ"));
}


void IMUSensor::begin() {
  pinMode(PIN_IMU_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_IMU_CHIP_SELECT, HIGH); //Be sure IMU is deselected
  
  enableCIPOpullUp(); // Enable CIPO pull-up on the OLA

  //There is a quirk in v2.1 of the Apollo3 mbed core which means that the first SPI transaction will
  //disable the pull-up on CIPO. We need to do a fake transaction and then re-enable the pull-up
  //to work around this...
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Do a fake transaction
  SPI.endTransaction();
  enableCIPOpullUp(); // Re-enable the CIPO pull-up

  
  //Reset ICM by power cycling it
  imuPowerOff();

  delay(10);

  imuPowerOn(); // Enable power for the OLA IMU

  delay(100); // Wait for the IMU to power up

  bool initialized = false;
  while( !initialized ){

    myICM.begin( PIN_IMU_CHIP_SELECT, SPI ); 


    Serial.print( F("Initialization of the sensor returned: ") );
    Serial.println( myICM.statusString() );
    if( myICM.status != ICM_20948_Stat_Ok ){
      Serial.println( "Trying again..." );
      delay(500);
    }else{
      initialized = true;
    }
  }
}
void IMUSensor::takeMeasurement() {
  if (myICM.dataReady()) {
    myICM.getAGMT(); // The values are only updated when you call 'getAGMT'
  }
}
void IMUSensor::print(Print &out) {
  out.print(myICM.accX()); 
  out.print(", ");
  out.print(myICM.accY());
  out.print(", ");
  out.print(myICM.accZ());
  out.print(", ");
  out.print(myICM.gyrX());
  out.print(", ");
  out.print(myICM.gyrY()); 
  out.print(", ");
  out.print(myICM.gyrZ());
  out.print(", ");
  out.print(myICM.magX()); 
  out.print(", ");
  out.print(myICM.magY());
  out.print(", ");
  out.print(myICM.magZ());
  out.print(", ");
  out.print(myICM.temp());
  out.print(", ");
}
void IMUSensor::printFormattedFloat(Print &out, float val, uint8_t leading, uint8_t decimals){
  float aval = abs(val);
  if(val < 0){
    out.print("-");
  }else{
    out.print(" ");
  }
  for( uint8_t indi = 0; indi < leading; indi++ ){
    uint32_t tenpow = 0;
    if( indi < (leading-1) ){
      tenpow = 1;
    }
    for(uint8_t c = 0; c < (leading-1-indi); c++){
      tenpow *= 10;
    }
    if( aval < tenpow){
      out.print("0");
    }else{
      break;
    }
  }
  if(val < 0){
    out.print(-val, decimals);
  }else{
    out.print(val, decimals);
  }
}
String IMUSensor::getHeader() {
  return String(F("(mG) accX, accY, accZ, (°/s) gyrX, gyrY, gyrZ, (µT) magX, magY, magZ, imuTemp (°C)"));
}
void IMUSensor::imuPowerOn() {
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, HIGH);
}
void IMUSensor::imuPowerOff() {
  pinMode(PIN_IMU_POWER, OUTPUT);
  digitalWrite(PIN_IMU_POWER, LOW);
}
bool IMUSensor::enableCIPOpullUp() {
  //Add 1K5 pull-up on CIPO
  am_hal_gpio_pincfg_t cipoPinCfg = g_AM_BSP_GPIO_IOM0_MISO;
  cipoPinCfg.ePullup = AM_HAL_GPIO_PIN_PULLUP_1_5K;
  pin_config(PinName(PIN_SPI_CIPO), cipoPinCfg);
  return (true);
}

void RTCSensor::begin() {
  #if defined(MANUALLY_SET_TIME)
  // Manually set RTC date and time
  Serial.println("Enter Time e.g. 2026-01-19 13:57:37.579");
  String timeString;

  // Wait for a non-empty string
  do {
    timeString = Serial.readString();
    delay(10);
  } while (timeString.length() == 0);

  // Keep trying until parsing succeeds
  while (!parseAndSetRTC(timeString)) {
    Serial.println(timeString);
    timeString = Serial.readString();
  }
  #else
    myRTC.setToCompilerTime();
  #endif
}
bool RTCSensor::parseAndSetRTC(String timeString) {
    // Expected format: "YYYY-MM-DD HH:MM:SS.mmm"
    int year, month, day, hour, minute, second, ms = 0;

    // Try parsing with milliseconds
    int count = sscanf(timeString.c_str(), "%d-%d-%d %d:%d:%d.%d", 
                       &year, &month, &day, &hour, &minute, &second, &ms);

    if (count < 6) {
        return false;  // parsing failed
    }

    // If milliseconds were not provided, default to 0
    if (count == 6) {
        ms = 0;
    }

    // Convert full year to 2-digit format for RTC
    int yy = year % 100;

    // Convert milliseconds to hundredths of seconds for myRTC.setTime()
    int hund = ms / 10;  // 0-999 ms → 0-99 hundredths

    myRTC.setTime(hund, second, minute, hour, day, month, yy);

    return true;  // success
}
int RTCSensor::calculateDayOfYear(int day, int month, int year)
{
  // Given a day, month, and year (4 digit), returns
  // the day of year. Errors return 999.

  int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

  // Verify we got a 4-digit year
  if (year < 1000) {
    return 999;
  }

  // Check if it is a leap year, this is confusing business
  // See: https://support.microsoft.com/en-us/kb/214019
  if (year % 4  == 0) {
    if (year % 100 != 0) {
      daysInMonth[1] = 29;
    }
    else {
      if (year % 400 == 0) {
        daysInMonth[1] = 29;
      }
    }
  }

  // Make sure we are on a valid day of the month
  if (day < 1)
  {
    return 999;
  } else if (day > daysInMonth[month - 1]) {
    return 999;
  }

  int doy = 0;
  for (int i = 0; i < month - 1; i++) {
    doy += daysInMonth[i];
  }

  doy += day;
  return doy;
}
uint64_t RTCSensor::rtcMillis()
{
  myRTC.getTime();
  uint64_t millisToday = 0;
  int dayOfYear = calculateDayOfYear(myRTC.dayOfMonth, myRTC.month, myRTC.year + 2000);
  millisToday += ((uint64_t)dayOfYear * 86400000ULL);
  millisToday += ((uint64_t)myRTC.hour * 3600000ULL);
  millisToday += ((uint64_t)myRTC.minute * 60000ULL);
  millisToday += ((uint64_t)myRTC.seconds * 1000ULL);
  millisToday += ((uint64_t)myRTC.hundredths * 10ULL);

  return (millisToday);
}
void RTCSensor::takeMeasurement() {
  return; // its not really a sensor, but I can't be bothered to change inheritance hierarchy
}
void RTCSensor::print( Print &out) {
    if (myRTC.month < 10) out.print('0');
    out.print(myRTC.month);
    out.print('/');

    if (myRTC.dayOfMonth < 10) out.print('0');
    out.print(myRTC.dayOfMonth);
    out.print('/');

    if (myRTC.year < 10) out.print('0');
    out.print(myRTC.year);

    out.print(", ");

    out.print(myRTC.hour);
    out.print(':');

    if (myRTC.minute < 10) out.print('0');
    out.print(myRTC.minute);
    out.print(':');

    if (myRTC.seconds < 10) out.print('0');
    out.print(myRTC.seconds);
    out.print('.');

    if (myRTC.hundredths < 10) out.print('0');
    out.print(myRTC.hundredths);

    out.print(", ");
}
String RTCSensor::getHeader() {
  return String(F("Date, Time"));
}


