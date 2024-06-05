// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "SHT31.h"
#include <Wire.h>
//#include <DebugPrintMacros.h>
//#define DEBUG_MORE 1
//#define ASYNC_TCP_DEBUG Serial.printf;


#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

// Uncomment according to your hardware type
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

// Defining size, and output pins
#define MAX_DEVICES 4
#define CS_PIN 15

// Create a new instance of the MD_Parola class with hardware SPI connection
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

const char* ssid = "coldspot";
const char* password = "123456781";
const char* host = "10.0.0.3";
const uint16_t port = 16789;
bool bigDiff = false;
AsyncClient* client = nullptr;

SHT31 sht(0x44);

float temperatureOutside = 0.0;
float temperatureInside = 0.0;
float humidityInside = 0.0;
unsigned long previousMillis = 0;
const long interval = 200;  // ms
int tempDuration = 10000;   /*ms*/
bool showTemp = true;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  sht.begin();
  Wire.begin();
  SPI.begin();

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
    c->write("Client:3\n");
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
    Serial.print("\nReceived: ");
    //Serial.write((uint8_t*)data, len);
    //Serial.println();
    String receivedData = String((char*)data).substring(0, len);
    processReceivedString(receivedData);
    // float temperature = receivedData.toFloat();
    // displayTemp(temperature);
    // Serial.println("");
  },
                 nullptr);


  // Connect to the server
  client->connect(host, port);
  // Intialize the object
  myDisplay.begin();

  // Set the intensity (brightness) of the display (0-15)
  myDisplay.setIntensity(10);

  // Clear the display
  myDisplay.displayClear();

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(true);
  myDisplay.print("!INIT!");
  myDisplay.setInvert(false);
  delay(750);
  myDisplay.displayClear();
  // myDisplay.displayText("Joe mamma", PA_CENTER, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);


  // float temp = 5;
  // while (temp <= 80) {
  //   displayTemp(temp);
  //   temp += 1;
  //   delay(100);
  // }
  // displayFire(true);
}

void displayFire(int state) {
  while (true) {
    myDisplay.setInvert(true);
    myDisplay.print("BRAND");
    delay(750);
    myDisplay.setInvert(false);
    myDisplay.print("BRAND");
    delay(750);
  }
}


void processReceivedString(String input) {
  int tempIndex = input.indexOf("Temp:");
  int humidIndex = input.indexOf("Humid:");

  if (tempIndex != -1) {
    int startIndex = tempIndex + 5;  // Move past "Temp:"
    int endIndex = (humidIndex != -1 && humidIndex > startIndex) ? humidIndex : input.indexOf(' ', startIndex);
    if (endIndex == -1) endIndex = input.length();  // If no space found and no Humid:, use the end of the string
    String tempValueString = input.substring(startIndex, endIndex);
    temperatureOutside = tempValueString.toFloat();

    Serial.print("temperature outside: ");
    Serial.println(temperatureOutside, 1);
  }

  if (humidIndex != -1) {
    int startIndex = humidIndex + 6;  // Move past "Humid:"
    int endIndex = input.indexOf(' ', startIndex);
    if (endIndex == -1) endIndex = input.length();  // If no space found, use the end of the string
    String humidValueString = input.substring(startIndex, endIndex);
    humidityInside = humidValueString.toFloat();

    Serial.print("humidity: ");
    Serial.println(humidityInside, 1);
  }
}

void displayTemp(float temperatureOutside) {
  char tempDisplay[10];
  sprintf(tempDisplay, "%2.1f%cC", temperatureOutside, '\xB0');
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(tempDisplay);
}

float oudeTempInside = 0.0;
float oudeHumidInside = 0.0;

void readSensor() {
  sht.read();
  Serial.print("Temperature Inside:");
  float nieuwTempInside = sht.getTemperature();
  Serial.print(nieuwTempInside, 2);

  const float drempelWaardeTempHoogInside = oudeTempInside + 0.09;
  const float drempelWaardeTempLaagInside = oudeTempInside - 0.09;

  if (drempelWaardeTempHoogInside <= nieuwTempInside || drempelWaardeTempLaagInside >= nieuwTempInside) {

    char str[50] = "Send:9:";
    snprintf(str + strlen(str), sizeof(str) - strlen(str), "TempI:%.1f\n", nieuwTempInside);  // '%.2f\n' appends the float and a newline

    if (client->canSend()) {
      temperatureInside = nieuwTempInside;
      oudeTempInside = nieuwTempInside;
      client->write(str);
      Serial.print(" (send)");
    } else {
      Serial.print(" (err)");
    }
  } else {
    Serial.print("\t");
  }

  float nieuwHumidInside = sht.getHumidity();
  Serial.print("\tHumidity inside:");
  Serial.print(nieuwHumidInside, 2);

  const float drempelWaardeHumHoogInside = oudeHumidInside + 0.09;
  const float drempelWaardeHumLaagInside = oudeHumidInside - 0.09;

  if (drempelWaardeHumHoogInside <= nieuwHumidInside || drempelWaardeHumLaagInside >= nieuwHumidInside) {
    char str[50] = "Send:9:";
    snprintf(str + strlen(str), sizeof(str) - strlen(str), "HumidI:%.1f\n", nieuwHumidInside);  // '%.2f\n' appends the float and a newline
    if (client->canSend()) {
      client->write(str);
      oudeHumidInside = nieuwHumidInside;

      Serial.print(" (send)");
    } else {
      Serial.print(" (err)");
    }
  }

  Serial.println();
}

void loop() {

  unsigned long currentMillis = millis();
  if (client && client->connected()) {
  } else {
    Serial.println("Client not connected");
    if (!client->connecting()) {
      client->connect(host, port);
    }
    delay(500);
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    showTemp = (millis() % tempDuration * 2) >= tempDuration;

    char textToDisplay[10];

    if (showTemp) {
      Serial.print("T");

      sprintf(textToDisplay, "%2.1f%cC", temperatureOutside, '\xB0');
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print(textToDisplay);
    } else {
      Serial.print("H");

      sprintf(textToDisplay, "%.1f%%", humidityInside);
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print(textToDisplay);
    }
  }
  readSensor();
  //NOT TESTED YET
  float diff = abs(temperatureOutside - temperatureInside);
  if (diff >= 2 && bigDiff == 1) {
    bigDiff = 0;
    char str[50] = "Send:12:Temp1\n";
    client->write(str);
    Serial.println("Temp1");
    myDisplay.setInvert(true);
    myDisplay.print("TEMP1");
    myDisplay.setInvert(false);

    delay(250);
  }
  if (diff <= 2 && bigDiff == 0) {
    bigDiff = 1;
    char str[50] = "Send:12:Temp0\n";
    client->write(str);
    Serial.println("Temp0");
    myDisplay.setInvert(true);
    myDisplay.print("TEMP0");
    myDisplay.setInvert(false);
    delay(250);
  }
  //END NOT TESTED YET
  delay(2000);
}