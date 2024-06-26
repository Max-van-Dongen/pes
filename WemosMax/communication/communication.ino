#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>

const char* ssid = "coldspot";
const char* password = "123456781";
const char* host = "192.168.217.130";
const uint16_t port = 16789;

AsyncClient* client = nullptr;

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
    Serial.println("");
  },
                 nullptr);

  // Connect to the server
  client->connect(host, port);
}

void loop() {

  if (client && client->connected()) {
    client->write("Send:14:LedOff\n");
    Serial.println("Off");
    delay(500);
    client->write("Send:14:LedOn\n");
    Serial.println("On");
    delay(500);
  } else {
    Serial.println("Client not connected");
    if (!client->connecting()) {
      client->connect(host, port);
    }
    delay(500);
  }
}
