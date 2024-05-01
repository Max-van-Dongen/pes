#include <ESP8266WiFi.h>
#include <WiFiClient.h>
const char* ssid = "coldspot";
const char* password = "123456781";
const char* serverIP = "192.168.230.130";

const int serverPort = 6789;

WiFiClient client;

const int ledje = 0;
const int ledje2 = 2;
int i = 0;
int ontvangst = 0;

void setup() {
  Serial.begin(115200);
  //pinMode(knopje, INPUT);
  pinMode(ledje, OUTPUT);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
  }

  Serial.println("success!");
  Serial.print("IP Address is: ");
  Serial.println(WiFi.localIP());
  
}

void loop() {
  if (!client.connected()) 
    if (client.connect(serverIP, serverPort)) {
      while (true) {
      Serial.println("Verbonden met server");
      //if (knopje == HIGH) {
      client.println("1");
      delay(100);
      client.println("Dit is een test \n\n");
      delay(100);
      digitalWrite(ledje, LOW);
      digitalWrite(ledje2, LOW);
      client.println(i);
      delay(2500);
      i++;
      //if (client.read() != -1) {
        ontvangst = client.read();
      while (ontvangst != -1) {
          Serial.println(ontvangst);
        if (ontvangst == 49) {
          digitalWrite(ledje, HIGH);
          ontvangst = client.read();
      }
        if (ontvangst == 50) {
          Serial.println("Ledje hoor tnu aan te zijn!");
          digitalWrite(ledje2, HIGH);
          ontvangst = client.read();
        }
      }
      //}
      //}
      if (!client.connected()) {
        break;
      }
      }
    } else {
      Serial.println("Verbinding mislukt. Opnieuw proberen...");
      delay(5000);
      return;
    }
  
  // Send a request every 30 seconds
  //Serial.println("30 seconden voorbij");
  //delay(30000);
}