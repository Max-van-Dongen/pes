/**@file*/

/**
 * @brief Includes the Buzzer library.
 */
#include <Buzzer.h>

/**
 * @brief Includes the ESP8266 WiFi library.
 */
#include <ESP8266WiFi.h>

/**
 * @brief Includes the ESPAsyncTCP library.
 */
#include <ESPAsyncTCP.h>

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
const char* host = "192.168.156.130";

/**
 * @brief Server port number.
 */
const uint16_t port = 16789;

/**
 * @brief Pointer to the asynchronous client.
 */
AsyncClient* client = nullptr;

/**
 * @brief Buzzer object initialized on pin 14.
 */
Buzzer buzzer(14);

/**
 * @brief Sets up the WiFi connection, initializes the client and defines event handlers.
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  buzzer.begin(0);

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
    c->write("Client:9\n");
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
}

/**
 * @brief Processes the received string and triggers buzzer sounds based on specific keywords.
 * @param input The received string input.
 */
void processReceivedString(String input) {
  int belIndex = input.indexOf("Bel");
  int rfidIndex = input.indexOf("RfidF");

  if (belIndex != -1) {
      buzzer.sound(1000, 400);
      buzzer.sound(750, 500);
  }

  if (rfidIndex != -1) {
      buzzer.sound(600, 250);
      buzzer.sound(1500, 500);
      buzzer.sound(600, 250);
  }
}

/**
 * @brief Main loop that continuously checks the client's connection status and attempts reconnection if necessary.
 */
void loop() {
  // put your main code here, to run repeatedly:
  
  if (client && client->connected()) {
  } else {
    Serial.println("Client not connected");
    if (!client->connecting()) {
      client->connect(host, port);
    }
    delay(500);
  }

  //buzzer.sound(2000, 5);
  
  
  //buzzer.end(1000);
}
