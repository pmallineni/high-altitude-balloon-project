#include <SdFat.h>
#include <sdios.h>

#include <SPI.h>
#include <RTC.h> // Include RTC library included with the Aruino_Apollo3 core


#include <Wire.h>

// --- Wire ---
const byte PIN_QWIIC_SCL = 8;
const byte PIN_QWIIC_SDA = 9;
TwoWire qwiic(PIN_QWIIC_SDA, PIN_QWIIC_SCL);

// --- Hardware Pins ---
#define HARDWARE_VERSION_MAJOR 1
#define HARDWARE_VERSION_MINOR 0
const byte PIN_PWR_LED = 29;
const byte PIN_MICROSD_CHIP_SELECT = 23;
const byte PIN_QWIIC_POWER = 18;
const byte PIN_MICROSD_POWER = 15;
const byte PIN_STAT_LED = 19;



// -- Sensors ---

#include "sensors.h"

#define NUM_SENSORS 8
SensorBase* sensors[8];
BME280Sensor bmeSensor;
SGP30Sensor sgpSensor;
LTR390Sensor ltrSensor;
IMUSensor imuSensor;
RTCSensor rtcSensor;
SHT31Sensor shtSensor;
BMP388Sensor bmpSensor;
LSM9DS1Sensor lsmSensor;
// --- RTC ---
int logDelay = 1000;  // 1 second
uint64_t lastLogTime;

// --- SD Card ---
#define SD_FAT_TYPE 3                                                               // SD_FAT_TYPE = 0 for SdFat/File, 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_CONFIG SdSpiConfig(PIN_MICROSD_CHIP_SELECT, SHARED_SPI, SD_SCK_MHZ(24))  // 24MHz

#if SD_FAT_TYPE == 1
SdFat32 sd;
File32 myFile;
#elif SD_FAT_TYPE == 2
SdExFat sd;
ExFile myFile;
#elif SD_FAT_TYPE == 3
SdFs sd;
FsFile myFile;
#else   // SD_FAT_TYPE == 0
SdFat sd;
File myFile;
#endif  // SD_FAT_TYPE

// -- flags & important vars --
bool logging = true;
bool printing = true;
bool fileOpen = false;

String csv_header = String();


// --- Input Commands ---

enum Command {
  CMD_NONE,
  CMD_CREATE,
  CMD_SAVE,
  CMD_CLOSE,
  CMD_PRINT,
  CMD_NOPRINT,
  CMD_STARTLOG,
  CMD_STOPLOG,
};

Command parseCommand(const String &cmd) {
  if (cmd.equalsIgnoreCase(String("create"))) return CMD_CREATE;
  if (cmd.equalsIgnoreCase(String("save"))) return CMD_SAVE;
  if (cmd.equalsIgnoreCase(String("close"))) return CMD_CLOSE;
  if (cmd.equalsIgnoreCase(String("print"))) return CMD_PRINT;
  if (cmd.equalsIgnoreCase(String("noprint"))) return CMD_NOPRINT;
  if (cmd.equalsIgnoreCase(String("startlog"))) return CMD_STARTLOG;
  if (cmd.equalsIgnoreCase(String("stoplog"))) return CMD_STOPLOG;
  return CMD_NONE;
}
void handleCommand(const String &cmd) {
  switch (parseCommand(cmd)) {
    case CMD_NONE:
      Serial.println("No Command Executed.");
      break;
    case CMD_CREATE:
      Serial.println("Create Command Executing.");
      createFile();
      break;
    case CMD_SAVE:
      myFile.flush();
      Serial.println("Save Command Executed.");
      break;
    case CMD_CLOSE:
      myFile.close();
      fileOpen = false;
      Serial.println("Close Command Executed.");
      break;
    case CMD_PRINT:
      printing = true;
      Serial.println("Print Command Executed.");
      break;
    case CMD_NOPRINT:
      printing = false;
      Serial.println("Noprint Command Executed.");
      break;
    case CMD_STARTLOG:
      logging = true;
      Serial.println("Startlog Command Executed.");
      break;
    case CMD_STOPLOG:
      logging = false;
      Serial.println("Stoplog Command Executed.");
      break;
  }
}



