/*--------------------------------------------------------------------------
  --------------------------VariÃ¡veis globais ajustaveis: ------------------------*/
char intervaloTempo[10] = "10"; // em milesegundos, para atualizar a temperatura lida
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
#define led D1
/*--------------------------------------------------------------------------/*--------------------------------------------------------------------------
  --------------------------Configuracao SENSOR IR ------------------------*/
#include <IRremoteESP8266.h>
#include <IRsend.h>
IRsend irsend(14); //FUNÇÃO RESPONSÁVEL PELO MÉTODO DE ENVIO DO SINAL IR / UTILIZA O GPIO14(D5)
int tamanho = 259; //TAMANHO DA LINHA RAW(68 BLOCOS)
int frequencia = 32; //FREQUÊNCIA DO SINAL IR(32KHz)
// BOTÃO LIGA
uint16_t liga[259] = {3292, 1624,  398, 418,  410, 440,  358, 1232,  420, 412,  398, 1238,  398, 408,  396, 454,  360, 458,  358, 1248,  394, 1220,  418, 446,  360, 458,  356, 460,  358, 1232,  400, 1214,  426, 452,  356, 460,  360, 456,  360, 456,  358, 458,  358, 458,  360, 456,  360, 456,  358, 458,  360, 458,  356, 460,  358, 458,  358, 458,  358, 1248,  386, 458,  358, 456,  362, 458,  358, 458,  356, 460,  358, 458,  358, 458,  360, 1246,  386, 456,  360, 458,  358, 458,  358, 460,  360, 1226,  406, 1228,  404, 1250,  386, 1242,  392, 1226,  406, 1248,  382, 1258,  378, 1228,  404, 460,  358, 458,  356, 1250,  386, 456,  358, 458,  356, 460,  356, 460,  356, 460,  356, 460,  358, 458,  360, 456,  360, 1246,  408, 1200,  416, 454,  358, 458,  356, 1252,  384, 460,  356, 462,  356, 458,  360, 456,  360, 458,  358, 458,  356, 1250,  388, 458,  356, 460,  358, 458,  356, 460,  358, 458,  358, 458,  358, 458,  358, 458,  358, 458,  354, 462,  354, 462,  356, 460,  358, 458,  358, 458,  362, 432,  384, 456,  356, 460,  358, 458,  358, 456,  362, 456,  358, 418,  400, 458,  358, 432,  386, 432,  380, 460,  356, 460,  358, 438,  380, 458,  358, 454,  360, 436,  384, 450,  366, 456,  358, 456,  360, 452,  362, 436,  380, 434,  378, 434,  384, 432,  384, 440,  376, 456,  362, 424,  394, 434,  380, 454,  362, 430,  386, 428,  392, 1246,  398, 446,  356, 460,  356, 1236,  402, 1234,  398, 1234,  396, 1242,  392, 462,  356, 1250,  400, 444,  356, 460,  358};  // UNKNOWN 7F5F07F8
// BOTÃO DESLIGA
uint16_t desliga[] = {4500, 4200, 650, 1450, 700, 400, 650, 1450, 650, 1500, 650, 400, 650, 450, 650, 1500, 600, 450, 650, 450, 600, 1500, 650, 450, 600, 500, 600, 1500, 600, 1550, 600, 450, 600, 1550, 600, 450, 600, 1550, 600, 1550, 600, 1550, 600, 1550, 600, 450, 600, 1550, 600, 1500, 600, 1550, 600, 450, 600, 500, 600, 450, 600, 500, 550, 1550, 600, 500, 550, 500, 600, 1550, 600, 1500, 600, 1550, 600, 500, 550, 500, 600, 450, 600, 500, 600, 450, 600, 500, 600, 450, 600, 450, 600, 1550, 600, 1550, 600, 1550, 550, 1600, 550, 1550, 550}; //COLE A LINHA RAW CORRESPONDENTE DENTRO DAS CHAVES
/*--------------------------------------------------------------------------
  /*--------------------------------------------------------------------------
  --------------------------Configuracao ServerMQTT ------------------------*/
