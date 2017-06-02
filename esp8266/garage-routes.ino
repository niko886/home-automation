/*
  
  API documetation: 
 
    https://www.arduino.cc/en/Reference/WiFi

  Github:

    https://github.com/niko886/home-automation


  Original mac:

    MAC: 8A:50:2C:7F:CF:5C


*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

extern "C" {
#include "user_interface.h"
}

// Update these with values suitable for your network.

#define PIN_MAIN_ROUTE 16 
#define PIN_GARAGE_ROUTE 12
#define PIN_GARAGE_ROUTE2 14

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";

IPAddress ip(192, 168, 0, 0);
IPAddress mask(255, 255, 255, 0);
IPAddress gate(0, 0, 0, 0);

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(PIN_MAIN_ROUTE, OUTPUT);
  pinMode(PIN_GARAGE_ROUTE, OUTPUT);
  pinMode(PIN_GARAGE_ROUTE2, OUTPUT);

  digitalWrite(PIN_MAIN_ROUTE, LOW);
  digitalWrite(PIN_GARAGE_ROUTE, LOW);
  digitalWrite(PIN_GARAGE_ROUTE2, LOW);

}


void printMac(byte* mac){
  
  Serial.print(mac[5],HEX);
  Serial.print(":");
  Serial.print(mac[4],HEX);
  Serial.print(":");
  Serial.print(mac[3],HEX);
  Serial.print(":");
  Serial.print(mac[2],HEX);
  Serial.print(":");
  Serial.print(mac[1],HEX);
  Serial.print(":");
  Serial.println(mac[0],HEX);
}


void changeMac(byte lastByte){

  byte mac[6];

  Serial.print("Original MAC:");
  
  WiFi.macAddress(mac);
  printMac(mac);

  if (lastByte){
    mac[5] = lastByte;
    wifi_set_macaddr(STATION_IF, mac);

    // Change MAC 
    WiFi.macAddress(mac);

    Serial.print("New MAC:");
    printMac(mac);
  }

}

  
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);

  changeMac(0);
  
  WiFi.config(ip, gate, mask);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void openCloseGates(int gatesNum){

  digitalWrite(gatesNum, HIGH);
  delay(10);
  digitalWrite(gatesNum, LOW);
  
}

void notifyWork(String str){

  client.publish("notifyRoutes", str.c_str());
  Serial.println(str.c_str());
}



void callback(char* topic, byte* payload, unsigned int length) {

  static int isPing = true;

  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  char buf[64] = {0};

  memcpy(buf, payload, length);

  String payloadStr = String(buf);
  String topicStr = String(topic);

  Serial.println(payloadStr);

  if (topicStr == "ping"){

    if (isPing){
      notifyWork("[+] pong!");
    }else{
      notifyWork("[+] ping!");
    }

    isPing = !isPing;
    
  } else if (topicStr == "controlRoutes"){
  
    if (payloadStr == String("mainRoute")){
      
      openCloseGates(PIN_MAIN_ROUTE);
      notifyWork("[+] 1");
      
    } else if (payloadStr == "garageRoute"){
      
      openCloseGates(PIN_GARAGE_ROUTE);
      notifyWork("[+] 2");
      
    } else if (payloadStr == "garageRoute2"){
      
      openCloseGates(PIN_GARAGE_ROUTE2);
      notifyWork("[+] 3");
      
    } else {
      Serial.println("[!] unknown command");
    }
    
  }

}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
   
    if (client.connect("ESP8266Client-garage")) {
      Serial.println("connected");
      notifyWork("[Connected]");
      client.subscribe("controlRoutes", 1);
      client.subscribe("ping", 1);
    } else {
      
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  digitalWrite(PIN_MAIN_ROUTE, LOW);
  digitalWrite(PIN_GARAGE_ROUTE, LOW);
  digitalWrite(PIN_GARAGE_ROUTE2, LOW);

  
}