void powerLEDOn() {
#if (HARDWARE_VERSION_MAJOR >= 1)
  pinMode(PIN_PWR_LED, OUTPUT);
  digitalWrite(PIN_PWR_LED, HIGH);  // Turn the Power LED on
#endif
}
void beginQwiic() {
  pinMode(PIN_QWIIC_POWER, OUTPUT);
  qwiicPowerOn();
  delay(20);
  qwiic.begin();
}

void qwiicPowerOn() {
#if (HARDWARE_VERSION_MAJOR == 0)
  digitalWrite(PIN_QWIIC_POWER, LOW);
#else
  digitalWrite(PIN_QWIIC_POWER, HIGH);
#endif
}
void takeMeasurements() {
  for (int i = 0; i < NUM_SENSORS; i++)
    if (sensors[i])
      sensors[i]->takeMeasurement();
}

void logSensorValues() {
  if (!fileOpen || !myFile) return;

  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensors[i] && myFile)
      sensors[i]->print(myFile);
  }
  myFile.println();
  myFile.flush();

  if (!printing) return; 
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensors[i])
      sensors[i]->print(Serial);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  powerLEDOn();
  beginQwiic();
  powerSD();
  I2CSensor::setWire(qwiic);
  bmeSensor.begin();
  sgpSensor.begin(&bmeSensor.tempEvent, &bmeSensor.humidityEvent);
  ltrSensor.begin();
  imuSensor.begin();
  shtSensor.begin();
 // bmpSensor.begin();
  lsmSensor.begin();
  rtcSensor.begin(); // Don't remove this

  sensors[0] = &rtcSensor; // Don't remove this
  sensors[1] = &bmeSensor;
  sensors[2] = &sgpSensor;
  sensors[3] = &ltrSensor;
  sensors[4] = &imuSensor; // on-board imu
  sensors[5] = &shtSensor; // not using sht
//  sensors[6] = &bmpSensor;
  sensors[7] = &lsmSensor; // external imu
  
  lastLogTime = rtcSensor.rtcMillis();

  int existingSensorIndex = 0;
  for (int i = 0; i < NUM_SENSORS; i++) {
    if (sensors[i]) {
      if (existingSensorIndex > 0) csv_header += String(", ");
      csv_header += sensors[i]->getHeader();
      existingSensorIndex++;
    }
  }

  // See if the card is present and can be initialized:
  if (!sd.begin(SD_CONFIG)) {
    Serial.println("Card failed, or not present. Freezing...");
    sd.errorPrint(&Serial);
    // don't do anything more:
    while (1)
      ;
  }
  Serial.println("SD card initialized.");
  handleCommand(String("create"));
}

void createFile() {
  if (fileOpen) {
    Serial.println("another file is open. Close before creating a new file.");
    return;
  }
  static unsigned int fileIndex = 0;
  char filename[12];
  snprintf(filename, sizeof(filename), "F%u.CSV", fileIndex);
  while (sd.exists(filename)) {
    fileIndex++;
    snprintf(filename, sizeof(filename), "F%u.CSV", fileIndex);
  }
  myFile.open(filename, O_RDWR | O_CREAT | O_EXCL);
  if (!myFile) {
    Serial.println("failed to create file");
    fileIndex++;
    return;
  }
  fileOpen = true;
  myFile.println(csv_header);
  myFile.flush();
  if (printing) {
    Serial.println(csv_header);
  }
  fileIndex++;
}

void loop() {
  unsigned long now = rtcSensor.rtcMillis();
  static String cmd = String("");
  if (Serial.available() > 0) {
    cmd = Serial.readString();
    cmd.trim();
    handleCommand(cmd);
    cmd = String("");
  }
  if (fileOpen && logging && now - lastLogTime >= logDelay) {
    digitalWrite(PIN_STAT_LED, HIGH); 
    takeMeasurements();
    logSensorValues();
    digitalWrite(PIN_STAT_LED, LOW);
    lastLogTime = now;
  }
}

void powerSD() {
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  pinMode(PIN_MICROSD_CHIP_SELECT, OUTPUT);
  digitalWrite(PIN_MICROSD_CHIP_SELECT, HIGH);  //Be sure SD is deselected

  delay(1);

  microSDPowerOn();
}
void microSDPowerOn() {
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, LOW);
}
void microSDPowerOff() {
  pinMode(PIN_MICROSD_POWER, OUTPUT);
  digitalWrite(PIN_MICROSD_POWER, HIGH);
}
