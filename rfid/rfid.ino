/**
RST  - D3
MISO - D6
MOSI - D7
SCK  - D5
SDA  - D8
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "SHT31.h"

constexpr uint8_t RST_PIN = 0;
constexpr uint8_t SS_PIN = 15;

MFRC522 mfrc522(SS_PIN, RST_PIN);
const byte myCardUID[4] = { 0xB1, 0xFF, 0x74, 0x1D };

const char* ssid = "coldspot1";
const char* password = "123456781";
const char* serverIP = "192.168.230.130";
const int serverPort = 6789;
WiFiClient client;

SHT31 sht(0x44);

void setup() {
  Serial.begin(115200); 
  while (!Serial)
    ;
  Serial.println("Setup");
  SPI.begin();
  mfrc522.PCD_Init();
  delay(500);
  Wire.begin();
  sht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());

  Serial.println("Setup done");
}


int checkAccess() {  // 0 = niks gedetecteerd 1 = toegestaan 2 = niet toegestaan
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  if (mfrc522.uid.size == 4 && memcmp(mfrc522.uid.uidByte, myCardUID, 4) == 0) {
    return 1;
  }
  return 2;
}

void readSensor() {
  sht.read();
  Serial.print("Temperature:");
  Serial.print(sht.getTemperature(), 1);
  Serial.print("\t");
  Serial.print("Humidity:");
  Serial.println(sht.getHumidity(), 1);
}
void loop() {
  int isAllowed = checkAccess();
  if (isAllowed == 1) {
    Serial.println("Mag naar binnen");
  } else if (isAllowed == 2) {
    Serial.println("Mag niet naar binnen");
  }
  readSensor();
  delay(500);
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}