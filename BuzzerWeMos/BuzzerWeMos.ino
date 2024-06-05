//Id 9
#include <Buzzer.h>

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>



const char* ssid = "coldspot";
const char* password = "123456781";
const char* host = "192.168.156.130";
const uint16_t port = 16789;

AsyncClient* client = nullptr;

Buzzer buzzer(14);

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
