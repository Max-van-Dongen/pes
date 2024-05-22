// Including the required Arduino libraries
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
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
const char* host = "192.168.207.130";
const uint16_t port = 16789;

AsyncClient* client = nullptr;


float temperature = 0.0;
float humidity = 0.0;
unsigned long previousMillis = 0;
const long interval = 5000;  // 2 seconds interval
bool showTemp = true;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

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
    Serial.print("Received: ");
    Serial.write((uint8_t*)data, len);
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
    temperature = tempValueString.toFloat();
  }

  if (humidIndex != -1) {
    int startIndex = humidIndex + 6;  // Move past "Humid:"
    int endIndex = input.indexOf(' ', startIndex);
    if (endIndex == -1) endIndex = input.length();  // If no space found, use the end of the string
    String humidValueString = input.substring(startIndex, endIndex);
    humidity = humidValueString.toFloat();
  }
}

void displayTemp(float temperature) {
  char tempDisplay[10];
  sprintf(tempDisplay, "%2.1f%cC", temperature, '\xB0');
  myDisplay.setTextAlignment(PA_CENTER);
  myDisplay.print(tempDisplay);
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
    if (showTemp) {
      Serial.print("Temperature: ");
      Serial.println(temperature);
      char tempDisplay[10];
      sprintf(tempDisplay, "%2.1f%cC", temperature, '\xB0');
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print(tempDisplay);
    } else {
      Serial.print("Humidity: ");
      Serial.println(humidity);
      char tempDisplay[10];
      sprintf(tempDisplay, "%.1f%%", humidity);
      myDisplay.setTextAlignment(PA_CENTER);
      myDisplay.print(tempDisplay);
    }
    showTemp = !showTemp;  // Toggle between temperature and humidity
  }
}