#include <PubSubClient.h>
char mqttServer[40] = "m14.cloudmqtt.com";
char mqttPort[10] = "15015";
char mqttUser[40] = "nyggmttv";
char mqttPassword[40] = "hMmfyHEX9rIr";
char mqttTopic[40] = "TemperaturaAtual";
char mqttName[40] = "Micros";
WiFiClient espClient;
PubSubClient client(espClient);
WiFiManagerParameter custom_mqtt_server("server", "mqttServer", mqttServer, 40);
WiFiManagerParameter custom_mqtt_port("port", "mqttPort", mqttPort, 10);
WiFiManagerParameter custom_mqtt_user("user", "mqttUser", mqttUser, 40);
WiFiManagerParameter custom_mqtt_password("password", "mqttPassword", mqttPassword, 40);
WiFiManagerParameter custom_mqtt_topic("topic", "mqttTopic", mqttTopic, 40);
WiFiManagerParameter custom_mqtt_name("name", "mqttName", mqttName, 40);
WiFiManagerParameter custom_intervalo_tempo("intervaloTempo", "intervaloTempo", intervaloTempo, 10);
/*--------------------------------------------------------------------------
  --------------------------------------------------------------------------*/
void initMQTT() {
  int port = parseInt(mqttPort);
  client.setServer(mqttServer, 15015);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect(mqttName, mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }
}

void piscaled() {
  for (int i = 0; i < 15; i++) {
    digitalWrite(led, HIGH);
    delay(150);
    digitalWrite(led, LOW);
    delay(150);
  }
}
void rotinaModo() {
  // faz a leitura do pino D2 (no nosso caso, o botão está ligado nesse pino)
  modo = digitalRead(botao);
  // checa se o botão está pressionado
  if (modo == HIGH) {
    piscaled();
    delay(1000);
    WiFiManager wifiManager;
    wifiManager.startConfigPortal ( "MicrosWifiAP", "janela"  );
    Serial. println ( " conectado ... yeey :) " );
  }
  else {
    WiFiManager wifiManager;
    wifiManager.autoConnect("MicrosWifiAP", "janela");
  }
}

int parseInt(char* chars)
{
  int sum = 0;
  int len = strlen(chars);
  for (int x = 0; x < len; x++)
  {
    int n = chars[len - (x + 1)] - '0';
    sum = sum + pow(n, x);
  }
  return sum;
}

int powInt(int x, int y)
{
  for (int i = 0; i < y; i++)
  {
    x *= 10;
  }
  return x;
}

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(botao, INPUT);
  rotinaModo();
  initMQTT();
  irsend.begin(); //INICIALIZA A FUNÇÃO

  client.subscribe("TemperaturaAtual");

}

void callback(char* topic, byte* payload, unsigned int length) {

  String message;
  for (int i = 0; i < length; i++) {
    char c = (char)payload[i];
    message += c;
  }
  Serial.println(String(topic) + "=" + String(message));

  if (message >= "18" ) {
    irsend.sendRaw(liga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Liga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO;
  }
  else {

  }
  Serial.flush();
}

void loop() {
  client.loop();
  char c = Serial.read(); //VARIÁVEL RESPONSÁVEL POR RECEBER O CARACTER DIGITADO NA JANELA SERIAL

  if (c == 'l') { //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
    irsend.sendRaw(liga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Liga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
  }
  if (c == 'd') { //SE CARACTER DIGITADO FOR IGUAL A "a", FAZ
    irsend.sendRaw(desliga, tamanho, frequencia); // PARÂMETROS NECESSÁRIOS PARA ENVIO DO SINAL IR
    Serial.println("Comando enviado: Desliga");
    delay(50); // TEMPO(EM MILISEGUNDOS) DE INTERVALO ENTRE UM COMANDO E OUTRO
  }

}
