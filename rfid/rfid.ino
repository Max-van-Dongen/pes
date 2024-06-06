/**@file*/

/**
 * @brief Pin configuration for the RFID and sensor modules.
 * 
 * RST  - D3
 * MISO - D6
 * MOSI - D7
 * SCK  - D5
 * SDA  - D8
 */
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include "SHT31.h"
#include <ESPAsyncTCP.h>

/**
 * @brief RFID reset pin.
 */
constexpr uint8_t RST_PIN = 0;

/**
 * @brief RFID slave select pin.
 */
constexpr uint8_t SS_PIN = 15;

/**
 * @brief Green LED pin.
 */
const int groen = 16;

/**
 * @brief Red LED pin.
 */
const int rood = 1;

/**
 * @brief Flag indicating if there is data to be sent.
 */
bool hasData = false;

/**
 * @brief Flag indicating if a new line is needed in Serial output.
 */
bool newline = false;

/**
 * @brief MFRC522 RFID reader object.
 */
MFRC522 mfrc522(SS_PIN, RST_PIN);

/**
 * @brief Predefined UID for a specific RFID card.
 */
const byte myCardUID[4] = { 0xB1, 0xFF, 0x74, 0x1D };

/**
 * @brief WiFi network SSID.
 */
const char* ssid = "coldspot";

/**
 * @brief WiFi network password.
 */
const char* password = "123456781";

/**
 * @brief Server host address.
 */
const char* host = "10.0.0.3";

/**
 * @brief Server port number.
 */
const uint16_t port = 16789;

/**
 * @brief Pointer to the asynchronous client.
 */
AsyncClient* client = nullptr;

/**
 * @brief SHT31 temperature and humidity sensor object.
 */
SHT31 sht(0x44);

/**
 * @brief Previous temperature reading.
 */
float oudeTemp = 0.0;

/**
 * @brief Previous humidity reading.
 */
float oudeHumid = 0.0;

/**
 * @brief Stores the previous time a check was performed.
 */
unsigned long previousMillis = 0;

/**
 * @brief Interval for performing periodic checks (in milliseconds).
 */
const long interval = 2000;  // 2 seconds

/**
 * @brief Initializes the system, sets up WiFi, and connects to the server.
 */
void setup() {
  Serial.begin(115200);
  pinMode(groen, OUTPUT);
  pinMode(rood, OUTPUT);
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
    newline = true;
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
  client->onConnect(
    [](void* arg, AsyncClient* c) {
      Serial.println("Connected");
      //c->write("Client:4\n");
    },
    nullptr);

  client->onError(
    [](void* arg, AsyncClient* c, int8_t error) {
      Serial.printf("Connect errored (%i)\r\n", error);
      c->close();
    },
    nullptr);

  client->onDisconnect(
    [](void* arg, AsyncClient* c) {
      Serial.println("Disconnected");
      delete c;
    },
    nullptr);

  client->onData(
    [](void* arg, AsyncClient* c, void* data, size_t len) {
      Serial.print("Received: ");
      Serial.write((uint8_t*)data, len);
      Serial.println("");

      c->write("");
    },
    nullptr);

  // Connect to the server
  client->connect(host, port);
}

/**
 * @brief Checks access by reading the RFID card.
 * @return Access status: 0 = no card detected, 1 = access allowed, 2 = access denied.
 */
int checkAccess() {  // 0 = niks gedetecteerd 1 = toegestaan 2 = niet toegestaan
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  if (mfrc522.uid.size == 4 && memcmp(mfrc522.uid.uidByte, myCardUID, 4) == 0) {
    return 1;  // Hier dus zenden naar server dat degene erin mag
  }
  return 2;
}

/**
 * @brief Reads temperature and humidity from the sensor and sends the data to the server if significant changes are detected.
 */
