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

struct response {
  char server;
  char datatype;
  String data;
};

struct response sendmsg(char dest, char *data) {
  
  char buff[50];
  sprintf(buff, "%c%c%s\0", clientId, dest, data);
  client.write(buff);
  
  delay(40);

  struct response res;
  res.server = client.read();
  res.datatype = client.read();

  if (res.datatype == '2') {
    res.data = client.readString();
  }
  
  return res;
}

void loop() {
  if (!client.connect(serverIP, serverPort)) { 
    Serial.println("Verbinding mislukt. Opnieuw proberen...");
    delay(5000);
    return;
  }
    
  struct response res = sendmsg('b',"iku");

  
  if (res.datatype == '2') {
    Serial.println(res.data.c_str());
  } else {
    Serial.println("got nothing :(");
  }

  delay(500);
}