#include <NTPClient.h>//Biblioteca do NTP.
#include <WiFiUdp.h>//Biblioteca do UDP.
WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configurações.
String hora;//Váriavel que armazenara o horario do NTP.
/*------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------
  --------------------------Configuracao Sensor DS18B20 ------------------------*/
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
char temperaturaString[6];
float temperatura;
int controleTemperatura = 0;
int diferencaTemperatura = 3;
/*--------------------------------------------------------------------------
  --------------------------Configuracao Wi-FI ------------------------*/
#include <ESP8266WiFi.h>
const char* ssid = "AndroidAP";
const char* password =  "s4ohredo";
/*--------------------------------------------------------------------------
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
const char* mqttServer = "m12.cloudmqtt.com";
const int mqttPort = 10610;
const char* mqttUser = "sunpkpbw";
const char* mqttPassword = "6tZ6hn10S6jZ";
WiFiClient espClient;
PubSubClient client(espClient);
/*--------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
float getTemperatura() {
  float temp;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

void initWifi(){
   WiFi.begin(ssid, password);
   while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

void initMQTT(){
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
}

void setup() {    
  Serial.begin(115200);
  initWifi(); 
  initMQTT();
  
  ntp.begin();//Inicia o NTP.
  ntp.forceUpdate();//Força o Update.

//  DS18B20.begin();
//  temperatura = getTemperatura();
//  dtostrf(temperatura, 2, 2, temperaturaString);
 client.subscribe("TemperaturaAtual");
  
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

//  temperatura = getTemperatura();
//  dtostrf(temperatura, 2, 2, temperaturaString);

  delay(5000);  
  Serial.println(temperaturaString);
  client.publish("TemperaturaAtual", "5" );
  hora = ntp.getFormattedTime();//Armazena na váriavel HORA, o horario atual.
  Serial.println(hora);//Printa a hora já formatada no monitor.
//  client.publish("TemperaturaAtual", hora );

  
}
