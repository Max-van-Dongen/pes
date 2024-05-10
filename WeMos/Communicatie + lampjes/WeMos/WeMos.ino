#include <ESP8266WiFi.h>
#include <WiFiClient.h>
const char* ssid = "coldspot";
const char* password = "123456781";
const char* serverIP = "192.168.25.207";
const char clientId = 'a';

const int serverPort = 6789;

WiFiClient client;

const int ledje = 0;
const int ledje2 = 2;
int i = 4;
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
  if (!client.connect(serverIP, serverPort)) { 
    Serial.println("Verbinding mislukt. Opnieuw proberen...");
    delay(5000);
    return;
  }
    Serial.println("Verbonden met server");
    Serial.printf("status: %i\r\n", client.status());
    
    sendmsg('a','v');
    
    delay(100);
    digitalWrite(ledje, LOW);
    digitalWrite(ledje2, LOW);
    
    delay(2500);
    //if (client.read() != -1) {
      ontvangst = client.read();
    while (ontvangst != -1) {
      Serial.printf("%c",ontvangst);


      ontvangst = client.read();
    }
    Serial.println();
}

void sendmsg(char dest, char data) {
  char buff[50];
  
  sprintf(buff, "%c%c%c", clientId, dest, data);
  client.write(buff);
}