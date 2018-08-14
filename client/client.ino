#include <ESP8266WiFi.h>
#include <PubSubClient.h>
 
const char* ssid = "AndroidAP";
const char* password =  "s4ohredo";
const char* mqttServer = "m12.cloudmqtt.com";
const int mqttPort = 10610;
const char* mqttUser = "sunpkpbw";
const char* mqttPassword = "6tZ6hn10S6jZ";
int cont = 20;
WiFiClient espClient;
PubSubClient client(espClient);
 
void setup() {
 
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("Micros", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
      
    }
  }


 client.subscribe("Temperatura Univates");

  
  
  
 
}
 
void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println(String(topic) + "=" + String(message));
  if (message == "1") {
    digitalWrite(D5, 1);
  } else {
    digitalWrite(D5, 0);
  }
  Serial.flush();
}
 
void loop() {
  client.loop();


  char tmp[100];
  cont++;
  sprintf(tmp, "%d", cont);

  delay(5000);
  
  client.publish("Temperatura Univates", tmp );

  
}
