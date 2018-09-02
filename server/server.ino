/*--------------------------------------------------------------------------
  --------------------------Toque quando entra no modo configuração: ------------------------*/
char* musica[] = {"La", "Re", "Fa", "Sol", "La", "Re", "Fa", "Sol", "Mi", "Pausa", "Sol", "Do", "Fa", "Mi", "Sol", "Do", "Fa", "Mi", "Re", "Fim"}; //Game of Thrones
int duracao[] = {700, 500, 300, 250, 250, 300, 200, 200, 700, 200, 500, 500, 200, 200, 200, 500, 200, 200, 500};
/*--------------------------------------------------------------------------
  --------------------------VariÃ¡veis globais ajustaveis: ------------------------*/
int intervalo_tempo; // em milesegundos, para atualizar a temperatura lida
char ssid[32];
char password[32];
char clientName[10] = "newClient";
char ipAddr[16] = "172.024.001.001";//Pi Access Point IP-Adr.
/*--------------------------------------------------------------------------
  --------------------------Configuracao Wi-FI ------------------------*/
#include <ESP8266WiFi.h>
/*--------------------------------------------------------------------------
  --------------------------Configuracao EEPROM ------------------------*/
#include <EEPROM.h>
/*--------------------------------------------------------------------------
  --------------------------Configuracao modo ------------------------*/
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>
byte modo; // false (0) - normal true (1) configuraÃ§ao
#define botao D2
#define buzzer D1
/*--------------------------------------------------------------------------
  --------------------------Configuracao NTP Server ------------------------*/
#include <NTPClient.h>//Biblioteca do NTP.
#include <WiFiUdp.h>//Biblioteca do UDP.
WiFiUDP udp;//Cria um objeto "UDP".
NTPClient ntp(udp, "a.st1.ntp.br", -3 * 3600, 60000);//Cria um objeto "NTP" com as configuraÃ§Ãµes.
String hora;//VÃ¡riavel que armazenara o horario do NTP.
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
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
const char* mqttServer = "m14.cloudmqtt.com";
const int mqttPort = 15015;
const char* mqttUser = "nyggmttv";
const char* mqttPassword = "hMmfyHEX9rIr";
WiFiClient espClient;
PubSubClient client(espClient);
/*--------------------------------------------------------------------------
  --------------------------------------------------------------------------*/

void writeEEPROM(int startAdr, int laenge, char* writeString) {
  EEPROM.begin(512); //Max bytes of eeprom to use
  yield();
  Serial.println();
  Serial.print("writing EEPROM: ");
  //write to eeprom
  for (int i = 0; i < laenge; i++)
  {
    EEPROM.write(startAdr + i, writeString[i]);
    Serial.print(writeString[i]);
  }
  EEPROM.commit();
  EEPROM.end();
}

void readEEPROM(int startAdr, int maxLength, char* dest) {
  EEPROM.begin(512);
  delay(10);
  for (int i = 0; i < maxLength; i++)
  {
    dest[i] = char(EEPROM.read(startAdr + i));
  }
  EEPROM.end();
  Serial.print("ready reading EEPROM:");
  Serial.println(dest);
}

float getTemperatura() {
  float temp;
  do {
    DS18B20.requestTemperatures();
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

void initWifi() {
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  Serial.println("Local IP: " + WiFi.localIP()); //Printa o IP que foi consebido ao ESP8266 (este ip que voce ira acessar).
  Serial.println("SSID: " + WiFi.SSID());
  Serial.println("MAC Adress: " + WiFi.macAddress());
}

void initMQTT() {
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
void rotinaModo() {
  // faz a leitura do pino D2 (no nosso caso, o botão está ligado nesse pino)
  modo = digitalRead(botao);
  // checa se o botão está pressionado
  if (modo == HIGH) {
    tocar(musica, duracao);
    delay(1000);
    WiFiManager wifiManager;
    wifiManager.startConfigPortal ( "MicrosWifiAP" );
    Serial. println ( " conectado ... yeey :) " );
  }
  else {
  }
}

void tocar(char* mus[], int tempo[]) {
  int tom = 0;
  for (int i = 0; mus[i] != "Fim"; i++) {
    if (mus[i] == "Do") tom = 262;
    if (mus[i] == "Re") tom = 294;
    if (mus[i] == "Mi") tom = 330;
    if (mus[i] == "Fa") tom = 349;
    if (mus[i] == "Sol") tom = 392;
    if (mus[i] == "La") tom = 440;
    if (mus[i] == "Si") tom = 494;
    if (mus[i] == "Do#") tom = 528;
    if (mus[i] == "Re#") tom = 622;
    if (mus[i] == "Fa#") tom = 370;
    if (mus[i] == "Sol#") tom = 415;
    if (mus[i] == "La#") tom = 466;
    if (mus[i] == "Pausa") tom = 0;
    tone(buzzer, tom, tempo[i]);
    delay(tempo[i]);
  }
  noTone(buzzer);
}

void beep() {
  tone(buzzer, 1500);
  delay(200);
  noTone(buzzer);
}
void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(botao, INPUT);
  Serial.begin(115200);
  rotinaModo();
  DS18B20.begin();
  initWifi();
  initMQTT();

  ntp.begin();//Inicia o NTP.
  ntp.forceUpdate();//ForÃ§a o Update.

  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);
  client.subscribe("TemperaturaAtual");

}

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println(String(topic) + "=" + String(message));
  Serial.flush();
}

void loop() {

  client.loop();
  temperatura = getTemperatura();
  dtostrf(temperatura, 2, 2, temperaturaString);
  Serial.println(temperaturaString);
  client.publish("TemperaturaAtual", temperaturaString );
  hora = ntp.getFormattedTime();//Armazena na vÃ¡riavel HORA, o horario atual.
  Serial.println(hora);//Printa a hora jÃ¡ formatada no monitor.
  //  client.publish("TemperaturaAtual", hora );
  delay(5000);
}


