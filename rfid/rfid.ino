/**
RST  - D3
MISO - D6
MOSI - D7
SCK  - D5
SDA  - D8
*/
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "SHT31.h"
#include <ESPAsyncTCP.h>

//todo: Zend naar server als temperatuur veranderd -> rond op server af op 0.5
//todo: Zenden van isAllowed naar server die hem doorpaast naar STM -> dus niet de binaire waardes van de kaarten
//todo: Wemos twee keer zenden naar stm die luchtsluis doet én naar de wemos die lichtkrant regelt

constexpr uint8_t RST_PIN = 0;
constexpr uint8_t SS_PIN = 15;
const int lampje = 16;

MFRC522 mfrc522(SS_PIN, RST_PIN);
const byte myCardUID[4] = { 0xB1, 0xFF, 0x74, 0x1D };

const char* ssid = "coldspot";
const char* password = "123456781";
const char* host = "192.168.54.130";
const uint16_t port = 16789;
//const char clientId = 'b';

AsyncClient* client = nullptr;

float oudeTemp = 0.0;

SHT31 sht(0x44);

void setup() {
  Serial.begin(115200);
  pinMode(lampje, OUTPUT);
  WiFi.begin(ssid, password);

  while (!Serial)
    ;
  Serial.println("Setup");
  SPI.begin();
  mfrc522.PCD_Init();
  delay(500);
  Wire.begin();
  sht.begin();

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client = new AsyncClient();
  if (!client) {  // Check if the client has been correctly created
    Serial.println("Cannot create client");
    return;
  }

  // Define event handlers
  client->onConnect([](void* arg, AsyncClient* c) {
    Serial.println("Connected");
    c->write("Client:4\n");
  },
                    nullptr);

  client->onError([](void* arg, AsyncClient* c, int8_t error) {
    Serial.println("Connect Failed");
    c->close();
  },
                  nullptr);

  client->onDisconnect([](void* arg, AsyncClient* c) {
    Serial.println("Disconnected");
    delete c;
  },
                       nullptr);

  client->onData([](void* arg, AsyncClient* c, void* data, size_t len) {
    Serial.print("Received: ");
    Serial.write((uint8_t*)data, len);
    Serial.println("");
  },
                 nullptr);

  // Connect to the server
  client->connect(host, port);
}

int checkAccess() {  // 0 = niks gedetecteerd 1 = toegestaan 2 = niet toegestaan
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  if (mfrc522.uid.size == 4 && memcmp(mfrc522.uid.uidByte, myCardUID, 4) == 0) {
    return 1;  //Hier dus zenden naar server dat degene erin mag
  }
  return 2;
}

void readSensor() {
  sht.read();
  Serial.print("Temperature:");
  float number = sht.getTemperature();
  Serial.print(number, 1);
  Serial.print("\t");
  Serial.print("Humidity:");
  Serial.println(sht.getHumidity(), 1);
  if (oudeTemp != sht.getTemperature()) {
    char str[50] = "Send:3:";
    snprintf(str + strlen(str), sizeof(str) - strlen(str), "%.1f\n", number);  // '%.2f\n' appends the float and a newline
    if (client->canSend()) {
    client->write(str);
    //Serial.println(str);
    }
  }
  oudeTemp = sht.getTemperature();
}
void loop() {

  if (client && client->connected()) {

    int isAllowed = checkAccess();
    if (isAllowed == 1) {
      Serial.println("Mag naar binnen");
      client->write("Send:15:Inside\n");  //Waar wil ik heen en welke boodschap
      analogWrite(lampje, 10000);
    } else if (isAllowed == 2) {
      Serial.println("Mag niet naar binnen");
      analogWrite(lampje, 0);
      //client->write("Send:12:DeurDicht\n");
    }
    readSensor();
    delay(2000);
    analogWrite(lampje, 50);
  } else {
    Serial.println("Client not connected");
    if (!client->connecting()) {
      client->connect(host, port);
    }
    delay(500);
  }
}
// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}