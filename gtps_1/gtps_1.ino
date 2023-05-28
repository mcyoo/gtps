/*
  GPS Logger
  Simple project which logs data from GPS module (NEO 6M) into SD card. 
  Locations are stored as file (yyyyMMdd.txt) and the file will contain one row per location (dd.MM.yyyy HH:mm:ss lat,lon). 
  Location is stored for each interval given as configuration variable 'frequency'. 
  
  Led modes:
  continuous -> error
  blinking -> looking for location
  off -> everything ok
  
  Connecting modules:
  Pin3 -> GPS-module-RX
  Pin4 -> GPS-module-TX
  Pin10 -> SD-module-SS
  Pin11/MOSI -> SD-module-MOSI
  Pin12/MISO -> SD-module-MISO
  Pin13/SCK -> SD-module-SCK
  
  Dependency(TinyGPS++ library): http://arduiniana.org/libraries/tinygpsplus/
*/

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

// Pins used for communicating with GPS module
static const int RXPin = 4, TXPin = 3;
// Status led
static const int GpsLedPin = 9;
// Baud rate of your GPS module (usually 4800 or 9600)
static const uint32_t GPSBaud = 9600;
// How frequently should we save current location (milliseconds)
static const unsigned long frequency = 5000;

// gps object
TinyGPSPlus gps;
// true when we have found location since last restart and file has been opened
boolean opened = false;
// current data file
File dataFile;
// file name
String fileName;
// timestamp of previous location capture. Used to check if we should save this location or skip it
unsigned long previous=0;
// The serial connection to the GPS device
SoftwareSerial gps_serial(RXPin, TXPin);

void setup()
{
  gps_serial.begin(GPSBaud);
  Serial.begin(9600);
  pinMode(GpsLedPin, OUTPUT);
  digitalWrite(GpsLedPin, LOW);

  while(1){
    if (SD.begin(10)) { //SD card SX-10 
      Serial.println("SD card Success..");
      digitalWrite(GpsLedPin, LOW);
      break;
    }
    Serial.println("SD card Fail...");
    digitalWrite(GpsLedPin, HIGH);
    delay(5000);
  }
}

void loop()
{
  // If we have data, decode and log the data
  while (gps_serial.available()){
    if (gps.encode(gps_serial.read())){
      Serial.println("gps encode");
      logInfo();
    }
  }
}

// Help function to pad 0 prefix when valus < 10
void printIntValue(int value)
{
  if(value < 10)
  {
    dataFile.print(F("0"));
  }
  dataFile.print(value);
}

// Log current info if we have valid location
void logInfo()
{
  uint8_t day,hour;
  // Wait until we have location locked!
  if(!gps.location.isValid())
  {
    digitalWrite(GpsLedPin, HIGH);
    delay(20);
    digitalWrite(GpsLedPin, LOW);
    return;
  }

  // Write data row (dd.MM.yyyy HH:mm:ss lat,lon)
  dataFile = SD.open(fileName, FILE_WRITE);

  if (dataFile) {
    // Show that everything is ok
    digitalWrite(GpsLedPin, LOW);

    // When we first get something to log we take file name from that time
    fileName = "";
    fileName += gps.date.year();
    if(gps.date.month() < 10) fileName += "0";
    fileName += gps.date.month();
    if(gps.date.day() < 10) fileName += "0";
    fileName += gps.date.day();
    fileName += ".txt";

    day = gps.date.day();
    hour = gps.time.hour();

    hour = hour + 9;
    if (hour > 24){
        hour = hour - 24;
        day = day + 1;
    }
      
    printIntValue(day);
    dataFile.print(F("."));
    printIntValue(gps.date.month());
    dataFile.print(F("."));
    dataFile.print(gps.date.year());
    dataFile.print(F(" "));      
    printIntValue(hour);
    dataFile.print(F(":"));
    printIntValue(gps.time.minute());
    dataFile.print(F(":"));
    printIntValue(gps.time.second());
    dataFile.print(F(" "));
    dataFile.print(gps.location.lat(), 6);
    dataFile.print(F(","));
    dataFile.print(gps.location.lng(), 6);
    dataFile.println();
    dataFile.close();

    Serial.print(gps.location.lat());
    Serial.print("/");
    Serial.print(gps.location.lng());
    Serial.println("Success!");
    delay(frequency);
  }
  else {
    Serial.println("Fail...");
  }
}
