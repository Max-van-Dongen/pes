/**@file*/

/** @brief Includes the required Arduino libraries. */
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "SHT31.h"
#include <Wire.h>
//#include <DebugPrintMacros.h>
//#define DEBUG_MORE 1
//#define ASYNC_TCP_DEBUG Serial.printf;

/** @brief Includes the ESP8266 WiFi library. */
#include <ESP8266WiFi.h>

/** @brief Includes the ESPAsyncTCP library. */
#include <ESPAsyncTCP.h>

// Uncomment according to your hardware type
/** @brief Defines the hardware type for MD_MAX72XX library. */
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
// #define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW

/** @brief Defines the number of devices and output pins. */
#define MAX_DEVICES 4
#define CS_PIN 15

/** 
 * @brief Creates a new instance of the MD_Parola class with hardware SPI connection.
 */
MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

/** @brief WiFi network SSID. */
const char* ssid = "coldspot";

/** @brief WiFi network password. */
const char* password = "123456781";

/** @brief Server host address. */
const char* host = "10.0.0.3";

/** @brief Server port number. */
const uint16_t port = 16789;

/** @brief Flag indicating a significant temperature difference. */
bool bigDiff = false;

/** @brief Pointer to the asynchronous client. */
AsyncClient* client = nullptr;

/** @brief SHT31 sensor object initialized with I2C address 0x44. */
SHT31 sht(0x44);

/** @brief Temperature outside the sensor location. */
float temperatureOutside = 0.0;

/** @brief Temperature inside the sensor location. */
float temperatureInside = 0.0;

/** @brief Humidity inside the sensor location. */
float humidityInside = 0.0;

/** @brief Stores the previous time for sensor reading interval. */
unsigned long previousMillis = 0;

/** @brief Interval for sensor reading in milliseconds. */
const long interval = 200;  // ms

/** @brief Duration to display temperature in milliseconds. */
int tempDuration = 10000;   /*ms*/

/** @brief Flag to indicate whether to show temperature or humidity. */
bool showTemp = true;

/**
 * @brief Sets up the WiFi connection, initializes the client, and configures the display.
 */
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

  // Initialize the display
  myDisplay.begin();
  myDisplay.setIntensity(10);  // Set the intensity (brightness) of the display (0-15)
  myDisplay.displayClear();  // Clear the display

  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.setInvert(true);
  myDisplay.print("!INIT!");
  myDisplay.setInvert(false);
  delay(750);
  myDisplay.displayClear();
}

/**
 * @brief Displays a fire warning on the LED matrix display.
 * @param state The state to determine if the fire warning should be displayed.
 */
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

/**
 * @brief Processes the received string and updates temperature or humidity values.
 * @param input The received string input.
 */
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

/**
 * @brief Displays the temperature on the LED matrix display.
 * @param temperatureOutside The temperature to display.
 */
void displayTemp(float temperatureOutside) {
  char tempDisplay[10];
  sprintf(tempDisplay, "%2.1f%cC", temperatureOutside, '\xB0');
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(tempDisplay);
}

/** @brief Previous temperature inside the sensor location. */
float oudeTempInside = 0.0;

/** @brief Previous humidity inside the sensor location. */
float oudeHumidInside = 0.0;

/**
 * @brief Reads the sensor data and sends it if there is a significant change.
 */
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

/**
 * @brief Main loop that continuously checks the client's connection status, reads sensor data, and updates the display.
 */
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
