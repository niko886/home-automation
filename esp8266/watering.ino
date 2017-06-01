/*
  
  API documetation: 
 
    https://www.arduino.cc/en/Reference/WiFi

  Github:

    https://github.com/niko886/home-automation

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h>

#define PIN_ZONE_0 0
#define PIN_ZONE_1 2
#define PIN_ZONE_2 4
#define PIN_ZONE_3 5
#define PIN_ZONE_4 12
#define PIN_ZONE_5 13
#define PIN_ZONE_6 14
#define PIN_ZONE_7 16

byte valveMap[8] = {PIN_ZONE_0, 
  PIN_ZONE_1, 
  PIN_ZONE_2, 
  PIN_ZONE_3, 
  PIN_ZONE_4, 
  PIN_ZONE_5, 
  PIN_ZONE_6, 
  PIN_ZONE_7};
  
byte valvesStatus[8] = {0};

const char* ssid = "...";
const char* password = "...";
const char* mqtt_server = "...";

IPAddress ip(192, 168, 0, 0);
IPAddress mask(255, 255, 255, 0);
IPAddress gate(0, 0, 0, 0);

WiFiClient espClient;
PubSubClient client(espClient);


#include <stdarg.h>
void log(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

long lastMsg = 0;
char msg[50];
int value = 0;

void initValves(){
  unsigned int i = 0;
  unsigned int valveId = 0;

  for (i = 0; i < sizeof(valvesStatus); i++){

    valveId = valveMap[i];

    if (valvesStatus[i]){
      digitalWrite(valveMap[i], HIGH);
    }else{
      digitalWrite(valveMap[i], LOW);  
    }    
  }
}

void setup() {
  
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(PIN_ZONE_0, OUTPUT);
  pinMode(PIN_ZONE_1, OUTPUT);
  pinMode(PIN_ZONE_2, OUTPUT);
  pinMode(PIN_ZONE_3, OUTPUT);
  pinMode(PIN_ZONE_4, OUTPUT);
  pinMode(PIN_ZONE_5, OUTPUT);
  pinMode(PIN_ZONE_6, OUTPUT);
  pinMode(PIN_ZONE_7, OUTPUT);

  initValves();
  
}
  
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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



void notifyWork(String str){

  client.publish("water/notify", str.c_str());
  log("[+] notify send: %s\n", str.c_str());
}


#define MAX_BUF 128


void switchValve(char zoneNumber, char onOf){

  int action;
  String actionText;

  if (onOf == '0'){
    action = LOW;
    actionText = "off";
  } else if (onOf = '1'){
    action = HIGH;
    actionText = "on";
  } else {
    log("[!] unknown code: %c\n", onOf);
    return;
  }


  switch(zoneNumber){

    case '0':
      valvesStatus[0] = action;
      notifyWork(String("0: ") + actionText);
      break;
      
    case '1':
      valvesStatus[1] = action;
      notifyWork(String("1: ") + actionText);
      break;
            
    case '2':
      valvesStatus[2] = action;
      notifyWork(String("2: ") + actionText);
      break;

    case '3':
      valvesStatus[3] = action;
      notifyWork(String("3: ") + actionText);
      break;

    case '4':
      valvesStatus[4] = action;
      notifyWork(String("4: ") + actionText);
      break;
      
    case '5':
      valvesStatus[5] = action;
      notifyWork(String("5: ") + actionText);
      break;
            
    case '6':
      valvesStatus[6] = action;
      notifyWork(String("6: ") + actionText);
      break;

    case '7':
      valvesStatus[7] = action;
      notifyWork(String("7: ") + actionText);
      break;
      
    default:
      log("[!] unknown zone: %c\n", zoneNumber);
      notifyWork("? zone");
      break;
  }  
}

void callback(char* topic, byte* payload, unsigned int length) {
  
  char buf[MAX_BUF] = {0};
  static bool isPing = false;
  
  memcpy(buf, payload, length);

  String payloadStr = String(buf);
  String topicStr = String(topic);

  do{

    log("[+] received message : %s | %s\n", topic, payloadStr.c_str());

    if (length > MAX_BUF){
      log("[!] received too long string (%d - actual, %d - max)\n", length, MAX_BUF);
      break;
    }

    if (topicStr == "water/ping"){

      if (isPing){
        notifyWork("pong");
      }else{
        notifyWork("ping");
      }

      isPing = !isPing;
      break;
    }

    if (topicStr == "water"){
      switchValve(payload[0], payload[1]);
    }


  }while(0);

}

void reconnect() {

  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
   
    if (client.connect("ESP8266Client-watering")) {
      Serial.println("connected");
      notifyWork("[Connected]");
      client.subscribe("water", 1);
      client.subscribe("water/ping", 1);
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

  initValves();

  delay(50);
  
}
