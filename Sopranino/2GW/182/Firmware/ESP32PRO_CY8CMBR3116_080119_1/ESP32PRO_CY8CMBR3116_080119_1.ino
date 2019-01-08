#include <Wire.h>

// EEPROM library
#include <EEPROM.h>

#define SENSOR_EN 0x00
#define FSS_EN 0x02
#define SENSITIVITY0 0x08
#define SENSITIVITY1 0x09
#define SENSITIVITY2 0x0A
#define SENSITIVITY3 0x0B

#define DEVICE_ID 0x90    // Should return 0xA05 (returns 2 bytes)
#define FAMILY_ID 0x8F
#define SYSTEM_STATUS 0x8A
#define I2C_ADDR 0x37     // Should return 0x37
#define REFRESH_CTRL 0x52
#define SENSOR_EN 0x00    // We should set it to 0xFF for 16 sensors
#define BUTTON_STAT 0xAA  // Here we red the status of the sensors (2 bytes)
#define CTRL_CMD 0x86     // To configure the Capsense
#define CTRL_CMD_STATUS 0x88
#define CTRL_CMD_ERROR 0x89
#define BUTTON_LBR 0x1F
#define SPO_CFG 0x4C      //CS15 configuration address
#define GPO_CFG 0x40
#define CALC_CRC 0x94
#define CONFIG_CRC 0x7E

#define DEBUG true
#define CALIB false

int deviceID = 0;

//control definitions
unsigned long then = 0;
unsigned long now = 0;
unsigned long started = 0;
unsigned long lastRead = 0;
byte interval = 10;
byte touchInterval = 15;
byte calibrate = 0;

// defaults
unsigned int infoTstick[2] = {0, 0};    // serial number and firmware revision
//int xres = D5;  // XRES pin on one of the CY8C201xx chips is connected to Arduino pin 13
//int xres1 = D0;
int pressurePin = A0;
int ledPin = 5; //changed for The thing dev during testing
int ledStatus = 0;
int ledTimer = 1000;
byte touch1 = 0;
byte touch[2] = {0, 0};
byte touchMask[2] = {255, 255};
unsigned int calibrationData[2] = {0, 1023};

uint32_t dataTransferRate = 50; // sending data at 20Hz
uint32_t deltaTransferRate = 0;

// the current address in the EEPROM (i.e. which byte
// we're going to write to next)
int addr = 0;             // Start of instrument info
int addrNet = addr + 12;  // Start of Network info


void setup() {

  setupSerial();

  setupCapsense();

  setupIMU();

  setupWiFi();

  EEPROM.begin(512);
  Serial.println();

  pinMode(ledPin, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly:
      if (millis()-lastRead>touchInterval){
        if (readTouch()){
          lastRead = millis();
        }
      }
}


boolean readTouch(){
  boolean changed = 0;
  byte temp[2] = {0, 0}; int i=0;
  Wire.beginTransmission(I2C_ADDR);
  Wire.write(BUTTON_STAT);
  Wire.endTransmission();

  Wire.requestFrom(I2C_ADDR,2);
  while (Wire.available()) { // slave may send less than requested
//    byte c = Wire.read();
//    temp[i] = c; // receive a byte as character
    temp[i] = Wire.read();
    i++;
  }    
  Wire.endTransmission();
  Serial.print(temp[0]); Serial.println(temp[1]);
  Serial.print(touch[0]); Serial.println(touch[1]);
//  Serial.print("changed: "); Serial.println(changed);

  for (int t = 0; t<2; t++){
    if (temp[t] != touch[t]){
      changed = 1;
//      Serial.print("changed_in: "); Serial.println(changed);
      touch[t] = temp[t];
    }
  }
  return changed;
}
