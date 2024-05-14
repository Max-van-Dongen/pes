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

//todo: Zend naar server als temperatuur veranderd -> rond op server af op 0.5
//todo: Zenden van isAllowed naar server die hem doorpaast naar STM -> dus niet de binaire waardes van de kaarten
//todo: Wemos twee keer zenden naar stm die luchtsluis doet én naar de wemos die lichtkrant regelt

constexpr uint8_t RST_PIN = 0;
constexpr uint8_t SS_PIN = 15;

MFRC522 mfrc522(SS_PIN, RST_PIN);
const byte myCardUID[4] = { 0xB1, 0xFF, 0x74, 0x1D };

const char* ssid = "coldspot";
const char* password = "123456781";
const char* serverIP = "192.168.12.207";
const int serverPort = 6789;
const char clientId = 'b';
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

struct response {
  char server;
  char datatype;
  String data;
};

struct response sendmsg(char dest, char *data) {
  
  char buff[50];
  sprintf(buff, "%c%c%s\0", clientId, dest, data);
  client.write(buff);
  
  delay(40);

  struct response res;
  res.server = client.read();
  res.datatype = client.read();
  Serial.println("Hij hoort nu te lezen");

  if (res.datatype == '2') {
    res.data = client.readString();
  }
  
  return res;
}

int checkAccess() {  // 0 = niks gedetecteerd 1 = toegestaan 2 = niet toegestaan
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  if (mfrc522.uid.size == 4 && memcmp(mfrc522.uid.uidByte, myCardUID, 4) == 0) {
    struct response res = sendmsg('a',"Hij mag naar binnen");
    return 1;//Hier dus zenden naar server dat degene erin mag
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
  if (!client.connect(serverIP, serverPort)) { 
    Serial.println("Verbinding mislukt. Opnieuw proberen...");
    delay(5000);
    return;
  }
  
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