void readSensor() {
  if (client->space() < 40) {
    Serial.printf("Buffer to short for temp (%i)\r\n", client->space());
    return;
  }
  sht.read();

  Serial.print("Temperature:");
  float nieuwTemp = sht.getTemperature();
  Serial.print(nieuwTemp, 2);

  const float drempelWaardeTempHoog = oudeTemp + 0.09;
  const float drempelWaardeTempLaag = oudeTemp - 0.09;

  if (drempelWaardeTempHoog <= nieuwTemp || drempelWaardeTempLaag >= nieuwTemp) {

    char str[50] = "Send:3:Temp:";                                                // size 17 - 18 chars (with or without null)
    snprintf(str + strlen(str), sizeof(str) - strlen(str), "%.1f\n", nieuwTemp);  // '%.1f\n' appends the float and a newline

    int wordVerstuurd = client->add(str, strlen(str), 0);
    hasData = true;

    if (wordVerstuurd == strlen(str)) {
      Serial.print(" (add)");
      oudeTemp = nieuwTemp;
    } else if (wordVerstuurd == 0) {
      Serial.print(" (won't)");
    } else {
      Serial.print(" (part)");
    }

  } else {
    Serial.print("\t");
  }

  float nieuwHumid = sht.getHumidity();
  Serial.print("\tHumidity:");
  Serial.print(nieuwHumid, 2);

  const float drempelWaardeHumHoog = oudeHumid + 0.09;
  const float drempelWaardeHumLaag = oudeHumid - 0.09;

  if (drempelWaardeHumHoog <= nieuwHumid || drempelWaardeHumLaag >= nieuwHumid) {

    char str[50] = "Send:3:Humid:";  // 18 - 19 without null
    snprintf(str + strlen(str), sizeof(str) - strlen(str), "%.1f\n", nieuwHumid);
    int wordVerstuurd = client->add(str, strlen(str), 0);
    hasData = true;

    if (wordVerstuurd == strlen(str)) {
      Serial.print(" (add) ");
      oudeHumid = nieuwHumid;
    } else if (wordVerstuurd == 0) {
      Serial.print(" (won't)");
    } else {
      Serial.print(" (part) ");
    }
  }
  Serial.println();
}

/**
 * @brief Handles RFID access control, checking the card and sending the access status to the server.
 */
void handleRFID() {
  unsigned long currentMillis = millis();

  // if (currentMillis - previousMillis >= interval) {
  previousMillis = currentMillis;

  int isAllowed = checkAccess();

  if (isAllowed == 1) {
    Serial.print("Mag naar binnen");
    digitalWrite(groen, 1);
    if (client->write("Send:12:Rfid\n")) {
      Serial.println(" (send)");
    } else {
      Serial.println();
    }
  } else if (isAllowed == 2) {
    Serial.println("Mag niet naar binnen");
    digitalWrite(rood, 1);
    if (client->write("Send:9:RfidF\n")) {
      Serial.println(" (send)");
    } else {
      Serial.println();
    }
  }
  // }
}

/**
 * @brief Main loop that checks client connection, handles RFID and sensor data, and sends data to the server if available.
 */
void loop() {

  if (!client || !client->connected()) {
    Serial.println("Client not connected");
    if (!client->connecting()) {
      client->connect(host, port);
    }
    delay(500);
    return;
  }

  if (!client->canSend()) {
    Serial.print(".");
    delay(800);
    newline = true;

    return;
  }
  if (newline) {
    Serial.println();
    newline = false;
  }

  handleRFID();

  readSensor();

  if (hasData) {

    if (client->send()) {
      Serial.println("sended");
    } else {
      Serial.println("send error?");
    }

    hasData = false;
  }

  delay(1000);
  analogWrite(groen, 0);
  analogWrite(rood, 0);
}

/**
 * @brief Helper routine to dump a byte array as hex values to Serial.
 * @param buffer Pointer to the byte array.
 * @param bufferSize Size of the byte array.
 */
void dump_byte_array(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
